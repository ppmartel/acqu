//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Update	JRM Annand      20th Nov 2007   Add is process flag
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
//	TA2DataSource
//	Basis of all online/offline sources for AcquRoot data streams
//
//---------------------------------------------------------------------------

#include "TA2DataSource.h"
//#include "Semaphore_t.h"

//	ClassImp.....for ROOT implementation of classes
//	Macro defined in $ROOTSYS/include/Rtypes.h


//-----------------------------------------------------------------------------
void* A2SourceThread( void* arg )
{
  //  Threaded running of data read from source
  //  so that other tasks may be performed in parallel
  //  Check that arg points to a TDataSource object 1st

  if( !((TObject*)arg)->InheritsFrom("TA2DataSource") ){
    TThread::Printf(" Error...TA2DataSource base class not supplied\n");
    return NULL;;
  }
  TA2DataSource* ds = (TA2DataSource*)arg;
  ds->Run();
  return NULL;
}

//-----------------------------------------------------------------------------
TA2DataSource::TA2DataSource( const char* name, Bool_t save, Int_t rsize )
  : TA2System( name, NULL )
{
//
//  Default data source constructor
//  Save pointer to format class
//  Create semaphore and shared-memory classes for
//  communications with other Acqu++ processes.
//  Up to 3 Acqu++ sessions may run simultaneously....
//  GetSubCluster specifies which one is being used
//  Dont initialise any shared memory of semaphore if
//  a null pointer df is supplied

  fIsStore = save;              // default no data storage
  fRingSize = rsize;            // ring buffer size

  fSourceThread = NULL;         // To run data source as separate thread
  //  fSem = NULL;                  // pointer to semaphore array
  fFileName = NULL;             // File names
  fStartList = NULL;		// 1st record to analyse
  fStopList = NULL;		// last record to analyse
  fStart = 0;		        // 1st record to analyse
  fStop = 0;		        // last record to analyse
  fInRecLen = 0;		// no. of bytes in record
  fInPath = -1;                 // invalid I/O path until initialised
  fOutPath = -1;                // invalid I/O path until initialised
  fNfile = 0;			// counter number files processed
  fNrecord = 0;		        // current record number
  fNbyte = 0;			// no. of bytes in current record
  //  fSourceCond = NULL;           // thread sync
  //  fServerCond = NULL;           // thread sync
  fIsProcess = kTRUE;
  return;
}

//----------------------------------------------------------------------------
TA2DataSource::~TA2DataSource()
{
  //
  //  Flush semaphores and memory
  //  after wait a second to allow other processes to finish
  //
  if( fBuffer ) delete fBuffer;
}

//-----------------------------------------------------------------------------
void TA2DataSource::Initialise( )
{
  fBuffer = new TA2RingBuffer( fInRecLen, fRingSize );
  //  fSourceCond = cond[nsource];
  //  fServerCond = cond[nsource+1];
}

//---------------------------------------------------------------------------
void TA2DataSource::Start()
{
  // Start the data read thread going. This will run in the "background"
  // but will produce to occasional message at root prompt acqu-root>

  if( fSourceThread ){
    printf(" Warning...deleting old RunThread and starting new one\n");
    fSourceThread->Delete();
  }
  fSourceThread = new TThread( this->GetName(),
			   (void(*) (void*))&(A2SourceThread),
			   (void*)this );
  fSourceThread->Run();
  return;
}

//---------------------------------------------------------------------------
void TA2DataSource::Run()
{
  Process();
  Shutdown();
}

ClassImp(TA2DataSource)
