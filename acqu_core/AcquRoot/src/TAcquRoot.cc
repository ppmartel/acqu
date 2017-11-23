//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...13th Mar 2003...workround for erroneous err (lynx)
//--Rev 	JRM Annand....7th Apr 2003...shared-mem size override
//--Rev 	JRM Annand...16th Aug 2003...multi-branch input
//--Rev 	JRM Annand... 1st Oct 2003...analysis = TA2Analysis
//--Rev 	JRM Annand...12th Dec 2003...multi ADC pointers
//--Rev 	JRM Annand... 8th Jan 2004...More flexible analyser choice
//--Rev 	JRM Annand...11th Jan 2004...mods for on-board data server
//--Rev 	JRM Annand...11th Jan 2004...remove scaler block check (temp)
//--Rev 	JRM Annand...31st Mar 2004...foreign ADC renumbering
//--Rev 	JRM Annand...22nd Apr 2004...redo foreign ADC LinkDataServer()
//--Rev 	JRM Annand...26th May 2004...Check event index setup
//--Rev 	JRM Annand...23rd Sep 2004...GetFileName
//--Rev 	JRM Annand...18th Oct 2004...Log file, EndFile, Finish
//--Rev 	JRM Annand...16th Jan 2005...Offline file list, no more TChain
//--Rev 	JRM Annand...18th Feb 2005...fIsFinished = sorting done
//--Rev 	JRM Annand... 3rd Mar 2005...batch log directories
//--Rev 	JRM Annand...11th Mar 2005...periodic flush log-file streams
//--Rev 	JRM Annand...18th May 2005...bug fix scalers OfflineLoop
//--Rev 	JRM Annand...12th Apr 2006...check event ID index sensible
//--Rev 	JRM Annand... 7th Jun 2006...non-clearing scaler handle
//--Rev 	JRM Annand... 6th Jul 2006...re-instate scaler error check
//--Rev 	JRM Annand...15th Sep 2006...scaler error...scan End-of-Event
//--Rev 	JRM Annand...12th Nov 2006...multiple scaler blocks
//--Rev 	JRM Annand...22nd Jan 2007...4v0 update
//--Rev 	JRM Annand...22nd Jan 2007...LinkDataServer flexi format iface
//--Rev 	JRM Annand...17th May 2007...Check offline tree, Mk2 decode
//--Rev 	JRM Annand...29th May 2007...StoreScalersMk2() debug
//--Rev 	JRM Annand...20th Jun 2007...Mk2 error handlers
//--Rev 	JRM Annand...20th Nov 2007...4v2 Local DAQ
//--Rev 	JRM Annand...29th Nov 2007...LinkDataServer() j<nadc, #buffer
//--Rev 	JRM Annand...12th Feb 2008...LinkDataServer() maxscaler
//--Rev 	JRM Annand... 1st May 2008...SetDataServer()...check dataserver
//--Rev 	JRM Annand... 1st Sep 2009...BuildName, delete[]
//--Rev 	K Livingston..7th Feb 2013   Support for handling EPICS buffers
//--Rev  	JRM Annand...22nd Apr 2013...Explicit Mk2 format flag
//--Update 	JRM Annand... 6nd Jun 2013...Optional hardware error printout
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TAcquRoot
//
// Main interface class ACQU <-> Root
// Link to the TA2DataServer and TA2Analysis
// Pass single event of data to TA2Analysis
// 1. Online. Take data buffers directly from TA2DataServer
// 2. Offline. Read from a list of Root files containing
//     the TTree of data. The TTree files are written
//     by a component of the Acqu++ <-> Root analyser where they
//     may be raw or partially processed data.
//

#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2DataServer.h"
#include "TBenchmark.h"
#include "TA2FileSource.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#ifdef WITH_LIBLZMA
#include <lzma.h>
#endif
#include <stdint.h>

using namespace std;

// For Configuration (SetConfig) of AcquRoot
enum { ERootName, ERootBranch, ETreeDir, ERootAnalysisType,
       ERootAnalysisSetup, ERootServerSetup, ERootSetADC, ERootScalerClr,
       ERootEventSize, ERootEventIndex, ERootTreeFile, ERootHbookFile,
       ERootADC, ERootUDS, ERootUDP, ERootMDS, ERootMDP, ERootEVENT,
       ERootBatchDir, ERootSplitScaler, ERootLocalDAQ, ERootLogHardError,
       ERootUseDirectIO, ERootEND};
static const Map_t ConfigMap[] = {
  {"Name:",           ERootName},
  {"Branch:",         ERootBranch},
  {"Directory:",      ETreeDir},
  {"Analysis:",       ERootAnalysisType},
  {"AnalysisSetup:",  ERootAnalysisSetup},
  {"ServerSetup:",    ERootServerSetup},
  {"SetADC:",         ERootSetADC},
  {"ScalerNoClear:",  ERootScalerClr},
  {"EventSize:",      ERootEventSize},
  {"EventIndex:",     ERootEventIndex},
  {"TreeFile:",       ERootTreeFile},
  {"HbookFile:",      ERootHbookFile},
  {"ADC",             ERootADC},
  {"UDS",             ERootUDS},
  {"UDP",             ERootUDP},
  {"MDS",             ERootMDS},
  {"MDP",             ERootMDP},
  {"EVENT",           ERootEVENT},
  {"BatchDir:",       ERootBatchDir},
  {"SplitScaler:",    ERootSplitScaler},
  {"Local-DAQ:",      ERootLocalDAQ},
  {"Log-Hard-Error:", ERootLogHardError},
  {"UseDirectIO:",    ERootUseDirectIO},
  {NULL,              -1}
};

// Recognised types of processing...raw, MC, physics
static const Map_t ARProcessType[] = {
  {"Raw",        EMk1Process},
  {"MC",         EMCProcess},
  {"Physics",    EPhysicsProcess},
  {NULL,          -1}
};


void* A2RunThread( void* arg )
{
  //  Threaded running of analyser
  //  so that other tasks may be performed
  //  Check that arg points to a TAcquRoot object 1st
  //  Analyser must already be loaded into TAcquRoot

  if( !((TObject*)arg)->InheritsFrom("TAcquRoot") ){
    TThread::Printf(" Error...Acqu-Root base class not supplied\n");
    return NULL;;
  }
  TAcquRoot* ar = (TAcquRoot*)arg;
  if( !ar->GetAnalysis() ){
    TThread::Printf(" Error...Acqu-Root analyser class not loaded\n");
    return NULL;
  }
  ar->Run();
  return NULL;
}

//-----------------------------------------------------------------------------
TAcquRoot::TAcquRoot( const char* name, Bool_t batch, Bool_t logfiles )
  :TA2System( name, ConfigMap )
{
  // Default contructor....don't allocate "new" memory here. Root will wipe
  // Zero pointers (not defined) 1st and then counters
  // Set some default values

  fTree = NULL;
  fBranch = NULL;
  fRunThread = NULL;
  fTreeFile = NULL;
  fTreeFileList = NULL;
  fNTreeFiles = 0;
  fTreeName = NULL;
  fBranchName = NULL;
  fTreeDir = NULL;
  fBatchDir = NULL;
  fAnalysis = NULL;
  fAnalysisSetup = NULL;
  fAnalysisType = NULL;
  fDataServer = NULL;
  fDataServerSetup = NULL;
  fLocalDAQSetup = NULL;
  fNEvent = 0;
  fNScalerEvent = 0;
  fTotScalerEvent = 0;
  fCurrEvent = 0;
  fNbranch = 0;
  fRecLen = 0;
  fNDataBuffer = 0;
  fEvent = NULL;
  fEventSize = 0;                    // reset event size
  fEventIndex = 0xffff;              // default undefined
  fHeaderBuff = NULL;
  fSBuff = NULL;
  fADCList = NULL;
  fScalerList = NULL;
  fHardwareList = NULL;
  fHardwareError = NULL;
  fNModule = 0;
  fMaxADC = 0;
  fMaxScaler = 0;
  fADCError = 0;
  fScalerError = 0;
  fHardError = 0;
  fADC = NULL;
  fMulti = NULL;
  fADCdefined = NULL;
  fScaler = fScalerS = NULL;
  fScalerClr = NULL;
  fScalerBuff = NULL;
  fNEventScaler = NULL;
  fScalerHist = NULL;
  fScalerSum = NULL;
  fNScalerBlock = 0;
  fScBlockLength = NULL;
  fScalerBlock = 0;
  fProcessType = EMk1Process;        // default Acqu Mk1 data (raw data)

  fIsOnline = EFalse;                // default offline analysis
  fIsHbookFile = EFalse;             // default NOT HBOOK input file
  fIsSortBusy = EFalse;
  fIsScalerRead = EFalse;            // no scalers read yet
  fIsEpicsRead = EFalse;             // no epics read yet
  fIsFinished = EFalse;              // flag to show sort finished
  fIsBatch = batch;                  // save batch flag
  fIsLogFile = logfiles;
  if( !batch ) 
  {
    if (fIsLogFile) SetLogFile( "AcquRoot.log" );
    else SetLogFile(NULL);
  }
  fIsLocalDAQ = kFALSE;              // default no local DAQ
  fIsMk2Format = kFALSE;             // default not Mk2 format
  fIsPrintError = kFALSE;            // default no error printout
  fUseDirectIO = kFALSE;            // default no error printout

  fNEpics = 0;                       //No of different epics "modules"

}

//-----------------------------------------------------------------------------
TAcquRoot::~TAcquRoot()
{
  // Free up allocated memory
  //
  if( fADC ) delete[] fADC;
  if( fADCdefined ) delete[] fADCdefined;
  if( fScalerBuff ) delete[] fScalerBuff;
  if( fScalerS ) delete[] fScalerS;
  if( fScalerClr ) delete[] fScalerClr;
  if( fScalerSum ) delete[] fScalerSum;
  if( fNEventScaler ) delete fNEventScaler;
  if( fScalerHist ){
    for(Int_t i=0; i<fNScalerEvent; i++ )
      if( fScalerHist[i] ) delete fScalerHist[i];
    delete[] fScalerHist;
  }
  if( fTreeName ) delete[] fTreeName;
  if( fBranchName ){
    for( Int_t i=0; i<fNbranch; i++ ) delete[] fBranchName[i];
    delete[] fBranchName;
  }
  if( fBranch ) delete[] fBranch;
  if( fEvent ){
    for(Int_t i=0; i<fNbranch; i++){
      if( fEvent[i] ) delete[] (Char_t*)fEvent[i];
      delete[] fEvent;
    }
  }
  if( fAnalysisType ) delete[] fAnalysisType;
  if( fAnalysisSetup ) delete[] fAnalysisSetup;
  if( fDataServerSetup ) delete[] fDataServerSetup;
  if( fTreeDir ) delete[] fTreeDir;
  if( fScBlockLength ) delete[] fScBlockLength;
  if( fTreeFileList ){
    for(Int_t i=0; i<fNTreeFiles; i++) delete[] fTreeFileList[i];
    delete[] fTreeFileList;
  }
  if( fHardwareError ) delete[] fHardwareError;
  if( fSBuff ) delete fSBuff;
}

//---------------------------------------------------------------------------
void TAcquRoot::LinkDataServer( )
{
  // Link to internal data server
  // Pull out info on ADC's etc.
  // Locate lists of ADC's, scalers and hardware info (eg. #bits in ADC word)
  // Create arrays showing which ADC's are defined and the number of channels
  // to allocate for histograms etc.
  // Foreign format (headerless) data supported via fHardwareList parameters.
  // Do this ONLY if fOnline flag set

  if( !fIsOnline ) return;
  TA2DataServer* ds = fDataServer;
  fHeaderBuff = ds->GetHeaderBuff();                // header info
  fSBuff = new TA2RingBuffer( ds->GetSortBuff() );  // data transfer buffer
  fRecLen = (ds->GetDataSource(0))->GetInRecLen();  // data record length

  TA2DataFormat* df = ds->GetDataFormat(0);
  if( df->InheritsFrom("TA2Mk2Format") )
    fIsMk2Format = kTRUE;
  //  Wait until the data server has started up
  while( !ds->IsHeaderInit() ) usleep(10);
  // Now pull the ACQU info out of the header
  fMaxScaler = df->GetNScaler();
  fMaxADC = df->GetNADC();

  Int_t bits,i,j;
  Int_t epicsIndex, epicsLength;

  // Check for foreign-format data...ADC's or scalers with indices > defined in
  // ACQU Mk1 data. maxadc accounts for these
  Int_t maxadc = 0;
  Int_t maxscaler = 0;
  Int_t nmod = df->GetNModule();                       // # hardware modules
  fHardwareError = new ReadErrorMk2_t[nmod];          // space for err-blocks
  Int_t iadc, iscaler;
  Int_t nadc,nscaler;
  for( i=0; i<nmod; i++ ){
    if( (df->GetModType(i)==EDAQ_ADC) || (df->GetModType(i)==EForeignADC) ){
      iadc = df->GetModAmin(i);
      nadc = df->GetModNChan(i);
      if( maxadc < iadc ) maxadc = iadc + nadc;
    }
    if( (df->GetModType(i)==EDAQ_Scaler) || (df->GetModType(i)==EForeignScaler) ){
      iscaler = df->GetModAmin(i);
      nscaler = df->GetModNChan(i);
      if( maxscaler < iscaler ) maxscaler = iscaler + nscaler;
      else maxscaler += nscaler;
    }
    if(df->GetModType(i)==EDAQ_Epics){   //if EPICS module make a buffer for it
      epicsIndex=df->GetModBits(i);      //for EPICS this holds the EPICS index 
      epicsLength=df->GetModAmin(i);     //holds the buffer length
      fEpicsBuff[epicsIndex] = new Char_t[epicsLength];
      fNEpics++;
    }
  }
  //init flags to zero for all epics module indices
  for( i=0; i<fNEpics; i++){fEpicsIndex[i]=kFALSE;}

  if( maxadc < fMaxADC ) maxadc = fMaxADC;;
  if( maxscaler < fMaxScaler ) maxscaler = fMaxScaler;;
  fprintf(fLogStream," %d ADC channels and %d Scaler channels in data.\n\n",
	 maxadc, fMaxScaler );

  maxadc++; // In case the ADC list is counting from zero, and needs the last index

  // Create storage areas for ADC and scaler info
  fADCdefined = new Int_t[maxadc];
  fADC = new UShort_t[maxadc];
  SetScaler( maxscaler );
  for( i=0; i<maxadc; i++ ){
    fADC[i] = (UShort_t)ENullADC;            // initialise "empty"
    fADCdefined[i] = EUndefinedADC;        // 0 denotes undefined
  }
  // Now setup ADC's storage
  for( i=0; i<nmod; i++ ){
    if( (df->GetModType(i) == EDAQ_ADC) || (df->GetModType(i)==EForeignADC) ){
      iadc = df->GetModAmin(i);
      nadc = df->GetModNChan(i) + iadc;
      bits = df->GetModBits(i);
      for( j=iadc; j<nadc; j++ ){
	if( fADCdefined[j] != EUndefinedADC )
	  PrintError("","<Conflict of ADC indices>",EErrFatal);
	if( bits >= 16 ) fADCdefined[j] = EPatternADC;
	else fADCdefined[j] = 1 << bits;
      }
    }
    //    if( df->GetModType(i) == EDAQ_Scaler ){
  }
  // Check for any event ID marker (not defined = 0xffff)
  // If its not a sensible number turn it off...set to 0xffff
  // then check if its already defined in the ADC lists....
  // If not define it as a pattern register
  Int_t mid = ds->GetDataFormat(0)->GetEventIndex();
  if( mid > maxadc ){ 
    ds->GetDataFormat(0)->SetEventIndex(0xffff);
    mid = 0xffff;
  }
  if( mid != 0xffff ){
    if( fADCdefined[mid] == EUndefinedADC ){
      fprintf(fLogStream," Defining Event-Marker Index %d in ADC lists\n",
	      mid );
      fADCdefined[mid] = EPatternADC;
    }
  }
  // store maximum numbers...maybe increased by foreign format
  fMaxADC = maxadc;
  fMaxScaler = maxscaler;
  fNModule = nmod;
  if( !fEvent ) SetEventSize( fEventSize );
}

//---------------------------------------------------------------------------
void TAcquRoot::SetADC( void* header )
{
  // Pull out info on defined ADC's and scalers from header buffer
  // Create arrays showing which ADC's are defined and the number of channels
  // to allocate for histograms etc.

  if( fMaxADC ) return;        // do nothing if ADCs previously set manually

  UInt_t* h = (UInt_t*)header;
  fMaxADC = *h++;
  fMaxScaler = *h++;

  fprintf(fLogStream," %d ADC channels and %d Scaler channels in data.\n\n",
	 fMaxADC, fMaxScaler );

  // Create storage areas for ADC and scaler info
  fADCdefined = new Int_t[fMaxADC];
  fADC = new UShort_t[fMaxADC];
  SetScaler( fMaxScaler );
  for( Int_t i=0; i<fMaxADC; i++ ){
    fADC[i] = (UShort_t)ENullADC;            // initialise "empty"
    fADCdefined[i] = *h++;                 // get definition from buffer
  }
}

//---------------------------------------------------------------------------
void TAcquRoot::SetADC( Int_t maxadc, Int_t maxscaler, Int_t bits )
{
  // Create arrays showing which ADC's are defined and the number of channels
  // to allocate for histograms etc. Manual version
  // If fIsOnline flag set this request is ignored (immediate return)
  // as the ADC setup comes automatically from an ACQU header record in
  // the data file

  if( fIsOnline ) return;    // ignore if online
  fMaxADC = maxadc;          // max ADC's any event
  fMaxScaler = maxscaler;    // # of scalers
  fprintf(fLogStream," %d ADC channels and %d Scaler channels defined\n\n",
	 fMaxADC, fMaxScaler );

  fADCdefined = new Int_t[fMaxADC];
  fADC = new UShort_t[fMaxADC];
  SetScaler( maxscaler );
  for( int i=0; i<fMaxADC; i++ ){
    fADC[i] = (UShort_t)ENullADC;            // initialise "empty"
    fADCdefined[i] = 1 << bits;            // if defined, contains # channels
  }
}

//---------------------------------------------------------------------------
void TAcquRoot::SetScaler( Int_t maxscaler )
{
  // Setup arrays for scaler handling
  // Check for any non-clearing scalers

  fScalerBuff = new UInt_t[maxscaler+1];
  fScaler = fScalerBuff + 1;
  fScalerS = new UInt_t[maxscaler];
  for( Int_t i=0; i<maxscaler; i++ ) fScalerS[i] = 0;

  Bool_t* temp = new Bool_t[maxscaler];
  if( !fScalerClr ){
    for( Int_t i=0; i<maxscaler; i++ ) temp[i] = ETrue;
  }
  else {
    for( Int_t i=0; i<maxscaler; i++ ) temp[i] = fScalerClr[i];
    delete[] fScalerClr;
  }
  fScalerClr = temp;
  fScalerSum = new Double_t[maxscaler];
}
  

//-----------------------------------------------------------------------------
void TAcquRoot::SetConfig( char* line, int key )
{
  // Decide what has to the setup on the basis of
  // a line of text read from file, or tty
  // 4v2 auto switch to offline if MC process detected --offline not required

  Char_t name[EMaxName];            		// temp string buffer
  Char_t type[EMaxName];
  Char_t* logfile;
  Int_t* sl;                                    // for split scalers
  Int_t n1,n2,n3;                               // temp integers
  static Int_t nbranch;                         // for branch setup

  switch( key ){
  case ERootName:
    // set name of Root Tree. Offline operation only
    switch(n1 = sscanf(line,"%s%d%s",name,&nbranch,type)){
    default:
      PrintError(line,"<ROOT Tree name not parsed>");
      return;
    case 3:
      if( (fProcessType = Map2Key(type, ARProcessType)) == -1)
	PrintError(line,"<Invalid Data Processing Type>",EErrFatal);
      fprintf(fLogStream,"Data processing type: %s  chosen\n\n", type);
      if( fProcessType == EMCProcess ){
	fprintf(fLogStream, "Setting offline mode for MC processing\n\n");
	fIsOnline = EFalse;
      }
      // no break here
    case 2:
      fNbranch = nbranch;
      break;
    case 1:
      fNbranch = 1;
      break;
    }
    fTreeName = BuildName(name);
    fBranchName = new char*[fNbranch];
    fBranch = new TBranch*[fNbranch];
    fEvent = new void*[fNbranch];
    nbranch = 0;
    fprintf(fLogStream," ROOT Tree name: %s for I/O\n\n", fTreeName );
    break;
  case ERootBranch:
    // set name(s) of Root TBranch(s) which holds sub-event of interest
    // and size of buffer to hold sub-event
    if( nbranch >= fNbranch ){
      PrintError(line, "<Too many TTree branches>");
      return;
    }
    if( (n2 = sscanf(line,"%s%d",name,&n1)) < 1 ){
      PrintError(line, "<Branch-Name-Init>");
      return;
    }
    else if( n2 == 1 ) n1 = 0;
    fBranchName[nbranch] = BuildName(name) ;
    if( n1 ) fEvent[nbranch] = new Char_t[n1];
    else fEvent[nbranch] = NULL;
    fprintf(fLogStream," Branch: %s, size %d added to offline analysis\n\n",
	    fBranchName[nbranch], n1 );
    nbranch++;
    fEventSize += n1;
    break;
  case ERootHbookFile:
    // Flag that the file is an Hbook ntuple file and proceed as ROOT file
    if( fIsOnline ) break;                      // ignore if online
    LoadHbookFile( line );
    break;
  case ERootTreeFile:
    // Add file to list of input Tree files
    if( fIsOnline ) break;                      // ignore if online
    if( sscanf(line,"%s",name) == 1 ) SaveTreeFile( name );
    else PrintError(line,"<Tree-File-Name parse>");
    break;
  case ERootAnalysisType:
    // choose analysis class to use
    // check here if event buffer if setup
    if( sscanf(line,"%s",name) != 1 ){
      PrintError(line, "<Analysis-Class parse>");
      return;
    }
    fAnalysisType = new char[strlen(name)+1];
    strcpy(fAnalysisType, name);
    fprintf(fLogStream,"Analysis Class: %s chosen\n",name);
    // Create default event buffer here if not already done
    if( !fEvent ) SetEventSize( EMaxEventSize );
    break;
  case ERootAnalysisSetup:
    // Specify the name of the setup file for the analysis system
    if( sscanf(line,"%s",name) != 1 ){
      PrintError(line, "<Analysis-Setup-File parse>");
      return;
    }
    fAnalysisSetup = new char[strlen(name)+1];
    strcpy(fAnalysisSetup, name);
    fprintf(fLogStream," Analysis setup from file %s\n\n", fAnalysisSetup );
    break;
  case ERootServerSetup:
    // Specify the name of the setup file for the analysis system
    if( sscanf(line,"%s",name) != 1 ){
      PrintError(line, "<Server-Setup-File parse>");
      return;
    }
    fDataServerSetup = new char[strlen(name)+1];
    strcpy(fDataServerSetup, name);
    fprintf(fLogStream," Data Server setup from file %s\n\n",
	    fDataServerSetup );
    break;
  case ERootSetADC:
    // manual setup of ADCs/scalers for Raw analysis
    if( sscanf(line,"%d%d%d",&n1,&n2,&n3) == 3 ) SetADC(n1,n2,n3);
    else PrintError(line,"<Manual-ADC-Setup>",EErrFatal);
    break;
  case ERootScalerClr:
    // Mark scaler indices as non-clearing
    if( sscanf(line,"%d%d",&n1,&n2) == 2 ){
      if( !fScalerClr ){
	fScalerClr = new Bool_t[EMaxScaler];
	for( n3=0; n3<EMaxScaler; n3++ ) fScalerClr[n3] = ETrue;
      }
      for( n3=n1; n3<=n2; n3++ ){ 
	if( n3 < EMaxScaler )
	  fScalerClr[n3] = EFalse;
      }
    }
    else PrintError(line,"<Mark scalers non-clearing>",EErrFatal);
    break;
    break;
  case ERootEventSize:
    // manual setup of size of buffer to hold 1 event
    if( sscanf(line,"%d",&n1) == 1) SetEventSize(n1);
    else PrintError(line,"<Event-Size-Setup>",EErrFatal);
    break;
  case ERootEventIndex:
    // Set event-marked index...only allowed for offline analyses
    if( fIsOnline ) return;
    if( sscanf(line,"%d", &fEventIndex) == 1 )
      PrintError(line,"<Manual-ADC-Setup>");
    break;
  case ETreeDir:
    // Specify the name of directory where ROOT output is stored
    if( sscanf(line,"%s",name) != 1 ){
      PrintError(line, "<Output ROOT tree directory parse>");
      return;
    }
    if( fTreeDir == NULL ){
      fTreeDir = BuildName(name);
      fprintf(fLogStream," ROOT output stored in directory: %s\n\n", fTreeDir );
    }
    break;
  case ERootEVENT:
    // no action & no error message
    break;
  case ERootBatchDir:
    // directory to store log files when in batch mode
    if( !fIsBatch ) break;              // check its in batch mode
    if( sscanf(line,"%s",name) != 1 ){
      PrintError(line, "<Output batch-log-file directory parse>");
      return;
    }
    if( fBatchDir == NULL ){
      fBatchDir = BuildName( name );;
      logfile = BuildName( fBatchDir, "AcquRoot.log" );
      if (fIsLogFile) SetLogFile( logfile );
      else SetLogFile(NULL);
      delete[] logfile;
      if (fIsLogFile) fprintf(fLogStream," Batch-mode log files stored in directory: %s\n\n",
	                      fBatchDir );
    }
    break;
  case ERootSplitScaler:
    // fragmented scaler reads (from multiple data sources)
    sl = fScBlockLength = new Int_t[EMaxScalerBlock];
    if( sscanf(line,"%d%d%d%d%d%d%d%d%d", &fNScalerBlock,
	       sl,sl+1,sl+2,sl+3,sl+4,sl+5,sl+6,sl+7) != fNScalerBlock+1 ){
      PrintError(line, "<Split scaler sources input>");
      fNScalerBlock = 0;
      return;
    }
    break;
  case ERootLocalDAQ:
    // flag local DAQ thread and read and store name of config file
    // fatal error if file name not given
    if( sscanf(line, "%s", name) != 1 )
      PrintError(line, "<No DAQ config file given>", EErrFatal);
    fLocalDAQSetup = BuildName(name);
    fIsLocalDAQ = kTRUE;
    break;
  case ERootLogHardError:
    // flag to turn on print out of hardware errors
    fIsPrintError = kTRUE;
    break;
  case ERootUseDirectIO:
    // flag to turn on print out of hardware errors
    fUseDirectIO = kTRUE;
    break;
  default:
    fprintf(fLogStream," Ignored unrecognised Acqu-Root parameter\n%s\n",
	    line);
    break;
  }
}

//-----------------------------------------------------------------------------
void TAcquRoot::LoadHbookFile( char* line )
{
  // Read in HBOOK file (containing full directory path) and ntuple ID
  // Make initial check about online/offline status

  if( fIsOnline ){
    PrintMessage(" Error..attempt to load offline input files for online analysis\n");
    return;
  }
  char hbookfile[256];               // name of ntuple file
  Int_t ntupleID;                    // ntuple ID 
  if( sscanf( line, "%s%d", hbookfile, &ntupleID ) != 2 ){
    PrintError( line, "<Hbook-File parse>" );
    return;
  }
  //  if( fHbookFile ) delete fHbookFile;       // flush any old instance
  //  fHbookFile = new THbookFile( hbookfile );
  //  fHbookTree = (THbookTree*)( fHbookFile->Get( ntupleID ) );
  //  fprintf(fLogStream," Ntuple %d found in HBOOK file %s\n", ntupleID, hbookfile );
  
  //  fHbookTree->Print();                   	// Show TTree statistics
  //  fNEvent = (int)(fHbookTree->GetEntries());    // no. of events in chain
  //  fprintf(fLogStream," Total events in HBOOK file = %d\n\n", fNEvent);
  return;
}

//-----------------------------------------------------------------------------
void TAcquRoot::SaveTreeFile( char* rootfile )
{
  // Offline operation ONLY
  // Save the name of a Tree file in a name list which will be processed
  // by OfflineLoop. Superceeds LoadTreeFile() which made a TChain

  if( fIsOnline ){
    PrintError("","<Saving Offline input files during Online analysis>");
    return;
  }
  if( !fTreeFileList ) fTreeFileList = new Char_t*[EMaxTreeFiles];
  fTreeFileList[fNTreeFiles] = BuildName(rootfile);
  fNTreeFiles++;
  return;
}

//---------------------------------------------------------------------------
void TAcquRoot::Run( )
{
  // Run Acqu-Root Online or Offline
  // Offline handles single or multi branch Trees

  gBenchmark = new TBenchmark();
  gBenchmark->Start("Sort_Thread");

  // ON line
  if( fIsOnline ){
    LinkDataServer( );                             // Header, if in data
    fAnalysis->FileConfig( fAnalysisSetup );    // config from file
    if( fDataServer ) {
      if( fUseDirectIO ) {
        DataLoopDirectIO();
      }
      else {
        DataLoop();
      }
    }
  }
  // Single-branch (unfragmented) or Multi-branch (fragmented) TTree decode
  else{
    fAnalysis->FileConfig( fAnalysisSetup );    // config from file
    OfflineLoop();
  }
  gBenchmark->Show("Sort_Thread");
  fIsFinished = ETrue;
}

//---------------------------------------------------------------------------
void TAcquRoot::Start()
{
  // Start the analysis thread going. This will run in the "background"
  // but will produce to occasional message at root prompt acqu-root>

  if( fRunThread ){
    printf(" Warning...deleting old RunThread and starting new one\n");
    fRunThread->Delete();
  }
  fRunThread = new TThread( "A2RunThread",
			   (void(*) (void*))&(A2RunThread),
			   (void*)this );
  fRunThread->Run();
  return;
}

//---------------------------------------------------------------------------
Bool_t TAcquRoot::LinkTree( Int_t n )
{
  // This one is for Linking to a pre-recorded  ROOT TTree files.
  // For standard raw data the Tree may contain in 1-3 branches.
  // Monte Carlo generated data may be fragmented in several branches,
  // as with GEANT-3.21 ntuple output converted by h2root
  // Requires arrays of fBranchName & fEvent to be preinitialised.
  //
  // A standard AcquRoot created by TA2Analysis contains the following...
  // Usually a tree with 3 branches is created for output
  // branch 0 has the event-by-event ADC information
  //   formated "nhit/i:ndaqevent/i:ntotevent/i:hit[nhit]/i:"
  //   hit[nhit] has index/value pairs
  // branch 1 has scaler info...every scaler read stored as separate entry
  //   formated "scevent/i:scaler[%d]/i:"
  // branch 2 has a single entry giving type/range info on ADC's
  //   formated "maxadc/I:maxscaler/I:adcdev[%d]/I:"

  fTreeFile = new TFile(fTreeFileList[n]);       // open the tree file
  fTree = (TTree*)fTreeFile->Get(fTreeName);     // extract tree ptr by name
  if( !fTree ){
    PrintError(fTreeFileList[n],"<Tree not found in file>");
    return kFALSE;
  }

  // Associate branches of the tree with AcquRoot memory buffers
  for( int j=0; j<fNbranch; j++){
    fTree->SetBranchAddress(fBranchName[j], fEvent[j]);
    fBranch[j] = fTree->GetBranch(fBranchName[j]);
  }
  return kTRUE;
}

//---------------------------------------------------------------------------
Bool_t TAcquRoot::LinkADC()
{
  // If the raw-data Tree has been created with a branch containing basic spec.
  // of the experimental ADC's then read that branch into memory
  // and use the info to setup ADC and Scaler arrays.
  // If the Tree doesn't contain this info the SetADC has to be performed
  // "manually"
  // This procedure is called before TA2Analysis is configured.

  // This is valid for raw data processing only
  if( fProcessType != EMk1Process ) return kFALSE;
  // Read the Tree from the 1st ROOT file in the offline list
  if( !LinkTree(0) ) return kFALSE;

  // Look for any stored header buffer
  // If found use info to setup ADC arrays etc.
  if( fNbranch > EARHeaderBr ){
    if( fBranch[EARHeaderBr]->GetEntries() ){
      fBranch[EARHeaderBr]->GetEntry(0);
      SetADC(fEvent[EARHeaderBr]);
    }
  }
  // Delete the Tree just now to avoid any coupling of created histograms
  // or cuts with the Tree file.
  delete fTreeFile;
  fTreeFile = NULL;
  return kTRUE;
}

//---------------------------------------------------------------------------
Bool_t TAcquRoot::UpdateTree( Int_t n )
{
  // Setup processing of the n-th input tree file
  // Open the Tree file and link to AcquRoot memory buffers
  // If scaler processing enabled update arrays for holding scaler reads

  if( n >= fNTreeFiles ){
    PrintError("","<Update Tree, total number of files exceeded>");
    return kFALSE;
  }
  // delete the last tree file, if not already done
  // and link to a new one
  if( fTreeFile ) delete fTreeFile;
  if( !LinkTree(n) ) return kFALSE;

  // Print Tree statistics
  // printf(" Statistics for input Tree file %s:\n\n", fTreeFileList[n]);
  // fTree->Print();
  // printf("\n\n");

  // Number of ADC read events
  fNEvent = (int)(fBranch[EARHitBr]->GetEntries());
  // Scaler Events and storage
  // NOT if MC data
  if( (fNbranch > EARScalerBr) && (fProcessType == EMk1Process) ){
    // If not the 1st file delete existing scaler-read arrays
    if( fNEventScaler ) delete fNEventScaler;
    if( fScalerHist ){
      for( Int_t i=0; i<fNScalerEvent; i++ )
	delete fScalerHist[i];
      delete fScalerHist;
    }
    // Create new scaler-read arrays based on the number
    // of scaler reads in newly opened ROOT file
    fNScalerEvent = (Int_t)(fBranch[EARScalerBr]->GetEntries());
    fTotScalerEvent += fNScalerEvent;
    fNEventScaler = new UInt_t[fNScalerEvent];
    fScalerHist = new UInt_t*[fNScalerEvent];
    for( Int_t i=0; i<fNScalerEvent; i++ )
      fScalerHist[i] = new UInt_t[fMaxScaler];
  }
  else fNScalerEvent = 0;

  fprintf(fLogStream," Total events in ROOT file %s = %d\n\
  Scaler write events = %d\n\n", fTreeFileList[n], fNEvent, fNScalerEvent);

  return kTRUE;

}

//---------------------------------------------------------------------------
void TAcquRoot::OfflineLoop()
{
  // Handle data reading from ROOT Tree files.
  // Loop over desired files....link to the Tree in each file.
  // Read all scaler info 1st, and process ADC events.
  // Saving of filtered data to a further Tree file is supported
  // NB TA2DataServer is NOT used for Offline operation

  for( Int_t j=0; j<fNTreeFiles; j++ ){
    // Link to the next input Tree file
    if( !UpdateTree(j) ) continue;
    // If filtered data saved then open the output Tree file
    fAnalysis->ChangeTreeFile(fTreeFileList[j]);
    // Retrieve all scaler buffers
    for( Int_t i=0; i<fNScalerEvent; i++ ){
      fBranch[EARScalerBr]->GetEntry(i);  // single scaler read
      fAnalysis->SaveEvent(EARScalerBr);  // store next file (if enabled)
      StoreOfflineScalers(i);             // store AcquRoot arrays
    }

    // And then the main body of events
    // Note that end-of-file stuff isn't processed in this scheme
    UInt_t* currEvent = (UInt_t*)fEvent[EARHitBr] + 1;
    UInt_t scalerEvent = 0;
    for( Int_t i=0; i<fNEvent; i++ ){
      fIsScalerRead = EFalse;
      // ADC/Scaler processing only on raw data
      if( fProcessType == EMk1Process ){
	fBranch[EARHitBr]->GetEntry(i);
	if( *currEvent == fNEventScaler[scalerEvent] ){
	  UInt_t* sc = fScaler;
	  UInt_t* sch = fScalerHist[scalerEvent];
	  fIsScalerRead = ETrue;
	  for( Int_t k=0; k<fMaxScaler; k++ ) *sc++ = *sch++;
	  scalerEvent++;
	}
      }
      else fTree->GetEntry(i);
      fAnalysis->Process();
    }
    fAnalysis->EndFile();
    fflush(NULL);          // flush any log output to disk
  }
  fAnalysis->Finish();     // call any end-of-run procedures
  fAnalysis->CloseEvent(); // check if save enabled and close ROOT file if so
}  


//---------------------------------------------------------------------------
void TAcquRoot::Clear( )
{
  fprintf(fLogStream," Clear not yet implemented\n");
}


//---------------------------------------------------------------------------
void TAcquRoot::Reset( )
{
  fprintf(fLogStream," Reset not yet implemented\n");
}

//---------------------------------------------------------------------------
void TAcquRoot::DataLoop( )
{
  //  "Online" operation...process buffers of data provided by DataServer
  //  in "native" Acqu format
  //  Loop until error or "kill" (sent by DataServer) encountered
  //  When DataServer is ready check the header and call the event
  //  decode procedure
  //  Pull out file-name pointer & buffer-header pointer from data base

  char* f = GetFileName();
  fNEvent = 0;
  UInt_t* h;
  fAnalysis->InitSaveTree( f );  // Init data store by analyser (if enabled)

  // fragmented scaler read...check the fragments add to the total
  if( fNScalerBlock ){
    Int_t j = 0;                 // ensure init to 0, JRMA 15/12/06
    for( Int_t i=0; i<fNScalerBlock; i++ ) j += fScBlockLength[i];
    if( j != fMaxScaler )
      PrintError("","<Multiple scaler blocks incompatible with total>", 
		 EErrFatal);
  }
  do{
    fSBuff->WaitFull();
    h = (UInt_t*)( fSBuff->GetStore() );
    fIsSortBusy = ETrue;
    fflush(NULL);               // flush any log output to disk
    switch( *h ){
    default:		        // Unrecognised buffer header..stop analysis
      fprintf(fLogStream,
	      " Online Acqu-Root, unrecognised buffer header: %x \n", *h);
      PrintError("","<Unrecoverable-Data-Buffer>", EErrFatal);
      return;
    case EDataBuff:		// Standard data buffer
      Mk1EventLoop( h );
      break;
    case EMk2DataBuff:		// Mk2 data buffer
      Mk2EventLoop( h );
      break;
    case EEndBuff:		// Standard trailer (EOF) buffer
      fprintf(fLogStream," End file %s, %d events sorted, last event #%d\n",
	     f,fNEvent,fCurrEvent);
      fAnalysis->EndFile();     // carry of end-of-file tasks
      fAnalysis->ChangeTreeFile(f);
      break;
    case EKillBuff:		// Online DAQ shutdown
      fAnalysis->Finish();      // call any end-of-run procedures
      fAnalysis->CloseEvent();  // if data saved in ROOT format, close file
      PrintMessage(" Online DAQ shut down...exiting event analysis loop\n");
      return;
    }
    fIsSortBusy = EFalse;
    fSBuff->CNext();
    fNDataBuffer++;
  }while( *h != EKillBuff );
  // 3v13 end-of-run sequence gets out of synch
  // should be caught in above switch
  fAnalysis->Finish();        // call any end-of-run procedures
  fAnalysis->CloseEvent();    // if data saved in ROOT format, close file
  PrintMessage(" Online DAQ shut down...exiting event analysis loop\n");
}


//---------------------------------------------------------------------------
void TAcquRoot::DataLoopDirectIO( )
{
  // this routine is a simplified version of the
  // DataLoop method above, it inherits the checks and algorithm
  // to handle data blocks, but reading data blocks is directly done 
  // from the files
  
  char* f = GetFileName();
  fNEvent = 0;
  
  fAnalysis->InitSaveTree( f );  // Init data store by analyser (if enabled)
  
  // fragmented scaler read...check the fragments add to the total
  if( fNScalerBlock ){
    Int_t j = 0;                 // ensure init to 0, JRMA 15/12/06
    for( Int_t i=0; i<fNScalerBlock; i++ ) j += fScBlockLength[i];
    if( j != fMaxScaler )
      PrintError("","<Multiple scaler blocks incompatible with total>", 
                 EErrFatal);
  }
  
  for(Int_t i=0;i<fDataServer->GetNDataSource();i++) {
    TA2DataSource* ds = fDataServer->GetDataSource(i);
    if(!ds->InheritsFrom("TA2FileSource")) {
      PrintError("","Ignoring non-file data source", 
                 EErrNonFatal);
      continue;
    }
    TA2FileSource* fs = (TA2FileSource*)ds;
    for(Int_t j=0;j<fs->GetNfile();j++) {
      UInt_t start, stop;
      string filename;
      fs->GetFileInfo(i, filename, start, stop);
      cout << "DIRECT IO MODE! EXPERIMENTAL! YOU HAVE BEEN WARNED! " 
           << endl
           << " Filename: " << filename 
           << endl;
      if(start != 0) {
        PrintError("","DirectIO mode does not support skipping data blocks.", 
                   EErrFatal);
      }
      if(filename.size()<4) {
        PrintError("","Unkown file extension, filename too short. Skipping.", 
                   EErrFatal);
      }
      Bool_t success = kFALSE;
      // check filename ending
      if(filename.compare(filename.size()-4, 4, ".dat") == 0) {
        success = DataLoopDirectIOWorker(filename, stop);
      }
#ifdef WITH_LIBLZMA
      else if(filename.compare(filename.size()-7, 7, ".dat.xz") == 0) {
        success = DataLoopDirectIOWorkerXZ(filename, stop);
      }
#endif
      else {
        PrintError("","Unkown file extension, only .dat and .dat.xz (if liblzma found) supported.", 
                   EErrFatal);
      }
      
      if(!success) {
        PrintError("","Something went wrong processing the file...", 
                   EErrFatal);        
      }
      
      fprintf(fLogStream," End file %s, %d events sorted, last event #%d\n",
	     f,fNEvent,fCurrEvent);
      fAnalysis->EndFile();     // carry of end-of-file tasks
      fAnalysis->ChangeTreeFile(f);
    }
  }
  
  fAnalysis->Finish();      // call any end-of-run procedures
  fAnalysis->CloseEvent();  // if data saved in ROOT format, close file
  PrintMessage(" Online DAQ shut down...exiting event analysis loop\n");
  
}

#ifdef WITH_LIBLZMA
Bool_t TAcquRoot::DataLoopDirectIOWorkerXZ(const string& filename, const UInt_t stop) {
  ifstream file(filename.c_str(), ios::in|ios::binary);
  if(!file.is_open()) {
    return kFALSE;
  }
  
  lzma_stream strm_ = LZMA_STREAM_INIT;
  lzma_stream* strm = &strm_;
  lzma_ret ret = lzma_stream_decoder(strm, UINT64_MAX, LZMA_CONCATENATED);
  if(ret != LZMA_OK) {
    PrintError("","Cannot init LZMA decoder.", 
               EErrNonFatal); 
    return kFALSE;
  }
  lzma_action action = LZMA_RUN;
  
  uint8_t inbuf[fRecLen];
  uint8_t outbuf[fRecLen];
  
  strm->next_in = NULL;
  strm->avail_in = 0;
  strm->next_out = outbuf;
  strm->avail_out = sizeof(outbuf);
  
  fNDataBuffer = 0;  
  
  while (true) {
    if (strm->avail_in == 0 && !file.eof()) {
      strm->next_in = inbuf;
      
      if(!file.read((char*)inbuf, fRecLen)) {
        // since we're reading compressed data 
	// with some arbitrary fRecLen, it might be
	// that we reached the end of file
	if(!file.eof()) {
	  PrintError("","File read error.", 
		     EErrNonFatal); 
	  return false;
	}
      }
      strm->avail_in = file.gcount();
    
      if (file.eof())
        action = LZMA_FINISH;
    }
    
    lzma_ret ret = lzma_code(strm, action);
    
    // full decompressed block available
    if (strm->avail_out == 0) {
      UInt_t* h = (UInt_t*)outbuf;
      //cout << hex << setw(8) << *h << dec << endl;
      
      if(fNDataBuffer>0) {
        fIsSortBusy = ETrue;
        //fflush(NULL);               // flush any log output to disk
        switch( *h ){
        default:		        // Unrecognised buffer header..stop analysis
          fprintf(fLogStream,
                  " Online Acqu-Root, unrecognised buffer header: %x \n", *h);
          PrintError("","<Unrecoverable-Data-Buffer>", EErrFatal);
          return kFALSE;
        case EDataBuff:		// Standard data buffer
          Mk1EventLoop( h );
          break;
        case EMk2DataBuff:		// Mk2 data buffer
          Mk2EventLoop( h );
          break;
        case EEndBuff:		// Standard trailer (EOF) buffer
          return kTRUE;
          break;
        }
        fIsSortBusy = EFalse;
      }
      
      fNDataBuffer++;
      if((UInt_t)fNDataBuffer==stop)
        return kTRUE;
      
      // decompress the next one, if available      
      if(ret != LZMA_STREAM_END) {
        strm->next_out = outbuf;
        strm->avail_out = sizeof(outbuf);
      }
    }
    
    if (ret != LZMA_OK) {
      if (ret == LZMA_STREAM_END) {
        // check if there's nothing left to analyze
        if(strm->avail_out==0)
          return true;
        PrintError("","Premature end of file. But rest was ok.", 
                   EErrNonFatal);  
        return kFALSE;
      }
      
      const char *msg;
      switch (ret) {
      case LZMA_MEM_ERROR:
        msg = "Memory allocation failed";
        break;
        
      case LZMA_FORMAT_ERROR:
        // .xz magic bytes weren't found.
        msg = "The input is not in the .xz format";
        break;
        
      case LZMA_OPTIONS_ERROR:
        msg = "Unsupported compression options";
        break;
        
      case LZMA_DATA_ERROR:
        msg = "Compressed file is corrupt";
        break;
        
      case LZMA_BUF_ERROR:
        msg = "Compressed file is truncated or "
            "otherwise corrupt";
        break;
        
      default:
        // This is most likely LZMA_PROG_ERROR.
        msg = "Unknown error, possibly a bug";
        break;
      }
      
      PrintError("", msg, 
                 EErrNonFatal);
      return false;
    }
  }
  
  return kFALSE;
}
#endif

Bool_t TAcquRoot::DataLoopDirectIOWorker(const string& filename, 
                                         const UInt_t stop) {
  ifstream file(filename.c_str(), ios::in|ios::binary);
  if(!file.is_open()) {
    PrintError("","Cannot open file.", 
               EErrNonFatal);  
    return kFALSE;
  }
  
  // seek to first data buffer, skip the header
  if(!file.seekg(fRecLen)) {
    PrintError("","File does not contain header.", 
               EErrNonFatal);  
    return kFALSE;
  }
  
  fNDataBuffer = 0;
  while(!file.eof()) {
    char buffer[fRecLen];
    if(!file.read(buffer, fRecLen)) {
      PrintError("","File read error.", 
                 EErrNonFatal); 
      return false;
    }
    if(file.gcount() != fRecLen) {
      PrintError("","Premature end of file. But rest was ok.", 
                 EErrNonFatal);  
      return kFALSE;
    }
    UInt_t* h = (UInt_t*)buffer;
    //cout << hex << setw(8) << *h << dec << endl;
      
    fIsSortBusy = ETrue;
    //fflush(NULL);               // flush any log output to disk
    switch( *h ){
    default:		        // Unrecognised buffer header..stop analysis
      fprintf(fLogStream,
	      " Online Acqu-Root, unrecognised buffer header: %x \n", *h);
      PrintError("","<Unrecoverable-Data-Buffer>", EErrFatal);
      return kFALSE;
    case EDataBuff:		// Standard data buffer
      Mk1EventLoop( h );
      break;
    case EMk2DataBuff:		// Mk2 data buffer
      Mk2EventLoop( h );
      break;
    case EEndBuff:		// Standard trailer (EOF) buffer
      return kTRUE;
      break;
    }
    fIsSortBusy = EFalse;
    
    fNDataBuffer++;
    if((UInt_t)fNDataBuffer==stop)
      return kTRUE;
  }
  
  // we expect to find a EEndBuff
  // but not finding it is a non-fatal error
  return kFALSE;
}


//---------------------------------------------------------------------------
void TAcquRoot::Mk1EventLoop( UInt_t* startdata )
{
  // Decode one Acqu++ Mk1-format (the original acqu format)
  // buffer of data event-by-event. Preincrement the buffer pointer
  // past the buffer header (ID word)
  // 13/3/03 Temp workround for garbled error blocks recorded in March 2003
  // 22/10/04 Add event number to event buffer (for reduced data output)

  UInt_t* datum = startdata;
  UInt_t* end = (UInt_t*)( (char*)datum + fRecLen );
  UInt_t* hit;	                        // recepticle buffer for event
  UInt_t nhit;                          // # adc's in event
  datum++;				// start of real data in buffer
  // loop until end of buffer
  do{
    if( *datum == EBufferEnd ) return;	// end-of-buffer marker
    else if( *datum == EDataBuff ) return;	// somehow reached next buffer
    fCurrEvent = *datum++;              // event number recorded in data
    nhit = 0;
    fHardError = 0;
    hit = (UInt_t*)(fEvent[EARHitBr]) + 1;     // recepticle buffer for event
    *hit++ = fCurrEvent;                // save DAQ event# (for saved data)
    hit++;                              // space for TA2Analysis event # 
    // transfer event's worth of data into fEvent buffer
    while( *datum != EEndEvent && *datum != EDataBuff ){
      switch( *(UInt_t*)datum ){
      case EScalerBuffer:			     // scaler array
	datum = StoreScalers( datum );
	fIsScalerRead = ETrue;                       // flag scaler read;
	break;
      case EReadError:				     // hardware error block
	datum = Mk1ErrorCheck( datum );
	break;
      case 0xffffefef:                               // garbled error block
	datum = Mk1ErrorCheck( datum );
	break;
      default:					     // adc...id, then value
	*hit++ = *datum++;
	nhit++;
	break;
      }
    }

    // event transferred...now analyse it
    *(UInt_t*)(fEvent[EARHitBr]) = nhit;             // 1st word is #hits
    fNEvent++;					     // event counter
    fAnalysis->Process();			     // analyse the event
    fIsScalerRead = EFalse;                          // reset scaler flag
    if( *datum == EDataBuff ) return;	// somehow reached next buffer
    datum++;                                         // start next event
  }while( datum < end );                             // don't overrun buffer
}

//---------------------------------------------------------------------------
void TAcquRoot::Mk2EventLoop( UInt_t* startdata )
{
  // Decode one Acqu++ Mk2-format
  // buffer of data event-by-event. Preincrement the buffer pointer
  // past the buffer header (ID word)

  UInt_t* datum = startdata;
  UInt_t* endEvent;
  UInt_t* end = (UInt_t*)( (char*)datum + fRecLen );
  UInt_t* hit;	                        // recepticle buffer for event
  UInt_t nhit;                          // # adc's in event
  UInt_t evLen;                         // length of event
  Int_t nScBlock = 0;                   // scaler block number
  datum++;				// start of real data in buffer
  // loop until end of buffer
  do{
    if( *datum == EBufferEnd ) return;	// end-of-buffer marker
    fCurrEvent = *datum++;              // event number recorded in data
    evLen = *datum;                     // event length
    endEvent = datum + evLen/sizeof(UInt_t);
    if( *endEvent != EEndEvent ) return;
    datum++;
    nhit = 0;
    fHardError = 0;
    hit = (UInt_t*)(fEvent[EARHitBr]) + 1;     // recepticle buffer for event
    *hit++ = fCurrEvent;                // save DAQ event# (for saved data)
    hit++;                              // space for TA2Analysis event # 
    // transfer event's worth of data into fEvent buffer
    while( *datum != EEndEvent ){
      switch( *(UInt_t*)datum ){
      case EEPICSBuffer:                             // EPICS buffer
	datum = StoreEpics( datum );
	fIsEpicsRead=kTRUE;
	break;
      case EScalerBuffer:			     // scaler array
	if( !nScBlock ){
	  for( Int_t i=0; i<fMaxScaler; i++ ) fScaler[i] = 0;
	}
	datum = StoreScalersMk2( datum );
	nScBlock++;
	fIsScalerRead = ETrue;                       // flag scaler read;
	break;
      case EReadError:				     // hardware error block
	datum = Mk2ErrorCheck ( datum );
	break;
      default:					     // adc...id, then value
	*hit++ = *datum++;
	nhit++;
	break;
      }
    }

    // event transferred...now analyse it
    *(UInt_t*)(fEvent[EARHitBr]) = nhit;             // 1st word is #hits
    fNEvent++;					     // event counter
    fAnalysis->Process();			     // analyse the event
    fIsScalerRead = EFalse;                          // reset scaler flag
    fIsEpicsRead = EFalse;                           // reset scaler flag
    for( int i=0;i<fNEpics;i++){fEpicsIndex[i]=EFalse;} // and epics index flags
    datum++;                                         // start next event
  }while( datum < end );                             // don't overrun buffer
}

//---------------------------------------------------------------------------
UInt_t TAcquRoot::SetEventSize( UInt_t size )
{
  // Create event buffer
  // Only works where data stream is NOT fragmented
  //
  if( fEvent ){
    PrintError("","<Resize-Event-Buffer>");
    return 0;
  };
  fEventSize = size;
  fEvent = new void*[1];             // buffer not fragmented
  fEvent[EARHitBr] = new char[size];
  return size;
}

//---------------------------------------------------------------------------
void TAcquRoot::SetAnalysis( TA2Analysis* analysis )
{
  // load externally the data-analysis class

  if( !analysis->InheritsFrom("TA2Analysis") ){
    PrintError("","<Invalid Acqu-Root analysis class>",EErrFatal);
    return;
  }
  fprintf(fLogStream," %s analysis loaded into %s Acqu-to-Root interface\n",
	  analysis->ClassName(), this->ClassName() );    
  fAnalysis = analysis;
  return;
}

//---------------------------------------------------------------------------
void TAcquRoot::SetDataServer( TA2DataServer* dataserver )
{
  // load externally the data-dataserver class
  // 1st check if there is a data server
  if( !dataserver ) return;

  if( !dataserver->InheritsFrom("TA2DataServer") ){
    PrintError("","<Invalid Acqu-Root data-server class>",EErrFatal);
    return;
  }
  fprintf(fLogStream," %s data server loaded into %s Acqu-to-Root interface\n",
	  dataserver->ClassName(), this->ClassName() );    
  fDataServer = dataserver;
  return;
}

ClassImp(TAcquRoot)
