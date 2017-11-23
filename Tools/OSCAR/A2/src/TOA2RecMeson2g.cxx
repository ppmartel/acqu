/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2RecMeson2g                                                       //
//                                                                      //
// Class for the reconstruction of mesons in the A2 setup using a two   //
// photon decay.                                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TLorentzVector.h"

#include "TOA2RecMeson2g.h"
#include "TOA2DetParticle.h"


//______________________________________________________________________________
TOA2RecMeson2g::TOA2RecMeson2g(Int_t nPart, Double_t mass)
    : TOA2RecParticle(2)
{
    // Constructor ('nPart' is not used here).

    // init members
    fNeutronCand = 0;
    fMass = mass;
}

//______________________________________________________________________________
TOA2RecMeson2g::~TOA2RecMeson2g()
{
    // Destructor.

    if (fNeutronCand) delete fNeutronCand;
}

//______________________________________________________________________________
Bool_t TOA2RecMeson2g::Reconstruct(Int_t nPart, TOA2DetParticle** partList)
{
    // Reconstruction method.

    // check the number of particles
    if (nPart < 2)
    {
        Error("Reconstruct", "Number of particles has to be >= 2!");
        return kFALSE;
    }
    // copy the particles directly in case of 2 hits
    else if (nPart == 2)
    {
        fDetectedProducts[0] = new TOA2DetParticle(*partList[0]);
        fDetectedProducts[1] = new TOA2DetParticle(*partList[1]);
        fDetectedProducts[0]->SetPDG_ID(kGamma);
        fDetectedProducts[1]->SetPDG_ID(kGamma);
        
        // calculate the 4-vector
        Calculate4Vector();

        return kTRUE;
    }
    else
    {
        // find the best combination to form a meson 
        Int_t id_1, id_2;
        FindBestMeson(nPart, partList, &id_1, &id_2);
        
        // leave if no solution was selected
        if (fChiSquare == 1e30) return kFALSE;
 
        // create detected particle list
        fDetectedProducts[0] = new TOA2DetParticle(*partList[id_1]);
        fDetectedProducts[1] = new TOA2DetParticle(*partList[id_2]);
        fDetectedProducts[0]->SetPDG_ID(kGamma);
        fDetectedProducts[1]->SetPDG_ID(kGamma);
       
        // save neutron candidate in 3 particle case
        if (nPart == 3) fNeutronCand = new TOA2DetParticle(*partList[3-id_1-id_2]);

        // calculate the 4-vector
        Calculate4Vector();

        return kTRUE;
    }
}

//______________________________________________________________________________
void TOA2RecMeson2g::FindBestMeson(Int_t n, TOA2DetParticle** list, Int_t* id_1, Int_t* id_2)
{
    // Find the best meson of two photons using 'n' particles from the list 'list' .
    // The indices of the two particles belonging to the best combination
    // will be stored in id_1 and id_2, respectively.
    
    // check number of particles
    if (n > 20)
    {
        Error("FindBestMeson", "Number of particles cannot exceed 20!");
        return;
    }

    // calculate the 4-vectors of all particles assuming
    // they are photons
    TLorentzVector p4[20];
    for (Int_t i = 0; i < n; i++) list[i]->Calculate4Vector(p4+i, 0.);
    
    Double_t bestChi = 1e30;
    // loop over first particle
    for (Int_t i = 0; i < n; i++)
    {
        // loop over second particle
        for (Int_t j = i+1; j < n; j++)
        {
            // calculate invariant mass and its error
            Double_t im = (p4[i] + p4[j]).M();
            Double_t im_err = im * list[i]->CalcChi2IMError(list[j]);
            
            // calculate chi square
            Double_t chi = ((fMass - im) / im_err) * ((fMass - im) / im_err);
            
            // chi square is smaller
            if (chi < bestChi)
            {
                *id_1 = i;
                *id_2 = j;
                bestChi = chi;
            } 
        } // second particle loop
    } // first particle loop

    // save best chi square value
    fChiSquare = bestChi;
}

//______________________________________________________________________________
void TOA2RecMeson2g::Reset()
{
    // Reset the content for a new reconstruction.
    
    // call parent method
    TOA2RecParticle::Reset();
    
    // delete neutron candidate
    if (fNeutronCand)
    {
        delete fNeutronCand;
        fNeutronCand = 0;
    }
}
ClassImp(TOA2RecMeson2g)
