//--Author	JRM Annand   27th Apr 2003
//--Rev 	JRM Annand...30th Sep 2003  New Detector bases
//--Rev 	JRM Annand...15th Oct 2003  ReadDecoded...MC data
//--Rev 	JRM Annand... 5th Feb 2004  3v8 compatible
//--Rev 	JRM Annand... 7th Jun 2005  ReadDecoded...use fEnergyScale
//--Rev 	JRM Annand...25th Oct 2005  ReadDecoded...energy thresh
//--Rev 	D.Glazier ...24th Aug 2007  ReadDecoded...include detector time
//--Update	JRM Annand ..17th Nov 2007  ReadDecoded total energy fix
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2CalArray
//
// Decoding and calibration methods for the Crystall Ball array of NaI(Tl)
// Configured as forward wall to work in conjunction with the CB
// This can use standard TA2ClusterDetector methods or ones redifined here
//

#ifndef __TA2CalArray_h__
#define __TA2CalArray_h__

#include "MCBranchID.h"
#include "TA2ClusterDetector.h"
#include "TA2UserControl.h"
#include <iostream>

class TA2CalArray : public TA2ClusterDetector
{
 private:
 protected:
  Double_t* fEnergyAll;
  UInt_t    fUseSigmaEnergy;        // Use energy resoution smearing for MC?
  UInt_t    fUseSigmaTime;          // Use time resolution smearing in MC?
  Double_t  fSigmaEnergyFactor;     // Factor in Energy Resolution Equation
  Double_t  fSigmaEnergyPower;      // Power in energy Resolution Equation
  Double_t  fSigmaTime;             // Sigma for time resolution
  Double_t  fOffsetTime;            // MC time offset
  Double_t  fSigmaTheta;            // Theta resolution for CB
  Double_t  fSigmaPhi;              // Phi Resolution for CB

 public:
  TA2CalArray(const char*, TA2System*);// Normal use
  virtual ~TA2CalArray();
  virtual void PostInit();             // initialise using setup info
  virtual void ParseDisplay(char*);    // display setup
  virtual void Decode();               // hits -> energy procedure
  virtual void SaveDecoded();          // save local analysis
  virtual void ReadDecoded();          // read back previous analysis
  virtual void SetConfig(char*, int);  // read in TA2CalArray specific parameters
  Double_t GetSigmaEnergyGeV(Double_t);
  Double_t GetSigmaEnergy(Double_t);
  Double_t GetSigmaPhi(Double_t);
  Double_t GetSigmaPhiDg(Double_t);
  Double_t GetSigmaTheta();       // Return Theta res. for given Theta (radian)
  Double_t GetSigmaThetaDg();     // Return Theta res. for given Theta (degree)
  Double_t GetSigmaTime();        // Return sigma of time resolution
  Double_t* GetEnergyAll()               { return fEnergyAll; }
  Double_t  GetEnergyAll(Int_t t)        { return fEnergyAll[t]; }
  Double_t GetClusterThreshold()	 { return fClEthresh; } //*** 
 ClassDef(TA2CalArray,1)
};

//---------------------------------------------------------------------------

inline Double_t TA2CalArray::GetSigmaEnergyGeV(Double_t pEnergy)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (fSigmaEnergyFactor * TMath::Power(pEnergy, fSigmaEnergyPower));
}

//---------------------------------------------------------------------------

inline Double_t TA2CalArray::GetSigmaEnergy(Double_t pEnergy)
{
  // Returns energy resolution in MeV when supplied Energy in MeV
  Double_t sigma, energy;
  energy = pEnergy / 1000.0;
  sigma = GetSigmaEnergyGeV(energy) * 1000.0;
  return sigma;
}

//---------------------------------------------------------------------------

inline  Double_t TA2CalArray::GetSigmaThetaDg()
{
  // Gives theta resolution in degrees
  return fSigmaTheta;
}

//---------------------------------------------------------------------------

inline  Double_t TA2CalArray::GetSigmaTheta()
{
  // Gives theta resolution in degrees
  return (fSigmaTheta * TMath::DegToRad());
}

//---------------------------------------------------------------------------

inline Double_t TA2CalArray::GetSigmaPhiDg(Double_t pTheta)
{
  // Returns Phi resolution in degrees when given theta of
  // cluster in degrees
  return (fSigmaPhi / TMath::Sin(pTheta * TMath::DegToRad()));
}

//---------------------------------------------------------------------------

inline Double_t TA2CalArray::GetSigmaPhi(Double_t pTheta)
{
  // Returns Phi resolution in degrees when given theta of
  // cluster in radian
  return (fSigmaPhi * TMath::DegToRad() / TMath::Sin(pTheta));
}

//---------------------------------------------------------------------------

inline Double_t TA2CalArray::GetSigmaTime()
{
  // Returns time resolution in ns
  return fSigmaTime;
}

//---------------------------------------------------------------------------

inline void TA2CalArray::Decode()
{
  // Decode the NaI ADC and TDC information
  // Decode raw TDC and Scaler information into
  // Hit pattern, "Energy" pattern, aligned OR etc.

  TA2ClusterDetector::Decode();

  HitD2A_t* Element;
  Double_t Energy;
  for(UInt_t t=0; t<fNelem; t++)
  {
    if((fTime[t]==EBufferEnd) || (fTime[t]==-1.0))
      fTime[t] = (Double_t)ENullHit;

    Element = (HitD2A_t*)fElement[t];
    //Energy = (Element->GetADC())->D2A() * fEnergyScale;
    Energy = Element->GetEnergy();// * fEnergyScale;
    if((Energy > 0.0) && (Energy < 9999.9))
      fEnergyAll[t] = Energy;
    else
      fEnergyAll[t] = 0.0;
  }
}

//---------------------------------------------------------------------------

inline void TA2CalArray::ReadDecoded()
{
  // Read from MC or reduced file.

  // connect branches
  Int_t nHits = *(Int_t*)(fEvent[EI_nhits]);
  Int_t* hit = (Int_t*)(fEvent[EI_icryst]);
  Float_t* energy = (Float_t*)(fEvent[EI_ecryst]);
  Float_t* time = NULL;
  if (fIsTime) time = (Float_t*)(fEvent[EI_tcryst]);

  fNhits = 0;  // init valid hits
  Double_t total = 0;  // total energy
  for (UInt_t t=0; t<fNelem; t++)
    fEnergyAll[t] = 0.0;

  // loop over hits
  for (Int_t i = 0; i < nHits; i++)
  {
    // slightly less ugly decoding of icryst
    Int_t elem = hit[i] % 10000;

    // check spurious index
    if (elem == -1) continue;

    // check for ignored element
    if (fElement[elem]->IsIgnored()) continue;

    // convert energy and smear
    Double_t e = energy[i] * fEnergyScale * fElement[elem]->GetA1();
    if (fUseSigmaEnergy) e += fRandom->Gaus(0.0, GetSigmaEnergyGeV(e));
    e *= 1000;
    fEnergyAll[i] = e;

    // check energy thresholds
    if ((e < fElement[elem]->GetEnergyLowThr()) ||
        (e > fElement[elem]->GetEnergyHighThr())) continue;

    // convert time and smear
    Double_t t = 0;
    if (fIsTime)
    {
      t = time[i] - fElement[elem]->GetT0() - fOffsetTime;
      if (fSigmaTime > 0 && fUseSigmaTime) t += fRandom->Gaus(0.0, fSigmaTime);

      // check time range
      if ((t < fElement[elem]->GetTimeLowThr()) ||
          (t > fElement[elem]->GetTimeHighThr())) continue;
    }

    // register valid hit
    fEnergy[elem] = e;
    fEnergyOR[fNhits] = e;
    total += e;
    if (fIsTime)
    {
      fTime[elem] = t;
      fTimeOR[fNhits] = t;
    }
    fHits[fNhits] = elem;
    fNhits++;
  }

  fTotalEnergy = total;

  // terminate arrays
  fHits[fNhits] = EBufferEnd;
  fEnergyOR[fNhits] = EBufferEnd;
  if (fIsTime) fTimeOR[fNhits] = EBufferEnd;

  if(fIsRawHits)
  {
    fRawEnergyHits[0] = EBufferEnd;
    fRawTimeHits[0] = EBufferEnd;
  }
}

//---------------------------------------------------------------------------

#endif
