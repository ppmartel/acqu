// ROOT
// #include "TF1.h"
#include "TCanvas.h"
#include "TArc.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TMarker.h"
#include "TH2F.h"
// AcquRoot
#include "TA2CylMwpc.h"
#include "TA2UserAnalysis.h"
  
enum {
  ENWCLayer = 200, ENWCChamber, ENWCLayersInChamber, EWCChamberParm,
  EWCTypePlaneWire, EWCTypeCylWire, EWCTypePlaneDrift,
  EWCTypePlaneStrip, EWCTypeCylStrip, EWCIntersection,
  EWCSigmaZ, EWCSigmaPhi, EWCDetEff,
  EWCReadGeneratedKinematics, EWCCalib
};

// Command-line key words which determine what to read in
static const Map_t kWCKeys[] = {
  {"Number-Layers:",     ENWCLayer},
  {"Number-Chambers:",   ENWCChamber},
  {"Layers-In-Chamber:", ENWCLayersInChamber},
  {"Chamber-Parameter:", EWCChamberParm},
  {"Plane-Wire:",        EWCTypePlaneWire},
  {"Cyl-Wire:",          EWCTypeCylWire},
  {"Plane-Drift:",       EWCTypePlaneDrift},
  {"Plane-Strip:",       EWCTypePlaneStrip},
  {"Cyl-Strip:",         EWCTypeCylStrip},
  {"Intersection:",      EWCIntersection},
  {"SigmaZ:",		 EWCSigmaZ},
  {"SigmaPhi:",		 EWCSigmaPhi},
  {"DetEff:",		 EWCDetEff},
  {"ReadGeneratedKinematics:",	EWCReadGeneratedKinematics},
  {"Calib:",		 EWCCalib},
  {NULL,                 -1}
  };
  
  // needed for the display
  const Double_t dphi = 15; // deg
  const Double_t innerPID = 58.;  // PID
  const Double_t outerPID = 62.;  // PID
  const Double_t PIDlength = 500.; 
  const Double_t innerCB = 254.;
  const Double_t outerCB = 360.4; 
  const Double_t outerCB_yz = 660.4;
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
  
//_________________________________________________________________________________________
//  TA2CylMwpc
//
//  The TA2CylMwpc class describes 2 co-axial MWPC as a single detector,
//  reconstructs the strip(internal)-wire-strip(external) intersections and 
//  makes particle tracks (cosmic or produced in the target).
//  If only 1 MWPC is defined then only cosmic tracks can be reconstructed.

//_________________________________________________________________________________________
TA2CylMwpc::TA2CylMwpc( const char* name, TA2System* apparatus ) : TA2WireChamber(name, apparatus)
{
  // Initial setting of local variables mostly set undefined
  
  // Test
  fWait        = kFALSE;
  fMwpcDisplay = kFALSE;
  fIfCalib     = kFALSE;
  // Geometrical consts
  fR = fRtE = fRtI = fC1 = fC2 = NULL;
  
  // Default parametrs of the analysis
  fMaxDphiWIE = 0.1;	// rad
  fMaxDr = 100.;	// mm
  
  // Layers
  fI = fE = NULL;
  fW = NULL;
  fUsedClW = fUsedClI = fUsedClE = NULL;
  
  // Intersections
  fIclI    = NULL;
  fIclE    = NULL;
  fZie     = NULL;
  fPhiIE   = NULL;
  fNie     = NULL;
  fDphiWIE = NULL;
  //
  fNinters = fNintersTrue = fNintersCandidate = NULL;
  fInters = NULL;
  fIintersTrue = fIintersCandidate = NULL;
  // Geometrical corrections
  fPhiCorrEI[0] = NULL;
  fPhiCorrEI[1] = NULL;
  fPhiCorrEI[2] = NULL;
  fPhiCorr = NULL;
  fXcorr = fYcorr = fZcorr[0] = fZcorr[1] = fZcorr[2] = fZcorr[3] = fZcorr[4] = fZcorr[5] = fZcorr[6] = fZcorr[7] = NULL;
  // For output hist
  fTypeInter = NULL;
  fZInter = fPhiInter = fDphiWS = NULL;
  fCGclI = fCGclW = fCGclE = NULL;
  fAclIE = NULL;
  
  // Tracks
  fNTrack = fNtracksTrue = fNtracksTrueCandidate = fNtracksCandidate = 0;
  fNtracksTrueBest = fNtracksTrueCandidateBest = fNtracksCandidateBest = 0;
  fTracks  = NULL;
  // For output hists
  fTypeTrack = NULL;
  fMagTrack = NULL;
  fThetaTrack = fPhiTrack = NULL;
  fDPhiInters = fDZInters = NULL;
  fPsVertex[0] = fPsVertex[1] = fPsVertex[2] = fPsVertexR = NULL;
  fEtrack = NULL;
  
  // Vertexes
  fNVertex = 0;
  fVertex  = NULL;
  fItracks = NULL;
  // For output hists
  for (Int_t i=0; i<3; ++i) fVert[i] = NULL;
  fVertR = NULL;
  fTrackDist = NULL;
  
  // Simulation
  fReadGeneratedKinematics = kFALSE;
  fSigmaZ   = NULL;
  fSigmaPhi = NULL;
  fDetEff = NULL;
  // For output hists
  for (Int_t i=0; i<3; ++i) fVertSim[i] = EBufferEnd;
  fDrHitsSim = NULL;
  
  //
  AddCmdList(kWCKeys); // for SetConfig()
}

//_________________________________________________________________________________________
TA2CylMwpc::~TA2CylMwpc()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  
  DeleteArrays();
}

//_________________________________________________________________________________________
void TA2CylMwpc::DeleteArrays()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  
  delete [] fVertexLimits;
  
  // Geometrical consts
  delete [] fR;
  delete [] fRtE;
  delete [] fRtI;
  delete [] fC1;
  delete [] fC2;
//   delete [] fD1;
//   delete [] fD2;
  
  // Layers
  delete [] fI;
  delete [] fE;
  delete [] fW;
  for(Int_t i=0;i<fNchamber;++i)
  {
    delete [] fUsedClI[i];
    delete [] fUsedClW[i];
    delete [] fUsedClE[i];
  }
  delete [] fUsedClI;
  delete [] fUsedClW;
  delete [] fUsedClE;
  
  // Intersections
  for(Int_t iCh=0; iCh<fNChamber; iCh++)
  {
    delete [] fIclI[iCh];
    delete [] fIclE[iCh];
    delete [] fZie[iCh];
    delete [] fPhiIE[iCh];
    //
    delete [] fInters[iCh];
    delete [] fIintersTrue[iCh];
    delete [] fIintersCandidate[iCh];
    // for histograms
    delete [] fTypeInter[iCh];
    delete [] fZInter[iCh];
    delete [] fPhiInter[iCh];
    delete [] fDphiWS[iCh];
    delete [] fCGclI[iCh];
    delete [] fCGclW[iCh];
    delete [] fCGclE[iCh];
    delete [] fAclIE[iCh];
  }
  delete [] fIclI;
  delete [] fIclE;
  delete [] fZie;
  delete [] fPhiIE;
  delete [] fNie;
  delete [] fDphiWIE;
  //
  delete [] fInters;
  delete [] fNinters;
  delete [] fNintersTrue;
  delete [] fIintersTrue;
  delete [] fNintersCandidate;
  delete [] fIintersCandidate;
  //Geometrical corrections
  delete [] fPhiCorrEI[0];
  delete [] fPhiCorrEI[1];
  delete [] fPhiCorrEI[3];
  delete [] fPhiCorr;
  delete [] fXcorr;
  delete [] fYcorr;
  for (Int_t i=0; i<8; ++i) delete [] fZcorr[i];
  // for histograms
  delete [] fTypeInter;
  delete [] fZInter;
  delete [] fPhiInter;
  delete [] fDphiWS;
  delete [] fCGclI;
  delete [] fCGclW;
  delete [] fCGclE;
  delete [] fAclIE;
  
  // Tracks
  delete [] fTracks;
  // for histograming
  delete [] fTypeTrack;
  delete [] fMagTrack;
  delete [] fThetaTrack;
  delete [] fPhiTrack;
  delete [] fDPhiInters;
  delete [] fDZInters;
  delete [] fEtrack;
  for (Int_t i=0;i<3;++i) delete [] fPsVertex[i];
  delete [] fPsVertexR;
  
  // Vertexes
  delete [] fVertex;
  delete [] fItracks;
  for (Int_t i=0; i<3; ++i) delete [] fVert[i];
  delete [] fVertR;
  delete [] fTrackDist;
  
  // Simulations
  for(Int_t iCh=0; iCh<fNChamber; iCh++)
  {
    if (fSigmaZ[iCh]) delete fSigmaZ[iCh];
    if (fSigmaPhi[iCh]) delete fSigmaPhi[iCh];
    for (map<Int_t,TGraph2D* >::iterator iter = fDetEff[iCh].begin(); iter != fDetEff[iCh].end(); ++iter)
    {
      delete iter->second;
    }
  }
  delete [] fSigmaZ;
  delete [] fSigmaPhi;
  delete [] fDetEff;
  delete [] fDrHitsSim;
}

//_________________________________________________________________________________________
void TA2CylMwpc::SetConfig( char* line, int key )
{
  // Load config parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kWCKeys array at the top of the source .cc file
  
  Int_t n, id, nelem, maxcl, maxclsize;
  Int_t layer[8];
  //   Int_t DefStrip;
  //   Int_t DefLayer;
  Double_t dparm[16];
//   Int_t iparm[16];
  Char_t name[64];
  
  // Cluster specific configuration
  switch ( key )
  {
    case ENWCLayer:
	// no. of active layers (or planes) in chamber
	if( sscanf( line, "%d", &fNLayer ) < 1 ) goto error;
	fWCLayer = new TA2WCLayer*[fNLayer];
	break;
    case ENWCChamber:
	// no. of chambers in detector
	if ( sscanf( line, "%d", &fNChamber ) < 1 ) goto error;
	fChamberLayers = new Int_t*[fNChamber];
	fPhiCorrEI[0] = new Double_t[fNChamber];
	fPhiCorrEI[1] = new Double_t[fNChamber];
	fPhiCorrEI[2] = new Double_t[fNChamber];
	fPhiCorr  = new Double_t[fNChamber];
	fXcorr    = new Double_t[fNChamber];
	fYcorr    = new Double_t[fNChamber];
	for (Int_t i=0; i<8; ++i) fZcorr[i] = new Double_t[fNChamber];
	// Mem allocation for SigmaZ, SigmaPhi and DetEff
	fSigmaZ   = new TGraph2D*[fNChamber];
	fSigmaPhi = new TGraph2D*[fNChamber];
	fDetEff   = new map<Int_t,TGraph2D*>[fNChamber];
	// Set default
	for (Int_t i=0; i<fNChamber; ++i)
	{
	  fPhiCorrEI[0][i] = 0.;
	  fPhiCorrEI[1][i] = 0.;
	  fPhiCorrEI[2][i] = 0.;
	  fPhiCorr[i]  = 0.;
	  fXcorr[i]    = 0.;
	  fYcorr[i]    = 0.;
	  for (Int_t j=0; j<8; ++j) fZcorr[j][i] = 0.;
	  //
	  fSigmaZ[i]   = NULL;
	  fSigmaPhi[i] = NULL;
	}
	break;
    case ENWCLayersInChamber:
	// layers in particular chamber
	if ( fNchamber >= fNChamber ){
	  PrintError ( line, "<Too many WC layers-in-chamber input>");
	  return;
	}
	if ( ( n = sscanf( line, "%d%d%d%d%d%d%d%d",
	  layer,layer+1,layer+2,layer+3,
	  layer+4,layer+5,layer+6,layer+7 ) ) < 1 ) goto error;
	  fChamberLayers[fNchamber]  = new Int_t[n+1];
	  fChamberLayers[fNchamber][0]  = n;
	  for ( Int_t i=1; i<=n; i++ ) fChamberLayers[fNchamber][i] = layer[i-1];
	  fNchamber++;
	  break;
    case EWCChamberParm:
	// wild-card chamber setup....depends on specific SetChamberParm
	n = sscanf( line, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		    dparm,    dparm+1,  dparm+2,  dparm+3,
		    dparm+4,  dparm+5,  dparm+6,  dparm+7,
		    dparm+8,  dparm+9,  dparm+10, dparm+11,
		    dparm+12, dparm+13, dparm+14, dparm+15 );
		    SetChamberParm( n, dparm );
		    break;
    case EWCTypePlaneWire:
	//I'm abusing this entry for parsing different stuff for MWPC simulation. Introducing a new
	//command-line keyword does not work due to the stupid idea of somebody to define the map
	//and keys also in class TA2WireChamber, so that introducing new words leads to segfaults!
	TA2CylMwpc::ParseMisc(line);
	break;
    case EWCTypeCylWire:
	// Wires arranged in cylinder
	// Has hit TDC, but no pulse height
	// layer parameters.....
	// radius, length,
	// quadratic z correction  coeff 0,1,2
	if ( fNlayer >= fNLayer ){
	  PrintError( line, "<Too many WC layers input>");
	  return;
	}
// 	fIsEnergy = EFalse;
// 	fIsTime = fIsPos = ETrue;
	if ( sscanf( line, "%s%d%d%d%lf%lf%lf%lf%lf",
	            name, &nelem, &maxcl, &maxclsize, dparm, dparm+1,
		    dparm+2, dparm+3, dparm+4 ) < 6 ) goto error;
	fWCLayer[fNlayer] = new TA2CylMwpcWire( name, nelem, maxcl, maxclsize,this, dparm );
	fNlayer++;
	break;
	//   case EWCTypePlaneStrip:
	//     //I'm abusing ths entry for defective cylindrical strips. Introducing a new command-line keyword
	//     //does not work due to the stupid idea of somebody to define the map and keys also in class
	//     //TA2WireChamber, so that introducing new words leads to segfaults!
	//     if(sscanf(line, "%d%d", &DefLayer, &DefStrip) < 2) goto error;
	//     DeadStrPatt[DefLayer][DefStrip] = 1;
	//     break;
	//   case EWCTypePlaneDrift:
	//     //I'm abusing ths entry for a general shift along z-coordinate. Introducing a new command-line
	//     //keyword does not work due to the stupid idea of somebody to define the map and keys also in
	//     //class TA2WireChamber, so that introducing new words leads to segfaults!
	//     if(sscanf(line, "%lf", &TargZShift) < 1) goto error;
	//     break;
    case EWCTypeCylStrip:
	// Cathode strips helically "wound" on cylinder
	// Has pulse height but no time
	// layer parameters.....
	// radius, length, TgWC, pitch,
	if ( fNlayer >= fNLayer ){
	  PrintError( line, "<Too many WC layers input>");
	  return;
	}
// 	fIsEnergy = fIsPos = ETrue;
// 	fIsTime  = EFalse;
	if ( sscanf( line, "%s%d%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
	            name, &nelem, &maxcl, &maxclsize, dparm, dparm+1, dparm+2, dparm+3,
		    dparm+4, dparm+5, dparm+6, dparm+7, dparm+8, dparm+9, dparm+10 ) < 8 ) goto error;
	fWCLayer[fNlayer] = new TA2CylMwpcStrip( name, nelem, maxcl, maxclsize, this, dparm );
	fNlayer++;
	break;
    case EWCCalib:
      fIfCalib = kTRUE;
      break;
    case EWCIntersection:
      // Parameters for intersections
      static Int_t nCh = 0;
      if (nCh >= fNchamber)
      {
	cout << "Nch = "<< nCh << endl;
	PrintError(line, "<Too many WC intersections input lines>", EErrFatal);
	return;
      }

      // Susanna ---- init
      if (fIfCalib) {
	parfile.open("mwpc_params.dat");
	cout << "Reading from mwpc_params.dat ..." << endl;
	Double_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13;
	Double_t param[2][14];
	for (int ii=0; ii<fNchamber; ii++) {
	  parfile >> t0 >> t1 >> t2 >> t3 >> t4 >> t5 >> t6 >> t7 >> t8 >> t9 >> t10 >> t11 >> t12 >> t13;
	  param[ii][0]  = t0;
	  param[ii][1]  = t1;
	  param[ii][2]  = t2;
	  param[ii][3]  = t3;
	  param[ii][4]  = t4;
	  param[ii][5]  = t5;
	  param[ii][6]  = t6;
	  param[ii][7]  = t7;
	  param[ii][8]  = t8;
	  param[ii][9]  = t9;
	  param[ii][10] = t10;
	  param[ii][11] = t11;
	  param[ii][12] = t12;
	  param[ii][13] = t13;
	}
	parfile.close();
	
	for (int j=0; j<14; j++) 
	  dparm[j] = param[nCh][j];

	cout << "===> Parameters for MWPC-" << nCh << ":" << endl;
	cout << param[nCh][0] << "\t" << param[nCh][1] << "\t" << param[nCh][2] << "\t" << param[nCh][3] << "\t" <<
	  param[nCh][4] << "\t" << param[nCh][5] << "\t" << param[nCh][6] << "\t" << param[nCh][7] << "\t" << 
	  param[nCh][8] << "\t" << param[nCh][9] << "\t" << param[nCh][10] << "\t" << param[nCh][11] << "\t" <<
	  param[nCh][12] << "\t" << param[nCh][13] << endl;
      } 
      // Susanna --- end

      else 	
	if( sscanf( line, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
	          dparm, dparm+1, dparm+2, dparm+3, dparm+4, dparm+5, dparm+6,
		  dparm+7, dparm+8, dparm+9, dparm+10, dparm+11, dparm+12, dparm+13) < 6 ) goto error;
      fPhiCorrEI[0][nCh] = dparm[0]*kDegToRad;
      fPhiCorrEI[1][nCh] = dparm[1]*kDegToRad;
      fPhiCorrEI[2][nCh] = dparm[2];
      fPhiCorr[nCh] = dparm[3]*kDegToRad;
      fXcorr[nCh] = dparm[4];
      fYcorr[nCh] = dparm[5];
      fZcorr[0][nCh] = dparm[6];
      fZcorr[1][nCh] = dparm[7];
      fZcorr[2][nCh] = dparm[8];
      fZcorr[3][nCh] = dparm[9];
      fZcorr[4][nCh] = dparm[10];
      fZcorr[5][nCh] = dparm[11];
      fZcorr[6][nCh] = dparm[12];
      fZcorr[7][nCh] = dparm[13];
      ++nCh;
      break;
    case EWCSigmaZ:
      // SigmaZ(phi,Z)
      if ( sscanf( line, "%d%s", &n, name) < 2 ) goto error;
      if (n < 0 || n >= fNChamber)
      {
	std::cout << "Error at SigmaZ(phi,Z) loading: iCh=" << n << " >= Nch=" << fNChamber << std::endl;
	goto error;
      }
      fSigmaZ[n] = new TGraph2D(static_cast<TString>(getenv("acqu")) + "/data/" + static_cast<TString>(name));
      {
	Char_t c[4];
	sprintf(c,"%d",n);
	fSigmaZ[n]->SetName(static_cast<TString>(c)+"_"+static_cast<TString>(name));
      }
      break;
    case EWCSigmaPhi:
      // SigmaPhi(phi,Z)
      if ( sscanf( line, "%d%s", &n, name) < 2 ) goto error;
      if (n < 0 || n >= fNChamber)
      {
	std::cout << "Error at SigmaPhi(phi,Z) loading: iCh=" << n << " >= Nch=" << fNChamber << std::endl;
	goto error;
      }
      fSigmaPhi[n] = new TGraph2D(static_cast<TString>(getenv("acqu")) + "/data/" + static_cast<TString>(name));
      {
	Char_t c[4];
	sprintf(c,"%d",n);
	fSigmaPhi[n]->SetName(static_cast<TString>(c)+"_"+static_cast<TString>(name));
      }
      break;
    case EWCDetEff:
      // DetEff(phi,Z)
      if ( sscanf( line, "%d%d%s", &n, &id, name) < 3 ) goto error;
      if (n < 0 || n >= fNChamber)
      {
	std::cout << "Error at DetEff(phi,Z) loading: iCh=" << n << " >= Nch=" << fNChamber << std::endl;
	goto error;
      }
      fDetEff[n][id] = new TGraph2D(static_cast<TString>(getenv("acqu")) + "/data/" + static_cast<TString>(name));
      {
	Char_t c1[4], c2[4];
	sprintf(c1,"%d",n);
	sprintf(c2,"%d",id);
	fDetEff[n][id]->SetName(static_cast<TString>(c1)+static_cast<TString>(c2));
      }
      break;
    case EWCReadGeneratedKinematics:
      // Read Generated Kinematics?
      fReadGeneratedKinematics = kTRUE;
      std::cout << "Reading generated kinematics..." << std::endl;
    default:
      // Default detector configuration
      // If this flags it has done nothing carry on with local setup
      TA2Detector::SetConfig( line, key );
      break;
  }
  return;
  error: PrintError( line, "<WC configuration>" );
  return;
}

//_________________________________________________________________________________________
void TA2CylMwpc::SetChamberParm( Int_t n, Double_t* parm )
{
  // Store some further chamber parameters
  // There should be 7 provided
  if ( n < 8 ) {
    PrintError("","<Invalid MWPC parameters supplied>");
    return;
  }
  fMaxIntersect = static_cast<Int_t>(parm[0]);	// max # strip intersect points
  fMaxTrack     = static_cast<Int_t>(parm[1]);	// max # particle tracks
  fMaxVertex    = static_cast<Int_t>(parm[2]);	// max # vertices
  fMaxDphiWIE   = parm[3]*kDegToRad;		// max diff phi wire and phi intersect
  
  // Vertex limits i=0,1,2  Rmax, +Zmax -Zmax;
  // Confine vertices to target area
  fVertexLimits = new Double_t[3];
  for ( Int_t i=0; i<3; i++ ) fVertexLimits[i] = parm[i+4];
  
  // Max distance beetween 2 intersections forming a track
  fMaxDr = parm[7];
}

//_________________________________________________________________________________________
void TA2CylMwpc::PostInit()
{
  // Do some further setup before any display specification
  
  // Geometrical consts arrays allocations
  fR   = new Double_t[fNChamber];	// chamber radii
  fRtI = new Double_t[fNChamber];	// chamber inner strip radii
  fRtE = new Double_t[fNChamber];	// chamber external strip radii
  fC1  = new Double_t[fNChamber];	// some geom. const
  fC2  = new Double_t[fNChamber];	// some geom. const
  
  // Layers
  fI = new TA2CylMwpcStrip*[fNChamber];
  fE = new TA2CylMwpcStrip*[fNChamber];
  fW = new TA2CylMwpcWire*[fNChamber];
  fUsedClI = new Bool_t*[fNChamber];
  fUsedClW = new Bool_t*[fNChamber];
  fUsedClE = new Bool_t*[fNChamber];
  for (Int_t iCh=0; iCh<fNChamber; iCh++)
  {
    // Init internal, external and wire chambers layers ptr
    fI[iCh] = (TA2CylMwpcStrip*)fWCLayer[fChamberLayers[iCh][1]];
    fE[iCh] = (TA2CylMwpcStrip*)fWCLayer[fChamberLayers[iCh][3]];
    fW[iCh] = (TA2CylMwpcWire*)fWCLayer[fChamberLayers[iCh][2]];
    //
    fUsedClI[iCh] = new Bool_t[fI[iCh]->GetMaxClust()];
    fUsedClW[iCh] = new Bool_t[fW[iCh]->GetMaxClust()];
    fUsedClE[iCh] = new Bool_t[fE[iCh]->GetMaxClust()];
    // Geometrical consts
    fR[iCh]   = fW[iCh]->GetRadius(); // mm
    fRtI[iCh] = fI[iCh]->GetRadius()/fI[iCh]->GetTgWC(); // mm
    fRtE[iCh] = fE[iCh]->GetRadius()/fE[iCh]->GetTgWC(); // mm
    fC1[iCh]  = 2.*kPi/(fRtE[iCh]+fRtI[iCh]);
    fC2[iCh]  = 2.*kPi*fRtE[iCh]*fRtI[iCh]/(fRtE[iCh]+fRtI[iCh]);
  }
  
  // Intersections
  fIclI  = new Int_t*[fNChamber];
  fIclE  = new Int_t*[fNChamber];
  fZie   = new Double_t*[fNChamber];
  fPhiIE = new Double_t*[fNChamber];
  fNie   = new Int_t[fNChamber];
  fDphiWIE = new map<Double_t,pair<Int_t,Int_t> >[fNChamber];
  //
  fNinters = new Int_t[fNChamber];
  fInters  = new TA2MwpcIntersection*[fNChamber];
  fNintersTrue = new Int_t[fNChamber];
  fIintersTrue = new Int_t*[fNChamber];
  fNintersCandidate = new Int_t[fNChamber];
  fIintersCandidate = new Int_t*[fNChamber];
  // for histograming
  fTypeInter = new Int_t*[fNChamber];
  fZInter    = new Double_t*[fNChamber];
  fPhiInter  = new Double_t*[fNChamber];
  fDphiWS    = new Double_t*[fNChamber];
  fCGclI = new Double_t*[fNchamber];
  fCGclW = new Double_t*[fNchamber];
  fCGclE = new Double_t*[fNchamber];
  fAclIE = new Double_t*[fNchamber];
  //
  Int_t nMaxIE;
  for (Int_t iCh=0; iCh<fNChamber; ++iCh) {
    fNie[iCh] = 0;
    fNinters[iCh] = fNintersTrue[iCh] = fNintersCandidate[iCh] = 0;
    nMaxIE = fI[iCh]->GetMaxClust()*fE[iCh]->GetMaxClust()*kMaxNsolIE;
    fIclI[iCh] = new Int_t[nMaxIE];
    fIclE[iCh] = new Int_t[nMaxIE];
    fZie[iCh]  = new Double_t[nMaxIE];
    fPhiIE[iCh]= new Double_t[nMaxIE];
    //
    fInters[iCh] = new TA2MwpcIntersection[fMaxIntersect];
    fIintersTrue[iCh]       = new Int_t[fMaxIntersect];
    fIintersCandidate[iCh]  = new Int_t[fMaxIntersect];
    // for histograms
    fTypeInter[iCh] = new Int_t[fMaxIntersect+1];
    fZInter[iCh]    = new Double_t[fMaxIntersect+1];
    fPhiInter[iCh]  = new Double_t[fMaxIntersect+1];
    fDphiWS[iCh]    = new Double_t[fMaxIntersect+1];
    fCGclI[iCh] = new Double_t[fMaxIntersect+1];
    fCGclW[iCh] = new Double_t[fMaxIntersect+1];
    fCGclE[iCh] = new Double_t[fMaxIntersect+1];
    fAclIE[iCh] = new Double_t[fMaxIntersect+1];
  }
  
  // Tracks
  fTracks = new TA2MwpcTrack[fMaxTrack];
  // for histograming
  fTypeTrack  = new Int_t[fMaxTrack+1];
  fMagTrack   = new Double_t[fMaxTrack+1];
  fThetaTrack = new Double_t[fMaxTrack+1];
  fPhiTrack   = new Double_t[fMaxTrack+1];
  fDPhiInters = new Double_t[fMaxTrack+1];
  fDZInters   = new Double_t[fMaxTrack+1];
  fEtrack = new Double_t[fMaxTrack+1];
  for (Int_t i=0;i<3;++i) fPsVertex[i] = new Double_t[fMaxTrack+1];
  fPsVertexR = new Double_t[fMaxTrack+1];
  
  // Vertexes
  fMaxVertex = fMaxTrack*(fMaxTrack-1)>>1;
  fVertexes = new TVector3[fMaxVertex];
  fItracks  = new pair<Int_t,Int_t>[fMaxVertex];
  // for histograming
  for (Int_t i=0;i<3;++i) fVert[i] = new Double_t[fMaxVertex+1];
  fVertR = new Double_t[fMaxVertex+1];
  fTrackDist = new Double_t[fMaxVertex+1];
  
  // Simulation
  fDrHitsSim = new Double_t[fMaxIntersect+1];
  
  // Standard detector stuff
  TA2Detector::PostInit();  
}

//_________________________________________________________________________________________
void TA2CylMwpc::LoadVariable()
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
  
  // Standard wire chamber stuff
  TA2WireChamber::LoadVariable();
  
  // Intersections
  Char_t ind[4];
  for ( Int_t iCh=0; iCh<fNChamber; iCh++ ) {
    sprintf(ind,"%d",iCh+1);
    TA2DataManager::LoadVariable((new TString("IntsN"+TString(ind)))->Data(),		&fNinters[iCh],		EISingleX);
    TA2DataManager::LoadVariable((new TString("IntsType"+TString(ind)))->Data(),	 fTypeInter[iCh],	EIMultiX);
    TA2DataManager::LoadVariable((new TString("IntsZ"+TString(ind)))->Data(),		 fZInter[iCh],		EDMultiX);
    TA2DataManager::LoadVariable((new TString("IntsPhi"+TString(ind)))->Data(),	 	 fPhiInter[iCh],	EDMultiX);
    TA2DataManager::LoadVariable((new TString("IntsDphiWS"+TString(ind)))->Data(),	 fDphiWS[iCh],		EDMultiX);
    TA2DataManager::LoadVariable((new TString("CGclI"+TString(ind)))->Data(),		 fCGclI[iCh],		EDMultiX);
    TA2DataManager::LoadVariable((new TString("CGclW"+TString(ind)))->Data(),		 fCGclW[iCh],		EDMultiX);
    TA2DataManager::LoadVariable((new TString("CGclE"+TString(ind)))->Data(),		 fCGclE[iCh],		EDMultiX);
    TA2DataManager::LoadVariable((new TString("AclIE"+TString(ind)))->Data(),		 fAclIE[iCh],		EDMultiX);
  }
  
  // Tracks
  TA2DataManager::LoadVariable("TrackN",	&fNTrack,	EISingleX);
  TA2DataManager::LoadVariable("TrackNTrueBest",&fNtracksTrueBest,EISingleX);
  TA2DataManager::LoadVariable("TrackType",	 fTypeTrack,	EIMultiX);
  TA2DataManager::LoadVariable("TrackMag",	 fMagTrack,	EDMultiX);
  TA2DataManager::LoadVariable("TrackTheta",	 fThetaTrack,	EDMultiX);
  TA2DataManager::LoadVariable("TrackPhi",	 fPhiTrack,	EDMultiX);
  TA2DataManager::LoadVariable("TrackDPhiInters",fDPhiInters,	EDMultiX);
  TA2DataManager::LoadVariable("TrackDZInters",	 fDZInters,	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexX",	 fPsVertex[0],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexY",	 fPsVertex[1],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexZ",	 fPsVertex[2],	EDMultiX);
  TA2DataManager::LoadVariable("PsVertexR",	 fPsVertexR,	EDMultiX);
  TA2DataManager::LoadVariable("Etrack",	 fEtrack,	EDMultiX);
  
  // Vertexes
  TA2DataManager::LoadVariable("VertexN",	&fNVertex,	EISingleX);
  TA2DataManager::LoadVariable("VertexX",	 fVert[0],	EDMultiX);
  TA2DataManager::LoadVariable("VertexY",	 fVert[1],	EDMultiX);
  TA2DataManager::LoadVariable("VertexZ",	 fVert[2],	EDMultiX);
  TA2DataManager::LoadVariable("VertexR",	 fVertR,	EDMultiX);
  TA2DataManager::LoadVariable("TrackDist",	 fTrackDist,	EDMultiX);
  
  // Simulation
  TA2DataManager::LoadVariable("VertexSimX",	&fVertSim[0],	EDSingleX);
  TA2DataManager::LoadVariable("VertexSimY",	&fVertSim[1],	EDSingleX);
  TA2DataManager::LoadVariable("VertexSimZ",	&fVertSim[2],	EDSingleX);
}

//_________________________________________________________________________________________
void TA2CylMwpc::ReadDecoded()
{
  // Read back...
  //   either previously analysed data from Root Tree file
  //   or MC simulation results, assumed to have the same data structure
  // No MC simulation of MWPC yet
  
  // TODO Should it also read previously analysed experimental data?
  
  // Some cleanup
  for (Int_t iCh=0; iCh<fNChamber; ++iCh)
  {
    *(fI[iCh]->GetNClustPtr()) = 0;
    *(fE[iCh]->GetNClustPtr()) = 0;
    *(fW[iCh]->GetNClustPtr()) = 0; 
  }
  
  //
  if (fReadGeneratedKinematics)
  {
    ReadGeneratedKinematics();
  }
  else
  {
    ReadMC();
  }
  
  // Reconstruct tracks
  ResetWiresAndStripsUsage();
  MakeTracks();
  
  // Reconstruct vertexes for the best tracks
  MakeVertexes(fTracksTrueBest);
  MakeVertexes(fTracksTrueCandidateBest);
  MakeVertexes(fTracksCandidateBest);
  
  // Simulated vertex
  TVector3 vertex((Float_t*)(fEvent[EI_vertex]));
  vertex *=10.; // mm
  fVertSim[0] = vertex.X();
  fVertSim[1] = vertex.Y();
  fVertSim[2] = vertex.Z();
  
  // Mark end buffers
  MarkEndBuffers();
  //
  fHits[fNhits] = EBufferEnd;
  for (Int_t i=0; i<fNchamber; ++i)
  {
    *(fI[i]->GetLayerHits()) = EBufferEnd;
    *(fW[i]->GetLayerHits()) = EBufferEnd;
    *(fE[i]->GetLayerHits()) = EBufferEnd;
    //
    *(fI[i]->GetCGClust()+*(fI[i]->GetNClustPtr())) = EBufferEnd;
    *(fW[i]->GetCGClust()+*(fW[i]->GetNClustPtr())) = EBufferEnd;
    *(fE[i]->GetCGClust()+*(fE[i]->GetNClustPtr())) = EBufferEnd;
    //
    *(fI[i]->GetLenClust()) = EBufferEnd;
    *(fW[i]->GetLenClust()) = EBufferEnd;
    *(fE[i]->GetLenClust()) = EBufferEnd;
    //
    *(fI[i]->GetClust()) = EBufferEnd;
    *(fW[i]->GetClust()) = EBufferEnd;
    *(fE[i]->GetClust()) = EBufferEnd;
  }
  
  // Test
//   fWait = kTRUE;
//   Test();
}

//________________________________________________________________________________________
void TA2CylMwpc::ReadGeneratedKinematics()
{
  // For every detectable particle create pseudo-intersections (TA2MwpcIntersection)
  // taking into account MWPC resolution and detection efficiency
  
  Float_t *dircos = (Float_t*)(fEvent[EI_dircos]);
  TVector3 vertex((Float_t*)(fEvent[EI_vertex]));
  vertex *= 10.; // mm
  Int_t    npart  = *(Int_t*)(fEvent[EI_npart]);
  Int_t   *idpart = (Int_t*)(fEvent[EI_idpart]);
  //
  TVector3 v2, r[2];
  Double_t z, phi;
  for (Int_t i = 0; i < npart; ++i)
  {
    // For each MWPC find z and phi of intersection point
    v2 = vertex + TVector3(dircos + i*3);
    for (Int_t iCh=0; iCh<fNchamber; ++iCh)
    {
      // Select only detectable particle
      if ( fDetEff[iCh].find(idpart[i]) == fDetEff[iCh].end() ) continue;
      
      // Find the intersection point r[1] with MWPC (r[0] is not needed)
      if ( TA2Math::IntersectLineAndCylinder(vertex, v2, fR[iCh], r[0], r[1]) == 0 ) continue;
      
      //
      z   = r[1].Z();
      phi = TVector2::Phi_0_2pi(r[1].Phi());
      
      // Smearing
      z   = SmearZ(iCh,phi,z);
      phi = SmearPhi(iCh,phi,z);
      
      // Check if Z is out of the chamber length
      if (z > fI[iCh]->GetLength() || z < -fI[iCh]->GetLength() ) continue;
      
      // Build MWPC intersection within given efficiency
      if ( gRandom->Rndm() > fDetEff[iCh].find(idpart[i])->second->Interpolate(phi,z) ) continue;
      
      // Find CG of wires and strips clusters
      // Inner strip
      fI[iCh]->CGClusterFromPhiZ(z,phi,TA2CylMwpcStrip::kMinus);
      // Outer strip
      fE[iCh]->CGClusterFromPhiZ(z,phi,TA2CylMwpcStrip::kPlus);
      // Wires clusters
      *(fW[iCh]->GetCGClust()+(*(fW[iCh]->GetNClustPtr()))++) = phi;
      
      //
      if (TooManyInters(iCh)) break;
      
      // Add a pseudo-intersection
      AddIntersection(iCh, TA2MwpcIntersection::kEWI, fI[iCh]->GetNClust(), fW[iCh]->GetNClust(), fE[iCh]->GetNClust(), z, phi, 0., 0.);
    }
  }
  
}

//________________________________________________________________________________________
void TA2CylMwpc::ReadMC()
{
  // Read the data simulated with A2-G4
  // NOTE: Works only for 1 or 2 MWPCs
  
  fNhits = *(Int_t*)(fEvent[EI_nmwpc]);		//total number of hits in all layers
  Int_t *index = (Int_t*)(fEvent[EI_imwpc]);	//index of layer hit
  Float_t *posx = (Float_t*)(fEvent[EI_mposx]); 
  Float_t *posy = (Float_t*)(fEvent[EI_mposy]); 
  Float_t *posz = (Float_t*)(fEvent[EI_mposz]);
  Float_t *e = (Float_t*)(fEvent[EI_emwpc]);
  
  // Read in all hit vectors
  Int_t nHitsLayer[] = {0,0,0,0};	// # hits per layer
  TVector3 hitLayer[100][4];		// put the cathode hit vectors 4=number of layers
  Double_t eLayer[100][4];
  for (UInt_t i=0; i<fNhits; ++i)
  {
    hitLayer[nHitsLayer[index[i]]][index[i]].SetXYZ(posx[i],posy[i],posz[i]);
    eLayer[nHitsLayer[index[i]]][index[i]] = e[i]*1000000.;
    nHitsLayer[index[i]]++;
  }
  
  //Now look for intersections between cathode planes
  Double_t zInter, phiInter, eInter = 0.;
  Double_t dR;
  Int_t iMax, jMax;
  // Loop over chambers
  for (Int_t k, l, iCh=0; iCh<=1; ++iCh)
  {
    k = 2*iCh;
    l = k + 1;
    //
    iMax = std::min(nHitsLayer[k],fI[iCh]->GetMaxClust());
    jMax = std::min(nHitsLayer[l],fE[iCh]->GetMaxClust());
    
    // Find CG of inner strips clusters
    for (Int_t i=0; i<iMax; ++i)
    {
      fI[iCh]->CGClusterFromPhiZ(hitLayer[i][k].Z(),TVector2::Phi_0_2pi(hitLayer[i][k].Phi()),TA2CylMwpcStrip::kMinus);
    }
    // Find CG of external strips clusters
    for (Int_t j=0; j<jMax; ++j)
    {
      fE[iCh]->CGClusterFromPhiZ(hitLayer[j][l].Z(),TVector2::Phi_0_2pi(hitLayer[j][l].Phi()),TA2CylMwpcStrip::kPlus);
    }
    
    // Loop over inner layer
    for (Int_t i=0; i<iMax; ++i)
    {
      // Loop over outer layer
      for (Int_t j=0; j<jMax; ++j)
      {
	// Check if there is an intersection
	dR = ( hitLayer[i][0] - hitLayer[j][1] ).Mag();
	if ( dR > 20. ) continue;
	
	// We have an intersection
	zInter = ( 0.5*(hitLayer[i][k] + hitLayer[j][l]) ).Z();
	phiInter = TVector2::Phi_0_2pi( ( hitLayer[i][k] + hitLayer[j][l] ).Phi() );
	eInter = eLayer[i][k] + eLayer[j][l];
	
	// CG of wires clusters
	if (fW[iCh]->GetNClust() < fW[iCh]->GetMaxClust())
	{
	  *(fW[iCh]->GetCGClust()+(*(fW[iCh]->GetNClustPtr()))++) = phiInter;
	}
	
	// Smearing
	zInter   = SmearZ(iCh,phiInter,zInter);
	phiInter = SmearPhi(iCh,phiInter,zInter);
	
	//
	if (TooManyInters(iCh)) continue;
	
	//
	fDrHitsSim[fNinters[iCh]] = dR;
	
	// Add a pseudo-intersection
	AddIntersection(iCh, TA2MwpcIntersection::kEWI, i, fNinters[iCh], j, zInter, phiInter, 0., eInter);
      }
    }
  }
  
}

//_________________________________________________________________________________________
void TA2CylMwpc::Decode()
{
  // Do the basic decoding (e.g. ADC -> Energy)
  // Find chamber coordinates from clusters of hits
  // Join coordinates to make tracks
  
  // Basic wire chamber decode: wires and strips clusters reconstruction
  TA2WireChamber::Decode();
  
  // Reconstruct intersections
  for (Int_t iCh=0; iCh<fNChamber; ++iCh)
  {
    ResetWiresAndStripsUsage(iCh);
    IntersectLayers(iCh);
  }
  
  // Reconstruct tracks
  ResetWiresAndStripsUsage();
  MakeTracks();
  
  // Reconstruct vertexes for the best tracks
  MakeVertexes(fTracksTrueBest);
  MakeVertexes(fTracksTrueCandidateBest);
  MakeVertexes(fTracksCandidateBest);
  
  // Mark end buffers
  MarkEndBuffers();

  // Test
//   Test();
}

//_________________________________________________________________________________________
void TA2CylMwpc::Cleanup()
{
  // Reset some arrays used in the analysis
  
  // Base class cleanup
  TA2WireChamber::Cleanup();
  
  // Intersections
  for (Int_t iCh = 0; iCh<fNChamber; ++iCh)
  {
    fNie[iCh] = 0;
    fDphiWIE[iCh].clear();
    //
    fNinters[iCh] = fNintersTrue[iCh] = fNintersCandidate[iCh] = 0;
  }
  
  // Tracks
  fNTrack = 0;
  fNtracksTrue = fNtracksTrueCandidate = fNtracksCandidate = 0;
  fNtracksTrueBest = fNtracksTrueCandidateBest = fNtracksCandidateBest = 0;
  fTracksTrue.clear();
  fTracksTrueCandidate.clear();
  fTracksCandidate.clear();
  fTracksTrueBest.clear();
  fTracksTrueCandidateBest.clear();
  fTracksCandidateBest.clear();
  
  // Vertexes
  fNVertex = 0;
}

//_________________________________________________________________________________________
void TA2CylMwpc::IntersectLayers(Int_t iCh)
{
  // For chamber iCh, find intersection points defined by
  // inner strip cluster clI, external strip cluster clE and wire cluster clW.
  // In case of >1 solutions, a solution with the minimal dPhiWSS is accepted
  
  // Possible internal-external strips intersections
  for (Int_t clI=0; clI<fI[iCh]->GetNClust(); ++clI)
  {
    Double_t phiI = fI[iCh]->GetCGClust(clI);
    for (Int_t clE=0; clE<fE[iCh]->GetNClust(); ++clE)
    {
      Double_t phiE = fE[iCh]->GetCGClust(clE);
      UInt_t nSolIE = 0; // strip-strip solutions counter
      for (Int_t j=-2; j<=0; ++j)
      {
	if (nSolIE == kMaxNsolIE) break;
	for (Int_t k=-1; k<=1; ++k)
	{
	  Double_t zNoCorr   = ZinterIE(iCh,phiI,phiE,j,k);	// Not corrected Z IE
	  Double_t phiNoCorr = PhiInterIE(iCh,phiI,phiE,j,k);	// Not corrected Phi IE
	  Double_t phiCorr = PhiCorrEI(iCh,zNoCorr,phiNoCorr);	// Phi correction for rotation of E wrt to I as a function of Z and Phi
	  Double_t zIE = zNoCorr + ZcorrEI(iCh,phiCorr);	// Z
	  if (!fE[iCh]->IsInside(zIE)) continue;
	  Double_t phiIE = phiNoCorr + phiCorr;			// Phi
	  if ( TMath::Abs(phiIE) < 0.0001 )        phiIE = 0.;
	  if ( TMath::Abs(phiIE-2.*kPi) < 0.0001 ) phiIE = 2.*kPi;
	  if ( phiIE < 0. || phiIE > 2.*kPi ) continue;
	  fIclI[iCh][fNie[iCh]]  = clI;
	  fIclE[iCh][fNie[iCh]]  = clE;
	  fZie[iCh][fNie[iCh]]   = zIE;
	  fPhiIE[iCh][fNie[iCh]] = phiIE;
	  ++fNie[iCh];
	  if (++nSolIE == kMaxNsolIE) break;
	}
      }
    }
  }
  
  // Find possible WIE intersections
  Double_t dPhiWIE;
  for (Int_t iW=0; iW<fW[iCh]->GetNClust(); ++iW)
  {
    for (Int_t iIE=0; iIE<fNie[iCh]; ++iIE)
    {
      dPhiWIE = TA2Math::AbsPhi( fW[iCh]->GetCGClust(iW) - fPhiIE[iCh][iIE] );
      if (dPhiWIE > fMaxDphiWIE) continue;
      fDphiWIE[iCh][dPhiWIE] = make_pair(iW,iIE);
    }
  }
  
  // Select true wire-strip-strip intersections
  Double_t ampl;
  for (map<Double_t,pair<Int_t,Int_t> >::iterator iterDPhi = fDphiWIE[iCh].begin(); iterDPhi != fDphiWIE[iCh].end(); ++iterDPhi)
  {
    if (TooManyInters(iCh)) break;
    Int_t iClW = iterDPhi->second.first;
    if (fUsedClW[iCh][iClW]) continue; // if used wire cluster => continue
    Int_t iIE  = iterDPhi->second.second;
    Int_t iClI = fIclI[iCh][iIE];
    if (fUsedClI[iCh][iClI]) continue; // if used internal strip cluster => continue
    Int_t iClE = fIclE[iCh][iIE];
    if (fUsedClE[iCh][iClE]) continue; // if used external strip cluster => continue
    //
    Double_t phiW  = fW[iCh]->GetCGClust(iClW);
    Double_t phiIE = fPhiIE[iCh][iIE];
    ampl = (fI[iCh]->GetClustEn(iClI) + fE[iCh]->GetClustEn(iClE))/2.;
    AddIntersection(iCh, TA2MwpcIntersection::kEWI, iClI, iClW, iClE, fZie[iCh][iIE], phiW, TVector2::Phi_mpi_pi(phiW-phiIE), ampl);
  }
  
  // Wire-strip intersections candidates: (110) and (011)
  Double_t z;
  for (Int_t clW=0; clW<fW[iCh]->GetNClust(); ++clW)
  {
    if (TooManyInters(iCh)) break;
    if (fUsedClW[iCh][clW]) continue;
    Double_t phiW = fW[iCh]->GetCGClust(clW);
     // Internal
    for (Int_t clI=0; clI<fI[iCh]->GetNClust(); ++clI)
    {
      if (TooManyInters(iCh)) break;
      if (fUsedClI[iCh][clI]) continue;
      Double_t phiI = fI[iCh]->GetCGClust(clI);
      ampl = fI[iCh]->GetClustEn(clI);
      for (Int_t m=-2; m<=0; ++m)
      {
	if (TooManyInters(iCh)) break;
	z = ZinterWI(iCh,phiW,phiI,m);
	if (!fI[iCh]->IsInside(z)) continue;
	AddIntersection(iCh, TA2MwpcIntersection::kWI, clI, clW, kNullHit, z, phiW, ENullFloat, ampl);
      }
    }
    // External
    for (Int_t clE=0; clE<fE[iCh]->GetNClust(); ++clE)
    {
      if (TooManyInters(iCh)) break;
      if (fUsedClE[iCh][clE]) continue;
      Double_t phiE = fE[iCh]->GetCGClust(clE);
      ampl = fE[iCh]->GetClustEn(clE);
      for (Int_t k=-1; k<=1; ++k)
      {
	if (TooManyInters(iCh)) break;
	Double_t zNoCorr = ZinterWE(iCh,phiW,phiE,k);
	z = zNoCorr + ZcorrEW(iCh,zNoCorr,phiW);
	if (!fE[iCh]->IsInside(z)) continue;
	AddIntersection(iCh, TA2MwpcIntersection::kEW, kNullHit, clW, clE, z, phiW, ENullFloat, ampl);
      }
    }
  }
  
  // IE strips intersection candidates: (101)
  for (Int_t iIE = 0; iIE<fNie[iCh]; ++iIE)
  {
    if (TooManyInters(iCh)) break;
    Int_t iClI = fIclI[iCh][iIE];
    if (fUsedClI[iCh][iClI]) continue;
    Int_t iClE = fIclE[iCh][iIE];
    if (fUsedClE[iCh][iClE]) continue;
    ampl = (fI[iCh]->GetClustEn(iClI) + fE[iCh]->GetClustEn(iClE))/2.;
    AddIntersection(iCh, TA2MwpcIntersection::kEI, iClI, kNullHit, iClE, fZie[iCh][iIE], fPhiIE[iCh][iIE], ENullFloat, ampl);
  }
  
}

//_________________________________________________________________________________________
void TA2CylMwpc::MakeTracks()
{
  // Make tracks
  // NOTE: works for 2 MWPCs only
  
  // If fNChamber < 2 there is no possibility to reconstruct a track, for > 2 it would be better to make a fit
  if (fNChamber!=2) return;
  
  // Build all possible tracks
  // Fill the array fTracks and the fTracksTrue, fTracksTrueCandidate, fTracksCandidate maps
  const TVector3 *r[2];
  Bool_t isTrueInter[2];
  Int_t type[2], i[2];
  for (i[0]=0; i[0]<fNinters[0]; ++i[0])
  {
    if (TooManyTracks()) break;
    type[0] = fInters[0][i[0]].GetType();
    isTrueInter[0] = type[0] == TA2MwpcIntersection::kEWI;
    r[0] = fInters[0][i[0]].GetPosition();
    for (i[1]=0; i[1]<fNinters[1]; ++i[1])
    {
      if (TooManyTracks()) break;
      type[1] = fInters[1][i[1]].GetType();
      isTrueInter[1] = type[1] == TA2MwpcIntersection::kEWI;
      r[1] = fInters[1][i[1]].GetPosition();
      // Try to build a track and in case of success add it to the tracks array
      if (fTracks[fNTrack].BuildTrack(*r[0],*r[1],fMaxDr,fVertexLimits,i,type))
      {
	if (isTrueInter[0] && isTrueInter[1])
	  AddTrackTo(fTracksTrue);		// True tracks
	else if (isTrueInter[0] || isTrueInter[1])
	  AddTrackTo(fTracksTrueCandidate);	// True-candidate tracks
	else
	  AddTrackTo(fTracksCandidate);		// Candidate tracks
	++fNTrack;
      }
    }
  }
  
  // Select the best tracks
  // Among the fTracksTrue
  SelectBestTracks(fTracksTrue,fTracksTrueBest);
  // Among the fTracksTrueCandidate
  SelectBestTracks(fTracksTrueCandidate,fTracksTrueCandidateBest);
  // Among the fTracksCandidate
  SelectBestTracks(fTracksCandidate,fTracksCandidateBest);
  
  // Get # of tracks
  // Possible
  fNtracksTrue          = fTracksTrue.size();
  fNtracksTrueCandidate = fTracksTrueCandidate.size();
  fNtracksCandidate     = fTracksCandidate.size();
  // Best
  fNtracksTrueBest          = fTracksTrueBest.size();
  fNtracksTrueCandidateBest = fTracksTrueCandidateBest.size();
  fNtracksCandidateBest     = fTracksCandidateBest.size();
  
}

//_________________________________________________________________________________________
void TA2CylMwpc::SelectBestTracks(const map<Double_t,Int_t> &mapTracks, map<Double_t, Int_t> &mapTracksBest)
{
  // Select best tracks
  
  for (map<Double_t,Int_t>::const_iterator iterTrack = mapTracks.begin(); iterTrack != mapTracks.end(); ++iterTrack)
  {
    if (IsUsedWIE(iterTrack->second)) continue;
    mapTracksBest[iterTrack->first] = iterTrack->second;
    SetUsedWIE(iterTrack->second);
  }
}

//_________________________________________________________________________________________
void TA2CylMwpc::MakeVertexes(const map<Double_t,Int_t> &mapTracks)
{
  // Reconstruct all possible vertexes
  
  Int_t i,j;
  for (map<Double_t,Int_t>::const_iterator iterTrack1 = mapTracks.begin(); iterTrack1 != mapTracks.end(); ++iterTrack1)
  {
    i = iterTrack1->second;
    fItracks[fNVertex].first = i;
    for (map<Double_t,Int_t>::const_iterator iterTrack2 = iterTrack1; iterTrack2 != mapTracks.end(); ++iterTrack2)
    {
      if (iterTrack1 == iterTrack2) continue;
      j = iterTrack2->second;
      fItracks[fNVertex].second = j;
      //
      fVertexes[fNVertex] = fTracks[i].Vertex(fTracks[j],fTrackDist[fNVertex]);
      //
      fVert[0][fNVertex] = fVertexes[fNVertex].X();
      fVert[1][fNVertex] = fVertexes[fNVertex].Y();
      fVert[2][fNVertex] = fVertexes[fNVertex].Z();
      fVertR[fNVertex]   = fVertexes[fNVertex].Perp();
      //
      ++fNVertex;
    }
  }
}

//_________________________________________________________________________________________
void TA2CylMwpc::MarkEndBuffers()
{
  // Mark EndBuffer for the output arrays to be displayed

  // Intersections
  for (Int_t iCh=0; iCh<fNChamber; ++iCh)
  {
    fTypeInter[iCh][fNinters[iCh]] = EBufferEnd;
    fZInter[iCh][fNinters[iCh]]    = EBufferEnd;
    fPhiInter[iCh][fNinters[iCh]]  = EBufferEnd;
    fDphiWS[iCh][fNinters[iCh]]    = EBufferEnd;
    fCGclI[iCh][fNinters[iCh]] = EBufferEnd;
    fCGclW[iCh][fNinters[iCh]] = EBufferEnd;
    fCGclE[iCh][fNinters[iCh]] = EBufferEnd;
    fAclIE[iCh][fNinters[iCh]] = EBufferEnd;
  }

  // Tracks
  fTypeTrack[fNTrack] = EBufferEnd;
  fMagTrack[fNTrack]  = EBufferEnd;
  fThetaTrack[fNTrack]= EBufferEnd;
  fPhiTrack[fNTrack]  = EBufferEnd;
  fDPhiInters[fNTrack] = EBufferEnd;
  fDZInters[fNTrack] = EBufferEnd;
  for (Int_t i=0; i<3; ++i)
  {
    fPsVertex[i][fNTrack] = EBufferEnd;
  }
  fPsVertexR[fNTrack] = EBufferEnd;
  fEtrack[fNTrack] = EBufferEnd;
  
  // Vertexes
  for (Int_t i=0; i<3; ++i)
  {
    fVert[i][fNVertex] = EBufferEnd;
  }
  fVertR[fNVertex] = EBufferEnd;
  fTrackDist[fNVertex] = EBufferEnd;
}

//_________________________________________________________________________________________
// void TA2CylMwpc::Test()
void TA2CylMwpc::Test(Bool_t WantDisplay)
{
  // Test output
  fMwpcDisplay = WantDisplay;
  if (fMwpcDisplay) {
    fWait = kTRUE;
    InitGeometry();
  }
  
  // Print file name & event number
  cout << endl;
  cout << "______________________________________ TA2CylMwpc ______________________________________"<< endl;
  cout << "Event#: " << gAN->GetNEvent() << "\tAccepted#: " << gAN->GetNEvAnalysed() << "\tDAQ#: " << gAN->GetNDAQEvent() << endl;
  
  // Test intersections
  for (Int_t iCh=0; iCh<fNchamber; ++iCh)
  {
    cout << endl << "===> Ch#: " << iCh << endl;
    // IE intersections
    cout << "> IE intersections" << endl;
    cout << "nClI: " << fI[iCh]->GetNClust() << "\t nClE: " << fE[iCh]->GetNClust() << "\t nIE: " << fNie[iCh] << endl;
    for (Int_t i=0; i<fNie[iCh]; ++i)
    {
      cout << i << "\tphiIE: " << fPhiIE[iCh][i]*kRadToDeg << "\tzIE: " << fZie[iCh][i] << "\tiClI: "<< fIclI[iCh][i] << "\tCGI: " << fI[iCh]->GetCGClust(fIclI[iCh][i]) << "\tiClE: " << fIclE[iCh][i] << "\tCGE: " << fE[iCh]->GetCGClust(fIclE[iCh][i]) << endl;
    }
    // W clusters
    cout << "> W clusters" << endl;
    for (Int_t iW=0; iW<fW[iCh]->GetNClust(); ++iW)
    {
      cout << iW << "\tphiW: " << fW[iCh]->GetCGClust(iW)*kRadToDeg << endl;
    }
    // Possible WIE intersections which fit the fMaxDphiWIE condition
    cout << "> Possible WIE intersections (possible true intersections)" << endl;
    for (map<Double_t,pair<Int_t,Int_t> >::iterator iterDPhi = fDphiWIE[iCh].begin(); iterDPhi != fDphiWIE[iCh].end(); ++iterDPhi)
    {
      cout << "phiWIE: " << iterDPhi->first*kRadToDeg << "\tiIE: " << iterDPhi->second.second << "\tiW: " << iterDPhi->second.first << endl;
    }
    // Intersections
    cout << "> Intersections" << endl;
    cout << "Ninters: " << fNinters[iCh] << "\tNintersTrue: " << fNintersTrue[iCh] << "\tNintersCandidate: " << fNintersCandidate[iCh] << endl;
    Int_t iInter;
    // True
    for (Int_t i=0; i<fNintersTrue[iCh]; ++i)
    {
      iInter = fIintersTrue[iCh][i];
      cout << iInter << "\tType: " << fInters[iCh][iInter].GetType() << "\tiClI: " <<  fInters[iCh][iInter].GetIclI() << "\tiClW: " << fInters[iCh][iInter].GetIclW() << "\tiClE: " << fInters[iCh][iInter].GetIclE() << "\tZ: " << fInters[iCh][iInter].GetZ() << "\tPhi: " << TVector2::Phi_0_2pi(fInters[iCh][iInter].GetPhi())*kRadToDeg << "\tphiWIE: " << fInters[iCh][iInter].GetDphiWIE()*kRadToDeg << endl;
    }
    // Candidates
    for (Int_t i=0; i<fNintersCandidate[iCh]; ++i)
    {
      iInter = fIintersCandidate[iCh][i];
      cout << iInter << "\tType: " << fInters[iCh][iInter].GetType() << "\tiClI: " <<  fInters[iCh][iInter].GetIclI() << "\tiClW: " << fInters[iCh][iInter].GetIclW() << "\tiClE: " << fInters[iCh][iInter].GetIclE() << "\tZ: " << fInters[iCh][iInter].GetZ() << "\tPhi: " << TVector2::Phi_0_2pi(fInters[iCh][iInter].GetPhi())*kRadToDeg << "\tphiWIE: " << fInters[iCh][iInter].GetDphiWIE()*kRadToDeg << endl;
    }
  }
  
  // Tracks
  cout << endl << "=================== Tracks ====================" << endl;
  cout << "Ntracks: " << fNTrack << "\tNtracksTrue: " << fNtracksTrue << "\tNtracksTrueCandidate: " << fNtracksTrueCandidate << "\tNtracksCandidate: " << fNtracksCandidate << endl;
  for (Int_t i=0; i<fNTrack; ++i)
  {
    cout << i << "\tiInter0: " << fTracks[i].GetIinter(0) << "\tiInter1: " << fTracks[i].GetIinter(1) << "\tType: "<< fTracks[i].GetType() << "\tMag: " << fTracks[i].GetDirCos().Mag() << "\tPhi: " <<  TVector2::Phi_0_2pi(fTracks[i].GetPhi())*kRadToDeg << "\tTheta: " << fTracks[i].GetTheta()*kRadToDeg << "\tPhiOrig: "<< TVector2::Phi_0_2pi(fTracks[i].GetOrigin().Phi())*kRadToDeg << "\tThetaOrig: " << fTracks[i].GetOrigin().Theta()*kRadToDeg <<
    "\tOx: "<< fTracks[i].GetOrigin().X() << " Oy: " << fTracks[i].GetOrigin().Y() << " Oz: " << fTracks[i].GetOrigin().Z() << endl;
  }
  cout << "NtracksTrueBest: " << fNtracksTrueBest << endl;
  Int_t iTr;
  for (map<Double_t,Int_t>::iterator iTrack = fTracksTrueBest.begin(); iTrack != fTracksTrueBest.end(); ++iTrack)
  {
    iTr = iTrack->second;
    cout << "iTrack: " << iTr << "\tMag: " << iTrack->first << "\tPhi: " << TVector2::Phi_0_2pi(fTracks[iTr].GetPhi())*kRadToDeg << "\tTheta: " << fTracks[iTr].GetTheta()*kRadToDeg << "\tPsR: " << fTracks[iTr].GetPsVertex().Perp() << "\tPsX: " << fTracks[iTr].GetPsVertex().X() << "\tPsY: " << fTracks[iTr].GetPsVertex().Y() << "\tPsZ: " << fTracks[iTr].GetPsVertex().Z() << endl;
  }
  cout << "NtracksTrueCandidateBest: " << fNtracksTrueCandidateBest << endl;
  for (map<Double_t,Int_t>::iterator iTrack = fTracksTrueCandidateBest.begin(); iTrack != fTracksTrueCandidateBest.end(); ++iTrack)
  {
    iTr = iTrack->second;
    cout << "iTrack: " << iTr << "\tMag: " << iTrack->first << "\tPhi: " << TVector2::Phi_0_2pi(fTracks[iTr].GetPhi())*kRadToDeg << "\tTheta: " << fTracks[iTr].GetTheta()*kRadToDeg << "\tPsR: " << fTracks[iTr].GetPsVertex().Perp() << "\tPsX: " << fTracks[iTr].GetPsVertex().X() << "\tPsY: " << fTracks[iTr].GetPsVertex().Y() << "\tPsZ: " << fTracks[iTr].GetPsVertex().Z() << endl;
  }
  cout << "NtracksCandidateBest: " << fNtracksCandidateBest << endl;
  for (map<Double_t,Int_t>::iterator iTrack = fTracksCandidateBest.begin(); iTrack != fTracksCandidateBest.end(); ++iTrack)
  {
    iTr = iTrack->second;
    cout << "iTrack: " << iTr << "\tMag: " << iTrack->first << "\tPhi: " << TVector2::Phi_0_2pi(fTracks[iTr].GetPhi())*kRadToDeg << "\tTheta: " << fTracks[iTr].GetTheta()*kRadToDeg << "\tPsR: " << fTracks[iTr].GetPsVertex().Perp() << "\tPsX: " << fTracks[iTr].GetPsVertex().X() << "\tPsY: " << fTracks[iTr].GetPsVertex().Y() << "\tPsZ: " << fTracks[iTr].GetPsVertex().Z() << endl;
  }
  
  // Vertexes
  cout << "> Possible vertexes" << endl;
  for (Int_t i=0; i<fNVertex; ++i)
  {
    cout << i << "\tiTr1: " << fItracks[i].first << "\tiTr2: "<< fItracks[i].second << "\tR: " << fVertexes[i].Perp() << "\tX: " << fVertexes[i].X() << "\tY: " << fVertexes[i].Y() << "\tZ: " << fVertexes[i].Z() << endl;
  }
  
  //
  cout << "________________________________________________________________________________________"<< endl;
  // start display
  if (fMwpcDisplay) {

    Int_t fVerbose = 2;

    if (fVerbose>0) {
      cout << endl;
      cout << "=======================================================" << endl;
      cout << "=================== MY TESTS ==========================" << endl;
      cout << "=======================================================" << endl;
      cout << "Event # " << gAN->GetNDAQEvent() << endl; 
      cout << "In this event " << fNTrack << " tracks have been found " << endl;
      cout << endl;
    }

    Double_t phiwire[2][20], phiint[2][20];
    Double_t xwire[2][20], ywire[2][20];//, zwire[2][20];
    Double_t xint[2][20], yint[2][20], zint[2][20];
    Double_t xtrk[2][20], ytrk[2][20], ztrk[2][20];
    TMarker *w[2][20], *inter[2][20], *inter_yz[2][20];
    Int_t iInter[2][20];

    for (Int_t iCh = 0; iCh < 2; iCh++) {
      
      if (fVerbose>0)
	cout << "++++++++++++++++ MWPC" << iCh << " ++++++++++++++++" << endl;
      
      // check on firing wires
      if (fVerbose>0)
	cout << "--> " << fW[iCh]->GetNClust() << " clusters (open circles)" << endl;
      for (Int_t iW = 0; iW < fW[iCh]->GetNClust(); ++iW) {
	phiwire[iCh][iW] = fW[iCh]->GetCGClust(iW); // rad
	xwire[iCh][iW] = fR[iCh] * TMath::Cos(phiwire[iCh][iW]);
	ywire[iCh][iW] = fR[iCh] * TMath::Sin(phiwire[iCh][iW]);
	if (fVerbose>1)
	  cout << "cluster " << iW << "\t phiW: " << phiwire[iCh][iW]*kRadToDeg << "\t ==> (" << xwire[iCh][iW] << ", " << ywire[iCh][iW] << ")" << endl;

	w[iCh][iW] = new TMarker(xwire[iCh][iW], ywire[iCh][iW], 24);
	if (iCh==0) w[iCh][iW]->SetMarkerColor(kAzure+7);
	else w[iCh][iW]->SetMarkerColor(kBlue);
	c->cd(); w[iCh][iW]->Draw("same");
      }
      c->Update();      c->Modified();

      // check on intersections on wires
      if (fVerbose>0)
	cout << "--> " << fNinters[iCh] << " intersections (stars)" << endl;
      for (Int_t iInt = 0; iInt < fNinters[iCh]; iInt++) {
	Int_t nW = fInters[iCh][iInt].GetIclW();
	if (nW != -1)
	  w[iCh][nW]->Delete();
	phiint[iCh][iInt] = TVector2::Phi_0_2pi(fInters[iCh][iInt].GetPhi());
	xint[iCh][iInt] = fInters[iCh][iInt].GetPosition()->X(); 
	yint[iCh][iInt] = fInters[iCh][iInt].GetPosition()->Y(); 
	if (fVerbose>1)
	  cout << "intersection " << iInt << " using wire cluster " << nW << "\t ==> phiInt: " << phiint[iCh][iInt]*kRadToDeg << "\t ==> (" << xint[iCh][iInt] << ", " << yint[iCh][iInt] << ")" << endl;

	inter[iCh][iInt] = new TMarker(xint[iCh][iInt], yint[iCh][iInt], 29);
	inter[iCh][iInt]->SetMarkerSize(1.3);
	if (iCh==0) inter[iCh][iInt]->SetMarkerColor(kAzure+7);
	else 	inter[iCh][iInt]->SetMarkerColor(kBlue);
	c->cd(); inter[iCh][iInt]->Draw("same");

	// yz plane
	zint[iCh][iInt] = fInters[iCh][iInt].GetPosition()->Z();
	inter_yz[iCh][iInt] = new TMarker(zint[iCh][iInt], yint[iCh][iInt], 29);
	inter_yz[iCh][iInt]->SetMarkerSize(1.3);
	if (iCh==0) inter_yz[iCh][iInt]->SetMarkerColor(kAzure+7);
	else 	inter_yz[iCh][iInt]->SetMarkerColor(kBlue);
	c2->cd(); inter_yz[iCh][iInt]->Draw("same");
      }
      c->Update();       c->Modified();
      c2->Update();      c2->Modified();

      // check on "global" tracks
      if (fVerbose>0)
	cout << "--> " << fNTrack << " track points (full circles)" << endl;
      for (Int_t iTrk = 0; iTrk < fNTrack; iTrk++) {
	iInter[iCh][iTrk] = fTracks[iTrk].GetIinter(iCh);
	inter[iCh][iInter[iCh][iTrk]]->SetMarkerStyle(20);
	xtrk[iCh][iTrk] = xint[iCh][iInter[iCh][iTrk]];
	ytrk[iCh][iTrk] = yint[iCh][iInter[iCh][iTrk]];
	// yz plane
	inter_yz[iCh][iInter[iCh][iTrk]]->SetMarkerStyle(20);
	ztrk[iCh][iTrk] = zint[iCh][iInter[iCh][iTrk]];
	if (fVerbose>1)
	  cout << "points for track " << iTrk << " using intersection " << iInter[iCh][iTrk] << " on MWPC" << iCh << " ==> (" << xtrk[iCh][iTrk] << ", " << ytrk[iCh][iTrk] << ")" << endl;

      }
      c->Update();      c->Modified();
      c2->Update();     c2->Modified();
    }

    TLine *trkl[20], *trkl_yz[20];
    Int_t quality[20];

    if (fVerbose>0) {
      cout << endl;
      cout << "----------------- " << fNTrack << " tracks -----------------" << endl;
    }

    // check on track "quality"
    // true = 6 --> RED
    for (map<Double_t,Int_t>::iterator iTrack = fTracksTrue.begin(); iTrack != fTracksTrue.end(); ++iTrack) {
	Int_t iTr = iTrack->second;
	quality[iTr] = 6;
    }
    // true best = 5 --> VIOLET
    for (map<Double_t,Int_t>::iterator iTrack = fTracksTrueBest.begin(); iTrack != fTracksTrueBest.end(); ++iTrack) {
       Int_t iTr = iTrack->second;
	quality[iTr] = 5;
    }
    // true candidate = 4 --> BLUE
    for (map<Double_t,Int_t>::iterator iTrack = fTracksTrueCandidate.begin(); iTrack != fTracksTrueCandidate.end(); ++iTrack) {
       Int_t iTr = iTrack->second;
       quality[iTr] = 4;
    }
    // true candidate best = 3 --> CYAN
    for (map<Double_t,Int_t>::iterator iTrack = fTracksTrueCandidateBest.begin(); iTrack != fTracksTrueCandidateBest.end(); ++iTrack) {
       Int_t iTr = iTrack->second;
       quality[iTr] = 3;
    }
    // candidate = 2 --> BLACK
    for (map<Double_t,Int_t>::iterator iTrack = fTracksCandidate.begin(); iTrack != fTracksCandidate.end(); ++iTrack) {
       Int_t iTr = iTrack->second;
       quality[iTr] = 2;
    }
    // candidate best = 1 --> GRAY
    for (map<Double_t,Int_t>::iterator iTrack = fTracksCandidateBest.begin(); iTrack != fTracksCandidateBest.end(); ++iTrack) {
       Int_t iTr = iTrack->second;
       quality[iTr] = 1;
    }

    for (Int_t iTrk = 0; iTrk < fNTrack; iTrk++) {
      if (fVerbose>1)
	cout << "track " << iTrk << " using intersection " << fTracks[iTrk].GetIinter(0) << " on MWPC0 (" << xtrk[0][iTrk] << ", " << ytrk[0][iTrk] << ") and intersection " << fTracks[iTrk].GetIinter(1) << " on MWPC1 (" << xtrk[1][iTrk] << ", " << ytrk[1][iTrk] << ")" << endl; 

      trkl[iTrk] = new TLine(xtrk[0][iTrk], ytrk[0][iTrk], xtrk[1][iTrk], ytrk[1][iTrk]);
      trkl[iTrk]->SetLineWidth(2);
      c->cd(); trkl[iTrk]->Draw("same");
      
      // yz plane
      trkl_yz[iTrk] = new TLine(ztrk[0][iTrk], ytrk[0][iTrk], ztrk[1][iTrk], ytrk[1][iTrk]);
      trkl_yz[iTrk]->SetLineWidth(2);
      c2->cd(); trkl_yz[iTrk]->Draw("same");
      
      switch (quality[iTrk]) {
      case 6:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": TRUE TRACK!!! [red]"  << endl;
	trkl[iTrk]->SetLineColor(kRed);
	trkl_yz[iTrk]->SetLineColor(kRed);
	break;
      case 5:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": BEST TRUE TRACK!!! [violet]"  << endl;
	trkl[iTrk]->SetLineColor(kViolet);
	trkl_yz[iTrk]->SetLineColor(kViolet);
	break;
      case 4:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": TRUE CANDIDATE TRACK!!! [blue]"  << endl;
	trkl[iTrk]->SetLineColor(kBlue);
	trkl_yz[iTrk]->SetLineColor(kBlue);
	break;
      case 3:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": BEST TRUE CANDIDATE TRACK!!! [cyan]"  << endl;
	trkl[iTrk]->SetLineColor(kCyan);
	trkl_yz[iTrk]->SetLineColor(kCyan);
	break;
      case 2:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": CANDIDATE TRACK!!! [black]"  << endl;
	trkl[iTrk]->SetLineColor(kBlack);
	trkl_yz[iTrk]->SetLineColor(kBlack);
	break;
      case 1:
	if (fVerbose>0)
	  cout << "==> track # " << iTrk << ": BEST CANDIDATE TRACK!!! [gray]"  << endl;
	trkl[iTrk]->SetLineColor(kGray+2);
	trkl_yz[iTrk]->SetLineColor(kGray+2);
	break;
      }
      
    } // end track loop

    c->Update();    c->Modified();
    c2->Update();   c2->Modified();
    h->Delete();    h2->Delete();
  }
  
  // Wait?
  if (fWait) {
    cout << endl;
    cout << "Press any key to continue / q to quit " << endl;
    if (cin.get()=='q') gROOT->ProcessLine(".q");
  }
  
  // Reset fWait
  fWait = kFALSE;
  
}

//________________________________________________________________________________________
void TA2CylMwpc::InitGeometry()
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
      Double_t xCB[4] = {dCB[i-1].X(), uCB[i-1].X(), uCB[i].X(), dCB[i].X()};
      Double_t yCB[4] = {dCB[i-1].Y(), uCB[i-1].Y(), uCB[i].Y(), dCB[i].Y()};
      TPolyLine *contourCB = new TPolyLine(4, xCB, yCB);
      contourCB->SetLineColor(kGreen);
      contourCB->SetFillStyle(0);
      contourCB->Draw("f");
      contourCB->Draw("same");
    }
  }
    
  // mwpcs
  TArc *mwpc1 = new TArc(0., 0., fR[0]);
  mwpc1->SetLineColor(kAzure+7);
  mwpc1->SetFillStyle(0);
  mwpc1->Draw("same");
  TArc *mwpc2 = new TArc(0., 0., fR[1]);
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
      TPolyLine *contour = new TPolyLine(5, x, y);
      contour->Draw("f");
      contour->Draw("same");
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
    Double_t xdCB_yz = TMath::Sin(phi*kDegToRad)*innerCB;
    Double_t ydCB_yz = TMath::Cos(phi*kDegToRad)*innerCB;
    if (TMath::Abs(xdCB_yz) < 1e-10) xdCB_yz = 0.;
    if (TMath::Abs(ydCB_yz) < 1e-10) ydCB_yz = 0.;
    dCB_yz[i].Set(xdCB_yz, ydCB_yz);
    Double_t xuCB_yz = TMath::Sin(phi*kDegToRad)*outerCB_yz;
    Double_t yuCB_yz = TMath::Cos(phi*kDegToRad)*outerCB_yz;
    if (TMath::Abs(xuCB_yz) < 1e-10) xuCB_yz = 0.;
    if (TMath::Abs(yuCB_yz) < 1e-10) yuCB_yz = 0.;
    uCB_yz[i].Set(xuCB_yz, yuCB_yz);
    if (i>0) {
      Double_t xCB_yz[4] = {dCB_yz[i-1].X(), uCB_yz[i-1].X(), uCB_yz[i].X(), dCB_yz[i].X()};
      Double_t yCB_yz[4] = {dCB_yz[i-1].Y(), uCB_yz[i-1].Y(), uCB_yz[i].Y(), dCB_yz[i].Y()};
      TPolyLine *contourCB_yz = new TPolyLine(4, xCB_yz, yCB_yz);
      contourCB_yz->SetLineColor(kGreen);
      contourCB_yz->SetFillStyle(0);
      contourCB_yz->Draw("f");
      contourCB_yz->Draw("same");

      Double_t yCB2_yz[4] = {-dCB_yz[i-1].Y(), -uCB_yz[i-1].Y(), -uCB_yz[i].Y(), -dCB_yz[i].Y()};
      TPolyLine *contourCB2_yz = new TPolyLine(4, xCB_yz, yCB2_yz);
      contourCB2_yz->SetLineColor(kGreen);
      contourCB2_yz->SetFillStyle(0);
      contourCB2_yz->Draw("f");
      contourCB2_yz->Draw("same");
    }
  }

  // MWPC
  Double_t xm[5] = {-MWPClength/2., -MWPClength/2., MWPClength/2., MWPClength/2., -MWPClength/2.};
  Double_t ym[5] = {-fR[1], fR[1], fR[1], -fR[1], -fR[1]};
  TPolyLine *mwpc_yz = new TPolyLine(5, xm, ym);
  mwpc_yz->SetFillStyle(0);
  mwpc_yz->SetLineColor(kBlue);
  mwpc_yz->Draw("same");
  TLine *mwpcup_yz = new TLine(-MWPClength/2., fR[0], MWPClength/2., fR[0]);
  mwpcup_yz->SetLineStyle(7);
  mwpcup_yz->SetLineColor(kAzure+7);
  mwpcup_yz->Draw("same");
  TLine *mwpcdown_yz = new TLine(-MWPClength/2., -fR[0], MWPClength/2., -fR[0]);
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
  TPolyLine *target_xz = new TPolyLine(5, xt, yt);
  target_xz->SetLineColor(kRed);
  target_xz->SetFillColor(kRed);
  target_xz->SetFillStyle(3013);
  target_xz->Draw("f");
  target_xz->Draw("same");
  
  origin->Draw("same");

  c2->Update();
  c2->Modified();

}

ClassImp(TA2CylMwpc)
