/*************************************************************************
 * Author: Dominik Werthmueller, 2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2Detector                                                         //
//                                                                      //
// This class offers some methods for the handling of A2 detector       //
// calibration files.                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2Detector
#define OSCAR_TOA2Detector

#include "TObject.h"

// define A2 detectors
enum EA2Detector {
    kNoDetector,            // no detector
    kCBDetector,            // Crystal Ball
    kTAPSDetector           // TAPS
};
typedef EA2Detector A2Detector_t;

// define TAPS types
enum EA2TAPSType {
    kTAPS_2007,             // TAPS 2007 with 384 BaF2
    kTAPS_2008,             // TAPS 2008 with 378 BaF2 + 24 PbWO4
    kTAPS_2009              // TAPS 2009 with 366 BaF2 + 72 PbWO4
};
typedef EA2TAPSType A2TAPSType_t;

class TOA2Detector : public TObject
{

public:
    TOA2Detector();
    virtual ~TOA2Detector();
    
    static Bool_t IsCBHole(Int_t elem);
    static Int_t GetVetoInFrontOfElement(Int_t id, A2TAPSType_t type);
    static Int_t GetTAPSRing(Int_t id, A2TAPSType_t type);
    static Int_t GetTAPSBlock(Int_t id, A2TAPSType_t type);
    static Bool_t IsTAPSPWO(Int_t id, A2TAPSType_t type);

    ClassDef(TOA2Detector, 0)  // A2 detector class
};

#endif

