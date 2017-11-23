/*************************************************************************
 * Author: Dominik Werthmueller, 2011
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2Trigger                                                          //
//                                                                      //
// This class represents the A2 hardware and software trigger.          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2Trigger
#define OSCAR_TOA2Trigger

#include "TObject.h"

// define L1 trigger bits
enum EA2TrigBitL1 {
    kCB = 0,
    kTAPS_OR,
    kTAPS_Pulser,
    kTAPS_M2,
    kPulser
};
typedef EA2TrigBitL1 A2TrigBitL1_t;

// define L2 trigger bits
enum EA2TrigBitL2 {
    kM1 = 0,
    kM2,
    kM3,
    kM4
};
typedef EA2TrigBitL2 A2TrigBitL2_t;

// define beam helicity bits
enum EA2BeamHelBit {
    kPos = 0,
    kNeg,
    kUndef
};
typedef EA2BeamHelBit A2BeamHelBit_t;

class TOA2Trigger : public TObject
{

protected:
    Char_t fPatternL1;                      // L1 trigger pattern
    Char_t fPatternL2;                      // L2 trigger pattern
    Char_t fBeamHelPattern;                 // beam helicity pattern
    Double_t fCBEnergySum;                  // CB energy sum (software trigger)
    Int_t fMultCB;                          // CB multiplicity (software trigger)
    Int_t fMultTAPSLED1;                    // TAPS LED1 multiplicity (software trigger)
    Int_t fMultTAPSLED2;                    // TAPS LED2 multiplicity (software trigger)

public:
    TOA2Trigger();
    virtual ~TOA2Trigger() { }
    
    void SetTriggerPatternL1(Char_t patt) { fPatternL1 = patt; }
    void SetTriggerPatternL2(Char_t patt) { fPatternL2 = patt; }
    void SetBeamHelPattern(Char_t patt) { fBeamHelPattern = patt; }

    void SetTriggerL1(A2TrigBitL1_t bit) { fPatternL1 |= (1 << bit); }
    void SetTriggerL2(A2TrigBitL2_t bit) { fPatternL2 |= (1 << bit); }
    void SetBeamHel(A2BeamHelBit_t bit) { fBeamHelPattern |= (1 << bit); }

    void SetCBEnergySum(Double_t e) { fCBEnergySum = e; }
    void SetMultCB(Int_t m) { fMultCB = m; }
    void SetMultTAPSLED1(Int_t m) { fMultTAPSLED1 = m; }
    void SetMultTAPSLED2(Int_t m) { fMultTAPSLED2 = m; }

    Bool_t IsTriggerL1(A2TrigBitL1_t bit) const { return fPatternL1 & (1 << bit); } 
    Bool_t IsTriggerL2(A2TrigBitL2_t bit) const { return fPatternL2 & (1 << bit); } 
    Bool_t IsBeamHel(A2BeamHelBit_t bit) const { return fBeamHelPattern & (1 << bit); } 

    Double_t GetCBEnergySum() const { return fCBEnergySum; }
    Int_t GetMultCB() const { return fMultCB; }
    Int_t GetMultTAPSLED1() const { return fMultTAPSLED1; }
    Int_t GetMultTAPSLED2() const { return fMultTAPSLED2; }

    void Reset();
    virtual void Print(Option_t* option = "") const;
    
    TOA2Trigger& operator=(const TOA2Trigger& t);

    ClassDef(TOA2Trigger, 1)  // A2 trigger
};

#endif

