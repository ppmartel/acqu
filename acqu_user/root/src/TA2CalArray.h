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
//#include <iostream>

#define MAXRUNS 2047

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
  Double_t  fSigmaTheta;            // Theta resolution for CB
  Double_t  fSigmaPhi;              // Phi Resolution for CB

  //For run-by-run energy scale factor handling:
  Bool_t UseScales;
  Int_t iRun;
  Int_t ScaleRuns;
  Char_t RunName[1024];
  Char_t CurrentRun[1024];
  Char_t ScaleFile[1024];
  Char_t ScaleRun[MAXRUNS+1][256];
  Double_t ScaleVal[MAXRUNS+1];
  Double_t fEnergyGlobal;

 public:
  TA2CalArray(const char*, TA2System*);// Normal use
  virtual ~TA2CalArray();
  virtual void PostInit();             // initialise using setup info
  virtual void ParseDisplay(char*);    // display setup
  virtual void Decode();               // hits -> energy procedure
  virtual void SaveDecoded();          // save local analysis
  virtual void ReadDecoded();          // read back previous analysis
  virtual void DecodeSaved( );         // decode previously written data
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
  ClassDef(TA2CalArray,1)
};

//---------------------------------------------------------------------------

inline void TA2CalArray::DecodeSaved()
{
  // Run basic TA2Detector decode, then cluster decode
  // and then histogram
  TA2ClusterDetector::DecodeSaved();
}

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

  if(UseScales)
  {
    //Get name of file being analysed
    gUAN->ReadRunName(RunName);
    //Check if file name has changed since last event (i.e. are we analysing a new file)
    if(strcmp(RunName, CurrentRun))
    {
      strcpy(CurrentRun, RunName);
      //Find current run in energy scale table
      for(iRun=0; iRun<ScaleRuns; iRun++)
        if(!strcmp(RunName, ScaleRun[iRun])) break;
      //Apply additional run-dependent correction for global energy scale value
      fEnergyScale = fEnergyGlobal*ScaleVal[iRun];
    }
  }

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
  // Read Crystal Ball energies from  GEANT-3 simulation output
  // See MCBranchID.h for GEANT-3 output details.
  // Add energy thresholds 25/10/05
  // D.Glazier addition of time decoding from A2 Geant4 model 24/08/07

  if(UseScales)
  {
    //Get name of file being analysed
    gUAN->ReadRunName(RunName);
    //Check if file name has changed since last event (i.e. are we analysing a new file)
    if(strcmp(RunName, CurrentRun))
    {
      strcpy(CurrentRun, RunName);
      //Find current run in energy scale table
      for(iRun=0; iRun<ScaleRuns; iRun++)
        if(!strcmp(RunName, ScaleRun[iRun])) break;
      //Apply additional run-dependent correction for global energy scale value
      fEnergyScale = fEnergyGlobal*ScaleVal[iRun];
    }
  }

  Double_t T, E;
  Double_t Lo, Hi;
  UInt_t k = 0;
  Int_t j;
  Int_t* Index;
  Float_t* Energy;
  Float_t* Time = NULL;

  fNhits = *(Int_t*)(fEvent[EI_nhits]);                 //# crystals fired
  Index = (Int_t*)(fEvent[EI_icryst]);                  //Crystal indices
  Energy = (Float_t*)(fEvent[EI_ecryst]);               //Energies in crystals
  if(fIsTime) Time = (Float_t*)(fEvent[EI_tcryst]);     //Time in crystals

  for(UInt_t t=0; t<fNelem; t++)
    fEnergyAll[t] = 0.0;
  fTotalEnergy = 0.0;

  for(UInt_t i=0; i<fNhits; i++) //Loop over hits
  {
    //Slightly less ugly decoding of icryst
    j = Index[i] % 10000;                                //AcquRoot index
    if(j==-1) continue;                                  //Check spurious index

    E = Energy[i] * fEnergyScale;                        //G3/4 output in GeV
    if(fUseSigmaEnergy) E+=fRandom->Gaus(0.0, GetSigmaEnergyGeV(E));
    E*=1000.0;                                           //G3/4 output in MeV
    fEnergyAll[j] = E;
    Lo = fElement[j]->GetEnergyLowThr();
    Hi = fElement[j]->GetEnergyHighThr();
    //Lo = fElement[j]->GetADCcut()->GetLowThr();
    //Hi = fElement[j]->GetADCcut()->GetHighThr();
    if((E < Lo) || (E > Hi)) continue;
    fEnergy[j] = E;                                      //Save energy
    fEnergyOR[k] = E;
    fTotalEnergy+=E;

    if(fIsTime)
    {
      //T = Time[i] - 1.6;
      T = Time[i];
      if(fUseSigmaTime) T+=fRandom->Gaus(0.0, fSigmaTime);
      Lo = fElement[j]->GetTimeLowThr();
      Hi = fElement[j]->GetTimeHighThr();
      //Lo = fElement[j]->GetTDCcut()->GetLowThr();
      //Hi = fElement[j]->GetTDCcut()->GetHighThr();
      if((T < Lo) || (T > Hi)) continue;
      fTime[j] = T;
      fTimeOR[k] = T;
    }

    fHits[k] = j;                                        //Store hit
    k++;                                                 //Update # good hits
  }
  fNhits = k;                                            //# hits inside thresh
  fHits[k] = EBufferEnd;                                 //End markers
  fEnergyOR[k] = EBufferEnd;
  if(fIsTime) fTimeOR[k] = EBufferEnd;

  if(fIsRawHits)
  {
    fRawEnergyHits[0] = EBufferEnd;
    fRawTimeHits[0] = EBufferEnd;
  }
}

//---------------------------------------------------------------------------

#endif
