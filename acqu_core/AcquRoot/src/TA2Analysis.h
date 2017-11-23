//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Rev 	JRM Annand... 5th Feb 2004   Implement cuts
//--Rev 	JRM Annand...31st Mar 2004   Scaler plots
//--Rev 	JRM Annand...26th Jun 2004   Sum scaler zero
//--Rev 	JRM Annand...27th Jun 2004   Rate plots, trig patterns
//--Rev 	JRM Annand...24th Aug 2004   Rate,trig patt. to TA2DataManager
//--Rev 	JRM Annand...21st Oct 2004   Log file, Periodic, ROOT save/rd
//--Rev 	JRM Annand...17th Nov 2004   EOF, finish macros
//--Rev 	JRM Annand... 1st Dec 2004   generic apparatus, physics
//--Rev 	JRM Annand...24th Feb 2005   check if I/P file name changes
//--Rev 	JRM Annand... 3rd Mar 2005   batch-mode log-file directory
//--Rev 	JRM Annand...23rd Mar 2005   particle ID stuff
//--Rev 	JRM Annand...18th May 2005   reduced data save mods (scalers)
//--Rev 	JRM Annand...12th Apr 2006   Scalers() stops at fMaxScaler
//--Rev 	JRM Annand... 7th Jun 2006   Scalers() 4/line, fTreeFileName
//--Rev 	JRM Annand...22nd Jan 2007   4v0 changes
//--Rev 	JRM Annand...20th Jun 2007   Error analysis and display
//--Rev 	JRM Annand... 1st May 2008   InitSaveTree gAR->GetScalerBuff()
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand...25th Nov 2008...add GenericPhysics
//--Rev 	JRM Annand...29th Apr 2009...fix TTree branch format strings
//--Rev 	JRM Annand... 1st Sep 2009...delete[]
//--Rev 	JRM Annand...29th Sep 2012   Add histogram of pattern #hits
//--Update	K Livingston..7th Feb 2013   Support for handling EPICS buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Analysis
//
//	Main data analyser class to implement Online or Offline analysis
//	Take events worth of data from TAcquRoot and process in the
//	manner specified by the configuration files read in at the
//	initialisation stage.

#ifndef __TA2Analysis_h__
#define __TA2Analysis_h__

#include "TList.h"

#include "TA2HistManager.h"            // Acqu-Root histogrammer
#include "DataFormats.h"               // Acqu data formats
#include "FlashADC_t.h"                // Flash ADC handler
#include "CatchTDC_t.h"                // 3-sum SG output handler
#include "SG3sumADC_t.h"               // 3-sum SG output handler
#include "TA2Physics.h"                // Physics analysis
#include <vector>
#include <sstream>

class TA2Apparatus;
class TA2Detector;

class TA2Analysis : public TA2HistManager {
protected:
  AcquBlock_t* fRawData;                  // start of raw event data
  UInt_t fNDAQEvent;                      // event # written to data stream
  Int_t fNhits;				  // # Raw ADC hits
  Int_t fNEvent;                          // events processed
  Int_t fNEvAnalysed;                     // accepted events
  Int_t fNOnlineEvent;                    // event # from orig. onlineanalysis
  Int_t fNHardwareError;                  // # detected hardware errors
  Int_t* fHardwareError;                  // module indices
  FlashADC_t** fFlash;                    // list of any flash ADC's
  MultiADC_t** fMulti;                    // list of any multi-hit ADC's
  TA2Physics* fPhysics;                   // physics level processing
  Char_t* fPrevFileName;                  // previous input data file name
  TA2ParticleID* fParticleID;             // -> PDG info
  Bool_t fIsPhysics;                      // switch on physics processing
  Bool_t fIsProcessComplete;              // event processor status
  std::vector<UShort_t> fMultiADCIds;     // IDs of MultiADC (for reference calculation)
public:
  TA2Analysis( const char* );             // normal use, pass ptr to TAcquRoot
  virtual ~TA2Analysis();
  virtual void SetConfig( char*, Int_t ); // decode and implement setup
  virtual void Process();                 // process Mk1 event
  virtual void MProcess();                // process ROOT/Mk1 event
  virtual void Cleanup();                 // reset at end of event
  virtual TA2DataManager* CreateChild( const char*, Int_t );
  virtual void ParseDisplay( char* );     // display setup
  virtual void LoadVariable();            // variables for standard display
  virtual void ParseADC( char*, Int_t );  // decode ADC spec line
  virtual void PostInitialise();          // some setup after parms read in
  virtual void Scalers( Char_t* = NULL ); // provide scaler info
  virtual void ZeroSumScalers();          // zero sum scalers
  virtual void RawDecode();               // pull ADCs, Scalers from data
  virtual void EndFile();                 // tasks @ end of file
  virtual void Finish();                  // tasks @ end of run
  virtual void InitSaveTree( Char_t* );   // init output data tree
  virtual void ChangeTreeFile( Char_t* ); // new input....new output tree file
  virtual void InitTreeFileName( Char_t*);// construct name of tree file
  virtual void SaveHist();                // save ROOT memory to file

  // Inlined getter procedures
  Int_t GetNhits(){ return fNhits; }                 // no. of hits in event
  FlashADC_t* GetFlash( Int_t i ){ return fFlash[i]; } // Flash ADC info
  MultiADC_t** GetMulti(){ return fMulti; }          // multi-hit ADC's
  MultiADC_t* GetMulti(Int_t i){ return fMulti[i]; } // multi-hit ADC's
  TA2Physics* GetPhysics(){ return fPhysics; }
  Bool_t IsPhysics(){ return fIsPhysics; }           // physics level analysis?
  Int_t GetNEvent(){ return fNEvent; }
  Int_t GetNEvAnalysed(){ return fNEvAnalysed; }
  Int_t GetNOnlineEvent(){ return fNOnlineEvent; }
  UInt_t GetNDAQEvent(){ return fNDAQEvent; }
  TA2ParticleID* GetParticleID(){ return fParticleID; }
  void SetParticleID( TA2ParticleID* pid )
  { if( !fParticleID ) fParticleID = pid; }
  Bool_t IsProcessComplete(){ return fIsProcessComplete; }
  //
  // Root needs this line for incorporation in dictionary
  ClassDef(TA2Analysis,1)
};

//---------------------------------------------------------------------------
inline void TA2Analysis::RawDecode( )
{
  // Decode event passed by Acqu-Root into ADC or Scaler arrays
  // Check ID's in event vector are valid
  // Accumulate Raw-ADC histograms if enabled
  // 22/10/04 fNDAQEvent written into event buffer
  // 26/10/04 fNEvent (total event counter) written to buffer if online
  //          fNEvent read from buffer if offline

  UInt_t* event = (UInt_t*)(fEvent[EARHitBr]);  // start of event buffer
  fNhits = *event++;                            // no. channels read in event
  fNDAQEvent = *event++;                        // DAQ event # in data
  // Total event counter...fNEvent already incremented
  if( gAR->IsOnline() ){
    *event++ = fNOnlineEvent = fNEvent - 1;
    fNHardwareError = gAR->GetHardError();      // store error stats...
    gAR->GetErrorModule(fHardwareError);        // only if online
  }
  else fNOnlineEvent = *event++;                // offline..read from data
  fRawData = (AcquBlock_t*)event;               // start of ADC data
  AcquBlock_t* d = fRawData;
  FlashADC_t* f;
  MultiADC_t* m;
  fMultiADCIds.clear(); // start with an empty array

  for( int j=0; j<fNhits; ){
    if( d->id >= fMaxADC ){
      std::stringstream ss;
      ss << " Error found undefined ADC " << d->id << " in DAQ event "
         << fNDAQEvent << "\n";
      PrintMessage(ss.str().c_str(), kTRUE);
      d++;j++;
      continue;
    }
    switch( fADCdefined[d->id] ){
    case 0:                             // something wrong if this happens
    {
      std::stringstream ss;
      ss << " Error found undefined ADC " << d->id << " in DAQ event "
         << fNDAQEvent << "\n";
      PrintMessage(ss.str().c_str(), kTRUE);
      d++;j++;
      break ;                           // cannot process
    }
    case EFlashADC:                     // save the multiple flash data
      f = fFlash[d->id];
      f->Fill( d );
      d += f->GetNstore();
      j += f->GetNstore();
      break;
    case EMultiADC:                     // save the multi-hit data
      // remember the index for reference calculation after this loop
      fMultiADCIds.push_back(d->id);
      // fill it
      m = fMulti[d->id];
      m->Fill( d );
      d += m->GetNtry();
      j += m->GetNtry();
      break;
    default:                            // standard single-value ADC
      fADC[d->id] = d->adc;             // store the value in the array
      d++;j++;
      break;
    }
  }

  // now, after all events are processed, we can apply
  // reference values to the stored hits
  // we cannot achieve this in general during the loop above,
  // since the needed reference value could come later in the
  // data stream than the value which should have been corrected
  for(size_t i=0;i<fMultiADCIds.size();i++) {
    fMulti[fMultiADCIds[i]]->ApplyRef();
  }

  if( fIsBitPattern ) fBitPattern->Decode();
  if( fIsRateMonitor ) fRateMonitor->Decode();
} 

//-----------------------------------------------------------------------------
inline void TA2Analysis::Process( )
{
  // More flexible data processor has multiple data paths
  // Mk1 format or ROOT Tree input
  // Usually called by AcquRoot event loop
  // Analyse a single event...parameter event points to the event start
  // All saves moved to Cleanup function 18/5/05

  fNEvent++;                        // Events processed
  fIsProcessComplete = EFalse;      // not passed cuts yet
  Periodic();                       // check if any period task due

  if( gAR->IsEpicsRead() ){         // If it's an EPICS event 
    Epicsodic();                    // Check if any epics task loaded
  }

  switch ( gAR->GetProcessType() ){
  // Extract data from input stream provided by TAcquRoot
  case EMk1Process:
    // Raw Event-mode data in adc-index/adc-value pairs
    RawDecode( );
    if( !fDataCuts->Test() ){
      Cleanup();
      return;
    }
    // Fill the raw ADC histograms (if any)
    FillHist();
    // Convert raw ADCs to energy time etc at detector level
    Decode();
    if( !fIsDecodeOK ){
      Cleanup();
      return;
    }
    break;                     // go to standard decode
  case EMCProcess:
    // Monte carlo generated data which is already "calibrated"
    // converted from ADC value to energy, time etc.
    DecodeSaved();
    if( !fIsDecodeOK ){
      Cleanup();
      return;
    }
    break;
  case EPhysicsProcess:
    // Data generally saved as 4-vectors with particle ID
    fPhysics->Decode();
    goto StartPhysics;
  default:
    PrintError("","<Invalid Process Type ID>",EErrFatal);
    break;
  }
  // Apparatus level
  if( fIsReconstruct ){
    Reconstruct();
    if( !fIsReconstructOK ){
      Cleanup();
      return;
    }
  }
  // Physics reconstruction from apparatus info
 StartPhysics:
  if( fIsPhysics ){
    fPhysics->Reconstruct();
    if( !fPhysics->GetDataCuts()->Test() ){
      Cleanup();
      return;
    }
    fPhysics->FillHist();    // accumulate any defined physics histograms
    fPhysics->SaveEvent();   // if physics data output save physics vectors
  }
  fNEvAnalysed++;            // accepted event counter
  fIsProcessComplete = ETrue;// all OK
  Cleanup();
}  

//-----------------------------------------------------------------------------
inline void TA2Analysis::Cleanup( )
{
  // Data saving (if enabled) moved here from Process() 18/5/05
  // Check if scaler read. If so store scalers (if enabled) 
  // and check if event passed conditions...
  // store null hits ADC event if the event did not pass (if enabled)
  // If event passed conditions store ADC hits (if enabled)
  // Clear the ADC array
  // Decend throught lists of child classes and call their Cleanup functions

  if( gAR->IsScalerRead() ){                    // scaler-read this event?
    SaveEvent(1);                               // save scalers
    if( !fIsProcessComplete ){                  // all cuts passed
      UInt_t* event = (UInt_t*)(fEvent[EARHitBr]);// start of event buffer
      *event = 0;                               // no hits
      SaveEvent(0);                             // null event saved
    }  
  }
  if( fIsProcessComplete ) SaveEvent(0);        // save hits if passed cuts
  //
  AcquBlock_t* d = fRawData;                    // now zero any ADC hits
  for( int i=0; i<fNhits; i++ ){
    if( d->id >= fMaxADC ){
      d++;
      continue;
    }
    switch( fADCdefined[d->id] ){
    case 0:                             // something wrong if this happens
      // Error already flagged in process loop
      // fprintf(fLogStream," Error found undefined ADC %d in even\n", d->id);
      break;
    case EMultiADC:                     // save the multi-hit data
      fMulti[d->id]->Flush();
      break;
    default:                            // standard single-value ADC
      fADC[d->id] = (UShort_t)ENullADC;   // clear ADC in event list
      break;
    }
    d++;
  }
  TA2DataManager::Cleanup();            // Cleanup all child classes
}

//*****************************************************************************
//    Depretiated Process Loop
//*****************************************************************************
//-----------------------------------------------------------------------------
inline void TA2Analysis::MProcess( )
{
  // Original default Process.....experimental format (Mk1) data only
  // Called by AcquRoot event loop
  // Analyse a single event...parameter event points to the event start
  // The default Analyse() histograms all adc channels
  fNEvent++;
  Periodic();

  // Extract raw ADCs and Scalers from data stream
  RawDecode( );
  if( !fDataCuts->Test() ){
    Cleanup();
    return;
  }
  // Fill the raw ADC histograms (if any)
  FillHist();
  // Convert raw ADCs to energy time etc at detector level
  Decode();
  if( !fIsDecodeOK ){
    Cleanup();
    return;
  }
  // Apparatus level
  if( fIsReconstruct ){
    Reconstruct();
    if( !fIsReconstructOK ){
      Cleanup();
      return;
    }
  }
  // Physics reconstruction from apparatus info
  if( fIsPhysics ){
    fPhysics->Reconstruct();
    if( !fPhysics->GetDataCuts()->Test() ){
      Cleanup();
      return;
    }
    fPhysics->FillHist();   // accumulate any defined physics histograms
    fPhysics->SaveEvent();  // if physics data output save physics vectors
  }
  SaveEvent();              // if raw data output save event
  fNEvAnalysed++;
  Cleanup();
}

#endif
