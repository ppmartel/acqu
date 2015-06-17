//--Author      JRM Annand   30th Sep 2003
//--Rev         JRM Annand...15th Oct 2003  ReadDecoded...MC data
//--Update      JRM Annand    5th Feb 2004  3v8 compatibility
//--Update        R Gregor    3th Jul 2005  Added veto and pulsshape reconstruction
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2TAPS_BaF2
//
// Decoding and calibration methods for TAPS array of BaF2
// Configured as forward wall to work in conjunction with the CB
//

#include "TA2TAPS_BaF2.h"
#include "HitClusterTAPS_t.h"
#include <string>
#include <sstream>


#include <TH2TAPS.h>


// constants for command-line maps below
enum {
  ETAPSSGMaxDet=510, ETAPSSG, EClustDetMaxTAPSCluster, EClustDetTAPSNeighbour,
  ETAPSEnergyResolution, ETAPSTimeResolution, ETAPSThetaResolution, ETAPSPhiResolution,
  ETAPSVetoEnergy, ETAPSVetoEfficiency, ETAPSVetoThreshold, ETAPSOffsetTime
};

// Command-line key words which determine what to read in
static const Map_t kTAPSClustDetKeys[] = {
  {"TAPSSGMaxDet:",        ETAPSSGMaxDet},
  {"TAPSSG:",              ETAPSSG},
  {"Max-TAPSCluster:",     EClustDetMaxTAPSCluster},
  {"Next-TAPS:",           EClustDetTAPSNeighbour},
  {"Energy-Resolution:",   ETAPSEnergyResolution},
  {"Time-Resolution:",     ETAPSTimeResolution},
  {"Theta-Resolution:",    ETAPSThetaResolution},
  {"Phi-Resolution:",      ETAPSPhiResolution},
  {"Veto-Energy:",         ETAPSVetoEnergy},
  {"Veto-Efficiency:",     ETAPSVetoEfficiency},
  {"Veto-Threshold:",      ETAPSVetoThreshold},
  {"Offset-Time:",         ETAPSOffsetTime},
  {NULL,          -1}
};


//---------------------------------------------------------------------------

TA2TAPS_BaF2::TA2TAPS_BaF2(const char* name, TA2System* apparatus)
             :TA2ClusterDetector(name, apparatus)
{
  fType = ENoType;

  fUseEnergyResolution    = 0;
  fUseTimeResolution      = 0;
  fEnergyResolutionFactor = -1.0;
  fEnergyResolutionConst  = -1.0;
  fTimeResolution         = -1.0;
  fTimeOffset             = 0.0;
  fThetaResolution        = -1.0;
  fPhiResolution          = -1.0;

  fVetoThreshold = 0.0;
  for(Int_t t=0; t<5; t++)
  {
    fVetoEfficiency[t] = 1.0;
    fVetoEnergy[t] = 100.0*t;
  }

  fSGEnergy = NULL;
  fLGEnergy = NULL;
  fMaxSGElements = 0;


  // defined in base class TA2ClusterDetector
  std::string s_name(GetName());
  std::string s_all = s_name + "_ClustersAll";
  fDispClusterHitsAll = new TH2TAPS(s_all, s_all);
  std::string s_energy = s_name + "_ClustersEnergy";
  fDispClusterHitsEnergy = new TH2TAPS(s_energy, s_energy);  
  fDispClusterHitsSingle = new TH2Crystals*[MAX_DISP_CLUSTERS];
  for(int i=0;i<MAX_DISP_CLUSTERS;i++) {
    std::stringstream s_single; 
    s_single << s_name << "_ClustersSingle_" << i;
    fDispClusterHitsSingle[i] = new TH2TAPS(s_single.str(), s_single.str());
  }


  // Do not allocate any "new" memory here...Root will wipe
  // Set private variables to zero/false/undefined state
  AddCmdList( kTAPSClustDetKeys );
}

//---------------------------------------------------------------------------

TA2TAPS_BaF2::~TA2TAPS_BaF2()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  for(UInt_t i=0; i<fMaxSGElements; i++)
    if(fSGEnergy[i]) delete fSGEnergy[i];

  delete fSGEnergy;
  DeleteClusterArrays();
}

//---------------------------------------------------------------------------

void TA2TAPS_BaF2::SaveDecoded()
{
  // Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------

void TA2TAPS_BaF2::SetConfig(Char_t* line, Int_t key)
{
  // Default detector configuration
  // If this flags it has done nothing carry on with local setup
  //TA2ClusterDetector::SetConfig( line, key );
  //if( !fIsConfigPass ) return;
  //fIsConfigPass = EFalse;           // reset for any inherited SetConfig

  // Own special cluster setup
  // Cluster specific configuration
  switch(key)
  {
   case ETAPSEnergyResolution:
    // Energy Resolution Read-in Line
    if(sscanf(line, "%lf%lf%d", &fEnergyResolutionFactor, &fEnergyResolutionConst, &fUseEnergyResolution) < 3)
      PrintError(line,"<TA2TAPS_BaF2 Energy Resolution>");
    break;
   case ETAPSTimeResolution:
    // Time resolution read-in line
    if(sscanf(line, "%lf%d", &fTimeResolution, &fUseTimeResolution) < 2)
      PrintError(line,"<TA2TAPS_BaF2 Time Resolution>");
    break;
   case ETAPSThetaResolution:
    // Theta resolution read-in line
    if(sscanf(line, "%lf", &fThetaResolution) < 1)
      PrintError(line,"<TA2TAPS_BaF2 Theta Resolution>");
    break;
   case ETAPSPhiResolution:
    // Phi resolution read-in line
    if(sscanf(line, "%lf", &fPhiResolution) < 1)
      PrintError(line,"<TA2TAPS_BaF2 Phi Resolution>");
    break;

   case ETAPSVetoThreshold:
    if(sscanf(line, "%lf", &fVetoThreshold) < 1)
      PrintError(line,"<TA2TAPS_BaF2 Veto Threshold>");
   case ETAPSVetoEfficiency:
    if(sscanf(line, "%lf%lf%lf%lf%lf", &fVetoEfficiency[0], &fVetoEfficiency[1], &fVetoEfficiency[2], &fVetoEfficiency[3], &fVetoEfficiency[4]) < 5)
      PrintError(line,"<TA2TAPS_BaF2 Veto Efficiency>");
    break;
   case ETAPSVetoEnergy:
    if(sscanf(line, "%lf%lf%lf%lf", &fVetoEnergy[0], &fVetoEnergy[1], &fVetoEnergy[2], &fVetoEnergy[3]) < 4)
      PrintError(line,"<TA2TAPS_BaF2 Veto Energy>");
    break;

   case ETAPSSGMaxDet:
    //Max number of BaF2-SG-clusters
    if(sscanf(line, "%d", &fMaxSGElements) < 1) goto error;
    fSGEnergy = new HitD2A_t*[fMaxSGElements+1];
    fSGEnergyValue = new Double_t[fMaxSGElements+1];
    fNSG = 0;
    break;
   case ETAPSSG:
    //SG-Pedestal and Gain of BaF2-Crystals
    // parameters for each element...need to input # of elements 1st
    if( fNSG>=fMaxSGElements )
    {
      PrintError(line,"<Too many detector elements input>");
      break;
    }
    // last kTRUE signals to ignore the given positions
    // workaround for this pretty ugly design feature...
    fSGEnergy[fNSG] = new HitD2A_t( line, fNSG, this, kTRUE );
    fNSG++;
    break;
   case EClustDetMaxTAPSCluster:
    // Max number of clusters
    if(sscanf(line, "%d%lf", &fMaxCluster, &fClEthresh) < 1) goto error;
    fEthresh = fClEthresh;
    fCluster = new HitCluster_t*[fNelement+1];
    fprintf(fLogStream, " HitClusterTAPS_t class used for shower reconstruction\n");
    fClustHit = new UInt_t[fMaxCluster+1];
    fTempHits = new UInt_t[fNelement+1];
    fNClustHitOR = new UInt_t[fNelement+1];
    fTheta = new Double_t[fNelement+1];
    fPhi      = new Double_t[fNelement+1];
    fClEnergyOR  = new Double_t[fNelement+1];
    fClTimeOR  = new Double_t[fNelement+1];
    fClCentFracOR  = new Double_t[fNelement+1];
    fClRadiusOR  = new Double_t[fNelement+1];
    fNCluster = 0;
    break;
 case EClustDetTAPSNeighbour:
    // Nearest neighbout input
    if(fNCluster < fNelement) fCluster[fNCluster] = new HitClusterTAPS_t(line, fNCluster);
    fNCluster++;
    break;
  case ETAPSOffsetTime:
    // Time offset read-in line
    if(sscanf(line, "%lf", &fOffsetTime) < 1)
      PrintError(line,"<TA2TAPS_BaF2 Timeoffset>");
    break;
  default:
    // Command not found...possible pass to next config
    TA2ClusterDetector::SetConfig(line, key);
    fIsConfigPass = ETrue;
    break;
  }
  return;
 error: PrintError(line);
  return;
}

//---------------------------------------------------------------------------

void TA2TAPS_BaF2::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Start with alignment offsets
  // Create space for various output arrays

    // init the type
    switch (GetNelement())
    {
        case 384:
            fType = EBaF2;
            break;
        case 402:
            fType = EBaF2_PWO_08;
            break;
        case 438:
            fType = EBaF2_PWO_09;
            break;
        default:
            fType = ENoType;
            break;
    }

  pRandoms = new TRandom3;
  pRandoms->SetSeed(0); //'Randomize Timer'

  EnergyAll = new Double_t[fNelem];
  theta_c = new Double_t[fNelem];
  phi_c = new Double_t[fNelem];
  x_c = new Double_t[fNelem];
  y_c = new Double_t[fNelem];
  z_c = new Double_t[fNelem];
  Z_c = new Double_t[fNelem];

  TVector3 direction;
  for(UInt_t t=0; t<fNelem; t++)
  {
    direction = fPosition[t]->Unit();
    x_c[t] = -direction.X(); //X position patch in G3 simulation
    y_c[t] = direction.Y();
    z_c[t] = direction.Z();
    Z_c[t] = fPosition[t]->Z();
    theta_c[t] = acos(z_c[t]);
    phi_c[t] = atan2(y_c[t], x_c[t]);
  }

  bSimul = false;

  TA2ClusterDetector::PostInit();
  fLGEnergy = TA2Detector::GetElement();
}

//-----------------------------------------------------------------------------

ClassImp(TA2TAPS_BaF2)
