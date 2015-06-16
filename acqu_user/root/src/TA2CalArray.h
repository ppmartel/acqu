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
#include "HitClusterUCLA_t.h"
#include "TA2ClusterDetector.h"
#include "TA2UserControl.h"
//#include <iostream>

class TA2CalArray : public TA2ClusterDetector
{
 private:
 protected:
  HitClusterUCLA_t** fClusterUCLA;  // Cluster pointer for UCLA Clustering
  Double_t* fEnergyAll;
  UInt_t    fUseSigmaEnergy;        // Use energy resoution smearing for MC?
  UInt_t    fUseSigmaTime;          // Use time resolution smearing in MC?
  UInt_t    fUseClusterDecodeUCLA;  // Use Cluster Decode by S. Prakhov - Not fully tested!
  Double_t  fSigmaEnergyFactor;     // Factor in Energy Resolution Equation
  Double_t  fSigmaEnergyPower;      // Power in energy Resolution Equation
  Double_t  fSigmaTime;             // Sigma for time resolution
  Double_t  fSigmaTheta;            // Theta resolution for CB
  Double_t  fSigmaPhi;              // Phi Resolution for CB
  UInt_t*   fTryHits;               // Element-Hit store - Sergey
  UInt_t*   fTempHits2;             // Element-Hit store - Sergey

 public:
  TA2CalArray(const char*, TA2System*);// Normal use
  virtual ~TA2CalArray();
  virtual void PostInit();             // initialise using setup info
  virtual void ParseDisplay(char*);    // display setup
  virtual void Decode();               // hits -> energy procedure
  virtual void SaveDecoded();          // save local analysis
  virtual void ReadDecoded();          // read back previous analysis
  virtual void DecodeClusterUCLA();    // UCLA DecodeCluster() version - UNTESTED!!!!!
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
  HitCluster_t** GetClusterUCLA()        { return (HitCluster_t**)fClusterUCLA; }
  HitCluster_t*  GetClusterUCLA(UInt_t i){ return (HitCluster_t*)fClusterUCLA[i]; }
  UInt_t    IsUCLAClustering()           { return fUseClusterDecodeUCLA; }
  UInt_t*   GetTempHits2()               { return fTempHits2; }
  Double_t GetClusterThreshold()	 { return fClEthresh; } //*** 
 ClassDef(TA2CalArray,1)
};

//---------------------------------------------------------------------------

inline void TA2CalArray::DecodeSaved()
{
  // Run basic TA2Detector decode, then cluster decode
  // and then histogram
  if(fUseClusterDecodeUCLA) {
    ReadDecoded();
    DecodeClusterUCLA();
  }
  else TA2ClusterDetector::DecodeSaved();
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

inline void TA2CalArray::DecodeClusterUCLA()
{
  // Determine clusters of hits
  // Search around peak energies absorbed in individual crystals
  // Make copy of hits array as the copy will be altered

/*   std::cout<<"Beginning DecodeClusterUCLA"<<std::endl; */
  memcpy( fTempHits, fHits, sizeof(UInt_t)*fNhits );  // temp copy

  const Double_t fEthcrs_CB = 4., fEthcls_CB = 12.;
  const Double_t fEthcrs_TAPS = 4., fEthcls_TAPS = 12.;
  const Double_t fEthcls2_CB = 50., fEthcls2_TAPS = 50.;
  const Double_t opangl = 32., difmax = 24.;
  Double_t fEthcrs, fEthclsi, fEthcls, fEthcls2, thet, phi, Ecl, Ecli, oang;

  Double_t maxenergy;
  UInt_t i, j, k, m, ntaken, nc;
  UInt_t kmax = 0;
  UInt_t jmax = 0;
  //UInt_t nwid=0,
  //UInt_t indwid[15]={15*0},
  UInt_t nomit=0;
  TVector3 vcl, vcln, vdif;

  /*static Int_t ifirst = 0;
  char hiname[100];

  if(ifirst==0)
  {
    ifirst = 1;
    for(i=0; i<720; i++)
    {
      sprintf(hiname,"h_energyor_timeor_%d",i);
      htmp[i] = new TH2F(hiname,hiname,200,-50.,150.,500,0.,500.);
    }
  }
  */
  fNCluster = 0;
  if(fNhits>250 || fNhits<1) goto OUT;
/*   std::cout<<"here in DecodeClusterUCLA"<<std::endl; */
  if(fNelement == 720)
  {
    fEthcrs=fEthcrs_CB; fEthcls=fEthcls_CB; fEthcls2=fEthcls2_CB;
    /*
    for(j=0; j<fNhits; j++)
    {
      k = fTempHits[j];
      if(k>=0 && k < 720) htmp[k]->Fill(fTime[k], fEnergy[k]);
    }
    */
  }
  else
  {
    fEthcrs=fEthcrs_TAPS; fEthcls=fEthcls_TAPS; fEthcls2=fEthcls2_TAPS;
  }
  // Find hit with maximum energy

  for(m=0; m<fNhits; m++) fTryHits[m] = fTempHits[m];

  for(i=0; i<fMaxCluster; )
  {
    maxenergy = 0;
    for(j=0; j<fNhits; j++)
    {
      if((k = fTryHits[j])  == ENullHit) continue;
      if((k = fTempHits[j]) == ENullHit) continue;
      if((maxenergy < fEnergy[k]) && (fEnergy[k] > fEthcrs))
      {
	maxenergy = fEnergy[k];
	kmax = k;
	jmax = j;
      }
    }
    if(maxenergy==0) break;              // no more isolated hits
    if(kmax < fNelement)
    {
      for(m=0; m<fNhits; m++) fTempHits2[m] = fTempHits[m];
      //std::cout<<"here2 in DecodeClusterUCLA"<<std::endl;
      fClusterUCLA[kmax]->ClusterDetermineUCLA((TA2CalArray*)this); // determine the cluster
      Ecl = fClusterUCLA[kmax]->GetEnergy();
      if(Ecl>=fEthcls)
      {
        if ( Ecl < fEthcls2 && fNCluster>0 )
        {
          for (j=0; j<fNCluster; j++)
          {
	    Ecli = fClEnergyOR[j];
	    fEthclsi = 25.+ 25. * Ecli / 1000.;
            thet = fTheta[j] * TMath::DegToRad();
            phi  = fPhi[j] * TMath::DegToRad();
	    vcl.SetMagThetaPhi ( 146., thet, phi );
            thet = fClusterUCLA[kmax]->GetTheta() * TMath::DegToRad();
            phi  = fClusterUCLA[kmax]->GetPhi() * TMath::DegToRad();
	    vcln.SetMagThetaPhi( 146., thet, phi );
	    if ( fNelement==720 )
            {
	      oang = vcl.Angle(vcln)*TMath::RadToDeg();
	      if ( oang < opangl && Ecl < fEthclsi )
              {
		nomit++;
		fTryHits[jmax] = ENullHit;
		goto NEXTCR;
	      }
	    }
	    else
            {
	      vdif = vcl - vcln;
	      if((vdif.Mag() < difmax)  && (Ecl < fEthclsi))
              {
		nomit++;
		fTryHits[jmax] = ENullHit;
		goto NEXTCR;
	      }
	    }
	  }
	}
        for(m=0; m<fNhits; m++) fTempHits[m] = fTempHits2[m];
	fClustHit[i] = kmax;
	fTheta[i] = fClusterUCLA[kmax]->GetTheta();
	fPhi[i] = fClusterUCLA[kmax]->GetPhi();
	fNClustHitOR[i] = fClusterUCLA[kmax]->GetNhits();
	fClEnergyOR[i] = Ecl;
	fClRadiusOR[i] = fClusterUCLA[kmax]->ClusterRadiusUCLA(this);
	i++;
	fNCluster = i;
      }
      else fTryHits[jmax] = ENullHit;
    }
    // If you reach here then there is an error in the decode
    // possible bad detector ID
    else fTryHits[jmax] = ENullHit;
  NEXTCR: continue;
  }
 OUT:
  fClustHit[fNCluster] = EBufferEnd;
  fTheta[fNCluster] = EBufferEnd;
  fPhi[fNCluster] = EBufferEnd;
  fNClustHitOR[fNCluster] = EBufferEnd;
  fClEnergyOR[fNCluster] = EBufferEnd;
  fClRadiusOR[fNCluster] = EBufferEnd;

  if(fNCluster==0) return;

  ntaken=0;
  for(m=0; m<fNhits; m++)
  {
    fTempHits2[m] = fTempHits[m];
    if(fTempHits2[m]==ENullHit) ntaken++;
  }
  if(ntaken==fNhits) return;

  for(j=0; j<fNCluster; j++)
  {
    kmax = fClustHit[j];
    if(fClusterUCLA[kmax]->ClusterDetermineUCLA2(this)) // the wider cluster
    {
      fTheta[j] = fClusterUCLA[kmax]->GetTheta();
      fPhi[j] = fClusterUCLA[kmax]->GetPhi();
      fNClustHitOR[j] = fClusterUCLA[kmax]->GetNhits();
      fClEnergyOR[j] = fClusterUCLA[kmax]->GetEnergy();
      fClRadiusOR[j] = fClusterUCLA[kmax]->ClusterRadiusUCLA(this);
    }
  }

  if(nomit==0) return;
  ntaken = 0;
  for(m=0; m<fNhits; m++)
  {
    if(fTempHits2[m]==ENullHit) ntaken++;
    fTryHits[m] = fTempHits[m] = fTempHits2[m];
  }
  if(ntaken==fNhits) return;

  nc = fNCluster;
  for( i=nc; i<fMaxCluster; )
  {
    maxenergy = 0;
    for( j=0; j<fNhits; j++ )
     {
      if((k = fTryHits[j])==ENullHit) continue;
      if((k = fTempHits[j])==ENullHit) continue;
      if((maxenergy < fEnergy[k]) && (fEnergy[k] > fEthcrs))
      {
	maxenergy = fEnergy[k];
	kmax = k;
	jmax = j;
      }
    }
    if(maxenergy==0) break;              // no more isolated hits
    if(kmax < fNelement)
    {
      for(m=0;m<fNhits;m++) fTempHits2[m] = fTempHits[m];
      fClusterUCLA[kmax]->ClusterDetermineUCLA(this); // determine the cluster
      Ecl = fClusterUCLA[kmax]->GetEnergy();
      if(Ecl>=fEthcls)
      {
        for(m=0;m<fNhits;m++) fTempHits[m] = fTempHits2[m];
	fClustHit[i] = kmax;
	fTheta[i] = fClusterUCLA[kmax]->GetTheta();
	fPhi[i] = fClusterUCLA[kmax]->GetPhi();
	fNClustHitOR[i] = fClusterUCLA[kmax]->GetNhits();
	fClEnergyOR[i] = Ecl;
	fClRadiusOR[i] = fClusterUCLA[kmax]->ClusterRadiusUCLA(this);
	i++;
	fNCluster = i;
      }
      else fTryHits[jmax] = ENullHit;
    }
    // If you reach here then there is an error in the decode
    // possible bad detector ID
    else fTryHits[jmax] = ENullHit;
  }
  fClustHit[fNCluster] = EBufferEnd;
  fTheta[fNCluster] = EBufferEnd;
  fPhi[fNCluster] = EBufferEnd;
  fNClustHitOR[fNCluster] = EBufferEnd;
  fClEnergyOR[fNCluster] = EBufferEnd;
  fClRadiusOR[fNCluster] = EBufferEnd;

  if(nc==fNCluster) return;
  ntaken = 0;
  for(m=0; m<fNhits; m++) if(fTempHits2[m]==ENullHit) ntaken++;
  if(ntaken==fNhits) return;

  for (j=nc; j<fNCluster; j++ )
  {
    kmax = fClustHit[j];
    if(fClusterUCLA[kmax]->ClusterDetermineUCLA2(this))  // the wider cluster
    {
      fTheta[j] = fClusterUCLA[kmax]->GetTheta();
      fPhi[j] = fClusterUCLA[kmax]->GetPhi();
      fNClustHitOR[j] = fClusterUCLA[kmax]->GetNhits();
      fClEnergyOR[j] = fClusterUCLA[kmax]->GetEnergy();
      fClRadiusOR[j] = fClusterUCLA[kmax]->ClusterRadiusUCLA(this);
    }
  }
}

//---------------------------------------------------------------------------

inline void TA2CalArray::Decode()
{
  // Decode the NaI ADC and TDC information
  // Decode raw TDC and Scaler information into
  // Hit pattern, "Energy" pattern, aligned OR etc.

  if(fUseClusterDecodeUCLA)
  {
    TA2ClusterDetector::DecodeBasic();
    DecodeClusterUCLA();
  }
  else TA2ClusterDetector::Decode();

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
