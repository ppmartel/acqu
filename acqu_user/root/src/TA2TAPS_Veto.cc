//--Author	S Schumann   10th Jun 2009
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2TAPS_Veto
//
// TAPS Veto detectors
// Plastic scintillator tiles in front of TAPS crystals
//

#include "TA2TAPS_Veto.h"

enum { ETVEnergyResolution=1955, ETVTimeResolution };

static const Map_t kTAPSVetoKeys[] =
{
  {"Energy-Resolution:", ETVEnergyResolution},
  {"Time-Resolution:",   ETVTimeResolution},
  {NULL, -1}
};


//---------------------------------------------------------------------------

TA2TAPS_Veto::TA2TAPS_Veto(const char* name, TA2System* apparatus)
             :TA2Detector(name, apparatus)
{
  //Default detector initialisation

  fUseSigmaEnergy = 0;
  fUseSigmaTime   = 0;
  fSigmaEnergy    = 0.0;
  fSigmaTime      = 0.0;

  AddCmdList(kTAPSVetoKeys); //For SetConfig()
}

//---------------------------------------------------------------------------

TA2TAPS_Veto::~TA2TAPS_Veto()
{
  //Free up all allocated memory
  //...arrays created at the initialisation stage
  DeleteArrays();
}

//-----------------------------------------------------------------------------

void TA2TAPS_Veto::LoadVariable()
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

void TA2TAPS_Veto::SaveDecoded()
{
  //Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------

void TA2TAPS_Veto::SetConfig(char* line, int key)
{
  //Load parameters from file or command line
  switch(key)
  {
  case ETVEnergyResolution:
    //Energy Resolution Read-in Line
    if(sscanf(line, "%lf%d", &fSigmaEnergy, &fUseSigmaEnergy) < 2)
      PrintError(line,"<TA2TAPS_Veto Energy Resolution>");
    break;
  case ETVTimeResolution:
    //Time resolution read-in line
    if(sscanf(line, "%lf%d", &fSigmaTime, &fUseSigmaTime) < 2)
      PrintError(line,"<TA2TAPS_Veto Time Resolution>");
    break;
  default:
    //Command not found...possible pass to next config
    TA2Detector::SetConfig(line, key);
    break;
  }
  return;
}

//---------------------------------------------------------------------------

inline void TA2TAPS_Veto::Decode()
{
  //Run basic TA2Detector decode
  DecodeBasic();
  for(UInt_t n=0; n<fNelement; n++)
    if((fTime[n]==EBufferEnd) || (fTime[n]==-1.0))
      fTime[n] = (Double_t)ENullHit;
}

//---------------------------------------------------------------------------

ClassImp(TA2TAPS_Veto)
