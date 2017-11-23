//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand      30th Apr 2003   InputList
//--Update	JRM Annand      22nd Jan 2007   4v0 update
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2NetSource
// Data comes from TCP/IP stream socket...normally online data acquisition
//
//---------------------------------------------------------------------------

#include "TA2NetSource.h"
#include "ARSocket_t.h"


//---------------------------------------------------------------------------
TA2NetSource::TA2NetSource( Char_t* name, Char_t* remoteHost,
			    Int_t socketport, Int_t save, Int_t rsize,
			    Int_t swap  ) :
  TA2DataSource( name, save, rsize )
{
  // Create a TCP/IP network socket to read data from a remote node

  fSocket = new ARSocket_t("A2Socket", remoteHost, socketport, ESkRemote, 0,0,
			   this);
  fSocket->SetRetry(2);
  fIsSwap = swap;
}

//---------------------------------------------------------------------------
TA2NetSource::~TA2NetSource()
{
  // Delete TCP/IP network socket

  if( fSocket ) delete fSocket;
}

//---------------------------------------------------------------------------
void TA2NetSource::Initialise( )
{
  // Initialise network socket and get some parameters back
  // Any initilisation failure is a fatal error
  // Check for outlandish transfer length numbers...if they are wrong
  // try byte swapping.

  fSocket->Initialise();
  fInRecLen = fSocket->GetBuffLen();        // record length
  fInPath = fSocket->GetID();               // I/O path to socket
  fPacketLen = fSocket->GetPacketLen();     // transfer packet length

  // Byte swap test
  if( (fInRecLen > EMaxDataLength) || (fPacketLen > EMaxDataLength) ||
      (fInRecLen < 0)         || (fPacketLen < 0) ) {
    fInRecLen = Swap4Byte(fInRecLen);
    fPacketLen = Swap4Byte(fPacketLen);
    fIsSwap = ETrue;                        // swap flag set
  }

  // Standard initialisation
  TA2DataSource::Initialise( );
}

//---------------------------------------------------------------------------
void TA2NetSource::Process()
{
  // Main body of data (from a TCP/IP socket) processing code
  // read data until the shutdown flag detected or error

  fprintf( fLogStream,"Starting input of data from TCPIP socket\n\
  Node: %s,  Socket Port: %d, Record Length: %d, Packet Length: %d\n\n",
	   fSocket->GetHost(), fSocket->GetPort(), fInRecLen, fPacketLen );
	   
  // read data in continuous loop while full record length read
  fNfile = 0;
  Char_t name[128];
  UInt_t header;
  for(;;){
    if( fIsStore ){
      strcpy(name, fFileName[0]);
      strcat(name,"_");
      sprintf(name+strlen(name),"%d",fNfile);
      strcat(name,".dat");
      if( (fOutPath = open(name, (O_RDWR | O_CREAT),(S_IRUSR|S_IWUSR))) == -1 )
	PrintError(name,"<Open SOURCE data file>",EErrFatal);
    }
    fNrecord = 0;		// init record number
    for(;;){
      // Check if current sub-buffer is empty, if not wait until it is
      fBuffer->WaitEmpty();
      ReadCheck();
      fNrecord++;
      if(fIsStore){
	if(write(fOutPath,fBuffer->GetStore(),fInRecLen) != fInRecLen){
	  PrintError("","<Date write to file>",EErrFatal);
	}
      }
      header = *(UInt_t*)(fBuffer->GetStore());
      fBuffer->FNext();            // Mark current buffer full and go to next
      if( fNbyte != fInRecLen ) break;
      if( header == EEndBuff ) break;
    }
    if( header == EEndBuff ){
      fprintf(fLogStream,"TA2NetSource: %d records read from file %d\n",
	      fNrecord, fNfile);
      fNfile++;
    }
    else{
      PrintMessage("Shutdown message received from DAQ\n");
      return;
    }
  }
}

//---------------------------------------------------------------------------
void TA2NetSource::Shutdown()
{
  // Close the TCP/IP socket connection

  delete fSocket;
}

//---------------------------------------------------------------------------
void TA2NetSource::HandShakeSk( int packet )
{
  // Return write to remote node through internet socket
  //
  if( fIsSwap ) packet = Swap4Byte(packet);
  if( write(fInPath, &packet, sizeof(Int_t)) != sizeof(Int_t) )
    PrintError("","<Socket Handshake>",EErrFatal);
  return;
}

//---------------------------------------------------------------------------
void TA2NetSource::ReadCheck()
{
  // Read a data buffer from a network socket
  // mtu.lrecord is the maximum size (bytes) of a complete data buffer
  // mtu.lpacket is the maximum no. of bytes sent in a single
  // read/write handshake
  // nbyte is the total buffer length read
  //
  Int_t bytes;			// no. of bytes single read
  Int_t maxbytes;		// max no. of bytes read
  Int_t packetbytes;		// accumulated packet bytes
  void* inbuff = fBuffer->GetStore();// running input buffer pointer

  fNbyte = 0;			// init accumulated buffer bytes

  do{				// main loop to accumulate packets
    packetbytes = 0;
    maxbytes = (fNbyte + fPacketLen) > fInRecLen ? (fInRecLen - fNbyte) 
      : fPacketLen;               // check for possible overflow
    // loop to accumulate single packet
    do{
      bytes = read(fInPath,inbuff,maxbytes); // read from the net socket
      switch(bytes){           	       // check if the read is OK
      case 0:		               // hard error
	*((UInt_t*)inbuff) = EKillBuff;// flag shutdown
	bytes = 30;
      case 8:                          // End Of File
      case 30:                         // kill the system
	fNbyte = bytes;
	HandShakeSk(bytes);            // remote node waits for it
	return;		               // nothing more to do
      default:	                       // "normal" read
	packetbytes += bytes;
	inbuff = (Char_t*)inbuff + bytes;
	break;
      }
    }while(packetbytes < maxbytes);

    fNbyte += packetbytes;              // accumulate packet contents
    HandShakeSk(packetbytes);	        // remote node waits for this

  }while(fNbyte < fInRecLen);
  fNrecord++;				// another record read

  return;
}

//---------------------------------------------------------------------------
void TA2NetSource::InputList(Char_t* name, UInt_t start, UInt_t stop)
{
  // Include file name in list of input files to analyse

  if( !fFileName ) fFileName = new Char_t*[1];
  
  fFileName[0] = new char[strlen(name)+1];
  strcpy(fFileName[0], name);
}

ClassImp(TA2NetSource)
