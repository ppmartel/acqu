// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCConfig                                                             //
//                                                                      //
// CaLib configuration namespace                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCONFIG_H
#define TCCONFIG_H

#include "Rtypes.h"


// detector enumeration
enum ECalibDetector
{
    kDETECTOR_NODET = 0,
    kDETECTOR_TAGG,
    kDETECTOR_CB,
    kDETECTOR_TAPS,
    kDETECTOR_PID,
    kDETECTOR_VETO
};
typedef ECalibDetector CalibDetector_t;


namespace TCConfig
{   
    // detector elements
    extern const Int_t kMaxCrystal;
    extern const Int_t kMaxCB;
    extern const Int_t kMaxTAPS;
    extern const Int_t kMaxPID;
    extern const Int_t kMaxVeto;
    extern const Int_t kMaxTAGGER;
    
    // database format definitions
    extern const Char_t* kCalibMainTableName;
    extern const Char_t* kCalibMainTableFormat; 
    extern const Char_t* kCalibDataTableHeader;
    extern const Char_t* kCalibDataTableSettings;
     
    // version numbers etc.
    extern const Char_t kCaLibVersion[];
    extern const Int_t kContainerFormatVersion;
    extern const Char_t kCaLibDumpName[];

    // constants
    extern const Double_t kPi0Mass;
    extern const Double_t kEtaMass;
}

#endif

