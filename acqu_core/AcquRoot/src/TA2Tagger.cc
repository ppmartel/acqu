//--Author	JRM Annand         4th Feb 2003
//--Rev 	JRM Annand...     26th Feb 2003  1st production tagger
//--Rev 	K   Livingston... 21th May 2004  Major KL mods
//--Rev 	JRM Annand....... 10th Feb 2005  gcc 3.4 compatible
//--Rev 	JRM Annand....... 21st Apr 2005  IsPrompt(i) etc.,fMaxParticle
//--Rev 	JRM Annand....... 10th Nov 2006  Eelect from ladder
//--Rev 	JRM Annand....... 13th Dec 2008  Add Ken's TA2LinearPol
//--Rev 	JRM Annand....... 14th Apr 2009  Add some Getters
//--Rev 	JRM Annand....... 21st May 2009  Add TA2GenericDetector to list
//--Rev 	JRM Annand....... 12th Nov 2009  TA2Particle support
//                                               fBadScalerChan init
//--Update	JRM Annand.......  1st Apr 2011  Class variables protected
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Tagger
//
// Mainz photon-tagging spectrometer apparatus class
// Tagger can consist of 1 or 2 ladders (FP, Microscope) plus PbGlass
// (not yet implemened).
// FP and Micro are instances of the TA2Ladder class
// TA2LinearPol is now incorporated into TA2Tagger. The methods used are as
// coded originally by K.Livingston
//

#include "TA2Tagger.h"
#include "TAcquRoot.h"
#include "ARFile_t.h"
#include "TA2GenericDetector.h"

// Default list of detector classes that the TA2Tagger apparatus may contain
enum { ETaggLadder, ETaggBeamMon, ETaggMicroscope, ETaggGenDet,
       ETaggPhotonEnergy,};
static Map_t kValidDetectors[] = {
  {"TA2Ladder",         ETaggLadder},
  {"TA2GenericDetector",ETaggGenDet},
  {"TA2BeamMonitor",  	ETaggBeamMon},
  {NULL, 		-1}
};
enum { ETaggerMagnet = 100, ETaggerLinPol, ETaggerLinPolScaler,
       ETaggerLinPolTDC };
static const Map_t kTaggerKeys[] = {
  {"Magnet:",           ETaggerMagnet},
  {"Linear-Pol:",       ETaggerLinPol},
  {"Linear-Pol-Scaler:",ETaggerLinPolScaler},
  {"Linear-Pol-TDC:",   ETaggerLinPolTDC},
  {NULL,            -1}
};


//-----------------------------------------------------------------------------
TA2Tagger::TA2Tagger( const char* name, TA2System* analysis )
  :TA2Apparatus( name, analysis, kValidDetectors )
{

  fNMR = 0.0;
  fBeamEnergy = fBeamPol = 0.0;
  fFP = NULL;
  fMicro = NULL;
  fPbGlass = NULL;
  fIsFP = kFALSE;
  fIsMicro = kFALSE;
  fIsPbGlass = kFALSE;
  fIsLinPol = kFALSE;
  fHelicityADC = EHelicityADC;

  fInitLevel=EInitLevel0;    // Flag that no init done yet
  fGoniPlane = (UShort_t)ENullHit;
  fGoniMode = 0;

  fDInc = EBufferEnd;	     // make all these Buffer ends, 
  fDCoh = EBufferEnd;	     // since they don't ever get filled by the system,	
  fDEnh = EBufferEnd;	     // but in the Reconstruct loop directly.
  fDCohPara = EBufferEnd;
  fDEnhPara = EBufferEnd;
  fDCohPerp = EBufferEnd;
  fDEnhPerp = EBufferEnd;
  
  fDTDCInc = EBufferEnd;
  fDTDCCoh = EBufferEnd;
  fDTDCEnh = EBufferEnd;
  fDTDCCohPara = EBufferEnd;
  fDTDCEnhPara = EBufferEnd;
  fDTDCCohPerp = EBufferEnd;
  fDTDCEnhPerp = EBufferEnd;

  fDEdge = EBufferEnd;
  fDEdgeEnergy = EBufferEnd;
  
  fDoingScalers = kFALSE;
  fDoingTDCs = kFALSE;
  fHaveIncScaler = kFALSE;	//Flag that Inc ref scaler is not loaded
  fHaveIncTDC = kFALSE;		//Flag that Inc ref TDC is not loaded
  fScalerCount = 0;		//Running counter for scaler reads
  AddCmdList( kTaggerKeys );    // for SetConfig()

}


//-----------------------------------------------------------------------------
TA2Tagger::~TA2Tagger()
{
  // Delete any owned "new" memory here
  delete fPhotonEnergy;
  delete fPhotonTime;
  delete fP4;
  delete fMinv;
}

//-----------------------------------------------------------------------------
TA2Detector*  TA2Tagger::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the TA2Tagger
  // Valid detector types are...
  // 1. TA2Ladder
  // 2. TA2BeamMonitor

  if( !name ) name = Map2String( dclass );
  switch (dclass){
  default:
    return NULL;
  case ETaggLadder:
    return new TA2Ladder( name, this );
  case ETaggGenDet:
    return new TA2GenericDetector( name, this );
  case ETaggBeamMon:
    //    return new TA2BeamMonitor( name, this );
    printf(" Sorry beam monitor not yet implemented\n");
    return NULL;
  }
}

//-----------------------------------------------------------------------------
void TA2Tagger::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Tagger apparatus
  //
  Char_t name[128];
  Int_t n;
  switch (key){
  case ETaggerMagnet:
    // Tagger dipole setting and beam energy
    if( sscanf( line, "%lf%lf%lf", &fNMR, &fBeamEnergy, &fBeamPol ) < 2 )
      PrintError( line, "<Tagger NMR Energy-Input>" );
    break;
  case ETaggerLinPol:
    if( (n = sscanf(line, "%d%d%d%lf%s", &fBaseADC, &fEdgeChMin, &fEdgeChMax,
		    &fNormEnergy, name)) < 2){
      PrintError( line, "<Linear Pol app  parameters>" );
    }
    fGoniADC = &fADC[fBaseADC];	  // point fGoniADC to goniometer adcs
    if( n < 5 ) fEdgeFitName = BuildName("pol4");
    else fEdgeFitName = BuildName(name);
    if( n < 4 ) fEdgeChMax = 8;
    if( n < 3 ) fEdgeChMin = -2;
    fIsLinPol = kTRUE;
    break;
  case ETaggerLinPolScaler:
    if((n = sscanf(line, "%s",name)) < 1){
      PrintError( line, "<Parse error Linear Pol scaler filename>" );
      return;
    }
    fRefFileName = BuildName(name);
    fDoingScalers = kTRUE;
    break;
  case ETaggerLinPolTDC:
    if((n = sscanf(line, "%s%d",name,&fFillFreq)) < 2){
      PrintError( line, "<Parse error Linear Pol TDC filename>" );
      return;
    }
    fTDCRefFileName = BuildName(name);
    fDoingTDCs = kTRUE;
    break;
   default:
    // default main apparatus SetConfig()
    TA2Apparatus::SetConfig( line, key );
    break;
  }
}

//-----------------------------------------------------------------------------
void TA2Tagger::LoadVariable( )
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

  //
  // Original TA2Tagger stuff
  //                            name                  pointer        type-spec
  TA2DataManager::LoadVariable("PhotonEnergy",        fPhotonEnergy, EDMultiX);
  TA2DataManager::LoadVariable("PhotonTime",          fPhotonTime,   EDMultiX);
  //
  // Linear polarisation stuff
  //
  TA2DataManager::LoadVariable("Incoherent", 	      &fDInc,        EDSingleX);
  TA2DataManager::LoadVariable("Coherent", 	      &fDCoh,        EDSingleX);
  TA2DataManager::LoadVariable("Enhancement", 	      &fDEnh,        EDSingleX);
  TA2DataManager::LoadVariable("CoherentPara", 	      &fDCohPara,    EDSingleX);
  TA2DataManager::LoadVariable("EnhancementPara",     &fDEnhPara,    EDSingleX);
  TA2DataManager::LoadVariable("CoherentPerp", 	      &fDCohPerp,    EDSingleX);
  TA2DataManager::LoadVariable("EnhancementPerp",     &fDEnhPerp,    EDSingleX);
  TA2DataManager::LoadVariable("IncoherentTDC",       &fDTDCInc,     EDSingleX);
  TA2DataManager::LoadVariable("CoherentTDC", 	      &fDTDCCoh,     EDSingleX);
  TA2DataManager::LoadVariable("EnhancementTDC",      &fDTDCEnh,     EDSingleX);
  TA2DataManager::LoadVariable("CoherentParaTDC",     &fDTDCCohPara, EDSingleX);
  TA2DataManager::LoadVariable("EnhancementParaTDC",  &fDTDCEnhPara, EDSingleX);
  TA2DataManager::LoadVariable("CoherentPerpTDC",     &fDTDCCohPerp, EDSingleX);
  TA2DataManager::LoadVariable("EnhancementPerpTDC",  &fDTDCEnhPerp, EDSingleX);
  TA2DataManager::LoadVariable("ScanStep", 	      &fDScanStep,   EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeChan", 	      &fDEdge,       EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeEnergy",       &fDEdgeEnergy, EDSingleX);
  TA2DataManager::LoadVariable("PolPlane",            &fGoniPlane,   EDSingleX);
  //
  // General Apparatus stuff
  //
  TA2Apparatus::LoadVariable();
}


//-----------------------------------------------------------------------------
void TA2Tagger::PostInitialise( )
{
  // Create Photon-Energy arrays etc.

  if( fInitLevel == EInitLevel1 ){
    InitLinPolHist();
    return;
  }

  Int_t nladders=0;	//=1 for FP, 2 for FP + Micro
  Int_t nchannels=0;	// total no of channels FP + Micro

  TIter nextdet(fChildren);
  TA2Detector* det;
  fMinv = new Double_t[1]; //need this for base class, although never makes sense for ladder
  fMinv[0] = (Double_t)EBufferEnd; //so make it always an end buffer
  while( ( det = (TA2Detector*)nextdet() ) ){
    Int_t i = 0;
    Int_t key;
    while( (key = GetMapKey(i, kValidDetectors)) != -1 ){
      i++;
      if( det->InheritsFrom(Map2String(key, kValidDetectors) ) ){
	switch( key ){
	case  ETaggLadder:
	  if(nladders==0){	//ie no ladders yet
	    if( !fFP ){		//first ladder is defined as FP
	      fFP = (TA2Ladder*)det;
	      fNFPElem = fFP->GetNelem();
	      fLadderTime=fFP->GetTimeOR();
	      fLadderHits=fFP->GetHits();
	      if(fFP->IsTimeWindows()) fLadderWindows = fFP->GetWindows();
	      nchannels+=fNFPElem;
	      fFPEcalib = fFP->GetECalibration();
	      fIsFP = ETrue;
	      nladders++;
	    }
	  }
	  else if(nladders==1){ // alread have FP, so this is the microscope
	    if( !fMicro ){	
	    fMicro = (TA2Ladder*)det;
	    fNMicroElem = fMicro->GetNelem();
	    nchannels+=fNMicroElem;
	    fMicroEcalib = fMicro->GetECalibration();
	    fIsMicro = ETrue;
	    nladders++;
	    }
	  }
	  break;
	case ETaggBeamMon:
	  fPbGlass = det;
	  fIsPbGlass = ETrue;
	  break;
	default:
	  break;
	}
	break;
      }
    }
  }
  // Check there's a ladder detector....fatal if not
  // Create a PDG data base if not already done. TA2Particle needs it
  if( !fFP ) PrintError("","<No ladder detector found>",EErrFatal);
  fPhotonEnergy = new Double_t[nchannels];
  fPhotonTime = new Double_t[nchannels];
  fP4 = new TLorentzVector[nchannels];          // gcc 3.4 remove initialise
  if( !fParticleID ){
    fParticleID = new TA2ParticleID( (Char_t*)"16" ); // new PDG database
    gAN->SetParticleID( fParticleID );                // return it to analysis
  }
  fParticles = new TA2Particle[nchannels];
  fMaxParticle = nchannels;                     // for generic apparatus
  //if no microscope
  // Cut arrays
  InitLinPol();
  TA2DataManager::PostInit();
  fIsInit = kTRUE;
}

//-----------------------------------------------------------------------------
void TA2Tagger::Reconstruct( )
{
  // From "hits" provided by the TA2Ladder detector classes
  // reconstruct valid Gamma-Ray energies for the event
  
  Double_t *FPEnergy,*FPTime,*MicroEnergy,*MicroTime;
  Int_t nFPNhits,nMicroNhits;
  Int_t nPhoton=0;
  Int_t i;
  Bool_t ignoreMicro = kTRUE;

  FPEnergy = fFP->GetEelecOR();
  FPTime = fFP->GetTimeOR();
  nFPNhits = fFP->GetNhits();
  if(fIsMicro){
    MicroEnergy = fMicro->GetEelecOR();
    MicroTime = fMicro->GetTimeOR();
    nMicroNhits = fMicro->GetNhits();
  }
  //if no microscope loop round the focal plane
  if((!fIsMicro) || ignoreMicro){
    for( i=0; i<nFPNhits; i++ ){
      fPhotonEnergy[i] = fBeamEnergy - FPEnergy[i];
      fPhotonTime[i] = FPTime[i];
      fP4[i].SetPxPyPzE( 0.0, 0.0, fPhotonEnergy[i], fPhotonEnergy[i] );
      SetParticleInfo(i);
    }
    fPhotonEnergy[nFPNhits] = (Double_t)EBufferEnd;
    fPhotonTime[nFPNhits] = (Double_t)EBufferEnd;
    fNparticle = nFPNhits;
  }
  // if Micro, use micro values in the energy range it covers
  // ignore FP hits just above and below microscope range 
  // to prevent double counting.
  else{
    for( i=0; i<nFPNhits; i++ ){
      if(FPEnergy[i]>0.9*fMicroEcalib[0]){ 		//if into Micro range
	for( int j=0; j<nMicroNhits; j++ ){		//do micro hits
	  fPhotonEnergy[nPhoton] = fBeamEnergy - MicroEnergy[j];
	  fPhotonTime[nPhoton] = MicroTime[j];
	  fP4[nPhoton].SetPxPyPzE( 0.0, 0.0, fPhotonEnergy[nPhoton], fPhotonEnergy[nPhoton] );
	  nPhoton++;
	}
	break;
      }
      else{
	fPhotonEnergy[nPhoton] = fBeamEnergy - FPEnergy[i];
	fPhotonTime[nPhoton] = FPTime[i];
	fP4[nPhoton].SetPxPyPzE( 0.0, 0.0, fPhotonEnergy[nPhoton], fPhotonEnergy[nPhoton] );
	nPhoton++;
      }
    }
    for(int k=i;k<nFPNhits;k++){
      if(FPEnergy[k]>1.1*fMicroEcalib[fNMicroElem-1]){
	fPhotonEnergy[nPhoton] = fBeamEnergy - FPEnergy[k];
	fPhotonTime[nPhoton] = FPTime[k];
	fP4[nPhoton].SetPxPyPzE( 0.0, 0.0, fPhotonEnergy[nPhoton], fPhotonEnergy[nPhoton] );
	SetParticleInfo(nPhoton);
	fParticles[nPhoton].SetDetector(EDetMicro);
	nPhoton++;
      }
    }
    fNparticle = nPhoton;
  }
  fPhotonEnergy[fNparticle] = (Double_t)EBufferEnd;
  fPhotonTime[fNparticle] = (Double_t)EBufferEnd;

  // Do any linear polarisation stuff if there's a scaler read
  // and linear polarisation is defined
  if( gAR->IsScalerRead() && fIsLinPol ) ReconstructLinPol(); 
}

//-----------------------------------------------------------------------------
void TA2Tagger::InitLinPol( )
{
  // We need more than one pass at this... 
  // ie an Initialise: must come before and after the Display lines in
  // the setup file for this if we're to get the histograms filled.
  Char_t* line;
  Int_t channel;
  Double_t content;
  const Double_t *ladderECal;
  Double_t energy;

  if( !fIsLinPol ) return;
  fNormChannel=fNFPElem-5;		// default norm chan to 5 from the end
  
  if((ladderECal = fFP->GetECalibration())){	// ladder energy calibration
    fEnergyCalib = new Double_t[fNFPElem];
    for(int n=0;n<fNFPElem;n++){		// fill the photon energy calib
      energy=fBeamEnergy-ladderECal[n];
      fEnergyCalib[fNFPElem-1-n]=energy;
    }
    // work out the normalisation channel based on the norm energy
    for(int n=0;n<fNFPElem;n++){
      if(fEnergyCalib[n]>fNormEnergy){
	fNormChannel=n;
	break;
      }
    }
  }
  
  if((fDoingScalers)&&(fFP->IsScaler())){   // If using scalers
    fIncSpectrum = new Double_t[fNFPElem];  // Inc ref. spectrum
    fCohSpectrum = new Double_t[fNFPElem];  // Coh spectrum
    fEnhSpectrum = new Double_t[fNFPElem];  // Enhancement spectrum
    fBadScalerChan = new Bool_t[fNFPElem];  // bad scaler channels
    for(Int_t i=0; i<fNFPElem; i++) fBadScalerChan[i] = EFalse;
    
    fScalerCurr = fFP->GetScalerCurr();
    
    if( fRefFileName ){
      ARFile_t sFile( fRefFileName, "r", this );
      channel=0;
      while( (line = sFile.ReadLine()) ){       // scan scaler dump file
	sscanf(line,"%*d%lf",&content);           // get chan and content
	
	if(channel>=fNFPElem) break; 		    // break if past last chan
	fIncSpectrum[fNFPElem-1-channel]=content; //fill array
	channel++;
      } 					    //(increasing E_g order)
      fHaveIncScaler=ETrue;	// flag that we have the Inc. scaler reference
      //run though the tagger scalers to find bad channels
      for(int n=0;n<fNFPElem-4;n++){ //flag if less that 1/3 of neighbours	
	if(( 9*fIncSpectrum[n]) <
	   (fIncSpectrum[n+2]+fIncSpectrum[n+3]+fIncSpectrum[n+4])){
	  fBadScalerChan[n]=ETrue;
	}
	if(fIncSpectrum[n]<10.0)fBadScalerChan[n]=ETrue; // or less than 10.0
      }
    }
    else fprintf(fLogStream,"Warning: no scaler file %s \n",fRefFileName);
    
  }
  if((fDoingTDCs)&&(fFP->IsTime())){		//If doing TDCs
    fPrompt =new Double_t[fNFPElem];
    fRand =new Double_t[fNFPElem];
    for(int n=0;n<fNFPElem;n++){
      fPrompt[n]=0.0;
      fRand[n]=0.0;
    }
    fTDCIncSpectrum = new Double_t[fNFPElem];	// Inc ref. spectrum
    fTDCCohSpectrum = new Double_t[fNFPElem];	// Coh spectrum
    fTDCEnhSpectrum = new Double_t[fNFPElem];	// Enhancement spectrum
    fBadTDCChan = new Bool_t[fNFPElem]; 	// Bad scaler channels 
    
    
    fPromptRandRatio=fFP->GetPromptRand();
    
    fFillCounter=0;	// init counter for filling TDC based spectra
    
    if( fTDCRefFileName ){
      ARFile_t tFile( fTDCRefFileName, "r", this );
      channel=0;
      while( (line = tFile.ReadLine()) ){   // scan lines from scaler dump file
	sscanf(line,"%*d%lf",&content);     // get chan and content
	
	if(channel>=fNFPElem) break; 	    // break if past last chan
	fTDCIncSpectrum[fNFPElem-1-channel]=content; //fill array
	channel++;
      } 				    //(increasing E_g order)
      fHaveIncTDC=ETrue;	// flag that we have the Inc. TDC reference
      //run though the tagger TDC channels to find bad channels
      for(int n=0;n<fNFPElem-4;n++){ //flag if less that 1/3 of neighbours	
	if((9*fTDCIncSpectrum[n])<(fTDCIncSpectrum[n+2]+fTDCIncSpectrum[n+3]+fTDCIncSpectrum[n+4])){
	  fBadTDCChan[n]=ETrue;
	}
	if(fTDCIncSpectrum[n]<10.0)fBadTDCChan[n]=ETrue; // or less than 10.0
      }
    }
    else fprintf(fLogStream,"Warning: no TDC file %s \n",fTDCRefFileName);
    
  }
  fInitLevel++;
}

//-----------------------------------------------------------------------------
void TA2Tagger::InitLinPolHist( )
{
  // We need more than one pass at this... 
  // ie an Initialise: must come before and after the Display lines in
  // the setup file for this if we're to get the histograms filled.
  
  Char_t histName[128];

  if( !fIsLinPol ) return;
  sprintf(histName,"%s_CohEdgeChan",fName.Data());
  fHEdge=(TH1F *)f1Dhist->FindObject(histName);
  if(fHEdge!=NULL){
    fHEdge->SetStats(kFALSE);
    fHEdge->GetXaxis()->SetTitle("Scaler Reads");
    fHEdge->GetYaxis()->SetTitle("Edge Position (Chan)");
  }
  sprintf(histName,"%s_CohEdgeEnergy",fName.Data());
  fHEdgeEnergy=(TH1F *)f1Dhist->FindObject(histName);
  if(fHEdgeEnergy!=NULL){
    fHEdgeEnergy->SetStats(kFALSE);
    fHEdgeEnergy->GetXaxis()->SetTitle("Scaler Reads");
    fHEdgeEnergy->GetYaxis()->SetTitle("Edge Position (MeV)");
  }
  sprintf(histName,"%s_Incoherent",fName.Data());
  fHInc=(TH1F *)f1Dhist->FindObject(histName);
  
  sprintf(histName,"%s_Coherent",fName.Data());
  fHCoh=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_Enhancement",fName.Data());
  fHEnh=(TH1F *)f1Dhist->FindObject(histName);
  if(fHEnh!=NULL){
    fHEnh->SetStats(kFALSE);
    fHEnh->SetMinimum(80.0);
    fHEnh->SetMaximum(300.0);
  }
  sprintf(histName,"%s_CoherentPara",fName.Data());
  fHCohPara=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_EnhancementPara",fName.Data());
  fHEnhPara=(TH1F *)f1Dhist->FindObject(histName);
  if(fHEnhPara!=NULL){
    fHEnhPara->SetStats(kFALSE);
    fHEnhPara->SetMinimum(80.0);
    fHEnhPara->SetMaximum(300.0);
  }
  sprintf(histName,"%s_CoherentPerp",fName.Data());
  fHCohPerp=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_EnhancementPerp",fName.Data());
  fHEnhPerp=(TH1F *)f1Dhist->FindObject(histName);
  if(fHEnhPerp!=NULL){
    fHEnhPerp->SetStats(kFALSE);
    fHEnhPerp->SetMinimum(80.0);
    fHEnhPerp->SetMaximum(300.0);
  }
  sprintf(histName,"%s_IncoherentTDC",fName.Data());
  fHTDCInc=(TH1F *)f1Dhist->FindObject(histName);
  
  sprintf(histName,"%s_CoherentTDC",fName.Data());
  fHTDCCoh=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_EnhancementTDC",fName.Data());
  fHTDCEnh=(TH1F *)f1Dhist->FindObject(histName);
  if(fHTDCEnh!=NULL){
    fHTDCEnh->SetStats(kFALSE);
    fHTDCEnh->SetMinimum(80.0);
    fHTDCEnh->SetMaximum(300.0);
  }
  sprintf(histName,"%s_CoherentParaTDC",fName.Data());
  fHTDCCohPara=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_EnhancementParaTDC",fName.Data());
  fHTDCEnhPara=(TH1F *)f1Dhist->FindObject(histName);
  if(fHTDCEnhPara!=NULL){
    fHTDCEnhPara->SetStats(kFALSE);
    fHTDCEnhPara->SetMinimum(80.0);
    fHTDCEnhPara->SetMaximum(300.0);
  }
  
  sprintf(histName,"%s_CoherentPerpTDC",fName.Data());
  fHTDCCohPerp=(TH1F *)f1Dhist->FindObject(histName);
  sprintf(histName,"%s_EnhancementPerpTDC",fName.Data());
  fHTDCEnhPerp=(TH1F *)f1Dhist->FindObject(histName);
  if(fHTDCEnhPerp!=NULL){
    fHTDCEnhPerp->SetStats(kFALSE);
    fHTDCEnhPerp->SetMinimum(80.0);
    fHTDCEnhPerp->SetMaximum(300.0);
  }
  
  sprintf(histName,"%s_Enhancement_v_ScanStep",fName.Data());
  fHScan=(TH2F *)f2Dhist->FindObject(histName);
  if(fHScan!=NULL){
    fHScan->SetStats(kFALSE);
    fHScan->SetMinimum(100);
    fHScan->SetMaximum(300);
  }
  if(fHaveIncScaler){	// if we have a reference
    if(fHInc!=NULL){	// and we have setup a histogram for it
      for(int n=0;n<fNFPElem;n++){
	fHInc->Fill(n,fIncSpectrum[n]);	//fill the histogram
      }
    }
  }
  if(fHaveIncTDC){		// if we have a reference
    if(fHTDCInc!=NULL){	// and we have setup a histogram for it
      for(int n=0;n<fNFPElem;n++){
	fHTDCInc->Fill(n,fTDCIncSpectrum[n]);	//fill the histogram
      }
    }
  }
  fEdgeFit = new TF1("edge",fEdgeFitName,0,10);

  fInitLevel++;
}

//-----------------------------------------------------------------------------
void TA2Tagger::ReconstructLinPol( )
{
  
  int ncount=0;
  int maxchan;
  //  TF1 *edgeFit;
  Double_t fitedge;
  Double_t maxgrad;
  Double_t interp;
  Double_t edgeenergy;

  
  if( fDoingScalers ){ // only for scaler events

    Double_t normValue=0.0;
    fGoniMode = fGoniADC[EGoniModeADC];         //get the goniometer mode
    //Fill the various arrays in ascending E_g order
    for(Int_t n=0;n<fNFPElem;n++){    // fill various hists ascending E_g order
      fCohSpectrum[fNFPElem-1-n] = fScalerCurr[n];
    }
    
    for(Int_t v=fNormChannel-5; v<fNormChannel+5; v++){
      if( (fCohSpectrum[v]>1.0) && (fIncSpectrum[v]>1.0) ){
	normValue += 100.0/(fCohSpectrum[v]/fIncSpectrum[v]);
	ncount++;
      }
    }
    normValue /= ncount;

    if(fHCoh!=NULL)fHCoh->Reset("ICE");	// These have to be reset (ie zeroed)
    if(fHEnh!=NULL)fHEnh->Reset("ICE");
    if((fHCohPara!=NULL)&&(fGoniMode==EPara))fHCohPara->Reset("ICE");
    if((fHEnhPara!=NULL)&&(fGoniMode==EPara))fHEnhPara->Reset("ICE");
    if((fHCohPerp!=NULL)&&(fGoniMode==EPerp))fHCohPerp->Reset("ICE");
    if((fHEnhPerp!=NULL)&&(fGoniMode==EPerp))fHEnhPerp->Reset("ICE");

    if(fGoniMode>=EScan){
      fGoniAOff=fGoniADC[EGoniAOffADC];
      fGoniVOff=fGoniADC[EGoniVOffADC];
      fGoniHOff=fGoniADC[EGoniHOffADC];
      fGoniRadius=fGoniADC[EGoniRadiusADC];
    }
    else if ((fGoniMode==EPara)||(fGoniMode==EPerp)){
      fGoniPlane=fGoniADC[EGoniPlaneADC];
    }
      
    for(int n=0;n<fNFPElem;n++){		       	//fill various hists
      if(fHCoh!=NULL) fHCoh->Fill(n,fCohSpectrum[n]);	//main raw one
      if((fHCohPara!=NULL)&&(fGoniMode==EPara)){	//para raw one
	fHCohPara->Fill(n,fCohSpectrum[n]);	
      }
      if((fHCohPerp!=NULL)&&(fGoniMode==EPerp)){	//perp raw one
	fHCohPerp->Fill(n,fCohSpectrum[n]);	
      }
      if(fHaveIncScaler){		//Now normalise if we have reference
	if((fCohSpectrum[n]<1.0)||(fIncSpectrum[n]<1.0)||(fBadScalerChan[n])){
	  if(n==0)fEnhSpectrum[n]=0;
	  else fEnhSpectrum[n]=  fEnhSpectrum[n-1];
	}
	else{
	  fEnhSpectrum[n]=normValue*fCohSpectrum[n]/fIncSpectrum[n];
	}

	if(fHEnh!=NULL)	fHEnh->Fill(n,fEnhSpectrum[n]);	

	if(fGoniMode>=EScan){   //some sort of scan
	  if(fGoniMode==EScan){ //quazi scam
	    if(fHScan!=NULL){
	      fHScan->Fill(fGoniADC[EGoniStepADC],(fNFPElem+n)/2,fEnhSpectrum[n]);
	    }	
	  }
	  else{
	    if(fHScan!=NULL){  // other scan
	      fHScan->Fill(fGoniADC[EGoniStepADC],n,fEnhSpectrum[n]);
	    }	
	  }
	}


	else{
	  switch(fGoniMode){	//now fill hists etc according to mode
	  case EPara:
	    if(fHEnhPara!=NULL){	//para enhancement one
	      fHEnhPara->Fill(n,fEnhSpectrum[n]);	
	    }
	    break;
	  case EPerp:
	    if(fHEnhPerp!=NULL){	//para enhancement one
	      fHEnhPerp->Fill(n,fEnhSpectrum[n]);	
	    }
	    break;
	  default:
	    break;
	  }
	}
      }
    }
    if(fHaveIncScaler){	//find the coherent edge	
      maxchan=fHEnh->GetMaximumBin();
      //      edgeFit=new TF1("edge","pol4",maxchan-2,maxchan+8);
      //      fEdgeFit->SetRange( maxchan-2, maxchan+8 );
      //      fHEnh->Fit("edge","R0Q");
      fHEnh->Fit( fEdgeFit, "R0Q", "", maxchan+fEdgeChMin, maxchan+fEdgeChMax );
      maxgrad=0.0;
      fitedge=0.0;
      for(float f=maxchan;f<maxchan+10;f+=0.1){
	if(-1.0*( fEdgeFit->Derivative(f)) > maxgrad ){
	  maxgrad=-1.0*(fEdgeFit->Derivative(f));
	  fitedge=f;
	}
      }
      interp=fitedge-(int)fitedge;
      edgeenergy=interp*(fEnergyCalib[(int)fitedge+1]-fEnergyCalib[(int)fitedge]);
      edgeenergy+=fEnergyCalib[(int)fitedge];
      //      delete edgeFit;
      fEdge=fitedge;
      fEdgeEnergy=edgeenergy;
      if(fHEdge){
	if(fScalerCount%(fHEdge->GetNbinsX()-1)==0){
	  fHEdge->Reset("ICE");
	}
	fHEdge->Fill(fScalerCount%(fHEdge->GetNbinsX()-1),fitedge);
      }
      if(fHEdgeEnergy){
	if(fScalerCount%(fHEdgeEnergy->GetNbinsX()-1)==0){
	  fHEdgeEnergy->Reset("ICE");
	}
	fHEdgeEnergy->Fill(fScalerCount%(fHEdge->GetNbinsX()-1),edgeenergy);
      }
      
    }

    if( fDoingTDCs && fLadderWindows ){ 	//if TDC's
      if(fFillCounter++%fFillFreq == 0){       	//are we due a fill ?
	if(fHTDCCoh!=NULL)fHTDCCoh->Reset("ICE");//These have to be zeroed 1st
	if(fHTDCEnh!=NULL)fHTDCEnh->Reset("ICE");
	if((fHTDCCohPara!=NULL)&&(fGoniMode==EPara))fHTDCCohPara->Reset("ICE");
	if((fHTDCEnhPara!=NULL)&&(fGoniMode==EPara))fHTDCEnhPara->Reset("ICE");
	if((fHTDCCohPerp!=NULL)&&(fGoniMode==EPerp))fHTDCCohPerp->Reset("ICE");
	if((fHTDCEnhPerp!=NULL)&&(fGoniMode==EPerp))fHTDCEnhPerp->Reset("ICE");
	for(int n=0;n<fNFPElem;n++){	//do background subtraction
	  fTDCCohSpectrum[n]=fPrompt[n]-(fPromptRandRatio*fRand[n]);
	  if(fHTDCCoh!=NULL) fHTDCCoh->Fill(n,fTDCCohSpectrum[n]);//main raw one
	}
	if(fHaveIncTDC){
	  Double_t TDCnormValue = 0.0;
	  ncount=0;
	  for(int v=fNormChannel-5;v<fNormChannel+5;v++){
	    if((fTDCCohSpectrum[v]>1.0)&&(fTDCIncSpectrum[v]>1.0)){
	      TDCnormValue+=100.0/(fTDCCohSpectrum[v]/fTDCIncSpectrum[v]);
	      ncount++;
	    }
	  }
	  TDCnormValue/=ncount;

	  for(int n=0;n<fNFPElem;n++){
	    if((fTDCCohSpectrum[n]<1.0)||(fTDCIncSpectrum[n]<1.0)||
	       (fBadTDCChan[n])){
	      if(n==0)fTDCEnhSpectrum[n]=0;
	      else fTDCEnhSpectrum[n]=  fTDCEnhSpectrum[n-1];
	    }
	    else{
	      fTDCEnhSpectrum[n] = 
		TDCnormValue*fTDCCohSpectrum[n]/fTDCIncSpectrum[n];
	    }
	    if(fHTDCEnh!=NULL)	fHTDCEnh->Fill(n,fTDCEnhSpectrum[n]);	
	    switch(fGoniMode){	//now fill hists etc according to mode
	    case EPara:
	      if(fHTDCEnhPara!=NULL){	//para enhancement one
		fHTDCEnhPara->Fill(n,fEnhSpectrum[n]);	
	      }
	      break;
	    case EPerp:
	      if(fHTDCEnhPerp!=NULL){	//para enhancement one
		fHTDCEnhPerp->Fill(n,fEnhSpectrum[n]);	
	      }
	      break;
	    default:
	      break;
	    }
	  }
	}
	for(int n=0;n<fNFPElem;n++){	//zero them
	  fPrompt[n]=0.0;
	  fRand[n]=0.0;
	}
      }
    }
    fScalerCount++;
  }

  if((fDoingTDCs)&&(fLadderWindows)){			//if doing TDCs, fill prompt and rand
    for(UInt_t n=0;n<fFP->GetNhits();n++){
      if(fLadderWindows[n]==ELaddPrompt) fPrompt[fNFPElem-1-fLadderHits[n]]++;
      else if(fLadderWindows[n]==ELaddRand) fRand[fNFPElem-1-fLadderHits[n]]++;
    }
  }
}

//-----------------------------------------------------------------------------
void TA2Tagger::ReconstructMicro( )
{
  // Correlate Microscope and main FP hits
}

ClassImp(TA2Tagger)
