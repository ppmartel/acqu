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


#include "TGraph.h"

#include "TOA2PSA.h"
#include "TOLoader.h"
#include "TOA2BaseDetParticle.h"


// init static class members
const Int_t TOA2PSA::fgMaxSize = 438;

//______________________________________________________________________________
TOA2PSA::TOA2PSA(const Char_t* file, Double_t sigma, 
                 Double_t nuclExcMin, Double_t nuclExcMax, Bool_t quiet)
    : TObject()
{
    // Constructor.
    //
    // Arguments:
    // 'file'       : ROOT file holding TGraphs of detector photon means and sigmas
    // 'sigma'      : sigma factor for photons
    // 'nuclExcMin' : minimum radius value of PSA nucleon exclusion zone
    // 'nuclExcMax' : maximum radius value of PSA nucleon exclusion zone
    // 'quiet'      : flag for quiet mode

    // init members
    fPhotonMean = new TGraph*[fgMaxSize];
    fPhotonSigma = new TGraph*[fgMaxSize];
    for (Int_t i = 0; i < fgMaxSize; i++)
    {
        fPhotonMean[i] = 0;
        fPhotonSigma[i] = 0;
    }
    fPhotonSigmaFactor = sigma;
    fNuclExclMin = nuclExcMin;
    fNuclExclMax = nuclExcMax;
    fIsQuiet = quiet;

    // load PSA data
    LoadPSAData(file);
}

//______________________________________________________________________________
TOA2PSA::TOA2PSA(const Char_t* file, Bool_t quiet)
    : TObject()
{
    // Constructor.
    //
    // Arguments:
    // 'file'       : ROOT file holding TGraphs of detector photon means and sigmas
    // 'quiet'      : flag for quiet mode
    
    // init members
    fPhotonMean = new TGraph*[fgMaxSize];
    fPhotonSigma = new TGraph*[fgMaxSize];
    for (Int_t i = 0; i < fgMaxSize; i++)
    {
        fPhotonMean[i] = 0;
        fPhotonSigma[i] = 0;
    }
    fPhotonSigmaFactor = 0;
    fNuclExclMin = 0;
    fNuclExclMax = 0;
    fIsQuiet = quiet;

    // load PSA data
    LoadPSAData(file);
}

//______________________________________________________________________________
TOA2PSA::TOA2PSA(Double_t sigma, Double_t nuclExcMin, Double_t nuclExcMax, Bool_t quiet)
    : TObject()
{
    // Constructor.
    //
    // Arguments:
    // 'sigma'      : sigma factor for photons
    // 'nuclExcMin' : minimum radius value of PSA nucleon exclusion zone
    // 'nuclExcMax' : maximum radius value of PSA nucleon exclusion zone
    // 'quiet'      : flag for quiet mode
    
    // init members
    fPhotonMean = new TGraph*[fgMaxSize];
    fPhotonSigma = new TGraph*[fgMaxSize];
    for (Int_t i = 0; i < fgMaxSize; i++)
    {
        fPhotonMean[i] = 0;
        fPhotonSigma[i] = 0;
    }
    fPhotonSigmaFactor = sigma;
    fNuclExclMin = nuclExcMin;
    fNuclExclMax = nuclExcMax;
    fIsQuiet = quiet;
}

//______________________________________________________________________________
TOA2PSA::~TOA2PSA()
{
    // Destructor.
    
    if (fPhotonMean)
    {
        for (Int_t i = 0; i < fgMaxSize; i++)
        {
            if (fPhotonMean[i]) delete fPhotonMean[i];
            if (fPhotonSigma[i]) delete fPhotonSigma[i];
        }
        delete [] fPhotonMean;
        delete [] fPhotonSigma;
    }
}

//______________________________________________________________________________
void TOA2PSA::LoadPSAData(const Char_t* file)
{
    // Load the PSA mean and sigma data from the file 'file'.
    
    Char_t tmp[256];

    Int_t nLoad = 0;
    for (Int_t i = 0; i < fgMaxSize; i++)
    {
        sprintf(tmp, "Mean_%03d", i);
        Bool_t ret1 = TOLoader::LoadObject(file, tmp, &fPhotonMean[i], "", "q");
        sprintf(tmp, "Sigma_%03d", i);
        Bool_t ret2 = TOLoader::LoadObject(file, tmp, &fPhotonSigma[i], "", "q");
        if (ret1 && ret2) nLoad++;
    }

    if (!fIsQuiet)
        Info("LoadPSAData", "Loaded PSA data for %d TAPS detectors", nLoad);
}

//______________________________________________________________________________ 
Bool_t TOA2PSA::IsPSAPhoton(TOA2BaseDetParticle* p)
{
    // Return kTRUE if the particle 'p' is considered as photon according to PSA:
    // PSA angle is larger than mean_gamma(r_psa) - fPhotonSigmaFactor*sigma_gamma(r_psa) 
    // and smaller than mean_gamma(r_psa) + fPhotonSigmaFactor*sigma_gamma(r_psa)
    // Return kFALSE otherwise.
    // Return kTRUE if the particle was detected in CB.
    // Return kTRUE if the corresponding PSA data were not found.

    // CB particles
    if (p->GetDetector() != kTAPSDetector) return kTRUE;

    // get PSA radius and angle
    Double_t psaR, psaA;
    if (!p->CalculatePSA(&psaR, &psaA))
    {
        if (!fIsQuiet)
            Error("IsPSAPhoton", "Could not get PSA information of the particle!");
        return kTRUE;
    }
    
    // get element index
    Int_t elem = p->GetCentralElement();

    // check if the needed PSA data were loaded
    if (!fPhotonMean[elem] || !fPhotonSigma[elem])
    {
        if (!fIsQuiet)
            Error("IsPSAPhoton", "No PSA data for detector %d found!", elem);
        return kTRUE;
    }

    // check for photon
    if (psaA > fPhotonMean[elem]->Eval(psaR) - fPhotonSigmaFactor*fPhotonSigma[elem]->Eval(psaR) &&
        psaA < fPhotonMean[elem]->Eval(psaR) + fPhotonSigmaFactor*fPhotonSigma[elem]->Eval(psaR))
        return kTRUE;
    else
        return kFALSE;
}

//______________________________________________________________________________ 
Bool_t TOA2PSA::IsPSANucleon(TOA2BaseDetParticle* p, Bool_t useExclZone)
{
    // Return kTRUE if the particle 'p' is considered as nucleon according to PSA:
    // For PSA radii between fNuclExclMin and fNuclExclMax the particle is always
    // considered as nucleon (punch-through) if 'useExclZone' is kTRUE.
    // For the other regions the PSA angle has to be smaller than
    // mean_gamma(r_psa) - fPhotonSigmaFactor*sigma_gamma(r_psa)
    // Return kFALSE otherwise.
    // Return kTRUE if the particle was detected in CB.
    // Return kTRUE if the corresponding PSA data were not found.

    // CB particles
    if (p->GetDetector() != kTAPSDetector) return kTRUE;

    // get PSA radius and angle
    Double_t psaR, psaA;
    if (!p->CalculatePSA(&psaR, &psaA))
    {
        if (!fIsQuiet)
            Error("IsPSANucleon", "Could not get PSA information of the particle!");
        return kTRUE;
    }

    // check if particle is in PSA nucleon exclusion zone
    if (useExclZone && psaR > fNuclExclMin && psaR < fNuclExclMax) return kTRUE;
    
    // get element index
    Int_t elem = p->GetCentralElement();

    // check if the needed PSA data were loaded
    if (!fPhotonMean[elem] || !fPhotonSigma[elem])
    {
        if (!fIsQuiet)
            Error("IsPSANucleon", "No PSA data for detector %d found!", elem);
        return kTRUE;
    }

    // check for nucleon
    if (psaA < fPhotonMean[elem]->Eval(psaR) - fPhotonSigmaFactor*fPhotonSigma[elem]->Eval(psaR))
        return kTRUE;
    else
        return kFALSE;
}
ClassImp(TOA2PSA)
