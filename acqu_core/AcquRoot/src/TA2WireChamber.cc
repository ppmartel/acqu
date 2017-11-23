//--Author	JRM Annand... 8th Jul 2004  AcquRoot C++ class
//--Rev 	JRM Annand... 7th Sep 2004  alpha-production version
//--Rev 	JRM Annand...15th Mar 2005  GetNTrack, GetMaxTrack
//--Rev 	JRM Annand...25th Jul 2005  SetConfig hierarchy
//--Update	JRM Annand... 1st Sep 2009  varname, delete[]
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2WireChamber
//
// General purpose framework of decoding and calibration methods for 
// position sensitive multi-wire gas counters.
// The wire chamber typically consists of n separate chambers
// each chamber having L separate layers.
//

#include "TA2WireChamber.h"


// constants for command-line maps below
enum { 
  ENWCLayer = 200, ENWCChamber, ENWCLayersInChamber, EWCChamberParm,
  EWCTypePlaneWire, EWCTypeCylWire, EWCTypePlaneDrift,
  EWCTypePlaneStrip, EWCTypeCylStrip,
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
  {NULL,          -1}
};

//---------------------------------------------------------------------------
TA2WireChamber::TA2WireChamber( const char* name, TA2System* apparatus )
  :TA2Detector(name, apparatus)
{
  // Basic primary setup in TA2Detector
  AddCmdList( kWCKeys );
  fNLayer = fNlayer = fNchamber = fNChamber = fMaxIntersect = 
    fMaxTrack = fNTrack = fMaxVertex = fNVertex = 0;
  fChamberLayers = NULL;
  fWCLayer = NULL;
  fVertex = NULL;
  fVertexLimits = NULL;
  fTrack = NULL;
  fTrackTheta = NULL;
  fTrackPhi = NULL;
  fPi = TMath::Pi();
  f2Pi = TMath::TwoPi();
}

//---------------------------------------------------------------------------
TA2WireChamber::~TA2WireChamber()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  DeleteArrays();
}

//---------------------------------------------------------------------------
void TA2WireChamber::DeleteArrays()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  TA2Detector::DeleteArrays();
  for( Int_t i=0; i<fNlayer; i++ ) delete fWCLayer[i];
  delete[] fWCLayer;
  for( Int_t i=0; i<fNChamber; i++ ) delete fChamberLayers[i];
  delete[] fChamberLayers;
}

//---------------------------------------------------------------------------
void TA2WireChamber::SaveDecoded( )
{
  // Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------
void TA2WireChamber::SetConfig( char* line, int key )
{
  // Load config parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kWCKeys array at the top of the source .cc file


  Int_t n, nelem, maxcl, maxclsize;
  Int_t layer[8];
  Double_t dparm[16];
  Char_t name[64];

  // Wire-chamber specific configuration
  switch( key ){
  case ENWCLayer:
    // no. of active layers (or planes) in chamber
    if( sscanf( line, "%d", &fNLayer ) < 1 ) goto error;
    fWCLayer = new TA2WCLayer*[fNLayer];
    break;
  case ENWCChamber:
    // no. of chambers in detector
    if( sscanf( line, "%d", &fNChamber ) < 1 ) goto error;
    fChamberLayers = new Int_t*[fNChamber];
    break;
  case ENWCLayersInChamber:
    // layers in particular chamber
    if( fNchamber >= fNChamber ){
      PrintError( line, "<Too many WC layers-in-chamber input>");
      return;
    }
    if( ( n = sscanf( line, "%d%d%d%d%d%d%d%d",
		      layer,layer+1,layer+2,layer+3,
		      layer+4,layer+5,layer+6,layer+7 ) ) < 1 ) goto error;
    fChamberLayers[fNchamber]  = new Int_t[n+1];
    fChamberLayers[fNchamber][0]  = n;
    for( Int_t i=1; i<=n; i++ ) fChamberLayers[fNchamber][i] = layer[i-1];
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
    // flat-plane MWPC wire
    break;
  case EWCTypeCylWire: 
    // Wires arranged in cylinder
    // Has hit TDC, but no pulse height
    // layer parameters.....
    // radius, length,
    // quadratic z correction  coeff 0,1,2
    if( fNlayer >= fNLayer ){
      PrintError( line, "<Too many WC layers input>");
      return;
    }
    fIsEnergy = EFalse;
    fIsTime = fIsPos = ETrue;
    if( sscanf( line, "%s%d%d%d%lf%lf%lf%lf%lf",
		name, &nelem, &maxcl, &maxclsize, dparm, dparm+1,
		dparm+2, dparm+3, dparm+4 ) < 9 ) goto error;
    fWCLayer[fNlayer] = new TA2CylWire( name, nelem, maxcl, maxclsize, 
					this, dparm );
    fNlayer++;
    break;
  case EWCTypePlaneDrift:
    // flat-plane drift wires
    break;
  case EWCTypePlaneStrip:
    // flat-plane cathode strip
    break;
  case EWCTypeCylStrip:
    // Cathode strips helically "wound" on cylinder
    // Has pulse height but no time
    // layer parameters.....
    // radius, length, TgWC, Z0, pitch,
    // quadratic z correction  coeff 0,1,2
    // quadratic phi correction coeff 0,1,2
    if( fNlayer >= fNLayer ){
      PrintError( line, "<Too many WC layers input>");
      return;
    }
    fIsEnergy = fIsPos = ETrue;
    fIsTime  = EFalse;
    if( sscanf( line, "%s%d%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		name, &nelem, &maxcl, &maxclsize,
		dparm, dparm+1, dparm+2, dparm+3, dparm+4,
		dparm+5, dparm+6, dparm+7,
		dparm+8, dparm+9, dparm+10 ) < 15 ) goto error;
    fWCLayer[fNlayer] = new TA2CylStrip( name, nelem, maxcl, maxclsize, 
					this, dparm );
    fNlayer++;
    break;
  default:
    // try standard detector config
    TA2Detector::SetConfig( line, key );
    break;;
  }
  return;
 error: PrintError( line, "<WC configuration>" );
  return;
}

//-----------------------------------------------------------------------------
void TA2WireChamber::LoadVariable( )
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

  // Specialised chamber diagnostic stuff
  //  Char_t name[256];
  Char_t* varname;
  //  Char_t* endname;
  // standard detector stuff
  TA2Detector::LoadVariable();
  // specialist wire chamber stuff
  for(Int_t i=0; i<fNlayer; i++){
    varname = BuildName( fWCLayer[i]->GetName(),"Hits");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetLayerHits(),
				 EIMultiX);
    // Cluster "C-of-G"
    varname = BuildName( fWCLayer[i]->GetName(),"CGClust");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetCGClust(),
				 EDMultiX);
    // Start index of cluster
    varname = BuildName( fWCLayer[i]->GetName(),"StClust");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetClust(),
				 EIMultiX);
    // Length of cluster
    varname = BuildName( fWCLayer[i]->GetName(),"LenClust");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetLenClust(),
				 EIMultiX);
    // # hits in layer
    varname = BuildName( fWCLayer[i]->GetName(),"NHits");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetNHitsPtr(),
				 EISingleX);
    // # clusters
    varname = BuildName( fWCLayer[i]->GetName(),"NClust");
    TA2DataManager::LoadVariable(varname, fWCLayer[i]->GetNClustPtr(),
				 EISingleX);
  }
  TA2DataManager::LoadVariable("NTrack", &fNTrack, EISingleX);
  TA2DataManager::LoadVariable("NVertex", &fNVertex, EISingleX);
  TA2DataManager::LoadVariable("TrackTheta", fTrackTheta, EDMultiX);
  TA2DataManager::LoadVariable("TrackPhi", fTrackPhi, EDMultiX);
}

ClassImp(TA2WireChamber)
