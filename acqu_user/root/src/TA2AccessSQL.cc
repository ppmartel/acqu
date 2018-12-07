//*************************************************************************
//* Author: Patrik Ott
//*************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TA2AccessSQL                                                         //
//                                                                      //
// Class used for reading the SQL config database.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////





#include "TA2AccessSQL.h"


//______________________________________________________________________________
TA2AccessSQL::TA2AccessSQL(const char* name, TA2Analysis* analysis)	: TA2Physics(name, analysis), 
  fCaLibReader(0)
{
  // command-line recognition for SetConfig()
  AddCmdList(AccessSQLConfigKeys);
  
  // init members 
  fAnalysisMode = gAR->GetProcessType();
  
  if (fAnalysisMode == EMCProcess) fIsMC = kTRUE;
  else fIsMC = kFALSE;
  
  fTagger		= 0;         				// pointer to the Tagger
  fLadder		= 0;    					// pointer to the Ladder
  fCB			= 0;   						// pointer to the Crystal Ball
  fNaI		= 0;       					// pointer to the NaI elements
  fPID		= 0;           				// pointer to the PID
  fMWPC       	= 0;						// pointer to the MWPC
  fTAPS		= 0;       					// pointer to TAPS
  fBaF2PWO	= 0;	       				// pointer to the BaF2 (or the BaF2/PWO) array
  fVeto		= 0;           				// pointer to the TAPS Vetos
  fLinPol	= 0;           				// pointer to the linear Polarization class (if available)
  fPairSpec     = 0;
  fPbWO4            = 0;
  fTOF              = 0;   
  fPbGlassApp       = 0;
  fPbGlass          = 0;  
  fMoeller          = 0;
  
  fRunNumber = fIsMC ? 0 : TOSUtils::ExtractRunNumber(gAR->GetFileName());
}



TA2AccessSQL::~TA2AccessSQL()
{
}

void TA2AccessSQL::SetConfig(Char_t* line, Int_t key)
{
  switch (key)
  {
  case ESQL_USE_CALIB:
  {
    Char_t tmp[5][256];
    Int_t useNumber = 0;

    // read CaLib parameters
    if (sscanf(line, "%s%s%s%s%s%d", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], &useNumber) >= 5)
    {
      // create the CaLib reader
      if (useNumber != 0) fCaLibReader = new CaLibReader_t(tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], useNumber);
      else fCaLibReader = new CaLibReader_t(tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], fRunNumber);
      
      // check database connection
      if (fCaLibReader->IsConnected())
        Info("SetConfig", "Using calibration '%s' in the CaLib database '%s' on '%s@%s'"
             , tmp[4], tmp[1], tmp[2], tmp[0]);
      else
      {
        delete fCaLibReader;
        fCaLibReader = 0;
        Error("SetConfig", "Could not connect to CaLib database server!");
        // block here to get the user's attention
        Error("SetConfig", "Blocking analysis due to CaLib error!");
        for (;;) { gSystem->Sleep(1000); }
      }
    }
    else Error("SetConfig", "CaLib could not be configured and is hence disabled!");
    break;
  }
  case ESQL_CALIB_TAGG:
  {   
    Int_t time;
    Int_t eff;
    
    // read CaLib TAGG parameters
    if (sscanf(line, "%d%d", &time, &eff) == 2)
    {
      // check if CaLib reader exists
      if (fCaLibReader) 
      {
        fCaLibReader->SetTAGGtime((Bool_t) time);
        fCaLibReader->SetTAGGeff((Bool_t) eff);
      }
      else
      {
        Error("SetConfig", "TAGG cannot be configured because CaLib was not configured!");
        break;
      }
    }
    else Error("SetConfig", "CaLib TAGG calibration could not be configured!");
    break;
  }
  case ESQL_CALIB_CB:
  {   
    Int_t energy;
    Int_t time;
    Int_t timewalk;
    Int_t quadEnergy;
    Int_t led;
    
    // read CaLib CB parameters
    if (sscanf(line, "%d%d%d%d%d", &energy, &time, &timewalk, &quadEnergy, &led) == 5)
    {
      // check if CaLib reader exists
      if (fCaLibReader)
      {
        fCaLibReader->SetCBenergy((Bool_t) energy);
        fCaLibReader->SetCBtime((Bool_t) time);
        fCaLibReader->SetCBwalk((Bool_t) timewalk);
        fCaLibReader->SetCBquadEnergy((Bool_t) quadEnergy);
        fCaLibReader->SetCBled((Bool_t) led);
      }
      else
      {
        Error("SetConfig", "CB cannot be configured because CaLib was not configured!");
        break;
      }
    }
    else Error("SetConfig", "CaLib CB calibration could not be configured!");
    break;
  }
  case ESQL_CALIB_TAPS:
  {   
    Int_t energy;  
    Int_t time;  
    Int_t quadEnergy;
    Int_t led;
    Int_t cfd;
    
    // read CaLib TAPS parameters
    if (sscanf(line, "%d%d%d%d%d", &energy, &time, &quadEnergy, &led, &cfd) == 5)
    {
      // check if CaLib reader exists
      if (fCaLibReader)
      {
        fCaLibReader->SetTAPSenergy((Bool_t) energy);
        fCaLibReader->SetTAPStime((Bool_t) time);
        fCaLibReader->SetTAPSquadEnergy((Bool_t) quadEnergy);
        fCaLibReader->SetTAPSled((Bool_t) led);
        fCaLibReader->SetTAPScfd((Bool_t) cfd);
      }
      else
      {
        Error("SetConfig", "TAPS cannot be configured because CaLib was not configured!");
        break;
      }
    }
    else Error("SetConfig", "CaLib TAPS calibration could not be configured!");
    break;
  }
  case ESQL_CALIB_PID:
  {   
    Int_t phi;
    Int_t energy;
    Int_t time;
    
    // read CaLib PID parameters
    if (sscanf(line, "%d%d%d", &phi, &energy, &time) == 3)
    {
      // check if CaLib reader exists
      if (fCaLibReader)
      {
        fCaLibReader->SetPIDphi((Bool_t) phi);
        fCaLibReader->SetPIDenergy((Bool_t) energy);
        fCaLibReader->SetPIDtime((Bool_t) time);
      }
      else
      {
        Error("SetConfig", "PID cannot be configured because CaLib was not configured!");
        break;
      }
    }
    else Error("SetConfig", "CaLib PID calibration could not be configured!");
    break;
  }
  case ESQL_CALIB_VETO:
  {   
    Int_t energy;
    Int_t time;
    Int_t led;
    
    // read CaLib Veto parameters
    if (sscanf(line, "%d%d%d", &energy, &time, &led) == 3)
    {
      // check if CaLib reader exists
      if (fCaLibReader)
      {
        fCaLibReader->SetVetoenergy((Bool_t) energy);
        fCaLibReader->SetVetotime((Bool_t) time);
        fCaLibReader->SetVetoled((Bool_t) led);
      }
      else
      {
        Error("SetConfig", "Veto cannot be configured because CaLib was not configured!");
        break;
      }
    }
    else Error("SetConfig", "CaLib Veto calibration could not be configured!");
    break;
  }
  case ESQL_CALIB_MCRUNNR:
  {
    // read run number for MC files
    if (sscanf(line, "%d", &fRunNumber) != 1)
      Error("SetConfig", "CaLib MC run number could not be set!");
    break;
  }
  default:
  {
    // default main apparatus SetConfig()
    TA2Physics::SetConfig(line, key);
    break;
  }
  }
}

void TA2AccessSQL::LoadDetectors(TA2DataManager* parent, Int_t depth)
{
  // Load recursively all detector components.
  
  Bool_t added = kFALSE;
  
  // get the children, leave if none found
  TList* children = parent->GetChildren();
  if (!children) return;
  
  // loop over children
  TObjLink *lnk = children->FirstLink();
  while (lnk)
  {
    TObject* obj = lnk->GetObject();
    
    // look for detectors
    if (!strcmp(obj->ClassName(), "TA2Tagger"))
    {
      fTagger = (TA2Tagger*) obj;
      added = kTRUE;
    }
    if (!strcmp(obj->ClassName(), "TA2Ladder"))
    {
      fLadder = (TA2Ladder*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2CentralApparatus"))
    {
      fCB = (TA2CentralApparatus*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2CalArray"))
    {
      fNaI = (TA2CalArray*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2PlasticPID"))
    {
      fPID = (TA2PlasticPID*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2CylMwpc"))
    {
      fMWPC = (TA2CylMwpc*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2Taps"))
    {
      fTAPS = (TA2Taps*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2TAPS_BaF2"))
    {
      fBaF2PWO = (TA2TAPS_BaF2*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2TAPS_Veto"))
    {
      fVeto = (TA2TAPS_Veto*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->GetName(), "PbWO4"))
    {
      fPbWO4 = (TA2GenericDetector*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->GetName(), "TOF"))
    {
      fTOF = (TA2LongScint*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->GetName(), "PbGlassApp"))
    {
      fPbGlassApp = (TA2GenericApparatus*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->GetName(), "PbGlassDet"))
    {
      fPbGlass = (TA2GenericDetector*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2LinearPolEpics"))
    {
      fLinPol = (TA2LinearPolEpics*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2PairSpec"))
    {
      fPairSpec = (TA2PairSpec*) obj;
      added = kTRUE;
    }
    else if (!strcmp(obj->ClassName(), "TA2Moeller"))
    {
      fMoeller = (TA2Moeller*) obj;
      added = kTRUE;
    }

    // print information if a detector was added
    if (added)
    {
      for (Int_t i = 0; i < depth; i++) printf("  ");
      printf( "   %s (%s)\n", obj->GetName(), obj->ClassName());
      added = kFALSE;
    }
    
    // add children of child
    LoadDetectors((TA2DataManager*) obj, depth + 1);
    lnk = lnk->Next();
  }
}

//______________________________________________________________________________ 
void TA2AccessSQL::ApplyCaLib()
{
  // Update the detector calibration parameters using CaLib.
  
  // calibrate TAGG
  if (fLadder)
  {
    if (!fCaLibReader->ApplyTAGGcalib(fLadder))
      Error("ApplyCaLib", "An error occured during tagger calibration!");
  }
  else Error("ApplyCaLib", "Tagger not found - could not calibrate the tagger using CaLib!");
  
  // calibrate CB
  if (fNaI)
  {
    if (!fCaLibReader->ApplyCBcalib(fNaI))
      Error("ApplyCaLib", "An error occured during CB calibration!");
  }
  else Error("ApplyCaLib", "CB not found - could not calibrate CB using CaLib!");
  
  // calibrate TAPS
  if (fBaF2PWO)
  {
    if (!fCaLibReader->ApplyTAPScalib(fBaF2PWO))
      Error("ApplyCaLib", "An error occured during TAPS calibration!");
  }
  else Error("ApplyCaLib", "TAPS not found - could not calibrate TAPS using CaLib!");
  
  // calibrate PID
  if (fPID)
  {
    if (!fCaLibReader->ApplyPIDcalib(fPID))
      Error("ApplyCaLib", "An error occured during PID calibration!");
  }
  else Error("ApplyCaLib", "PID not found - could not calibrate PID using CaLib!");
  
  // calibrate Veto 
  if (fVeto)
  {
    if (!fCaLibReader->ApplyVetocalib(fVeto))
      Error("ApplyCaLib", "An error occured during Veto calibration!");
  }
  else Error("ApplyCaLib", "Veto not found - could not calibrate Veto using CaLib!");
}

void TA2AccessSQL::PostInit()
{
  TA2Physics::PostInit();
  
  // Show some general information
  printf("\n\n\n  ****** GENERAL INFORMATION ******\n\n");
  
  if (gAR->IsOnline()) 
  {
      printf("   ACQU data name      : %s\n", gAR->GetFileName());
      printf("   Analysis type       : Online\n");
  }
  else
  {
      printf("   ROOT input file #1  : %s\n", gAR->GetTreeFileList(0));
      printf("   Analysis type       : Offline\n");
  }

  if (fAnalysisMode == EMk1Process) printf("   Analysis input      : Raw data\n");
  else if (fAnalysisMode == EMCProcess) printf("   Analysis input      : MC data\n");
  else if (fAnalysisMode == EPhysicsProcess) printf("   Analysis input      : Physics\n");
  else printf("   Analysis input      : Unknown\n");
  printf("   Run number          : %d\n", fRunNumber);
  printf("   Analysis class      : %s\n", gAR->GetAnalysisType());
  printf("   Analysis setup      : %s\n", gAR->GetAnalysisSetup());
  printf("   Physics class       : %s\n", gAR->GetAnalysis()->GetPhysics()->ClassName());
  printf("   Physics name        : %s\n", gAR->GetAnalysis()->GetPhysics()->GetName());
  printf("   Max ADC             : %d\n", gAR->GetMaxADC());
  printf("   Max Scaler          : %d\n", gAR->GetMaxScaler());
  printf("\n");
  
  // load the detectors and their components
  printf("\n\n\n  ****** DETECTOR LIST ******\n\n");
  printf("   Loaded Detectors:\n\n");
  LoadDetectors(fParent, 0);
  printf("\n");
  
  // CaLib
  if (fCaLibReader)
  {
    // try to apply the calibration
    ApplyCaLib();
    fCaLibReader->Deconnect();
    
    // block when an error occurred to get the user's attention
    if (fCaLibReader->WasError()) 
    {
      Error("SetConfig", "Blocking analysis due to CaLib error!");
      for (;;) { gSystem->Sleep(1000); }
    }
  }
}

void TA2AccessSQL::LoadVariable()
{
  TA2Physics::LoadVariable();
}

void TA2AccessSQL::Reconstruct()
{
  TA2Physics::Reconstruct();
}

ClassImp(TA2AccessSQL)
