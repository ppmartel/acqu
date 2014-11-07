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

inline Double_t TA2TAPS_BaF2::GetEnergyResolutionGeV(Double_t pEnergy)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (fEnergyResolutionFactor * TMath::Sqrt(pEnergy) + fEnergyResolutionConst * pEnergy);

}
//---------------------------------------------------------------------------
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

#endif
