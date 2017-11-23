////////////////////////////////////////////////////////////////////////////
//                                                                        //
//     --Author		H. Berghaeuser June 2008                          //
//                      HenningBerghaeuser@web.de                         //
//     --Update         H.Berghaeuser Okt 2008                            //
//     --Update         H.Berghaeuser Dez 2008 -> TAPS2009LE Version      //
//     --Update         H.Berghaeuser FEB 2009 -> TAPS2009 Version        //
//     --Update         H.Berghaeuser June2009 -> TAPS2009GT Version      //
//     --Update         H.Berghaeuser Sept2009                            //
//     --Update         H.Berghaeuser Oktober2009                         //
//     --Update         H.Berghaeuser FEB 2010                            //
//                                                                        //
//     -- Use the TAPS.dat !                                              //
//                                                                        //
//      TA2Taps is a new TAPS Class for Acqu 4v2                          //
//      Read BaF2, Veto and  use Veto-dE                                  //
//                                                                        //
//      This is designed for AR_HB2v3                                     //
//      Please read TA2Taps.README                                        //
//                                                                        //
// -----------------------------------------------------------------------//
//                                                                        //
//            TA2Taps.h            Version 1.1                           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef __TA2Taps_h__
#define __TA2Taps_h__

#include "TA2TAPS_BaF2.h"
#include "TA2TAPS_Veto.h"
#include "TA2Calorimeter.h"
#include "TCutG.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TNtuple.h"
#include "TA2Particle.h"


class TA2Taps : public TA2Apparatus
{
 protected:
  //concerning class-management
  UInt_t fTapsRunStep;                  // step of running
  UInt_t fCalibVetoStep;                // step of Veto Energy calibration
  UInt_t fCalibBaF2Step;                // step of BaF2 Energy calibration
  UInt_t fCalibTimeBaF2Step;            // step of BaF2 Time calibration
  //Detectors
  TA2TAPS_BaF2* fBaF2;                  // BaF2-calorimeeter
  TA2Detector* fVeto;                   // Veto-Detectors
  // Concerning BaF2
  Int_t fBaF2_Ncluster;                 // Nr of BaF2 clusters in event
  UInt_t* id_clBaF2;                    // indices of hit clusters
  HitCluster_t** clBaF2;                // -> cluster structs
  HitCluster_t* cl;                     // cluster struct
  UInt_t clBaF2_Nhits;                  // crystal-hits inside a cluster
  UInt_t* clBaF2_elements;              // crystal-elements inside a cluster
  Float_t fTapsRingEnergycorr1[11];     // Energy Ring correction
  Float_t fTapsRingEnergycorr2[11];     // Energy Ring correction
  Double_t fTapsTimeshift;
  // Concerning Vetos
  Bool_t* fIsVCharged;                  // to save if hit is charged or not
  UInt_t fVeto_NHits;                   // # veto-hits in this event
  Int_t* fVeto_Hits;                    // all veto-elements that fired
  UInt_t fVetoIndex;                    // Store Veto-ID of Veto that fired (saved to TA2Particle objects)
  Double_t fVeto_dE;                    // to save Veto-dE
  Double_t fVeto_Time;                  // to save Veto-Prompt-Time
  Double_t fEnergy_BaF2;
  UInt_t fMultipleVetoHit;              // not in use
  Bool_t fCutOnVetoBaF2Time;            // Cut on VetoBaF2TimeWindo active or not ?
  Double_t fVetoTimeWindoMin;           // Min of Cut Window
  Double_t fVetoTimeWindoMax;           // Max of Cut Window
  Bool_t fDoFlightTimeCorrection;
  Double_t fTapsTargetDistance;         // Distance between Target &  BaF2_Front -> see configFile
  // Variables concerning reconstruction
  TLorentzVector* fP4_Nphoton;          // Total invariant mass N photons
  Double_t* fM_Nphoton;                 // Inv mass for N cluster totals
  TVector3 fp3;                         // used to make p4
  // reconstruction: make ID of particle:
  Bool_t* fdEvE_IsProton;               // array important for CheckParticleID
  Bool_t* fdEvE_IsChPion;               // array important for CheckParticleID
  Bool_t* fdEvE_IsElectron;             // array important for CheckParticleID
  Bool_t* fTOF_IsNucleon;               // array important for CheckParticleID
  Double_t* fIsClusterEnergy;           // energy of cluster for TOF info
  Double_t* fIsClusterTime;             // time of cluster for TOF info
  Double_t* fTOFproton;
  Double_t* fTOF;
  Double_t* fVetoPromptTime;
  Double_t* fVetoBaf2Time;
  Double_t* fDeltaE;
  Double_t* fEcharged;
  Double_t* fEchargedcl;
  Double_t* fHitMatrixY;                // for 2d Plot TAPS charged hit cluster
  Double_t* fHitMatrixX;                // for 2d Plot TAPS charged hit cluster
  TA2Particle* particles;               // use this to store particle-information
  Int_t  *fPDG_ID_sec;                  // to store particles second (maybe) id
  Bool_t *fPDG_ID_unclear;              // if fPDG_ID_sec is set then this should be true

  Bool_t fSimpleReconstruct;            // RUN Mode ?  read TAPS2009GT.dat
  Double_t fTapsFudge;                  // Sven Variable ... Out Of Use
  Double_t fTimeShift;                  // not in real use till now
  Int_t *fDetectedParticle;             // for spectrum of detected particles(IDs)
  UInt_t MaximumNrOfParticles;

  Bool_t fTAPS_dEvE_Active;
  TFile* fTAPS_dEvE_CutFile;
  char   fTAPS_dEvE_Cuts[256];
  Bool_t fTAPS_dEvE_Proton;
  TCutG* fTAPS_dEvE_ProtonCut;
  char   fTAPS_dEvE_Proton_CutName[64];
  Bool_t fTAPS_dEvE_ChPion;
  TCutG* fTAPS_dEvE_ChPionCut;
  char   fTAPS_dEvE_ChPion_CutName[64];
  Bool_t fTAPS_dEvE_Electron;
  TCutG* fTAPS_dEvE_ElectronCut;
  char   fTAPS_dEvE_Electron_CutName[64];

  TFile* fTAPS_TOF_CutFile;
  char   fTAPS_TOF_Cuts[256];
  TCutG* fTAPS_TOF_NucleonCut;
  char   fTAPS_TOF_Nucleon_CutName[64];
  Bool_t fTAPS_TOF_Nucleon;
  Double_t fTaggerTime;                  // Mean/Center Time of Taggers TimePromptPeak
  Bool_t fTAPS_TOF_onlyPhoton;

  // Calibration output;
  TH1F* calibTAPS_m1g_Single;
  TH2F* calibTAPS_m1g_AllCh;
  TH2F* calibTAPS_VetoCorr;
  TNtuple* VetoData;
  TH2F* calib_TAPSdEvEcl;
  TH2F* calib_TAPSdEvE;
  TH2F *calibTAPS_t2g_AllCh;
  TH1F *calibTAPS_time2g;

public:
  TA2Taps(const char*, TA2System*);
  virtual ~TA2Taps();
  virtual void PostInit();
  virtual void SetConfig(char* line, int key);
  virtual TA2DataManager* CreateChild(const char*, Int_t);
  virtual void MakeAllRootinos();
  virtual void LoadVariable();
  virtual void SimplePhotonReconstruct();
  virtual void Reconstruct();
  virtual void MainReconstruct();
  virtual TLorentzVector SetLorentzVector(Double_t, HitCluster_t*);
  virtual void Cleanup();
  virtual void SetParticleInfo(UInt_t NrParticle);

  TA2Particle* GetParticles(){ return particles; }                    // return array with all particles
  TA2Particle GetParticles(Int_t index){ return particles[index]; } // return a single particle
  Int_t GetNparticle(){ return fNparticle; }

  TA2ClusterDetector* GetCal(){ return fBaF2; }
  TA2Detector* GetVeto(){ return fVeto; }
  TLorentzVector* GetP4_Nphoton(){ return fP4_Nphoton; }
  Double_t* GetM_Nphoton(){ return fM_Nphoton; }
  Int_t CheckParticleID(UInt_t i);
  Double_t GetTapsTargetDistance(){ return fTapsTargetDistance;}
  Bool_t CheckVetoBaF2TimeIsPrompt(Double_t Time);
  Bool_t CheckTOF(Double_t Time, UInt_t NrParticle);      // old remains: from TA2Taps2009

  UInt_t GetVetoInFrontOfElement(UInt_t id) const;
  Double_t CheckClusterVeto(HitCluster_t* inCluster, UInt_t* outVetoIndex = 0) const;

  Bool_t   *GetfIsVCharged(){ return fIsVCharged; }
  Double_t *GetfIsClusterEnergy(){ return fIsClusterEnergy; }
  Double_t *GetfIsClusterTime(){ return fIsClusterTime; }

  UInt_t GetTAPSRing(UInt_t ClusterIndex);
  Double_t FlightTimeCorrection(UInt_t ClusterIndex);

  // ----- Calibration Functions ----------------------------
  virtual void CalibrateVetoEnergy();
  virtual void CalibrateBaF2Energy(TLorentzVector *photonCB);
  virtual void CalibrateBaF2Time();

  Float_t TAPS_GetCLInfo(UInt_t module, UInt_t TimeOrEnergy);

  ClassDef(TA2Taps,2)
};

//-----------------------------------------------------------------------------

inline void TA2Taps::Reconstruct()
{
  if(fSimpleReconstruct == kTRUE)
    SimplePhotonReconstruct();
  else
    MainReconstruct();

  fTapsRunStep = 2;
}

//-----------------------------------------------------------------------------

inline void TA2Taps::Cleanup()
{
  Int_t i;

  //Clear any arrays holding variables
  TA2DataManager::Cleanup();

  if(!fNparticle) return;
  for(i=0; i<fNparticle; i++)
  {
    fP4[i].SetXYZT(0.,0.,0.,0.);
    fMinv[i] = 0.0;
  }
  fP4_Nphoton[i-1].SetXYZT(0.,0.,0.,0.);
  fM_Nphoton[i-1] = 0.0;
  fNparticle = 0;
}

//-----------------------------------------------------------------------------

inline void TA2Taps::SimplePhotonReconstruct()
{
 // All in TAPS detected particles are reconstructed as PHOTONS
 // does absolutely the same as TA2Calorimeter does!
 // no VETO is used .... no dEvE is used and no TOF ! JUST BaF2 readout

  if(fTapsRunStep == 1)
  {
    printf("\n\nINFO: TAPS only reads BaF2s and 'makes' only photons !!!\n\n");
  }

  fNparticle = fBaF2->GetNCluster();
  if( !fNparticle ) return;
  id_clBaF2 = fBaF2->GetClustHit();
  clBaF2 = fBaF2->GetCluster();

  fP4tot.SetXYZT(0,0,0,0);
  Int_t i;
  for (i=0; i<fNparticle; i++)
  {
    cl = clBaF2[id_clBaF2[i]];
    fPDG_ID[i] = kGamma;
    fParticleID->SetP4(fP4+i,fPDG_ID[i],cl->GetEnergy(),cl->GetMeanPosition());
    fP4tot+=fP4[i];
    fMinv[i] = fP4[i].M();
    SetParticleInfo(i);
  }
  fP4_Nphoton[i-1]= fP4tot;
  fM_Nphoton[i-1]= fP4tot.M();
  fMtot = fP4tot.M();
}

//-----------------------------------------------------------------------------

inline TLorentzVector TA2Taps::SetLorentzVector(Double_t mass, HitCluster_t* cl)
{
  //Get cluster kinetic energy and particle direction
  //Construct 4 momentum using input assumed mass

  Double_t E = cl->GetEnergy() + mass;
  Double_t P = TMath::Sqrt(E*E - mass*mass);
  TLorentzVector p4;
  p4.SetE(E);
  p4.SetVect( (cl->GetMeanPosition())->Unit() * P );
  return p4;
}

//-----------------------------------------------------------------------------

inline Bool_t TA2Taps::CheckTOF(Double_t Time , UInt_t NrParticle)
{
  //Time  should be the time of a Tagger or CB gamma
  //NrParticle ..... you get by calling: proton.[i].GetParticleNumber();  but first ask if proton.[i].GetDetector() == 2 !!
  if( fTAPS_TOF_Nucleon == kTRUE)
  {
    return fTAPS_TOF_NucleonCut->IsInside( (particles[NrParticle].GetTime() + Time), fIsClusterEnergy[NrParticle]);
  }
  else
  {
    printf("\n TA2Taps: TOF nucleon cut is NOT active !\n");
    return fTAPS_TOF_Nucleon;
  }
}

//-----------------------------------------------------------------------------

inline Bool_t TA2Taps::CheckVetoBaF2TimeIsPrompt(Double_t Time)
{
  if(fCutOnVetoBaF2Time == kTRUE)
  {
    if( Time > fVetoTimeWindoMin && Time < fVetoTimeWindoMax) return kTRUE; else return kFALSE;
  }
  else
  {
    return kTRUE;
  }
}

//-----------------------------------------------------------------------------

inline UInt_t TA2Taps::GetVetoInFrontOfElement(UInt_t id) const
{
    // Return the index of the veto that is installed in front of the
    // BaF2 or PWO element with the index 'id'.
    
    // check TAPS setup
    switch (fBaF2->GetType())
    {
        case EBaF2:
        {
            return id;
        }
        case EBaF2_PWO_08:
        {
            // 1st PWO ring
            if (id >=   0 && id <=   3) return   0;
            if (id >=  67 && id <=  70) return  64;
            if (id >= 134 && id <= 137) return 128;
            if (id >= 201 && id <= 204) return 192;
            if (id >= 268 && id <= 271) return 256;
            if (id >= 335 && id <= 338) return 320;
            
            // other elements
            else return id - 3*(id/67 + 1);
        }
        case EBaF2_PWO_09:
        {
	    if ((fVeto->GetNelement()) == 384)
	    {
	        // 1st PWO ring
	        if (id >=   0 && id <=   3) return   0;
		if (id >=  73 && id <=  76) return  64;
		if (id >= 146 && id <= 149) return 128;
		if (id >= 219 && id <= 222) return 192;
		if (id >= 292 && id <= 295) return 256;
		if (id >= 365 && id <= 368) return 320;
            
		// 2nd PWO ring
		if (id >=   4 && id <=   7) return   1;
		if (id >=   8 && id <=  11) return   2;
		if (id >=  77 && id <=  80) return  65;
		if (id >=  81 && id <=  84) return  66;
		if (id >= 150 && id <= 153) return 129;
		if (id >= 154 && id <= 157) return 130;
		if (id >= 223 && id <= 226) return 193;
		if (id >= 227 && id <= 230) return 194;
		if (id >= 296 && id <= 299) return 257;
		if (id >= 300 && id <= 303) return 258;
		if (id >= 369 && id <= 372) return 321;
		if (id >= 373 && id <= 376) return 322;
		
		// other elements
		else return id - 9*(id/73 + 1);
	    }
	    else return id;
        }
        default:
        {
            Error("GetVetoInFrontOfElement", "TAPS setup could not be identified!");
            return 0;
        }
    }
}

//-----------------------------------------------------------------------------

// Use this function with Sven Schumanns Acqu
inline void TA2Taps::SetParticleInfo(UInt_t NrParticle)
{
  particles[NrParticle].Reset();
  particles[NrParticle].SetP4(fP4[NrParticle]);
  if(fBaF2->GetElement(id_clBaF2[NrParticle])->GetT1() > 0 || gAR->GetProcessType() == EMCProcess)
  {
      particles[NrParticle].SetTime(-(fBaF2->GetTime(id_clBaF2[NrParticle])) + fTapsTimeshift);
  }
  else
  {
      particles[NrParticle].SetTime((fBaF2->GetTime(id_clBaF2[NrParticle])) + fTapsTimeshift);
  }
  particles[NrParticle].SetParticleIDA(fPDG_ID[NrParticle]);
  particles[NrParticle].SetCentralIndex(id_clBaF2[NrParticle]);
  particles[NrParticle].SetApparatus(EAppTAPS);
  particles[NrParticle].SetDetector(EDetBaF2);
  particles[NrParticle].SetClusterSize(clBaF2[id_clBaF2[NrParticle]]->GetNhits());
  //particles[NrParticle].SetPDG(fPDG_ID[NrParticle]);
  if(fIsVCharged[NrParticle] == kFALSE) //fIsVCharged ... V is for VETO
  {
      particles[NrParticle].SetVetoEnergy(0.0);
  }
  else
  {
      particles[NrParticle].SetVetoEnergy(fVeto_dE);
      //particles[NrParticle].SetIsCharged(fIsVCharged[NrParticle]);
      particles[NrParticle].SetDetector(EDetVeto);
      particles[NrParticle].SetVetoIndex(fVetoIndex );
      if(fVeto->GetElement(fVetoIndex)->GetT1() > 0 || gAR->GetProcessType() == EMCProcess)
      {
          particles[NrParticle].SetVetoTime(-fVeto_Time);
      }
      else
      {
          particles[NrParticle].SetVetoTime(fVeto_Time);
      }
  }
  particles[NrParticle].SetPSAShort(fBaF2->GetSGEnergy(id_clBaF2[NrParticle]));

  particles[NrParticle].SetSigmaE(fBaF2->GetEnergyResolution(clBaF2[id_clBaF2[NrParticle]]->GetEnergy()));
  particles[NrParticle].SetSigmaPhi(fBaF2->GetPhiResolution());
  particles[NrParticle].SetSigmaTheta(fBaF2->GetThetaResolution());
  particles[NrParticle].SetSigmaTime(fBaF2->GetTimeResolution());
}

inline UInt_t TA2Taps::GetTAPSRing(UInt_t ClusterIndex)
{
  UInt_t nRing = 0;
  UInt_t j = ClusterIndex + 1;

  j = j % 64;
  if(j==1)           nRing = 1;
  if(j==2 || j==3)   nRing = 2;
  if(j>=4 && j<=6)   nRing = 3;
  if(j>=7 && j<=10)  nRing = 4;
  if(j>=11 && j<=15) nRing = 5;
  if(j>=16 && j<=21) nRing = 6;
  if(j>=22 && j<=28) nRing = 7;
  if(j>=29 && j<=36) nRing = 8;
  if(j>=37 && j<=45) nRing = 9;
  if(j>=46 && j<=55) nRing = 10;
  if(j>=56 && j<=64) nRing = 11;
  if(j==0)           nRing = 11;
  return nRing;
}

//-----------------------------------------------------------------------------

inline Double_t TA2Taps::FlightTimeCorrection(UInt_t ClusterIndex)
{

  Double_t  FlightTimeCorrectionFactor = 1.0;

  if(fDoFlightTimeCorrection == kTRUE)
  {
    UInt_t ring = GetTAPSRing(ClusterIndex);
    FlightTimeCorrectionFactor = sqrt(pow(((Double_t)(ring)) * 6.00, 2) + pow(fTapsTargetDistance, 2))/fTapsTargetDistance;
  }

  return FlightTimeCorrectionFactor;
}

//-----------------------------------------------------------------------------

#endif

