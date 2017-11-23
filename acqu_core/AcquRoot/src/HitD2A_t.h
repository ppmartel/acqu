//--Author	JRM Annand    2nd Oct 2003
//--Rev 	JRM Annand...12th Dec 2003 Multi-hit ADC's
//--Rev 	JRM Annand...18th Feb 2004 D2AM_t for Multi-hit ADC's
//--Rev 	JRM Annand...27th May 2004 Time walk
//--Rev 	JRM Annand...21st Oct 2005 Getter for cuts, global energy scale
//--Rev 	JRM Annand....8th Sep 2006 SetWalk check mem leak
//--Rev 	JRM Annand...30th Oct 2008 Multihit TDCs
//--Rev 	JRM Annand...29th Nov 2008 Remove D2A_t and Cut_t
//--Rev 	JRM Annand... 3rd Dec 2008 Time walk options
//--Rev 	JRM Annand... 1st Sep 2009 constructer no incr nelem
//--Rev 	JRM Annand...11th Oct 2012 add time over threshold
//--Rev 	JRM Annand....8th Nov 2012 init fA2, fT2 zero
//--Rev 	JRM Annand...27th Mar 2013 Incorporate Basle mods
//--Update	JRM Annand...23rd Apr 2013 Ensure all updates incorporated
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// HitD2A_t
// Digital-to-analogue conversion for a single detector element
// The element may have amplitude (ADC) and time (TDC) digitised
// readout. Window cuts are applied
//

#ifndef __THitD2A_h__
#define __THitD2A_h__

#include "EnumConst.h"
#include "TimeWalk_t.h"

class TA2Detector;

class HitD2A_t{
private:
  // ADC stuff
  Double_t fA0;                     // pedestal
  Double_t fA1;                     // conversion gain
  Double_t fA2;                     // quadratic term?
  UShort_t* fADC;                   // -> stored ADC value
  Bool_t fIsMultiADC;               // multi-hit or not
  Double_t fEnergyLowThr;           // valid energy window
  Double_t fEnergyHighThr;
  // TDC stuff
  Double_t fT0;                     // Time-zero offset
  Double_t fT1;                     // conversion gain
  Double_t fT2;                     // quadratic term?
  UShort_t* fTDC;                   // -> stored TDC value
  UShort_t** fTDCM;                 // -> stored multihit TDC value
  Bool_t fIsMultiTDC;               // multi-hit or not
  Double_t fTimeLowThr;             // valid time window
  Double_t fTimeHighThr;
  // Time over threshold
  Double_t fToThr0;                 // offset
  Double_t fToThr1;                 // conversion gain
  UShort_t* fTDCtothr;              // -> TDC value
  //
  TimeWalk_t* fWalk;            // time walk correction
  Double_t* fEnergy;            // energy MeV
  Double_t* fTime;              // time ns
  Double_t** fTimeM;            // multihit time ns  
  Double_t fTimeOvThr; 		// pulse time over threshold
  Double_t fEnergyScale; 	// global energy scale factor
  Int_t fMode;                  // ADC, TDC, ADC+TDC
  Int_t fNMultihit;             // analysis of multiple hits in TDC
  Int_t fNhit;                  // # TDC hits
  Int_t fIadc;                  // ADC index
  Int_t fItdc;                  // TDC index
  Bool_t fIsIgnored;            // ignore this element
  Double_t* fEnergyScalePtr;    // DO NOT MOVE THIS DECLARATION!
public:
  HitD2A_t( char*, UInt_t, TA2Detector*, Bool_t ignorePosition = kFALSE );
  virtual ~HitD2A_t();
  Bool_t Calibrate();
  Bool_t CheckEnergy();
  Bool_t CheckTime();
  Bool_t CheckToThr(Double_t);
  UShort_t* GetRawADC(){ return fADC; }
  UShort_t* GetRawTDC(){ return fTDC; }
  UShort_t GetRawADCValue(){ return *fADC; }
  UShort_t GetRawTDCValue(){ return *fTDC; }
  Double_t GetEnergy(){ return *fEnergy; }
  Double_t GetTime(){ return *fTime; }
  Double_t** GetTimeM(){ return fTimeM; }
  Double_t* GetTimeM(Int_t m){ return fTimeM[m]; }
  Double_t GetEnergyLowThr(){ return fEnergyLowThr; }
  Double_t GetEnergyHighThr(){ return fEnergyHighThr; }
  Double_t GetTimeLowThr(){ return fTimeLowThr; }
  Double_t GetTimeHighThr(){ return fTimeHighThr; }
  Double_t GetTimeOvThr(){ return fTimeOvThr; }
  Double_t GetA0() { return fA0; }
  Double_t GetA1() { return fA1; }
  Double_t GetA2() { return fA2; }
  Double_t GetT0() { return fT0; }
  Double_t GetT1() { return fT1; }
  Double_t GetT2() { return fT2; }
  TimeWalk_t* GetTimeWalk() { return fWalk; }
  Bool_t IsIgnored() { return fIsIgnored; }
  void SetIgnored(Bool_t b) { fIsIgnored = b; }
  void SetWalk( Char_t* );
  void SetWalk( Double_t, Double_t );
  void SetToThr( Char_t*, TA2Detector* );
  void SetEnergyLowThr(Double_t p) { fEnergyLowThr = p; }
  void SetEnergyHighThr(Double_t p) { fEnergyHighThr = p; }
  void SetTimeLowThr(Double_t p) { fTimeLowThr = p; }
  void SetTimeHighThr(Double_t p) { fTimeHighThr = p; }
  void SetA0(Double_t p) { fA0 = p; }
  void SetA1(Double_t p) { fA1 = p; }
  void SetA2(Double_t p) { fA2 = p; }
  void SetT0(Double_t p) { fT0 = p; }
  void SetT1(Double_t p) { fT1 = p; }
  void SetT2(Double_t p) { fT2 = p; }
  Int_t GetMode(){ return fMode; }
  Int_t GetNMultihit(){ return fNMultihit; }
  Int_t GetNhit(){ return fNhit; }
  Int_t GetIadc(){ return fIadc; }
  Int_t GetItdc(){ return fItdc; }
};

//---------------------------------------------------------------------------
inline Bool_t HitD2A_t::Calibrate( )
{
  // Convert digital ADC & TDC to energy, time
  // Apply window cuts and return true/false to cut decision
  // Apply global energy scale factor (default 1.0)
  // Return logical result of testing within lower/upper limits

  //  Double_t energy;

  switch( fMode ){
  case 2:
    // TDC's only
    return CheckTime();
  case 3:
    // ADC and TDC
    if( CheckEnergy() )
      return CheckTime();
    else return EFalse;
  case 1:
    // ADC's only
    return CheckEnergy();
  default:
    return EFalse;
  }
}

//---------------------------------------------------------------------------
inline Bool_t HitD2A_t::CheckEnergy( )
{
  // Convert ADC channel to energy
  // Check 1st if ADC has valid channel contents
  // Check 2nd if energy is within bounds
  // Return TRUE if OK
  //
  Double_t energy,chan;
  if (!fIsMultiADC) {
    if( *fADC == (UShort_t)ENullADC ) return EFalse;   // SADC
    chan = (Double_t)(*fADC);
  }
  else {
    Short_t* madc = (Short_t*)fADC;
    if( *madc == (Short_t)ENullStore ) return EFalse; // standard QDC
    chan = (Double_t)(*madc);
  }
  chan -= fA0;                                   // pedestal subtract
  energy = fA1*chan;                             // linear scale
  if( fA2 ) energy += fA2*chan*chan;             // quadratic correction
  fEnergyScale = *fEnergyScalePtr;
  energy *= fEnergyScale;                        // global scale factor
  // check ADC thr.
  if( (energy < fEnergyLowThr) || (energy > fEnergyHighThr) ) return EFalse;
  *fEnergy = energy;
  return ETrue;
}

//---------------------------------------------------------------------------
inline Bool_t HitD2A_t::CheckTime( )
{
  // Convert TDC channel to time
  // Check 1st if TDC has valid channel contents
  // Check 2nd if time is within bounds
  // Return TRUE if OK
  // This one will analyse multiple hits from a TDC if fNMulihit > 0
  fNhit = 0;
  Double_t chan, time;
  // Single hit TDC (or analyse only 1 hit from multihit TDC)
  if( !fNMultihit ){
    if( !fIsMultiTDC ){
      if( *fTDC ==  (UShort_t)ENullADC ) return EFalse; // channel not defined
      chan = (Double_t)(*fTDC);
    }
    else{
      Short_t* madc = (Short_t*)fTDC;
      if( *madc == (Short_t)ENullStore ) return EFalse;// channel not defined
      chan = (Double_t)(*madc);
    }
    Double_t energyW = 0.0;
    if( fTDCtothr ){
      if( CheckToThr(chan) ) energyW = fTimeOvThr;  // time over threshold?
    }
    else if( fADC ) energyW = *fEnergy;             // otherwise pulse height
    chan -= fT0;                                    // subtract offset
    time = fT1*chan;                                // linear scale
    if( fT2 ) time += fT2*chan*chan;                // quadratic correction
    if( fWalk ) time = fWalk->Tcorr( energyW, time ); // walk correction
    if( (time < fTimeLowThr) || (time > fTimeHighThr) ){
      *fTime = (Double_t)ENullADC;          // not inside window
      return EFalse;
    }
    else{
      *fTime = time;
      return kTRUE;
    }
  }
  // Multi-hit TDC  
  for( Int_t m=0; m<fNMultihit; m++ ){
    Short_t* madc = (Short_t*)fTDCM[m];
    if( *madc == (Short_t)ENullStore ) break;  // channel not defined
    chan = (Double_t)(*madc);
    chan -= fT0;                               // subtract offset
    time = fT1*chan;                           // linear scale
    if( fT2 ) time += fT2*chan*chan;           // quadratic correction
    if( fWalk ) time = fWalk->Tcorr( *fEnergy, time ); // walk correction
    if( (time < fTimeLowThr) || (time > fTimeHighThr) )
      *fTimeM[m] = (Double_t)ENullADC;
    else{
      if( !fNhit ) *fTime = time;             // time from 1st hit
      *fTimeM[fNhit] = time;                      // store all hit times
      fNhit++;                                // log # hits
    }
  }
  if( fNhit )
    return kTRUE;
  else{
    *fTime = (Double_t)ENullADC;
    return kFALSE;
  }
}

inline Bool_t HitD2A_t::CheckToThr(Double_t t)
{
  // Get time over threshold
  // Single hit TDC (or analyse only 1 hit from multihit TDC)
  Double_t tothr;
  if( !fIsMultiTDC){
    if( *fTDCtothr == (UShort_t)ENullADC ) return kFALSE; // channel not def.
    tothr = (Double_t)(*fTDCtothr);
  }
  else{
    Short_t* madc = (Short_t*)fTDCtothr;
    if( *madc == (Short_t)ENullStore ) return kFALSE; // channel not def.
    tothr = (Double_t)(*madc);
  }
  tothr -= t; // trailing edge - leading edge
  tothr -= fToThr0; // subtract offset
  fTimeOvThr = fToThr1*tothr; // convertion gain
  return kTRUE;
}

#endif
