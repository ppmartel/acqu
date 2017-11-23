//--Author	JRM Annand        12th Feb 2003
//--Rev 	JRM Annand...     26th Feb 2003...1st production version
//--Rev	        JRM Annand...      5th Feb 2004...3v8 compatibility
//--Rev 	K   Livingston... 20th Apr 2004...changed fLadder to fHits
//--Rev 	K   Livingston... 20th Apr 2004...PostInit() init fLaddDoubles
//--Rev 	K   Livingston... 16th Jun 2004...Prompt and rand timing wind
//--Rev 	K   Livingston... 25th Apr 2005...Multi random windows 10 max
//--Rev 	JRM Annand...     16th May 2005...ReadDecoded for Monter Carlo
//--Rev  	JRM Annand...     25th Jul 2005...SetConfig hierarchy
//--Rev 	JRM Annand...     26th Oct 2006...Adapt for TA2Detector upgrade
//--Rev 	JRM Annand...     17th Sep 2008...ReadDecoded & Decode doubles
//--Rev 	JRM Annand...     26th Aug 2009...Init fMuHits NULL
//--Rev 	JRM Annand...      1st Sep 2009...delete[], new HitD2A_t
//--Rev  	JRM Annand...     22nd Dec 2009...fDoubles MC
//--Update 	JRM Annand...      5th Sep 2012...ReadDecoded check iHit sensible
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Ladder
//
// "Linear" array of simple detector elements which provide hit
// timing and rates info. The hit-position is related to an energy scale
// e.g. as in focal-plane plastic scintillator array
// of the Glasgow-Mainz photon tagger
// Expect this to be a high-rate device...so multiple hits.
//
// In general, a particle may go through a single element, or 2 overlapping 
// elements. Elements are defined as for TA2Detector, but have extra setup
// parameters relating to energy calibration and scalers.
// An element can have any of the following:
// QDC, TDC, Position, Energy, OverlapEnergy, Scaler
// For details, see the source code and the setupfile.

#ifndef __TA2Ladder_h__
#define __TA2Ladder_h__

#include "TA2Detector.h"        // basic detector
#include "D2A_t.h"              // digital-to-analogue conversion
#include "Cut_t.h"              // basic data cuts
#include "MCBranchID.h"         // GEANT-3 output format defs

enum {		//define whether hits in is prompt or rand window
  ELaddAny,
  ELaddPrompt,
  ELaddRand
};

enum { EMaxRandWindows = 16 };


class TA2Ladder : public TA2Detector {

 private:
 protected:
  HitD2A_t** fTrigger;         	// Trigger info array (only if separate trig)
  UInt_t* fTrigg;              	// Trigger info for each each hit
  UInt_t* fDoubles;            	// double-hit info
  UInt_t* fHitsAll;            	// before sorting doubles
  UInt_t* fWindows;	       	// for each hit, flag prompt/rand/neither
  Int_t* fHitsRand;		// array with indices of random hit channls 
  Int_t* fHitsPrompt;		// array with indices of prompt hit channels
  Int_t* fMuHits;               // microscope hits
  Double_t* fECalibration;     	// Energy calibration
  Double_t* fEWidth;            // Energy width of channel
  Double_t* fEelec;             // electron energy
  Double_t* fEelecOR;           // OR of electron energy
  Double_t* fMeanEelecOR;       // OR of overlap electron energy
  Double_t* fEOverlap;         	// Energy calibration for overlaps
  Double_t* fRandMin;	       	// random windows maxima
  Double_t* fRandMax;           // random windows minima
  Double_t* fTimeORAll;         // before sorting doubles
  Double_t* fMeanTime;          // doubles mean time
  Double_t* fMeanTimeOR;        // doubles mean time OR
  Double_t* fMeanEnergy;        // doubles mean pulse height
  Double_t* fMeanEnergyOR;      // doubles mean pulse height OR
  Double_t* fDiffTime;          // doubles time difference
  Double_t* fDiffTimeOR;        // doubles time difference OR
  Double_t fTimingRes;         	// Timing resolution (window) for doubles
  Double_t fPromptRand;        	// Ratio of prompt/rand windows
  Double_t fPromptMin;	       	// Prompt window minimum
  Double_t fPromptMax;          // Prompt window maximum
  UInt_t fNDoubles;           	// no of doubles
  UInt_t fNThits;              	// No. triggers in event
  UInt_t fNtrigger;            	// No. separate triggers defined
  UInt_t fNtrig;               	// Separate triggers running counter
  UInt_t fNRandWindows;		// No of random windows defined
  UInt_t fNhitsPrompt;		// no of prompt hits
  UInt_t fNhitsRand;		// no of random hits
  UInt_t fNhitsAll;             // # hits before sorting doubles
  UInt_t fNMuHits;              // # microscope hits
  UInt_t fNMuElem;              // # microscope overlap channels
  UInt_t fFence;		// Fence plot for microscope calibration
  Bool_t fIsOverlap;           	// Is different energy to be used for overlaps
  Bool_t fIsTimeWindows;	// Are prompt and rand windows defined
  Bool_t fIsFence;              // Fence plot?
  Bool_t fIsMicro;              // Microscope 1/3 overlap decode?
  
public:
  TA2Ladder( const char*, TA2System* );// Standard detector-type constructer
  virtual ~TA2Ladder();
  virtual void SetConfig( char*, int );// decode and load setup info
  virtual void LoadVariable( );        // display setup
  virtual void PostInit( );            // initialise using setup info
  virtual void Decode( );              // hits -> energy procedure
  virtual void Cleanup( );             // end-of-event cleanup
  virtual void SaveDecoded( );         // save local analysis
  virtual void DecodeDoubles( );       // decode single/double hits
  virtual void DecodeMicro( );         // decode 1/3 overlap strips
  virtual void DecodePrRand( );        // decode prompt and random coincidences
  virtual UInt_t Fence( );             // fence plot for microscope calibration
  virtual void ReadDecoded( );         // read back previous analysis
  // Getters for read-only variables
  HitD2A_t** GetTrigger(){ return fTrigger; }
  UInt_t* GetTrigg(){ return fTrigg; }
  UInt_t* GetDoubles(){ return fDoubles; }
  UInt_t* GetHitsAll(){ return fHitsAll; }
  UInt_t* GetWindows(){ return fWindows; }
  Int_t* GetHitsRand(){ return fHitsRand; }
  Int_t* GetHitsPrompt(){ return fHitsPrompt; }
  Int_t* GetMuHits(){ return fMuHits; }
  const Double_t* GetECalibration(){ return (const Double_t*)fECalibration; }
  Double_t* GetEWidth(){ return fEWidth; }
  Double_t* GetEelec(){ return fEelec; }
  Double_t* GetEelecOR(){ return fEelecOR; }
  Double_t* GetMeanEelecOR(){ return fMeanEelecOR; }
  Double_t* GetEOverlap(){ return fEOverlap; }
  Double_t* GetRandMin(){ return fRandMin; }
  Double_t* GetRandMax(){ return fRandMax; }
  Double_t* GetTimeORAll(){ return fTimeORAll; }
  Double_t* GetMeanTime(){ return fMeanTime; }
  Double_t* GetMeanTimeOR(){ return fMeanTimeOR; }
  Double_t* GetMeanEnergy(){ return fMeanEnergy; }
  Double_t* GetMeanEnergyOR(){ return fMeanEnergyOR; }
  Double_t* GetDiffTime(){ return fDiffTime; }
  Double_t* GetDiffTimeOR(){ return fDiffTimeOR; }
  Double_t GetTimingRes(){ return fTimingRes; }
  Double_t GetPromptRand(){ return fPromptRand; }
  Double_t GetPromptMin(){ return fPromptMin; }
  Double_t GetPromptMax(){ return fPromptMax; }
  UInt_t GetNDoubles(){ return fNDoubles; }
  UInt_t GetNThits(){ return fNThits; }
  UInt_t GetNtrigger(){ return fNtrigger; }
  UInt_t GetNtrig(){ return fNtrig; }
  UInt_t GetNRandWindows(){ return fNRandWindows; }
  UInt_t GetNhitsPrompt(){ return fNhitsPrompt; }
  UInt_t GetNhitsRand(){ return fNhitsRand; }
  UInt_t GetNhitsAll(){ return fNhitsAll; }
  UInt_t GetFence(){ return fFence; }
  UInt_t GetNMuHits(){ return fNMuHits; }
  UInt_t GetNMuElem(){ return fNMuElem; }
  Bool_t IsOverlap(){ return fIsOverlap; }
  Bool_t IsTimeWindows(){return fIsTimeWindows;}
  Bool_t IsFence(){ return fIsFence; }
  Bool_t IsMicro(){ return fIsMicro; }

  
  UInt_t* fScalerPromptIndex;            // local prompt scalers indices
  UInt_t* fScalerPromptCurr;             // local prompt scalers current buffer
  Double_t* fScalerPromptAcc;            // local prompt scalers accumulated buffer
  UInt_t* fScalerRandIndex;              // local rand scalers indices
  UInt_t* fScalerRandCurr;               // local rand scalers current buffer
  Double_t* fScalerRandAcc;              // local rand scalers accumulated buffer
  
  UInt_t* GetScalerPromptIndex(){ return fScalerPromptIndex; }       // local prompt scalers
  UInt_t* GetScalerPromptCurr(){ return fScalerPromptCurr; }         // local prompt scalers
  Double_t* GetScalerPromptAcc(){ return fScalerPromptAcc; }         // local prompt scalers
  UInt_t* GetScalerRandIndex(){ return fScalerRandIndex; }       // local rand scalers
  UInt_t* GetScalerRandCurr(){ return fScalerRandCurr; }         // local rand scalers
  Double_t* GetScalerRandAcc(){ return fScalerRandAcc; }         // local rand scalers
  
  ClassDef(TA2Ladder,1)
};

//---------------------------------------------------------------------------
inline void TA2Ladder::Cleanup( )
{
  TA2Detector::Cleanup();
  UInt_t i,j;
  for( i=0; i<fNDoubles; i++ ){
    j = fDoubles[i];
    if( fIsEnergy ) fMeanEnergy[j] = (Double_t)ENullHit;
    if( fIsTime ){
      fMeanTime[j] = (Double_t)ENullHit;
      fDiffTime[j] = (Double_t)ENullHit;
    }
  }
  if( fMuHits ) fMuHits[0] = EBufferEnd;
}

//---------------------------------------------------------------------------
inline void TA2Ladder::DecodeDoubles( )
{
  // Now loop round and work out doubles, if required
  // loop over all hits searching for adjacent hits
  // check hit pair are within time coincidence bounds
  if( fIsTime && fIsOverlap ){         	        //are we handling overlaps
    fNDoubles = 0;
    Int_t ngood = 0;
    Int_t j;
    Double_t tdiff;
    for(UInt_t i=0; i<fNhits;){             // loop all hits
      j = fHitsAll[i];
      // Look for a chain of coincident hits
      UInt_t k,ik,jk;
      for( k=1;; k++ ){
	jk = j+k;
	if( (ik = i+k) >= fNhits ) break;
	if( jk != fHitsAll[ik]) break;
	tdiff = fTime[j]-fTime[jk];
	if( fabs(tdiff) >= fTimingRes ) break;
      }
      // Isolated single-channel hit
      if( k == 1 ){
	fHits[ngood] = j;
	if(fIsECalib) fEelecOR[ngood] = fECalibration[j];
	fTimeOR[ngood] = fTime[j];
	if( fIsEnergy ) fEnergyOR[ngood] = fEnergy[j];
	ngood++;
      }
      // Coincident double-channel hit
      else if( k == 2 ){
	jk--; 
	fHits[ngood] = j;       //save hit
	fTimeOR[ngood] = fTime[j];
	if( fIsEnergy ) fEnergyOR[ngood] = fEnergy[j];
	fDoubles[fNDoubles] = j;              //save index of double
	// Save means of time, pulse height and cal electron energy
	fMeanTime[j] = 0.5 * (fTime[j] + fTime[jk]);
	fMeanTimeOR[fNDoubles] = fMeanTime[j];
	fDiffTimeOR[fNDoubles] = fDiffTime[j] = tdiff;
	if(fIsEnergy){
	  fMeanEnergy[j] = 0.5 * (fEnergy[j] + fEnergy[jk]);
	  fMeanEnergyOR[fNDoubles] = fMeanEnergy[j];
	}
	if(fIsECalib)
	  fEelecOR[ngood] = fMeanEelecOR[fNDoubles] = fEOverlap[j];
	fNDoubles++;
        ngood++;
      }
      // >2 adjacent hits....clear hit and energy/time element
      else{
	for( UInt_t m=i; m<ik; m++ ){
	  UInt_t m1 = fHitsAll[m];
	  fTime[m1] = (Double_t)ENullHit;
	  if( fIsEnergy ) fEnergy[m1] = (Double_t)ENullHit;
	}
      }
      i = ik;
    }
    fNhits = ngood;
    fHits[ngood] = EBufferEnd;
    fTimeOR[ngood] = EBufferEnd;
    if( fIsEnergy ) fEnergyOR[ngood] = fEnergy[j];
    fDoubles[fNDoubles] = EBufferEnd;
    fMeanTimeOR[fNDoubles] = EBufferEnd;
    fDiffTimeOR[fNDoubles] = EBufferEnd;
    if(fIsEnergy){
      fMeanEnergyOR[fNDoubles] = EBufferEnd;
    }
    if(fIsECalib){
      fEelecOR[ngood] = EBufferEnd;
      fMeanEelecOR[fNDoubles] = EBufferEnd;
    }
  }
}


//---------------------------------------------------------------------------
inline void TA2Ladder::DecodeMicro( )
{
  // Decode 1/3 overlapping scintillator strips of the FP microscope
  if( !fIsMicro ) return;
  UInt_t i,j,k,l,m;
  fNMuHits = 0;
  // loop over "standard" hits
  for( i=0; i<fNhits; i++ ){
    j = fHits[i];
    k = fHits[i+1];
    if( k == (j+1) ){
      //      Double_t t = TMath::Abs( fTime[j] - fTime[k] );
      //      if( t < fTimingRes ) 
      l = 2*j +1;      // adjacent double, assumed overlap
      //      else continue;
    }
    else l = 2*j;                     // isolated single hit
    if( i ){
      m = fHits[i-1];
      if( m == (j-1) ) continue;
    }
    fMuHits[fNMuHits] = l;
    fNMuHits++;
  }
  fMuHits[fNMuHits] = EBufferEnd;    // end markers in buffers
}

//---------------------------------------------------------------------------
inline void TA2Ladder::DecodePrRand( )
{
  // Compare hit times with prompt and random bounds
  // Flag whether prompt or rand or neither
  if( fIsTime && fIsTimeWindows ){
    fNhitsPrompt = 0;
    fNhitsRand = 0;
    //loop all hits looking for prompt or rand
    for(UInt_t i=0; i<fNhits; i++){
      fWindows[i] = ELaddAny;	              //default neither prompt/random
      if( (fTimeOR[i] > fPromptMin) && (fTimeOR[i] <= fPromptMax)){
	fWindows[i] = ELaddPrompt;   	      // indicate it was in prompt
	fHitsPrompt[fNhitsPrompt] = fHits[i]; // store prompt hit
	fNhitsPrompt++;
      }
      else{
	for(UInt_t n=0; n<fNRandWindows; n++){  //check all random windows
	  if( (fTimeOR[i] > fRandMin[n]) && (fTimeOR[i] <= fRandMax[n]) ){
	    fWindows[i] = ELaddRand+n;          // indicate in rand window n
	    fHitsRand[fNhitsRand] = fHits[i];   // store random hit
	    fNhitsRand++;
	    break;                              // exit after 1st rand sampled
	  }
	}
      }
    }
    fWindows[fNhits]=EBufferEnd;
    fHitsPrompt[fNhitsPrompt] = EBufferEnd;
    fHitsRand[fNhitsRand] = EBufferEnd;
  }
}

//---------------------------------------------------------------------------
inline UInt_t TA2Ladder::Fence( )
{
  // Dirk Krambrich's "fence" plot for microscope calibration
  switch (fNhits){
  case 1:
    return (fHits[0] * 2);
  case 2:
    if (fHits[1] == fHits[0] + 1) return ((fHits[0] * 2) + 1);
    else                          return (UInt_t)ENullADC;
  default:
    return (UInt_t)ENullADC;
  }
}


#endif
