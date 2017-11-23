/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2DetParticle                                                      //
//                                                                      //
// Class for reconstructed particles in the A2 setup.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TVector3.h"
#include "TMath.h"

#include "TOA2DetParticle.h"
#include "TOSUtils.h"


//______________________________________________________________________________
TOA2DetParticle::TOA2DetParticle(const TOA2DetParticle& orig)
    : TOA2BaseDetParticle(orig)
{
    // Copy constructor.

    fClusterHits        = new UInt_t[fClusterSize];
    fClusterHitEnergies = new Double_t[fClusterSize];
    fClusterHitTimes    = new Double_t[fClusterSize];
    for (Int_t i = 0; i < fClusterSize; i++) 
    {
        fClusterHits[i]        = orig.fClusterHits[i];
        fClusterHitEnergies[i] = orig.fClusterHitEnergies[i];
        fClusterHitTimes[i]    = orig.fClusterHitTimes[i];
    }

    fErrorTheta = orig.fErrorTheta;
    fErrorPhi = orig.fErrorPhi;
    fErrorEnergy = orig.fErrorEnergy;
}

//______________________________________________________________________________
TOA2DetParticle::TOA2DetParticle(const TOA2BaseDetParticle& orig)
    : TOA2BaseDetParticle(orig)
{
    // Copy constructor from TOA2BaseDetParticle.
    
    fClusterHits        = new UInt_t[fClusterSize];
    fClusterHitEnergies = new Double_t[fClusterSize];
    fClusterHitTimes    = new Double_t[fClusterSize];
    for (Int_t i = 0; i < fClusterSize; i++) 
    {
        fClusterHits[i]        = 0;
        fClusterHitEnergies[i] = 0;
        fClusterHitTimes[i]    = 0;
    }

    fErrorTheta  = 0;
    fErrorPhi    = 0;
    fErrorEnergy = 0;
}

//______________________________________________________________________________
TOA2DetParticle::~TOA2DetParticle()
{
    // Destructor.

    if (fClusterHits) delete [] fClusterHits;
    if (fClusterHitEnergies) delete [] fClusterHitEnergies;
    if (fClusterHitTimes) delete [] fClusterHitTimes;
}

//______________________________________________________________________________
void TOA2DetParticle::SetClusterHits(Int_t size, UInt_t* hits)
{
    // Set the cluster hit elements using the hit array 'hits' and its
    // length 'size'.

    // delete old array if necessary
    if (fClusterHits) delete [] fClusterHits;

    // set the cluster size
    fClusterSize = size;

    // create a new array
    fClusterHits = new UInt_t[fClusterSize];

    // copy the array elements
    for (Int_t i = 0; i < fClusterSize; i++) fClusterHits[i] = hits[i];
}

//______________________________________________________________________________
void TOA2DetParticle::SetClusterHitEnergies(Int_t size, Double_t* energies)
{
    // Set the cluster hit energies using the energy array 'energies' and its
    // length 'size'.
    
    // delete old array if necessary
    if (fClusterHitEnergies) delete [] fClusterHitEnergies;

    // set the cluster size
    fClusterSize = size;

    // create a new array
    fClusterHitEnergies = new Double_t[fClusterSize];

    // copy the array elements
    for (Int_t i = 0; i < fClusterSize; i++) fClusterHitEnergies[i] = energies[i];
}

//______________________________________________________________________________
void TOA2DetParticle::SetClusterHitTimes(Int_t size, Double_t* times)
{
    // Set the cluster hit times using the time array 'times' and its
    // length 'size'.
    
    // delete old array if necessary
    if (fClusterHitTimes) delete [] fClusterHitTimes;

    // set the cluster size
    fClusterSize = size;

    // create a new array
    fClusterHitTimes = new Double_t[fClusterSize];

    // copy the array elements
    for (Int_t i = 0; i < fClusterSize; i++) fClusterHitTimes[i] = times[i];
}

//______________________________________________________________________________
Double_t TOA2DetParticle::CalcChi2IMError(TOA2DetParticle* p)
{
    // Calculate the relative error dm(p1,p2)/m(p1,p2) on the reconstructed invariant 
    // mass of this particle and the particle 'p'. 
    // This method takes into account the errors of the detected energy
    // and of the theta and phi angles.
    // If one of the errors of any particle is zero an error of 1 is returned.
    
    // check errors
    if (GetErrorTheta() == 0 || GetErrorPhi() == 0 || GetErrorEnergy() == 0) return 1;
    if (p->GetErrorTheta() == 0 || p->GetErrorPhi() == 0 || p->GetErrorEnergy() == 0) return 1;

    // theta
    Double_t th1 = GetTheta();
    Double_t dth1 = GetErrorTheta();
    Double_t th2 = p->GetTheta();
    Double_t dth2 = p->GetErrorTheta();
    
    // phi
    Double_t ph1 = GetPhi();
    Double_t dph1 = GetErrorPhi();
    Double_t ph2 = p->GetPhi();
    Double_t dph2 = p->GetErrorPhi();

    // energy
    Double_t e1 = GetEnergy();
    Double_t de1 = GetErrorEnergy();
    Double_t e2 = p->GetEnergy();
    Double_t de2 = p->GetErrorEnergy();

    // cos(psi12) (opening angle)
    TVector3 v1(GetX(), GetY(), GetZ());
    TVector3 v2(p->GetX(), p->GetY(), p->GetZ());
    Double_t cos_psi12 = TMath::Cos(v1.Angle(v2));

    // error of cos(psi12)
    Double_t a = -TMath::Sin(th1)*TMath::Sin(th2)*TMath::Sin(ph1-ph2)*dph1;
    Double_t b = -TMath::Sin(th1)*TMath::Sin(th2)*TMath::Sin(ph2-ph1)*dph2;
    Double_t c = (TMath::Cos(th1)*TMath::Sin(th2)*TMath::Cos(ph1-ph2) - TMath::Sin(th1)*TMath::Cos(th2))*dth1;
    Double_t d = (TMath::Sin(th1)*TMath::Cos(th2)*TMath::Cos(ph2-ph1) - TMath::Cos(th1)*TMath::Sin(th2))*dth2; 
    Double_t dcos_psi12 = TMath::Sqrt(a*a + b*b + c*c + d*d);

    // return the relative error on the invariant mass
    return 0.5 * TMath::Sqrt((de1*de1/e1/e1) + (de2*de2/e2/e2) + 
                             (dcos_psi12*dcos_psi12/(1.-cos_psi12)/(1.-cos_psi12)));
}

//______________________________________________________________________________
void TOA2DetParticle::Print(Option_t* option) const
{
    // Print out the content of this class.

    // call parent method
    TOA2BaseDetParticle::Print();

    printf("TOA2DetParticle content:\n");
    printf("Cluster hits           : %s\n", TOSUtils::FormatArrayList(fClusterSize, fClusterHits));
    printf("Cluster hit energies   : %s\n", TOSUtils::FormatArrayList(fClusterSize, fClusterHitEnergies, "%f"));
    printf("Cluster hit times      : %s\n", TOSUtils::FormatArrayList(fClusterSize, fClusterHitTimes, "%f"));
    printf("Error in theta         : %f\n", fErrorTheta*TMath::RadToDeg());
    printf("Error in phi           : %f\n", fErrorPhi*TMath::RadToDeg());
    printf("Error in energy        : %f\n", fErrorEnergy);
}
ClassImp(TOA2DetParticle)
