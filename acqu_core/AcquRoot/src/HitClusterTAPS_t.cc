//--Author	JRM Annand   30th Aug 2003
//--Rev 	A.Starostin..24th Jan 2004  add theta,phi
//--Rev 	JRM Annand   30th Jan 2004  mod neighbour determination
//--Update	JRM Annand   21st Oct 2004  cluster time
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// THitCluster
// Specimen for hit cluster determination in a segmented calorimeter
// Mean cluster position obtained from the sqrt(E) weighted sum
//
//---------------------------------------------------------------------------

#include "HitClusterTAPS_t.h"
#include "TA2ClusterDetector.h"

//---------------------------------------------------------------------------

HitClusterTAPS_t::HitClusterTAPS_t( Char_t* line, UInt_t index )
                 :HitCluster_t(line, index)
{

}

//______________________________________________________________________________
void HitClusterTAPS_t::ClusterDetermine(TA2ClusterDetector* cl)
{
    // Cluster algorithm for TAPS.

    // get detector pointers
    Double_t* energy   = cl->GetEnergy();
    Double_t* time     = cl->GetTime();
    UInt_t* hits       = cl->GetTempHits();
    TVector3** pos     = cl->GetPosition();
    UInt_t nhits       = cl->GetNhits();

    // define local variables
    Double_t w, w_tot;
    UInt_t loc_nhits = 0;
    UInt_t loc_hits[fMaxHits+1];
    TVector3 loc_pos[fMaxHits+1];
    Double_t loc_energy[fMaxHits+1];
    UInt_t loc_orighit[fMaxHits+1];

    // process central element
    loc_nhits = 1;
    loc_hits[0] = fIndex;
    loc_pos[0] = *(pos[fIndex]);
    loc_energy[0] = energy[fIndex];
    if (cl->IsTime()) fTime = time[fIndex];

    // loop over cluster hits
    for (UInt_t i = 0; i < loc_nhits; i++)
    {
        // cluster element
        UInt_t index = loc_hits[i];

        // loop over total hits
        for (UInt_t j = 0; j < nhits; j++)
        {
            // skip already added hits
            if (hits[j] == ENullHit) continue;

            // skip and mark central hit
            if (hits[j] == fIndex)
            {
                hits[j] = ENullHit;
                continue;
            }

            // add if neighbour
            if (cl->GetCluster(index)->IsNeighbour(hits[j]))
            {
                // save position, energy, element index, original hit index
                loc_pos[loc_nhits] = *(pos[hits[j]]);
                loc_energy[loc_nhits] = energy[hits[j]];
                loc_orighit[loc_nhits] = j;
                loc_hits[loc_nhits] = hits[j];

                // count hit
                loc_nhits++;

                // mark as added in total hit array
                hits[j] = ENullHit;
            }
        }
    }

    // check if two local, not-neighouring maxima were found
    Int_t secMax = -1;
    //if (loc_nhits > 2)
    //{
    //    // sort according to energy
    //    UInt_t id_sort[loc_nhits];
    //    TMath::Sort(loc_nhits, loc_energy, id_sort);

    //    // check if the two maxima are neighbours
    //    if (!cl->GetCluster(loc_hits[id_sort[0]])->IsNeighbour(loc_hits[id_sort[1]]) &&
    //        loc_energy[id_sort[1]] / loc_energy[id_sort[0]] > 0.5)
    //    {
    //        // unmark second maximum as added
    //        hits[loc_orighit[id_sort[1]]] = loc_hits[id_sort[1]];
    //        loc_hits[id_sort[1]] = ENullHit;
    //        secMax = id_sort[1];
    //    }
    //}

    // compose final cluster
    fNhits = 0;
    fEnergy = 0;
    fSqrtEtot = 0.;
    fRadius = 0.;
    for (UInt_t i = 0; i < loc_nhits; i++)
    {
        // remove hits belonging to second cluster
        if (secMax != -1)
        {
            Double_t dist_1 = (loc_pos[0] - loc_pos[i]).Mag();
            Double_t dist_2 = (loc_pos[secMax] - loc_pos[i]).Mag();

            // hit is nearer to second center
            if (dist_2 < dist_1)
            {
                // unmark as added
                hits[loc_orighit[i]] = loc_hits[i];
                continue;
            }
        }

        // skip removed hits
        if (loc_hits[i] != ENullHit)
        {
            fHits[fNhits++] = loc_hits[i];
            fEnergy += loc_energy[i];
        }
    }

    fHits[fNhits] = EBufferEnd;                  // mark no more hits

    // Calculate weights and mean position
    w_tot = 0.;
    fMeanPosition->SetXYZ(0., 0., 0.);
    for (UInt_t i = 0; i < loc_nhits; i++)
    {
        // skip removed hits
        if (loc_hits[i] != ENullHit)
        {
            // calculate log. weight
            w = 5. + TMath::Log(loc_energy[i] / fEnergy);
            if (w < 0)  w = 0.;
            w_tot += w;

            // update mean position
            *fMeanPosition += (w * loc_pos[i]);
        }
    }

    // Normalize mean position
    *fMeanPosition *= 1. / w_tot;

    // correct for the shower depth
    Double_t sh_dep = 2.05 * (TMath::Log(fEnergy / 12.7) + 1.2);
    if (sh_dep > 0)
    {
        Double_t sh_corr = fMeanPosition->Mag() / sh_dep + 1.;
        fMeanPosition->SetX(fMeanPosition->X() - fMeanPosition->X() / sh_corr);
        fMeanPosition->SetY(fMeanPosition->Y() - fMeanPosition->Y() / sh_corr);
    }

    // Calculate the rest of the cluster information
    fCentralFrac = energy[fIndex] / fEnergy;
    fTheta = TMath::RadToDeg() * fMeanPosition->Theta();
    fPhi   = TMath::RadToDeg() * fMeanPosition->Phi();
}

