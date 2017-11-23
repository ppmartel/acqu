/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2BaseDetParticle                                                  //
//                                                                      //
// Base class for reconstructed particles in the A2 setup.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TClass.h"
#include "TLorentzVector.h"

#include "TOA2BaseDetParticle.h"


//______________________________________________________________________________
TOA2BaseDetParticle::TOA2BaseDetParticle()
    : TObject()
{
    // Empty constructor.
    
    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();

    fPDG_ID   = kRootino;
    fDetector = kNoDetector;

    fX = 0;
    fY = 0;
    fZ = 0;

    fEnergy = 0;
    fTime   = 0;;

    fClusterSize = 0;

    fCentralElement = 0;
    fCentralEnergy  = 0;
    
    fPIDIndex  = -1;
    fPIDEnergy =  0;
    fPIDTime   =  0;

    fCentralSGEnergy = 0;
    
    fVetoIndex       = -1;
    fVetoEnergy      =  0;
    fVetoTime        =  0;
    
    fMWPC_X1 = 0;
    fMWPC_Y1 = 0;
    fMWPC_Z1 = 0;
    fMWPC_X2 = 0;
    fMWPC_Y2 = 0;
    fMWPC_Z2 = 0;
}

//______________________________________________________________________________
TOA2BaseDetParticle::TOA2BaseDetParticle(const TOA2BaseDetParticle& orig)
    : TObject(orig)
{
    // Copy constructor.
    
    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();

    fPDG_ID   = orig.fPDG_ID;
    fDetector = orig.fDetector;

    fX = orig.fX;
    fY = orig.fY;
    fZ = orig.fZ;

    fEnergy = orig.fEnergy;
    fTime   = orig.fTime;

    fClusterSize = orig.fClusterSize;

    fCentralElement = orig.fCentralElement;
    fCentralEnergy  = orig.fCentralEnergy;
    
    fPIDIndex  = orig.fPIDIndex;
    fPIDEnergy = orig.fPIDEnergy;
    fPIDTime   = orig.fPIDTime;

    fCentralSGEnergy = orig.fCentralSGEnergy;
    
    fVetoIndex  = orig.fVetoIndex;
    fVetoEnergy = orig.fVetoEnergy;
    fVetoTime   = orig.fVetoTime;

    fMWPC_X1 = orig.fMWPC_X1;
    fMWPC_Y1 = orig.fMWPC_Y1;
    fMWPC_Z1 = orig.fMWPC_Z1;
    fMWPC_X2 = orig.fMWPC_X2;
    fMWPC_Y2 = orig.fMWPC_Y2;
    fMWPC_Z2 = orig.fMWPC_Z2;
}

//______________________________________________________________________________
void TOA2BaseDetParticle::Calculate4Vector(TLorentzVector* p4, Double_t mass)
{
    // Calculate the 4-vector of this particle using 'mass' as the particle's 
    // mass and the detected energy and position information. The components of
    // the 4-vector will be stored in 'p4'.
 
    Double_t tot_e = fEnergy + mass;
    p4->SetE(tot_e);
    TVector3 p(fX, fY, fZ);
    p4->SetVect(p.Unit() * TMath::Sqrt(tot_e*tot_e - mass*mass));
}

//______________________________________________________________________________
void TOA2BaseDetParticle::Calculate4Vector(TLorentzVector* p4, Double_t mass, Double_t energy)
{
    // Calculate the 4-vector of this particle using 'mass' as the particle's 
    // mass, 'energy' as the kinetic energy and the detected position information. 
    // The energy 'fEnergy' saved in this class will not be used in this method.
    // The components of the 4-vector will be stored in 'p4'.
    
    Double_t tot_e = energy + mass;
    p4->SetE(tot_e);
    TVector3 p(fX, fY, fZ);
    p4->SetVect(p.Unit() * TMath::Sqrt(tot_e*tot_e - mass*mass));
}

//______________________________________________________________________________
Bool_t TOA2BaseDetParticle::Calculate4VectorTOFTagger(TLorentzVector* p4, Double_t mass, Double_t taggerTime,
                                                      Bool_t isMC)
{
    // Calculate the 4-vector of this particle using 'mass' as the particle's 
    // mass, 'taggerTime' as the tagger time and the detected position information. 
    // The components of the 4-vector will be stored in 'p4'.
    //
    // Return kFALSE if the calculated solution is unphysical, otherwise kTRUE.

    // calculate the kinetic energy
    Double_t ekin = CalculateEkinTOFTagger(mass, taggerTime, isMC);
    
    // reject unphysical solutions
    if (ekin < 0.) return kFALSE;

    // calculate the 4-vector
    Double_t tot_e = ekin + mass;
        
    // set the 4-vector
    p4->SetE(tot_e);
    TVector3 p(fX, fY, fZ);
    p4->SetVect(p.Unit() * TMath::Sqrt(tot_e*tot_e - mass*mass));
    
    return kTRUE;
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::CalculateEkinTOFTagger(Double_t mass, Double_t taggerTime, Bool_t isMC)
{
    // Calculate the kinetic energy of this particle using 'mass' as the particle's 
    // mass, 'taggerTime' as the tagger time and the detected position information. 
    //
    // Return a negative value if the calculated solution is unphysical.

    // calculate the time of flight for 1 meter [ns/1m]
    Double_t t_norm = CalculateTOFTagger(taggerTime, isMC);
    
    // calculate the beta
    Double_t beta = 1. / (t_norm * 1e-9 * TMath::C());

    // calculate the kinetic energy
    if (beta*beta < 1)
    {
        Double_t gamma = TMath::Sqrt(1. / (1. - beta*beta));
        return  mass * (gamma - 1.);
    }
    else return -1.;
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::CalculateEkinTOFTAPS(Double_t mass, Double_t tapsTime, Bool_t isMC)
{
    // Calculate the kinetic energy of this particle using 'mass' as the particle's 
    // mass, 'tapsTime' as the TAPS time and the detected position information. 
    //
    // Return a negative value if the calculated solution is unphysical.

    // calculate the time of flight for 1 meter [ns/1m]
    Double_t t_norm = CalculateTOFTAPS(tapsTime, isMC);
    
    // calculate the beta
    Double_t beta = 1. / (t_norm * 1e-9 * TMath::C());

    // calculate the kinetic energy
    if (beta*beta < 1)
    {
        Double_t gamma = TMath::Sqrt(1. / (1. - beta*beta));
        return  mass * (gamma - 1.);
    }
    else return -1.;
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::CalculateTOFTagger(Double_t taggerTime, Bool_t isMC,
                                                 A2TAPSType_t tapsType)
{
    // Return the time-of-flight normalized to 1 meter [ns/1m] relative
    // to the tagger time 'taggerTime'.
    //
    // If 'isMC' is kTRUE the MC timing is returned (different calculation for 
    // the TAPS TOF).
    // 'tapsType' is used for the different calculation for PbWO4 elements.
    
    // calculate the particle tof
    Double_t tof = 0;

    //
    // TAPS
    // t_tagg = t_scint - t_trig
    if (fDetector == kTAPSDetector) 
    {
        // check if PbWO4 or BaF2
        if (TOA2Detector::IsTAPSPWO(fCentralElement, tapsType))
        {
            // PbWO4
            // t_TAPS = t_PWO - t_trig
            // t = t_TAPS - t_tagg = t_PWO - t_scint
            tof = fTime - taggerTime;
        }
        else
        {
            // BaF2
            // t_TAPS = t_trig - t_CFD
            // t = -(t_TAPS + t_tagg) = -(t_scint - t_CFD) = t_CFD - t_scint
            tof = - (fTime + taggerTime);
        }
    }
    //
    // CB
    // t_CB = t_NaI - t_trig
    // t = t_CB - t_tagg = t_NaI - t_scint
    else if (fDetector == kCBDetector) 
    {
        tof = fTime - taggerTime;
    }

    // calculate the time of flight for 1 meter [ns/1m]
    if (isMC)
        return (fTime - taggerTime) / (GetFlightPath() / 100.);
    else
        return tof / (GetFlightPath() / 100.) + 1e9 / TMath::C();
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::CalculateTOFCB(Double_t cbTime, Bool_t isMC)
{
    // Return the time-of-flight normalized to 1 meter [ns/1m] relative
    // to the CB time 'cbTime'.
    //
    // If 'isMC' is kTRUE the MC timing is returned (different calculation).
    
    // calculate the particle tof
    Double_t tof = 0;

    //
    // TAPS
    // t_TAPS = t_trig - t_CFD
    // t_CB = t_NaI - t_trig
    // t = -(t_TAPS + t_CB) = -(t_NaI - t_CFD) = t_CFD - t_NaI
    if (fDetector == kTAPSDetector) 
    {
        tof = - (fTime + cbTime);
    }
    //
    // CB
    // t_CB = t_NaI - t_trig
    // t = t_CB - t_CB_ref = t_NaI - t_NaI_ref
    else if (fDetector == kCBDetector) 
    {
        tof = fTime - cbTime;
    }

    // calculate the time of flight for 1 meter [ns/1m]
    if (isMC)
        return fTime / (GetFlightPath() / 100.);
    else
        return tof / (GetFlightPath() / 100.) + 1e9 / TMath::C();
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::CalculateTOFTAPS(Double_t tapsTime, Bool_t isMC)
{
    // Return the time-of-flight normalized to 1 meter [ns/1m] relative
    // to the TAPS time 'tapsTime'.
    //
    // If 'isMC' is kTRUE the MC timing is returned (different calculation).
    
    // calculate the particle tof
    Double_t tof = 0;

    //
    // TAPS
    // t_TAPS = t_trig - t_CFD
    // t = t_TAPS_ref - t_TAPS = t_CFD - t_CFD_ref
    if (fDetector == kTAPSDetector) 
    {
        tof = tapsTime - fTime;
    }
    //
    // CB
    // t_TAPS = t_trig - t_CFD
    // t_CB = t_NaI - t_trig
    // t = t_CB + t_TAPS = t_NaI - t_CFD
    else if (fDetector == kCBDetector) 
    {
        tof = fTime + tapsTime;
    }

    // calculate the time of flight for 1 meter [ns/1m]
    if (isMC)
        return fTime / (GetFlightPath() / 100.);
    else
        return tof / (GetFlightPath() / 100.) + 1e9 / TMath::C();
}

//______________________________________________________________________________
void TOA2BaseDetParticle::SetPosition(TVector3* pos)
{
    // Set the hit position of the particle to 'pos.

    fX = pos->X();
    fY = pos->Y();
    fZ = pos->Z();
}

//______________________________________________________________________________
void TOA2BaseDetParticle::SetTheta(Double_t theta)
{
    // Set the theta angle keeping the magnitude and the azimuth angle constant (ROOT).

    Double_t mag = TMath::Sqrt(fX*fX + fY*fY + fZ*fZ); 
    Double_t phi = GetPhi();
    fX = mag * TMath::Sin(theta) * TMath::Cos(phi);
    fY = mag * TMath::Sin(theta) * TMath::Sin(phi);
    fZ = mag * TMath::Cos(theta);
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::GetFlightPath() const
{
    // Return the length of the flight path.

    return TMath::Sqrt(fX*fX + fY*fY + fZ*fZ);
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::GetTheta() const 
{
    // Return the polar angle (ROOT).

    return fX == 0.0 && fY == 0.0 && fZ == 0.0 ? 0.0 : TMath::ATan2(TMath::Sqrt(fX*fX + fY*fY), fZ); 
}

//______________________________________________________________________________
Double_t TOA2BaseDetParticle::GetPhi() const 
{
    // Return the azimuth angle (from -pi to pi) (ROOT).

    return fX == 0.0 && fY == 0.0 ? 0.0 : TMath::ATan2(fY, fX);
}

//______________________________________________________________________________
Bool_t TOA2BaseDetParticle::CalculatePSA(Double_t* psaR, Double_t* psaA)
{   
    // Calculate the PSA information of this particle.
    // Save the PSA radius to 'psaR' and the PSA angle to psA.
    // If the particle was detected in TAPS and the PSA information could
    // be calculated and saved, kTRUE is returned, otherwise kFALSE.
    
    // check if particle was detected in TAPS
    if (GetDetector() == kTAPSDetector)
    {   
        // calculate the PSA information
        Double_t lgE  = GetCentralEnergy();
        Double_t sgE  = GetCentralSGEnergy();
        Double_t r = TMath::Sqrt(sgE*sgE + lgE*lgE); 
        Double_t a = TMath::ATan(sgE/lgE)*TMath::RadToDeg();
        
        // saved information
        if (psaR) *psaR = r;
        else return kFALSE;
        if (psaA) *psaA = a;
        else return kFALSE;

        // calculation and checks were successful here
        return kTRUE;
    }
    else return kFALSE;
}

//______________________________________________________________________________
void TOA2BaseDetParticle::Print(Option_t* option) const
{
    // Print out the content of this class.

    // detector string
    Char_t detector[256] = "";
    switch (fDetector)
    {
        case kNoDetector:
            strcpy(detector, "no detector");
            break;
        case kCBDetector:
            strcpy(detector, "CB");
            break;
        case kTAPSDetector:
            strcpy(detector, "TAPS");
            break;
    }

    printf("TOA2BaseDetParticle content:\n");
    printf("PDG ID                 : %d\n", fPDG_ID);
    printf("Detector               : %s\n", detector);
    printf("Direction (x, y, z)    : %f, %f, %f\n", fX, fY, fZ);
    printf("Direction (theta, phi) : %f, %f\n", GetTheta()*TMath::RadToDeg(), GetPhi()*TMath::RadToDeg());
    printf("Energy                 : %f\n", fEnergy);
    printf("Time                   : %f\n", fTime);
    printf("Cluster size           : %d\n", fClusterSize);
    printf("Central element        : %d\n", fCentralElement);
    printf("Central energy         : %f\n", fCentralEnergy);
    printf("PID index              : %d\n", fPIDIndex);
    printf("PID energy             : %f\n", fPIDEnergy);
    printf("PID time               : %f\n", fPIDTime);
    printf("Central SG energy      : %f\n", fCentralSGEnergy);
    printf("Veto index             : %d\n", fVetoIndex);
    printf("Veto energy            : %f\n", fVetoEnergy);
    printf("Veto time              : %f\n", fVetoTime);
    printf("MWPC Hit 1 (x, y, z)   : %f, %f, %f\n", fMWPC_X1, fMWPC_Y1, fMWPC_Z1);
    printf("MWPC Hit 2 (x, y, z)   : %f, %f, %f\n", fMWPC_X2, fMWPC_Y2, fMWPC_Z2);
}

//______________________________________________________________________________
TOA2BaseDetParticle& TOA2BaseDetParticle::operator=(const TOA2BaseDetParticle& p)
{
    // Assignment operator.
    
    // check self assignment
    if (this != &p)
    {
        fPDG_ID   = p.fPDG_ID;
        fDetector = p.fDetector;

        fX = p.fX;
        fY = p.fY;
        fZ = p.fZ;

        fEnergy = p.fEnergy;
        fTime   = p.fTime;

        fClusterSize = p.fClusterSize;

        fCentralElement = p.fCentralElement;
        fCentralEnergy  = p.fCentralEnergy;
        
        fPIDIndex  = p.fPIDIndex;
        fPIDEnergy = p.fPIDEnergy;
        fPIDTime   = p.fPIDTime;

        fCentralSGEnergy = p.fCentralSGEnergy;
        
        fVetoIndex  = p.fVetoIndex;
        fVetoEnergy = p.fVetoEnergy;
        fVetoTime   = p.fVetoTime;
        
        fMWPC_X1 = p.fMWPC_X1;
        fMWPC_Y1 = p.fMWPC_Y1;
        fMWPC_Z1 = p.fMWPC_Z1;
        fMWPC_X2 = p.fMWPC_X2;
        fMWPC_Y2 = p.fMWPC_Y2;
        fMWPC_Z2 = p.fMWPC_Z2;
    }

    return *this;
}
ClassImp(TOA2BaseDetParticle)
