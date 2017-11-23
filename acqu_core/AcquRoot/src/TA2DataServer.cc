//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand      24th Mar 2004   Data stream merging
//--Rev 	JRM Annand      19th Apr 2004   Write data file
//--Rev 	JRM Annand       3rd Mar 2005   Batch-mode log-file directory
//--Rev 	JRM Annand       5th Nov 2006   Flexi start offset GetEvent()
//--Rev 	JRM Annand      13th Nov 2006   Delay in multi-source start
//--Rev 	JRM Annand      10th Dec 2006   FlushBuffers() limit on loop
//--Rev 	JRM Annand       8th Jan 2007   Attempt re-align during merge
//--Rev 	JRM Annand...   22nd Jan 2007   4v0 changes ARFile_t
//--Rev 	JRM Annand...   27th Apr 2007   ProcessHeader format iterface
//--Rev 	JRM Annand...   20th Nov 2007   Local DAQ coupling
//--Rev 	JRM Annand...   22nd Mar 2008   ProcessHeader error non-fatal
//--Rev 	JRM Annand...   21st Apr 2008   Call UpdateInfo (header merge)
//--Rev 	JRM Annand...    1st May 2008   Process() no end record fix
//--Rev 	JRM Annand...    3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand...    1st Sep 2009   delete[]
//--Rev 	JRM Annand...   31st Aug 2012   Mk2 data buffer recognised 
//                                              in data merging
//--Rev 	JRM Annand...   21st Nov 2012   More Mk2 data buffer recognised
//                                              in data merging
//--Rev 	JRM Annand...    1st Mar 2013   More Mk2 merge debugging
//--Rev 	JRM Annand...    6th Mar 2013   Add TA2TAPSMk2Format
//--Rev 	JRM Annand...   18th Sep 2013   Start sources inc usleep 100-200
//--Update	JRM Annand...   29th Mar 2014   Merged data written at end-run 
//
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2DataServer
// Read data from file or network and make available to AcquRoot sorting
// process. Data may optionally be written to file
//
//---------------------------------------------------------------------------

#include "TA2DataServer.h"
#include "TA2NetSource.h"
#include "TA2FileSource.h"
#include "TA2LocalSource.h"
#include "TA2Mk1Format.h"
#include "TA2Mk2Format.h"
#include "TA2TAPSFormat.h"
#include "TA2TAPSMk2Format.h"
#include "ARFile_t.h"


// Command-line key words which determine what to read in
// constants for command-line maps below
enum { 
  EDataSrvInputStreams, EDataSrvStreamSpec,
  EDataSrvNetwork, EDataSrvFile, EDataSrvLocal,
  EDataSrvFileName, EDataSrvDirName,
  EDataSrvOpt,
  EDataSrvMk1, EDataSrvMk2, EDataSrvTAPS, EDataSrvTAPSMk2
};
// Basic Server setup keys
const Map_t TA2DataServer::kDataSrvKeys[] = {
  {"Input-Streams:",     EDataSrvInputStreams},
  {"Stream-Spec:",       EDataSrvStreamSpec},
  {"File-Name:",         EDataSrvFileName},
  {"File-Directory:",    EDataSrvDirName},
  {"Stream-Options:",    EDataSrvOpt},
  {NULL,          -1}
};
// Data source keys
static const Map_t kDataSrvSrc[] = {
  {"Network",       EDataSrvNetwork},
  {"File",          EDataSrvFile},
  {"Local",         EDataSrvLocal},
  {NULL,          -1}
};
// Data format keys
static const Map_t kDataSrvFormat[] = {
  {"Mk1",           EDataSrvMk1},
  {"Mk2",           EDataSrvMk2},
  {"TAPS",          EDataSrvTAPS},
  {"TAPSMk2",       EDataSrvTAPSMk2},
  {NULL,          -1}
};

//-----------------------------------------------------------------------------
void* A2ServerThread( void* arg )
{
  //  Threaded running of analyser
  //  so that other tasks may be performed
  //  Check that arg points to a TDataServer object 1st

  if( !((TObject*)arg)->InheritsFrom("TA2DataServer") ){
    TThread::Printf(" Error...TA2DataServer base class not supplied\n");
    return NULL;;
  }
  TA2DataServer* ds = (TA2DataServer*)arg;
  ds->Run();
  return NULL;
}

//-----------------------------------------------------------------------------
TA2DataServer::TA2DataServer( const Char_t* name, TAcquRoot* ar )
  : TA2System( name,kDataSrvKeys  )
{
//
//  Data Server constructer
//  Check validity of TAcquRoot ptr (fatal error if wrong)
//  and then Zero everything

  if( !ar->InheritsFrom("TAcquRoot") )
    PrintError(" ","<Load AcquRoot ptr>",EErrFatal);
  nStreamSpecs = 0;
  fAcquRoot = ar;
  fDataOutFile = NULL;
  fDataSource = NULL;
  fDataFormat = NULL;
  fSourceBuff = NULL;
  fSortBuff = NULL;
  fServerThread = NULL;
  fHeaderBuff = NULL;
  fFileDir = NULL;
  fEvStart = NULL;
  fEvTempStart = NULL;
  fEvLength = NULL;
  fEvOffset = NULL;
  fNEvSearch = NULL;
  fEvMaxDiff = NULL;
  fEvMax = NULL;
  fEvStatus = NULL;
  fOutBuff = NULL;
  fEndBuff = NULL;
  fEvTempBuff = NULL;
  fNDataSource = 0;
  fNDataBuffer = 0;
  fNeventError = 0;
  fRecLen = 0;
  fIsStore = EFalse;
  fIsSortLock = ETrue;
  fIsHeaderInit = EFalse;
  Char_t* logfile;
  // Check batch log-file redirection
  if( fAcquRoot->IsBatch() ){
    if( fAcquRoot->GetBatchDir() )
      logfile = BuildName(fAcquRoot->GetBatchDir(), "DataServer.log");
  }
  else logfile = BuildName("DataServer.log");
  if (fAcquRoot->IsLogFile()) SetLogFile(logfile);
  else SetLogFile(NULL);
  delete[] logfile;
  return;
}

//-----------------------------------------------------------------------------
TA2DataServer::~TA2DataServer()
{
  //
  //  if( fSem ) delete fSem;
}

//---------------------------------------------------------------------------
void TA2DataServer::SetConfig( Char_t* line, int key )
{
  // Load basic detector parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kDataSrvKeys array at the top of the source .cc file
  // The following are setup...
  //	1. # input streams to handle simultaneously
  //	2. Specification of each input stream... data source & format
  //	3. Directory spec for file input
  //	4. File names for data from disk/tape file
  // 13/1/07 added options to customise recovery from event-ID mismatch

  UInt_t i,j;
  UInt_t socketPort, recl, save, rsize, start, stop, swap, mid;
  Char_t name1[EMaxName];
  Char_t name2[EMaxName];
  Int_t n = nStreamSpecs; // shortcut

  switch( key ){
  case EDataSrvInputStreams:
    // # of input data streams to handle
    PrintMessage("Starting configuration from file\n");
    if( sscanf(line,"%d%d%d",&fNDataSource,&recl,&save) != 3 )
      PrintError( line, "<DataServer:input streams etc>", EErrFatal );
    fDataSource = new TA2DataSource*[fNDataSource];
    fDataFormat = new TA2DataFormat*[fNDataSource];
    fSourceBuff = new TA2RingBuffer*[fNDataSource];
    //    fSem = new Semaphore_t( ESourceSem  );
    fRecLen = recl;
    fIsStore = (Bool_t)save;
    fHeaderBuff = new Char_t[fRecLen];
    memset( fHeaderBuff, 0, fRecLen );
    //    fSortBuff = new Char_t[fRecLen];
    fSortBuff = new TA2RingBuffer(fRecLen, 4);
    if( fNDataSource > 1 ){
      fEvTempBuff = new Char_t[fRecLen];
      fEvStart = new void*[fNDataSource];
      fEvTempStart = new void*[fNDataSource];
      fEvLength = new UInt_t[fNDataSource];
      fEvOffset = new Int_t[fNDataSource];
      fNEvSearch = new Int_t[fNDataSource];
      fEvMaxDiff = new Int_t[fNDataSource];
      fEvMax = new Int_t[fNDataSource];
      fEvStatus = new Int_t[fNDataSource];
      for( Int_t k=0; k<fNDataSource; k++ ){
	fEvStart[k] = NULL;
	fEvTempStart[k] = NULL;
	fEvLength[k] = 0;
	fEvOffset[k] = 0;
	fNEvSearch[k] = 0;
	fEvMaxDiff[k] = 0;
	fEvMax[k] = 0;
	fEvStatus[k] = ESubEventOK;
      }
   }
    break;
  case EDataSrvStreamSpec:
    // Parameters for each data source...need to input # of elements 1st
    // Input data format and source, then do some initialisation
    if( n >= fNDataSource )            // check numbers
      PrintError( line, "<DataServer too many input streams>", EErrFatal );
    if( sscanf(line,"%s%s%d",name1,name2,&save) != 3 )
      PrintError( line, "<DataServer data format spec.>", EErrFatal );
    // Data format determine
    j = Map2Key( name2,  kDataSrvFormat );
    switch( j ){
    case EDataSrvMk1:
      // Acqu format Mk1 (original version)
      fDataFormat[n] = new TA2Mk1Format((Char_t*)"Mk1Format", fRecLen, n);
      break;
    case EDataSrvMk2:
      // Acqu format Mk2 (new version)
      fDataFormat[n] = new TA2Mk2Format((Char_t*)"Mk2Format", fRecLen, n);
      break;
    case EDataSrvTAPS:
      // TAPS format CB@Mainz experiments
      fDataFormat[n] = new TA2TAPSFormat((Char_t*)"TAPSFormat", fRecLen, n);
      break;
    case EDataSrvTAPSMk2:
      // TAPS Mk2 format CB@Mainz experiments
      fDataFormat[n] = new TA2TAPSMk2Format((Char_t*)"TAPSMk2Format", fRecLen, n);
      break;
    default:
      PrintError( line, "<DataServer unknown data format>", EErrFatal );
    }
    // Data source determination
    i = Map2Key( name1,  kDataSrvSrc );
    switch( i ){
    case EDataSrvNetwork:
      // Network connection
      if( (j = sscanf(line,"%*s%*s%*d%s%d%d%d%d",
		      name1,&socketPort,&rsize,&mid,&swap)) < 4  )
	PrintError( line, "<DataServer network source spec.>", EErrFatal );
      if( j==4 ) swap = EFalse;
      fDataSource[n] =
	new TA2NetSource( name2, name1, socketPort, save, rsize, swap );
      
      fIsSortLock = EFalse;
     break;
    case EDataSrvFile:
      // File on local disk/tape
      if( (j = sscanf(line,"%*s%*s%*d%d%d%d%d",&recl,&rsize,&swap,&mid)) < 4 )
	PrintError( line, "<DataServer file source spec.>", EErrFatal );
      fDataSource[n] =
	new TA2FileSource( (Char_t*)"FileSource",recl, save, rsize, swap );
      fIsSortLock = ETrue;
      break;
    case EDataSrvLocal:
      // DAQ read by same node
      if( (j = sscanf(line,"%*s%*s%*d%d%d%d",
		      &recl,&mid,&swap)) < 2  )
	PrintError( line, "<DataServer local source spec.>", EErrFatal );
      if( j==2 ) swap = EFalse;
      fDataSource[n] =
	new TA2LocalSource( name2, recl, save, 0, swap );
      
      fIsSortLock = EFalse;
      break;
    default:
      PrintError( line, "<DataServer unknown file source>", EErrFatal );
    }
    // Give the semaphore spec. to the data source, get the source data buffer
    // and if not previously done, create header and sort buffers
    fDataSource[n]->SetLogStream( fLogStream );   // log file for text output
    fDataSource[n]->Initialise( );
    fSourceBuff[n] = new TA2RingBuffer( fDataSource[n]->GetBuffer() );
    fDataFormat[n]->SetLogStream( fLogStream );   // same log file  
    fDataFormat[n]->Initialise( fHeaderBuff, fSortBuff, fSourceBuff[n], mid );
    nStreamSpecs++;                               // for next possible source channel
    break;
  case EDataSrvFileName:
    // Input file specification source=name, start record, stop record
    // Only has an effect where the data source is file
    if( fFileDir ) strcpy( name2, fFileDir );
    else strcpy(name2, "");
    if( (j = sscanf(line,"%s%d%d",name1,&start,&stop)) < 1 )
      PrintError( line, "<DataServer input file spec.>", EErrFatal );
    strcat( name2, name1 );
    if( j==2 ) stop = 0;
    if( j==1 ) start = 0;
    fDataSource[n-1]->InputList(name2, start, stop);
    break;
  case EDataSrvDirName:
    // Directory where input file resides
    if( (j = sscanf(line,"%s",name1)) < 1 )
      PrintError( line, "<DataServer input file directory spec.>", EErrFatal );
    fFileDir = new Char_t[strlen(name1)+1];
    strcpy(fFileDir, name1);
    break;
  case EDataSrvOpt:
    // Further options for data-stream merging
    // Event-ID offset and # subsequent events to search if mismatch found
    // Max difference of ID....ignore if greater then this
    // Max event ID to worry about....ignore if greater than this
    if( sscanf(line,"%d%d%d%d",
	       fEvOffset+n-1, fNEvSearch+n-1, fEvMaxDiff+n-1, fEvMax+n-1) < 4 )
      PrintError( line, "<DataServer stream options spec.>", EErrFatal );
    break;
  default:
    // Not a fatal error
    PrintError( line, "<DataServer unrecognised command line>" );
    return;
  }
  return;
}

//---------------------------------------------------------------------------
void TA2DataServer::Run( )
{
  // Start the loop to transfer data to sort. A data source must have
  // been defined beforehand. Run either single of multiple source processing

  if( fNDataSource < 1 )PrintError("","<No data sources specified>",EErrFatal);
  else if( fNDataSource == 1 ) Process();
  else MultiProcess();
}

//---------------------------------------------------------------------------
void TA2DataServer::Start()
{
  // Start the analysis thread going. This will run in the "background"
  // but will produce to occasional message at root prompt acqu-root>

  if( fServerThread ){
    printf(" Warning...deleting old RunThread and starting new one\n");
    fServerThread->Delete();
  }
  fServerThread = new TThread( "A2ServerThread",
			   (void(*) (void*))&(A2ServerThread),
			   (void*)this );
  fServerThread->Run();
  return;
}

//---------------------------------------------------------------------------
void TA2DataServer::StartSources()
{
  // Start the threads to run the data source handlers
  // If the run-process flag is set
  // Add a delay after each start. On fast CPUs starting of multiple
  // data sources in quick succession can cause failure

  for( Int_t i = 0; i<fNDataSource; i++ ){
    fDataSource[i]->Start();
    usleep(500);
  }
}

//---------------------------------------------------------------------------
void TA2DataServer::Process()
{
  // Take the data buffers read by a single data source classes 
  // which inherits from TA2DataSource. Pass that data to the analyser
  // Server-Source threads are synchronised via TConditions which are part
  // of the TA2RingBuffer which holds the data

  Bool_t finished = EFalse;
  void* inbuff;

  TA2DataSource* ds = fDataSource[0];        // data source handler
  TA2DataFormat* df = fDataFormat[0];        // data format handler
  TA2RingBuffer* rb = fSourceBuff[0];        // data buffer handler
  Int_t recl = ds->GetInRecLen();            // data buffer size

  // Loop until a kill signal is supplied
  while( !finished ){
    Bool_t endrun = EFalse;
    if( !ProcessHeader() ){
      df->SendTerminate();
      break;
    }
    while( !endrun ){
      // Check if the source has read a sub-buffer of data
      // Wait for it if not
      rb->WaitFull();
      inbuff = rb->GetStore();
      switch( df->GetBufferType(inbuff) ){
      case EKillBuff:
	PrintMessage("End of session detected...shuting down\n");
	df->SendTerminate();
	finished = ETrue;
      case EEndBuff:
	PrintMessage("End of run detected\n");
	if( fIsStore ){
	  fDataOutFile->WriteBuffer( inbuff, fRecLen );
	  delete fDataOutFile;
	  fDataOutFile = NULL;
	}
	endrun = ETrue;
	df->SendEOF();
	usleep(10);
	break;
      default:
	// Sometimes a file does not have an end record
	// Check here if a header buffer has been received
	// If not its a hard (fatal) error
	if( df->IsBoundary() ){
	  fprintf( fLogStream," Buffer Header %x received\n",
		   df->GetBufferType(inbuff) );
	  if( df->GetBufferType(inbuff) != EHeadBuff )
	    PrintError("","<Unrecognised data buffer header>",EErrFatal);
	  PrintMessage("End of run assumed\n");
	  if( fIsStore ){
	    delete fDataOutFile;
	    fDataOutFile = NULL;
	  }
	  endrun = ETrue;
	  df->SendEOF();
	  usleep(10);
	  goto EndBuffSwitch;    // skip buffer clear & increment
	}
	// No break here on purpose. if !fIsBoundary
	// carry on and process as data buffer.
      case EDataBuff:
      case EMk2DataBuff:
	// Byte-swap the data buffer if necessary  
	if( ds->IsSwap() )Swap4ByteBuff((UInt_t*)inbuff, recl);
	// Write buffer to disk if desired
	if( fIsStore ) fDataOutFile->WriteBuffer( inbuff, fRecLen );
	// Offline running...insist on analysing every data buffer
	// SortLock SET. Otherwise Online (data from DAQ)...
	// Send the buffer if there is space, but do not wait for the
	// analyser to make space
	if( fIsSortLock || !fSortBuff->IsFull() ) df->SendData();
	break;
      }
      // Mark sub-buffer clear for further use and shift pointer
      // to the next sub-buffer in the linked list
      rb->CNext();
      fNDataBuffer++;
    EndBuffSwitch: continue;
    }
  }
}

//---------------------------------------------------------------------------
void TA2DataServer::MultiProcess()
{
  // Merge multiple input data streams to single stream
  // Take the data buffers read by a data sources, pull out events, join
  // them and pass to the analyser
  // Server-Source threads are synchronised via TConditions which are part
  // of the TA2RingBuffers which hold the data

  Bool_t finished = EFalse;
  void* inbuff;
  UInt_t buffType;

  TA2DataSource* ds = fDataSource[0];        // data source handler
  TA2DataFormat* df = fDataFormat[0];        // data format handler
  TA2RingBuffer* rb = fSourceBuff[0];        // data buffer handler
  Int_t recl = ds->GetInRecLen();            // data buffer size

  // Loop until a kill signal is supplied
  while( !finished ){
    Bool_t endrun = EFalse;
    if( !ProcessHeader() ){
      df->SendTerminate();
      break;
    }
    while( !endrun ){
      // Check if the source has read a sub-buffer of data
      // Wait for it if not
      rb->WaitFull();
      inbuff = rb->GetStore();
      buffType = df->GetBufferType(inbuff);
      switch( buffType ){
      case EKillBuff:
	PrintMessage("End of session detected...shuting down\n");
	df->SendTerminate();
	finished = ETrue;
      case EEndBuff:
	PrintMessage("End of run detected\n");
	// If data storage enabled
	// Check if anything left in the merged output buffer
	// Write it to merged file....JRMA 29/3/14
	if( fIsStore ){
	  UInt_t* bStart = (UInt_t*)(fSortBuff->GetStore());
	  if( fOutBuff > (bStart + 1) ){
	    *fOutBuff = EBufferEnd;
	    fOutBuff = NULL; 
	    fDataOutFile->WriteBuffer(fSortBuff->GetStore(),fRecLen);
	  }
	}
	// Check that the other data sources have reached the end-of-file
	// state....flush any extraneous data buffers from these sources
	FlushBuffers();
	if( fIsStore ){
	  fDataOutFile->WriteBuffer( inbuff, fRecLen );
	  delete fDataOutFile;
	  fDataOutFile = NULL;
	}
	endrun = ETrue;
	df->SendEOF();
	usleep(10);
	break;
      default:
	if( df->IsBoundary() ){
	  fprintf( fLogStream," Buffer Header %x received\n",
		   df->GetBufferType(inbuff) );
	  PrintError("","<Unrecognised data buffer header>",EErrFatal);
	  break;
	  // Fatal Error....exit here
	}
	// No break here on purpose. if !fIsBoundary
	// carry on and process as data buffer.
      case EDataBuff:
      case EMk2DataBuff:
	// Byte-swap the data buffer if necessary  and then do the merging
	if( ds->IsSwap() )Swap4ByteBuff((UInt_t*)inbuff, recl);
	MergeBuffers(buffType);
	break;
      }
      // Mark sub-buffer clear for further use and shift pointer
      // to the next sub-buffer in the linked list
      rb->CNext();
      fNDataBuffer++;
    }
  }
}

//---------------------------------------------------------------------------
Bool_t TA2DataServer::ProcessHeader()
{
  // Read in header buffers if data format (eg. Mk1 Acqu) supplies one,
  // Otherwise update any constructed header information
  // Check its actually a header buffer...fatal error if not

  TA2DataSource* ds;        // data source handler
  TA2DataFormat* df;        // data format handler
  TA2RingBuffer* rb;        // data buffer handler

  // If the data format supplies a header e.g. Mk1 Acqu, use it to construct
  // an overall header...just copy it over if a single data source.
  // Otherwise assume the data format class constructs an artifical one
  Int_t i;
  if( !fIsHeaderInit ){                    // do this 
    for( i=0; i<fNDataSource; i++ ){
      ds = fDataSource[i];                 // data source handler
      df = fDataFormat[i];                 // data format handler
      rb = fSourceBuff[i];                 // data buffer handler
      if( df->IsHeader() ){                // header in data?
	while( !rb->IsFull() ) usleep(10); // hang about til buffer supplied
	if( ds->IsSwap() ) df->SwapHeader( rb->GetStore() );
	df->MergeHeader(fHeaderBuff,rb->GetStore(),fRecLen,ds->GetInRecLen());
	rb->CNext();                       // buffer free to fill again
      }                                    // and advance buffer ptr.
      else df->MergeHeader( fHeaderBuff, df->GetHeader(), fRecLen,
			    ds->GetInRecLen() ); // get constructed header
    }
    fDataFormat[0]->UpdateInfo();    // ensure all module info saved channel 0 
    fIsHeaderInit = ETrue;           // block further Initialisation
    //    fSem->Signal(EServerSem);              // sort waits to get header
  }
  // Procedure after end-of-file detected
  // Check 1st for a termination marker...causes DataServer shut down
  // If the data provides a new header when a new data file
  // is started then ensure it is there, and update the run-spec.
  // If the header doesn't appear its a fatal error.
  else{
    ds = fDataSource[0];                   // data source handler
    df = fDataFormat[0];                   // data format handler
    rb = fSourceBuff[0];                   // data buffer handler
    while( !rb->IsFull() ) usleep(10);     // wait for next buffer
    if( df->IsTermBuff(rb->GetStore()) ){  // check kill signal
      PrintMessage("Shutdown Message received\n");
      return EFalse;
    }
    else if( df->IsHeader() ){             // If expected, header has to show
      if( !df->IsHeadBuff(rb->GetStore()) ){
	fprintf( fLogStream,"\nHeader ID %x found\n",
		 *(UInt_t*)(rb->GetStore()) );
	//PrintError("","<Invalid ID marker to Header Buffer>",EErrFatal);
	PrintError("","<Invalid ID marker to Header Buffer>");
	return EFalse;
      }
      // Its a header, swap if necessary and update run-spec.
      if( ds->IsSwap() ) df->SwapHeader( rb->GetStore() );
      df->UpdateHeader( fHeaderBuff, rb->GetStore() );
      rb->CNext();
    }
    else df->UpdateHeader( fHeaderBuff, NULL ); // artificial update
  }
  // If data storage anabled open the data file
  // Open read/write, create, 
  if( fIsStore ){
    if( fDataOutFile ) PrintError("","<Data file already Open>",EErrFatal);
    fDataOutFile = new ARFile_t( fDataFormat[0]->GetFileName(),
				 O_RDWR|O_CREAT,
				 S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, this );
    fprintf( fLogStream," Data File: %s  Opened by TA2DataServer on path %d\n",
	     fDataOutFile->GetName(), fDataOutFile->GetPath() );
    fDataOutFile->WriteBuffer( fHeaderBuff, fRecLen );
  }
  return ETrue;
}

//---------------------------------------------------------------------------
void TA2DataServer::MergeBuffers(UInt_t buffType)
{
  // Merge events from multiple data streams
  // Initial setup of buffer delimiters/pointers for the data merging process
  if( !fOutBuff ){
    fSortBuff->WaitEmpty();
    fOutBuff = (UInt_t*)fSortBuff->GetStore();
    *fOutBuff++ = buffType;
    fEndBuff = (Char_t*)fSortBuff->GetStore() + fRecLen - sizeof(UInt_t);
  }
  UInt_t* evstart;
  UInt_t length;
  fEvStart[0] = (Char_t*)(fSourceBuff[0]->GetStore()) +
    fDataFormat[0]->GetStartOffset();
  Int_t i;
  for(;;){
    evstart = fOutBuff;;
    length = 0;
    for( i=0; i<fNDataSource; i++ ){
      GetEvent(i);
      // Nothing left in source buffer 0...return
      if( !fEvLength[0] )
	return;
      length += fEvLength[i];
    }
    if( fEndBuff < ((Char_t*)fOutBuff + length) ){
      // overflow...write end marker to output buffer
      // send the buffer, and reset output buff pointer and start
      // of event pointers if part way throught the event
      *evstart++ = EEndEvent;             // end of buffer marker
      *evstart = EBufferEnd;              // end of buffer marker
      // If data storage enabled write the data buffer
      if( fIsStore ){
	fDataOutFile->WriteBuffer(fSortBuff->GetStore(),fRecLen);
      }
      // Offline running...insist on analysing every data buffer
      // SortLock SET. Otherwise Online (data from DAQ)...
      // Send the buffer if there is space, but do not wait for the
      // analyser to make space
      if( fIsSortLock || !fSortBuff->IsFull() ) fSortBuff->FNext();            
      fSortBuff->WaitEmpty();                    // wait for next available
      fOutBuff = (UInt_t*)fSortBuff->GetStore(); // next buffer
      fEndBuff = (Char_t*)fSortBuff->GetStore() + fRecLen - sizeof(UInt_t);
      *fOutBuff++ = *((UInt_t*)(fSourceBuff[0]->GetStore()));
      evstart = fOutBuff;
    }
    CompareEvent();
    UInt_t* lenMerged = fOutBuff + 1;
    length = 0;                             // recalculate event length
    for( i=0; i<fNDataSource; i++ ){
      if( fEvStatus[i] == ESubEventOK ){
	memcpy(fOutBuff, fEvStart[i], fEvLength[i]);
	fOutBuff = (UInt_t*)((Char_t*)fOutBuff + fEvLength[i]);
	length += fEvLength[i];
      }
      if( fEvStatus[i] != ESubEventHigh ){
	if( !fEvTempStart[i] ){
	  fEvStart[i] = (Char_t*)fEvStart[i] + fEvLength[i] + 
	    fDataFormat[i]->GetStartOffset();
	}
	else fEvStart[i] = fEvTempStart[i];
      }
    }
    // Mk2 format only
    if( buffType == EMk2DataBuff ) *lenMerged = length - 4;
    *fOutBuff++ = EEndEvent;
  }
}
ClassImp(TA2DataServer)
