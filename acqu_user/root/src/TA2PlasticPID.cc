//--Author	JRM Annand   30th Sep 2003
//--Rev 	JRM Annand...15th Oct 2003 ReadDecoded...MC data
//--Rev 	JRM Annand... 5th Feb 2004 3v8 compatible
//--Rev 	JRM Annand...19th Feb 2004 User code
//--Update	JRM Annand...16th May 2005 ReadDecoded (bug G3 output)
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2PlasticPID
//
// Internal Particle Identification Detector for the Crystal Ball
// Cylindrical array of plastic scintillators
//

#include "TA2PlasticPID.h"

enum { EPPEnergyResolution=1950, EPPTimeResolution, EPPEnergyThreshold };

static const Map_t kPlasticPIDKeys[] =
{
  {"Energy-Resolution:", EPPEnergyResolution},
  {"Time-Resolution:",   EPPTimeResolution},
  {"Energy-Threshold:",  EPPEnergyThreshold},
  {NULL, -1}
};


//---------------------------------------------------------------------------

TA2PlasticPID::TA2PlasticPID(const char* name, TA2System* apparatus)
              :TA2Detector(name, apparatus)
{
  //Default detector initialisation

  fUseSigmaEnergy = 0;
  fUseSigmaTime   = 0;
  fSigmaEnergy    = 0.0;
  fPowerEnergy    = 0.0;
  fSigmaTime      = 0.0;
  fThrMean        = 0.0;
  fThrSigma       = 0.0;

  AddCmdList(kPlasticPIDKeys); //For SetConfig()
}

//---------------------------------------------------------------------------

TA2PlasticPID::~TA2PlasticPID()
{
  //Free up all allocated memory
  //...arrays created at the initialisation stage
  DeleteArrays();
}

//-----------------------------------------------------------------------------

void TA2PlasticPID::LoadVariable()
{
  //Input name - variable pointer associations for any subsequent
  //cut or histogram setup.
  //LoadVariable( "name", pointer-to-variable, type-spec );
  //NB scaler variable pointers need the preceeding &
  //while array variable pointers do not.
  //type-spec ED prefix for a Double_t variable
  //          EI prefix for an Int_t variable
  //type-spec SingleX for a single-valued variable
  //          MultiX  for a multi-valued variable

  //                              name     pointer          type-spec
  //TA2DataManager::LoadVariable("Energy", &fTotalEnergy,   EDSingleX);

  //Call the standard detector
  TA2Detector::LoadVariable();
}

//---------------------------------------------------------------------------

void TA2PlasticPID::SaveDecoded()
{
  //Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------

void TA2PlasticPID::SetConfig(char* line, int key)
{
  //Load parameters from file or command line
  switch(key)
  {
  case EPPEnergyResolution:
    //Energy Resolution Read-in Line
    if(sscanf(line, "%lf %lf %d", &fSigmaEnergy, &fPowerEnergy, &fUseSigmaEnergy) < 3)
      PrintError(line,"<TA2PlasticPID Energy Resolution>");
    break;
  case EPPTimeResolution:
    //Time resolution read-in line
    if(sscanf(line, "%lf %d", &fSigmaTime, &fUseSigmaTime) < 2)
      PrintError(line,"<TA2PlasticPID Time Resolution>");
    break;
  case EPPEnergyThreshold:
    //Energy Threshold Read-in Line
    if(sscanf(line, "%lf %lf", &fThrMean, &fThrSigma) < 2)
      PrintError(line,"<TA2PlasticPID Energy Threshold>");
    break;
  default:
    //Command not found...possible pass to next config
    TA2Detector::SetConfig(line, key);
    break;
  }
  return;
}

//---------------------------------------------------------------------------

inline void TA2PlasticPID::Decode()
{
  // Run basic TA2Detector decode
  DecodeBasic();
  for(UInt_t n=0; n<fNelement; n++)
    if((fTime[n]==EBufferEnd) || (fTime[n]==-1.0))
      fTime[n] = (Double_t)ENullHit;
}

//---------------------------------------------------------------------------

void TA2PlasticPID::ReadDecoded()
{
  //Read back:
  //either previously analysed data from Root Tree file
  //or MC simulation results, assumed to have the same data structure
  Double_t E, T;
  Double_t Lo, Hi;
  Double_t total = 0.0;
  UInt_t i,k;
  Int_t j;
  Int_t* index;
  Float_t* time = NULL;
  Float_t* energy;

  fNhits = *(Int_t*)(fEvent[EI_vhits]);
  index = (Int_t*)(fEvent[EI_iveto]);
  energy = (Float_t*)(fEvent[EI_eveto]);
  if(fIsTime) time = (Float_t*)(fEvent[EI_tveto]);

  for(i=0,k=0; i<fNhits; i++)
  {
    j = *index++;
    if(!j) //Is it a real hit
    {
      i--;
      energy++;
      time++;
      continue;
    }
    j--;
    E = (*energy++) * 1000.0 * fEnergyScale; //Convert GeV to MeV and correct bad simulation energies
    if(fUseSigmaEnergy) E+=gRandom->Gaus(0.0, TMath::Power(E, fPowerEnergy) * fSigmaEnergy);
    Lo = fElement[j]->GetEnergyLowThr();
    Hi = fElement[j]->GetEnergyHighThr();
    //Lo = fElement[j]->GetADCcut()->GetLowThr();
    //Hi = fElement[j]->GetADCcut()->GetHighThr();
    if((E < Lo) || (E > Hi)) continue; //Energy inside thresh?
    if(E < gRandom->Gaus(fThrMean, fThrSigma)) continue;
    fEnergy[j] = E;
    fEnergyOR[k] = E;
    total+=E;
    if(fIsTime)
    {
      T = (*time++);
      if(fUseSigmaTime) T+=gRandom->Gaus(0.0, fSigmaTime);
      Lo = fElement[j]->GetTimeLowThr();
      Hi = fElement[j]->GetTimeHighThr();
      //Lo = fElement[j]->GetTDCcut()->GetLowThr();
      //Hi = fElement[j]->GetTDCcut()->GetHighThr();
      if((T < Lo) || (T > Hi)) continue; //Time inside thresh?
      fTime[j] = T;
      fTimeOR[k] = T;
    }
    fHits[k] = j;
    k++;
  }

  fHits[k] = EBufferEnd;
  fEnergyOR[k] = EBufferEnd;
  if(fIsTime) fTimeOR[k] = EBufferEnd;
  fTotalEnergy = total;
  fNhits = k;
}

//---------------------------------------------------------------------------

ClassImp(TA2PlasticPID)
