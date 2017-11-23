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


#include "TClass.h"

#include "TOA2Trigger.h"
#include "TOSUtils.h"


//______________________________________________________________________________
TOA2Trigger::TOA2Trigger()
    : TObject()
{
    // Default constructor.

    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();
    
    // init members
    Reset();
}

//______________________________________________________________________________
void TOA2Trigger::Reset()
{
    // Reset class content

    fPatternL1 = 0;
    fPatternL2 = 0;
    fBeamHelPattern = 0;
    fCBEnergySum = 0;
    fMultCB = 0;
    fMultTAPSLED1 = 0;
    fMultTAPSLED2 = 0;
}

//______________________________________________________________________________
void TOA2Trigger::Print(Option_t* option) const
{
    // Print the class content.
    
    printf("TOA2Trigger content:\n");
    printf("L1 pattern             : %s\n", TOSUtils::FormatBinary(fPatternL1, 8));
    printf("L1 CB                  : "); IsTriggerL1(kCB) ? printf("yes\n") : printf("no\n");
    printf("L1 TAPS OR             : "); IsTriggerL1(kTAPS_OR) ? printf("yes\n") : printf("no\n");
    printf("L1 TAPS Pulser         : "); IsTriggerL1(kTAPS_Pulser) ? printf("yes\n") : printf("no\n");
    printf("L1 TAPS M2             : "); IsTriggerL1(kTAPS_M2) ? printf("yes\n") : printf("no\n");
    printf("L1 Pulser              : "); IsTriggerL1(kPulser) ? printf("yes\n") : printf("no\n");
    
    printf("L2 pattern             : %s\n", TOSUtils::FormatBinary(fPatternL2, 8));
    printf("L2 M1                  : "); IsTriggerL2(kM1) ? printf("yes\n") : printf("no\n");
    printf("L2 M2                  : "); IsTriggerL2(kM2) ? printf("yes\n") : printf("no\n");
    printf("L2 M3                  : "); IsTriggerL2(kM3) ? printf("yes\n") : printf("no\n");
    printf("L2 M4                  : "); IsTriggerL2(kM4) ? printf("yes\n") : printf("no\n");
    
    printf("Beam helicity pattern  : %s\n", TOSUtils::FormatBinary(fBeamHelPattern, 8));
    printf("positive helicity      : "); IsBeamHel(kPos) ? printf("yes\n") : printf("no\n");
    printf("negative helicity      : "); IsBeamHel(kNeg) ? printf("yes\n") : printf("no\n");
    printf("undefined helicity     : "); IsBeamHel(kUndef) ? printf("yes\n") : printf("no\n");
    
    printf("CB energy sum          : %.2f\n", fCBEnergySum);
    printf("CB multiplicity        : %d\n", fMultCB);
    printf("TAPS LED1 multiplicity : %d\n", fMultTAPSLED1);
    printf("TAPS LED2 multiplicity : %d\n", fMultTAPSLED2);
}

//______________________________________________________________________________
TOA2Trigger& TOA2Trigger::operator=(const TOA2Trigger& t)
{
    // Assignment operator.
    
    // check self assignment
    if (this != &t)
    {
        fPatternL1 = t.fPatternL1;
        fPatternL2 = t.fPatternL2;
        fBeamHelPattern = t.fBeamHelPattern;
        fCBEnergySum = t.fCBEnergySum;
        fMultCB = t.fMultCB;
        fMultTAPSLED1 = t.fMultTAPSLED1;
        fMultTAPSLED2 = t.fMultTAPSLED2;
    }

    return *this;
}
ClassImp(TOA2Trigger)
