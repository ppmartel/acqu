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

#include "TA2CalArray.h"
#include <string>
#include <sstream>

#include <TH2CB.h>

enum
{
  ECAEnergyResolution=1900, ECATimeResolution, ECAThetaResolution, ECAPhiResolution,
  ECAScaleFile
};

static const Map_t kCalArrayKeys[] =
{
  {"Energy-Resolution:",   ECAEnergyResolution},
  {"Time-Resolution:",     ECATimeResolution},
  {"Theta-Resolution:",    ECAThetaResolution},
  {"Phi-Resolution:",      ECAPhiResolution},
  {"Max-Cluster:",         EClustDetMaxCluster},
  {"Next-Neighbour:",      EClustDetNeighbour},
  {"Scale-File:",          ECAScaleFile},
  {NULL,            -1}
};


//---------------------------------------------------------------------------
TA2CalArray::TA2CalArray(const char* name, TA2System* apparatus)
            :TA2ClusterDetector(name, apparatus)
{
  // Do not allocate any "new" memory here...Root will wipe
  // Set private variables to zero/false/undefined state
  // Pass kLaddKeys (command-control keywords) and
  // kLaddHist (names of histograms) to progenitor classes

  fClustAlgoType = EClustAlgoTrad;

  fUseSigmaEnergy       = 0;
  fUseSigmaTime         = 0;
  fSigmaEnergyFactor    = -1.0;
  fSigmaEnergyPower     = -1.0;
  fSigmaTime            = -1.0;
  fSigmaTheta           = -1.0;
  fSigmaPhi             = -1.0;
  fEthresh              = 0.0;

  ScaleRuns = 0;
  ScaleVal[0] = 1.0;
  UseScales = false;
  CurrentRun[0] = '\0';

  fRandom = new TRandom();

  // defined in base class TA2ClusterDetector
  std::string s_name(GetName());
  std::string s_all = s_name + "_ClustersAll";
  fDispClusterHitsAll = new TH2CB(s_all, s_all);
  std::string s_energy = s_name + "_ClustersEnergy";
  fDispClusterHitsEnergy = new TH2CB(s_energy, s_energy);
  fDispClusterHitsSingle = new TH2Crystals*[MAX_DISP_CLUSTERS];
  for(int i=0;i<MAX_DISP_CLUSTERS;i++) {
    std::stringstream s_single; 
    s_single << s_name << "_ClustersSingle_" << i;
    fDispClusterHitsSingle[i] = new TH2CB(s_single.str(), s_single.str());
  }


  AddCmdList(kCalArrayKeys);                  // for SetConfig()
}

//---------------------------------------------------------------------------

TA2CalArray::~TA2CalArray()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  // Start with common arrays of TA2Detector class
  DeleteClusterArrays();
}

//---------------------------------------------------------------------------

void TA2CalArray::SetConfig(char* line, int key)
{
  FILE* ScalFile;

  // Load CalArray parameters from file or command line
  // CalArray specific configuration
  switch(key)
  {
  case ECAScaleFile:
    if(sscanf(line, "%s", ScaleFile) < 1)
    {
      PrintError(line,"<TA2CalArray Scale File>");
      break;
    }
    printf("NaI energy scale values from:\n %s\n", ScaleFile);
    ScalFile = fopen(ScaleFile, "r");
    ScaleRuns = 0;
    while(!feof(ScalFile))
      if(fscanf(ScalFile, "%s %lf", ScaleRun[ScaleRuns], &ScaleVal[ScaleRuns])==2)
      {
        ScaleRuns++;
        if(ScaleRuns > MAXRUNS) break;
      }
    fclose(ScalFile);
    UseScales = true;
    break;
  case ECAEnergyResolution:
    // Energy Resolution Read-in Line
    if(sscanf(line, "%lf%lf%d", &fSigmaEnergyFactor, &fSigmaEnergyPower, &fUseSigmaEnergy) < 3)
      PrintError(line,"<TA2CalArray Energy Resolution>");
    break;
  case ECATimeResolution:
    // Time resolution read-in line
    if(sscanf(line, "%lf%d", &fSigmaTime, &fUseSigmaTime) < 2)
      PrintError(line,"<TA2CalArray Time Resolution>");
    break;
  case ECAThetaResolution:
    // Time resolution read-in line
    if(sscanf(line, "%lf", &fSigmaTheta) < 1)
      PrintError(line,"<TA2CalArray Theta Resolution>");
    break;
  case ECAPhiResolution:
    // Time resolution read-in line
    if(sscanf(line, "%lf", &fSigmaPhi) < 1)
      PrintError(line,"<TA2CalArray Phi Resolution>");
    break;
  default:
    // Command not found...possible pass to next config
    TA2ClusterDetector::SetConfig(line, key);
    break;
  }
  return;
}

//-----------------------------------------------------------------------------

void TA2CalArray::ParseDisplay(char* line)
{
  // Input private histogram spec to general purpose parse
  // and setup routine

  //  const Name2Variable_t hist[] = {
  // Name          ->variable  single/mult    Fill-condition
  //    {"Nphoton_Minv", fM_Nphoton, EHistSingleX},
  //    {NULL,          0,         0}
  //  };
  // Do not remove the final NULL line

  TA2ClusterDetector::ParseDisplay(line);
  return;
}

//---------------------------------------------------------------------------

void TA2CalArray::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Start with alignment offsets
  // Create space for various output arrays

  fEnergyAll = new Double_t[fNelem+1];

  //Store global energy scale value
  fEnergyGlobal = fEnergyScale;

  TA2ClusterDetector::PostInit();
}

//---------------------------------------------------------------------------

void TA2CalArray::SaveDecoded()
{
  // Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------

ClassImp(TA2CalArray)
