#include "TA2PairSpec.h"
#include <sstream>
#include <iostream>

// Valid Keywords for command-line setup of CB apparatus
enum { EPairSpecScalerBlock = 2000, EPairSpecVuprom, EPairSpecSetup };
static const Map_t kPairSpecKeys[] = {
  {"ScalerBlock:",   EPairSpecScalerBlock},   
  {"Vuprom:",        EPairSpecVuprom},
  {"PairSpecSetup:", EPairSpecSetup},
  {NULL,            -1}
};


using namespace std;

//-----------------------------------------------------------------------------

TA2PairSpec::TA2PairSpec( const char* name, TA2System* analysis  )
               :TA2Detector( name, analysis)
{
  fVupromSumSize = 0; 
  fScalerIndex = NULL;
  
  fScalerOpen = NULL; 
  fScalerGated = NULL;
  fScalerGatedDly = NULL;
  
  fScalerSumOpen = NULL; 
  fScalerSumGated = NULL;
  fScalerSumGatedDly = NULL;
  
  AddCmdList( kPairSpecKeys );                  // for SetConfig()
}


//-----------------------------------------------------------------------------

TA2PairSpec::~TA2PairSpec() 
{
  if(fScalerIndex) delete[] fScalerIndex;
  
  if(fScalerOpen)     delete[] fScalerOpen;
  if(fScalerGatedDly) delete[] fScalerGatedDly;
  if(fScalerGatedDly) delete[] fScalerGatedDly;
  
  if(fScalerSumOpen)     delete[] fScalerSumOpen;
  if(fScalerSumGatedDly) delete[] fScalerSumGatedDly;
  if(fScalerSumGatedDly) delete[] fScalerSumGatedDly;
}

//-----------------------------------------------------------------------------

void TA2PairSpec::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus
  std::stringstream ss(line);
  switch (key) {
  case EPairSpecScalerBlock: {
    UInt_t start;
    if(!(ss >> start)) {
      PrintError(line, "TA2PairSpec Start of ScalerBlock", EErrFatal);
    }
    // init the scaler map
    if(!fScalerIndex) fScalerIndex = new UInt_t[fNHistograms*fNchannels];
    
    // you can specify a custom permutation,
    // by default it's the identity permutation
    vector<UInt_t> perm;
    perm.resize(fScalerBlockSize);
    for(UInt_t i=0;i<fScalerBlockSize;i++) {
      if(!(ss >> perm[i])) {
        perm[i] = i;
      }
    }
    
    // generate the map "ordered channel (three histograms)" -> "ScalerIndex"
    for(UInt_t i=0;i<fScalerBlockSize;i++) {
      for(UInt_t j=0;j<fNHistograms;j++) {
        UInt_t ch = j*fNchannels+start+perm[i]; // respect the permutation
        UInt_t vupOffset = fScalerBlockSize*fNHistograms*(fVupromSumSize-fVupromSize);
        UInt_t offset = fScalerBlockSize*(fVupromSize*j+fScalerBlockIdx);        
        UInt_t idx = fScalerOffset+vupOffset+offset+i;
        fScalerIndex[ch] = idx;
        //cout << ch << " -> " << idx << endl;
      }      
    }
    fScalerBlockIdx++;
    break;
  }
  case EPairSpecVuprom: {
    if(!(ss >> fVupromSize)) {
      PrintError(line, "TA2PairSpec Size of Vuprom", EErrFatal);
    }
    fVupromSumSize += fVupromSize;
    fScalerBlockIdx=0;
    break;
  }
  case EPairSpecSetup: {
    if(!(ss >> fNchannels)) {
      PrintError(line, "TA2PairSpec Nchannels", EErrFatal);
    }
    if(!(ss >> fNHistograms)) {
      PrintError(line, "TA2PairSpec NHistograms", EErrFatal);
    }
    if(!(ss >> fScalerOffset)) {
      PrintError(line, "TA2PairSpec ScalerOffset", EErrFatal);
    }
    if(!(ss >> fScalerBlockSize)) {
      PrintError(line, "TA2PairSpec ScalerOffset", EErrFatal);
    }
    break;
  }
  default: {
    // default main apparatus SetConfig()
    TA2Detector::SetConfig( line, key );
    break;
  }
  }
}

//-----------------------------------------------------------------------------

void TA2PairSpec::LoadVariable( )
{
  // buffers for histogram contents 
  fScalerOpen = new UInt_t[fNchannels]; 
  fScalerGated = new UInt_t[fNchannels];
  fScalerGatedDly = new UInt_t[fNchannels];
  
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
  TA2DataManager::LoadVariable("Open",     fScalerOpen,     EIScalerX);
  TA2DataManager::LoadVariable("Gated",    fScalerGated,    EIScalerX);
  TA2DataManager::LoadVariable("GatedDly", fScalerGatedDly, EIScalerX);
  TA2DataManager::LoadVariable("SumOpen",     fScalerSumOpen,     EDScalerX);
  TA2DataManager::LoadVariable("SumGated",    fScalerSumGated,    EDScalerX);
  TA2DataManager::LoadVariable("SumGatedDly", fScalerSumGatedDly, EDScalerX);
}



//-----------------------------------------------------------------------------

void TA2PairSpec::PostInit( )
{
  // skip TA2Detector PostInit
  TA2DataManager::PostInit();
}

//-----------------------------------------------------------------------------

void TA2PairSpec::Decode()
{
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerOpen[i] = fScaler[fScalerIndex[i+0*fNchannels]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerGated[i] = fScaler[fScalerIndex[i+1*fNchannels]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerGatedDly[i] = fScaler[fScalerIndex[i+2*fNchannels]];
  }
  
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerSumOpen[i] = fScalerSum[fScalerIndex[i+0*fNchannels]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerSumGated[i] = fScalerSum[fScalerIndex[i+1*fNchannels]];
  }
  for(UInt_t i=0;i<fNchannels;i++) {
    fScalerSumGatedDly[i] = fScalerSum[fScalerIndex[i+2*fNchannels]];
  }
}

//-----------------------------------------------------------------------------
ClassImp(TA2PairSpec)
