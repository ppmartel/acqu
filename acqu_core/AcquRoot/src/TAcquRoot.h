//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...13th Mar 2003...workround for erroneous err (lynx)
//--Rev 	JRM Annand....7th Apr 2003...shared-mem size override
//--Rev 	JRM Annand...16th Aug 2003...multi-branch input
//--Rev  	JRM Annand... 1st Oct 2003...analysis = TA2Analysis
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

#ifndef __TAcquRoot_h__
#define __TAcquRoot_h__

#include "TFile.h"                     // Root file for TTree's
#include "TTree.h"                     // The Tree class
#include "TThread.h"                   // for threaded event loop
#include "TA2System.h"                 // base Acqu-Root class
#include "TA2DataFormat.h"             // Structs defining ACQU data formats
#include "TA2Mk2Format.h"              // Structs defining Mk2 data formats
#include "MultiADC_t.h"                // multi-hit ADC handler
#include "TA2RingBuffer.h"             // data buffer class

#include <string>
#include "time.h"

// Types raw-data tree branch
enum { EARHitBr, EARScalerBr, EARHeaderBr };
// Types of data processing
enum { EMk1Process, EMCProcess, EPhysicsProcess };
// Max number offline files to analyse
enum { EMaxTreeFiles = 1024 };
enum { EMaxScaler = 65536 };
enum { EMaxScalerBlock = 8 };

// Max no of epics modules to handle (noramlly 0,1 or 2)
enum {EMaxEpicsModules = 100 }; 

void* A2RunThread( void* );

class TA2Analysis;                     // data analyser
class TA2DataServer;                   // data provider

class TAcquRoot : public TA2System {
private:
  TTree* fTree;		        // Tree for input data
  TBranch** fBranch;            // separate branches of tree
  TThread* fRunThread;          // To run analyser as separate thread
  TFile* fTreeFile;             // ROOT file holding tree
  Char_t** fTreeFileList;       // list of names of tree files
  Int_t fNTreeFiles;            // Number Tree files to analyse (offline)
  Char_t* fTreeName;		// string- name of TTree
  Char_t** fBranchName;         // branch(s) of TTree to analyse
  Char_t* fTreeDir;             // Output root data directory
  Char_t* fBatchDir;            // batch mode log-file directory
  TA2Analysis* fAnalysis;	// data sorting basis class
  Char_t* fAnalysisSetup;       // setup file for analysis class
  Char_t* fAnalysisType;        // type of analysis to run
  TA2DataServer* fDataServer;   // data input class
  Char_t* fDataServerSetup;     // setup file for data server class
  Char_t* fLocalDAQSetup;       // setup file for TDAQexperiment
  Int_t fNEvent;		// number of events in Tree (offline)
  Int_t fNScalerEvent;          // number scaler events in Tree (offline)
  Int_t fTotScalerEvent;        // total number offline scaler events
  Int_t fCurrEvent;		// current event number
  Int_t fNbranch;               // # branches to analyse
  Int_t fRecLen;                // expected record length
  Int_t fNDataBuffer;           // # data buffers analysed
  void** fEvent;		// -> analysed event buffer(s)
  UInt_t fEventSize;            // length of event buffer
  UInt_t fEventIndex;           // ADC index of event marker
  void* fHeaderBuff;            // Header information (ACQU format)
  TA2RingBuffer* fSBuff;        // Data buffer for analysis
  ADCInfo_t* fADCList;          // list of defined ADC's
  ADCInfo_t* fScalerList;       // list of defined scalers
  void* fHardwareList;          // list of hardware modules
  ReadErrorMk2_t* fHardwareError;// list of hardware error (per event)
  Int_t fNModule;               // no. hardware modules
  Int_t fMaxADC;                // no. ADC's defined in data base
  Int_t fMaxScaler;             // no. scalers defined in data base
  Int_t fADCError;              // ADC Error counter
  Int_t fScalerError;           // Scaler Error counter
  Int_t fHardError;             // Hardware Error counter
  UShort_t* fADC;               // ADC array for subsequent analysis
  MultiADC_t** fMulti;          // multi-hit handlers
  Int_t* fADCdefined;           // array showing if ADC[i] defined
  UInt_t* fScaler;              // Scaler buffer
  UInt_t* fScalerS;             // Scaler sum (used for non-clearing scalers)
  Bool_t* fScalerClr;           // Scaler cleared on read?
  UInt_t* fScalerBuff;          // Scaler buffer + extra
  UInt_t* fNEventScaler;        // list of event # for scaler storage
  UInt_t** fScalerHist;         // list of read values for all scalers
  Double_t* fScalerSum;         // Accumulated scaler buffer
  Int_t fNScalerBlock;          // fragmented scaler read...#blocks
  Int_t* fScBlockLength;        // length of each block
  Int_t fScalerBlock;           // scaler block counter
  Int_t fProcessType;           // type of data processing

  Bool_t fIsOnline;		// Online/offline mode
  Bool_t fIsHbookFile;          // Flag input file is HBOOK ntuple file
  Bool_t fIsSortBusy;           // set when analysing buffer of data
  Bool_t fIsScalerRead;         // set when scaler read during event
  Bool_t fIsFinished;           // set when all sorting done
  Bool_t fIsBatch;              // running batch mode?
  Bool_t fIsLocalDAQ;           // local DAQ thread?
  Bool_t fIsMk2Format;          // flag Mk2 data format
  Bool_t fIsLogFile;            // log files or stdout?
  Bool_t fIsPrintError;         // flag printout of hardware errors
  Bool_t fUseDirectIO;         // use direct FileIO for reading
  

  //For EPICS buffers
  Int_t fNEpics;                        //No of different epics event types
  Char_t *fEpicsBuff[EMaxEpicsModules]; //Buffers for the EPICS data
  Bool_t fIsEpicsRead;                  // set when EPICS read during event
  Bool_t fEpicsIndex[EMaxEpicsModules]; //Flag EPICS indices read in EPICS event

   // Private functions to decode Acqu data buffer
  void Mk1EventLoop( UInt_t* );        // decode one Mk1 data record
  void Mk2EventLoop( UInt_t* );        // decode one Mk2 data record
  UInt_t* StoreScalers( UInt_t* );     // identify and store scalers Mk1 format
  UInt_t* StoreScalersMk2( UInt_t* );  // identify and store scalers Mk2 format
  UInt_t* StoreEpics( UInt_t*);        // identify and store epics 
  void StoreOfflineScalers( Int_t );   // identify and store scalers from tree
  void StoreScalerBlock( UInt_t*, Int_t, Int_t);
  UInt_t* Mk1ErrorCheck( UInt_t* );    // identify and skip error record
  UInt_t* Mk2ErrorCheck( UInt_t* );    // Mk2 data..store error block

public:
   TAcquRoot( const Char_t*, Bool_t=EFalse, Bool_t=kTRUE );
   virtual ~TAcquRoot();
   void SetConfig( Char_t*, int );
   void SaveTreeFile( Char_t* );
   void LoadHbookFile( Char_t* );
   void Run( );
   void Start( );
   void DataLoop();
   void DataLoopDirectIO();   
   Bool_t DataLoopDirectIOWorker(const std::string& filename, const UInt_t stop);
#ifdef WITH_LIBLZMA
   Bool_t DataLoopDirectIOWorkerXZ(const std::string& filename, const UInt_t stop); 
#endif
   void OfflineLoop();
   void Clear( );
   void Reset( );
   void LinkDataServer( );
   Bool_t LinkTree( Int_t );             // Setup read from tree file
   Bool_t LinkADC();                     // Get ADC info from Tree
   Bool_t UpdateTree( Int_t );           // New Tree file
   void SetADC( void* );                 // setup ADCs from header info
   void SetADC( Int_t, Int_t, Int_t );   // manual setup of ADCs
   void SetScaler( Int_t );              // setup of scaler buffers
   UInt_t SetEventSize( UInt_t );
   void SetAnalysis( TA2Analysis* );
   void SetDataServer( TA2DataServer* );
   void GetErrorModule( Int_t* );       // Pull module indices from error block
   // Inline Get/Set functions
   TA2Analysis* GetAnalysis() { return fAnalysis; }       // ->Analysis Class
   TA2DataServer* GetDataServer() { return fDataServer; } // -> Data Server Cl
   Char_t* GetAnalysisType(){ return fAnalysisType; }     // Analysis species
   Char_t* GetAnalysisSetup(){ return fAnalysisSetup; }   // Analyse.param.file
   Char_t* GetDataServerSetup(){ return fDataServerSetup; }// Server param.file
   Char_t* GetLocalDAQSetup(){ return fLocalDAQSetup; }    // DAQ param file
   UInt_t GetEventSize(){ return fEventSize; }  // return event-buffer size
   Int_t GetNEvent(){ return fNEvent; }		// # events on offline files
   Int_t GetNbranch(){ return fNbranch; }       // # branches to event
   void** GetEvent(){ return fEvent; }          // return event-buffer ptr
   void* GetHardwareList(){ return fHardwareList; }
   ReadErrorMk2_t* GetHardwareError(){ return fHardwareError; }
   Int_t GetNModule(){ return fNModule; }
   Int_t GetMaxADC(){ return fMaxADC; }
   Int_t GetMaxScaler(){ return fMaxScaler; }
   Int_t GetADCError(){ return fADCError; }
   Int_t GetScalerError(){ return fScalerError; }
   Int_t GetHardError(){ return fHardError; }
   Int_t GetCurrEvent(){ return fCurrEvent; }   // current event number
   Int_t* GetADCdefined(){ return fADCdefined;}
   UShort_t* GetADC(){ return fADC; }
   UInt_t* GetScalerBuff(){ return fScalerBuff; }
   UInt_t* GetScaler(){ return fScaler; }
   UInt_t* GetScalerS(){ return fScalerS; }
   Bool_t* GetScalerClr(){ return fScalerClr; }
   Double_t* GetScalerSum(){ return fScalerSum; }
   Int_t GetNScalerBlock(){ return fNScalerBlock; }
   Int_t* GetScBlockLength(){ return fScBlockLength; }
   Int_t GetScalerBlock(){ return fScalerBlock; }
   UInt_t** GetScalerHist(){ return fScalerHist; }
   UInt_t* GetScalerHist( Int_t i ){
     if( fScalerHist ) return fScalerHist[i];
     else return NULL; }
   UInt_t GetScalerHist(Int_t i, Int_t j){ 
     if( fScalerHist ) return fScalerHist[i][j];
     else return 0; }
   Int_t GetNScalerEvent(){ return fNScalerEvent; }
   Int_t GetTotScalerEvent(){ return fTotScalerEvent; }
   UInt_t* GetNEventScaler(){ return fNEventScaler; }
   MultiADC_t** GetMulti(){ return fMulti; }          // multi-hit ADC's
   MultiADC_t* GetMulti(Int_t i){ return fMulti[i]; } // multi-hit ADC's
   Char_t* GetFileName(){
     // ONLINE...access the current file name from the ACQU header record
     if( fIsMk2Format )
       return((AcquMk2Info_t*)((UInt_t*)fHeaderBuff + 1))->fOutFile;
     else
       return((AcquExptInfo_t*)((UInt_t*)fHeaderBuff + 1))->fOutFile;
   }
   Char_t* GetFileTimeString(){
     // ONLINE...access the start time of the current file from the ACQU header record
     if( fIsMk2Format )
       return((AcquMk2Info_t*)((UInt_t*)fHeaderBuff + 1))->fTime;
     else
       return((AcquExptInfo_t*)((UInt_t*)fHeaderBuff + 1))->fTime;
   }
   Int_t GetFileTimeEpoch(){
     const char *headertime = GetFileTimeString();
     struct tm tm;
     strptime(headertime, "%a %b %d %T %Y", &tm);
     time_t timestamp = mktime(&tm);
     return (Int_t)timestamp;
   }
   Int_t GetRunNumber(){
     // ONLINE...access the current run number from the ACQU header record
     if( fIsMk2Format )
       return((AcquMk2Info_t*)((UInt_t*)fHeaderBuff + 1))->fRun;
     else
       return((AcquExptInfo_t*)((UInt_t*)fHeaderBuff + 1))->fRun;
   }
   void* GetHeaderBuff(){ return fHeaderBuff; }
   TFile* GetTreeFile(){ return fTreeFile; }
   Char_t** GetTreeFileList(){ return fTreeFileList; }
   Char_t* GetTreeFileList(Int_t i){
     if(fTreeFileList) return fTreeFileList[i];
     else return NULL; }
   Int_t GetNTreeFiles(){ return fNTreeFiles; }
   Char_t* GetTreeName(){ return fTreeName; }
   Char_t* GetTreeDir(){ return fTreeDir; }
   Char_t* GetBatchDir(){ return fBatchDir; }
   TBranch** GetBranch(){ return fBranch; }
   TBranch* GetBranch( Int_t i ){ 
     if( fBranch ) return fBranch[i];
     else return NULL; }
   Char_t** GetBranchName(){ return fBranchName; }
   Char_t* GetBranchName( Int_t i ){ 
     if( fBranchName ) return fBranchName[i];
     else return NULL; }
   Int_t GetRecLen(){ return fRecLen; };
   Int_t GetNDataBuffer(){ return fNDataBuffer; }
   Int_t GetProcessType(){ return fProcessType; }

   Char_t** GetEpicsBuffer(){ return fEpicsBuff; }
   Bool_t* GetEpicsIndex(){ return fEpicsIndex; }
   Int_t   GetNEpics(){ return fNEpics; }
   
   void SetNTreeFiles(Int_t i){ fNTreeFiles = i; }
   void SetTreeDir(Char_t* dir){ fTreeDir = dir; }
   void SetBatchDir(Char_t *dir){ fBatchDir = dir; }

   void SetIsOnline(){ fIsOnline = ETrue; }
   void SetRecLen( Int_t len ){ fRecLen = len; }
   void SetMulti(MultiADC_t** multi){ fMulti = multi; }
   Bool_t IsSortBusy(){ return fIsSortBusy; }
   Bool_t IsScalerRead(){ return fIsScalerRead; }
   Bool_t IsEpicsRead(){ return fIsEpicsRead; }
   Bool_t IsOnline(){ return fIsOnline; }
   Bool_t* GetScalerRead(){ return &fIsScalerRead; }
   Bool_t IsFinished(){ return fIsFinished; }
   Bool_t IsBatch(){ return fIsBatch; }
   Bool_t IsLocalDAQ(){ return fIsLocalDAQ; }
   Bool_t IsLogFile(){ return fIsLogFile; }
   Bool_t IsPrintError(){ return fIsPrintError; }
   Bool_t UseDirectIO() { return fUseDirectIO; }
   
   void PrintTree(){ if( fTree ) fTree->Print(); }

   ClassDef(TAcquRoot,1)   
};

//
//  Inlined functions...basic sorting of Acqu data buffer
//----------------------------------------------------------------------------
inline UInt_t* TAcquRoot::StoreScalers( UInt_t* sc )
{
  //  Copy scaler block from event into buffer
  //  Add current scaler contents to accumulated scaler contents
  //  If wrong number of scalers found, don't store any scaler
  //  values, but search for the end-of-event marker...and return
  //  it to update the buffer pointer.

  Int_t i;
  Int_t ngoodSc = fMaxScaler;           // # good scalers
  UInt_t* endSc = NULL;                 // end scaler block...NULL = not found
  sc++;		         		// go past scaler block header
  fScalerBuff[0] = fCurrEvent;          // event# for data save

  if( fNScalerBlock ){
    Int_t j = 0;
    for( i=0; i<fScalerBlock; i++ ) j += fScBlockLength[i]; 
    StoreScalerBlock(sc, j, j+fScBlockLength[i]);
    fScalerBlock++;
    if( fScalerBlock >= fNScalerBlock ) fScalerBlock = 0;
    endSc = sc + fScBlockLength[i];
    return endSc;
  }
    
  // Error check. If error found zero scaler buffer and search end-of-event
  // Which should be preceeded (4 UInt_t's before) by a read-error marker
  if( sc[fMaxScaler] != EEndEvent ){
    for( i=0; i<fMaxScaler; i++ ) fScaler[i] = 0;
    fScalerError++;			// error counter
    for( i=0; i<fMaxScaler; i++){	// search end-of-event
      if( sc[i] == EReadError ){        // found read error block
        if( ngoodSc == fMaxScaler )ngoodSc = i;
        i += 4;
      }
      else if( sc[i] == EEndEvent ){
        endSc = sc+i;
        if( ngoodSc == fMaxScaler )ngoodSc = i;
        break;
      }
    }
  }
  // everything in order
  else{
    ngoodSc = fMaxScaler;
    endSc = sc + fMaxScaler;
  }
  // store good scalers  
  for( i=0; i<ngoodSc; i++ ){	            // Save scalers which look OK
    if( fScalerClr[i] ) fScaler[i] = sc[i]; // scaler cleared after read
    else{
      if( sc[i] > (UInt_t)(fScalerS[i]) )
	fScaler[i] = sc[i] - fScalerS[i];
      else  fScaler[i] = sc[i];
      fScalerS[i] = sc[i];
    }
    fScalerSum[i] += fScaler[i];
    if( fScalerSum[i] < 0.5 ) fScalerSum[i] = 0.0;
  }
  // Scan failed to find end-of-event marker....scan further until found
  if( !endSc ){
    endSc = sc + fMaxScaler;
    while( *endSc != EEndEvent ) endSc++;
  }
  return endSc;
}

//----------------------------------------------------------------------------
inline UInt_t* TAcquRoot::StoreEpics( UInt_t* ebuff )
{
  // Copy EPICS data block to the data stream
  //
  Char_t *lbuff;
  //Copy from event to the relevant epics buffer
  Char_t *ptr=(Char_t*)ebuff;
  ptr+=sizeof(UInt_t); //past the header
  
  EpicsHeaderInfo_t *bhead=(EpicsHeaderInfo_t *)ptr;//buffer header
  lbuff=fEpicsBuff[bhead->index];                   //copy to relevant buffer
  for(int i=0;i<bhead->len;i++){
    *lbuff++=*ptr++;
  }
  fEpicsIndex[bhead->index] = kTRUE;                //flag buffer was filled
  return (UInt_t*)ptr;
}

//----------------------------------------------------------------------------
inline UInt_t* TAcquRoot::StoreScalersMk2( UInt_t* sc )
{
  //  Copy Mk2 scaler block from event into buffer
  //  Indexed scaler data and buffer size cross check
  //  Add current scaler contents to accumulated scaler contents
  //  The scaler buffer marker not found after read/decode
  //  zero current scaler buffer and search until end-of-event found
  //  in input buffer

  UInt_t index;
  UInt_t scaler;
  sc++;	                                    // past scaler block header
  index = *sc;                              // scaler block size (bytes)
  Char_t* scbuffEnd = (Char_t*)sc + index;  // end of scaler buffer
  sc++;
  fScalerBuff[0] = fCurrEvent;              // event# for data save
  // store scalers until end of scaler buffer
  while( sc < (UInt_t*)scbuffEnd ){
    index = *sc;
    // Check for an error block
    if( index == EReadError ){
      UInt_t* sctemp = Mk2ErrorCheck(sc);
      if( sctemp > sc ){                    // double check really an error
	sc = sctemp;                        // yes it was
	continue;
      }
    }
    else if( index >= (UInt_t)fMaxScaler ){
      PrintError( "Error in scaler index","< Mk2 Scaler Decode >");
      sc++;
      continue;
    }
    sc++;
    scaler = *sc++;
    if( fScalerClr[index] ) fScaler[index] = scaler; // scaler clear after read
    else{
      if( scaler > (UInt_t)(fScalerS[index]) )
	fScaler[index] = scaler - fScalerS[index];
      else  fScaler[index] = scaler;
      fScalerS[index] = scaler;
    }
    fScalerSum[index] += fScaler[index];
    if( fScalerSum[index] < 0.5 ) fScalerSum[index] = 0.0;
  }
    
  // Buffer integrity check: if error zero scaler buffer, search end-of-event
  if( *sc != EScalerBuffer ){
    for( Int_t i=0; i<fMaxScaler; i++ ) fScaler[i] = 0;
    fScalerError++;			// error counter
    PrintError( "Current scaler buffer zeroed","< Mk2 Scaler Decode >");
    // search end-of-event
    while( *sc != EEndEvent ){ sc++; }
  }
  else sc++;
  return sc;
}

//----------------------------------------------------------------------------
inline void TAcquRoot::StoreScalerBlock(UInt_t* sc, Int_t start, Int_t stop)
{
  // Store scaler block over indices start to stop
  //
  for( Int_t i=start; i<stop; i++ ){
    if( fScalerClr[i] ) fScaler[i] = sc[i-start]; // scaler cleared after read
    else{
      if( sc[i] > (UInt_t)(fScalerS[i]) )
	fScaler[i] = sc[i] - fScalerS[i];
      else  fScaler[i] = sc[i];
      fScalerS[i] = sc[i];
    }
    fScalerSum[i] += fScaler[i];
    if( fScalerSum[i] < 0.5 ) fScalerSum[i] = 0.0;
  }
}

//----------------------------------------------------------------------------
inline void TAcquRoot::StoreOfflineScalers( Int_t scevent )
{
  // Decode scaler data block read from ROOT tree
  //
  UInt_t* sptr = (UInt_t*)fEvent[EARScalerBr];
  UInt_t* hptr = fScalerHist[scevent];
  fNEventScaler[scevent] = *sptr++;             // original event number
  for( Int_t i=0; i<fMaxScaler; i++ ){
    *hptr = *sptr++;                            // store scaler value
    fScalerSum[i] += *hptr++;                   // scaler sum
  }
}

//----------------------------------------------------------------------------
inline UInt_t* TAcquRoot::Mk1ErrorCheck( UInt_t* datum )
{
  // Decode hardware error block and store error
  // statistics
  //
  ReadError_t* rerror = (ReadError_t*)datum;  // Hardware error block struct
  ReadErrorMk2_t* sterror = fHardwareError + fHardError;  // stored err block
  sterror->fHeader = sterror->fTrailer = EReadError;
  sterror->fModIndex = rerror->fBus;
  sterror->fModID = rerror->fCrate;
  sterror->fErrCode = rerror->fCode;
  fHardError++;			              // register error
  rerror++;                                   // advance past error block
  return (UInt_t*)rerror;
}

//----------------------------------------------------------------------------
inline UInt_t* TAcquRoot::Mk2ErrorCheck( UInt_t* datum )
{
  // Decode hardware error block. Double check it really is an error block
  // ie error-block fTrailer = EReadError
  // If it is, store error statistics and increment data pointer
  // past error block 
  //
  ReadErrorMk2_t* rerror = (ReadErrorMk2_t*)datum;      // error block in data
  if( rerror->fTrailer != EReadError ) return (UInt_t*)rerror;
  fHardwareError[fHardError] = *rerror;                 // store error block
  if(fIsPrintError)
    fprintf(fLogStream,
	"Hardware Error  Event:%d  Mod-ID:%x  Mod-Index:%d  Error-Code:%d\n",
	    fCurrEvent, rerror->fModID, rerror->fModIndex, rerror->fErrCode );
  fHardError++;			                        // register error
  rerror++;                                   // advance past error block
  return (UInt_t*)rerror;
}

//----------------------------------------------------------------------------
inline void TAcquRoot::GetErrorModule( Int_t* errormod )
{
  // Pull out hardware module indices from error-block array
  // ie error-block fTrailer = EReadError
  // If it is, store error statistics and increment data pointer
  // past error block 
  //
  ReadErrorMk2_t* rerror;
  Int_t i;
  for( i=0; i<fHardError; i++ ){
    rerror = fHardwareError + i;    // error block in data
    errormod[i] = rerror->fModIndex;
  }
  errormod[i] = ENullHit;
  return;
}

#endif

