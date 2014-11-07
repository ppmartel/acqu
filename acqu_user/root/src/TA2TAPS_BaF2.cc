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
#include <string>
#include <sstream>


#include <TH2TAPS.h>


// constants for command-line maps below
enum {
  ETAPSSGMaxDet=510, ETAPSSG, 
  ETAPSEnergyResolution, ETAPSTimeResolution, ETAPSThetaResolution, ETAPSPhiResolution,
  ETAPSVetoEnergy, ETAPSVetoEfficiency, ETAPSVetoThreshold
};

// Command-line key words which determine what to read in
static const Map_t kTAPSClustDetKeys[] = {
  {"TAPSSGMaxDet:",        ETAPSSGMaxDet},
  {"TAPSSG:",              ETAPSSG},
  {"Energy-Resolution:",   ETAPSEnergyResolution},
  {"Time-Resolution:",     ETAPSTimeResolution},
  {"Theta-Resolution:",    ETAPSThetaResolution},
  {"Phi-Resolution:",      ETAPSPhiResolution},
  {"Veto-Energy:",         ETAPSVetoEnergy},
  {"Veto-Efficiency:",     ETAPSVetoEfficiency},
  {"Veto-Threshold:",      ETAPSVetoThreshold},
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
    // Time resolution read-in line
    if(sscanf(line, "%lf", &fThetaResolution) < 1)
      PrintError(line,"<TA2TAPS_BaF2 Theta Resolution>");
    break;
   case ETAPSPhiResolution:
    // Time resolution read-in line
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

//---------------------------------------------------------------------------

inline void TA2TAPS_BaF2::ReadDecoded()
{
  // Read back "decoded" data for the BaF2 array
  // In this case produced by the GEANT3 CB simulation

  UInt_t j;
  Int_t nVetos = 0;
  Int_t nBaF2s = 0;
  Double_t total = 0.0;
  fNhits = *(Int_t*)(fEvent[EI_ntaps]);
  Float_t* energy = (Float_t*)(fEvent[EI_ectapsl]);
  Float_t* time = (Float_t*)(fEvent[EI_tctaps]);
  UInt_t* index = (UInt_t*)(fEvent[EI_ictaps]);
  Float_t* venergy = (Float_t*)(fEvent[EI_evtaps]);
  Float_t* dircos = (Float_t*)(fEvent[EI_dircos]);
  Int_t npart = *(Int_t*)(fEvent[EI_npart]);
  Int_t* idpart = (Int_t*)(fEvent[EI_idpart]);
  Double_t E, T;                                    //Energy, time
  Double_t LoE, HiE, LoT, HiT;
  Int_t proton;
  Double_t dphi, dtheta;
  Double_t domega2, temp2;
  UInt_t crystal;
  Double_t Efficiency = 0.0;
  Double_t GammaToF;
  Bool_t IsBaF2;

  //Find proton index in simulated data
  for(proton=0; proton<npart; proton++)
    if(idpart[proton]==14) break; //14 = GEANT proton
  //Move pointer to correct positions for proton data
  for(Int_t t=0; t<proton; t++) //For all particles before proton:
    for(Int_t i=0;i<3; i++) dircos++; //Jump over x,y,z components
  //Read x,y,z components for proton
  Double_t x_p = *dircos++;
  Double_t y_p = *dircos++;
  Double_t z_p = *dircos;
  //Calculate theta and phi angles out of cartesian components
  Double_t theta_p = acos(z_p);
  Double_t phi_p = atan2(y_p, x_p);

  //For each crystal: evaluate deviation from proton direction in theta-phi plane
  domega2 = 1e38;
  crystal = 65535;
  if(theta_p < (TMath::DegToRad()*20.0))
    for(UInt_t t=0; t<fNelem; t++)
    {
      dtheta = theta_p - theta_c[t];
      dphi = phi_p - phi_c[t];
      if(dphi > TMath::Pi()) dphi = TMath::TwoPi() - dphi;
      if(dphi < -TMath::Pi()) dphi = -TMath::TwoPi() - dphi;
       //Find crystal which fits best for proton direction in theta-phi plane
      temp2 = dphi*dphi + dtheta*dtheta;
      if(temp2 < domega2)
      {
        domega2 = temp2;
        crystal = t;
      }
    }

  //I want this empty...
  if(fMaxSGElements)
    for(UInt_t t=0; t<fMaxSGElements; t++)
      fSGEnergyValue[t] = EBufferEnd;
  for(UInt_t t=0; t<fNelem; t++)
  {
    if(fPatternHits) fPatternHits[0][t] = EBufferEnd;
    EnergyAll[t] = 0.0;
  }

  //Evaluate all simulated hits
  for(UInt_t i=0; i<fNhits; i++)
  {
    j = index[i];
    if(!j) break;
    //Use following line for cbsim since 2006-06-22!
    j--; //For reasons Stefan won't tell...
    E = energy[i] * fEnergyScale;                                   //G3/4 output in GeV
//    if(fUseEnergyResolution) E+=pRandoms->Gaus(0.0, GetEnergyResolutionGeV(E));
    if(fUseEnergyResolution) E+=pRandoms->Gaus(0.0, GetSigmaEnergyGeV(E));
    E*=1000.0;                                                      //G3/4 output in MeV
    EnergyAll[j] = E;
    GammaToF = (Z_c[j] * TMath::Cos(theta_c[j]))/30.0;
    T = -time[i] + GammaToF + pRandoms->Gaus(0.0, fTimeResolution);

   //Fill veto stuff...
    if(fPatternHits) //...but only if bit pattern unit is active
      if((venergy[i] * 1000.0) > fVetoThreshold) //If energy deposited: Veto Hit
      {
        if(E < fVetoEnergy[0]) Efficiency = fVetoEfficiency[0];
        else if((E > fVetoEnergy[0]) && (E < fVetoEnergy[1])) Efficiency = fVetoEfficiency[1];
        else if((E > fVetoEnergy[1]) && (E < fVetoEnergy[2])) Efficiency = fVetoEfficiency[2];
        else if((E > fVetoEnergy[2]) && (E < fVetoEnergy[3])) Efficiency = fVetoEfficiency[3];
        else if(E > fVetoEnergy[3]) Efficiency = fVetoEfficiency[4];
        if(pRandoms->Rndm() < Efficiency)
        {
          fPatternHits[0][nVetos] = j;
         nVetos++;
        }
      }

    //Fill BaF2 stuff
    LoE = fElement[j]->GetEnergyLowThr();
    HiE = fElement[j]->GetEnergyHighThr();
    LoT = fElement[j]->GetTimeLowThr();
    HiT = fElement[j]->GetTimeHighThr();
    if((E > LoE) && (E < HiE) && (T > LoT) && (T < HiT))
    {
      fEnergy[j] = E;
      fTime[j] = T;
      fEnergyOR[nBaF2s] = E;
      fTimeOR[nBaF2s] = T;
      fHits[nBaF2s] = j;
      total+=E;
      nBaF2s++;

      //PSA - reasonable?
      if(fMaxSGElements)
      {
        IsBaF2 = true;
        if((fNelem==402) && ((j % 67) <  4)) IsBaF2 = false; //If 1 PbWO ring is installed, the first 4 crystals in a TAPS block are PbWO4s
        if((fNelem==438) && ((j % 73) < 12)) IsBaF2 = false; //If 2 PbWO rings are installed, the first 12 crystals in a TAPS block are PbWO4s

        if(IsBaF2) //Do PSA information only for BaF2s
        {
          if((j==crystal) || fCluster[j]->IsNeighbour(crystal)) //Hit in or near crystal best fitting for proton
            //fSGEnergyValue[j] = E * TMath::Power(E, 0.05) * 0.65; //Sven's function
            fSGEnergyValue[j] = -0.9691614 + 0.7108238*E + 0.1819194e-2*E*E - 0.9019634e-5*E*E*E + 0.1610750e-7*E*E*E*E; //Marc's function
          else
            fSGEnergyValue[j] = 0.96 * E;
          fSGEnergyValue[j]+=pRandoms->Gaus(0.0, 0.25 * TMath::Sqrt(fSGEnergyValue[j]));
        }
        else
         fSGEnergyValue[j] = E;
      }
    }
  }

  fHits[nBaF2s] = EBufferEnd;
  fTimeOR[nBaF2s] = EBufferEnd;
  fEnergyOR[nBaF2s] = EBufferEnd;
  fNhits = nBaF2s;
  fTotalEnergy = total;

  if(fPatternHits)
  {
    fPatternHits[0][nVetos] = EBufferEnd;
    fNPatternHits[0] = nVetos;
    //Note, that this may conflict with other ReadDecoded() functions later on, as this destroys the venergy[]
    //information! So, the bit pattern veto should not be used if another veto implementation is running!
    for(UInt_t i=0; i<fNelem; i++) venergy[i] = 0.0; //Need to cleanup, so that no data from previous event remains
  }

  if(fIsRawHits)
  {
    fRawEnergyHits[0] = EBufferEnd;
    fRawTimeHits[0] = EBufferEnd;
  }

  bSimul = true;
}

//-----------------------------------------------------------------------------

ClassImp(TA2TAPS_BaF2)
