#include <iostream>
using namespace std;

// AcquROOT includes
#include "TA2UserAnalysis.h"
#include "TA2CentralApparatus.h"
#include "TA2CalArray.h"
#include "TA2PlasticPID.h"
#include "TA2CylMwpc.h"
#include "TA2CentralTrack.h"

// My includes
#include "TA2GeomCalibPhysics.h"


//_____________________________________________________________________________________
TA2GeomCalibPhysics::TA2GeomCalibPhysics( const char* name, TA2Analysis* analysis ) : TA2Physics( name, analysis )
{
  // Initialise Physics variables here
  // Default null pointers, zeroed variables
  
  // Apparatuses & Detectrors
  fCB   = NULL;
  //
  fNaI  = NULL;
  //
  fMwpc = NULL;
  fNintersTrue = NULL;
  //
  fPid  = NULL;
}

//_____________________________________________________________________________________
TA2GeomCalibPhysics::~TA2GeomCalibPhysics()
{
// Free up allocated memory...after checking its allocated
// detector and cuts lists
  
  // E-WI
  if (fMwpc)
  {
    for (Int_t i=0; i<kNch; ++i)
    {
      delete [] fPhiInters[i];
      delete [] fZinters[i];
      delete [] fDphiEI[i];
      delete [] fCGclI[i];
      delete [] fCGclW[i];
      delete [] fCGclE[i];
    }
  }
  
  // Mwpc
  if (fH3Mwpc_dPhi_vs_Phi_vs_Z) delete fH3Mwpc_dPhi_vs_Phi_vs_Z;
  if (fH3Mwpc_dZ_vs_Phi_vs_Z) delete fH3Mwpc_dZ_vs_Phi_vs_Z;
  
  // Mwpc-NaI
  for (Int_t i=0; i<kNch; ++i)
  {
    if (fH3MwpcNaI_dPhi_vs_Phi_vs_Z[i]) delete fH3MwpcNaI_dPhi_vs_Phi_vs_Z[i];
    if (fH3MwpcNaI_dZ_vs_Phi_vs_Z[i]) delete fH3MwpcNaI_dZ_vs_Phi_vs_Z[i];
    if (fH3EI_dPhiEI_PhiEI_Zinters[i]) delete fH3EI_dPhiEI_PhiEI_Zinters[i];
  }
}

// //_____________________________________________________________________________________
// void TA2GeomCalibPhysics::SetConfig(Char_t* line, Int_t key)
// {
//   //  Reading the setup file
// }

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::PostInit()
{
  // Initialise arrays to contain 4 momenta and plotable scaler variables
  // Missing mass, missing energy, cm momentum, energies, angles
  // Initialisation will abort if CB or Tagger not initialised
  // TAPS is optional
  
  // CB
  fCB = (TA2CentralApparatus*)((TA2Analysis*)fParent)->GetChild("CB");
  if (!fCB) PrintError("","<No Central Apparatus class found in annalysis>",EErrFatal);

  // NaI
  fNaI = (TA2CalArray*)((TA2Analysis*)fParent)->GetGrandChild("NaI");
  if (!fNaI) PrintError("Warning!","<No NaI class found in annalysis>");
    
  // Mwpc
  fMwpc = (TA2CylMwpc*)((TA2Analysis*)fParent)->GetGrandChild("CylMWPC");
  if (!fMwpc)
  {
    PrintError("Warning!","<No Mwpc class found in annalysis>");
  }
  else
  {
    fNintersTrue = fMwpc->GetNintersTrue();
    Int_t nInters = fMwpc->GetMaxIntersect();
    Char_t buf[4];
    for (Int_t i=0; i<kNch; ++i)
    {
      fPhiInters[i] = new Double_t[nInters+1];
      fZinters[i]   = new Double_t[nInters+1];
      fDphiEI[i]    = new Double_t[nInters+1];
      fCGclI[i]     = new Double_t[nInters+1];
      fCGclW[i]     = new Double_t[nInters+1];
      fCGclE[i]     = new Double_t[nInters+1];
      fPhiEI[i]     = new Double_t[nInters+1];
      fDxInterMwpcNaI[i][2]   = EBufferEnd;
      fDyInterMwpcNaI[i][2]   = EBufferEnd;
      fDzInterMwpcNaI[i][2]   = EBufferEnd;
      fDphiInterMwpcNaI[i][2] = EBufferEnd;
      fZinterMwpcNaI[i][2]   = EBufferEnd;
      fPhiInterMwpcNaI[i][2] = EBufferEnd;
      sprintf(buf,"%d",i);
      fH3EI_dPhiEI_PhiEI_Zinters[i] = new TH3D("dPhiEI_PhiEI_Zinters_"+TString(buf),"dPhiEI(Phi,Z)",90,0.,360.,100,-300.,300.,50,-50.,50.);
      fH3MwpcNaI_dPhi_vs_Phi_vs_Z[i] = new TH3D("dPhi_vs_Phi_vs_Z_"+TString(buf),"dPhi(Phi,Z)",90,0.,360.,100,-300.,300.,50,-50.,50.);
      fH3MwpcNaI_dZ_vs_Phi_vs_Z[i]   = new TH3D("dZ_vs_Phi_vs_Z_"+TString(buf),"dZ(Phi,Z)",90,0.,360.,100,-300.,300.,100,-50.,50.);
    }
    fDxInterMwpc[2]   = EBufferEnd;
    fDyInterMwpc[2]   = EBufferEnd;
    fDzInterMwpc[2]   = EBufferEnd;
    fDphiInterMwpc[2] = EBufferEnd;
    fZinterMwpc[2]   = EBufferEnd;
    fPhiInterMwpc[2] = EBufferEnd;
    fDtetTracks2[2] = EBufferEnd;
    fThetaTracks2[2] = EBufferEnd;
    fH3Mwpc_dPhi_vs_Phi_vs_Z = new TH3D("dPhi_vs_Phi_vs_Z","dPhi(Phi,Z)",90,0.,360.,200,-400.,400.,40,-5.,5.);
    fH3Mwpc_dZ_vs_Phi_vs_Z   = new TH3D("dZ_vs_Phi_vs_Z","dZ(Phi,Z)",90,0.,360.,200,-400.,400.,100,-15.,15.);
  }
  
  // Pid
  fPid = (TA2PlasticPID*)((TA2Analysis*)fParent)->GetGrandChild("PID");
  if (!fPid) PrintError("Warning!","<No Pid class found in annalysis>");
  for (Int_t i=0; i<kNch; ++i)
  {
    fZinterMwpcPid[i][2]       = EBufferEnd;
    fDphiInterMwpcHitPid[i][2] = EBufferEnd;
    fIhitPid[i][2]             = EBufferEnd;
  }
  
  // Default physics initialisation
  TA2Physics::PostInit();
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::LoadVariable()
{

  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  TA2Physics::LoadVariable();
  
  // E-IW
  // 0
  TA2DataManager::LoadVariable("PhiInters0",	fPhiInters[0],		EDMultiX);
  TA2DataManager::LoadVariable("ZInters0",	fZinters[0],		EDMultiX);
  TA2DataManager::LoadVariable("DphiEI0",	fDphiEI[0],		EDMultiX);
  TA2DataManager::LoadVariable("CGclI0",	fCGclI[0],		EDMultiX);
  TA2DataManager::LoadVariable("CGclW0",	fCGclW[0],		EDMultiX);
  TA2DataManager::LoadVariable("CGclE0",	fCGclE[0],		EDMultiX);
  TA2DataManager::LoadVariable("PhiEI0",	fPhiEI[0],		EDMultiX);
  // 1
  TA2DataManager::LoadVariable("PhiInters1",	fPhiInters[1],		EDMultiX);
  TA2DataManager::LoadVariable("ZInters1",	fZinters[1],		EDMultiX);
  TA2DataManager::LoadVariable("DphiEI1",	fDphiEI[1],		EDMultiX);
  TA2DataManager::LoadVariable("CGclI1",	fCGclI[1],		EDMultiX);
  TA2DataManager::LoadVariable("CGclW1",	fCGclW[1],		EDMultiX);
  TA2DataManager::LoadVariable("CGclE1",	fCGclE[1],		EDMultiX);
  TA2DataManager::LoadVariable("PhiEI1",	fPhiEI[1],		EDMultiX);
  
  // Mwpc_i -> Mwpc_0
  // 1
  // intersection with XYZ
  TA2DataManager::LoadVariable("DphiMwpc21",		&fDphiMwpc21,		EDSingleX);
  TA2DataManager::LoadVariable("DxMwpc21",		&fDxMwpc21,		EDSingleX);
  TA2DataManager::LoadVariable("DyMwpc21",		&fDyMwpc21,		EDSingleX);
  TA2DataManager::LoadVariable("DzMwpc21",		&fDzMwpc21,		EDSingleX);
  TA2DataManager::LoadVariable("Z0Mwpc21",		&fZ0Mwpc21,		EDSingleX);
  // intersection with MWPC-1
  TA2DataManager::LoadVariable("DxInterMwpc1",		 fDxInterMwpc,		EDMultiX);
  TA2DataManager::LoadVariable("DyInterMwpc1",		 fDyInterMwpc,		EDMultiX);
  TA2DataManager::LoadVariable("DzInterMwpc1",		 fDzInterMwpc,		EDMultiX);
  TA2DataManager::LoadVariable("DphiInterMwpc1",	 fDphiInterMwpc,	EDMultiX);
  TA2DataManager::LoadVariable("ZinterMwpc1",		 fZinterMwpc,		EDMultiX);
  TA2DataManager::LoadVariable("PhiInterMwpc1",		 fPhiInterMwpc,		EDMultiX);
  //
  TA2DataManager::LoadVariable("DPhiIntersMwpc0",	&fDphiIntersMwpc0,	EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracks",		&fDpsiTracks,		EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracks",		&fDphiTracks,		EDSingleX);
  TA2DataManager::LoadVariable("DTetTracks",		&fDtetTracks,		EDSingleX);
  TA2DataManager::LoadVariable("DzPsVertexTracks",	&fDzPsVertexTracks,	EDSingleX);
  TA2DataManager::LoadVariable("ThetaTracks",		&fThetaTracks,		EDSingleX);
  TA2DataManager::LoadVariable("PhiTracks",		&fPhiTracks,		EDSingleX);
  TA2DataManager::LoadVariable("DTetTracks2",		 fDtetTracks2,		EDMultiX);
  TA2DataManager::LoadVariable("ThetaTracks2",		 fThetaTracks2,		EDMultiX);

  // Mwpc_i -> NaI
  // 0
  // intersection with XYZ
  TA2DataManager::LoadVariable("DPhiClNaI0",		&fDphiClNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DphiMwpc0NaI",		&fDphiMwpcNaI[0],	EDSingleX);
  TA2DataManager::LoadVariable("DxMwpc0NaI",		&fDxMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DyMwpc0NaI",		&fDyMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DzMwpc0NaI",		&fDzMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("Z0Mwpc0NaI",		&fZ0MwpcNaI[0],		EDSingleX);
  // intersection with MWPC-1
  TA2DataManager::LoadVariable("DxInterMwpc0NaI",	 fDxInterMwpcNaI[0],		EDMultiX);
  TA2DataManager::LoadVariable("DyInterMwpc0NaI",	 fDyInterMwpcNaI[0],		EDMultiX);
  TA2DataManager::LoadVariable("DzInterMwpc0NaI",	 fDzInterMwpcNaI[0],		EDMultiX);
  TA2DataManager::LoadVariable("DphiInterMwpc0NaI",	 fDphiInterMwpcNaI[0],		EDMultiX);
  TA2DataManager::LoadVariable("ZinterMwpc0NaI",	 fZinterMwpcNaI[0],		EDMultiX);
  TA2DataManager::LoadVariable("PhiInterMwpc0NaI",	 fPhiInterMwpcNaI[0],		EDMultiX);
  //
  TA2DataManager::LoadVariable("DPsiTracksMwpc0NaI",		&fDpsiTracksMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracksSingleMwpc0NaI",	&fDpsiTracksSingleMwpcNaI[0],	EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksMwpc0NaI",		&fDphiTracksMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksSingleMwpc0NaI",	&fDphiTracksSingleMwpcNaI[0],	EDSingleX);
  TA2DataManager::LoadVariable("DTetTracksMwpc0NaI",		&fDtetTracksMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DzPsVertexTracksMwpc0NaI",	&fDzPsVertexTracksMwpcNaI[0],	EDSingleX);
  TA2DataManager::LoadVariable("ThetaTracksMwpc0NaI",		&fThetaTracksMwpcNaI[0],	EDSingleX);
  TA2DataManager::LoadVariable("PhiTracksMwpc0NaI",		&fPhiTracksMwpcNaI[0],		EDSingleX);
  TA2DataManager::LoadVariable("DTetTracksSingleMwpc0NaI",	&fDtetTracksSingleMwpcNaI[0],	EDSingleX);
  // 1
  // intersection with XYZ
  TA2DataManager::LoadVariable("DPhiClNaI1",		&fDphiClNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DphiMwpc1NaI",		&fDphiMwpcNaI[1],	EDSingleX);
  TA2DataManager::LoadVariable("DxMwpc1NaI",		&fDxMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DyMwpc1NaI",		&fDyMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DzMwpc1NaI",		&fDzMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("Z0Mwpc1NaI",		&fZ0MwpcNaI[1],		EDSingleX);
  //  intersection with MWPC-2
  TA2DataManager::LoadVariable("DxInterMwpc1NaI",	 fDxInterMwpcNaI[1],		EDMultiX);
  TA2DataManager::LoadVariable("DyInterMwpc1NaI",	 fDyInterMwpcNaI[1],		EDMultiX);
  TA2DataManager::LoadVariable("DzInterMwpc1NaI",	 fDzInterMwpcNaI[1],		EDMultiX);
  TA2DataManager::LoadVariable("DphiInterMwpc1NaI",	 fDphiInterMwpcNaI[1],		EDMultiX);
  TA2DataManager::LoadVariable("ZinterMwpc1NaI",	 fZinterMwpcNaI[1],		EDMultiX);
  TA2DataManager::LoadVariable("PhiInterMwpc1NaI",	 fPhiInterMwpcNaI[1],		EDMultiX);
  //
  TA2DataManager::LoadVariable("DPsiTracksMwpc1NaI",		&fDpsiTracksMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracksSingleMwpc1NaI",	&fDpsiTracksSingleMwpcNaI[1],	EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksMwpc1NaI",		&fDphiTracksMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksSingleMwpc1NaI",	&fDphiTracksSingleMwpcNaI[1],	EDSingleX);
  TA2DataManager::LoadVariable("DTetTracksMwpc1NaI",		&fDtetTracksMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DzPsVertexTracksMwpc1NaI",	&fDzPsVertexTracksMwpcNaI[1],	EDSingleX);
  TA2DataManager::LoadVariable("ThetaTracksMwpc1NaI",		&fThetaTracksMwpcNaI[1],	EDSingleX);
  TA2DataManager::LoadVariable("PhiTracksMwpc1NaI",		&fPhiTracksMwpcNaI[1],		EDSingleX);
  TA2DataManager::LoadVariable("DTetTracksSingleMwpc1NaI",	&fDtetTracksSingleMwpcNaI[1],	EDSingleX);
  
  // Mwpc_i -> Pid
  // 0
  TA2DataManager::LoadVariable("DphiMwpc0Pid",		&fDphiMwpcPid[0],	EDSingleX);
  TA2DataManager::LoadVariable("DxMwpc0Pid",		&fDxMwpcPid[0],		EDSingleX);
  TA2DataManager::LoadVariable("DyMwpc0Pid",		&fDyMwpcPid[0],		EDSingleX);
  //
  TA2DataManager::LoadVariable("DPhiHitsPid0",		&fDphiHitsPid[0],	EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracksMwpc0Pid",	&fDpsiTracksMwpcPid[0],	EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksMwpc0Pid",	&fDphiTracksMwpcPid[0],	EDSingleX);
  TA2DataManager::LoadVariable("ZinterMwpc0Pid",	fZinterMwpcPid[0],	EDMultiX);
  TA2DataManager::LoadVariable("DphiInterMwpc0HitPid",	fDphiInterMwpcHitPid[0],EDMultiX);
  TA2DataManager::LoadVariable("IhitPid0",		fIhitPid[0],		EIMultiX);
  // 1
  TA2DataManager::LoadVariable("DphiMwpc1Pid",		&fDphiMwpcPid[1],	EDSingleX);
  TA2DataManager::LoadVariable("DxMwpc1Pid",		&fDxMwpcPid[1],		EDSingleX);
  TA2DataManager::LoadVariable("DyMwpc1Pid",		&fDyMwpcPid[1],		EDSingleX);
  //
  TA2DataManager::LoadVariable("DPhiHitsPid1",		&fDphiHitsPid[1],	EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracksMwpc1Pid",	&fDpsiTracksMwpcPid[1],	EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksMwpc1Pid",	&fDphiTracksMwpcPid[1],	EDSingleX);
  TA2DataManager::LoadVariable("ZinterMwpc1Pid",	fZinterMwpcPid[1],	EDMultiX);
  TA2DataManager::LoadVariable("DphiInterMwpc1HitPid",	fDphiInterMwpcHitPid[1],EDMultiX);
  TA2DataManager::LoadVariable("IhitPid1",		fIhitPid[1],		EIMultiX);
  
  // Pid -> NaI
  TA2DataManager::LoadVariable("DphiPidNaI",		&fDphiPidNaI,		EDSingleX);
  TA2DataManager::LoadVariable("DxPidNaI",		&fDxPidNaI,		EDSingleX);
  TA2DataManager::LoadVariable("DyPidNaI",		&fDyPidNaI,		EDSingleX);
  //
  TA2DataManager::LoadVariable("DphiClNaIwithPid",	&fDphiClNaIwithPid,	EDSingleX);
  TA2DataManager::LoadVariable("DPsiTracksPidNaI",	&fDpsiTracksPidNaI,	EDSingleX);
  TA2DataManager::LoadVariable("DPhiTracksPidNaI",	&fDphiTracksPidNaI,	EDSingleX);
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::Reconstruct()
{
  //
  
  //
  ResetEvent();
  
  // Test
//   if (fMwpc->GetNinters(0) > 0 ||fMwpc->GetNinters(1) > 0 )
//   {
//     fMwpc->Test(kTRUE);
//   }
//   fCB->Test(kTRUE);
  
  // Calib
  static Int_t nCh = (fMwpc) ? fMwpc->GetNchamber() : 0;
  for (Int_t i=0; i<nCh; ++i)
  {
    CalibEIW(i);		// E->IW
    CalibMwpc(i);		// Mwpc_i->Mwpc_0
    if (fNaI) CalibMwpcNaI(i);	// Mwpc_i->NaI
    if (fPid) CalibPidMwpc(i);	// Pid->Mwpc_i
  }
  if (fNaI&&fPid) CalibPidNaI();// Pid->NaI

  //
  MarkEndBuffers();
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::CalibEIW(const Int_t iCh)
{
  // Fill arrays to plot histograms for E-IW calibration
  
  // Filling
  const TA2MwpcIntersection *inter;
  for (Int_t i=0; i<fNintersTrue[iCh]; ++i)
  {
    inter = fMwpc->GetInterTrue(iCh,i);
    fPhiInters[iCh][i] = TVector2::Phi_0_2pi(inter->GetPhi())*kRadToDeg;
    fZinters[iCh][i]   = inter->GetZ();
    fDphiEI[iCh][i]    = inter->GetDphiWIE()*kRadToDeg;
    fCGclI[iCh][i]     = fMwpc->GetStripsI(iCh)->GetCGClust(inter->GetIclI()) * fMwpc->GetStripsI(iCh)->GetNElement();
    fCGclW[iCh][i]     = fMwpc->GetWires(iCh)->GetCGClust(inter->GetIclW())   * fMwpc->GetWires(iCh)->GetNElement()/(2.*kPi);
    fCGclE[iCh][i]     = fMwpc->GetStripsE(iCh)->GetCGClust(inter->GetIclE()) * fMwpc->GetStripsE(iCh)->GetNElement();
    fPhiEI[iCh][i]     = TVector2::Phi_0_2pi(fMwpc->GetWires(iCh)->GetCGClust(inter->GetIclW()) - fDphiEI[iCh][i]*kDegToRad)*kRadToDeg;
//     fH3EI_dPhiEI_PhiEI_Zinters[iCh]->Fill(fPhiEI[iCh][i],fZinters[iCh][i],fDphiEI[iCh][i]);
  }
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::CalibMwpc(const Int_t iCh)
{
  // Mwpc_iCh -> Mwpc_0
  
  //
  if (iCh == 0) return;
  //
  if (fNintersTrue[0]   != 2 || fMwpc->GetNinters(0)   != 2) return;
  if (fNintersTrue[iCh] != 2 || fMwpc->GetNinters(iCh) != 2) return;
  if (fMwpc->GetNtracksTrueBest() != 2) return;
  //
//   fMwpc->Test(kTRUE);
  //
  fDphiIntersMwpc0 = ( fMwpc->GetInters(0,0)->GetPhi() + fMwpc->GetInters(0,1)->GetPhi() )*kRadToDeg;
  if (TMath::Abs(fDphiIntersMwpc0) > 30.) return;
  //
  map<Double_t,Int_t>::const_iterator itr = fMwpc->GetTracksTrueBest()->begin();
  Int_t ii[2];
  ii[0] = itr->second;		// index of the 1st track
  ii[1] = (++itr)->second;	// index of the 2nd track
  const TA2MwpcTrack *tracks = fMwpc->GetTracks();
  fDpsiTracks = fMwpc->GetTrack(ii[0])->Angle(fMwpc->GetTrack(ii[1]))*kRadToDeg;
  fDphiTracks = TVector2::Phi_mpi_pi(fMwpc->GetTrack(ii[0])->GetPhi() - fMwpc->GetTrack(ii[1])->GetPhi() - kPi)*kRadToDeg;
  fDtetTracks = (fMwpc->GetTrack(ii[0])->GetTheta() + fMwpc->GetTrack(ii[1])->GetTheta() - kPi)*kRadToDeg;
  fDtetTracks2[0] = fDtetTracks2[1] = fDtetTracks/TMath::Sqrt(2.);
  fThetaTracks2[0] = fMwpc->GetTrack(ii[0])->GetTheta()*kRadToDeg;
  fThetaTracks2[1] = fMwpc->GetTrack(ii[1])->GetTheta()*kRadToDeg;
  Int_t iTrack = ii[gRandom->Integer(2)];
  fThetaTracks = fMwpc->GetTrack(iTrack)->GetTheta()*kRadToDeg;
  fPhiTracks = TVector2::Phi_0_2pi(fMwpc->GetTrack(iTrack)->GetPhi())*kRadToDeg;
  fDzPsVertexTracks = fMwpc->GetTrack(ii[0])->GetPsVertex().Z() - fMwpc->GetTrack(ii[1])->GetPsVertex().Z();
//   if(fDPsiTracks < 160.) return;
  
  // XY, YZ
  const TA2MwpcIntersection *intersMwpc[2];
  Double_t xyInterMwpc[2][2][2], zyInterMwpc[2][2][2];
  for (Int_t i=0; i<2; ++i)
  {
    intersMwpc[0] = fMwpc->GetInters(0,tracks[ii[i]].GetIinter(0));
    intersMwpc[1] = fMwpc->GetInters(iCh,tracks[ii[i]].GetIinter(iCh));
    // X
    xyInterMwpc[0][i][0] = intersMwpc[0]->GetPosition()->X();
    xyInterMwpc[1][i][0] = intersMwpc[1]->GetPosition()->X();
    // Y
    xyInterMwpc[0][i][1] = zyInterMwpc[0][i][1] = intersMwpc[0]->GetPosition()->Y();
    xyInterMwpc[1][i][1] = zyInterMwpc[1][i][1] = intersMwpc[1]->GetPosition()->Y();
    // Z
    zyInterMwpc[0][i][0] = intersMwpc[0]->GetPosition()->Z();
    zyInterMwpc[1][i][0] = intersMwpc[1]->GetPosition()->Z();
  }
  
  // Phi rotation of the external chamber relative to the internal one
  fDphiMwpc21 = TA2Math::CalcRotation2D(xyInterMwpc[0][0],xyInterMwpc[0][1],xyInterMwpc[1][0],xyInterMwpc[1][1])*kRadToDeg;
  
  // XY shift
  Double_t shift[2], x0y0InterMwpc[2][2];
  TA2Math::CalcShift2D(xyInterMwpc[0][0],xyInterMwpc[0][1],xyInterMwpc[1][0],xyInterMwpc[1][1],shift,x0y0InterMwpc[0],x0y0InterMwpc[1]);
  fDxMwpc21 = shift[0];
  fDyMwpc21 = shift[1];

  // YZ shift
  Double_t z0y0InterMwpc[2][2];
  TA2Math::CalcShift2D(zyInterMwpc[0][0],zyInterMwpc[0][1],zyInterMwpc[1][0],zyInterMwpc[1][1],shift,z0y0InterMwpc[0],z0y0InterMwpc[1]);
  fDzMwpc21 = shift[0];
  fZ0Mwpc21 = z0y0InterMwpc[0][0]; // TODO Probably it must be choosen randomly...
  
  // dPhi, dX, dY, dZ for an individual MWPC intersection
  TVector3 r[2];
  const TVector3 *v[2];
  v[0] = fMwpc->GetInters(0,tracks[ii[0]].GetIinter(0))->GetPosition();
  v[1] = fMwpc->GetInters(0,tracks[ii[1]].GetIinter(0))->GetPosition();
  if ( TA2Math::IntersectLineAndCylinder(*v[0],*v[1],*(fMwpc->GetR()+iCh),r[0],r[1]) == 0 ) return;
  for (Int_t i=0; i<2; ++i)
  {
    fDxInterMwpc[i]   = fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetPosition()->X() - r[i].X();
    fDyInterMwpc[i]   = fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetPosition()->Y() - r[i].Y();
    fDzInterMwpc[i]   = fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetPosition()->Z() - r[i].Z();
    fDphiInterMwpc[i] = TVector2::Phi_mpi_pi(fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetPhi() - r[i].Phi())*kRadToDeg;
    fZinterMwpc[i]   = fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetZ();
    fPhiInterMwpc[i] = TVector2::Phi_0_2pi(fMwpc->GetInters(iCh,tracks[i].GetIinter(iCh))->GetPhi())*kRadToDeg;
    fH3Mwpc_dPhi_vs_Phi_vs_Z->Fill(fPhiInterMwpc[i],fZinterMwpc[i],fDphiInterMwpc[i]);
    fH3Mwpc_dZ_vs_Phi_vs_Z->Fill(fPhiInterMwpc[i],fZinterMwpc[i],fDzInterMwpc[i]);
  }
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::CalibMwpcNaI(const Int_t iCh)
{
  //
  
  //
  if (fNaI->GetNCluster() != 2) return;
//   UInt_t *iClNaI = fNaI->GetClustHit();
//   if (fNaI->GetCluster(iClNaI[0])->GetNhits() != 1 ) return;
//   if (fNaI->GetCluster(iClNaI[1])->GetNhits() != 1 ) return;
  fDphiClNaI[iCh] = TVector2::Phi_mpi_pi(fCB->GetPositionsNaI(0)->Phi() + fCB->GetPositionsNaI(1)->Phi() ) *kRadToDeg;
  if (TMath::Abs(fDphiClNaI[iCh]) > 50.) return;
  
  // Selection conditions for cosmic rays
  if (fCB->GetNtracks() != 2 ) return;
  //
  const TA2CentralTrack *tracks = fCB->GetTracks();
  if (!tracks[0].HasNaI() || !tracks[0].HasMwpc(iCh)) return;
  if (!tracks[1].HasNaI() || !tracks[1].HasMwpc(iCh)) return;
  //
  if ( fMwpc->GetInters(iCh,tracks[0].GetIinterMwpc(iCh))->GetType() != TA2MwpcIntersection::kEWI ) return;
  if ( fMwpc->GetInters(iCh,tracks[1].GetIinterMwpc(iCh))->GetType() != TA2MwpcIntersection::kEWI ) return;

  //
//   fCB->Test(kTRUE);
  
  //
  fDpsiTracksMwpcNaI[iCh] = tracks[0].Angle(tracks[1])*kRadToDeg;
  fDphiTracksMwpcNaI[iCh] = TVector2::Phi_mpi_pi(tracks[0].GetPhi() - tracks[1].GetPhi() - kPi)*kRadToDeg;
  fDtetTracksMwpcNaI[iCh] = (tracks[0].GetTheta() + tracks[1].GetTheta() - kPi)*kRadToDeg;
  fDzPsVertexTracksMwpcNaI[iCh] = tracks[0].GetPsVertex().Z() - tracks[1].GetPsVertex().Z();
  Int_t iTrack = gRandom->Integer(2);
  fThetaTracksMwpcNaI[iCh] = tracks[iTrack].GetTheta()*kRadToDeg;
  fPhiTracksMwpcNaI[iCh] = TVector2::Phi_0_2pi(tracks[iTrack].GetPhi())*kRadToDeg;
  if (tracks[0].HasSingleMwpc() && tracks[1].HasSingleMwpc())
  {
    fDpsiTracksSingleMwpcNaI[iCh] = tracks[0].Angle(tracks[1])*kRadToDeg;
    fDphiTracksSingleMwpcNaI[iCh] = TVector2::Phi_mpi_pi(tracks[0].GetPhi() - tracks[1].GetPhi() - kPi) *kRadToDeg;
    fDtetTracksSingleMwpcNaI[iCh] = (tracks[0].GetTheta() + tracks[1].GetTheta() - kPi)*kRadToDeg;
  }
  
  //
  const TA2MwpcIntersection *intersMwpc;
  const TVector3 *posClNaI;
  Double_t xyNaI[2][2], zyNaI[2][2];
  Double_t xyMwpc[2][2], zyMwpc[2][2];
  for (Int_t i=0; i<2; ++i)
  {
    intersMwpc = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh));
    posClNaI   = fCB->GetPositionsNaI(tracks[i].GetIclNaI());
    // X
    xyMwpc[i][0] = intersMwpc->GetPosition()->X();
    xyNaI[i][0]  = posClNaI->X();
    // Y
    xyMwpc[i][1] = zyMwpc[i][1] = intersMwpc->GetPosition()->Y();
    xyNaI[i][1]  = zyNaI[i][1]  = posClNaI->Y();
    // Z
    zyMwpc[i][0] = intersMwpc->GetPosition()->Z();
    zyNaI[i][0]  = posClNaI->Z();
  }

  // Phi rotation of the i-th MWPC relative to the NaI
  fDphiMwpcNaI[iCh] = TA2Math::CalcRotation2D(xyMwpc[0],xyMwpc[1],xyNaI[0],xyNaI[1])*kRadToDeg;
  
  // XY shift
  Double_t shift[2], x0y0Mwpc[2], x0y0NaI[2];
  TA2Math::CalcShift2D(xyMwpc[0],xyMwpc[1],xyNaI[0],xyNaI[1],shift,x0y0Mwpc,x0y0NaI);
  fDxMwpcNaI[iCh] = shift[0];
  fDyMwpcNaI[iCh] = shift[1];

  // YZ shift
  Double_t z0y0Mwpc[2], z0y0NaI[2];
  TA2Math::CalcShift2D(zyMwpc[0],zyMwpc[1],zyNaI[0],zyNaI[1],shift,z0y0Mwpc,z0y0NaI);
  fDzMwpcNaI[iCh] = -shift[0];
  fZ0MwpcNaI[iCh] = z0y0NaI[0]; // TODO Probably it must be choosen randomly...
  
  // Z shift as a function of MwpcZinter
  TVector3 r[2];
  const TVector3 *v[2];
  v[0] = fCB->GetPositionsNaI(tracks[0].GetIclNaI());
  v[1] = fCB->GetPositionsNaI(tracks[1].GetIclNaI());
  if ( TA2Math::IntersectLineAndCylinder(*v[0],*v[1],*(fMwpc->GetR()+iCh),r[0],r[1]) == 0 ) return;
  for (Int_t i=0; i<2; ++i)
  {
    fDxInterMwpcNaI[iCh][i]   = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetPosition()->X() - r[i].X();
    fDyInterMwpcNaI[iCh][i]   = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetPosition()->Y() - r[i].Y();
    fDzInterMwpcNaI[iCh][i]   = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetPosition()->Z() - r[i].Z();
    fDzInterMwpcNaI[iCh][i] = - fDzInterMwpcNaI[iCh][i];
    fDphiInterMwpcNaI[iCh][i] = TVector2::Phi_mpi_pi(fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetPhi() - r[i].Phi())*kRadToDeg;
    fZinterMwpcNaI[iCh][i]   = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetZ();
    fPhiInterMwpcNaI[iCh][i] = TVector2::Phi_0_2pi(fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh))->GetPhi())*kRadToDeg;
    fH3MwpcNaI_dPhi_vs_Phi_vs_Z[iCh]->Fill(fPhiInterMwpcNaI[iCh][i],fZinterMwpcNaI[iCh][i],fDphiInterMwpcNaI[iCh][i]);
    fH3MwpcNaI_dZ_vs_Phi_vs_Z[iCh]->Fill(fPhiInterMwpcNaI[iCh][i],fZinterMwpcNaI[iCh][i],fDzInterMwpcNaI[iCh][i]);
  }
  
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::CalibPidMwpc(const Int_t iCh)
{
  //
  
  //
  if (fPid->GetNhits() != 2) return;
  fDphiHitsPid[iCh] = TVector2::Phi_mpi_pi(fCB->GetPositionsPid(0)->Phi() + fCB->GetPositionsPid(1)->Phi() ) * kRadToDeg;
  if (TMath::Abs(fDphiHitsPid[iCh]) > 30.) return;
  
  // Selection conditions for cosmic rays
  if (fCB->GetNtracks() !=2 ) return;
  //
  const TA2CentralTrack *tracks = fCB->GetTracks();
  if (!tracks[0].HasPid() || !tracks[0].HasMwpc(iCh)) return;
  if (!tracks[1].HasPid() || !tracks[1].HasMwpc(iCh)) return;
  //
  if ( fMwpc->GetInters(iCh,tracks[0].GetIinterMwpc(iCh))->GetType() != TA2MwpcIntersection::kEWI ) return;
  if ( fMwpc->GetInters(iCh,tracks[1].GetIinterMwpc(iCh))->GetType() != TA2MwpcIntersection::kEWI ) return;
  //
  fDpsiTracksMwpcPid[iCh] = tracks[0].Angle(tracks[1])*kRadToDeg;
  fDphiTracksMwpcPid[iCh] = TVector2::Phi_mpi_pi(tracks[0].GetPhi() - tracks[1].GetPhi() - kPi)*kRadToDeg;
  
  //
  const TA2MwpcIntersection *intersMwpc;
  const TVector2 *posHitPid;
  Double_t xyPid[2][2];
  Double_t xyMwpc[2][2];
  for (Int_t i=0; i<2; ++i)
  {
    intersMwpc = fMwpc->GetInters(iCh,tracks[i].GetIinterMwpc(iCh));
    posHitPid   = fCB->GetPositionsPid(tracks[i].GetIhitPid());
    // X
    xyMwpc[i][0] = intersMwpc->GetPosition()->X();
    xyPid[i][0]  = posHitPid->X();
    // Y
    xyMwpc[i][1] = intersMwpc->GetPosition()->Y();
    xyPid[i][1]  = posHitPid->Y();
    //
    fZinterMwpcPid[iCh][i] = intersMwpc->GetPosition()->Z();
    fDphiInterMwpcHitPid[iCh][i] = TVector2::Phi_mpi_pi(posHitPid->Phi() -  intersMwpc->GetPosition()->Phi() ) *kRadToDeg;
    fIhitPid[iCh][i] = fPid->GetHits(tracks[i].GetIhitPid());
  }
  
  // Phi rotation of the i-th MWPC relative to the NaI
  fDphiMwpcPid[iCh] = TA2Math::CalcRotation2D(xyMwpc[0],xyMwpc[1],xyPid[0],xyPid[1])*kRadToDeg;
  
  // XY shift
  Double_t shift[2],x0y0Mwpc[2], x0y0Pid[2];
  TA2Math::CalcShift2D(xyMwpc[0],xyMwpc[1],xyPid[0],xyPid[1],shift,x0y0Mwpc,x0y0Pid);
  fDxMwpcPid[iCh] = shift[0];
  fDyMwpcPid[iCh] = shift[1];
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::CalibPidNaI()
{
  //
  
  //
  if (fNaI->GetNCluster() != 2) return;
  fDphiClNaIwithPid = TVector2::Phi_mpi_pi(fCB->GetPositionsNaI(0)->Phi() + fCB->GetPositionsNaI(1)->Phi() ) * kRadToDeg;
  if (TMath::Abs(fDphiClNaIwithPid) > 50.) return;
  
  // Selection conditions for cosmic rays
  if (fCB->GetNtracks() !=2 ) return;
  //
  const TA2CentralTrack *tracks = fCB->GetTracks();
  if (!tracks[0].HasPid() || !tracks[0].HasNaI()) return;
  if (!tracks[1].HasPid() || !tracks[1].HasNaI()) return;
  //
  fDpsiTracksPidNaI = tracks[0].Angle(tracks[1])*kRadToDeg;
  fDphiTracksPidNaI = TVector2::Phi_mpi_pi(tracks[0].GetPhi() - tracks[1].GetPhi() - kPi)*kRadToDeg;
  
  //
  const TVector3 *posClNaI;
  const TVector2 *posHitPid;
  Double_t xyPid[2][2];
  Double_t xyNaI[2][2];
  for (Int_t i=0; i<2; ++i)
  {
    posClNaI  = fCB->GetPositionsNaI(tracks[i].GetIclNaI());
    posHitPid = fCB->GetPositionsPid(tracks[i].GetIhitPid());
    // X
    xyNaI[i][0] = posClNaI->X();
    xyPid[i][0] = posHitPid->X();
    // Y
    xyNaI[i][1] = posClNaI->Y();
    xyPid[i][1] = posHitPid->Y();
  }
  
  // Phi rotation of the i-th MWPC relative to the NaI
  fDphiPidNaI = TA2Math::CalcRotation2D(xyNaI[0],xyNaI[1],xyPid[0],xyPid[1])*kRadToDeg;
  
  // XY shift
  Double_t shift[2], x0y0NaI[2], x0y0Pid[2];
  TA2Math::CalcShift2D(xyNaI[0],xyNaI[1],xyPid[0],xyPid[1],shift,x0y0NaI,x0y0Pid);
  fDxPidNaI = shift[0];
  fDyPidNaI = shift[1];
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::ResetEvent()
{
  // Set default value for the variables being chenged during an event
  
  // Mwpc2-Mwpc1
  fDphiMwpc21 = ENullFloat;
  fDxMwpc21 = fDyMwpc21 = fDzMwpc21 = fZ0Mwpc21 = ENullFloat;
  fDphiIntersMwpc0 = fDpsiTracks = fDphiTracks = fDtetTracks = fThetaTracks = fPhiTracks = fDzPsVertexTracks = ENullFloat;
  //
  fDxInterMwpc[0] = fDxInterMwpc[1] = ENullFloat;
  fDyInterMwpc[0] = fDyInterMwpc[1] = ENullFloat;
  fDzInterMwpc[0] = fDzInterMwpc[1] = ENullFloat;
  fDphiInterMwpc[0] = fDphiInterMwpc[1] = ENullFloat;
  fZinterMwpc[0]   = fZinterMwpc[1]   = ENullFloat;
  fPhiInterMwpc[0] = fPhiInterMwpc[1] = ENullFloat;
  fDtetTracks2[0] = fDtetTracks2[1] = ENullFloat;
  fThetaTracks2[0] = fThetaTracks2[1] = ENullFloat;
  
  // Mwpc-NaI
  for (Int_t i=0; i<kNch; ++i)
  {
    fDphiMwpcNaI[i] = ENullFloat;
    fDxMwpcNaI[i] = fDyMwpcNaI[i] = fDzMwpcNaI[i] = ENullFloat;
    fZ0MwpcNaI[i] = ENullFloat;
    fDphiClNaI[i] = ENullFloat;
    fDpsiTracksMwpcNaI[i] = fDpsiTracksSingleMwpcNaI[i] = ENullFloat;
    fDphiTracksMwpcNaI[i] = fDphiTracksSingleMwpcNaI[i] = ENullFloat;
    fDtetTracksMwpcNaI[i] = fDtetTracksSingleMwpcNaI[i] = ENullFloat;
    fDzPsVertexTracksMwpcNaI[i] = ENullFloat;
    fThetaTracksMwpcNaI[i] = ENullFloat;
    fPhiTracksMwpcNaI[i] = ENullFloat;
    //
    fDxInterMwpcNaI[i][0]   = fDxInterMwpcNaI[i][1]   = ENullFloat;
    fDyInterMwpcNaI[i][0]   = fDyInterMwpcNaI[i][1]   = ENullFloat;
    fDzInterMwpcNaI[i][0]   = fDzInterMwpcNaI[i][1]   = ENullFloat;
    fDphiInterMwpcNaI[i][0] = fDphiInterMwpcNaI[i][1] = ENullFloat;
    fZinterMwpcNaI[i][0]   = fZinterMwpcNaI[i][1]   = ENullFloat;
    fPhiInterMwpcNaI[i][0] = fPhiInterMwpcNaI[i][1] = ENullFloat;
  }
  
  // Pid-Mwpc
  for (Int_t i=0; i<kNch; ++i)
  {
    fDphiMwpcPid[i] = ENullFloat;
    fDxMwpcPid[i] = fDyMwpcPid[i] = ENullFloat;
    //
    fDphiHitsPid[i] = fDphiTracksMwpcPid[i] = fDpsiTracksMwpcPid[i] = ENullFloat;
    for (Int_t j=0; j<2; ++j)
    {
      fZinterMwpcPid[i][j] = fDphiInterMwpcHitPid[i][j] = ENullFloat;
      fIhitPid[i][j] = ENullHit;
    }
  }

  // Pid -> NaI
  fDphiPidNaI = ENullFloat;
  fDxPidNaI = fDyPidNaI = ENullFloat;
  //
  fDphiClNaIwithPid = fDphiTracksPidNaI = fDpsiTracksPidNaI = ENullFloat;
  
}

//_____________________________________________________________________________________
void TA2GeomCalibPhysics::MarkEndBuffers()
{
  // Mark end buffers for arrays
  
  // E-IW
  for (Int_t i=0; i<kNch; ++i)
  {
    fPhiInters[i][fNintersTrue[i]] = EBufferEnd;
    fZinters[i][fNintersTrue[i]]   = EBufferEnd;
    fDphiEI[i][fNintersTrue[i]]    = EBufferEnd;
    fCGclI[i][fNintersTrue[i]]     = EBufferEnd;
    fCGclW[i][fNintersTrue[i]]     = EBufferEnd;
    fCGclE[i][fNintersTrue[i]]     = EBufferEnd;
    fPhiEI[i][fNintersTrue[i]]     = EBufferEnd;
  }
  
}

ClassImp(TA2GeomCalibPhysics)
