//--Author	JRM Annand    9th Jan 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...30th Sep 2003...new private variables
//--Rev 	JRM Annand...15th Oct 2003   Generalise methods  
//--Rev 	JRM Annand... 5th Feb 2004   Cuts & filling 
//--Rev 	JRM Annand...27th May 2004   Time walk 
//--Rev 	JRM Annand...22nd Jun 2004   Raw hits and patterns
//--Rev 	JRM Annand...24th Aug 2004   Trig patt. to TA2DataManager
//--Rev 	JRM Annand...18th Oct 2004   log files & periodic tasks
//--Rev 	JRM Annand...18th Mar 2005   Bug GetMaxHits()
//--Rev 	JRM Annand... 7th Jun 2005   add fEnergyScale
//--Rev 	JRM Annand...11th Nov 2005   add coordinate system shift
//--Rev 	JRM Annand... 2nd Dec 2005   finalise coordinate system shift
//--Rev 	JRM Annand... 8th Sep 2006   "ALL" memory leaks, indexed walk
//--Rev 	JRM Annand...25th Oct 2006   Add some getter inline fn.
//--Rev 	JRM Annand...22nd Jan 2007   4v0 update
//--Rev 	JRM Annand...22nd Jun 2007   Some housekeeping
//--Rev 	JRM Annand...30th Oct 2008   4v3 Multi hit TDC processing
//--Rev 	JRM Annand... 3rd Dec 2008   Walk correction input
//--Rev 	JRM Annand... 1st Sep 2009   delete[], fNelem++...HitD2A
//--Rev 	JRM Annand...22nd Dec 2009   Cleanup bug fix
//--Rev 	DL Hornidge..27th Jan 2011   fixed array-termination bug
//--Rev 	JRM Annand... 6th Jul 2011   PostInit (A.Mushkarenkov)
//--Rev 	JRM Annand... 6th Oct 2012   DecodeBasic bug fix time hits
//--Rev 	JRM Annand... 7th Mar 2013   DecodeBasic filter out time = -1
//--Update	JRM Annand...23rd Apr 2013   Add GetEnergyScalePtr() (git)
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Detector
//
//	Base class for any detector system...cannot be instantiated
//	Specify set of procedures which user-defined derivatives
//	must implement
//	User-defined detector classes must inherit from this one.
//      Provides basis to digital to energy/time conversion
//

#ifndef __TA2Detector_h__
#define __TA2Detector_h__

#include "TA2HistManager.h"
#include "HitD2A_t.h"
#include "TVector3.h"

// constants for default detector command-line maps below
enum { 
  EDetectorSize, EDetectorElement,EDetectorAllElement, EDetectorInit,
  EDetectorDisplay, EDetectorDataCut, EDetectorTimeWalk, EDetectorAllTimeWalk,
  EDetectorRawHits, EDetectorBitPattern, EDetectorPeriod,
  EDetectorEndOfFile, EDetectorFinish, EDetectorEnergyScale,
  EDetectorEnergyRandom, EDetectorShiftCoord, EDetectorMultihit,
  EDetectorEnergy, EDetectorTime, EDetectorHits, EDetectorMulti,
  EDetectorTotalEnergy, EDetectorTimeOR, EDetectorIgnoreElement
};

// Coordinate transformation operator identities
enum { 
  E_TX, E_TY, E_TZ, E_RX, E_RY, E_RZ, E_IX, E_IY, E_IZ, E_MX, E_MY, E_MZ 
};

// Command-line default detector key words which determine
// what information is read in
static const Map_t kDetectorDefaultKeys[] = {
  {"Size:",         EDetectorSize},
  {"Element:",      EDetectorElement},
  {"AllElement:",   EDetectorAllElement},
  {"Initialise:",   EDetectorInit},
  {"Display:",      EDetectorDisplay},
  {"Data-Cut:",     EDetectorDataCut},
  {"TimeWalk:",     EDetectorTimeWalk},
  {"AllTimeWalk:",  EDetectorAllTimeWalk},
  {"RawHits:",      EDetectorRawHits},
  {"BitPattern:",   EDetectorBitPattern},
  {"Period:",       EDetectorPeriod},
  {"EndFile:",      EDetectorEndOfFile},
  {"Finish:",       EDetectorFinish},
  {"Energy-Scale:", EDetectorEnergyScale},
  {"Energy-Random:",EDetectorEnergyRandom},
  {"Shift-Coord:",  EDetectorShiftCoord},
  {"Multi-hit:",    EDetectorMultihit},
  {"Ignore-Elem:",  EDetectorIgnoreElement},
  {NULL,          -1}
};

class TA2Detector : public TA2HistManager {
 private:
 protected:
  HitD2A_t** fElement;                   // Array of calibration info
  Double_t* fEnergy;                     // stored hit energies if any
  Double_t* fEnergyOR;                   // stored energy OR
  Double_t* fTime;                       // stored hit times
  Double_t** fTimeM;                      // stored multihit (TDC) times
  Double_t* fTimeOR;                     // stored hit times for OR timing
  Double_t** fTimeORM;                   // stored hit times for OR timing
  Double_t* fTimeAll;                    // stored hit times for OR timing
  TVector3** fPosition;                  // stored hit positions
  TVector3 fMeanPos;                     // "centre" of detector
  Int_t* fShiftOp;                       // operations global shift of coords
  Double_t* fShiftValue;                 // values global shift of coords
  Int_t fNShiftOp;                       // # global shift operations
  Int_t* fHits;                          // array elements which fired in event
  Int_t** fHitsM;                        // array elements which M-fired
  Int_t* fRawTimeHits;                   // array raw TDC hits
  Int_t* fRawEnergyHits;                 // array raw Energy hits
  const Map_t* fDetectorComm;            // input commands
  const Map_t* fDetectorHist;            // histograms
  Double_t fTotalEnergy;                 // accumulated energy in element array
  Double_t fEnergyScale;                 // global scaling factor for energy
  Double_t fTimeOffset;                  // global time offset
  UInt_t fNhits;                         // No. detector hits in event
  UInt_t* fNhitsM;                       // No. detector M-hits in event
  UInt_t fNhitsAll;                      // No. detector hits+M-hits in event
  UInt_t fNADChits;                      // No. ADC hits in event
  UInt_t fNTDChits;                      // No. TDC hits in event
  UInt_t fNMultihit;                     // Max number multihits to analyse
  UInt_t fNelement;                      // # elements in array
  UInt_t fNelem;                         // counter of defined elements
  UInt_t fMaxHits;                       // max # hits to process
  Bool_t fIsECalib;                      // Is there an energy calibration?
  Bool_t fIsScaler;                      // Are there scalers?
  Bool_t fIsEnergy;                      // pulse height (energy) decoded?
  Bool_t fIsTime;                        // is TDC info decoded
  Bool_t fIsPos;                         // position decoded?
  Bool_t fIsRawHits;                     // Raw hits info stored?
  virtual void DecodeBasic();            // basic decoding without Histograms

public:
  TA2Detector( const char*, TA2System*, const Map_t* = kDetectorDefaultKeys,
	       const Map_t* = NULL );
  virtual ~TA2Detector();
  virtual void SetConfig( char*, Int_t ); // read in setup parameters
  virtual void PostInit( );               // init after parameter input
  virtual void Decode( );                 // default basic calibration/display
  virtual void DecodeSaved( );            // decode previously written data
  virtual void Reconstruct( ){}           // dummy reconstruct function
  virtual void Cleanup();                 // default cleaning up
  virtual void SaveDecoded() = 0;         // inherited class must provide
  virtual void ReadDecoded( ) = 0;        // inherited class must provide
  virtual void SetupArrays( char* );      // # elements to detector etc.
  virtual void DeleteArrays( );           // free memory for arrays
  virtual TA2DataManager* CreateChild( const char*, Int_t ){ return NULL; } 
                                          // dummy default function
  virtual void LoadVariable(  );          // name-variable association
  void ParseShift( Char_t* );             // coord shift variables
  void ApplyShift( Int_t );               // coord shift variables
  void ParseMultihit( Char_t* );          // pase multi-hit command
  HitD2A_t** GetElement(){ return fElement; }       // ptr calibration array
  HitD2A_t* GetElement(Int_t i){                    // ptr calibration element
    if(fElement) return fElement[i];
    else return NULL;
  }
  Double_t* GetEnergy(){ return fEnergy; }          // ptr to energy array
  Double_t* GetEnergyOR(){ return fEnergyOR; }      // ptr to energy OR
  Double_t GetEnergy(Int_t i){ return fEnergy[i]; } // energy value elem i
  Double_t* GetTime(){ return fTime; }              // ptr to time array
  Double_t** GetTimeM(){ return fTimeM; }           // ptr to multi-time array
  Double_t* GetTimeOR(){ return fTimeOR; }          // ptr to time OR
  Double_t* GetTimeORM(Int_t m){return fTimeORM[m];}// ptr to multi time OR
  Double_t* GetTimeAll(){ return fTimeAll; }        // ptr to time All
  Double_t GetTime(Int_t i){ return fTime[i]; }     // time value elem i
  TVector3** GetPosition(){ return fPosition; }     // ptr position array
  TVector3* GetPosition(Int_t i){                   // ptr position vector
    if(fPosition) return fPosition[i];
    else return NULL;
  }
  TVector3 GetMeanPos(){ return fMeanPos; }
  Int_t* GetShiftOp(){ return fShiftOp; }
  Double_t* GetShiftValue(){ return fShiftValue; }
  Int_t GetNShiftOp(){ return fNShiftOp; }
  Int_t* GetHits(){ return fHits; }          // elements which fired in event
  Int_t GetHits(Int_t i){ return fHits[i]; } // single element
  Int_t* GetHitsM(Int_t m){ return fHitsM[m]; } // elements multiplicity m
  Int_t* GetRawTimeHits(){ return fRawTimeHits; }      // TDC hits
  Int_t* GetRawEnergyHits(){ return fRawEnergyHits; }  // QDC hits
  Double_t GetTotalEnergy(){ return fTotalEnergy; }// accumulated energy 
  Double_t GetEnergyScale(){ return fEnergyScale; }// overall energy scaling 
  Double_t* GetEnergyScalePtr(){ return &fEnergyScale; }// ditto pointer 
  Double_t GetTimeOffset(){ return fTimeOffset; }  // overall time offset
  UInt_t GetNhits(){ return fNhits; }              // No. hits in event
  UInt_t GetNhitsM(Int_t m){ return fNhitsM[m]; }  // No. hits multiplicity m
  UInt_t GetNhitsAll(){ return fNhitsAll; }        // No. hits+M-hits in event
  UInt_t GetNADChits(){ return fNADChits; }        // No. ADC  hits in event
  UInt_t GetNTDChits(){ return fNTDChits; }        // No. TDC hits in event
  UInt_t GetNMultihit(){ return fNMultihit; }      // No. multihits stored
  UInt_t GetNelement(){ return fNelement; }        // max detector elements
  UInt_t GetNelem(){ return fNelem; }              // No. detector elements
  UInt_t GetMaxHits(){ return fMaxHits; }          // max # hits to process

  Bool_t IsEcalib(){ return fIsECalib; }           // calib loaded? 
  Bool_t IsScaler(){ return fIsScaler; }           // any scalers?
  Bool_t IsEnergy(){ return fIsEnergy; }
  Bool_t IsTime(){ return fIsTime; }
  Bool_t IsPos(){ return fIsPos; }
  Bool_t IsRawHits(){ return fIsRawHits; }
 
  void SetEnergyScale(Double_t s) { fEnergyScale = s; }

  ClassDef(TA2Detector,1)
};

//---------------------------------------------------------------------------
inline void TA2Detector::Decode( )
{
  // Do the basic decoding (e.g. ADC -> Energy)
  // and histogram basic decoded parameters

  DecodeBasic();
}

//---------------------------------------------------------------------------
inline void TA2Detector::DecodeSaved( )
{
  // Energy etc. info previously read from ROOT file into arrays
  // Transfer this info to in-class parameters and proceed with analysis

  ReadDecoded();
}


//---------------------------------------------------------------------------
inline void TA2Detector::DecodeBasic( )
{
  // Decode the ADC and TDC information from each element of the detector
  // Elements with ADC &/OR TDC info within a window cut are stored in the
  // fHits array
  // Callable only by TA2Detector or classes which inherit from it
  // Add multihit TDC decoding

  fNADChits = fNTDChits = fNhits = fNhitsAll = 0;
  if( fHitsM ){
    for( UInt_t m=0; m<fNMultihit; m++ ) fNhitsM[m] = 0;
  }
  UShort_t datum;
  Short_t tdatum;
  fTotalEnergy = 0.0;
  UInt_t j;
  HitD2A_t* elem;

  if( fIsBitPattern ){
    fBitPattern->Decode();
  }
  
  for(j=0; j<fNelem; j++){
    elem = fElement[j];
    if (elem->IsIgnored()) continue;
    if( fIsRawHits ){
      if( fIsEnergy ){
	datum = elem->GetRawADCValue();
	if( datum < (UShort_t)ENullStore )
	  fRawEnergyHits[fNADChits++] = j;
      }
      if( fIsTime ){
	tdatum = elem->GetRawTDCValue(); 
	if( (abs(tdatum) < (UShort_t)ENullStore) && ((UShort_t)tdatum != 0xffff) )
	  fRawTimeHits[fNTDChits++] = j;
      }
    }
    if( !elem->Calibrate() ) continue;
    fHits[fNhits] = j;
    if( fIsEnergy ){
      fTotalEnergy += elem->GetEnergy();
      fEnergyOR[fNhits] = elem->GetEnergy();
    }
    if( fIsTime ){
      fTimeOR[fNhits] = fTime[j];
      if( fTimeM ){
	for( Int_t m=0; m<elem->GetNhit(); m++ ){
	  fHitsM[m][fNhitsM[m]] = j;
	  fTimeORM[m][fNhitsM[m]] = fTimeM[m][j];
	  fNhitsM[m]++;
	  fTimeAll[fNhitsAll++] = fTimeM[m][j];
	}
      }
      else fTimeAll[fNhitsAll++] = fTime[j];
    }
    fNhits++;
  }
  fHits[fNhits] = EBufferEnd;
  if( fIsTime ){
    fTimeOR[fNhits] = EBufferEnd;
    fTimeAll[fNhitsAll] = EBufferEnd;
    if( fIsRawHits ) fRawTimeHits[fNTDChits] = EBufferEnd;
    for(UInt_t m=0; m<fNMultihit; m++){
      fHitsM[m][fNhitsM[m]] = EBufferEnd;
      fTimeORM[m][fNhitsM[m]] = EBufferEnd;
    }
  }
  if( fIsEnergy ){
    fEnergyOR[fNhits] = EBufferEnd;
    if( fIsRawHits ) fRawEnergyHits[fNADChits] = EBufferEnd;
  }
}


//---------------------------------------------------------------------------
inline void TA2Detector::Cleanup( )
{
  // basic end-of-event resetting of Energy/Time/Position parameters 
  // of any elements recording a hit

  // K.Livingston...replace energy/time flush
  //  UInt_t i,j;
  //  for( i=0; i<fNhits; i++ ){
  //    j = fHits[i];
  //    if( fIsEnergy ) fEnergy[j] = (Double_t)ENullHit;
  //    if( fIsTime ) fTime[j] = (Double_t)ENullHit;
  //  }
  UInt_t i;
  for( i=0; i<fNelement; i++ ){
    if( fIsEnergy ) fEnergy[i] = (Double_t)ENullHit;
    if( fIsTime ) fTime[i] = (Double_t)ENullHit;
  }

  if( fTimeM ){
    for( UInt_t m=0; m<fNMultihit; m++ ){
      // Similar for multi-hit
      //      j = fNhitsM[m];
      //      for( i=0; i<j; i++ ) fTimeM[m][fHitsM[m][i]] = ENullHit;
      for( i=0; i<fNelement; i++ ) fTimeM[m][i] = ENullHit;
    }
  }
}


#endif
