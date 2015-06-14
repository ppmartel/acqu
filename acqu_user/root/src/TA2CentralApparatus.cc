// ROOT
#include "TF1.h"
#include "TCanvas.h"
#include "TArc.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TMarker.h"
#include "TH2F.h"
#include "TMinuit.h"
// AcquRoot
#include "TA2UserAnalysis.h"
#include "TA2CentralApparatus.h"
#include "TA2PlasticPID.h"
#include "TA2CylMwpc.h"
#include "TA2CalArray.h"
// #include "HitCluster_t.h"
#include "TA2CentralTrack.h"
// #include "TA2MwpcTrack.h"
#include "TA2Particle.h"

// Default list of detector classes that the TA2CentralApparatus
// apparatus may contain
enum { ECB_PlasticPID, ECB_CalArray, ECB_CylMWPC, ECB_TA2CylMwpc};
static Map_t kValidDetectors[] = {
  {"TA2PlasticPID",	ECB_PlasticPID},
  {"TA2CylMwpc",	ECB_TA2CylMwpc},
  {"TA2CalArray",	ECB_CalArray},
  {NULL, 		-1}
};

// Valid Keywords for command-line setup of CB apparatus
enum { ECBAngleLimits = 1000, ECBParticleCuts,
       ECBUseTracksBestMwpc, ECBDroop, ECBTrackLimits };
static const Map_t kCBKeys[] = {
  {"AngleLimits:",      ECBAngleLimits},
  {"ParticleID-Cut:",   ECBParticleCuts},
  {"UseBestMwpcTracks:",ECBUseTracksBestMwpc},
  {"Droop:",             ECBDroop}, // TODO It would be better to move it to TA2PlasticPID
  {"TrackLimits:",      ECBTrackLimits},
  {NULL,            -1}
};

  // needed for the display
  const Double_t dphi = 15; // deg
  const Double_t innerPID = 58.;  // PID
  const Double_t outerPID = 62.;  // PID
  const Double_t PIDlength = 500.; 
  const Double_t innerCB = 254.;
  const Double_t outerCB = 660.4; //360.4; 
  const Double_t outerCB_yz = 660.4;
  const Double_t outerCB_xz = 660.4;
  const Double_t shortCB = 51.;
  const Double_t longCB = 254.;
  //   const Double_t dphiCB = 2*TMath::ASin((shortCB/2)/longCB)*kRadToDeg;
  const Double_t dphiCB = 360./31.; // deg
  const Double_t thetaMinCB = 21.; // deg
  const Double_t thetaMaxCB = 159.; // deg
  const Double_t innerMWPC = 74.;
  const Double_t outerMWPC = 94.5;
  const Double_t MWPClength = 650.;
  const Double_t targetLength = 100.;
  const Double_t targetDiameter = 21.5;

  TPolyLine *contourPID[25], *contourCB[32], *contourCB_yz[13], *contourCB2_yz[13];
  TPolyLine *contourCB_xz[13], *contourCB2_xz[13];

//_________________________________________________________________________________________
// TA2CentralApparatus
//
// The TA2CentralApparatus class describes the PID, cylindrical MWPC and Crystall Ball as a single apparatus and
// provides methods for geometrical assotiation of their signals combining them into the particles tracks.
//

//_______________________________________________________________________________
  TA2CentralApparatus::TA2CentralApparatus( const char* name, TA2System* analysis ) : TA2Apparatus( name, analysis, kValidDetectors )
{
  // Zero pointers and counters, add local SetConfig command list
  // to list of other apparatus commands
   
  // Test
  fWait = kFALSE;
  fDisplay = kFALSE;
  
  // Parameters/Options
  fUseTracksBestMwpc = kFALSE;
  // Angle limits
  fMaxPhiMwpcNaI = 1000.;
  fMaxPhiTrackPid = 1000.;
  fMaxPhiPidNaI = 1000.;
  fMaxPhiMwpcInterNaI = 1000.;
  fMaxPhiMwpcInterPid = 1000.;
  // Track limits
  fMaxTrack = 10;
  fLimitsPsVertex[0] = 1000.;
  fLimitsPsVertex[1] = 1000.;
  fLimitsPsVertex[2] = -1000.;
  // NaI 
  fFactorTrackLengthNaI = 1.; // TEST option being tested
  // Pid droop
  fLengthPid = 30.;
  fNpointsDroopPid = 0;
  fLostCorrPosPid = NULL;
  fLostCorrFacPid = NULL;
  
  // Detectors
  // PID
  fPid = NULL;
  fNhitsPid = 0;
  fHitsPid = NULL;
  fPositionsPid = NULL;
  fIhitsPidUsed = NULL;
  fIhitsPidNotUsed = NULL;
  // MWPC
  fMwpc = NULL;
  fNchambers = 0;
  fTracksMwpc = NULL;
  fTracksTrueMwpc = NULL;
  fTracksTrueCandidateMwpc = NULL;
  fTracksCandidateMwpc = NULL;
  fNintersTrueMwpc = NULL;
  fIintersTrueMwpc = NULL;
  fNintersCandidateMwpc = NULL;
  fIintersCandidateMwpc = NULL;
  // NaI
  fNaI = NULL;
  fNclNaI = 0;
  fClNaI = NULL;
  fIdClNaI = NULL;
  fUsedClNaI = NULL;
  fPositionsNaI = NULL;
  
  // Tracks
  fNtracks = 0;
  fTracks = NULL;
  fNchTracks = 0;
  fIchTracks = NULL;
  fNneTracks = 0;
  fIneTracks = NULL;
  // Single MWPC tracks
  fNtracksSingleMwpc = 0;
  fTracksSingleMwpc = NULL;
  fNtracksSingleMwpcBest = 0;
  // Not used MWPC intersections
  fNintersTrueMwpcNotUsed = NULL;
  fNintersCandidateMwpcNotUsed = NULL;
  // For histograms
  fPhiMwpcTrackNaI = NULL;
  fPhiTrackPid = NULL;
  fPhiMwpcTrackPid = NULL;
  fPhiInterSingleMwpcNaI = NULL;
  fPhiTrackSingleMwpcPid = NULL;
  fPhiMwpcInterNaI = NULL;
  fPhiMwpcInterPid = NULL;
  fPhiPidNaI = NULL;
  fTrackType = NULL;
  fTrackTheta = NULL;
  fTrackPhi = NULL;
  fEclNaI = NULL;
  fEhitPid = NULL;
  fEtrackMwpc = NULL;
  fEtrackMwpc0 = NULL; 
  fEtrackMwpc1 = NULL;
  fPsVertex[0] = fPsVertex[1] = fPsVertex[2] = fPsVertexR = NULL;
  
  // Vertexes
  fNvertexes = 0;
  fVertexes = NULL;
  fIvertexes = NULL;
  // For histograms
  for(Int_t i=0; i<3; ++i) fVert[i] = NULL;
  fVertR = NULL;
  
  // ParticleInfo
  fDet = NULL;
  fSize = NULL;
  fCentral = NULL;
  fTime    = NULL;
  fType    = NULL;
  
  //
  AddCmdList(kCBKeys); // for SetConfig()
}

//_______________________________________________________________________________
TA2CentralApparatus::~TA2CentralApparatus()
{
  // Free up allocated memory
  
  DeleteArrays();
}

//_______________________________________________________________________________
void TA2CentralApparatus::DeleteArrays()
{
  // Free up all allocated memory
  
  // Parameters
  if (fLostCorrFacPid) delete [] fLostCorrFacPid;
  if (fLostCorrPosPid) delete [] fLostCorrPosPid;
  
  // Detectors
  if (fPid) {
    delete fPid; // PID
    delete [] fPositionsPid;
    delete [] fIhitsPidUsed;
    delete [] fIhitsPidNotUsed;
  }
  if (fMwpc) {
    delete fMwpc;// MWPC
    delete [] fIintersTrueMwpc;
    delete [] fIintersCandidateMwpc;
    delete [] fNintersTrueMwpcNotUsed;
    delete [] fNintersCandidateMwpcNotUsed;
  }
  if (fNaI) {
    delete fNaI; // NaI
    delete [] fPositionsNaI;
    delete [] fUsedClNaI;
  }
  
  // Tracks
  delete [] fTracks;
  delete [] fIchTracks;
  delete [] fIneTracks;
  //
  delete [] fTracksSingleMwpc;
  // For histograms
  delete [] fPhiMwpcTrackNaI;
  delete [] fPhiTrackPid;
  delete [] fPhiMwpcTrackPid;
  delete [] fPhiInterSingleMwpcNaI;
  delete [] fPhiTrackSingleMwpcPid;
  delete [] fPhiMwpcInterNaI;
  delete [] fPhiMwpcInterPid;
  delete [] fPhiPidNaI;
  delete [] fTrackType;
  delete [] fTrackTheta;
  delete [] fTrackPhi;
  delete [] fEclNaI;
  delete [] fEhitPid;
  delete [] fEtrackMwpc;
  delete [] fEtrackMwpc0;
  delete [] fEtrackMwpc1;    
  for (Int_t i=0;i<3;++i) delete [] fPsVertex[i];
  delete [] fPsVertexR;
  
  // Vertexes
  delete [] fVertexes;
  for (Int_t i=0; i<fMaxTrack; ++i) delete [] fIvertexes[i];
  delete [] fIvertexes;
  //
  for (Int_t i=0;i<3;++i) delete [] fVert[i];
  delete [] fVertR;
  delete [] fTrackDist;
  
  // Particles
  delete [] fParticleInfo;
  delete [] fType;
  delete [] fDet;
  delete [] fSize;
  delete [] fCentral;
  delete [] fTime;
}

//_______________________________________________________________________________
TA2DataManager* TA2CentralApparatus::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the TA2CentralApparatus
  // Valid detector types are...
  // 1. TA2PlasticPID
  // 2. TA2CalArray
  // 3. TA2CylMwpc
  
  if ( !name ) name = Map2String( dclass );
  switch (dclass) {
  default:
    return NULL;
  case ECB_PlasticPID:
    fPid = new TA2PlasticPID( name, this );
    return fPid;
  case ECB_CalArray:
    fNaI = new TA2CalArray( name, this );
    return fNaI;
  case ECB_TA2CylMwpc:
    fMwpc = new TA2CylMwpc( name, this );
    return fMwpc;
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup.
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  // array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  // EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  // MultiX for a multi-valued variable
  
  TA2DataManager::LoadVariable("Ntracks",	&fNtracks,	EISingleX);
  TA2DataManager::LoadVariable("NchTracks",	&fNchTracks,	EISingleX);
  TA2DataManager::LoadVariable("NneTracks",	&fNneTracks,	EISingleX);
  
  TA2DataManager::LoadVariable("PhiMwpcTrackNaI",	fPhiMwpcTrackNaI,	EDMultiX);
  TA2DataManager::LoadVariable("PhiTrackPid",	fPhiTrackPid,	EDMultiX);
  TA2DataManager::LoadVariable("PhiMwpcTrackPid",	fPhiMwpcTrackPid,	EDMultiX);
  TA2DataManager::LoadVariable("PhiInterSingleMwpcNaI",	fPhiInterSingleMwpcNaI,	EDMultiX);
  TA2DataManager::LoadVariable("PhiTrackSingleMwpcPid",	fPhiTrackSingleMwpcPid,	EDMultiX);
  TA2DataManager::LoadVariable("PhiMwpcInterNaI",	fPhiMwpcInterNaI,	EDMultiX);
  TA2DataManager::LoadVariable("PhiMwpcInterPid",	fPhiMwpcInterPid,	EDMultiX);

  TA2DataManager::LoadVariable("PhiPidNaI",		fPhiPidNaI,		EDMultiX);
  
  TA2DataManager::LoadVariable("TrackTypes",	 fTrackType,	EDMultiX);
  TA2DataManager::LoadVariable("TrackTheta",	 fTrackTheta,	EDMultiX);
  TA2DataManager::LoadVariable("TrackPhi",	 fTrackPhi,	EDMultiX);
  TA2DataManager::LoadVariable("EclNaI",	 fEclNaI,	EDMultiX);
  TA2DataManager::LoadVariable("EhitPid",	 fEhitPid,	EDMultiX);
  TA2DataManager::LoadVariable("EtrackMwpc",	 fEtrackMwpc,	EDMultiX);
  TA2DataManager::LoadVariable("EtrackMwpc0",	 fEtrackMwpc0,	EDMultiX);  
  TA2DataManager::LoadVariable("EtrackMwpc1",	 fEtrackMwpc1,	EDMultiX);  
  TA2DataManager::LoadVariable("PsVertexX",	 fPsVertex[0],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexY",	 fPsVertex[1],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexZ",	 fPsVertex[2],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexR",	 fPsVertexR,	EDMultiX);
  
  // Vertexes
  TA2DataManager::LoadVariable("VertexN",	&fNvertexes,	EISingleX);
  TA2DataManager::LoadVariable("VertexX",	fVert[0],	EDMultiX);
  TA2DataManager::LoadVariable("VertexY",	fVert[1],	EDMultiX);
  TA2DataManager::LoadVariable("VertexZ",	fVert[2],	EDMultiX);
  TA2DataManager::LoadVariable("VertexR",	fVertR,	EDMultiX);
  TA2DataManager::LoadVariable("TrackDist",	fTrackDist,	EDMultiX);
  
  // ParticleInfo
  TA2DataManager::LoadVariable("Type",	fType,	EIMultiX);
  
  TA2Apparatus::LoadVariable();
}

//_______________________________________________________________________________
void TA2CentralApparatus::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus
  
  Double_t dparm[20];
  
  switch (key){
  case ECBUseTracksBestMwpc:
    // Use best MWPC tracks
    fUseTracksBestMwpc = kTRUE;
    cout << "TA2CentralApparatus: Best MWPC tracks are used" << endl;
    break;
  case ECBDroop:
    // Droop correction for light attenuation inside PID strips
    if( sscanf( line, "%lf%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		&fLengthPid, &fNpointsDroopPid,
		dparm, dparm+1, dparm+2, dparm+3, dparm+4, dparm+5, dparm+6, dparm+7, dparm+8, dparm+9,
		dparm+10, dparm+11, dparm+12, dparm+13, dparm+14, dparm+15, dparm+16, dparm+17, dparm+18, dparm+19 ) < 2*fNpointsDroopPid + 2 )
      {
	PrintError(line,"<Droop correction for light attenuation inside PID strips>");
	return;
      }
    fLostCorrPosPid = new Double_t[fNpointsDroopPid];
    fLostCorrFacPid = new Double_t[fNpointsDroopPid];
    for(Int_t i=0; i<fNpointsDroopPid; ++i)
      {
	fLostCorrPosPid[i] = dparm[i];
	fLostCorrFacPid[i] = dparm[fNpointsDroopPid+i];
      }
    break;
  case ECBAngleLimits:
    // Angle difference limits
    if ( sscanf(line,"%lf%lf%lf%lf%lf",&fMaxPhiMwpcNaI,&fMaxPhiTrackPid,&fMaxPhiPidNaI,&fMaxPhiMwpcInterNaI,&fMaxPhiMwpcInterPid) != 5 )
      {
	PrintError(line,"<CB angle difference limits>");
	return;
      }
    fMaxPhiMwpcNaI *= kDegToRad;
    fMaxPhiTrackPid *= kDegToRad;
    fMaxPhiPidNaI *= kDegToRad;
    fMaxPhiMwpcInterNaI *= kDegToRad;
    fMaxPhiMwpcInterPid *= kDegToRad;
    break;
  case ECBTrackLimits:
    // Track limits
    if( sscanf(line,"%d%lf%lf%lf",&fMaxTrack,&fLimitsPsVertex[0],&fLimitsPsVertex[1],&fLimitsPsVertex[2]) != 4 )
      {
	PrintError(line,"<Track limits>");
	return;
      }
    break;
  default:
    // default main apparatus SetConfig()
    TA2Apparatus::SetConfig( line, key );
    break;
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::PostInit()
{
  // Initialise arrays used to correlate hits in NaI, PiD and MWPC detectors.
  // Load 2D cuts file and get the contained cuts classes
  // Demand particle ID class...if not there self destruct
  // Does not come back if called with EErrFatal
  
  // Pointer to PID hits, elements, paddle positions
  if (fPid)
    {
      fHitsPid = fPid->GetHits();
      fPositionsPid = new TVector2[fPid->GetNelement()];
      fIhitsPidUsed = new Int_t[fPid->GetNelement()];
      fIhitsPidNotUsed = new Bool_t[fPid->GetNelement()];
    }
  
  // Init the pointers to MWPC tracks
  if (fMwpc)
    {
      fNchambers = fMwpc->GetNchamber();
      //
      fRmwpc = fMwpc->GetR();
      fTracksMwpc = fMwpc->GetTracks();
      if (fUseTracksBestMwpc)
	{
	  fTracksTrueMwpc = fMwpc->GetTracksTrueBest();
	  fTracksTrueCandidateMwpc = fMwpc->GetTracksTrueCandidateBest();
	  fTracksCandidateMwpc = fMwpc->GetTracksCandidateBest();
	}
      else
	{
	  fTracksTrueMwpc = fMwpc->GetTracksTrue();
	  fTracksTrueCandidateMwpc = fMwpc->GetTracksTrueCandidate();
	  fTracksCandidateMwpc = fMwpc->GetTracksCandidate();
	}
      fNintersTrueMwpc = fMwpc->GetNintersTrue();
      fIintersTrueMwpc = new const Int_t*[fNchambers];
      fNintersCandidateMwpc = fMwpc->GetNintersCandidate();
      fIintersCandidateMwpc = new const Int_t*[fNchambers];
      fNintersTrueMwpcNotUsed = new Int_t[fNchambers];
      fNintersCandidateMwpcNotUsed = new Int_t[fNchambers];
      for (Int_t i=0; i<fNchambers; ++i)
	{
	  fIintersTrueMwpc[i] = fMwpc->GetIintersTrue(i);
	  fIintersCandidateMwpc[i] = fMwpc->GetIintersCandidate(i);
	  fNintersTrueMwpcNotUsed[i] = 0;
	  fNintersCandidateMwpcNotUsed[i] = 0;
	}
    }
  
  // Pointer to NaI clasters
  if (fNaI)
    {
      fClNaI = fNaI->GetCluster();
      fIdClNaI = fNaI->GetClustHit();
      fPositionsNaI = new TVector3[fNaI->GetMaxCluster()];
      fUsedClNaI = new Bool_t[fNaI->GetMaxCluster()];
      for (UInt_t i=0; i<fNaI->GetMaxCluster(); ++i) fUsedClNaI[i] = kFALSE;
    }
  
  // Tracks
  fTracks = new TA2CentralTrack[fMaxTrack];
  fIchTracks = new Int_t[fMaxTrack];
  fIneTracks = new Int_t[fMaxTrack];
  fTracksSingleMwpc = new TA2CentralTrack[fMaxTrack];
  // For histograms
  fPhiMwpcTrackNaI = new Double_t[fMaxTrack+1];	fPhiMwpcTrackNaI[fMaxTrack] = EBufferEnd;
  fPhiTrackPid = new Double_t[fMaxTrack+1];	fPhiTrackPid[fMaxTrack] = EBufferEnd;
  fPhiMwpcTrackPid = new Double_t[fMaxTrack+1];	fPhiMwpcTrackPid[fMaxTrack] = EBufferEnd;
  fPhiInterSingleMwpcNaI = new Double_t[fMaxTrack+1];	fPhiInterSingleMwpcNaI[fMaxTrack] = EBufferEnd;
  fPhiTrackSingleMwpcPid = new Double_t[fMaxTrack+1];	fPhiTrackSingleMwpcPid[fMaxTrack] = EBufferEnd;
  fPhiMwpcInterNaI = new Double_t[fMaxTrack+1];	fPhiMwpcInterNaI[fMaxTrack] = EBufferEnd;
  fPhiMwpcInterPid = new Double_t[fMaxTrack+1];	fPhiMwpcInterPid[fMaxTrack] = EBufferEnd;
  fPhiPidNaI = new Double_t[fMaxTrack+1];	fPhiPidNaI[fMaxTrack] = EBufferEnd;
  fTrackType = new Double_t[fMaxTrack+1];
  fTrackTheta = new Double_t[fMaxTrack+1];
  fTrackPhi = new Double_t[fMaxTrack+1];
  fEclNaI = new Double_t[fMaxTrack+1];
  fEhitPid = new Double_t[fMaxTrack+1];
  fEtrackMwpc = new Double_t[fMaxTrack+1];
  fEtrackMwpc0 = new Double_t[fMaxTrack+1];
  fEtrackMwpc1 = new Double_t[fMaxTrack+1];
  for (Int_t i=0;i<3;++i) fPsVertex[i] = new Double_t[fMaxTrack+1];
  fPsVertexR = new Double_t[fMaxTrack+1];
  
  // Vertexes
  Int_t fMaxVertex = fMaxTrack*(fMaxTrack-1)>>1;
  fVertexes = new TVector3[fMaxVertex];
  fIvertexes = new Int_t*[fMaxTrack];
  for (Int_t i=0; i<fMaxTrack; ++i)
    {
      fIvertexes[i] = new Int_t[fMaxTrack];
      fIvertexes[i][i] = -1;
    }
  // for histograming
  for (Int_t i=0;i<3;++i) fVert[i] = new Double_t[fMaxVertex+1];
  fVertR = new Double_t[fMaxVertex+1];
  fTrackDist = new Double_t[fMaxVertex+1];
  
  // ParticleInfo
  fMaxParticle = fMaxTrack;
  fParticleInfo = new TA2Particle[fMaxParticle];
  fType = new Int_t[fMaxParticle];
  fDet = new Int_t[fMaxParticle];
  fSize = new Int_t[fMaxParticle];
  fCentral = new Int_t[fMaxParticle];
  fTime = new Double_t[fMaxParticle];
  
  // Finally call the default apparatus post initialise
  TA2Apparatus::PostInit();
}

//_______________________________________________________________________________
void TA2CentralApparatus::Cleanup()
{
  // Reset event
  
  //
  TA2Apparatus::Cleanup();
  
  //
  for (Int_t i=0; i<fNparticle; ++i) fP4[i].SetXYZT(0.,0.,0.,0.);
  fNparticle = 0;
  
  // Reset Pid
  if (fPid) {
    fNhitsPidNotUsed = 0;
    for(UInt_t i=0; i<fPid->GetNelement(); ++i) {
      fIhitsPidUsed[i] = 0;
      fIhitsPidNotUsed[i] = kFALSE;
    }
  }
  
  // Reset Mwpc
  if (fMwpc) {
    for(Int_t i=0; i<fNchambers; ++i) {
      fNintersTrueMwpcNotUsed[i] = fNintersCandidateMwpcNotUsed[i] = 0;
    }
  }
  
  // Reset NaI
  if (fNaI) {
    for(UInt_t i=0; i<fNaI->GetMaxCluster(); ++i) {
      fUsedClNaI[i] = kFALSE;
    }
  }
  
  // Tracks
  fNtracks = 0;
  fNchTracks = 0;
  fNneTracks = 0;
  // Tracks with 2 MWPC
  fMapMwpcTrueTrackNaI.clear();
  fMapMwpcTrueCandidateTrackNaI.clear();
  fMapMwpcCandidateTrackNaI.clear();
  // Tracks with 1 MWPC
  fNtracksSingleMwpc = 0;
  fMapMwpcTrueInterNaI.clear();
  fMapMwpcCandidateInterNaI.clear();
  fNtracksSingleMwpcBest = 0;
  // For histograms
  for (Int_t i=0; i<fMaxTrack; ++i)
    {
      fPhiMwpcTrackNaI[i] = kNullFloat;
      fPhiTrackPid[i] = kNullFloat;
      fPhiMwpcTrackPid[i] = kNullFloat;
      fPhiInterSingleMwpcNaI[i] = kNullFloat;
      fPhiTrackSingleMwpcPid[i] = kNullFloat;
      fPhiMwpcInterNaI[i] = kNullFloat;
      fPhiMwpcInterPid[i] = kNullFloat;
      fPhiPidNaI[i] = kNullFloat;
      for (Int_t j=0; j<3; ++j) fPsVertex[j][i] = kNullFloat;
      fPsVertexR[i] = kNullFloat;
    }
  
  // Vertexes
  fNvertexes = 0;
}

//_______________________________________________________________________________
void TA2CentralApparatus::Reconstruct()
{
  // Main reconstruction function
  
  // # of hits in PID
  if (fPid)
    {
      Double_t rPid, phiPid;
      TVector3 **pos = fPid->GetPosition();
      fNhitsPid = fPid->GetNhits();
      for (Int_t i=0; i<fNhitsPid; ++i)
	{
	  rPid = pos[fHitsPid[i]]->X();	// Pid radius in mm
	  phiPid = pos[fHitsPid[i]]->Z()*kDegToRad;	// Pid phi in radians
	  fPositionsPid[i] = TVector2(rPid*TMath::Cos(phiPid),rPid*TMath::Sin(phiPid));
	}
    }
  else
    fNhitsPid = 0;
  
  // # of clastrers in NaI
  if (fNaI)
    {
      fNclNaI = fNaI->GetNCluster();
      for (Int_t i=0; i<fNclNaI; ++i)
	{
	  fPositionsNaI[i] = *(fClNaI[fIdClNaI[i]]->GetMeanPosition())*fFactorTrackLengthNaI*10.;// TODO: init fFactorPenetroNaI in cfg
	}
    }
  else
    fNclNaI = 0;
  
  // PID-MWPC-CB geometrical association
  if (fMwpc) fMwpc->ResetWiresAndStripsUsage();
  MakeTracks();
  
  // Reconstruct vertexes
  MakeVertexes();
  
  // Mark end buffers
  MarkEndBuffers();

  // Test
  // Test();
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracks()
{
  // Make tracks of particles comming from the target
  
  if (fMwpc)
    {
      // Tracks with 2 MWPC
      if (fMwpc->GetNtracks() > 0)
	{
	  // PID & MWPC(true) & NaI
	  MakeTracksTrue(fTracksTrueMwpc,fMapMwpcTrueTrackNaI);
	  // PID & MWPC(true-candidate) & NaI
	  MakeTracksTrue(fTracksTrueCandidateMwpc,fMapMwpcTrueCandidateTrackNaI);
	  // PID & MWPC(true)
	  MakeTracksPidMwpc(fTracksTrueMwpc);
	  // PID & MWPC(true-candidate)
	  MakeTracksPidMwpc(fTracksTrueCandidateMwpc);
	  // PID & MWPC(candidate) & NaI
	  MakeTracksTrue(fTracksCandidateMwpc,fMapMwpcCandidateTrackNaI);
	  // PID & MWPC(candidate)
	  MakeTracksPidMwpc(fTracksCandidateMwpc);
	  // MWPC(true)
	  MakeTracksMwpc(fTracksTrueMwpc);
	  // MWPC(true-candidate)
	  MakeTracksMwpc(fTracksTrueCandidateMwpc);
	}
    
      // Tracks with 1 MWPC
      // PID & MWPC(true intersections) & NaI
      MakeTracksTrue(fIintersTrueMwpc,fNintersTrueMwpc,fMapMwpcTrueInterNaI);
      // PID & MWPC(true intersections)
      MakeTracksPidMwpc(fIintersTrueMwpc,fNintersTrueMwpc);
      // PID & MWPC(condidate intersections) & NaI
      MakeTracksTrue(fIintersCandidateMwpc,fNintersCandidateMwpc,fMapMwpcCandidateInterNaI);
      // PID & MWPC(condidate intersections)
      MakeTracksPidMwpc(fIintersCandidateMwpc,fNintersCandidateMwpc);
    
      // Tracks with 2 MWPC
      // MWPC(candidate tracks)
      if (fMwpc->GetNtracksCandidate() > 0)
	{
	  MakeTracksMwpc(fTracksCandidateMwpc);
	}
    
      // Not used MWPC true intersections
      CountNotUsedIntersMwpc(fIintersTrueMwpc,fNintersTrueMwpc,fNintersTrueMwpcNotUsed);
      // Not used MWPC candidate intersections
      CountNotUsedIntersMwpc(fIintersCandidateMwpc,fNintersCandidateMwpc,fNintersCandidateMwpcNotUsed);
    }
  
  // Tracks without MWPC
  // PID & NaI
  MakeTracksPidNaI();
  // PID
  CountNotUsedPid();
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksTrue(const map<Double_t,Int_t> *tracksMwpc, map<Double_t,pair<Int_t,Int_t> > &mapPhiMwpcNaI)
{
  // Find CBcluster & MWPCtrack & PIDpaddle associated events and
  // combine them into common tracks
  
  // All possible MWPC track & NaI combinations
  Double_t phi;
  Int_t iTrMwpc;
  for (map<Double_t,Int_t>::const_iterator iterTracksMwpc=tracksMwpc->begin(); iterTracksMwpc!=tracksMwpc->end(); ++iterTracksMwpc)
  {
    iTrMwpc = iterTracksMwpc->second;
    if (fMwpc->IsUsedWIE(iTrMwpc)) continue; // check if the strips or wires were already used
    //
    for (Int_t iClNaI=0; iClNaI<fNclNaI; ++iClNaI)
    {
      if (IsUsedClNaI(iClNaI)) continue; // check if the clNaI was used already
      phi = fTracksMwpc[iTrMwpc].Angle(fPositionsNaI[iClNaI]);
      if (phi > fMaxPhiMwpcNaI) continue;
      mapPhiMwpcNaI[phi] = make_pair(iTrMwpc,iClNaI);
    }
  }
  
  // The best MWPC track & NaI & PID and MWPC track & NaI pairs
  Int_t iClNaI, iPid;
  const TVector3 *r[2];
  TA2TrackLine track;
  for (map<Double_t,pair<Int_t,Int_t> >::iterator iterPhi = mapPhiMwpcNaI.begin(); iterPhi != mapPhiMwpcNaI.end(); ++iterPhi)
  {
    if (TooManyTracks()) break;
    //
    iTrMwpc = iterPhi->second.first;
    if (fMwpc->IsUsedWIE(iTrMwpc)) continue; // check if the strips or wires were already used
    iClNaI = iterPhi->second.second;
    if (IsUsedClNaI(iClNaI)) continue; // check if the clNaI was used already
    //
    r[0] = &(fTracksMwpc[iTrMwpc].GetOrigin());
    r[1] = fPositionsNaI + iClNaI;
    if (track.BuildTrack(*r[0],*r[1],-1.,fLimitsPsVertex))
    {
      // Test the track & PID combinations and find the best one
      fPhiTrackPid[fNtracks] = fMaxPhiTrackPid;
      iPid = kNullHit;
      for (Int_t i=0; i<fNhitsPid; ++i)
      {
	phi = track.Angle(fPositionsPid[i]);
	if (TMath::Abs(phi) > TMath::Abs(fPhiTrackPid[fNtracks])) continue;
	fPhiTrackPid[fNtracks] = phi;
	iPid = i;
      }
      fPhiTrackPid[fNtracks] *= (iPid == kNullHit) ? kNullFloat/fMaxPhiTrackPid : kRadToDeg;
      fPhiMwpcTrackNaI[fNtracks] = iterPhi->first*kRadToDeg;
      // Add a new track
      AddTrack(iPid,
	       fTracksMwpc[iTrMwpc].GetIinter(0),
	       fTracksMwpc[iTrMwpc].GetIinter(1),
	       iClNaI,
	       track.GetOrigin(),
	       track.GetDirCos(),
	       track.GetPsVertex());
    }
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksPidMwpc(const map<Double_t,Int_t> *tracksMwpc)
{
  // MWPC track & PID
  
  Int_t iTrMwpc, iPid;
  Double_t phi;
  for (map<Double_t,Int_t>::const_iterator iterTrackMwpc=tracksMwpc->begin(); iterTrackMwpc!=tracksMwpc->end(); ++iterTrackMwpc)
  {
    if (TooManyTracks()) break;
    //
    iTrMwpc = iterTrackMwpc->second;
    // Check if the strips or wires were already used
    if (fMwpc->IsUsedWIE(iTrMwpc)) continue;
    // Test the MWPC track & PID combinations and find the best one
    fPhiMwpcTrackPid[fNtracks] = fMaxPhiTrackPid;
    iPid = kNullHit;
    for (Int_t i=0; i<fNhitsPid; ++i)
    {
      phi = fTracksMwpc[iTrMwpc].Angle(fPositionsPid[i]);
      if (TMath::Abs(phi) > TMath::Abs(fPhiMwpcTrackPid[fNtracks])) continue;
      fPhiMwpcTrackPid[fNtracks] = phi;
      iPid = i;
    }
    if (iPid == kNullHit)
    {
      fPhiMwpcTrackPid[fNtracks] = kNullFloat;
      continue;
    }
    else
    {
      fPhiMwpcTrackPid[fNtracks] *= kRadToDeg;
      // Add a new track
      AddTrack(iPid,
	       fTracksMwpc[iTrMwpc].GetIinter(0),
	       fTracksMwpc[iTrMwpc].GetIinter(1),
	       kNullHit,
	       fTracksMwpc[iTrMwpc].GetOrigin(),
	       fTracksMwpc[iTrMwpc].GetDirCos(),
	       fTracksMwpc[iTrMwpc].GetPsVertex());
    }
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksMwpc(const map<Double_t,Int_t> *tracksMwpc)
{
  // MWPC alone
  
  Int_t iTrMwpc;
  for (map<Double_t,Int_t>::const_iterator iterTracksMwpc=tracksMwpc->begin(); iterTracksMwpc!=tracksMwpc->end(); ++iterTracksMwpc)
  {
    if (TooManyTracks()) break;
    //
    iTrMwpc = iterTracksMwpc->second;
    if ( fMwpc->IsUsedWIE(iTrMwpc)) continue; // check if the strips or wires were already used
    // Add a new track
    AddTrack(kNullHit,
	     fTracksMwpc[iTrMwpc].GetIinter(0),
	     fTracksMwpc[iTrMwpc].GetIinter(1),
	     kNullHit,
	     fTracksMwpc[iTrMwpc].GetOrigin(),
	     fTracksMwpc[iTrMwpc].GetDirCos(),
	     fTracksMwpc[iTrMwpc].GetPsVertex());
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksTrue(const Int_t **iInters, const Int_t *nInters, map<Double_t,Int_t> &mapMwpcInterNaI)
{
  // Find all possible NaI & MWPC1(2) intersection & (PID) combinations
  // and add the best ones to the fTracks array
  
  // Possible tracks
  const TA2MwpcIntersection *inter;
  const TVector3 *r[2];
  Double_t phi;
  TA2CentralTrack *track = fTracksSingleMwpc + fNtracksSingleMwpc;
  for (Int_t iCh=0; iCh<fNchambers; ++iCh)
  {
    for (Int_t iInter=0; iInter<nInters[iCh]; ++iInter)
    {
      if (TooManyTracksSingleMwpc()) break;
      //
      inter = fMwpc->GetInters(iCh) + iInters[iCh][iInter];
      if (fMwpc->IsUsedWIE(iCh,inter->GetIclI(),inter->GetIclW(),inter->GetIclE())) continue; // check if the strips or wires were already used
      r[0] = inter->GetPosition();
      for (Int_t iNaI=0; iNaI<fNclNaI; ++iNaI)
      {
	if (TooManyTracksSingleMwpc()) break;
	//
	if (IsUsedClNaI(iNaI)) continue; // Check if the clNaI was used already
	//
	r[1] = fPositionsNaI + iNaI;
	phi = r[0]->Angle(*r[1]);
	if (phi > fMaxPhiMwpcInterNaI) continue;
	if (track->BuildTrack(*r[0],*r[1],-1.,fLimitsPsVertex))
	{
	  track->SetIintersMwpc(kNullHit,kNullHit); // Reset IintersMwpc
	  track->SetIinterMwpc(iCh,iInters[iCh][iInter]);
	  track->SetIclNaI(iNaI);
	  mapMwpcInterNaI[phi] = fNtracksSingleMwpc;
	  fPhiInterSingleMwpcNaI[fNtracksSingleMwpc] = phi*kRadToDeg;
	  // Find the best PID association with the track
	  fPhiTrackSingleMwpcPid[fNtracksSingleMwpc] = fMaxPhiTrackPid;
	  track->SetIhitPid(kNullHit); // Reset IhitPid
	  for (Int_t iPid=0; iPid<fNhitsPid; ++iPid)
	  {
	    phi = track->Angle(fPositionsPid[iPid]);
	    if (TMath::Abs(phi) > TMath::Abs(fPhiTrackSingleMwpcPid[fNtracksSingleMwpc])) continue;
	    fPhiTrackSingleMwpcPid[fNtracksSingleMwpc] = phi;
	    track->SetIhitPid(iPid);
	  }
	  fPhiTrackSingleMwpcPid[fNtracksSingleMwpc] *= (track->GetIhitPid() == kNullHit) ? kNullFloat/fMaxPhiTrackPid : kRadToDeg;
	  ++fNtracksSingleMwpc;
	  ++track;
	}
      }
    }
  }
  
  // Best tracks
  Int_t iTrack, iInterMwpc, iCh, iNaI;
  for (map<Double_t,Int_t>::iterator iterTrack = mapMwpcInterNaI.begin(); iterTrack != mapMwpcInterNaI.end(); ++iterTrack)
  {
    if (TooManyTracks()) break;
    //
    iTrack = iterTrack->second;
    iCh = fTracksSingleMwpc[iTrack].HasSingleMwpc()-1;
    iInterMwpc = fTracksSingleMwpc[iTrack].GetIinterMwpc(iCh);
    if (fMwpc->IsUsedWIE(iCh,iInterMwpc)) continue; // Check if the strips or wires were already used
    iNaI = fTracksSingleMwpc[iTrack].GetIclNaI();
    if (IsUsedClNaI(iNaI)) continue; // Check if the clNaI was used already
    // Add track
    fPhiMwpcInterNaI[fNtracks] = fPhiInterSingleMwpcNaI[iTrack];
    fPhiTrackPid[fNtracks] = fPhiTrackSingleMwpcPid[iTrack];
    AddTrack(fTracksSingleMwpc[iTrack]);
    ++fNtracksSingleMwpcBest;
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksPidMwpc(const Int_t **iInters, const Int_t *nInters)
{
  // Single Mwpc & Pid
  
  const TA2MwpcIntersection *inter;
  Double_t phi;
  Int_t iPid;
  TA2CentralTrack *track = fTracksSingleMwpc + fNtracksSingleMwpc;
  for (Int_t iCh=0; iCh<fNchambers; ++iCh)
  {
    for (Int_t iInter=0; iInter<nInters[iCh]; ++iInter)
    {
      if (TooManyTracksSingleMwpc()) break;
      if (TooManyTracks()) break;
      //
      inter = fMwpc->GetInters(iCh) + iInters[iCh][iInter];
      if (fMwpc->IsUsedWIE(iCh,iInters[iCh][iInter])) continue; // check if the strips or wires were already used
      // Find iPid correlated with the intersection
      fPhiMwpcInterPid[fNtracks] = fMaxPhiMwpcInterPid;
      iPid = kNullHit;
      for (Int_t i=0; i<fNhitsPid; ++i)
      {
	phi = TVector2::Phi_mpi_pi( inter->GetPhi() - fPositionsPid[i].Phi() );
	if (TMath::Abs(phi) > TMath::Abs(fPhiMwpcInterPid[fNtracks])) continue;
	fPhiMwpcInterPid[fNtracks] = phi;
	iPid = i;
      }
      if (iPid == kNullHit)
      {
	// There is no Pid hit correlated with the intersection
	fPhiMwpcInterPid[fNtracks] = kNullFloat;
	continue;
      }
      else
      {
	// There is a Pid hit correlated with the intersection, biuld a new track
	track->BuildTrack(TVector3(0,0,0),*(inter->GetPosition()));
	track->SetIhitPid(iPid);
	track->SetIintersMwpc(kNullHit,kNullHit); // Reset IintersMwpc
	track->SetIinterMwpc(iCh,iInters[iCh][iInter]);
	track->SetIclNaI(kNullHit);
	fPhiMwpcInterPid[fNtracks] *= kRadToDeg;
	// Add a new track
	AddTrack(*track);
	++track;
	++fNtracksSingleMwpc;
	++fNtracksSingleMwpcBest;
      }
    }
  }
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeTracksPidNaI()
{
  // NaI & Pid or NaI alone (pure neutral claster)
  
  Double_t phi;
  Int_t iHitPid;
  for (Int_t iNaI=0; iNaI<fNclNaI; ++iNaI)
  {
    if (TooManyTracks()) break;
    if (IsUsedClNaI(iNaI)) continue; //Check if the clNaI was used already
    // Find iHitPid correlated with the NaI cluster
    fPhiPidNaI[fNtracks] = fMaxPhiPidNaI;
    iHitPid = kNullHit;
    for (Int_t iPid=0; iPid<fNhitsPid; ++iPid)
    {
      phi = TVector2::Phi_mpi_pi( fPositionsPid[iPid].Phi() - fPositionsNaI[iNaI].Phi() );
      if (TMath::Abs(phi) > TMath::Abs(fPhiPidNaI[fNtracks])) continue;
      fPhiPidNaI[fNtracks] = phi;
      iHitPid = iPid;
    }
    fPhiPidNaI[fNtracks] *= (iHitPid == kNullHit) ? kNullFloat/fMaxPhiPidNaI : kRadToDeg;
    AddTrack(iHitPid,
	     kNullHit,
	     kNullHit,
	     iNaI,
	     TVector3(0,0,0),
	     fPositionsNaI[iNaI],
	     TVector3(0,0,0));
  }
  
}

//_______________________________________________________________________________
void TA2CentralApparatus::CountNotUsedPid()
{
  // Count not used PID
  
  for (Int_t i=0; i<fNhitsPid; ++i)
  {
    if (IsUsedHitPid(i)) continue;
    fIhitsPidNotUsed[i]++;
    ++fNhitsPidNotUsed;
  }
  
}

//_______________________________________________________________________________
void TA2CentralApparatus::CountNotUsedIntersMwpc(const Int_t **iInters, const Int_t *nInters, Int_t *nIntersMwpcNotUsed)
{
  // Count not used Mwpc intersections
  
  const TA2MwpcIntersection *inter;
  for (Int_t iCh=0; iCh<fNchambers; ++iCh)
  {
    for (Int_t iInter=0; iInter<nInters[iCh]; ++iInter)
    {
      inter = fMwpc->GetInters(iCh) + iInters[iCh][iInter];
      if (fMwpc->IsUsedWIE(iCh,inter->GetIclI(),inter->GetIclW(),inter->GetIclE())) continue; // check if the strips or wires were already used
      nIntersMwpcNotUsed[iCh]++;
    }
  }
  
}

//_______________________________________________________________________________
void TA2CentralApparatus::MakeVertexes()
{
  // Reconstruct all possible vertexes (based on 2 tracks)
  
  for (Int_t ii, i=0; i<fNchTracks-1; ++i)
  {
    ii = fIchTracks[i];
    if ( !fTracks[ii].HasMwpc() || !fTracks[ii].HasNaI() ) continue;
    for (Int_t jj, j=i+1; j<fNchTracks; ++j)
    {
      jj = fIchTracks[j];
      if ( !fTracks[jj].HasMwpc() || !fTracks[jj].HasNaI() ) continue;
      //
      fVertexes[fNvertexes] = fTracks[ii].Vertex(fTracks[jj],fTrackDist[fNvertexes]);
      //
      fVert[0][fNvertexes] = fVertexes[fNvertexes].X();
      fVert[1][fNvertexes] = fVertexes[fNvertexes].Y();
      fVert[2][fNvertexes] = fVertexes[fNvertexes].Z();
      fVertR[fNvertexes]   = fVertexes[fNvertexes].Perp();
      //
      fIvertexes[ii][jj] = fIvertexes[jj][ii] = fNvertexes++;
    }
  }
  
}

//_______________________________________________________________________________
void TA2CentralApparatus::AddTrack(const Int_t iHitPid, const Int_t iInterMwpc0, const Int_t iInterMwpc1, const Int_t iClNaI, const TVector3 &orig, const TVector3 &dircos, const TVector3 &psvertex)
{
  // Add a new track to fTracks array
  // NOTE: No check for value of fNtracks is done
  
  // Set track
  TA2CentralTrack &track = fTracks[fNtracks];
  track.Reset();
  track.SetTrack(iHitPid,iInterMwpc0,iInterMwpc1,iClNaI,orig,dircos,psvertex);
  // Set Pid info
  if (track.HasPid())
  {
    track.SetEhitPidCorr(CalcEhitPid(iHitPid,orig,dircos));
    track.SetEhitPid(fPid->GetEnergy(fHitsPid[iHitPid]));
    if (fPid->IsTime()) track.SetThitPid(fPid->GetTime(fHitsPid[iHitPid]));
    track.SetPhiPid(TVector2::Phi_0_2pi(fPositionsPid[iHitPid].Phi()));
  }
  // Set MWPC info
  if (track.HasMwpc())
  {
    track.SetEtrackMwpc0(CalcEtrackMwpcInd(iInterMwpc0,0));	//single MWPC energy (0)
    track.SetEtrackMwpc1(CalcEtrackMwpcInd(iInterMwpc1,1));	//single MWPC energy (1)    
    track.SetEtrackMwpc(CalcEtrackMwpc(iInterMwpc0,iInterMwpc1)); // (e1+e2)/2 or e1(2) in case of a single mwpc track
    
    //   track.SetTtrackMwpc(); // TODO
  }
  
  // Set NaI info
  if (track.HasNaI())
  {
    track.SetEclNaI(fClNaI[fIdClNaI[iClNaI]]->GetEnergy());
    track.SetEclNaICorr(CalcEclNaI(iClNaI));
    track.SetTclNaI(fClNaI[fIdClNaI[iClNaI]]->GetTime());
    track.SetMclNaI(fClNaI[fIdClNaI[iClNaI]]->GetNhits());
    track.SetCentralIndexNaI(fClNaI[fIdClNaI[iClNaI]]->GetIndex());
    track.SetThetaNaI(fPositionsNaI[iClNaI].Theta());
    track.SetPhiNaI(TVector2::Phi_0_2pi(fPositionsNaI[iClNaI].Phi()));
    track.SetSigmaTimeNaI(fNaI->GetSigmaTime());
    track.SetSigmaEnergyNaI(fNaI->GetSigmaEnergy(track.GetEclNaI()));
    track.SetSigmaPhiNaI(fNaI->GetSigmaPhi(fPositionsNaI[iClNaI].Theta()));
    track.SetSigmaThetaNaI(fNaI->GetSigmaTheta());
  }
  
  // Mark the PID hit as "used"
  if (iHitPid != kNullHit) SetUsedHitPid(iHitPid);
  // Mark the MWPC strips and wires clusters as "used"
  if (iInterMwpc0 != kNullHit) fMwpc->SetUsedWIE(0,iInterMwpc0);
  if (iInterMwpc1 != kNullHit) fMwpc->SetUsedWIE(1,iInterMwpc1);
  // Mark the NaI cluster as "used"
  if (iClNaI != kNullHit) SetUsedClNaI(iClNaI);
  
  // Neutral or charged
  if (track.IsNeutral())
    fIneTracks[fNneTracks++] = fNtracks;
  else
    fIchTracks[fNchTracks++] = fNtracks;
  
  // For histograms
  fTrackType[fNtracks] = track.GetType();
  fTrackTheta[fNtracks] = track.GetTheta()*kRadToDeg;
  fTrackPhi[fNtracks] = TVector2::Phi_0_2pi(track.GetPhi())*kRadToDeg;
  fEclNaI[fNtracks] = track.GetEclNaI();
  fEhitPid[fNtracks] = track.GetEhitPid();
  fEtrackMwpc[fNtracks] = track.GetEtrackMwpc();
  if ( track.HasMwpc() && track.HasNaI() )
  {
    // To plot PsVertex only for NaI & Mwpc tracks
    fPsVertex[0][fNtracks] = track.GetPsVertex().X();
    fPsVertex[1][fNtracks] = track.GetPsVertex().Y();
    fPsVertex[2][fNtracks] = track.GetPsVertex().Z();
    fPsVertexR[fNtracks] = track.GetPsVertex().Perp();
  }
  
  // Add a new TA2Particle to the fParticleInfo array
  AddParticleInfo(track);
  
  // Increment the tracks counter
  ++fNtracks;
}

//_______________________________________________________________________________
void TA2CentralApparatus::AddTrack(const TA2CentralTrack &track)
{
  // Add the track to fTracks
  AddTrack(track.GetIhitPid(),
	   track.GetIinterMwpc(0),
	   track.GetIinterMwpc(1),
	   track.GetIclNaI(),
	   track.GetOrigin(),
	   track.GetDirCos(),
	   track.GetPsVertex());
}

//_______________________________________________________________________________
void TA2CentralApparatus::AddParticleInfo(const TA2CentralTrack &track)
{
  // Add a new particle to the fParticleInfo array
  
  fParticleInfo[fNparticle].Reset();
  fType[fNparticle] = kRootino;
  Double_t mass = fParticleID->GetMassMeV(kRootino);
  Int_t m, iPid;

  if ( track.HasPid()) iPid = fHitsPid[track.GetIhitPid()]; 
  else iPid = ENullHit;
  
  //Check dE/E cuts
  if ( fPCut && track.HasPid() && track.HasNaI() )
  {
	m = fPCutStart[iPid];					// Start cut index for pid[iPid]
	for (Int_t i=0; i<fNSectorCut[iPid]; ++i, ++m)		// Loop over specified cuts
	{
	  if ( static_cast<TA2Cut2DM<Double_t>* >(fPCut[m])->TestXY(track.GetEclNaI(),track.GetEhitPid()) )	// Condition met?
	  {
	fType[fNparticle] = GetCutPDGIndex(m);			//Set particle ID code...
	mass = fParticleID->GetMassMeV(fType[fNparticle]);	//...and particle mass
	break;	// cut OK so exit loop
	  }
	}
  }
  else if ( track.IsNeutral() )
  {
	  fType[fNparticle] = kGamma;
  }
  
  //
  fP4[fNparticle] = TA2Math::LorentzVector(track.GetDirCos().Unit(), track.GetEclNaI(), mass);
  fTime[fNparticle] = track.GetTclNaI();
  fSize[fNparticle] = track.GetMclNaI();
  fCentral[fNparticle] = track.GetCentralIndexNaI();
  fDet[fNparticle] = EDetUndef;
  if (track.HasNaI()) fDet[fNparticle] += EDetNaI;
  if (track.HasMwpc()) fDet[fNparticle] += EDetMWPC;
  if (track.HasPid()) fDet[fNparticle] += EDetPID;
  //
  fParticleInfo[fNparticle].SetP4(fP4[fNparticle]);
  fParticleInfo[fNparticle].SetTime(fTime[fNparticle]);
  fParticleInfo[fNparticle].SetClusterSize(fSize[fNparticle]);
  fParticleInfo[fNparticle].SetCentralIndex(fCentral[fNparticle]);
  fParticleInfo[fNparticle].SetApparatus(EAppCB);
  fParticleInfo[fNparticle].SetDetector(fDet[fNparticle]);
  fParticleInfo[fNparticle].SetSigmaTime(track.GetSigmaTimeNaI());
  fParticleInfo[fNparticle].SetSigmaE(track.GetSigmaEnergyNaI());
  fParticleInfo[fNparticle].SetSigmaPhi(track.GetSigmaPhiNaI());
  fParticleInfo[fNparticle].SetSigmaTheta(track.GetSigmaThetaNaI());
  fParticleInfo[fNparticle].SetParticleIDA(fType[fNparticle]);
  fParticleInfo[fNparticle].SetVetoTime(track.GetThitPid());
  fParticleInfo[fNparticle].SetVetoEnergy(track.GetEhitPidCorr());
  fParticleInfo[fNparticle].SetVetoIndex(iPid);
  fParticleInfo[fNparticle].SetDetectorA( fDet[fNparticle] - fParticleInfo[fNparticle].GetDetectors() );
//  fParticleInfo[fNparticle].SetIintersMwpc(track.GetIinterMwpc(0),track.GetIinterMwpc(1));
//  fParticleInfo[fNparticle].SetEtrackMwpc(track.GetEtrackMwpc());
//  fParticleInfo[fNparticle].SetTtrackMwpc(track.GetTtrackMwpc());
  fParticleInfo[fNparticle].SetTrackIntersects(track.GetIinterMwpc(0),track.GetIinterMwpc(1));
  fParticleInfo[fNparticle].SetTrackEnergy(track.GetEtrackMwpc());
  fParticleInfo[fNparticle].SetEnergyMwpc0(track.GetEtrackMwpc0());  
  fParticleInfo[fNparticle].SetEnergyMwpc1(track.GetEtrackMwpc1());
  fParticleInfo[fNparticle].SetPsVertex(track.GetPsVertex()); 
  fParticleInfo[fNparticle].SetTrackTime(track.GetTtrackMwpc());
  
  ++fNparticle;
  
}

//////////////////////////////////////////////////////////////////////
////////// TEST ////////////// TEST ///////////// TEST ///////////////
//////////////////////////////////////////////////////////////////////
//_______________________________________________________________________________
// void TA2CentralApparatus::Test()
void TA2CentralApparatus::Test(Bool_t WantDisplay)
{
  // Test output
  
  fDisplay = WantDisplay;
  Int_t fVerbose = 2;
  
  if (fDisplay)
  {
    fWait = kTRUE;
    InitGeometry();
  }
  
  // Print file name & event number
  cout << "____________________________________ TA2CentralApparatus _______________________________"<< endl;
  cout << "Event#: " << gAN->GetNEvent() << "\tAccepted#: " << gAN->GetNEvAnalysed() << "\tDAQ#: " << gAN->GetNDAQEvent() << endl;
  
  // NaI
  cout << "> NaI" << endl;
  cout << "Ncl: " << fNclNaI << endl;
  for (Int_t i=0; i<fNclNaI; ++i)
  {
    cout << i << "\tPhi: " << TVector2::Phi_0_2pi(fPositionsNaI[i].Phi())*kRadToDeg << "\tTheta: " << fPositionsNaI[i].Theta()*kRadToDeg << endl;
  }
  
  // Pid
  cout << "> Pid" << endl;
  cout << "Npid: " << fNhitsPid << "\tNpidNotUsed: " << fNhitsPidNotUsed << endl;
  cout << "NotUsedPidHits:\t";
  for (Int_t i=0; i<fNhitsPid; ++i)
  {
    cout << fIhitsPidNotUsed[i] << " ";
  }
  cout << endl;
  cout << "UsedPidHits:\t";
  for (Int_t i=0; i<fNhitsPid; ++i)
  {
    cout << fIhitsPidUsed[i] << " ";
  }
  cout << endl;
  for (Int_t i=0; i<fNhitsPid; ++i)
  {
    cout << i << "\tPhi: "<< TVector2::Phi_0_2pi(fPositionsPid[i].Phi())*kRadToDeg << endl;
  }
  
  // Posslible tracks with 2 MWPC
  // True
  cout << "> fMapMwpcTrueTrackNaI" << endl;
  for (map<Double_t,pair<Int_t,Int_t> >::const_iterator iterPhi = fMapMwpcTrueTrackNaI.begin(); iterPhi != fMapMwpcTrueTrackNaI.end(); ++iterPhi)
  {
    cout << "dPhi: " << iterPhi->first*kRadToDeg << "\tiTrMwpc: " << iterPhi->second.first << "\tiClNaI: " << iterPhi->second.second << endl;
  }
  
  // TrueCandidate
  cout << "> fMapMwpcTrueCandidateTrackNaI" << endl;
  for (map<Double_t,pair<Int_t,Int_t> >::const_iterator iterPhi = fMapMwpcTrueCandidateTrackNaI.begin(); iterPhi != fMapMwpcTrueCandidateTrackNaI.end(); ++iterPhi)
  {
    cout << "dPhi: " << iterPhi->first*kRadToDeg << "\tiTrMwpc: " << iterPhi->second.first << "\tiClNaI: " << iterPhi->second.second << endl;
  }
  
  // Candidate
  cout << "> fMapMwpcCandidateTrackNaI" << endl;
  for (map<Double_t,pair<Int_t,Int_t> >::const_iterator iterPhi = fMapMwpcCandidateTrackNaI.begin(); iterPhi != fMapMwpcCandidateTrackNaI.end(); ++iterPhi)
  {
    cout << "dPhi: " << iterPhi->first*kRadToDeg << "\tiTrMwpc: " << iterPhi->second.first << "\tiClNaI: " << iterPhi->second.second << endl;
  }
  
  // Tracks with 1 MWPC
  cout << "> Possible tracks with SingleMwpc" << endl;
  cout << "NtracksSingleMwpc: " << fNtracksSingleMwpc << endl;
  for (Int_t i=0; i<fNtracksSingleMwpc; ++i)
  {
    cout << i;
    cout << "\tiHitPid: " << fTracksSingleMwpc[i].GetIhitPid();
    cout << "\tiInterMwpc0: " << fTracksSingleMwpc[i].GetIinterMwpc(0) << "\tiInterMwpc1: " << fTracksSingleMwpc[i].GetIinterMwpc(1);
    cout << "\tiClNaI: " << fTracksSingleMwpc[i].GetIclNaI();
    cout << "\tType: " << fTracksSingleMwpc[i].GetType();
    cout << "\tPhi: " << TVector2::Phi_0_2pi(fTracksSingleMwpc[i].GetPhi())*kRadToDeg << "\tTheta: " << fTracksSingleMwpc[i].GetTheta()*kRadToDeg;
    cout << "\tPhiInterMwpcClNaI: " << fPhiInterSingleMwpcNaI[i];
    cout << "\tPhiTrSingleMwpcPid: " << fPhiTrackSingleMwpcPid[i] << endl;
  }
  cout << "NtracksTrueSingleMwpc: " << fMapMwpcTrueInterNaI.size() << endl;
  for (map<Double_t,Int_t>::const_iterator iter=fMapMwpcTrueInterNaI.begin(); iter!=fMapMwpcTrueInterNaI.end(); ++iter)
  {
    cout << "PhiInterSingleMwpcNaI: " << iter->first << "\tiTracksSingleMwpc: " << iter->second << endl;
  }
  cout << "NtracksCandidatesSingleMwpc: " << fMapMwpcCandidateInterNaI.size() << endl;
  for (map<Double_t,Int_t>::const_iterator iter=fMapMwpcCandidateInterNaI.begin(); iter!=fMapMwpcCandidateInterNaI.end(); ++iter)
  {
    cout << "PhiInterSingleMwpcNaI: " << iter->first << "\tiTracksSingleMwpc: " << iter->second << endl;
  }
  cout << "NtracksSingleMwpcBest: " << fNtracksSingleMwpcBest << endl;
  for (Int_t i=0; i<fNchambers; ++i)
  {
    cout << "iCh" << i << ":" << "\tNintersTrueMwpcNotUsed: " << fNintersTrueMwpcNotUsed[i] << "\tNintersCandidateMwpcNotUsed: " << fNintersCandidateMwpcNotUsed[i] << endl;
  }
  
  // Tracks
  cout << "===================== Tracks =======================" << endl;
  cout << "Ntracks: " << fNtracks << "\tNch: " << fNchTracks << "\tNne: " << fNneTracks << endl;
  for (Int_t i=0; i<fNtracks; ++i)
  {
    cout << "\tiHitPid: " << fTracks[i].GetIhitPid();
    cout << "\tiInterMwpc0: " << fTracks[i].GetIinterMwpc(0) << "\tiInterMwpc1: " << fTracks[i].GetIinterMwpc(1);
    cout << "\tiClNaI: " << fTracks[i].GetIclNaI();
    cout << "\tType: " << fTracks[i].GetType() << "\tPhi: " << TVector2::Phi_0_2pi(fTracks[i].GetPhi())*kRadToDeg << "\tTheta: " << fTracks[i].GetTheta()*kRadToDeg << "\tiCh: " << fTracks[i].HasSingleMwpc()-1 << endl;
  }
  
  // start display
  if (fDisplay) {
    
    if (fVerbose>0) {
      cout << endl;
      cout << "==============================================================" << endl;
      cout << "========================== MY TESTS ==========================" << endl;
      cout << "==============================================================" << endl;
      cout << "Event # " << gAN->GetNEvent() << endl;
      cout << "In this event " << fNtracks << " tracks have been found " << endl;
    }
    
    // CB
    Double_t xcb[50], ycb[50], zcb[50];
    if (fVerbose>0) {
      cout << "++++++++++++++++ CB ++++++++++++++++" << endl;
      cout << "--> " << fNclNaI << " CB sectors were hit" << endl;
    }
    for (Int_t iNaI=0; iNaI<fNclNaI; iNaI++) {
      xcb[iNaI] = fPositionsNaI[iNaI].X();
      ycb[iNaI] = fPositionsNaI[iNaI].Y();
      TMarker *mcb = new TMarker(xcb[iNaI], ycb[iNaI], 3);
      mcb->SetMarkerColor(kGreen-2);
      c->cd(); mcb->Draw("same");
      for (int iCbSect=0; iCbSect<32; iCbSect++) {
	Double_t tempx[5], tempy[5];
	for (int j=0; j<5; j++) {
	  tempx[j] = xCB[j][iCbSect]; tempy[j] = yCB[j][iCbSect];
	}
	if (TMath::IsInside(xcb[iNaI], ycb[iNaI], 5, tempx, tempy)) {
	  contourCB[iCbSect]->SetFillStyle(3001);
	  contourCB[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB[iCbSect]->SetFillStyle(1001);
	  }
	  c->cd(); contourCB[iCbSect]->Draw("same");
	}
      }
      
      // yz plane
      zcb[iNaI] = fPositionsNaI[iNaI].Z();
      if (fVerbose>1)
	cout << "CB cluster " << iNaI << "\t ==> (" << xcb[iNaI] << ", " << ycb[iNaI] << ", " << zcb[iNaI] << ")" << endl;
      TMarker *mcb_yz = new TMarker(zcb[iNaI], ycb[iNaI], 3);
      mcb_yz->SetMarkerColor(kGreen-2);
      c2->cd(); mcb_yz->Draw("same");
      for (int iCbSect=0; iCbSect<13; iCbSect++) {
	Double_t tempz[5], tempy[5], tempy2[5];
	for (int j=0; j<5; j++) {
	  tempz[j] = zCB_yz[j][iCbSect]; tempy[j] = yCB_yz[j][iCbSect]; tempy2[j] = y2CB_yz[j][iCbSect];
	}
	if (TMath::IsInside(zcb[iNaI], ycb[iNaI], 5, tempz, tempy)) {
	  contourCB_yz[iCbSect]->SetFillStyle(3001);
	  contourCB_yz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB_yz[iCbSect]->SetFillStyle(1001);
	  }
	}
	if (TMath::IsInside(zcb[iNaI], ycb[iNaI], 5, tempz, tempy2)) {
	  contourCB2_yz[iCbSect]->SetFillStyle(3001);
	  contourCB2_yz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB2_yz[iCbSect]->SetFillStyle(1001);
	  }
	}
      }
      
      // xz plane
      TMarker *mcb_xz = new TMarker(zcb[iNaI], xcb[iNaI], 3);
      mcb_xz->SetMarkerColor(kGreen-2);
      c3->cd(); mcb_xz->Draw("same");
      for (int iCbSect=0; iCbSect<13; iCbSect++) {
	Double_t tempz[5], tempx[5], tempx2[5];
	for (int j=0; j<5; j++) {
	  tempz[j] = zCB_yz[j][iCbSect]; tempx[j] = xCB_xz[j][iCbSect]; tempx2[j] = x2CB_xz[j][iCbSect];
	}
	if (TMath::IsInside(zcb[iNaI], xcb[iNaI], 5, tempz, tempx)) {
	  contourCB_xz[iCbSect]->SetFillStyle(3001);
	  contourCB_xz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB_xz[iCbSect]->SetFillStyle(1001);
	  }
	}
	if (TMath::IsInside(zcb[iNaI], xcb[iNaI], 5, tempz, tempx2)) {
	  contourCB2_xz[iCbSect]->SetFillStyle(3001);
	  contourCB2_xz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB2_xz[iCbSect]->SetFillStyle(1001);
	  }
	}
      }
    }
    c->Update(); c->Modified();
    c2->Update(); c2->Modified();
    c3->Update(); c3->Modified();
    
    
    
    // PID
    Double_t xpid[50], ypid[50];
    
    if (fVerbose>0) {
      cout << "++++++++++++++++ PID ++++++++++++++++" << endl;
      cout << "--> " << fNhitsPid << " PID sectors were hit " << endl;
    }
    
    for (Int_t iPid=0; iPid<fNhitsPid; ++iPid) {
      xpid[iPid] = fPositionsPid[iPid].X();
      ypid[iPid] = fPositionsPid[iPid].Y();
      if (fVerbose>1)
	cout << "PID cluster " << iPid << "\t ==> (" << xpid[iPid] << ", " << ypid[iPid] << ")" << endl;
      
      for (int iSect=0; iSect<25; iSect++) {
	Double_t tempx[5], tempy[5];
	for (int j=0; j<5; j++) {
	  tempx[j] = xPID[j][iSect]; tempy[j] = yPID[j][iSect];
	}
	if (TMath::IsInside(xpid[iPid], ypid[iPid], 5, tempx, tempy)) {
	  contourPID[iSect]->SetFillStyle(3001);
	  contourPID[iSect]->SetFillColor(kBlack);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iPid == fTracks[iTrk].GetIhitPid())
	      contourPID[iSect]->SetFillStyle(1001);
	  }
	  c->cd(); contourPID[iSect]->Draw("same");
	}
      }
    }
    c->Update();
    c->Modified();
    
    // MWPC
    // check on wire clusters
    TMarker *w[2][20];
    TMarker *inter[2][20], *inter_yz[2][20], *inter_xz[2][20];
    Double_t phiwire[2][20], xwire[2][20], ywire[2][20];
    Double_t xint[2][20], yint[2][20], zint[2][20];
    Double_t xtrk[2][20], ytrk[2][20], ztrk[2][20];
    Double_t xmwpc[2][20], ymwpc[2][20], zmwpc[2][20];
    Int_t iInter[2][20];
    Int_t iItrk[2][20];
    
    for (Int_t iCh=0; iCh<2; iCh++) {
      if (fVerbose>0) {
	cout << "++++++++++++++++ MWPC" << iCh << " ++++++++++++++++" << endl;
	cout << "--> " << fMwpc->GetWires(iCh)->GetNClust() << " clusters (open circles)" << endl;
      }
      for (Int_t iW = 0; iW < fMwpc->GetWires(iCh)->GetNClust(); ++iW) {
	phiwire[iCh][iW] = fMwpc->GetWires(iCh)->GetCGClust(iW); // rad
	xwire[iCh][iW] = fRmwpc[iCh] * TMath::Cos(phiwire[iCh][iW]);
	ywire[iCh][iW] = fRmwpc[iCh] * TMath::Sin(phiwire[iCh][iW]);
	if (fVerbose>1)
	  cout << "cluster " << iW << "\t phiW: " << phiwire[iCh][iW]*kRadToDeg << "\t ==> (" << xwire[iCh][iW] << ", " << ywire[iCh][iW] << ")" << endl;
	
	w[iCh][iW] = new TMarker( xwire[iCh][iW], ywire[iCh][iW], 24);
	if (iCh==0) w[iCh][iW]->SetMarkerColor(kAzure+7);
	else w[iCh][iW]->SetMarkerColor(kBlue);
	c->cd(); w[iCh][iW]->Draw("same");
      }
      
      // check on intersections on wires
      if (fVerbose>0)
	cout << "--> " << fMwpc->GetNinters(iCh) << " intersections (stars)" << endl;
      for (Int_t iInt = 0; iInt<fMwpc->GetNinters(iCh); iInt++) {
	Int_t nW = fMwpc->GetInters(iCh, iInt)->GetIclW();
	if (nW != -1)
	  w[iCh][nW]->Delete();
	xint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->X();
	yint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->Y();
	if (fVerbose>1)
	  cout << "intersection " << iInt << " using wire cluster " << nW << "\t ==> (" << xint[iCh][iInt] << ", " << yint[iCh][iInt] << ")" << endl;
	
	inter[iCh][iInt] = new TMarker(xint[iCh][iInt], yint[iCh][iInt], 29);
	inter[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter[iCh][iInt]->SetMarkerColor(kAzure+7);
	else inter[iCh][iInt]->SetMarkerColor(kBlue);
	c->cd(); inter[iCh][iInt]->Draw("same");
	
	// yz plane
	zint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->Z();
	inter_yz[iCh][iInt] = new TMarker(zint[iCh][iInt], yint[iCh][iInt], 29);
	inter_yz[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter_yz[iCh][iInt]->SetMarkerColor(kAzure+7);
	else inter_yz[iCh][iInt]->SetMarkerColor(kBlue);
	c2->cd(); inter_yz[iCh][iInt]->Draw("same");
	
	// xz plane
	inter_xz[iCh][iInt] = new TMarker(zint[iCh][iInt], xint[iCh][iInt], 29);
	inter_xz[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter_xz[iCh][iInt]->SetMarkerColor(kAzure+7);
	else inter_xz[iCh][iInt]->SetMarkerColor(kBlue);
	c3->cd(); inter_xz[iCh][iInt]->Draw("same");
      }
      c->Update(); c->Modified();
      c2->Update(); c2->Modified();
      c3->Update(); c3->Modified();
      
      // check for MWPC tracks
      if (fVerbose>0)
	cout << "--> " << fMwpc->GetNtracks() << " tracks using both MWPCs (full circles and thin line)" <<endl;
      for (Int_t iChTrk=0; iChTrk<fMwpc->GetNtracks(); iChTrk++) {
	iInter[iCh][iChTrk] = fMwpc->GetTrack(iChTrk)->GetIinter(iCh);
	inter[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
	xtrk[iCh][iChTrk] = xint[iCh][iInter[iCh][iChTrk]];
	ytrk[iCh][iChTrk] = yint[iCh][iInter[iCh][iChTrk]];
	// yz plane
	inter_yz[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter_yz[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
	ztrk[iCh][iChTrk] = zint[iCh][iInter[iCh][iChTrk]];
	// xz plane
	inter_xz[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter_xz[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
      }
      c->Update(); c->Modified();
      c2->Update(); c2->Modified();
      c3->Update(); c3->Modified();
      
      // "global" tracks
      for (Int_t iTrk=0; iTrk<fNtracks; ++iTrk) {
	iItrk[iCh][iTrk] = fTracks[iTrk].GetIinterMwpc(iCh);
	if (iItrk[iCh][iTrk] != -1) {
	  inter[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	  xmwpc[iCh][iTrk] = xint[iCh][iItrk[iCh][iTrk]];
	  ymwpc[iCh][iTrk] = yint[iCh][iItrk[iCh][iTrk]];
	  
	  // yz plane
	  inter_yz[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter_yz[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	  zmwpc[iCh][iTrk] = zint[iCh][iItrk[iCh][iTrk]];
	  
	  // xz plane
	  inter_xz[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter_xz[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	}
      }
      c->Update(); c->Modified();
      c2->Update(); c2->Modified();
      c2->Update(); c3->Modified();
    }
    
    
    TLine *trkl[20], *trkl_yz[20], *trkl_xz[20];
    for (Int_t iChTrk=0; iChTrk<fMwpc->GetNtracks(); iChTrk++) {
      if (fVerbose>1)
	cout << "Track " << iChTrk << " using intersection " << fMwpc->GetTrack(iChTrk)->GetIinter(0) << " on MWPC0 and intersection " << fMwpc->GetTrack(iChTrk)->GetIinter(1) << " on MWPC1" << endl;
      
      trkl[iChTrk] = new TLine(xtrk[0][iChTrk], ytrk[0][iChTrk], xtrk[1][iChTrk], ytrk[1][iChTrk]);
      c->cd(); trkl[iChTrk]->Draw("same");
      
      // yz plane
      trkl_yz[iChTrk] = new TLine(ztrk[0][iChTrk], ytrk[0][iChTrk], ztrk[1][iChTrk], ytrk[1][iChTrk]);
      c2->cd(); trkl_yz[iChTrk]->Draw("same");
      
      // xz plane
      trkl_xz[iChTrk] = new TLine(ztrk[0][iChTrk], xtrk[0][iChTrk], ztrk[1][iChTrk], xtrk[1][iChTrk]);
      c3->cd(); trkl_xz[iChTrk]->Draw("same");
    }
    c->Update(); c->Modified();
    c2->Update(); c2->Modified();
    c3->Update(); c3->Modified();
    
    if (fVerbose>0) {
      cout << endl;
      cout << "----------------- " << fNtracks << " tracks (full squares and red lines) -----------------" << endl;
    }
    
    for (Int_t iTrk=0; iTrk<fNtracks; ++iTrk) {
      if (iItrk[0][iTrk] != -1 && iItrk[1][iTrk] != -1) {
	TLine *l = new TLine(xmwpc[0][iTrk], ymwpc[0][iTrk], xmwpc[1][iTrk], ymwpc[1][iTrk]);
	c->cd(); l->SetLineWidth(2); l->SetLineColor(kRed); l->Draw("same");
	TLine *l_yz = new TLine(zmwpc[0][iTrk], ymwpc[0][iTrk], zmwpc[1][iTrk], ymwpc[1][iTrk]);
	c2->cd(); l_yz->SetLineWidth(2); l_yz->SetLineColor(kRed); l_yz->Draw("same");
	TLine *l_xz = new TLine(zmwpc[0][iTrk], xmwpc[0][iTrk], zmwpc[1][iTrk], xmwpc[1][iTrk]);
	c3->cd(); l_xz->SetLineWidth(2); l_xz->SetLineColor(kRed); l_xz->Draw("same");
      }
      if (fVerbose>1) {
	Int_t trackType = fTracks[iTrk].GetType();
	switch (trackType) {
	  case 3:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0" << endl;
	    break;
	  case 5:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 6:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 7:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 8:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 9:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 10:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 11:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 12:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 13:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 14:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 15:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	}
      }
    }
    
    c->Update(); c->Modified();
    c2->Update(); c2->Modified();
    c3->Update(); c3->Modified();
    h->Delete(); h2->Delete(); h3->Delete();
  }
  
  // start display
  if (fDisplay) {
    
    if (fVerbose>0) {
      cout << endl;
      cout << "==============================================================" << endl;
      cout << "========================== MY TESTS ==========================" << endl;
      cout << "==============================================================" << endl;
      cout << "Event # " << gAN->GetNEvent() << endl; 
      cout << "In this event " << fNtracks << " tracks have been found " << endl;
    }
    
    // CB
    Double_t xcb[50], ycb[50], zcb[50];
    if (fVerbose>0) {
      cout << "++++++++++++++++ CB ++++++++++++++++" << endl;
      cout << "--> " << fNclNaI << " CB sectors were hit" << endl;
    }
    for (Int_t iNaI=0; iNaI<fNclNaI; iNaI++) {
      xcb[iNaI] = fPositionsNaI[iNaI].X();
      ycb[iNaI] = fPositionsNaI[iNaI].Y();
      TMarker *mcb = new TMarker(xcb[iNaI], ycb[iNaI], 3);
      mcb->SetMarkerColor(kGreen-2);
      c->cd(); mcb->Draw("same");
      for (int iCbSect=0; iCbSect<32; iCbSect++) {
	Double_t tempx[5], tempy[5];
	for (int j=0; j<5; j++) {
	  tempx[j] = xCB[j][iCbSect]; tempy[j] = yCB[j][iCbSect];
	}
	if (TMath::IsInside(xcb[iNaI], ycb[iNaI], 5, tempx, tempy)) {
	  contourCB[iCbSect]->SetFillStyle(3001); 
	  contourCB[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB[iCbSect]->SetFillStyle(1001);
	  }
	  c->cd(); contourCB[iCbSect]->Draw("same");
	}
      }
      
      // yz plane
      zcb[iNaI] = fPositionsNaI[iNaI].Z();
      if (fVerbose>1)
	cout << "CB cluster " << iNaI << "\t ==> (" << xcb[iNaI] << ", " << ycb[iNaI] << ", " << zcb[iNaI] << ")" << endl;
      TMarker *mcb_yz = new TMarker(zcb[iNaI], ycb[iNaI], 3);
      mcb_yz->SetMarkerColor(kGreen-2);
      c2->cd(); mcb_yz->Draw("same");
      for (int iCbSect=0; iCbSect<13; iCbSect++) {
	Double_t tempz[5], tempy[5], tempy2[5];
	for (int j=0; j<5; j++) {
	  tempz[j] = zCB_yz[j][iCbSect]; tempy[j] = yCB_yz[j][iCbSect]; tempy2[j] = y2CB_yz[j][iCbSect];
	}
	if (TMath::IsInside(zcb[iNaI], ycb[iNaI], 5, tempz, tempy)) {
	  contourCB_yz[iCbSect]->SetFillStyle(3001);
	  contourCB_yz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB_yz[iCbSect]->SetFillStyle(1001);
	  }
	}
	if (TMath::IsInside(zcb[iNaI], ycb[iNaI], 5, tempz, tempy2)) {
	  contourCB2_yz[iCbSect]->SetFillStyle(3001);
	  contourCB2_yz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB2_yz[iCbSect]->SetFillStyle(1001);
	  }
	}
      }
      
      // xz plane
      TMarker *mcb_xz = new TMarker(zcb[iNaI], xcb[iNaI], 3);
      mcb_xz->SetMarkerColor(kGreen-2);
      c3->cd(); mcb_xz->Draw("same");
      for (int iCbSect=0; iCbSect<13; iCbSect++) {
	Double_t tempz[5], tempx[5], tempx2[5];
	for (int j=0; j<5; j++) {
	  tempz[j] = zCB_yz[j][iCbSect]; tempx[j] = xCB_xz[j][iCbSect]; tempx2[j] = x2CB_xz[j][iCbSect];
	}
	if (TMath::IsInside(zcb[iNaI], xcb[iNaI], 5, tempz, tempx)) {
	  contourCB_xz[iCbSect]->SetFillStyle(3001);
	  contourCB_xz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB_xz[iCbSect]->SetFillStyle(1001);
	  }
	}
	if (TMath::IsInside(zcb[iNaI], xcb[iNaI], 5, tempz, tempx2)) {
	  contourCB2_xz[iCbSect]->SetFillStyle(3001);
	  contourCB2_xz[iCbSect]->SetFillColor(kGreen);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iNaI == fTracks[iTrk].GetIclNaI())
	      contourCB2_xz[iCbSect]->SetFillStyle(1001);
	  }
	}
      }
    }
    c->Update();    c->Modified();
    c2->Update();   c2->Modified();
    c3->Update();   c3->Modified();
    
    
    
    // PID
    Double_t xpid[50], ypid[50];
    
    if (fVerbose>0) {
      cout << "++++++++++++++++ PID ++++++++++++++++" << endl;
      cout << "--> " << fNhitsPid << " PID sectors were hit " << endl;
    }
    
    for (Int_t iPid=0; iPid<fNhitsPid; ++iPid) {
      xpid[iPid] = fPositionsPid[iPid].X();
      ypid[iPid] = fPositionsPid[iPid].Y();
      if (fVerbose>1)
	cout << "PID cluster " << iPid << "\t ==> (" << xpid[iPid] << ", " << ypid[iPid] << ")" << endl;
      
      for (int iSect=0; iSect<25; iSect++) {
	Double_t tempx[5], tempy[5];
	for (int j=0; j<5; j++) {
	  tempx[j] = xPID[j][iSect]; tempy[j] = yPID[j][iSect]; 
	}
	if (TMath::IsInside(xpid[iPid], ypid[iPid], 5, tempx, tempy)) {
	  contourPID[iSect]->SetFillStyle(3001);
	  contourPID[iSect]->SetFillColor(kBlack);
	  for (Int_t iTrk=0; iTrk<fNtracks; iTrk++) {
	    if (iPid == fTracks[iTrk].GetIhitPid())
	      contourPID[iSect]->SetFillStyle(1001);
	  }
	  c->cd(); contourPID[iSect]->Draw("same");
	}
      }
    }
    c->Update();
    c->Modified();
    
    // MWPC
    // check on wire clusters
    TMarker *w[2][20];
    TMarker *inter[2][20], *inter_yz[2][20], *inter_xz[2][20];
    Double_t phiwire[2][20], xwire[2][20], ywire[2][20];
    Double_t xint[2][20], yint[2][20], zint[2][20];
    Double_t xtrk[2][20], ytrk[2][20], ztrk[2][20];
    Double_t xmwpc[2][20], ymwpc[2][20], zmwpc[2][20];
    Int_t iInter[2][20];
    Int_t iItrk[2][20];
    
    for (Int_t iCh=0; iCh<2; iCh++) {
      if (fVerbose>0) {
	cout << "++++++++++++++++ MWPC" << iCh << " ++++++++++++++++" << endl;
	cout << "--> " << fMwpc->GetWires(iCh)->GetNClust() << " clusters (open circles)" << endl;
      }
      for (Int_t iW = 0; iW < fMwpc->GetWires(iCh)->GetNClust(); ++iW) {
	phiwire[iCh][iW] = fMwpc->GetWires(iCh)->GetCGClust(iW); // rad
	xwire[iCh][iW] = fRmwpc[iCh] * TMath::Cos(phiwire[iCh][iW]);
	ywire[iCh][iW] = fRmwpc[iCh] * TMath::Sin(phiwire[iCh][iW]);
	if (fVerbose>1)
	  cout << "cluster " << iW << "\t phiW: " << phiwire[iCh][iW]*kRadToDeg << "\t ==> (" << xwire[iCh][iW] << ", " << ywire[iCh][iW] << ")" << endl;
	
	w[iCh][iW] = new TMarker( xwire[iCh][iW], ywire[iCh][iW], 24);
	if (iCh==0) w[iCh][iW]->SetMarkerColor(kAzure+7);
	else w[iCh][iW]->SetMarkerColor(kBlue);
	c->cd(); w[iCh][iW]->Draw("same");
      }
      
      // check on intersections on wires
      if (fVerbose>0)
	cout << "--> " << fMwpc->GetNinters(iCh) << " intersections (stars)" << endl;
      for (Int_t iInt = 0; iInt<fMwpc->GetNinters(iCh); iInt++) {
	Int_t nW = fMwpc->GetInters(iCh, iInt)->GetIclW();
	if (nW != -1)
	  w[iCh][nW]->Delete();
	xint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->X();
	yint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->Y();
	if (fVerbose>1)
	  cout << "intersection " << iInt << " using wire cluster " << nW <<  "\t ==> (" << xint[iCh][iInt] << ", " << yint[iCh][iInt] << ")" << endl;
	
	inter[iCh][iInt] = new TMarker(xint[iCh][iInt], yint[iCh][iInt], 29);
	inter[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter[iCh][iInt]->SetMarkerColor(kAzure+7);
	else 	inter[iCh][iInt]->SetMarkerColor(kBlue);
	c->cd(); inter[iCh][iInt]->Draw("same");
	
	// yz plane
	zint[iCh][iInt] = fMwpc->GetInters(iCh, iInt)->GetPosition()->Z();
	inter_yz[iCh][iInt] = new TMarker(zint[iCh][iInt], yint[iCh][iInt], 29);
	inter_yz[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter_yz[iCh][iInt]->SetMarkerColor(kAzure+7);
	else 	inter_yz[iCh][iInt]->SetMarkerColor(kBlue);
	c2->cd(); inter_yz[iCh][iInt]->Draw("same");
	
	// xz plane
	inter_xz[iCh][iInt] = new TMarker(zint[iCh][iInt], xint[iCh][iInt], 29);
	inter_xz[iCh][iInt]->SetMarkerSize(1.2);
	if (iCh==0) inter_xz[iCh][iInt]->SetMarkerColor(kAzure+7);
	else 	inter_xz[iCh][iInt]->SetMarkerColor(kBlue);
	c3->cd(); inter_xz[iCh][iInt]->Draw("same");
      }
      c->Update();    c->Modified();
      c2->Update();   c2->Modified();
      c3->Update();   c3->Modified();
      
      // check for MWPC tracks
      if (fVerbose>0) 
	cout << "--> " << fMwpc->GetNtracks() << " tracks using both MWPCs (full circles and thin line)" <<endl;
      for (Int_t iChTrk=0; iChTrk<fMwpc->GetNtracks(); iChTrk++) {
	iInter[iCh][iChTrk] = fMwpc->GetTrack(iChTrk)->GetIinter(iCh);
	inter[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
	xtrk[iCh][iChTrk] = xint[iCh][iInter[iCh][iChTrk]];
	ytrk[iCh][iChTrk] = yint[iCh][iInter[iCh][iChTrk]];
	// yz plane
	inter_yz[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter_yz[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
	ztrk[iCh][iChTrk] = zint[iCh][iInter[iCh][iChTrk]];
	// xz plane
	inter_xz[iCh][iInter[iCh][iChTrk]]->SetMarkerStyle(20);
	inter_xz[iCh][iInter[iCh][iChTrk]]->SetMarkerSize(1.0);
      }
      c->Update();       c->Modified();
      c2->Update();      c2->Modified();
      c3->Update();      c3->Modified();
      
      // "global" tracks
      for (Int_t iTrk=0; iTrk<fNtracks; ++iTrk) {    
	iItrk[iCh][iTrk] = fTracks[iTrk].GetIinterMwpc(iCh);
	if (iItrk[iCh][iTrk] != -1) {
	  inter[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	  xmwpc[iCh][iTrk] = xint[iCh][iItrk[iCh][iTrk]];
	  ymwpc[iCh][iTrk] = yint[iCh][iItrk[iCh][iTrk]]; 
	  
	  // yz plane
	  inter_yz[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter_yz[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	  zmwpc[iCh][iTrk] = zint[iCh][iItrk[iCh][iTrk]];
	  
	  // xz plane
	  inter_xz[iCh][iItrk[iCh][iTrk]]->SetMarkerStyle(21);
	  inter_xz[iCh][iItrk[iCh][iTrk]]->SetMarkerSize(1.0);
	}
      }
      c->Update();      c->Modified();
      c2->Update();     c2->Modified();
      c2->Update();     c3->Modified();
    }
    
    
    TLine *trkl[20], *trkl_yz[20], *trkl_xz[20];
    for (Int_t iChTrk=0; iChTrk<fMwpc->GetNtracks(); iChTrk++) {
      if (fVerbose>1)
	cout << "Track " << iChTrk << " using intersection " << fMwpc->GetTrack(iChTrk)->GetIinter(0) << " on MWPC0 and intersection " << fMwpc->GetTrack(iChTrk)->GetIinter(1) << " on MWPC1" << endl; 
      
      trkl[iChTrk] = new TLine(xtrk[0][iChTrk], ytrk[0][iChTrk], xtrk[1][iChTrk], ytrk[1][iChTrk]);
      c->cd(); trkl[iChTrk]->Draw("same");
      
      // yz plane
      trkl_yz[iChTrk] = new TLine(ztrk[0][iChTrk], ytrk[0][iChTrk], ztrk[1][iChTrk], ytrk[1][iChTrk]);
      c2->cd(); trkl_yz[iChTrk]->Draw("same");
      
      // xz plane
      trkl_xz[iChTrk] = new TLine(ztrk[0][iChTrk], xtrk[0][iChTrk], ztrk[1][iChTrk], xtrk[1][iChTrk]);
      c3->cd(); trkl_xz[iChTrk]->Draw("same");
    }
    c->Update();      c->Modified();
    c2->Update();     c2->Modified();
    c3->Update();     c3->Modified();
    
    if (fVerbose>0) {
      cout << endl;
      cout << "----------------- " << fNtracks << " tracks (full squares and red lines) -----------------" << endl;
    }
    
    for (Int_t iTrk=0; iTrk<fNtracks; ++iTrk) {    
      if (iItrk[0][iTrk] != -1 && iItrk[1][iTrk] != -1) {
	TLine *l = new TLine(xmwpc[0][iTrk], ymwpc[0][iTrk], xmwpc[1][iTrk], ymwpc[1][iTrk]);
	c->cd(); l->SetLineWidth(2); l->SetLineColor(kRed); l->Draw("same");
	TLine *l_yz = new TLine(zmwpc[0][iTrk], ymwpc[0][iTrk], zmwpc[1][iTrk], ymwpc[1][iTrk]);
	c2->cd(); l_yz->SetLineWidth(2); l_yz->SetLineColor(kRed); l_yz->Draw("same");
	TLine *l_xz = new TLine(zmwpc[0][iTrk], xmwpc[0][iTrk], zmwpc[1][iTrk], xmwpc[1][iTrk]);
	c3->cd(); l_xz->SetLineWidth(2); l_xz->SetLineColor(kRed); l_xz->Draw("same");
      }
      if (fVerbose>1) {
	Int_t trackType = fTracks[iTrk].GetType();
	switch (trackType) {
	  case 3:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0" << endl;
	    break;
	  case 5:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 6:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 7:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1" << endl;
	    break;
	  case 8:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 9: 
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 10:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 11:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 12:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 13:
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 14:
	    cout << "Track " << iTrk << " using intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	  case 15: 
	    cout << "Track " << iTrk << " using cluster " << fTracks[iTrk].GetIhitPid() << " on PID and intersection " << fTracks[iTrk].GetIinterMwpc(0) << " on MWPC0 and intersection " << fTracks[iTrk].GetIinterMwpc(1) << " on MWPC1 and cluster " << fTracks[iTrk].GetIclNaI() << " on CB" << endl;
	    break;
	}
      }
    }
    
    c->Update();     c->Modified();
    c2->Update();    c2->Modified();
    c3->Update();    c3->Modified();
//     h->Delete();     h2->Delete();    h3->Delete();
  }
  
  //   fWait = fNhitsPid ==2 && fNclNaI == 2 && fMwpc->GetNtracks() == 2;//fTracksTrueMwpc->size() > 0;
  //   fWait = fMwpc->GetNtracks() == 2;
  //      fWait = fNintersTrueMwpc[1] == 2 && fNtracks == 2;
  //   fWait = gAN->GetNDAQEvent() == 3000;
  //   fWait = fNtracksSingleMwpc > 0;
  
  // Wait?
  if (fWait)
  {
    cout << endl;
    cout << "Press any key to continue / q to quit " << endl;
    if (cin.get()=='q') gROOT->ProcessLine(".q");
  }
  
  // Reset fWait
  fWait = kFALSE;
  
  cout << "________________________________________________________________________________________"<< endl;
}

//________________________________________________________________________________________
void TA2CentralApparatus::InitGeometry()
{
  
  c = new TCanvas("display_xy","display_xy", 0, 0, 1000, 1000);
  h = new TH2F("xy","xy", 200, -300, 300, 200, -300, 300);
  c->cd();
  h->Draw();
  c->Update();
  c->Modified();

  // target
  TArc * target = new TArc(0., 0., targetDiameter);
  target->SetLineColor(kRed);
  target->SetFillColor(kRed);
  target->SetFillStyle(3013);
  target->Draw("same");
    
  TMarker *origin = new TMarker(0., 0., 5);
  origin->Draw("same");
    
  // CB  
  TArc *cbInn = new TArc(0.,0., innerCB);
  cbInn->SetLineColor(kGreen);
  cbInn->SetFillStyle(0);
  cbInn->Draw("same");
  //   TArc *cbOut = new TArc(0.,0., outerCB);
  //   cbOut->SetLineColor(kGreen);
  //   cbOut->SetFillStyle(0);
  //   cbOut->Draw("same");
    
  TVector2 dCB[32], uCB[32];
  for (int i=0; i<32; i++) {
    Double_t phi = i*dphiCB;
    Double_t xdCB = TMath::Sin(phi*kDegToRad)*innerCB;
    Double_t ydCB = TMath::Cos(phi*kDegToRad)*innerCB;
    if (TMath::Abs(xdCB) < 1e-10) xdCB = 0.;
    if (TMath::Abs(ydCB) < 1e-10) ydCB = 0.;
    dCB[i].Set(xdCB, ydCB);
    Double_t xuCB = TMath::Sin(phi*kDegToRad)*outerCB;
    Double_t yuCB = TMath::Cos(phi*kDegToRad)*outerCB;
    if (TMath::Abs(xuCB) < 1e-10) xuCB = 0.;
    if (TMath::Abs(yuCB) < 1e-10) yuCB = 0.;
    uCB[i].Set(xuCB, yuCB);
    if (i>0) {
      Double_t x[5] = {dCB[i-1].X(), uCB[i-1].X(), uCB[i].X(), dCB[i].X(), dCB[i-1].X()};
      Double_t y[5] = {dCB[i-1].Y(), uCB[i-1].Y(), uCB[i].Y(), dCB[i].Y(), dCB[i-1].Y()};
      xCB[0][i] = dCB[i-1].X(); xCB[1][i] = uCB[i-1].X(); xCB[2][i] = uCB[i].X(); xCB[3][i] = dCB[i].X(); xCB[4][i] = dCB[i-1].X();
      yCB[0][i] = dCB[i-1].Y(); yCB[1][i] = uCB[i-1].Y(); yCB[2][i] = uCB[i].Y(); yCB[3][i] = dCB[i].Y(); yCB[4][i] = dCB[i-1].Y();
      contourCB[i] = new TPolyLine(5, x, y);
      contourCB[i]->SetLineColor(kGreen);
      contourCB[i]->SetFillStyle(0);
      contourCB[i]->Draw("f");
      contourCB[i]->Draw("same");
    }
  }
    
  // mwpcs
  TArc *mwpc1 = new TArc(0., 0., fRmwpc[0]);
  mwpc1->SetLineColor(kAzure+7);
  mwpc1->SetFillStyle(0);
  mwpc1->Draw("same");
  TArc *mwpc2 = new TArc(0., 0., fRmwpc[1]);
  mwpc2->SetLineColor(kBlue);
  mwpc2->SetFillStyle(0);
  mwpc2->Draw("same");
    
  // PID
  TVector2 d[30], u[30];
  for (int i=0; i<25; i++) {
    Double_t phi = i*dphi;
    Double_t xd = TMath::Sin(phi*kDegToRad)*innerPID;
    Double_t yd = TMath::Cos(phi*kDegToRad)*innerPID;
    if (TMath::Abs(xd) < 1e-10) xd = 0.;
    if (TMath::Abs(yd) < 1e-10) yd = 0.;
    d[i].Set(xd, yd);
    Double_t xu = TMath::Sin(phi*kDegToRad)*outerPID;
    Double_t yu = TMath::Cos(phi*kDegToRad)*outerPID;
    if (TMath::Abs(xu) < 1e-10) xu = 0.;
    if (TMath::Abs(yu) < 1e-10) yu = 0.;
    u[i].Set(xu, yu);
    if (i>0) {
      Double_t x[5] = {d[i-1].X(), u[i-1].X(), u[i].X(), d[i].X(), d[i-1].X()};
      Double_t y[5] = {d[i-1].Y(), u[i-1].Y(), u[i].Y(), d[i].Y(), d[i-1].Y()};
      xPID[0][i] = d[i-1].X(); xPID[1][i] = u[i-1].X(); xPID[2][i] = u[i].X(); xPID[3][i] = d[i].X(); xPID[4][i] = d[i-1].X();
      yPID[0][i] = d[i-1].Y(); yPID[1][i] = u[i-1].Y(); yPID[2][i] = u[i].Y(); yPID[3][i] = d[i].Y(); yPID[4][i] = d[i-1].Y();
      contourPID[i] = new TPolyLine(5, x, y);
      contourPID[i]->Draw("f");
      contourPID[i]->Draw("same");
    }
  }
  c->Update();
  c->Modified();

  // y-z projection
  c2 = new TCanvas("c2","c2", 0, 0, 1000, 1000);
  h2 = new TH2F("yz","yz", 200, -650, 650, 200, -680, 680);
  c2->cd();
  h2->Draw();

  // CB
  //   TArc *cbOut_yz = new TArc(0., 0., outerCB_yz, thetaMinCB, thetaMaxCB);
  //   cbOut_yz->SetLineColor(kGreen);
  //   cbOut_yz->SetFillStyle(0);
  //   cbOut_yz->Draw("same");
  TArc * cbInn_yz = new TArc(0., 0., innerCB, thetaMinCB, thetaMaxCB);
  cbInn_yz->SetLineColor(kGreen);
  cbInn_yz->SetFillStyle(0);
  cbInn_yz->SetNoEdges();
  cbInn_yz->Draw("same");

  //   TArc *cbOut2_yz = new TArc(0., 0., outerCB_yz, -thetaMinCB, -thetaMaxCB);
  //   cbOut2_yz->SetLineColor(kGreen);
  //   cbOut2_yz->SetFillStyle(0);
  //   cbOut2_yz->Draw("same");
  TArc * cbInn2_yz = new TArc(0., 0., innerCB, -thetaMinCB, -thetaMaxCB);
  cbInn2_yz->SetLineColor(kGreen);
  cbInn2_yz->SetFillStyle(0);
  cbInn2_yz->SetNoEdges();
  cbInn2_yz->Draw("same");

  TVector2 dCB_yz[32], uCB_yz[32];
  for (int i=0; i<13; i++) {
    Double_t phi = i*dphiCB + thetaMinCB - 90;
    Double_t zdCB_yz = TMath::Sin(phi*kDegToRad)*innerCB;
    Double_t ydCB_yz = TMath::Cos(phi*kDegToRad)*innerCB;
    if (TMath::Abs(zdCB_yz) < 1e-10) zdCB_yz = 0.;
    if (TMath::Abs(ydCB_yz) < 1e-10) ydCB_yz = 0.;
    dCB_yz[i].Set(zdCB_yz, ydCB_yz);
    Double_t zuCB_yz = TMath::Sin(phi*kDegToRad)*outerCB_yz;
    Double_t yuCB_yz = TMath::Cos(phi*kDegToRad)*outerCB_yz;
    if (TMath::Abs(zuCB_yz) < 1e-10) zuCB_yz = 0.;
    if (TMath::Abs(yuCB_yz) < 1e-10) yuCB_yz = 0.;
    uCB_yz[i].Set(zuCB_yz, yuCB_yz);
    if (i>0) {
      Double_t z_yz[5] = {dCB_yz[i-1].X(), uCB_yz[i-1].X(), uCB_yz[i].X(), dCB_yz[i].X(), dCB_yz[i-1].X()};
      Double_t y_yz[5] = {dCB_yz[i-1].Y(), uCB_yz[i-1].Y(), uCB_yz[i].Y(), dCB_yz[i].Y(), dCB_yz[i-1].Y()};
      zCB_yz[0][i] = z_yz[0]; zCB_yz[1][i] = z_yz[1]; zCB_yz[2][i] = z_yz[2];
      zCB_yz[3][i] = z_yz[3]; zCB_yz[4][i] = z_yz[4];
      yCB_yz[0][i] = y_yz[0]; yCB_yz[1][i] = y_yz[1]; yCB_yz[2][i] = y_yz[2];
      yCB_yz[3][i] = y_yz[3]; yCB_yz[4][i] = y_yz[4];
      contourCB_yz[i] = new TPolyLine(5, z_yz, y_yz);
      contourCB_yz[i]->SetLineColor(kGreen);
      contourCB_yz[i]->SetFillStyle(0);
      contourCB_yz[i]->Draw("f");
      contourCB_yz[i]->Draw("same");

      Double_t y2_yz[5] = {-dCB_yz[i-1].Y(), -uCB_yz[i-1].Y(), -uCB_yz[i].Y(), -dCB_yz[i].Y(), -dCB_yz[i-1].Y()};
      y2CB_yz[0][i] = y2_yz[0]; y2CB_yz[1][i] = y2_yz[1]; y2CB_yz[2][i] = y2_yz[2];
      y2CB_yz[3][i] = y2_yz[3]; y2CB_yz[4][i] = y2_yz[4];
      contourCB2_yz[i] = new TPolyLine(5, z_yz, y2_yz);
      contourCB2_yz[i]->SetLineColor(kGreen);
      contourCB2_yz[i]->SetFillStyle(0);
      contourCB2_yz[i]->Draw("f");
      contourCB2_yz[i]->Draw("same");
    }
  }

  // MWPC
  Double_t xm[5] = {-MWPClength/2., -MWPClength/2., MWPClength/2., MWPClength/2., -MWPClength/2.};
  Double_t ym[5] = {-fRmwpc[1], fRmwpc[1], fRmwpc[1], -fRmwpc[1], -fRmwpc[1]};
  TPolyLine *mwpc_yz = new TPolyLine(5, xm, ym);
  mwpc_yz->SetFillStyle(0);
  mwpc_yz->SetLineColor(kBlue);
  mwpc_yz->Draw("same");
  TLine *mwpcup_yz = new TLine(-MWPClength/2., fRmwpc[0], MWPClength/2., fRmwpc[0]);
  mwpcup_yz->SetLineStyle(7);
  mwpcup_yz->SetLineColor(kAzure+7);
  mwpcup_yz->Draw("same");
  TLine *mwpcdown_yz = new TLine(-MWPClength/2., -fRmwpc[0], MWPClength/2., -fRmwpc[0]);
  mwpcdown_yz->SetLineStyle(7);
  mwpcdown_yz->SetLineColor(kAzure+7);
  mwpcdown_yz->Draw("same");
  
  // PID
  Double_t xp[5] = {-PIDlength/2., -PIDlength/2., PIDlength/2., PIDlength/2., -PIDlength/2.};
  Double_t yp[5] = {-outerPID, outerPID, outerPID, -outerPID, -outerPID};
  TPolyLine *pid_yz = new TPolyLine(5, xp, yp);
  pid_yz->SetFillStyle(0);
  pid_yz->Draw("same");
  TLine *pidup_yz = new TLine(-PIDlength/2., innerPID, PIDlength/2., innerPID);
  pidup_yz->SetLineStyle(7);
  pidup_yz->Draw("same");
  TLine *piddown_yz = new TLine(-PIDlength/2., -innerPID, PIDlength/2., -innerPID);
  piddown_yz->SetLineStyle(7);
  piddown_yz->Draw("same");

  // target
  Double_t xt[5] = {-targetLength/2., -targetLength/2., targetLength/2., targetLength/2., -targetLength/2.};
  Double_t yt[5] = {-targetDiameter/2., targetDiameter/2., targetDiameter/2., -targetDiameter/2., -targetDiameter/2.};
  TPolyLine *target_yz = new TPolyLine(5, xt, yt);
  target_yz->SetLineColor(kRed);
  target_yz->SetFillColor(kRed);
  target_yz->SetFillStyle(3013);
  target_yz->Draw("f");
  target_yz->Draw("same");
  
  origin->Draw("same");

  c2->Update();
  c2->Modified();


  // x-z projection
  c3 = new TCanvas("c3","c3", 0, 0, 1000, 1000);
  h3 = new TH2F("xz","xz", 200, -650, 650, 200, -680, 680);
  c3->cd();
  h3->Draw();

  // CB
  TArc * cbInn_xz = new TArc(0., 0., innerCB, thetaMinCB, thetaMaxCB);
  cbInn_xz->SetLineColor(kGreen);
  cbInn_xz->SetFillStyle(0);
  cbInn_xz->SetNoEdges();
  cbInn_xz->Draw("same");

  //   TArc *cbOut2_xz = new TArc(0., 0., outerCB_xz, -thetaMinCB, -thetaMaxCB);
  //   cbOut2_xz->SetLineColor(kGreen);
  //   cbOut2_xz->SetFillStyle(0);
  //   cbOut2_xz->Draw("same");
  TArc * cbInn2_xz = new TArc(0., 0., innerCB, -thetaMinCB, -thetaMaxCB);
  cbInn2_xz->SetLineColor(kGreen);
  cbInn2_xz->SetFillStyle(0);
  cbInn2_xz->SetNoEdges();
  cbInn2_xz->Draw("same");

  TVector2 dCB_xz[32], uCB_xz[32];
  for (int i=0; i<13; i++) {
    Double_t phi = i*dphiCB + thetaMinCB - 90;
    Double_t zdCB_xz = TMath::Sin(phi*kDegToRad)*innerCB;
    Double_t xdCB_xz = TMath::Cos(phi*kDegToRad)*innerCB;
    if (TMath::Abs(zdCB_xz) < 1e-10) zdCB_xz = 0.;
    if (TMath::Abs(xdCB_xz) < 1e-10) xdCB_xz = 0.;
    dCB_xz[i].Set(zdCB_xz, xdCB_xz);
    Double_t zuCB_xz = TMath::Sin(phi*kDegToRad)*outerCB_xz;
    Double_t xuCB_xz = TMath::Cos(phi*kDegToRad)*outerCB_xz;
    if (TMath::Abs(zuCB_xz) < 1e-10) zuCB_xz = 0.;
    if (TMath::Abs(xuCB_xz) < 1e-10) xuCB_xz = 0.;
    uCB_xz[i].Set(zuCB_xz, xuCB_xz);
    if (i>0) {
      Double_t z_xz[5] = {dCB_xz[i-1].X(), uCB_xz[i-1].X(), uCB_xz[i].X(), dCB_xz[i].X(), dCB_xz[i-1].X()};
      Double_t x_xz[5] = {dCB_xz[i-1].Y(), uCB_xz[i-1].Y(), uCB_xz[i].Y(), dCB_xz[i].Y(), dCB_xz[i-1].Y()};
      zCB_xz[0][i] = z_xz[0];       zCB_xz[1][i] = z_xz[1];       zCB_xz[2][i] = z_xz[2]; 
      zCB_xz[3][i] = z_xz[3];       zCB_xz[4][i] = z_xz[4]; 
      xCB_xz[0][i] = x_xz[0];       xCB_xz[1][i] = x_xz[1];       xCB_xz[2][i] = x_xz[2];      
      xCB_xz[3][i] = x_xz[3];       xCB_xz[4][i] = x_xz[4];
      contourCB_xz[i] = new TPolyLine(5, z_xz, x_xz);
      contourCB_xz[i]->SetLineColor(kGreen);
      contourCB_xz[i]->SetFillStyle(0);
      contourCB_xz[i]->Draw("f");
      contourCB_xz[i]->Draw("same");

      Double_t x2_xz[5] = {-dCB_xz[i-1].Y(), -uCB_xz[i-1].Y(), -uCB_xz[i].Y(), -dCB_xz[i].Y(), -dCB_xz[i-1].Y()};
      x2CB_xz[0][i] = x2_xz[0];       x2CB_xz[1][i] = x2_xz[1];       x2CB_xz[2][i] = x2_xz[2];
      x2CB_xz[3][i] = x2_xz[3];       x2CB_xz[4][i] = x2_xz[4];
      contourCB2_xz[i] = new TPolyLine(5, z_xz, x2_xz);
      contourCB2_xz[i]->SetLineColor(kGreen);
      contourCB2_xz[i]->SetFillStyle(0);
      contourCB2_xz[i]->Draw("f");
      contourCB2_xz[i]->Draw("same");
    }
  }

  // MWPC
  //   Double_t xm[5] = {-MWPClength/2., -MWPClength/2., MWPClength/2., MWPClength/2., -MWPClength/2.};
  //   Double_t ym[5] = {-fR[1], fR[1], fR[1], -fR[1], -fR[1]};
  TPolyLine *mwpc_xz = new TPolyLine(5, xm, ym);
  mwpc_xz->SetFillStyle(0);
  mwpc_xz->SetLineColor(kBlue);
  mwpc_xz->Draw("same");
  TLine *mwpcup_xz = new TLine(-MWPClength/2., fRmwpc[0], MWPClength/2., fRmwpc[0]);
  mwpcup_xz->SetLineStyle(7);
  mwpcup_xz->SetLineColor(kAzure+7);
  mwpcup_xz->Draw("same");
  TLine *mwpcdown_xz = new TLine(-MWPClength/2., -fRmwpc[0], MWPClength/2., -fRmwpc[0]);
  mwpcdown_xz->SetLineStyle(7);
  mwpcdown_xz->SetLineColor(kAzure+7);
  mwpcdown_xz->Draw("same");
  
  // PID
  //   Double_t xp[5] = {-PIDlength/2., -PIDlength/2., PIDlength/2., PIDlength/2., -PIDlength/2.};
  //   Double_t yp[5] = {-outerPID, outerPID, outerPID, -outerPID, -outerPID};
  TPolyLine *pid_xz = new TPolyLine(5, xp, yp);
  pid_xz->SetFillStyle(0);
  pid_xz->Draw("same");
  TLine *pidup_xz = new TLine(-PIDlength/2., innerPID, PIDlength/2., innerPID);
  pidup_xz->SetLineStyle(7);
  pidup_xz->Draw("same");
  TLine *piddown_xz = new TLine(-PIDlength/2., -innerPID, PIDlength/2., -innerPID);
  piddown_xz->SetLineStyle(7);
  piddown_xz->Draw("same");

  // target
  //   Double_t xt[5] = {-targetLength/2., -targetLength/2., targetLength/2., targetLength/2., -targetLength/2.};
  //   Double_t yt[5] = {-targetDiameter/2., targetDiameter/2., targetDiameter/2., -targetDiameter/2., -targetDiameter/2.};
  TPolyLine *target_xz = new TPolyLine(5, xt, yt);
  target_xz->SetLineColor(kRed);
  target_xz->SetFillColor(kRed);
  target_xz->SetFillStyle(3013);
  target_xz->Draw("f");
  target_xz->Draw("same");
  
  origin->Draw("same");

  c3->Update();
  c3->Modified();

}

ClassImp(TA2CentralApparatus)
