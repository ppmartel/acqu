//--Author	S Schumann   29th Oct 2008
//--Rev         E J Downie   16th mar 2009 combine SS & JRMA updates
//--Rev         JRM Annand   23rd Apr 2009 call TObj constructor
//--Rev         S Schumann   19th Jun 2009 add Reset() function
//--Update      JRM Annand   23rd Apr 2013 MWPC stuff when? not by JRMA
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2Particle
//
// Interface class for particle information transfer between <TA2Apparatus>
// and <TA2Physics>

#include "TA2Particle.h"

R__EXTERN TA2Analysis*  gAN;


//-----------------------------------------------------------------------------

TA2Particle::TA2Particle(TDatabasePDG* pPDG) : TObject()
{
  fPDG = pPDG;
  Reset();
}

//-----------------------------------------------------------------------------

TA2Particle::TA2Particle() : TObject()
{
  fPDG = gAN->GetParticleID()->GetPDG();
  Reset();
}

//-----------------------------------------------------------------------------

TA2Particle::~TA2Particle()
{

}

//-----------------------------------------------------------------------------

void TA2Particle::Reset()
{
  //Clear all entries either at construction time or
  //when TA2Particle instance is re-used in new event
  fP4.SetXYZT(0.0, 0.0, 0.0, 0.0);
  fTime = (Double_t)EBufferEnd;
  fVetoEnergy = (Double_t)EBufferEnd;
  fVetoTime = (Double_t)EBufferEnd;
  fPSAShort = (Double_t)EBufferEnd;
  fSigmaE = 0.0;
  fSigmaTheta = 0.0;
  fSigmaPhi = 0.0;
  fSigmaTime = 0.0;
  fCircDegree = 0.0;
  fLinDegree = 0.0;
  fClusterSize = 0;
  fVetoIndex = EBufferEnd;
  fCentralIndex = EBufferEnd;
  fParticleID = kRootino;
  fSecondID = kRootino;
  fCircState = ECircNone;
  fLinState = ELinNone;
  fWindow = EWindowNone;
  fApparati = EAppUndef;
  fDetectors = EDetUndef;
  fUnclear = false;
  fIintersMwpc[0] = 0;
  fIintersMwpc[1] = 0;
  fTrackEnergy = 0.0;
  fEnergyMwpc0 = 0.0;
  fEnergyMwpc1 = 0.0;
  fPsVertex.SetXYZ(0.0,0.0,0.0);
  fTrackTime = 0.0;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetLinState(Int_t pLinState)
{
 if((pLinState==ELinPara) || (pLinState==ELinPerp))
   fLinState = pLinState;
  else
   fLinState = ELinNone;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetCircState(Int_t pCircState)
{
 if((pCircState==ECircPos) || (pCircState==ECircNeg))
   fCircState = pCircState;
  else
   fCircState = ECircNone;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetWindow(Int_t pWindow)
{
 if((pWindow==EWindowPrompt) || (pWindow==EWindowRandom))
   fWindow = pWindow;
  else
   fWindow = EWindowNone;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetApparatus(Int_t pApparatus)
{
  switch(pApparatus)
  {
   case EAppCB:
   case EAppTAPS:
   case EAppTagger:
   case EAppTOF:
    fApparati+=pApparatus;
  }
}

//-----------------------------------------------------------------------------

void TA2Particle::SetDetector(Int_t pDetector)
{
  switch(pDetector)
  {
   case EDetNaI:
   case EDetPID:
   case EDetMWPC:
   case EDetBaF2:
   case EDetPbWO4:
   case EDetVeto:
   case EDetLadder:
   case EDetMicro:
   case EDetEndpoint:
   case EDetTOFBar:
   case EDetCerenkov:
    fDetectors+=pDetector;
  }
}

//-----------------------------------------------------------------------------

Bool_t TA2Particle::HasApparatus(Int_t pApparatus)
{
  switch(pApparatus)
  {
   case EAppCB:
   case EAppTAPS:
   case EAppTagger:
   case EAppTOF:
    return(fApparati & pApparatus);
    break;
   default:
    return false;
  }
}

//-----------------------------------------------------------------------------

Bool_t TA2Particle::HasDetector(Int_t pDetector)
{
  switch(pDetector)
  {
   case EDetNaI:
   case EDetPID:
   case EDetMWPC:
   case EDetBaF2:
   case EDetPbWO4:
   case EDetVeto:
   case EDetLadder:
   case EDetMicro:
   case EDetEndpoint:
   case EDetTOFBar:
   case EDetCerenkov:
    return (fDetectors & pDetector);
    break;
   default:
    return false;
  }
}

//-----------------------------------------------------------------------------

void TA2Particle::SetP4(Int_t pParticleID, Double_t pT, TVector3 pP_)
{
  Double_t M;
  Double_t P;
  Double_t E;

  M = fPDG->GetParticle(pParticleID)->Mass() * 1000.0;
  E = pT + M;
  P = TMath::Sqrt(E*E - M*M);

  fP4.SetE(E);
  fP4.SetVect(pP_.Unit() * P);

  fParticleID = pParticleID;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetMass(Double_t pM)
{
  Double_t P;
  Double_t E;
  Double_t T;

  T = fP4.E() - fP4.M();
  E = T + pM;
  P = TMath::Sqrt(E*E - pM*pM);

  fP4.SetE(E);
  fP4.SetVect(fP4.Vect().Unit() * P);

  fParticleID = kRootino;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetParticleID(Int_t pParticleID)
{
  Double_t M;
  Double_t P;
  Double_t E;

  M = fPDG->GetParticle(pParticleID)->Mass() * 1000.0;
  E = (fP4.E() - fP4.M()) + M;
  P = TMath::Sqrt(E*E - M*M);

  fP4.SetE(E);
  fP4.SetVect(fP4.Vect().Unit() * P);

  fParticleID = pParticleID;
}

//-----------------------------------------------------------------------------

void TA2Particle::SetKinetic(Double_t pT)
{
  Double_t M;
  Double_t P;
  Double_t E;

  M = fP4.M();
  E = M + pT;
  P = TMath::Sqrt(E*E - M*M);

  fP4.SetE(E);
  fP4.SetVect(fP4.Vect().Unit() * P);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetTotal(Double_t pE)
{
  Double_t M;
  Double_t P;

  M = fP4.M();
  P = TMath::Sqrt(pE*pE - M*M);

  fP4.SetE(pE);
  fP4.SetVect(fP4.Vect().Unit() * P);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetMomentum(Double_t pP)
{
  Double_t M;
  Double_t E;

  M = fP4.M();
  E = TMath::Sqrt(pP*pP + M*M);

  fP4.SetE(E);
  fP4.SetVect(fP4.Vect().Unit() * pP);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetP3(TVector3 pP3)
{
  Double_t M;
  Double_t E;

  M = fP4.M();
  E = TMath::Sqrt(pP3.Mag2() + M*M);

  fP4.SetE(E);
  fP4.SetVect(pP3);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetPx(Double_t pPx)
{
  Double_t Py;
  Double_t Pz;
  Double_t P2;
  Double_t M;
  Double_t E;

  Py = fP4.Py();
  Pz = fP4.Pz();
  P2 = pPx*pPx + Py*Py + Pz*Pz;
  M = fP4.M();
  E = TMath::Sqrt(M*M + P2);

  fP4.SetE(E);
  fP4.SetX(pPx);
  fP4.SetY(Py);
  fP4.SetZ(Pz);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetPy(Double_t pPy)
{
  Double_t Px;
  Double_t Pz;
  Double_t P2;
  Double_t M;
  Double_t E;

  Px = fP4.Px();
  Pz = fP4.Pz();
  P2 = Px*Px + pPy*pPy + Pz*Pz;
  M = fP4.M();
  E = TMath::Sqrt(M*M + P2);

  fP4.SetE(E);
  fP4.SetX(Px);
  fP4.SetY(pPy);
  fP4.SetZ(Pz);
}

//-----------------------------------------------------------------------------

void TA2Particle::SetPz(Double_t pPz)
{
  Double_t Px;
  Double_t Py;
  Double_t P2;
  Double_t M;
  Double_t E;

  Px = fP4.Px();
  Py = fP4.Py();
  P2 = Px*Px + Py*Py + pPz*pPz;
  M = fP4.M();
  E = TMath::Sqrt(M*M + P2);

  fP4.SetE(E);
  fP4.SetX(Px);
  fP4.SetY(Py);
  fP4.SetZ(pPz);
}

//-----------------------------------------------------------------------------

TA2Particle TA2Particle::Add(TA2Particle pParticle)
{
  TA2Particle Combined(fPDG);
  Double_t VetoEnergy = 0.0;
  Double_t VetoTime = 0.0;
  Double_t PSAShort = 0.0;
  Bool_t IsPresent[2];

  Combined.SetParticleIDA(kRootino);

  Combined.SetP4(pParticle.GetP4() + GetP4());
  Combined.SetTime((pParticle.GetTime() + GetTime())/2.0);

  Combined.SetClusterSize(pParticle.GetClusterSize() + GetClusterSize());

  Combined.SetApparatusA(pParticle.GetApparati() | GetApparati());
  Combined.SetDetectorA(pParticle.GetDetectors() | GetDetectors());

  IsPresent[0] = (GetVetoEnergy()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle.GetVetoEnergy()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) VetoEnergy+=GetVetoEnergy();
  if(IsPresent[1]) VetoEnergy+=pParticle.GetVetoEnergy();
  if(IsPresent[0] | IsPresent[1]) Combined.SetVetoEnergy(VetoEnergy); else Combined.SetVetoEnergy(EBufferEnd);


  IsPresent[0] = (GetVetoTime()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle.GetVetoTime()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) VetoTime+=GetVetoTime();
  if(IsPresent[1]) VetoTime+=pParticle.GetVetoTime();
  if(IsPresent[0] | IsPresent[1]) Combined.SetVetoTime(VetoTime); else Combined.SetVetoTime(EBufferEnd);

  IsPresent[0] = (GetPSAShort()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle.GetPSAShort()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) PSAShort+=GetPSAShort();
  if(IsPresent[1]) PSAShort+=pParticle.GetPSAShort();
  if(IsPresent[0] | IsPresent[1]) Combined.SetPSAShort(PSAShort); else Combined.SetPSAShort(EBufferEnd);

  Combined.SetCircDegree(0.0);
  Combined.SetCircState(ECircNone);

  Combined.SetLinDegree(0.0);
  Combined.SetLinState(ELinNone);

  Combined.SetWindow(EWindowNone);
  Combined.SetVetoIndex(EBufferEnd);

  return Combined;
}

//-----------------------------------------------------------------------------

TA2Particle TA2Particle::Add(TA2Particle pParticle1, TA2Particle pParticle2)
{
  TA2Particle Combined(fPDG);
  Double_t VetoEnergy = 0.0;
  Double_t VetoTime = 0.0;
  Double_t PSAShort = 0.0;
  Bool_t IsPresent[3];

  Combined.SetParticleIDA(kRootino);

  Combined.SetP4(pParticle1.GetP4() + pParticle2.GetP4() + GetP4());
  Combined.SetTime((pParticle1.GetTime() + pParticle2.GetTime() + GetTime())/3.0);

  Combined.SetClusterSize(pParticle1.GetClusterSize() + pParticle2.GetClusterSize() + GetClusterSize());

  Combined.SetApparatusA(pParticle1.GetApparati() | pParticle2.GetApparati() | GetApparati());
  Combined.SetDetectorA(pParticle1.GetDetectors() | pParticle2.GetDetectors() | GetDetectors());

  IsPresent[0] = (GetVetoEnergy()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle1.GetVetoEnergy()!=(Double_t)EBufferEnd);
  IsPresent[2] = (pParticle2.GetVetoEnergy()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) VetoEnergy+=GetVetoEnergy();
  if(IsPresent[1]) VetoEnergy+=pParticle1.GetVetoEnergy();
  if(IsPresent[2]) VetoEnergy+=pParticle2.GetVetoEnergy();
  if(IsPresent[0] | IsPresent[1] | IsPresent[2]) Combined.SetVetoEnergy(VetoEnergy); else Combined.SetVetoEnergy(EBufferEnd);

  IsPresent[0] = (GetVetoTime()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle1.GetVetoTime()!=(Double_t)EBufferEnd);
  IsPresent[2] = (pParticle2.GetVetoTime()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) VetoTime+=GetVetoTime();
  if(IsPresent[1]) VetoTime+=pParticle1.GetVetoTime();
  if(IsPresent[2]) VetoTime+=pParticle2.GetVetoTime();
  if(IsPresent[0] | IsPresent[1] | IsPresent[2]) Combined.SetVetoTime(VetoTime); else Combined.SetVetoTime(EBufferEnd);

  IsPresent[0] = (GetPSAShort()!=(Double_t)EBufferEnd);
  IsPresent[1] = (pParticle1.GetPSAShort()!=(Double_t)EBufferEnd);
  IsPresent[2] = (pParticle2.GetPSAShort()!=(Double_t)EBufferEnd);
  if(IsPresent[0]) PSAShort+=GetPSAShort();
  if(IsPresent[1]) PSAShort+=pParticle1.GetPSAShort();
  if(IsPresent[2]) PSAShort+=pParticle2.GetPSAShort();
  if(IsPresent[0] | IsPresent[1] | IsPresent[2]) Combined.SetPSAShort(PSAShort); else Combined.SetPSAShort(EBufferEnd);

  Combined.SetCircDegree(0.0);
  Combined.SetCircState(ECircNone);

  Combined.SetLinDegree(0.0);
  Combined.SetLinState(ELinNone);

  Combined.SetWindow(EWindowNone);
  Combined.SetVetoIndex(EBufferEnd);

  return Combined;
}

//-----------------------------------------------------------------------------

TA2Particle TA2Particle::Add(TA2Particle* pParticles, Int_t pNumber)
{
  if(!pParticles) return *this;

  TA2Particle Combined(fPDG);
  TLorentzVector P4;
  Double_t Time;
  Double_t VetoEnergy = 0.0;
  Double_t VetoTime = 0.0;
  Double_t PSAShort = 0.0;
  Int_t Apparatus;
  Int_t Detector;
  Int_t ClusterSize;
  Bool_t IsPresent[pNumber];
  Bool_t IsPresentBase;

  Combined.SetParticleIDA(kRootino);

  P4 = GetP4();
  Time = GetTime();
  ClusterSize = GetClusterSize();
  Apparatus = GetApparati();
  Detector = GetDetectors();
  for(Int_t t=0; t<pNumber; t++)
  {
    P4+=pParticles[t].GetP4();
    Time+=pParticles[t].GetTime();
    ClusterSize+=pParticles[t].GetClusterSize();
    Apparatus = (Apparatus | pParticles[t].GetApparati());
    Detector = (Detector | pParticles[t].GetDetectors());
  }
  Combined.SetP4(P4);
  Combined.SetTime(Time/(Double_t)pNumber);
  Combined.SetClusterSize(ClusterSize);
  Combined.SetApparatusA(Apparatus);
  Combined.SetDetectorA(Detector);

  IsPresentBase = (GetVetoEnergy()!=(Double_t)EBufferEnd);
  if(IsPresentBase) VetoEnergy+=GetVetoEnergy();
  for(Int_t t=0; t<pNumber; t++)
  {
    IsPresent[t] = (pParticles[t].GetVetoEnergy()!=(Double_t)EBufferEnd);
    if(IsPresent[t]) VetoEnergy+=pParticles[t].GetVetoEnergy();
    IsPresentBase = (IsPresentBase | IsPresent[t]);
  }
  if(IsPresentBase) Combined.SetVetoEnergy(VetoEnergy); else Combined.SetVetoEnergy(EBufferEnd);

  IsPresentBase = (GetVetoTime()!=(Double_t)EBufferEnd);
  if(IsPresentBase) VetoTime+=GetVetoTime();
  for(Int_t t=0; t<pNumber; t++)
  {
    IsPresent[t] = (pParticles[t].GetVetoTime()!=(Double_t)EBufferEnd);
    if(IsPresent[t]) VetoTime+=pParticles[t].GetVetoTime();
    IsPresentBase = (IsPresentBase | IsPresent[t]);
  }
  if(IsPresentBase) Combined.SetVetoTime(VetoTime); else Combined.SetVetoTime(EBufferEnd);

  IsPresentBase = (GetPSAShort()!=(Double_t)EBufferEnd);
  if(IsPresentBase) PSAShort+=GetPSAShort();
  for(Int_t t=0; t<pNumber; t++)
  {
    IsPresent[t] = (pParticles[t].GetPSAShort()!=(Double_t)EBufferEnd);
    if(IsPresent[t]) PSAShort+=pParticles[t].GetPSAShort();
    IsPresentBase = (IsPresentBase | IsPresent[t]);
  }
  if(IsPresentBase) Combined.SetPSAShort(PSAShort); else Combined.SetPSAShort(EBufferEnd);

  Combined.SetCircDegree(0.0);
  Combined.SetCircState(ECircNone);

  Combined.SetLinDegree(0.0);
  Combined.SetLinState(ELinNone);

  Combined.SetWindow(EWindowNone);
  Combined.SetVetoIndex(EBufferEnd);

  return Combined;
}

//-----------------------------------------------------------------------------

HitCluster_t* TA2Particle::FindCluster(const Char_t* pAppName, const Char_t* pDetName)
{
  HitCluster_t* Cluster = NULL;
  TA2Apparatus* Apparatus = NULL;
  TA2ClusterDetector* Detector = NULL;

  Apparatus = (TA2Apparatus*)(gAN)->GetChild(pAppName);
  if(Apparatus) Detector = (TA2ClusterDetector*)Apparatus->GetChild(pDetName, "TA2ClusterDetector");

  if(Detector)
    if((Detector->GetClustHit()) && (Detector->GetCluster()))
    {
      for(UInt_t t=0; t<Detector->GetNCluster(); t++)
        if(fCentralIndex==(Int_t)(Detector->GetClustHit()[t])) Cluster = Detector->GetCluster(t);
    }

  return Cluster;
}

//-----------------------------------------------------------------------------
ClassImp(TA2Particle)
