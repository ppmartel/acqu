////////////////////////////////////////////////////////////////////////////
//                                                                        //
//     --Author		H. Berghaeuser June 2008                          //
//                      HenningBerghaeuser@web.de                         //
//     -- Update        H. Berghaeuser Okt 2008                           //
//     -- Update        H. Berghaeuser Dez 2008                           //
//     -- Update        H. Berghaeuser FEB 2009                           //
//     -- Update        H. Berghaeuser July 2009                          //
//      --Update        H.Berghaeuser Oktober2009                         //
//      --Update        H.Berghaeuser Feb 2010       			  //
//      --Update	C.Collicott	Dec 2012 	                  //
//                                                                        //
//     -- Use the TAPS.da !                                               //
//                                                                        //
//            TA2Taps.cc       version 1.1                                //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#include "TA2Taps.h"
#include "TA2Analysis.h"

//-----------------------------------------------------------------------------

enum { ETAPS_BaF2=6000, ETAPS_Veto };

enum { ETAPSFactor=6002, ETAPS_SimpleReconstruct, ETAPS_dEvE_Active, ETAPS_dEvE_Cuts,
       ETAPS_dEvE_Proton, ETAPS_dEvE_Proton_CutName,
       ETAPS_dEvE_ChPion, ETAPS_dEvE_ChPion_CutName,
       ETAPS_dEvE_Electron, ETAPS_dEvE_Electron_CutName,
       ETAPS_TOF_Cuts,ETAPS_TOF_Nucleon, ETAPS_TOF_Nucleon_CutName, ETAPS_TOF_TaggerTime,
       ETapsTargetDistance, ETAPSVetoBaF2TimeCut, ETAPSDoFlightTimeCorrection, ETAPS_TOF_onlyPhoton,
       ETAPSRingEnergycorr1,ETAPSRingEnergycorr2,
       ETAPSTimeshift };

static const Map_t kTapsKeys[] = {
  {"TAPS-Factor:",                ETAPSFactor},
  {"TAPS_SimpleReconstruct:",     ETAPS_SimpleReconstruct},
  {"TAPS_dEvE_Active:",           ETAPS_dEvE_Active},
  {"TAPS_dEvE_Cuts:",             ETAPS_dEvE_Cuts},
  {"TAPS_dEvE_Proton:",           ETAPS_dEvE_Proton},
  {"TAPS_dEvE_Proton_CutName:",   ETAPS_dEvE_Proton_CutName},
  {"TAPS_dEvE_ChPion:",           ETAPS_dEvE_ChPion},
  {"TAPS_dEvE_ChPion_CutName:",   ETAPS_dEvE_ChPion_CutName},
  {"TAPS_dEvE_Electron:",         ETAPS_dEvE_Electron},
  {"TAPS_dEvE_Electron_CutName:", ETAPS_dEvE_Electron_CutName},
  {"TAPS_TOF_Cuts:",              ETAPS_TOF_Cuts},
  {"TAPS_TOF_Nucleon:",           ETAPS_TOF_Nucleon},
  {"TAPS_TOF_Nucleon_CutName:",   ETAPS_TOF_Nucleon_CutName},
  {"TAPS_TOF_TaggerTime:",        ETAPS_TOF_TaggerTime},
  {"TAPS_TOF_onlyPhoton:",        ETAPS_TOF_onlyPhoton},
  {"TAPSTargetDistance:",         ETapsTargetDistance},
  {"TAPSVetoBaF2TimeCut:",        ETAPSVetoBaF2TimeCut},
  {"TAPSDoFlightTimeCorrection:", ETAPSDoFlightTimeCorrection},
  {"TAPSRingEnergyCorr1:",        ETAPSRingEnergycorr1},
  {"TAPSRingEnergyCorr2:",        ETAPSRingEnergycorr2},
  {"TAPSTimeshift:",              ETAPSTimeshift},
  {NULL,            -1}
};

static Map_t kValidTapsDetectors[] = {
  {"TA2TAPS_BaF2", ETAPS_BaF2},
  {"TA2TAPS_Veto", ETAPS_Veto},
  {NULL,                -1}
};


//-----------------------------------------------------------------------------

TA2Taps::TA2Taps(const char* name, TA2System* analysis)
        :TA2Apparatus( name, analysis, kValidTapsDetectors )
{
  fTapsRunStep        = 0;
  fCalibVetoStep      = 0;
  fCalibBaF2Step      = 0;
  fCalibTimeBaF2Step  = 0;
  fBaF2               = NULL;
  fVeto               = NULL;
  fDeltaE             = NULL;
  fEcharged           = NULL;
  fEchargedcl         = NULL;
  fVetoPromptTime     = NULL;
  fTOFproton          = NULL;
  fTOF                = NULL;
  fVetoBaf2Time       = NULL;
  fTAPS_TOF_onlyPhoton= kTRUE;
  fSimpleReconstruct  = kTRUE;      // default -> will be changed/set via ConfigFile
  fTAPS_dEvE_Active   = kFALSE;     // default -> will be changed/set via ConfigFile
  fTAPS_dEvE_Proton   = kFALSE;     // default -> will be changed/set via ConfigFile
  fTAPS_dEvE_ChPion   = kFALSE;     // default -> will be changed/set via ConfigFile
  fTAPS_dEvE_Electron = kFALSE;     // default -> will be changed/set via ConfigFile
  fTimeShift          = 0.0;        // maybe will later use SvensTimeShift
  fTapsFudge          = 1.0;        // not in use
  fTapsTargetDistance = 146.0;      // normal TAPS-Target Distance
  fTapsTimeshift      = 0.0;
  fDoFlightTimeCorrection=kFALSE;   // default -> chenge via ConfigFile
  fTaggerTime         = 0.0;        // center of TaggerTimePromptPeak ->via ConfigFile
  fCutOnVetoBaF2Time  = kFALSE;     // default: change viaConfigFile
  fVetoTimeWindoMin   = 0.0;        // default: change viaConfigFile
  fVetoTimeWindoMax   = 0.0;        // default: change viaConfigFile

  AddCmdList( kTapsKeys );

  //printf("-- TA2Taps1v1................ done\n");

  MaximumNrOfParticles = 12; //only in use for AR_HB2v3
}

//-----------------------------------------------------------------------------

TA2Taps::~TA2Taps()
{    // Free up allocated memory
  if (fParticleID)      delete fParticleID;
  if(fDeltaE)           delete fDeltaE;
  if(fEcharged)         delete fEcharged;
  if(fEchargedcl)       delete fEchargedcl;
  if(fHitMatrixX)       delete fHitMatrixX;
  if(fHitMatrixY)       delete fHitMatrixY;
  if(fVetoPromptTime)   delete fVetoPromptTime;
  if(fTOF)              delete fTOF;
  if(fTOFproton)        delete fTOFproton;
  if(fVetoBaf2Time)     delete fVetoBaf2Time;
  if(fP4_Nphoton)       delete fP4_Nphoton;
  if(fM_Nphoton)        delete fM_Nphoton;
  if(particles)         delete particles;
  if(fIsVCharged)       delete fIsVCharged;
  if(fdEvE_IsProton)    delete fdEvE_IsProton;
  if(fdEvE_IsChPion)    delete fdEvE_IsChPion;
  if(fTOF_IsNucleon)    delete fTOF_IsNucleon;
  if(fdEvE_IsElectron)  delete fdEvE_IsElectron;
  if(fIsClusterEnergy)  delete fIsClusterEnergy;
  if(fIsClusterTime)    delete fIsClusterTime;
  if(fDetectedParticle) delete fDetectedParticle;
}

//-----------------------------------------------------------------------------

TA2DataManager*  TA2Taps::CreateChild(const char* name, Int_t dclass)
{

  if(!name) name = Map2String(dclass);
  switch (dclass)
  {
   case ETAPS_BaF2:
    fBaF2 = new TA2TAPS_BaF2(name, this);
    return fBaF2;
   case ETAPS_Veto:
    fVeto = new TA2TAPS_Veto(name, this);
    return fVeto;
   default:
    return NULL;
  }
}

//-----------------------------------------------------------------------------

void TA2Taps::LoadVariable( )
{
  //                            name                pointer             type-spec
  TA2DataManager::LoadVariable("DeltaE",             fDeltaE,           EDSingleX);
  TA2DataManager::LoadVariable("Echarged",           fEcharged,         EDSingleX);
  TA2DataManager::LoadVariable("Echargedcl",         fEchargedcl,       EDSingleX);
  TA2DataManager::LoadVariable("HitMatrix_X",        fHitMatrixX ,      EDSingleX);
  TA2DataManager::LoadVariable("HitMatrix_Y",        fHitMatrixY ,      EDSingleX);
  TA2DataManager::LoadVariable("VetoPromptTime",     fVetoPromptTime ,  EDSingleX);
  TA2DataManager::LoadVariable("TOFparticle",        fTOF,              EDSingleX);
  TA2DataManager::LoadVariable("TOFproton",          fTOFproton,        EDSingleX);
  TA2DataManager::LoadVariable("VetoBaf2Time",       fVetoBaf2Time ,    EDSingleX);
  TA2DataManager::LoadVariable("DetectedParticleIDs",fDetectedParticle, EISingleX);

  TA2Apparatus::LoadVariable();
}

//-----------------------------------------------------------------------------

void TA2Taps::PostInit()
{
 if( !fParticleID )
    PrintError("", "<Configuration aborted: ParticleID class MUST be specified>", EErrFatal);

  fMaxParticle      = fBaF2->GetMaxCluster();
  fDeltaE           = new Double_t[fMaxParticle+1];
  fEcharged         = new Double_t[fMaxParticle+1];
  fEchargedcl       = new Double_t[fMaxParticle+1];
  fVetoPromptTime   = new Double_t[fMaxParticle+1];
  fTOFproton        = new Double_t[fMaxParticle+1];
  fTOF              = new Double_t[fMaxParticle+1];
  fHitMatrixX       = new Double_t[fMaxParticle+1];
  fHitMatrixY       = new Double_t[fMaxParticle+1];
  fVetoBaf2Time     = new Double_t[fMaxParticle+1];
  fTapsRunStep      = 1;
  fP4_Nphoton       = new TLorentzVector[fMaxParticle+1];
  fM_Nphoton        = new Double_t[fMaxParticle+1];
  particles         = new TA2Particle[fMaxParticle+1];
  fIsClusterEnergy  = new Double_t[fMaxParticle+1];
  fIsClusterTime    = new Double_t[fMaxParticle+1];
  fIsVCharged       = new Bool_t[fMaxParticle+1];
  fdEvE_IsProton    = new Bool_t[fMaxParticle+1];
  fdEvE_IsChPion    = new Bool_t[fMaxParticle+1];
  fdEvE_IsElectron  = new Bool_t[fMaxParticle+1];
  fTOF_IsNucleon    = new Bool_t[fMaxParticle+1];
  fDetectedParticle = new Int_t[fMaxParticle+1];

  TA2Apparatus::PostInit();
}

//-----------------------------------------------------------------------------

void TA2Taps::MakeAllRootinos()
{
  fMultipleVetoHit = 0;
  fVeto_dE         = 0.0;
  fEnergy_BaF2     = 0.0;

  //set all array-elements to 0, 0.0 or kFALSE
  for(Int_t j=0;j<fMaxParticle;j++)
  {
    fIsVCharged[j]       = kFALSE;
    fdEvE_IsProton[j]    = kFALSE;
    fdEvE_IsChPion[j]    = kFALSE;
    fdEvE_IsElectron[j]  = kFALSE;
    fIsClusterEnergy[j]  = 0.0;
    fIsClusterTime[j]    = 0.0;
    fTOF_IsNucleon[j]    = kFALSE;
    fDeltaE[j]           = EBufferEnd;
    fEcharged[j]         = EBufferEnd;
    fEchargedcl[j]       = EBufferEnd;
    fVetoPromptTime[j]   = EBufferEnd;
    fTOFproton[j]        = EBufferEnd;
    fTOF[j]              = EBufferEnd;
    fHitMatrixX[j]       = EBufferEnd;
    fHitMatrixY[j]       = EBufferEnd;
    fVetoBaf2Time[j]     = EBufferEnd;
    fDetectedParticle[j] = EBufferEnd;
  }

  // first all particles are rootinos!
  for(Int_t i=0; i<fNparticle; i++)
  {
    cl = clBaF2[id_clBaF2[i]];
    fp3 = *((clBaF2[id_clBaF2[i]])->GetMeanPosition());
    fp3.SetX(fp3.X()); // + PosShift[1]);
    fp3.SetY(fp3.Y()); // + PosShift[2]);
    fp3.SetZ(fp3.Z()); // + PosShift[3]);
    fParticleID->SetP4(&fP4[i], kRootino, (fTapsFudge*(clBaF2[id_clBaF2[i]])->GetEnergy()), &fp3);
    fPDG_ID[i] = kRootino;
  }

 for( Int_t i=0; i<fNparticle; i++ )
 {
   fDeltaE[i]   = (Double_t)ENullHit;  //not needed anymore... but i leave it here
   fEcharged[i] = (Double_t)ENullHit;  //not needed anymore... but i leave it here
   fEchargedcl[i] = (Double_t)ENullHit;  //not needed anymore... but i leave it here
 }
}


//-----------------------------------------------------------------------------
void TA2Taps::MainReconstruct()
{
  fBaF2_Ncluster = fBaF2->GetNCluster();
  fNparticle     = fBaF2_Ncluster;
  if(!fNparticle) return;

  id_clBaF2   = fBaF2->GetClustHit();
  clBaF2      = fBaF2->GetCluster();
  fVeto_NHits = fVeto->GetNhits();
  fVeto_Hits  = fVeto->GetHits();

  MakeAllRootinos();  // first: all detected particles are made rootinos
  UInt_t clindex = 0;
  fVetoIndex = 0;

  //loop over all BaF2-Clusters
  for(Int_t i=0; i<fBaF2_Ncluster; i++)
  {
    cl              = clBaF2[id_clBaF2[i]];         // i-th cluster
    clBaF2_Nhits    = cl->GetNhits();               // # hits of i-th cluster
    clBaF2_elements = cl->GetHits();                // elements (nr. of each crystal) of i-th cluster
    clindex         = (UInt_t)cl->GetIndex();       // get index-crystal of actual cluster
    fVeto_dE        = 0.0;
    fVeto_Time      = 0.0;
    // fIsClusterEnergy[i] = cl->GetEnergy();           // without ring-correction
    fIsClusterEnergy[i] = (cl->GetEnergy()) * fTapsRingEnergycorr1[GetTAPSRing(clindex)-1]
                        + pow(cl->GetEnergy(), 2) * fTapsRingEnergycorr2[GetTAPSRing(clindex)-1] ;
    fIsClusterTime[i]   = fBaF2->GetTime(id_clBaF2[i]) * FlightTimeCorrection(clindex);

    fVeto_dE = CheckClusterVeto(cl, &fVetoIndex);
    if(fVeto_dE > 0)
    {
      if(CheckVetoBaF2TimeIsPrompt(fVeto->GetTime(fVetoIndex)))
      {
	fIsVCharged[i] = kTRUE;
	fVeto_Time     = fVeto->GetTime(fVetoIndex) * FlightTimeCorrection(clindex);
	fEnergy_BaF2   = fBaF2->GetEnergy(clindex);
      }
    }

    // check Veto-dE versus BaF2-E Banana-cuts -> dEvE@TAPS
    if(fIsVCharged[i] == kTRUE)
    {
      fDeltaE[i]        = fVeto_dE;             // dE of Veto              - for Histogram TA2DataManager::LoadVariable
      fEcharged[i]      = fEnergy_BaF2;         // BaF2_E of Index-Crystal - for Histogram TA2DataManager::LoadVariable
      fEchargedcl[i]    = fIsClusterEnergy[i];  // BaF2_E of Cluster       - for Histogram TA2DataManager::LoadVariable
      fVetoPromptTime[i]= fVeto_Time;           // veto Prompt-Time        - for Histogram TA2DataManager::LoadVariable
      fVetoBaf2Time[i]  = fIsClusterTime[i] - fVeto_Time; // Save Veto-BaF2 Time Diff  - for Histogram TA2DataManager::LoadVariable

       if(fTAPS_dEvE_Active == kTRUE && fTAPS_dEvE_Proton==kTRUE && fTAPS_dEvE_ProtonCut->IsInside(fEchargedcl[i],fVeto_dE))
         fdEvE_IsProton[i]   = kTRUE;
       if(fTAPS_dEvE_Active == kTRUE && fTAPS_dEvE_ChPion==kTRUE && fTAPS_dEvE_ChPionCut->IsInside(fEchargedcl[i],fVeto_dE))
         fdEvE_IsChPion[i]   = kTRUE;
       if(fTAPS_dEvE_Active == kTRUE && fTAPS_dEvE_Electron==kTRUE && fTAPS_dEvE_ElectronCut->IsInside(fEchargedcl[i],fVeto_dE))
         fdEvE_IsElectron[i] = kTRUE;

       if(fdEvE_IsProton[i] == kTRUE )
         fTOFproton[i]=fIsClusterTime[i]+fTaggerTime; // TOF if Charged for Histogram
    }

    fTOF[i] = fIsClusterTime[i] + fTaggerTime;         // TOF if Charged or not - for Histogram TA2DataManager::LoadVariable

    // check TimeOfFlight
    if(fTAPS_TOF_Nucleon == kTRUE)
    {
     fTOF_IsNucleon[i] = fTAPS_TOF_NucleonCut->IsInside(fIsClusterEnergy[i] ,fIsClusterTime[i] + fTaggerTime);
     if(fdEvE_IsProton[i] == kTRUE && fdEvE_IsChPion[i] == kFALSE)
       fTOFproton[i] = fIsClusterTime[i] + fTaggerTime;
    }

    // determine particle ID based on gathered information
    fPDG_ID[i]           = CheckParticleID(i);
    fDetectedParticle[i] = fPDG_ID[i];
    // check HitMatrix if charged ... if you see a hole in the spectrum ,then a veto is miss-wired
    if(fIsVCharged[i] == kTRUE)
    {
      TVector3 direction   = particles[i].GetP4().Vect().Unit();
      direction           *= fTapsTargetDistance  / direction.Z();
      fHitMatrixX[i]       = direction.X();
      fHitMatrixY[i]       = direction.Y();
    }

    // Save information of this particle
    fParticleID->SetP4( fP4+i,fPDG_ID[i],fIsClusterEnergy[i],cl->GetMeanPosition() );
    fP4tot  += fP4[i];
    fMinv[i] = fP4[i].M();
    SetParticleInfo(i);  // Set TA2Particle
  }
}

//-----------------------------------------------------------------------------

Int_t TA2Taps::CheckParticleID(UInt_t i)
{
  // 1. Veto:  no veto -> photon
  // 2. dEvE:  test (Banana)TCuts
  // 3. TOF:   test (Banana)TCuts

  if(fIsVCharged[i]== kFALSE)//is neutral hit
  {
   if(fTOF_IsNucleon[i] == kTRUE && fTAPS_TOF_onlyPhoton == kFALSE)
     return kNeutron;
   else
    return kGamma;
  }

  else //is charged hit
  {
    if(fTAPS_dEvE_Active == kTRUE) // dEvE Analysis is active
    {
      if(fTAPS_TOF_Nucleon == kFALSE) //  TOF analysis not ative
      {
        if(fdEvE_IsProton[i]== kTRUE)   return kProton;
        if(fdEvE_IsChPion[i]== kTRUE)   return kPiPlus;
        if(fdEvE_IsElectron[i]== kTRUE) return kElectron;
        else return kRootino;
      }
      if(fTAPS_TOF_Nucleon == kTRUE) //  TOF analysis is ative
      {
        if(fdEvE_IsProton[i]== kTRUE  && fTOF_IsNucleon[i] == kTRUE)  return kProton;
        if(fdEvE_IsProton[i]== kTRUE  && fTOF_IsNucleon[i] == kFALSE) return kPiPlus;  // decide yourself if you want to use this line!
        if(fdEvE_IsChPion[i]== kTRUE  && fTOF_IsNucleon[i] == kFALSE) return kPiPlus;
        if(fdEvE_IsElectron[i]==kTRUE && fTOF_IsNucleon[i] == kFALSE) return kElectron;
        else return kRootino;
      }
    }
    else return kRootino; // if dEvE analysis is NOT active (changed from AUTOMATIC proton assumption C.C)
  }

  return kRootino;
}

//--------------------------------------------------------------------------------

Double_t TA2Taps::CheckClusterVeto(HitCluster_t* inCluster, UInt_t* outVetoIndex) const
{
    // Return the maximum energy deposited in one of the veto detectors of the
    // TAPS cluster 'inCluster'.
    // Returns 0 if no cluster veto fired.
    //
    // If 'outVetoIndex' is provided write the index of the corresponding veto
    // element to that variable (-1 if no cluster veto fired).
    
    Int_t center        = inCluster->GetIndex();
    UInt_t nNeighbours  = inCluster->GetNNeighbour();
    UInt_t* neighbours  = inCluster->GetNeighbour();
        
    Double_t maxVetoEnergy = 0;
    if (outVetoIndex) *outVetoIndex = -1;

    // loop over all veto hits
    for (UInt_t i = 0; i < fVeto_NHits; i++)
    {
        // check central element
        if ((UInt_t)fVeto_Hits[i] == GetVetoInFrontOfElement(center))
        {
	    if (fVeto->GetEnergy(fVeto_Hits[i]) > maxVetoEnergy) 
            {
	        maxVetoEnergy = fVeto->GetEnergy(fVeto_Hits[i])
                                * TMath::Abs(TMath::Cos(inCluster->GetTheta() * TMath::DegToRad()));
                if (outVetoIndex) *outVetoIndex = (UInt_t)fVeto_Hits[i];
            }
        }

        // loop over all neighbour elements
        for (UInt_t j = 0; j < nNeighbours; j++)
        {
            if ((UInt_t)fVeto_Hits[i] == GetVetoInFrontOfElement((Int_t)neighbours[j]))
            {
	        if (fVeto->GetEnergy(fVeto_Hits[i]) > maxVetoEnergy) 
                {
		    maxVetoEnergy = fVeto->GetEnergy(fVeto_Hits[i])
                                    * TMath::Abs(TMath::Cos(inCluster->GetTheta() * TMath::DegToRad()));
                    if (outVetoIndex) *outVetoIndex = (UInt_t)fVeto_Hits[i];
                }
            }
        }
    }

    return maxVetoEnergy;
}

//--------------------------------------------------------------------------------

void TA2Taps::CalibrateBaF2Energy(TLorentzVector *photonCB)
{
  if(fCalibBaF2Step==0)
  {
    calibTAPS_m1g_AllCh  = new TH2F("Calib_TAPS_1g_IM_AllCh", "Calib_TAPS_1g_IM_AllCh", 2000, 0, 1000, 385, 0, 385);
    calibTAPS_m1g_Single = new TH1F("Calib_TAPS_1g_IMS", "Calib_TAPS_1g_IMS", 700, 0, 700);
  }

  Int_t* VetoHits= fVeto->GetHits();
  HitCluster_t** clBaF2; clBaF2 = fBaF2->GetCluster(); // get cluster structs
  HitCluster_t* cl;                                    // cluster struct
  UInt_t* id_clBaF2; id_clBaF2 = fBaF2->GetClustHit();
  Int_t fBaF2_Ncluster = fBaF2->GetNCluster();
  UInt_t clBaF2_Nhits;                                 // crystal-hits inside a cluster
  UInt_t* clBaF2_elements;
  Bool_t IsVCharged[fBaF2_Ncluster];
  for(Int_t i=0; i<fBaF2_Ncluster; i++)
    IsVCharged[i]=kFALSE;
  UInt_t CountUnchargedClusters = 0;
  UInt_t UnchargedClusters[fBaF2_Ncluster];

  //first: find neutral hits !!!  require: veto must NOT have fired in front of any cluster-crystal!
  //loop over all BaF2-clusters
  if(fVeto->GetNhits() < (UInt_t)fBaF2_Ncluster)
  {
    for (Int_t i=0; i< fBaF2_Ncluster; i++)
    {
      IsVCharged[i] = kFALSE;
      cl = clBaF2[id_clBaF2[i]];        // i-th cluster
      clBaF2_Nhits = cl->GetNhits();    // # hits of i-th cluster
      clBaF2_elements= cl->GetHits();   // elements (nr. of each crystal) of i-th cluster

      // check if there is a Veto-Hit infront of at least one cluster-crystal
      for(UInt_t n = 0; n< clBaF2_Nhits; n++)
      {
        for(UInt_t j = 0; j< fVeto->GetNhits(); j++)
        {
          if(clBaF2_elements[n]==(UInt_t)VetoHits[j])
            IsVCharged[i] = kTRUE;
        }
      }
      if(IsVCharged[i] == kFALSE)
      {
        UnchargedClusters[CountUnchargedClusters] = i;
        CountUnchargedClusters++;
      }
    }
  }

  //BaF2 - Energy-Calib (pi0-method)
  if(CountUnchargedClusters == 1)
  {
    HitCluster_t* cl0 = clBaF2[id_clBaF2[UnchargedClusters[0]]];
    UInt_t index = (UInt_t)cl0->GetIndex();
    TLorentzVector photonTAPS, pion;
    TVector3  photonTAPS_TV3;
    photonTAPS_TV3 = (cl0->GetMeanPosition())->Unit() * (cl0->GetEnergy()) ;
    photonTAPS.SetE( cl0->GetEnergy() );
    photonTAPS.SetVect( photonTAPS_TV3 );
    pion = *photonCB + photonTAPS;
    calibTAPS_m1g_Single->Fill(pion.M());
    calibTAPS_m1g_AllCh->Fill(pion.M(), index);
  }

  fCalibBaF2Step  = 1;
}

//--------------------------------------------------------------------------------

void TA2Taps::CalibrateBaF2Time()
{
  //Time align /calibrate all TAPS-BaF2Channels. Therefore you nee a macro from D. Werthmueller called
  // "TAPSTimeCalib.C"       ... and this Macro need a histogramm as input ... "Calib_TAPS_2g_Time_AllCh"
  //This function will fill this histogram ...... to use this function write in TA2PhotoPhysics:
  //fTaps2009GT->CalibrateBaF2Time()   (in the Reconstruction Function of TA2PhotoPhysics)

  if(fCalibTimeBaF2Step  == 0)
  {
    calibTAPS_t2g_AllCh  = new TH2F("Calib_TAPS_2g_Time_AllCh", "Calib_TAPS_2g_Time_AllCh", 2000, -50, 50, 385, 0, 385);
    calibTAPS_time2g     = new TH1F("Calib_TAPS_2g_TimeS", "Calib_TAPS_2g_TimeS", 1000, -100, 100);
  }

  UInt_t CountUnchargedClusters = 0;
  UInt_t* id_clBaF2; id_clBaF2 = fBaF2->GetClustHit();
  clBaF2 = fBaF2->GetCluster();
  HitCluster_t* cl[2];

  //find 2 Uncharged Clusters in TAPS
  for(UInt_t i = 0 ; i < fBaF2->GetNCluster(); i++)
  {
    if(fIsVCharged[i]==kTRUE && CountUnchargedClusters<2)
    {
      cl[CountUnchargedClusters] = clBaF2[id_clBaF2[i]];
      CountUnchargedClusters++;
    }
  }

  if(CountUnchargedClusters==2)
  {
    UInt_t index0 = (UInt_t)cl[0]->GetIndex();
    UInt_t index1 = (UInt_t)cl[1]->GetIndex();
    Float_t time0 = (Float_t)cl[0]->GetTime();
    Float_t time1 = (Float_t)cl[1]->GetTime();

    calibTAPS_time2g->Fill(time0);
    calibTAPS_time2g->Fill(time1);
    calibTAPS_t2g_AllCh->Fill(time0-time1 ,index0);
    calibTAPS_t2g_AllCh->Fill(time1-time0 ,index1);
  }

  fCalibTimeBaF2Step = 1;
}

// -------------------------------------------------------------------------------

void TA2Taps::CalibrateVetoEnergy()
{
  if(fCalibVetoStep  == 0)
  {
    VetoData = new TNtuple("TapsData",
                           "Ntuple containing correlated BaF2 and Veto Hits",
                           "module:vetoRawEnergy:baf2Energy:baf2Time:vetoEnergy:vetoTime:baf2ClEnergy:baf2ClTime");
    calibTAPS_VetoCorr = new TH2F("Calib_TAPS_VetoCorrel",  "Calib_TAPS_VetoCorrel" , 384, 0, 384, 384, 0, 384);
    calib_TAPSdEvEcl = new TH2F("Calib_TAPS_dEvEcl","Calib_TAPS_dEvEcl", 1000, 0, 1000, 100, 0, 10);
    calib_TAPSdEvE   = new TH2F("Calib_TAPS_dEvE","Calib_TAPS_dEvE", 500, 0, 500, 100, 0, 10);
  }

  // ------  TAPS BaF2 Veto correlation --- Start
  Int_t* VetoHits= fVeto->GetHits();
  Int_t* BaF2Hits = fBaF2->GetHits();

  for (UInt_t i = 0; i < fBaF2->GetNhits(); i++)
  {
      // loop over veto hits
      for (UInt_t j = 0; j < fVeto->GetNhits(); j++)
      {
       calibTAPS_VetoCorr->Fill(BaF2Hits[i], VetoHits[j]);
      }

  }
  // ------  TAPS BaF2 Veto correlation --- End

  Bool_t baf2Hit[384] = {0};
  Bool_t vetoHit[384] = {0};

  for (UInt_t i = 0; i < fBaF2->GetNhits(); ++i)
    baf2Hit[fBaF2->GetHits(i)] = 1;

  for (UInt_t i = 0; i < fVeto->GetNhits(); ++i)
    vetoHit[fVeto->GetHits(i)] = 1;

  for (Int_t module = 0; module < 384; ++module)
  {
    if (baf2Hit[module] && vetoHit[module])
    {
      //baf2RawEnergy = fBaF2->GetElement(module)->GetRawADCValue();
      //baf2RawTime   = fBaF2->GetElement(module)->GetRawTDCValue();
      Float_t vetoRawEnergy = (Float_t) fVeto->GetElement(module)->GetRawADCValue();
      //vetoRawTime   = fVeto->GetElement(module)->GetRawTDCValue();

      Float_t baf2Energy    = (Float_t) fBaF2->GetEnergy(module);
      Float_t baf2ClEnergy  = TAPS_GetCLInfo(module, 0);
      Float_t baf2ClTime    = TAPS_GetCLInfo(module, 1);
      Float_t baf2Time      = (Float_t) fBaF2->GetTime  (module);
      Float_t vetoEnergy    = (Float_t) fVeto->GetEnergy(module);
      Float_t vetoTime      = (Float_t) fVeto->GetTime  (module);

      calib_TAPSdEvEcl->Fill(baf2ClEnergy,vetoEnergy);
      calib_TAPSdEvE->Fill(baf2Energy,vetoEnergy);

      VetoData->Fill((Float_t)module, vetoRawEnergy, baf2Energy, baf2Time, vetoEnergy, vetoTime, baf2ClEnergy,baf2ClTime);
    }
  }
  fCalibVetoStep = 1;
}

// -------------------------------------------------------------------------------

Float_t TA2Taps::TAPS_GetCLInfo(UInt_t module, UInt_t TimeOrEnergy)
{
  HitCluster_t** clBaF2; clBaF2 = fBaF2->GetCluster(); // -> cluster structs
  HitCluster_t* cl;                                    // cluster struct
  UInt_t* id_clBaF2; id_clBaF2 = fBaF2->GetClustHit();
  Int_t fBaF2_Ncluster = fBaF2->GetNCluster();

  for (Int_t i=0; i<fBaF2_Ncluster; i++)
  {
    cl = clBaF2[id_clBaF2[i]];        // i-th cluster
    if(cl->GetIndex()==module && TimeOrEnergy==0) return (Float_t )cl->GetEnergy();
    if(cl->GetIndex()==module && TimeOrEnergy==1) return (Float_t )cl->GetTime();
  }

  return (Float_t )0.0;
}

//--------------------------------------------------------------------------------

void TA2Taps::SetConfig(char* line, int key)
{
  fTapsRunStep = 1;
  UInt_t check = 0;
  TCutG* CutPtr;

  switch( key )
  {
   case ETAPSFactor:
    {
     if( sscanf( line, "%lf", &fTapsFudge ) < 1 ){ goto error;}
    }
    break;

   case ETAPS_SimpleReconstruct:
    {
     if( sscanf( line, "%d", &check ) < 1 ){ printf("\nERROR\n");}
     if(check == 0) fSimpleReconstruct=kFALSE; else fSimpleReconstruct=kTRUE;
    }
    break;

   case ETAPS_dEvE_Active:
    {
     if( sscanf( line, "%d", &check ) < 1 ){ printf("\nERROR\n");}
     if(check == 0) fTAPS_dEvE_Active=kFALSE; else fTAPS_dEvE_Active=kTRUE;
    }
    break;

   case ETAPS_dEvE_Cuts:
    {
    sscanf(line, "%s", fTAPS_dEvE_Cuts);
    TFile *f = new TFile(fTAPS_dEvE_Cuts, "READ");
    if(f)
    {
      f->Close();
      delete f;
    }
    else
    {
      printf("\n!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf("!       File containing dEvE-CUTs is missing.       !\n");
      printf("!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
      delete f;
    }
    }
    break;

   case ETAPS_dEvE_Proton:
    {
      sscanf(line, "%d", &check);
      if(check == 0) fTAPS_dEvE_Proton = kFALSE; else fTAPS_dEvE_Proton = kTRUE;
    }
    break;

   case ETAPS_dEvE_Proton_CutName:
    {
      if(fTAPS_dEvE_Proton == kTRUE )
      {
        sscanf(line, "%s", fTAPS_dEvE_Proton_CutName);
        fTAPS_dEvE_CutFile = new TFile(fTAPS_dEvE_Cuts, "READ");
        CutPtr = (TCutG*)fTAPS_dEvE_CutFile->Get(fTAPS_dEvE_Proton_CutName);
        // check if the cut was found, otherwise exit to prevent later segfaults
        if(!CutPtr){
          fTAPS_dEvE_CutFile->Close();
          printf("ERROR: TAPS dEvE cut '%s' not found!\n", fTAPS_dEvE_Proton_CutName);
          PrintError(line, "<TAPS dEvE Proton cut: no cut with specified cutname found>", EErrFatal);
        }
        fTAPS_dEvE_ProtonCut = CutPtr;
        fTAPS_dEvE_CutFile->Close();
      }
    }
    break;

   case ETAPS_dEvE_ChPion:
    {
      sscanf( line, "%d", &check );
      if(check == 0){fTAPS_dEvE_ChPion = kFALSE;}    // dEvE-pi+ cut inactive!
      else{fTAPS_dEvE_ChPion = kTRUE;}               // dEvE-pi+ cut active!
    }
    break;

   case ETAPS_dEvE_ChPion_CutName:
    {
      if(fTAPS_dEvE_ChPion == kTRUE)
      {
        sscanf(line, "%s", fTAPS_dEvE_ChPion_CutName);
        fTAPS_dEvE_CutFile = new TFile(fTAPS_dEvE_Cuts, "READ");
        CutPtr = (TCutG*)fTAPS_dEvE_CutFile->Get(fTAPS_dEvE_ChPion_CutName);
        // check if the cut was found, otherwise exit to prevent later segfaults
        if(!CutPtr){
          fTAPS_dEvE_CutFile->Close();
          printf("ERROR: TAPS dEvE cut '%s' not found!\n", fTAPS_dEvE_ChPion_CutName);
          PrintError(line, "<TAPS dEvE Pion cut: no cut with specified cutname found>", EErrFatal);
        }
        fTAPS_dEvE_ChPionCut = CutPtr;
        fTAPS_dEvE_CutFile->Close();
      }
    }
    break;


   case ETAPS_dEvE_Electron:
    {
      sscanf( line, "%d", &check );
      if(check == 0){fTAPS_dEvE_Electron = kFALSE;}  // dEvE-electron  cut inactive!
      else{fTAPS_dEvE_Electron = kTRUE;}             // dEvE-electron  cut  active!
    }
    break;

   case ETAPS_dEvE_Electron_CutName:
    {
     if(fTAPS_dEvE_Electron == kTRUE)
      {
        sscanf(line, "%s", fTAPS_dEvE_Electron_CutName);
        fTAPS_dEvE_CutFile = new TFile(fTAPS_dEvE_Cuts, "READ");
        CutPtr = (TCutG*)fTAPS_dEvE_CutFile->Get(fTAPS_dEvE_Electron_CutName);
        // check if the cut was found, otherwise exit to prevent later segfaults
        if(!CutPtr){
          fTAPS_dEvE_CutFile->Close();
          printf("ERROR: TAPS dEvE cut '%s' not found!\n", fTAPS_dEvE_Electron_CutName);
          PrintError(line, "<TAPS dEvE Electron cut: no cut with specified cutname found>", EErrFatal);
        }
        fTAPS_dEvE_ElectronCut = CutPtr;
        fTAPS_dEvE_CutFile->Close();
      }
    }
    break;

   case ETAPS_TOF_Nucleon:
    {
      sscanf(line, "%d", &check);
      if(check == 0) fTAPS_TOF_Nucleon = kFALSE; else fTAPS_TOF_Nucleon = kTRUE;
    }
    break;

   case ETAPS_TOF_Cuts:
    {
      sscanf(line, "%s", fTAPS_TOF_Cuts);
      TFile *f = new TFile(fTAPS_TOF_Cuts,"READ");
      if(f)
      {
        f->Close();
        delete f;
      }
      else
      {
        printf("\n!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!       File containing TOF-CUTs is missing.        !\n");
        printf("!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        delete f;
      }
    }
    break;

   case ETAPS_TOF_Nucleon_CutName:
    {
      if(fTAPS_TOF_Nucleon == kTRUE)
      {
        sscanf(line, "%s", fTAPS_TOF_Nucleon_CutName);
        fTAPS_TOF_CutFile = new TFile(fTAPS_TOF_Cuts, "READ");
        CutPtr = (TCutG*)fTAPS_TOF_CutFile->Get(fTAPS_TOF_Nucleon_CutName);
        // check if the cut was found, otherwise exit to prevent later segfaults
        if(!CutPtr){
          fTAPS_TOF_CutFile->Close();
          printf("ERROR: TAPS TOF cut '%s' not found!\n", fTAPS_TOF_Nucleon_CutName);
          PrintError(line, "<TAPS TOF Nucleon cut: no cut with specified cutname found>", EErrFatal);
        }
        fTAPS_TOF_NucleonCut = CutPtr;
        fTAPS_TOF_CutFile->Close();
      }
    }
    break;

   case ETAPS_TOF_TaggerTime:
    {
      sscanf( line, "%lf", &fTaggerTime );
    }
    break;

   case ETAPS_TOF_onlyPhoton:
    {
      sscanf(line, "%d", &check);
      if(check == 0) fTAPS_TOF_onlyPhoton = kFALSE; else fTAPS_TOF_onlyPhoton = kTRUE;
    }
    break;

   case ETapsTargetDistance:
    {
      sscanf(line, "%lf", &fTapsTargetDistance);
    }
    break;

   case ETAPSVetoBaF2TimeCut:
    {
      sscanf( line, "%d%lf%lf", &check, &fVetoTimeWindoMin, &fVetoTimeWindoMax );
      if(check == 1) fCutOnVetoBaF2Time = kTRUE; else fCutOnVetoBaF2Time = kFALSE;
    }
    break;

   case ETAPSDoFlightTimeCorrection:
    {
      sscanf( line, "%d", &check );
      if(check == 1) fDoFlightTimeCorrection = kTRUE; else fDoFlightTimeCorrection = kFALSE;
    }
    break;

   case ETAPSRingEnergycorr1:
    {
      //Read ringwise correction factors: E' = c1*E + c2*E*E. Here: c1
      if( sscanf( line, "%f%f%f%f%f%f%f%f%f%f%f", &fTapsRingEnergycorr1[0], &fTapsRingEnergycorr1[1], &fTapsRingEnergycorr1[2], &fTapsRingEnergycorr1[3], &fTapsRingEnergycorr1[4], &fTapsRingEnergycorr1[5], &fTapsRingEnergycorr1[6], &fTapsRingEnergycorr1[7], &fTapsRingEnergycorr1[8], &fTapsRingEnergycorr1[9], &fTapsRingEnergycorr1[10]) < 11 ) goto error;
    }
    break;

    case ETAPSRingEnergycorr2:
     {
      //Read ringwise correction factors: E' = c1*E + c2*E*E. Here: c2
      if( sscanf( line, "%f%f%f%f%f%f%f%f%f%f%f", &fTapsRingEnergycorr2[0], &fTapsRingEnergycorr2[1], &fTapsRingEnergycorr2[2], &fTapsRingEnergycorr2[3], &fTapsRingEnergycorr2[4], &fTapsRingEnergycorr2[5], &fTapsRingEnergycorr2[6], &fTapsRingEnergycorr2[7], &fTapsRingEnergycorr2[8], &fTapsRingEnergycorr2[9], &fTapsRingEnergycorr2[10]) < 11 ) goto error;
     }
     break;

    case ETAPSTimeshift:
     {
      sscanf(line, "%lf", &fTapsTimeshift);
     }
    break;

    default:
     {
       //Command not found...possible pass to next config
       TA2Apparatus::SetConfig( line, key );
     }
     break;
  }
  return;

error:
  PrintError(line);
  return;
}

//-----------------------Thank you for reading ---------------------------
ClassImp(TA2Taps)
