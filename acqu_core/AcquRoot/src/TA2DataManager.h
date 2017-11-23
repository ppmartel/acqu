//--Rev 	JRM Annand... 4th Feb 2004...Data-cuts
//--Rev 	JRM Annand...26th Jun 2004...GetGrandChild check child
//--Rev 	JRM Annand...24th Aug 2004...Add rate-mon and bit-pattern stuff
//--Rev 	JRM Annand...18th Oct 2004...AddChild pass log stream, periodic
//--Rev 	JRM Annand...21st Oct 2004...Data read/save ROOT format
//--Rev 	JRM Annand...17th Nov 2004...EOF, finish macros
//--Rev 	JRM Annand...14th Jan 2005...fTreeFileName
//--Rev 	JRM Annand...23rd Mar 2005...Tree file title
//--Rev 	JRM Annand... 6th Apr 2005...2D cuts, misc cut scope
//--Rev 	JRM Annand...14th Apr 2005...Wild-card cut options
//--Rev 	JRM Annand... 7th Jun 2005...Random generator
//--Rev 	JRM Annand... 1st Aug 2005...SetCut == in if()
//--Rev 	JRM Annand...26th Oct 2006...Local scalers
//--Rev 	JRM Annand...29th Jan 2007...4v0 TA2System compatible
//--Rev 	JRM Annand...25th Nov 2008...GetChildType() nth instance
//--Rev 	JRM Annand... 1st Sep 2009   delete[]
//--Update	K Livingston..7th Feb 2013   Support for handling EPICS buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2DataManager
//
// Base class for any analysis entity (analysis, apparatus, detector) to
// enable transfer of raw or analysed data between parent and child classes
//
#ifndef __TA2DataManager_h__
#define __TA2DataManager_h__

#include "TAcquRoot.h"
#include "TA2Cut1D.h"
#include "TA2Cut2D.h"
#include "TA2MultiCut.h"
#include "MultiADC_t.h"
#include "TA2BitPattern.h"     // Trigger bit patterns
#include "TA2RateMonitor.h"    // Scalers as function of time
#include "TA2_N2V.h"           // holds name to variable ptr association

#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"

// Global pointers to Acqu - Root interface class and main analyser
R__EXTERN TAcquRoot*   gAR;
R__EXTERN TA2Analysis*   gAN;

class TA2DataManager : public TA2System {
 private:
 protected:
  virtual void PostInit();               // Postinitialise without flags
  TRandom* fRandom;                      // Random generator
  Int_t fRandSeed;                       // initial seed for random
  TA2DataManager* fParent;               // one tier up in analysis
  TList* fChildren;                      // one tier down in analysis
  TList* fVariables;                     // info on cut/histgram variables
  TFile* fTreeFile;                      // output file for trees etc
  TTree* fTree;                          // ROOT tree for output
  TBranch** fBranch;                     // separate branches of tree
  Int_t fNbranch;                        // # branches to analyse
  TA2MultiCut* fDataCuts;                // data filtering conditions
  TA2MultiCut* fHistCuts;                // histogram filtering conditions
  TA2MultiCut* fMiscCuts;                // miscellaneous filtering conditions
  TA2MultiCut* fCompoundCut;             // for setting up compount cuts
  TA2RateMonitor* fRateMonitor;          // scaler rate monitors
  TA2BitPattern* fBitPattern;            // pattern-unit handler
  Int_t** fPatternHits;                  // hits from bit pattern units
  UInt_t* fNPatternHits;                 // # hits from bit pattern units 
  UShort_t* fADC;                        // ADC array for subsequent analysis
  Int_t* fADCdefined;                    // Tag if ADC[i] defined
  MultiADC_t** fMulti;                   // list of any multi-hit ADC's
  UInt_t* fScaler;                       // Scaler dump
  UInt_t* fScalerBuff;                   // Scaler dump + other info
  Double_t* fScalerSum;                  // Accumulated scaler
  void** fEvent;                         // decoded event buffer
  Int_t fMaxADC;                         // no. ADC's defined in data base
  Int_t fMaxScaler;                      // no. scalers defined in data base
  UInt_t* fScalerIndex;                  // local scaler indices
  UInt_t* fScalerCurr;                   // local scalers current buffer
  Double_t* fScalerAcc;                  // local scalers accumulated buffer
  UInt_t fNPeriod;                       // periodic task event counter
  UInt_t fMaxPeriod;                     // # events between periodic tasks
  Char_t* fPeriodCmd;                    // period command to execute
  Char_t* fEpicsCmd;                     // epics command to execute
  Char_t* fEndOfFileCmd;                 // end-of-file command to execute
  Char_t* fFinishCmd;                    // end-of-run command to execute
  Char_t* fTreeFileName;                 // reduced data tree file name
  Bool_t fIsRawDecode;                   // perform Raw decoding?
  Bool_t fIsDecode;                      // perform decoding?
  Bool_t fIsReconstruct;                 // perform reconstruction?
  Bool_t fIsRawDecodeOK;                 // Raw decoding passed conditions?
  Bool_t fIsDecodeOK;                    // Decoding passed conditions?
  Bool_t fIsReconstructOK;               // Reconstruction passed conditions?
  Bool_t fIsCut;                         // perform data cuts?
  Bool_t fIsBitPattern;                  // bit pattern decode
  Bool_t fIsRateMonitor;                 // scaler time dependence
  Bool_t fIsEndOfFile;                   // end-of-file procedures?
  Bool_t fIsFinish;                      // end-of-run procedures?
  Bool_t fIsSaveEvent;                   // save output data?
  const Map_t* kValidChild;              // valid child class types
  // Stuff for name-variable associations used for histograms
  // and data-filter cuts
  template<class T>
    void LoadVariable( const char*, T*, Int_t );
  TObject* GetN2V( const char* );
  template<class T> T* GetVar( const char* );
  Int_t GetVarType( const char* );
  Bool_t IsInt( const char* );
public:
  TA2DataManager( const Char_t*, const Map_t* ); // standard constructor
  virtual ~TA2DataManager();
  virtual TA2DataManager* CreateChild( const char*, Int_t ) = 0;
  virtual void LoadVariable();
  virtual void ParseCut( char* );
  virtual void ResetCut();
  virtual void PostInitialise();
  template<typename T>
    void SetCut( Char_t*, UInt_t, Int_t, Int_t, T*, T, T, T* = NULL,
		 TCutG* = NULL, Double_t* = NULL );
  virtual void AddChild( TA2DataManager*, const char* = "TA2Detector" );
  virtual void AddChild( char*, TA2DataManager** = NULL );
  virtual void Decode( );
  virtual void DecodeSaved( );
  virtual void Reconstruct( );
  virtual void Cleanup( );             // end of event tasks
  virtual Bool_t UpdatePeriod( );      // check if period due
  virtual void Periodic( );            // tasks after n events
  virtual void Epicsodic( );           // tasks after epics events
  virtual void ParsePeriod( Char_t* ); // parse period
  virtual void ParseEpics( Char_t* );  // parse EPICS
  virtual void ParseEndOfFile( Char_t* ); // parse EOF macro
  virtual void ParseFinish( Char_t* ); // parse end of run macro
  virtual void SetEndOfFile();         // enable end-of-file tasks
  virtual void SetFinish();            // enable end-of-run tasks
  virtual void EndFile( );             // end of file tasks
  virtual void Finish( );              // end of run tasks
  virtual void MacroExe( Char_t* );    // execute ROOT macro       
  virtual void SetSaveEvent();         // enable event-mode data save
  virtual void InitSaveTree(Char_t**, void**, Char_t* = NULL, 
			    Char_t** = NULL); // init output data tree
  virtual void InitSaveTree();         // for nested data saves
  virtual void SaveEvent( );           // save "event" to Tree file
  virtual void SaveEvent( Int_t );     // save "branch" to Tree file
  virtual void CloseEvent();           // close tree and file
  TA2DataManager* GetChildType( const Char_t*, Int_t=0 );
  TA2DataManager* GetChild( const char*, const char* = "TA2Apparatus" );
  TA2DataManager* GetGrandChild( const char*, const char* = "TA2Detector" );

  TA2DataManager* GetParent(){ return fParent; }         // parent link
  TList* GetChildren(){ return fChildren; }              // child list
  TA2MultiCut* GetDataCuts(){ return fDataCuts; }        // data cuts list
  TA2MultiCut* GetHistCuts(){ return fHistCuts; }        // hist cuts list
  TA2MultiCut* GetMiscCuts(){ return fMiscCuts; }        // misc cuts list
  UShort_t* GetADC(){ return fADC; }                     // -> ADC array
  Int_t* GetADCdefined(){ return fADCdefined; }          // -> ADC defs
  UInt_t* GetScaler(){ return fScaler; }                 // -> Scaler array
  Double_t* GetScalerSum(){ return fScalerSum; }         // -> Scaler sum array
  void** GetEvent(){ return fEvent; }                    // -> Decoded event
  void* GetEvent(Int_t i){ return fEvent[i]; }           // -> Decoded subevent
  Int_t GetMaxADC(){ return fMaxADC; }                   // # ADCs
  Int_t GetMaxScaler(){ return fMaxScaler; }             // # Scalers
  UInt_t* GetScalerIndex(){ return fScalerIndex; }       // local scalers
  UInt_t* GetScalerCurr(){ return fScalerCurr; }         // local scalers
  Double_t* GetScalerAcc(){ return fScalerAcc; }         // local scalers
  Int_t GetScalerError(){ return gAR->GetScalerError();} // scaler errors
  MultiADC_t* GetMulti( int i ){ return fMulti[i]; }     // multi-hit ADC info
  MultiADC_t** GetMulti( ){ return fMulti; }             // multi-hit ADC info
  TA2RateMonitor* GetRateMonitor(){ return fRateMonitor;}// scaler rate mon
  TA2BitPattern* GetBitPattern(){ return fBitPattern; }  // pattern-unit handle
  Int_t** GetPatternHits(){ return fPatternHits; }       // pattern-unit hits
  UInt_t* GetNPatternHits(){ return fNPatternHits; }     // pattern # hits
  TFile* GetTreeFile(){ return fTreeFile; }              // output data file
  TTree* GetTree(){ return fTree; }                      // output data tree
  UInt_t GetNPeriod(){ return fNPeriod; }        // periodic task counter
  UInt_t GetMaxPeriod(){ return fMaxPeriod; }    // events twixt periodic tasks
  Char_t* GetPeriodCmd(){ return fPeriodCmd; }   // period command
  Char_t* GetEpicsCmd(){ return fPeriodCmd; }   // Epics command
  Char_t* GetEndOfFileCmd(){ return fEndOfFileCmd; }// end-of-file command
  Char_t* GetFinishCmd(){ return fFinishCmd; }   // end-of-run command
  Bool_t IsRawDecode(){ return fIsRawDecode; }           // decoding?
  Bool_t IsDecode(){ return fIsDecode; }                 // decoding?
  Bool_t IsReconstruct(){ return fIsReconstruct; }       // reconstruction?
  Bool_t IsRawDecodeOK(){ return fIsRawDecodeOK; }       // decoding?
  Bool_t IsDecodeOK(){ return fIsDecodeOK; }             // decoding?
  Bool_t IsReconstructOK(){ return fIsReconstructOK; }   // reconstruction?
  Bool_t IsBitPattern(){ return fIsBitPattern; }         // pattern unit stuff?
  Bool_t IsRateMonitor(){ return fIsRateMonitor; }       // rate-stuff enabled?
  Bool_t IsEndOfFile(){ return fIsEndOfFile; }           // end-of-file task?
  Bool_t IsFinish(){ return fIsFinish; }                 // end-of-run task?
  Bool_t IsSaveEvent(){ return fIsSaveEvent; }           // save output data?

  ClassDef(TA2DataManager,1)
};

//-----------------------------------------------------------------------------
inline void TA2DataManager::Decode( )
{
  // Step through list of child analysis entities
  // and call the child decode method for each
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->Decode();
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::DecodeSaved( )
{
  // Step through list of child analysis entities
  // and for each read back decoded (or MC)  data into
  // the child's private data banks
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->DecodeSaved();
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::Reconstruct( )
{
  // Step through list of child analysis entities
  // and call the child reconstruct method for each
  // Do only if reconstruct flag is set

  if( !fIsReconstruct ) return;
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->Reconstruct();
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::Cleanup( )
{
  // Clean up after analysing 1 event
  // Step through list of child analysis entities
  // and call the child cleanup method for each
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->Cleanup();
}

//-----------------------------------------------------------------------------
inline Bool_t TA2DataManager::UpdatePeriod( )
{
  // update periodicity counter if periodic procedures defined
  // and check if the procedure is due this event
  // return true if so
  if( !fMaxPeriod ) return EFalse;             // no periodic task
  if( ++fNPeriod < fMaxPeriod ) return EFalse; // period not yet up
  fNPeriod = 0;                                // period up...reset counter
  return ETrue;
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::MacroExe( Char_t* macroCmd )
{
  // call a macro, if the command is defined
  if( macroCmd ) gROOT->ProcessLine( macroCmd );
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::PostInitialise( )
{
  // Wrap PostInit in the fIsInit exclusion flag
  if( fIsInit ) return;
  PostInit();
  fIsInit = ETrue;
}

//-----------------------------------------------------------------------------
template<class T>
inline void TA2DataManager::LoadVariable(const char* name, T* var, Int_t type)
{
  if( !var ){
    const Char_t* line = Form("%s\n",name);
    PrintError(line,"<Load-Variable...NULL variable pointer supplied>");
    return;
  }
  if( !fVariables ) fVariables = new TList;
  TObject* n2v = new TA2_N2V<T>( name, var, type );
  fVariables->AddLast( n2v );
}

//-----------------------------------------------------------------------------
inline TObject* TA2DataManager::GetN2V( const char* name )
{
  // Search name-string variable-pointer association for match with
  // name. Return associated pointer for 1st match found. Return NULL
  // if no match found.

  if( !fVariables ){
    PrintError((char*)name,"<Need to LoadVariables() beforehand\n>");
    return NULL;
  }
  TIter nextn2v( fVariables );
  TObject* n2v;
  while( (n2v = nextn2v()) ){
    if( strcmp( name, ((TA2_N2V<void>*)n2v)->GetName() ) ) continue;
    return n2v;
  }
  if(!fChildren) return NULL;

  // If there are children search their variable lists
  TIter nextchild(fChildren);
  TA2DataManager* child;
  const char* nm;
  while( ( child = (TA2DataManager*)nextchild() ) ){
    nm = child->GetName();
    if(strncmp(name,nm,strlen(nm))) continue;
    nm = name + strlen(nm) + 1;
    if( (n2v = child->GetN2V(nm)) ) return n2v;  // success
  }
  return NULL;
} 

//-----------------------------------------------------------------------------
template<class T> inline T* TA2DataManager::GetVar( const char* name )
{
  TObject* n2v = GetN2V( name );
  if( ! n2v ) return NULL;
  else return ((TA2_N2V<T>*)n2v)->GetVar();
}

//-----------------------------------------------------------------------------
inline Int_t TA2DataManager::GetVarType( const char* name )
{
  TObject* n2v = GetN2V( name );
  if( ! n2v ) return -1;
  else return ((TA2_N2V<void>*)n2v)->GetType();
}

//-----------------------------------------------------------------------------
inline Bool_t TA2DataManager::IsInt( const char* name )
{
  TObject* n2v = GetN2V( name );
  if( ! n2v ) return EFalse;
  else return ((TA2_N2V<void>*)n2v)->IsInt();
}

//-----------------------------------------------------------------------------
inline void TA2DataManager::ResetCut( )
{
  if( !fCompoundCut ) return;
  delete fCompoundCut;
  fCompoundCut = NULL;
}

//----------------------------------------------------------------------------
inline void TA2DataManager::SaveEvent(Int_t i)
{
  // Save branch(es) to Tree
  // Check event saving enabled
  if( !fIsSaveEvent ) return;
  if( i < fNbranch ) fBranch[i]->Fill();
}
//----------------------------------------------------------------------------
inline void TA2DataManager::SaveEvent()
{
  // Save event to Tree
  // Check event saving enabled
  if( !fIsSaveEvent ) return;
  fTree->Fill();
}

#endif
