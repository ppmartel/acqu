#ifndef __TA2PairSpec_h__
#define __TA2PairSpec_h__

#include "TA2Detector.h"            // Acqu-Root histogrammer
#include "TA2Tagger.h"
#include "TA2Ladder.h"

class TA2PairSpec : public TA2Detector
{
protected:
  TA2Ladder* fLadder;	// Ladder
private:
  UInt_t fNchannels;
  // To distinguish old/new FPD
  UInt_t fDay0; 
  UInt_t fTimeStamp;
  Bool_t isOldFPD;
  
  UInt_t* fScalerOpen;
  UInt_t* fScalerGated;    // Gated with PairSpec
  UInt_t* fScalerGatedDly; // Gated, but out of coincidence with PairSpec
  
  Double_t* fScalerSumOpen;
  Double_t* fScalerSumGated;    // Gated with PairSpec
  Double_t* fScalerSumGatedDly; // Gated, but out of coincidence with PairSpec

public:
  TA2PairSpec( const char*, TA2System* );  // pass ptr to analyser
  virtual ~TA2PairSpec();                  // destructor
  virtual void PostInit();                    // some setup after parms read in
  virtual void LoadVariable();              // display setup
  virtual void Decode();                // event by event analysis
  virtual void Cleanup() {}                     // reset at end of event
  virtual void SaveDecoded() {}
  virtual void ReadDecoded() {}
 
  // Root needs this line for incorporation in dictionary
  ClassDef(TA2PairSpec,1)
};

#endif
