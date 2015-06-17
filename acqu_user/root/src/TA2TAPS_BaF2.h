//--Author      JRM Annand   30th Sep 2003
//--Rev         JRM Annand...15th Oct 2003  ReadDecoded...MC data
//--Update      JRM Annand    5th Feb 2004  3v8 compatibility
//--Update        R Gregor    3th Jul 2005  Added veto and pulseshape reconstruction
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2TAPS_BaF2
//
// Decoding and calibration methods for TAPS array of BaF2
// Configured as forward wall to work in conjunction with the CB
//

#ifndef __TA2TAPS_BaF2_h__
#define __TA2TAPS_BaF2_h__

#include "TRandom3.h"

#include "MCBranchID.h"
#include "TA2ClusterDetector.h"
#include "HitClusterTAPS_t.h"

enum ETAPSType {
    ENoType,
    EBaF2,
    EBaF2_PWO_08,
    EBaF2_PWO_09,
};
typedef ETAPSType TAPSType_t;

class TA2TAPS_BaF2 : public TA2ClusterDetector
{
 protected:
     TAPSType_t fType;  // type of TAPS configuration	
	
 private:
  HitD2A_t** fSGEnergy;
  HitD2A_t** fLGEnergy;
  Double_t* fSGEnergyValue;
  UInt_t fMaxSGElements;
  UInt_t fNSG;
  TRandom3* pRandoms;
  Bool_t bSimul;
  Double_t* EnergyAll;
  Double_t* theta_c;
  Double_t* phi_c;
  Double_t* x_c;
  Double_t* y_c;
  Double_t* z_c;
  Double_t* Z_c;
  UInt_t    fUseEnergyResolution;      // Use energy resoution smearing for MC?
  UInt_t    fUseTimeResolution;        // Use time resolution smearing in MC?
  Double_t  fUseVetoEfficiency;
  Double_t  fEnergyResolutionFactor;   // Factor in Energy Resolution Equation
  Double_t  fEnergyResolutionConst;    // Power in energy Resolution Equation
  Double_t  fTimeResolution;           // Sigma for time resolution
  Double_t  fOffsetTime;               // MC time offset
  Double_t  fThetaResolution;          // Theta resolution
  Double_t  fPhiResolution;            // Phi Resolution
  Double_t  fVetoThreshold;
  Double_t  fVetoEfficiency[5];
  Double_t  fVetoEnergy[5];
 public:
  TA2TAPS_BaF2( const char*, TA2System* ); //Normal use
  virtual ~TA2TAPS_BaF2();
  virtual void PostInit();                 //Initialise using setup info
  virtual void SaveDecoded();              //Save local analysis
  virtual void ReadDecoded();              //Read back previous analysis
  virtual void Decode();
  virtual void SetConfig(Char_t*, Int_t);
  Int_t GetMaxSGElements() { return fMaxSGElements; }
  Double_t GetSGEnergy(Int_t);             //Get short-gate energy of crystal i
  Double_t GetLGEnergy(Int_t);             //Get long-gate energy of crystal i
  Double_t* GetEnergyAll(){ return EnergyAll; }
  Double_t GetEnergyAll(Int_t t){ return EnergyAll[t]; }
  Double_t GetEnergyResolutionGeV(Double_t);
  Double_t GetSigmaEnergyGeV(Double_t);
  Double_t GetEnergyResolution(Double_t);
  Double_t GetPhiResolution();
  Double_t GetPhiResolutionDg();
  Double_t GetThetaResolution();       // Return Theta res. for given Theta (radian)
  Double_t GetThetaResolutionDg();     // Return Theta res. for given Theta (degree)
  Double_t GetTimeResolution();        // Return sigma of time resolution
  Double_t GetClusterThreshold() {return fClEthresh; }
  HitD2A_t* GetLGElement(Int_t i);
  HitD2A_t* GetSGElement(Int_t i);

  TAPSType_t GetType() const { return fType; }
    
  ClassDef(TA2TAPS_BaF2,1)
};

//---------------------------------------------------------------------------

inline void TA2TAPS_BaF2::Decode()
{
  bSimul = false;

  //I want this empty...
  if(fMaxSGElements)
    for(UInt_t t=0; t<fMaxSGElements; t++)
      fSGEnergyValue[t] = 0.0;

  TA2ClusterDetector::Decode();

  //Get crystal energies without thresholds from configuration file
  HitD2A_t* elem;
  Double_t EnergySgl;
  //Double_t TimeSgl;

  for(UInt_t j=0; j<fNelem; j++)
  {
    elem = (HitD2A_t*)fElement[j];
    EnergySgl = elem->GetEnergy() * fEnergyScale;
    //TimeSgl = elem->GetTime();
    if((EnergySgl > 0.0) && (EnergySgl < 9999.9))
      EnergyAll[j] = EnergySgl;
    else
      EnergyAll[j] = 0.0;
  }
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetSGEnergy(Int_t i)
{
  if(fSGEnergy)
  {
    if(!bSimul)
    {
      fSGEnergyValue[i] = 0.0;
      if(fSGEnergy[i]->Calibrate())
        fSGEnergyValue[i] = fSGEnergy[i]->GetEnergy();
    }
  }
  else
    fSGEnergyValue[i] = 0.0;
  return fSGEnergyValue[i];
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetLGEnergy(Int_t i)
{
  if(fLGEnergy)
  {
    if(!bSimul)
    {
      if(fLGEnergy[i]->Calibrate())
        return (fLGEnergy[i]->GetEnergy());
    }
    else
      return fEnergy[i];
  }

  return 0.0;
}

//---------------------------------------------------------------------------

inline HitD2A_t* TA2TAPS_BaF2::GetLGElement(Int_t i)
{
	// get LG element i
	if (fLGEnergy) 
	{
		if (fLGEnergy[i]) return fLGEnergy[i];
		else return 0;
	}
	else return 0;
}

//---------------------------------------------------------------------------

inline HitD2A_t* TA2TAPS_BaF2::GetSGElement(Int_t i)
{
	// get SG element i
	if (fSGEnergy) 
	{
		if (fSGEnergy[i]) return fSGEnergy[i];
		else return 0;
	}
	else return 0;
}

//---------------------------------------------------------------------------

// resolution behaviour as given in PhysRev C 79, 035204 (2009)
inline Double_t TA2TAPS_BaF2::GetEnergyResolutionGeV(Double_t pEnergy)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (fEnergyResolutionFactor * TMath::Sqrt(pEnergy) + fEnergyResolutionConst * pEnergy);
}

//---------------------------------------------------------------------------

// this function is assumed by default by the parameters in the config file
inline Double_t TA2TAPS_BaF2::GetSigmaEnergyGeV(Double_t pEnergy)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (fEnergyResolutionFactor * TMath::Power(pEnergy, fEnergyResolutionConst));
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetEnergyResolution(Double_t pEnergy)
{
  // Returns energy resolution in MeV when supplied Energy in MeV
  Double_t resolution, energy;
  energy = pEnergy / 1000.0;
  resolution = GetEnergyResolutionGeV(energy) * 1000.0;
  return resolution;
}

//---------------------------------------------------------------------------

inline  Double_t TA2TAPS_BaF2::GetThetaResolutionDg()
{
  // Gives theta resolution in degrees
  return fThetaResolution;
}

//---------------------------------------------------------------------------

inline  Double_t TA2TAPS_BaF2::GetThetaResolution()
{
  // Gives theta resolution in degrees
  return (fThetaResolution * TMath::DegToRad());
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetPhiResolutionDg()
{
  // Returns Phi resolution in degrees when given theta of
  // cluster in degrees
  return (fPhiResolution);
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetPhiResolution()
{
  // Returns Phi resolution in degrees when given theta of
  // cluster in radian
  return (fPhiResolution * TMath::DegToRad());
}

//---------------------------------------------------------------------------

inline Double_t TA2TAPS_BaF2::GetTimeResolution()
{
  // Returns time resolution in ns
  return fTimeResolution;
}

//---------------------------------------------------------------------------

inline void TA2TAPS_BaF2::ReadDecoded()
{
  // Read back "decoded" data for the BaF2 array
  // In this case produced by the GEANT simulation

  Int_t nVetos = 0;
  Int_t nBaF2s = 0;
  Double_t total = 0.0;
  fNhits = *(Int_t*)(fEvent[EI_ntaps]);
  Float_t* energy = (Float_t*)(fEvent[EI_ectapsl]);
  Float_t* time = (Float_t*)(fEvent[EI_tctaps]);
  UInt_t* index = (UInt_t*)(fEvent[EI_ictaps]);
  Float_t* venergy = (Float_t*)(fEvent[EI_evtaps]);
  Float_t* dircos = (Float_t*)(fEvent[EI_dircos]);
  Int_t npart = *(Int_t*)(fEvent[EI_npart]);
  Int_t* idpart = (Int_t*)(fEvent[EI_idpart]);
  Double_t E, T;  // Energy, time
  UInt_t proton_crystal;

  // Find proton index in simulated data
  UInt_t proton = 0;
  while (proton++ < (UInt_t)npart)
    if (idpart[proton] == 14) break;  // 14 = GEANT proton

  // Move pointer to correct positions of proton data
  // Therefore skip all x, y, z components for all particles before the proton
  for (UInt_t t = 0; t < proton; t++)
    for (UInt_t i = 0; i < 3; i++) dircos++;
  // Read x, y, and z components of proton
  Double_t x_p = *dircos++;
  Double_t y_p = *dircos++;
  Double_t z_p = *dircos;
  // Calculate proton theta and phi angles out of Cartesian components
  Double_t theta_p = acos(z_p);
  Double_t phi_p = atan2(y_p, x_p);

  // For each crystal: evaluate deviation from proton direction in theta-phi plane
  Double_t delta = 1e38;  // difference between the true proton position and the cluster hit
  proton_crystal = 65535;  // id of the crystal closest to the true proton position
  Double_t dphi, dtheta, tmp;

  if (theta_p < (TMath::DegToRad()*22.0))  // check if proton hit is not off
    for (UInt_t t = 0; t < fNelem; t++)
    {
      dtheta = theta_p - theta_c[t];
      dphi = phi_p - phi_c[t];
      if (dphi > TMath::Pi()) dphi = TMath::TwoPi() - dphi;
      if (dphi < -TMath::Pi()) dphi = -TMath::TwoPi() - dphi;
      tmp = dphi*dphi + dtheta*dtheta;
      // Find crystal which fits best for proton direction in theta-phi plane
      if (tmp < delta)
      {
        delta = tmp;
        proton_crystal = t;
      }
    }

  // Clear arrays / set them to EBufferEnd
  if (fMaxSGElements)
    memset(fSGEnergyValue, EBufferEnd, fMaxSGElements*sizeof(Double_t));
  if (fPatternHits)
    memset(fPatternHits[0], EBufferEnd, fNelem*sizeof(Int_t));
  memset(EnergyAll, 0., fNelem*sizeof(Double_t));

  // Finally evaluate all simulated hits
  for (UInt_t i = 0; i < fNhits; i++)
  {
    if (!index[i]) break;
    // IDs in simulation: 1 to n, here 0 to n-1
    UInt_t elem = index[i] - 1;

    // check for ignored element
    if (fElement[elem]->IsIgnored()) continue;

    // convert energy and smear
    E = energy[i] * fEnergyScale * fElement[elem]->GetA1();
    if (fUseEnergyResolution) E += pRandoms->Gaus(0.0, GetSigmaEnergyGeV(E));
    E *= 1000.;
    EnergyAll[elem] = E;

    // check energy thresholds
    if ((E < fElement[elem]->GetEnergyLowThr()) ||
        (E > fElement[elem]->GetEnergyHighThr())) continue;

    // convert time
    T = 0.;
    if (fIsTime)
    {
      T = time[i] - fElement[elem]->GetT0() - fOffsetTime;
      if (fTimeResolution > 0) T += pRandoms->Gaus(0.0, fTimeResolution);

      // check time thresholds
      if ((T < fElement[elem]->GetTimeLowThr()) ||
          (T > fElement[elem]->GetTimeHighThr())) continue;

      // calculate time of flight
      //TODO: What's with protons? Handle them differently! (Patrik A. and Sascha W.; 2015-06-17)
      Double_t c = 30.; // speed of light, cm per ns
      T += (Z_c[elem] * TMath::Cos(theta_c[elem]))/c;  // apply gamma TOF factor to time
    }

    /* at this point we have a valid hit */

    // Fill Veto information
    if(fPatternHits) // ... but only if bit pattern unit is active
      if((venergy[i] * 1000.0) > fVetoThreshold) // Veto hit if deposited enery is above threshold
      {
        Double_t Efficiency = 0.;
        if (E < fVetoEnergy[0]) Efficiency = fVetoEfficiency[0];
        else if ((E > fVetoEnergy[0]) && (E < fVetoEnergy[1])) Efficiency = fVetoEfficiency[1];
        else if ((E > fVetoEnergy[1]) && (E < fVetoEnergy[2])) Efficiency = fVetoEfficiency[2];
        else if ((E > fVetoEnergy[2]) && (E < fVetoEnergy[3])) Efficiency = fVetoEfficiency[3];
        else if (E > fVetoEnergy[3]) Efficiency = fVetoEfficiency[4];
        if (pRandoms->Rndm() < Efficiency)
        {
          fPatternHits[0][nVetos] = elem;
          nVetos++;
        }
      }

    // Fill BaF2 information
    fEnergy[elem] = E;
    fEnergyOR[nBaF2s] = E;
    if (fIsTime)
    {
      fTime[elem] = T;
      fTimeOR[nBaF2s] = T;
    }
    fHits[nBaF2s] = elem;
    total += E;
    nBaF2s++;

    //PSA - reasonable?
    if(fMaxSGElements)
    {
      Bool_t IsBaF2 = true;
      if ((fNelem == 402) && ((elem % 67) <  4)) IsBaF2 = false; // If 1 PbWO ring is installed, the first 4 crystals in a TAPS block are PbWO4s
      if ((fNelem == 438) && ((elem % 73) < 12)) IsBaF2 = false; // If 2 PbWO rings are installed, the first 12 crystals in a TAPS block are PbWO4s

      if (IsBaF2) // Do PSA information only for BaF2s
      {
        if ((elem == proton_crystal) || fCluster[elem]->IsNeighbour(proton_crystal)) // Hit in or neighbouring crystal best fitting to proton
          //fSGEnergyValue[j] = E * TMath::Power(E, 0.05) * 0.65; //Sven's function
          fSGEnergyValue[elem] = -0.9691614 + 0.7108238*E + 0.1819194e-2*E*E - 0.9019634e-5*E*E*E + 0.1610750e-7*E*E*E*E; //Marc's function
      else
          fSGEnergyValue[elem] = 0.96 * E;
        fSGEnergyValue[elem] += pRandoms->Gaus(0.0, 0.25 * TMath::Sqrt(fSGEnergyValue[elem]));
      }
      else
        fSGEnergyValue[elem] = E;
    }

  }

  fHits[nBaF2s] = EBufferEnd;
  fTimeOR[nBaF2s] = EBufferEnd;
  fEnergyOR[nBaF2s] = EBufferEnd;
  fNhits = nBaF2s;
  fTotalEnergy = total;

  if (fPatternHits)
  {
    fPatternHits[0][nVetos] = EBufferEnd;
    fNPatternHits[0] = nVetos;
    // Note, that this may conflict with other ReadDecoded() functions later on, as this destroys the venergy[]
    // information! So, the bit pattern veto should not be used if another veto implementation is running!
    memset(venergy, 0., fNelem*sizeof(Float_t));  // Need to cleanup, so that no data from previous event remains
  }

  if (fIsRawHits)
  {
    fRawEnergyHits[0] = EBufferEnd;
    fRawTimeHits[0] = EBufferEnd;
  }

  bSimul = true;
}

//---------------------------------------------------------------------------

#endif
