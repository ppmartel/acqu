//--Author	JRM Annand    4th Apr 2006
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 9th Feb 2007  Integrate with AcquRoot
//--Rev 	JRM Annand... 7th Sep 2010  getenv("HOST") not "localhost"
//--Rev 	JRM Annand... 9th Sep 2010  add data no-store
//--Update	JRM Annand... 1st Oct 2012  print message when connecting
//--Update  A Neiser...   6th June 2013  Make char_t* const
//--Update  A Neiser...   21st June 2013 If no HOST set, listen on all devices
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQstore
// Storeage of data read out from experimental hardware by DAQ
// Typically the data is stored on local disk or send over a network socket

#include "TDAQstore.h"
#include "ARSocket_t.h"
#include "ARFile_t.h"
#include "TA2Mk2Format.h"


// Type of data output channel
static Map_t kExpDataOut[] = {
  {"ENet",        EStoreDONet },         // data output over ethernet
  {"Disk",        EStoreDODisk },        // data storage direct to disk
  {"AcquRoot",    EStoreDOAcquRoot },    // couple directly to AcquRoot
  {"No-Store",    EStoreDOUndef },       // don't store data
  {NULL,          EStoreDOUndef }
};

//-----------------------------------------------------------------------------
TDAQstore::TDAQstore( Char_t* name, Char_t* input, FILE* log, Char_t* line )
  : TA2System( name, NULL, input, log )
{
  // Initialise data storage class
  //
  fOutBuff = NULL;
  fSocket = NULL;
  fDataOutFile = NULL;
  fFileName = NULL;
  fRecLen = fPacLen = fPort = fMode = 0;
  fNfile = fNrecord = 0;
  // fRingSize = 0;
  fIsSwap = kFALSE;
  Char_t mode[256];
  if( sscanf( line, "%*s%*d%*d%d%s%d%d",
	      &fRecLen, mode, &fPacLen, &fPort ) != 4 )
    PrintError(line,"<Parse data-storage specification>",EErrFatal);
  fMode = Map2Key( mode, kExpDataOut );
  return;
}

//----------------------------------------------------------------------------
TDAQstore::~TDAQstore()
{
  //  Flush semaphores and memory
  //  after wait a second to allow other processes to finish
  //
  if( fOutBuff ) delete fOutBuff;
}

//-----------------------------------------------------------------------------
void TDAQstore::PostInit( )
{
  // Create buffers and data path(s)
  //  fOutBuff = new TA2RingBuffer( fRecLen, fRingSize );
  //    fOutBuff = new TA2RingBuffer( fStore->GetOutBuff() );
  const Char_t* lhost;
  switch( fMode ){
  case EStoreDONet:
    if( (!fPacLen) || (fPacLen > fRecLen) ) fPacLen = fRecLen;
    lhost = getenv("HOST");
    // if there's no HOST variable set, listen on all interfaces by default
    if(lhost==NULL)
        lhost = "0.0.0.0";
    fSocket = new ARSocket_t( "DAQ-Net-Socket", lhost, 
			      fPort, ESkLocal, fRecLen, fPacLen, this );
    fSocket->Initialise();
    break;
  case EStoreDODisk:
    break;
  default:
    break;
  }
  // this control message is needed for AcquControl script
  // to ensure AcquDAQ is running in NoStore mode
  printf("\n<TDAQstore successfully started>\n");
  fflush(stdout);
}

//---------------------------------------------------------------------------
void TDAQstore::Run()
{
  Process();
  Shutdown();
}

//---------------------------------------------------------------------------
void TDAQstore::Process()
{
  //	Main body of data (from a TCP/IP socket) processing code
  //	read data until the shutdown flag detected or error
  // read data in continuous loop while full record length read
  fNfile = 0;
  UInt_t* header;
  fNrecord = 0;		// init record number
  for(;;){
    // Check if current sub-buffer is full, if not wait until it is
    fOutBuff->WaitFull();
    fNrecord++;
    header = (UInt_t*)(fOutBuff->GetStore());
    switch( fMode ){
    case  EStoreDODisk:
      // Store to local disk
      if( *header == EHeadBuff ){
	fFileName = ((AcquMk2Info_t*)(header+1))->fOutFile;
	fDataOutFile = new ARFile_t( fFileName, O_RDWR|O_CREAT,
				     S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, this );
      }	
      if( *header == EKillBuff ) break;
      fDataOutFile->WriteBuffer(fOutBuff->GetStore(),fRecLen);
      break;
    case EStoreDONet:
      // Write data to internet socket
      fSocket->WriteChunked(fOutBuff->GetStore(),fRecLen,fPacLen);
      break;
    case EStoreDOUndef:
      // Don't store the data.....this is for diagnostic running
      break;
    }
    if( *header == EEndBuff ){
      fprintf(fLogStream,"TDAQstore: %d records written to file %d\n",
	      fNrecord, fNfile);
      if( fMode == EStoreDODisk ){
	delete fDataOutFile;
	fFileName = NULL;
      }
      fNfile++;
    }
    else if( *header == EKillBuff ){
      PrintMessage("Shutdown message received from DAQ\n");
      return;
    }	
    fOutBuff->CNext();            // Mark current buffer empty and go to next
  }
}
ClassImp(TDAQstore)
