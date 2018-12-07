#include "TA2PairSpec.h"
#include <sstream>
#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------

TA2PairSpec::TA2PairSpec( const char* name, TA2System* analysis  )
               :TA2Detector( name, analysis)
{
  fNchannels = 0;
  fDay0 = 0;
  fTimeStamp = 0;
  
  fScalerOpen = NULL; 
  fScalerGated = NULL;
  fScalerGatedDly = NULL;
  
  fScalerSumOpen = NULL; 
  fScalerSumGated = NULL;
  fScalerSumGatedDly = NULL;

  fLadder	= NULL; // Tagger ladder
  
}


//-----------------------------------------------------------------------------

TA2PairSpec::~TA2PairSpec() 
{
  if(fScalerOpen)     delete[] fScalerOpen;
  if(fScalerGated)    delete[] fScalerGated;
  if(fScalerGatedDly) delete[] fScalerGatedDly;
  
  if(fScalerSumOpen)     delete[] fScalerSumOpen;
  if(fScalerSumGated)    delete[] fScalerSumGated;
  if(fScalerSumGatedDly) delete[] fScalerSumGatedDly;
}

//-----------------------------------------------------------------------------

void TA2PairSpec::PostInit( )
{
  // Ladder
  fLadder = (TA2Ladder*)gAN->GetGrandChild("FPD","TA2Detector");
  if (!fLadder) PrintError( "", "<No Ladder class found>", EErrFatal);

  fNchannels = fLadder -> GetNelement();

  // Need to distinguish between OLD/NEW tagger, for the decoding.
  // To avoid flag in the config, this can be done using the timestamp
  // Decided to set day-0 of the new FPD on the 2017-10-01 at 00.00.01
  isOldFPD = kFALSE;
  fDay0 = 1506816000; // 2017-10-01 at 00.00.01
  fTimeStamp = gAR->GetFileTimeEpoch();

  if (fTimeStamp < fDay0)
    isOldFPD = kTRUE;
  
  // skip TA2Detector PostInit
  TA2DataManager::PostInit();
}
  
  //-----------------------------------------------------------------------------

void TA2PairSpec::LoadVariable( )
{
  // buffers for histogram contents 
  fScalerOpen = new Double_t[fNchannels]; 
  fScalerGated = new Double_t[fNchannels];
  fScalerGatedDly = new Double_t[fNchannels];
  
  fScalerSumOpen = new Double_t[fNchannels]; 
  fScalerSumGated = new Double_t[fNchannels];
  fScalerSumGatedDly = new Double_t[fNchannels];
  
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup.
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  //                            name                     pointer          type-spec
  TA2DataManager::LoadVariable("Open",     fScalerOpen,     EDScalerX);
  TA2DataManager::LoadVariable("Gated",    fScalerGated,    EDScalerX);
  TA2DataManager::LoadVariable("GatedDly", fScalerGatedDly, EDScalerX);
  TA2DataManager::LoadVariable("SumOpen",     fScalerSumOpen,     EDScalerX);
  TA2DataManager::LoadVariable("SumGated",    fScalerSumGated,    EDScalerX);
  TA2DataManager::LoadVariable("SumGatedDly", fScalerSumGatedDly, EDScalerX);
}

//-----------------------------------------------------------------------------

void TA2PairSpec::Decode()
{
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerOpen[i] = fScaler[fLadder->GetScalerIndex()[i]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    if(isOldFPD && (i<=31 || i>=321) )fScalerGated[i] = fScaler[fLadder->GetScalerIndex()[i]+64];
    else fScalerGated[i] = fScaler[fLadder->GetScalerIndex()[i]+96];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    if(isOldFPD && (i<=31 || i>=321) ) fScalerGatedDly[i] = fScaler[fLadder->GetScalerIndex()[i]+(2*64)];
    else fScalerGatedDly[i] = fScaler[fLadder->GetScalerIndex()[i]+(2*96)];
  }
  
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerSumOpen[i] = fScalerSum[fLadder->GetScalerIndex()[i]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    if(isOldFPD && (i<=31 || i>=321) ) fScalerSumGated[i] = fScalerSum[fLadder->GetScalerIndex()[i]+64];
    else fScalerSumGated[i] = fScalerSum[fLadder->GetScalerIndex()[i]+96];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    if(isOldFPD && (i<=31 || i>=321) ) fScalerSumGatedDly[i] = fScalerSum[fLadder->GetScalerIndex()[i]+(2*64)];
    else fScalerSumGatedDly[i] = fScalerSum[fLadder->GetScalerIndex()[i]+(2*96)];
  }  
}

//-----------------------------------------------------------------------------
ClassImp(TA2PairSpec)
