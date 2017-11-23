/*************************************************************************
 * Author: Dominik Werthmueller, 2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2PSA                                                              //
//                                                                      //
// Pulse-shape analysis for particles in TAPS.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2PSA
#define OSCAR_TOA2PSA

#include "TObject.h"

class TGraph;
class TOA2BaseDetParticle;

class TOA2PSA : public TObject
{

protected:
    TGraph** fPhotonMean;           // graphs of mean positions of photon band
    TGraph** fPhotonSigma;          // graphs of sigmas of photon band
    Double_t fPhotonSigmaFactor;    // sigma factor for photons
    Double_t fNuclExclMin;          // minimum radius value of PSA nucleon exclusion zone
    Double_t fNuclExclMax;          // maximum radius value of PSA nucleon exclusion zone
    Bool_t fIsQuiet;                // flag for quiet mode

    static const Int_t fgMaxSize;

public:
    TOA2PSA() : TObject(),
                fPhotonMean(0), fPhotonSigma(0), fPhotonSigmaFactor(0),
                fNuclExclMin(0), fNuclExclMax(0), fIsQuiet(kFALSE) { }
    TOA2PSA(const Char_t* file, Double_t sigma, 
            Double_t nuclExcMin, Double_t nuclExcMax, Bool_t quiet = kFALSE);
    TOA2PSA(const Char_t* file, Bool_t quiet = kFALSE);
    TOA2PSA(Double_t sigma, Double_t nuclExcMin, Double_t nuclExcMax, Bool_t quiet = kFALSE);
    virtual ~TOA2PSA();
    
    void LoadPSAData(const Char_t* file);
    void SetPhotonSigmaFactor(Double_t f) { fPhotonSigmaFactor = f; }
    void SetNucleonExclZone(Double_t rmin, Double_t rmax)
    {
        fNuclExclMin = rmin;
        fNuclExclMax = rmax;
    }

    Bool_t IsPSAPhoton(TOA2BaseDetParticle* p);
    Bool_t IsPSANucleon(TOA2BaseDetParticle* p, Bool_t useExclZone = kTRUE);

    ClassDef(TOA2PSA, 1)  // PSA for particles in TAPS
};

#endif

