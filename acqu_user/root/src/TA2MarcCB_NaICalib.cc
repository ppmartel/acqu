// Author: Marc Unvezagt
// Date:   10.01.12
//
// In method decode the calibration data with Am/Be are analysed.

#include "TA2MarcCB_NaICalib.h"

ClassImp(TA2MarcCB_NaICalib)

//---------------------------------------------------------------------------
TA2MarcCB_NaICalib::TA2MarcCB_NaICalib( const char* name, TA2System* apparatus ):TA2CalArray(name, apparatus){
  // Do not allocate any "new" memory here...Root will wipe
  // Set private variables to zero/false/undefined state
  // Pass kLaddKeys (command-control keywords) and
  // kLaddHist (names of histograms) to progenitor classes

  fCalEn = NULL;
  fCalEnSingle = NULL;
  fCalEnSingleHit = NULL;
  fClNHits = NULL;

  printf("TA2MarcCB_NaICalib constructed!\n\n");
}


//---------------------------------------------------------------------------
void TA2MarcCB_NaICalib::PostInit(){
  // Some further initialisation after all setup parameters read in
  // Start with alignment offsets
  // Create space for various output arrays


  fCalEn = new Double_t[fNelem+1];
  fCalEnSingle = new Double_t[fNelem+1];
  fCalEnSingleHit = new Double_t[fNelem+1];
  fClNHits = new UInt_t[32];

  TA2CalArray::PostInit();

}

//---------------------------------------------------------------------------
void TA2MarcCB_NaICalib::Decode( ){

  TA2Detector::Cleanup();

  TA2CalArray::Decode();

  // Zero all entries
  for(UInt_t i=0; i<=fNelem; i++){
    fCalEn[i] = EBufferEnd;
    fCalEnSingle[i] = EBufferEnd;
    fCalEnSingleHit[i] = EBufferEnd;
  }
  for(UInt_t i=0; i<32; i++){
    fClNHits[i] = EBufferEnd;
  }

  //for(UInt_t i=0; i<fNelem; i++){
  for(UInt_t i=0; i<fNhits; i++){
    //fCalEn[i] = fEnergy[i];
    fCalEn[fHits[i]] = fEnergy[fHits[i]];
    //printf("%f %f\n", fCalEn[fHits[i]], fEnergy[fHits[i]]);
  }

  for(UInt_t i=0; i<fNCluster; i++){
    fClNHits[i] = fCluster[fClustHit[i]]->GetNhits();
    if(fClNHits[i] == 1){
      fCalEnSingleHit[fClustHit[i]] = fCalEn[fClustHit[i]];
    }
  }
  if(fNCluster == 1) {
    for(UInt_t i=0; i<fNhits; i++){
      fCalEnSingle[fHits[i]] = fCalEn[fHits[i]];
    }
  }

}

//---------------------------------------------------------------------------

void TA2MarcCB_NaICalib::LoadVariable(){
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup.
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scalar variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable
  //
  //                            name     pointer          type-spec

  TA2DataManager::LoadVariable("CalEn",          fCalEn,           EDSingleX);
  TA2DataManager::LoadVariable("CalEnSingle",    fCalEnSingle,     EDSingleX);
  TA2DataManager::LoadVariable("CalEnSingleHit", fCalEnSingleHit,  EDSingleX);
  TA2DataManager::LoadVariable("ClNHits", fClNHits,  EIMultiX);

  TA2Detector::LoadVariable();
}

//---------------------------------------------------------------------------
