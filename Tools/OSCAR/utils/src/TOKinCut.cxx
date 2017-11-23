/*************************************************************************
 * Author: Dominik Werthmueller, 2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOKinCut                                                             //
//                                                                      //
// Class representing a kinematical cut.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TLorentzVector.h"

#include "TOKinCut.h"
#include "TOA2BaseDetParticle.h"


//______________________________________________________________________________
TOKinCut::TOKinCut(const TOKinCut& orig)
    : TObject(orig)
{
    // Copy constructor.

    fType = orig.fType;
    fMin  = orig.fMin;
    fMax  = orig.fMax;
}

//______________________________________________________________________________
Bool_t TOKinCut::IsInside(TLorentzVector* p4)
{
    // Check if the Lorentz-vector 'p4' fulfills this cut.
    // Return kTRUE if yes, otherwise kFALSE.
    // If no cut is defined always kTRUE is returned.
    
    switch (fType)
    {
        case kNoCut: return kTRUE;
        case kEnergyCut:
        {
            Double_t e = p4->E();
            if (e >= fMin && e < fMax) return kTRUE;
            else return kFALSE;
        }
        case kThetaCut:
        {
            Double_t th = p4->Theta() * TMath::RadToDeg();
            if (th >= fMin && th < fMax) return kTRUE;
            else return kFALSE;
        }
        case kPhiCut:
        {
            Double_t phi = p4->Phi() * TMath::RadToDeg();
            if (phi >= fMin && phi < fMax) return kTRUE;
            else return kFALSE;
        }
        default: return kTRUE;
    }
}

//______________________________________________________________________________
Bool_t TOKinCut::IsInside(TOA2BaseDetParticle* p)
{
    // Check if the detected particle 'p' fulfills this cut.
    // Return kTRUE if yes, otherwise kFALSE.
    // If no cut is defined always kTRUE is returned.
    
    switch (fType)
    {
        case kNoCut: return kTRUE;
        case kEnergyCut:
        {
            Double_t e = p->GetEnergy();
            if (e >= fMin && e < fMax) return kTRUE;
            else return kFALSE;
        }
        case kThetaCut:
        {
            Double_t th = p->GetTheta() * TMath::RadToDeg();
            if (th >= fMin && th < fMax) return kTRUE;
            else return kFALSE;
        }
        case kPhiCut:
        {
            Double_t phi = p->GetPhi() * TMath::RadToDeg();
            if (phi >= fMin && phi < fMax) return kTRUE;
            else return kFALSE;
        }
        default: return kTRUE;
    }
}

//______________________________________________________________________________
void TOKinCut::PrintToString(Char_t* outString)
{
    // Print the content of this class to 'outString'.
    
    Char_t name[256];

    switch (fType)
    {
        case kNoCut:     strcpy(name, "No cut"); break;
        case kEnergyCut: strcpy(name, "Energy cut"); break;
        case kThetaCut:  strcpy(name, "Theta cut"); break;
        case kPhiCut:    strcpy(name, "Phi cut"); break;
        default:         strcpy(name, "No cut"); break; 
    }

    sprintf(outString, "%s from %.2f to %.2f", name, fMin, fMax);
}
ClassImp(TOKinCut)
