#include "TA2Moeller.h"
#include <sstream>
#include <iostream>
#include <iomanip>

// Valid Keywords for command-line setup of CB apparatus
enum { EMoellerSetup = 2100, EVupromOffset, EDisplayAllHistos};
static const Map_t kMoellerKeys[] = {
  {"MoellerSetup:",     EMoellerSetup},   
  {"VupromOffset:",     EVupromOffset},
  {"DisplayAllHistos:", EDisplayAllHistos},
  {NULL,            -1}
};


using namespace std;

//-----------------------------------------------------------------------------

TA2Moeller::TA2Moeller( const char* name, TA2System* analysis  )
               :TA2Detector( name, analysis)
{
  fReadoutStarted = kFALSE;
  AddCmdList( kMoellerKeys );                  // for SetConfig()
}


//-----------------------------------------------------------------------------

TA2Moeller::~TA2Moeller() 
{
  
}

//-----------------------------------------------------------------------------

void TA2Moeller::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus
  std::stringstream ss(line);
  switch (key) {
  case EMoellerSetup: {
    if(!(ss >> fNLeftChannels)) {
      PrintError(line, "TA2Moeller #left channels", EErrFatal);
    }
    if(!(ss >> fNPairsPerCh)) {
      PrintError(line, "TA2Moeller #pairs per ch", EErrFatal);
    }
    if(!(ss >> fNBins)) {
      PrintError(line, "TA2Moeller #Bins", EErrFatal);
    }
    break;
  }
  case EVupromOffset: {
    UInt_t offset;
    if(!(ss >> offset)) {
      PrintError(line, "TA2Moeller VupromOffset", EErrFatal);
    }
    fVupromOffset.push_back(offset);
    break;
  }  
  case EDisplayAllHistos: {
    for(UInt_t i=0;i<fTDCDisplayLines.size();i++)
      ParseDisplay(fTDCDisplayLines[i]);
    break;
  }
  default: {
    // default main detector SetConfig()
    TA2Detector::SetConfig( line, key );
    break;
  }
  }
}

//-----------------------------------------------------------------------------

void TA2Moeller::LoadVariable( )
{
  
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
  
  // create pointer storage
  fTDCs.resize(fVupromOffset.size()*fNLeftChannels*fNPairsPerCh*2);
  fTDCDisplayLines.resize(fTDCs.size());
  
  // loop over the combinations
  for(UShort_t i=0;i<fVupromOffset.size();i++) {
    for(UShort_t j=0;j<fNLeftChannels;j++) {
      for(UShort_t k=0;k<fNPairsPerCh;k++) {
        for(UShort_t hel=0;hel<2;hel++) {
          UShort_t idx = 
              i * 2*fNLeftChannels*fNPairsPerCh+
              j * 2*fNPairsPerCh+
              k * 2+
              hel;
          // create the buffer
          UInt_t* ptr = new UInt_t[fNBins];
          for(UInt_t l=0;l<fNBins;l++)
            ptr[l] = 0;
          fTDCs[idx] = ptr;
          // construct the name
          stringstream ss; // = new stringstream();        
          ss << "TDC_Vup_Left_Pair_Hel_";
          ss << i << "_" << j << "_" << k << "_" << hel;
          // str needs to live longer than this scope...
          // this is just another memory leak here          
          string* str = new string(ss.str()); 
          TA2DataManager::LoadVariable(str->c_str(), fTDCs[idx], EIScalerX);
          
          // create a parsable display line to easily enable all histograms
          stringstream ss_display;
          ss_display << "1D " << ss.str() << " " << 
                        fNBins << " 0 " << fNBins;
          string* str_display = new string(ss_display.str()); 
          fTDCDisplayLines[idx] = str_display->c_str();
        } 
      }
    }
  }
}

//-----------------------------------------------------------------------------

void TA2Moeller::PostInit( )
{ 
  
  // skip TA2Detector PostInit
  TA2DataManager::PostInit();
}

//-----------------------------------------------------------------------------

void TA2Moeller::Decode()
{
  if(gAR->IsScalerRead()) {
    //cout << "Scaler read" << endl;
    // check for start marker, sent out by the first Vuprom
    UInt_t marker = fScaler[fVupromOffset[0]];
    if((marker & 0xff000000) == 0xaa000000) {
      if(fReadoutStarted) {
        //cerr << "Another readout although previous wasn't done. Skipping." << endl;
        return;
      }
      //UInt_t nReadouts = marker & 0xffff;
      //cout << "Moeller readout: "<< nReadouts << endl;
      // fScaler[fOffset+1] and fScaler[fOffset+2] 
      // contain a timestamp
      fNvaluesRead=0;
      fReadoutStarted = true;
      // the next call to Decode should show some data
      return;
    }
  }
  if(fReadoutStarted) {
    
    for(UShort_t i=0;i<fVupromOffset.size();i++) {
      for(UShort_t j=0;j<fNLeftChannels;j++) {
        for(UShort_t k=0;k<fNPairsPerCh;k++) {
          UInt_t iADC = fVupromOffset[i]
              + j * fNPairsPerCh
              + k;
          // process the adc value  
          UInt_t nHits = fMulti[iADC]->GetNstore();
          
          if(nHits != 3) {
            //if(nHits != 0)
            //  cerr << "Non-zero empty hit detected. weird." << endl;
            continue;
          }
          // process the hit        
          // this is ugly, but we want our UShort_t back 
          // which we put in with ADCStore!
          UShort_t* data = (UShort_t*)(fMulti[iADC]->GetStore());
          UShort_t ramAddr = data[0];
          UInt_t value = 0xffff*data[2]+data[1];
          
          UShort_t hel = ramAddr / fNBins;
          UShort_t bin = (ramAddr+fNBins/2) % fNBins; // shift the bins 
          UShort_t idx = i * 2*fNLeftChannels*fNPairsPerCh +
              j * 2*fNPairsPerCh +
              k * 2 +
              hel;
          //cout << " Bin " << bin << " Data " << value << endl;
          fTDCs[idx][bin] = value;
          fNvaluesRead++;
          if(fNvaluesRead == fTDCs.size()*fNBins) {
            //cout << "Moeller readout done" << endl;
            fReadoutStarted = false;
          }
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
ClassImp(TA2Moeller)
