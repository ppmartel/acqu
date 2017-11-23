//--Author	Ken Livingston   17th March 2004
//--Rev 	Ken Livingston... sometime 1st production version
//--Update	Ken Livingston...19th Feb 2004...User code
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2LinearPolEpics
//
// User coded version of Crystal Ball apparatus

#include "TA2LinearPolEpics.h"
#include "TF1.h"
#include <iostream>
#include "TCanvas.h"
#include <TSystem.h>
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "TChain.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include <fstream>
#include <TMath.h>
#include <TF1.h>
#include <TMinuit.h>

enum { ELpInitLevel0,ELpInitLevel1,ELpInitLevel2 };  	// allow multiple calls to PostInitialise();
enum { ELpMiscApp, // options for ParseMisc
       ELpMiscPVNames,ELpMiscRads,ELpMiscPolLookupTable, ELpMiscPolCuts, ELpMiscRunRanges };
enum { ELpPolLookupPhotonEnergy,ELpPolLookupEdge, 	//parsing pol lookup table
       ELpPolLookupPol,ELpPolLookupPlane};
enum { ELpPolFromParams, ELpPolFromFile };



static Map_t kValidDetectors[] = {
 {NULL, 		-1}
};



static const Map_t kValidMiscParams[] = {
  {"app",      	ELpMiscApp},
  {"pvnames", 	ELpMiscPVNames},
  {"rads", 	ELpMiscRads},
  {"poltable",  ELpMiscPolLookupTable},
  {"polcuts",   ELpMiscPolCuts},
  {"runs",      ELpMiscRunRanges},
  {NULL,      	-1}
};

static const Map_t kValidPolLookupTableParams[] = {
  {"PhotonEnergy:",      	ELpPolLookupPhotonEnergy},
  {"Edge:",    			ELpPolLookupEdge},
  {"Pol:", 			ELpPolLookupPol}, 
  {"Plane:", 			ELpPolLookupPlane}, 
  {NULL,      	-1}
};

const Double_t k = 26.5601; 
const Int_t VECTORS[]={2,4,6,8,10};    //list of the vectors to be included (022,044);

//fit of a gaussian on a baseline, for gausFit,
Double_t GausOnBase(Double_t *x, Double_t *par) {
  Double_t arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
   Double_t fitval = par[3] + par[0]*TMath::Exp(-0.5*arg*arg);
   return fitval;
}


//-----------------------------------------------------------------------------
TA2LinearPolEpics::TA2LinearPolEpics( const char* name, TA2System* fAnalysis  )
  :TA2Apparatus( name, fAnalysis, kValidDetectors )
{
  fInitLevel=EInitLevel0;	//Flag that no init done yet
  fPolPlane = (UShort_t)ENullHit;
  fPlane = -1;
  fLastEdge = -1.0;
  fEdgeSetting = -1.0;
  fEdgeRange   = -1.0;
  fNRunRanges   = 0;
  fA2LinPolEpicsIndex = 0;
  sprintf(fPlaneString,"Epics"); //Assume plane information from data stream

  sprintf(fCurrentRunFileName,"none"); 	//set to some dummy value
  fIsNewFile = ETrue;			//to begin with it's a new file
  
  fRunNo=0;
  
  fDInc = EBufferEnd;		//make all these Buffer ends, 
  fDCoh = EBufferEnd;		//since they don't ever get filled	
  fDEnh = EBufferEnd;		//by the system, but in the Reconstruct loop directly.
  fDCohPara = EBufferEnd;
  fDEnhPara = EBufferEnd;
  fDCohPerp = EBufferEnd;
  fDEnhPerp = EBufferEnd;
  fDGatedCoherent = EBufferEnd;
  fDGatedCoherentPara = EBufferEnd;
  fDGatedCoherentPerp = EBufferEnd;
  fDGatedCurrEnh = EBufferEnd;
  fDGatedCurrEnhPara = EBufferEnd;
  fDGatedCurrEnhPerp = EBufferEnd;
  
  fDEdge = EBufferEnd;
  fDEdgeGated = EBufferEnd;
  fDEdgePerp = EBufferEnd;
  fDEdgePara = EBufferEnd;
  fDEdgeGatedPara = EBufferEnd;
  fDEdgeGatedPerp = EBufferEnd;
  fDEdgeDistPara = EBufferEnd;
  fDEdgeDistPerp = EBufferEnd;
  fDEdgeEpics = EBufferEnd;
  fDEdgePerpEpics = EBufferEnd;
  fDEdgeParaEpics = EBufferEnd;
  fDEdgeDistParaEpics = EBufferEnd;
  fDEdgeDistPerpEpics = EBufferEnd;
  fDPolTableEnh = EBufferEnd;
  fDPolTablePol = EBufferEnd;
  fDPolMean =  EBufferEnd;
  fDPolCount = EBufferEnd;

  fDoingScalers = EFalse;
  fHaveIncScaler = EFalse;	//Flag that Inc ref scaler is not loaded
  fHaveIncGatedScaler = EFalse;
  fHaveTaggerApp = EFalse;	//Flag that tagger apparatus is not loaded
  fHaveLadderDet = EFalse;	//Flag that ladder detector is not loaded

  fNPolLookupEdges[ETablePara] = 0;  //init no of edge positions for which there are tables
  fNPolLookupEdges[ETablePerp] = 0;  //init no of edge positions for which there are tables
  fNPolLookupEnergies =0;	//init no of photon energies in each table
  fPolLookupEdges[ETablePara] = NULL;      	//no pol lookup edge array yet
  fPolLookupEdges[ETablePerp] = NULL;      	//no pol lookup edge array yet
  fPolLookupEnergies = NULL;	//no pol lookup energy table ceated yet
  fPolLookupPolarisations[ETablePara] = NULL; //no pol  lookup pol tables ceated yet
  fPolLookupPolarisations[ETablePerp] = NULL; //no pol  lookup pol tables ceated yet
    
  fEpics           = NULL;      // init epics stuff
  fEpicsInit       = 0;
  fNEpics          = 0;
  fEpicsBuff       = NULL;
  fEpicsIndex      = NULL;
  fEpicsCount      = 0; 


  fIsEpics         = kTRUE;   //assume epics unless overridden by setup file
  fIsDiamond       = kFALSE;  //assume no diamond to start

  fHaveApp         = kFALSE; //init flags for options in the setup file
  fHaveScaler      = kFALSE; 
  fHaveCbrem       = kFALSE; 
  fHavePVNames     = kFALSE; 
  fHaveRads        = kFALSE; 
  fHaveFit         = kFALSE; 
  fHavePolTable    = kFALSE; 

  fPolTableNRanges[ETablePara] = 0;
  fPolTableNRanges[ETablePerp] = 0;
  

  fNEpicsVariables = 8;
  fLinPolEpicsVariables[0]=&fA2LinPolCohEdgeEpics;  //store the addresses of the epics
  fLinPolEpicsVariables[1]=&fA2LinPolCohRadiatorEpics;  //values in array
  fLinPolEpicsVariables[2]=&fA2LinPolCohPlaneEpics;
  fLinPolEpicsVariables[3]=&fA2GoniAxis1Epics;
  fLinPolEpicsVariables[4]=&fA2GoniAxis2Epics;
  fLinPolEpicsVariables[5]=&fA2GoniAxis3Epics;
  fLinPolEpicsVariables[6]=&fA2GoniAxis4Epics;
  fLinPolEpicsVariables[7]=&fA2GoniAxis5Epics;

  //default values for all in setup file
  //app
  fNormEnergy      = 700.0;  // defaults for Ee=855, peak ~ 350;
  fEdgeMin         = 280.0;
  fEdgeMax         = 420.0;
  sprintf(fTaggerName,"TAGG"); //default to TAGG, FPD
  sprintf(fTaggerName,"FPD");

  //scaler
  sprintf(fRunRefFiles[0],"dats/scaler.dmp");

  //cbrem
  sprintf(fEdgeString,"epics"); //default to using epics for plane and coh edge
  fForceFit=kFALSE;
  sprintf(fPlaneString,"epics");
  fForcePlane=kFALSE;

  //pvnames relating to variables above in  fLinPolEpicsVariables array
  sprintf(fLinPolEpicsPVNames[0],"A2LinPolCohEdge");
  sprintf(fLinPolEpicsPVNames[1],"A2LinPolCohRadiator");
  sprintf(fLinPolEpicsPVNames[2],"A2LinPolCohPlane");
  sprintf(fLinPolEpicsPVNames[3],"A2GoniAxis1");
  sprintf(fLinPolEpicsPVNames[4],"A2GoniAxis2");
  sprintf(fLinPolEpicsPVNames[5],"A2GoniAxis3");
  sprintf(fLinPolEpicsPVNames[6],"A2GoniAxis4");
  sprintf(fLinPolEpicsPVNames[7],"A2GoniAxis5");

  //rads
  fDiamondID       = 4;       //default rads from recent beamtime, override in setup file
  fAmoID           = 2;


  //
  fBeforeEdge      = 150.0;
  fAfterEdge       =  20.0;
  fPolMin          =   0.1;

  ///Fit Enhancement
  fLOWFIT = 400;
  fFitInitialised = kFALSE;
}


//-----------------------------------------------------------------------------
TA2LinearPolEpics::~TA2LinearPolEpics()
{
  // Free up allocated memory
  // (Note: It is safe to delete NULL pointers)
  delete [] fPolLookupEnergies;

  for(int i=0; i<2; ++i) {
      delete fPolLookupEdges[i];
  }

  for(int m=0;m<2;m++){
    if(fPolLookupPolarisations[m]){	
      for(int n=0;n<fNPolLookupEdges[m];n++){
        delete fPolLookupPolarisations[m][n];
      }
      delete [] fPolLookupPolarisations[m];
    }
  }
}

void TA2LinearPolEpics::LoadVariable( )
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

  //                            name        		pointer      	type-spec
  TA2DataManager::LoadVariable("Incoherent", 		&fDInc,    	EDSingleX);
  TA2DataManager::LoadVariable("GatedIncoherent",	&fDGatedInc,   	EDSingleX);
  TA2DataManager::LoadVariable("Coherent", 		&fDCoh,       	EDSingleX);
  TA2DataManager::LoadVariable("Enhancement", 		&fDEnh,      	EDSingleX);
  TA2DataManager::LoadVariable("CoherentPara", 		&fDCohPara,    	EDSingleX);
  TA2DataManager::LoadVariable("EnhancementPara", 	&fDEnhPara,    	EDSingleX);
  TA2DataManager::LoadVariable("CoherentPerp", 		&fDCohPerp,    	EDSingleX);
  TA2DataManager::LoadVariable("EnhancementPerp", 	&fDEnhPerp,    	EDSingleX);
  TA2DataManager::LoadVariable("CohEdge",        	&fDEdge,        EDSingleX);
  TA2DataManager::LoadVariable("CohEdgePara",           &fDEdgePara,  	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgePerp",           &fDEdgePerp,  	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeDistPara",      	&fDEdgeDistPara,       	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeDistPerp",      	&fDEdgeDistPerp,       	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeEpics",   	&fDEdgeEpics,  EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeParaEpics",      &fDEdgeParaEpics,  	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgePerpEpics",      &fDEdgePerpEpics,  	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeDistParaEpics",  &fDEdgeDistParaEpics,       	EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeDistPerpEpics",  &fDEdgeDistPerpEpics,       	EDSingleX);
  TA2DataManager::LoadVariable("PolPlane",        	&fPolPlane,  	EDSingleX);
  TA2DataManager::LoadVariable("PolTableEnh",        	&fDPolTableEnh,  	EDSingleX);
  TA2DataManager::LoadVariable("PolTablePol",        	&fDPolTablePol,  	EDSingleX);
  TA2DataManager::LoadVariable("PolMean",        	&fDPolMean,  	EDSingleX);
  TA2DataManager::LoadVariable("PolCount",        	&fDPolCount,  	EDSingleX);
  TA2DataManager::LoadVariable("GatedCoherent",         &fDGatedCoherent, EDSingleX);
  TA2DataManager::LoadVariable("GatedCoherentPara",     &fDGatedCoherentPara, EDSingleX);
  TA2DataManager::LoadVariable("GatedCoherentPerp",     &fDGatedCoherentPerp, EDSingleX);    
  TA2DataManager::LoadVariable("GatedCurrEnh",          &fDGatedCurrEnh,    EDSingleX);
  TA2DataManager::LoadVariable("GatedCurrEnhPara",      &fDGatedCurrEnhPara,EDSingleX);
  TA2DataManager::LoadVariable("GatedCurrEnhPerp",      &fDGatedCurrEnhPerp,EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeGated",          &fDEdgeGated,   EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeGatedPara",      &fDEdgeGatedPara, EDSingleX);
  TA2DataManager::LoadVariable("CohEdgeGatedPerp",      &fDEdgeGatedPerp, EDSingleX);
  //
}
//-----------------------------------------------------------------------------
void TA2LinearPolEpics::PostInitialise( )
{
	
  //we need more than one pass at this (ie an Initialise: must come before and after the Display lines in
  //the setup file for this if we're to get the histograms filled.
  Char_t histName[100];

  const Double_t *ladderECal;
  Double_t energy;

  switch(fInitLevel){
  case ELpInitLevel0:	//1st pass before Display: to do most of the init
    

    //look for the Tagger
    if((fTagger=(TA2Tagger *)fParent->GetChild(fTaggerName))!=NULL){
      fHaveTaggerApp=ETrue;
    }
    else{
      fprintf(stderr,"Warning: Couldn't find TA2Tagger *%s\n",fTaggerName);
      break;
    }
    if((fLadder=(TA2Ladder *)fTagger->GetChild(fLadderName,"TA2Detector"))!=NULL){
      fHaveLadderDet=ETrue;
      fPolArray = new Double_t[fLadder->GetNelem()];
      fPolArray[0] =  (Double_t)ENullHit;
      fLadderHits = fLadder->GetHits();

      //This version will have the pol for the multihits tagged on at the end
      fPolArrayM = new Double_t[fLadder->GetNelem()];
      fPolArrayM[0] =  (Double_t)ENullHit;
    }
    else{
      fprintf(stderr,"Warning: Couldn't find TA2Ladder *%s\n",fLadderName);
      break;
    }
    
    fBeamEnergy = fTagger->GetBeamEnergy();	//get the beam energy
    fTaggerChannels = fLadder->GetNelem();	//get the no of elements in the Ladder
    fCurrentPolTable = new Double_t[fLadder->GetNelem()]; //get the no of elements in the Ladder
    fCurrentPolTable_TC = new Double_t[fLadder->GetNelem()]; //get the no of elements in the Ladder
    fCurrentEnhTable = new Double_t[fLadder->GetNelem()]; //get the no of elements in the Ladder
    fCurrentEnhTable_TC = new Double_t[fLadder->GetNelem()]; //get the no of elements in the Ladder
    for(UInt_t n=0;n<fLadder->GetNelem();n++){
      fCurrentPolTable[n]	=-1.0;
      fCurrentPolTable_TC[n]=-1.0;
      fCurrentEnhTable[n]	= 0.0;      
      fCurrentEnhTable_TC[n]= 0.0;
    }
    // to allow for summing consecutive scaler buffers to get better stats.
    fAccScaler=new Double_t*[fNScalerBuffers];
    fAccPromptScaler=new Double_t*[fNScalerBuffers];
    fAccRandScaler=new Double_t*[fNScalerBuffers];
    for(int n=0;n<fNScalerBuffers;n++){
      fAccScaler[n]=new Double_t[fLadder->GetNelem()];
      fAccPromptScaler[n]=new Double_t[fLadder->GetNelem()];
      fAccRandScaler[n]=new Double_t[fLadder->GetNelem()];
      for(UInt_t m=0;m<fLadder->GetNelem();m++){
	fAccScaler[n][m]=0.0;
	fAccPromptScaler[n][m]=0.0;
	fAccRandScaler[n][m]=0.0;
      }
    }
    fScalerEvent=0;
    
    fNormChannel=fTaggerChannels/2;			// default norm chan to .5 of range

    if((ladderECal = fLadder->GetECalibration())){	//get the ladder energy calibration
      fEnergyCalib = new Double_t[fTaggerChannels];
      fLadderPhotonEnergy = new Double_t[fTaggerChannels];
      fEnergyBins = new Double_t[fTaggerChannels+1];
      for(int n=0;n<fTaggerChannels;n++){		// fill the photon energy calib
	energy=fBeamEnergy-ladderECal[n];
	fLadderPhotonEnergy[n]=energy;
	fEnergyCalib[fTaggerChannels-1-n]=energy;
      }
      //Make array of bin lower limits for hist axes
      //first bin low edge
      fEnergyBins[0]=fEnergyCalib[0]-((fEnergyCalib[1]-fEnergyCalib[0])/2.0);
      //last bin high edge
      fEnergyBins[fTaggerChannels]=fEnergyCalib[fTaggerChannels-1]+
	((fEnergyCalib[fTaggerChannels-1]-fEnergyCalib[fTaggerChannels-2])/2.0);
      for(int n=1;n<fTaggerChannels;n++){		// fill the photon energy calib
	fEnergyBins[n]=0.5*(fEnergyCalib[n-1]+fEnergyCalib[n]);
      }
    }

    if((fDoingScalers)&&(fLadder->IsScaler())){		// If using scalers
      fIncSpectrum = new Double_t[fTaggerChannels];	// Array to hold Inc ref. spectrum
      fIncGatedSpectrum = new Double_t[fTaggerChannels];// Array to hold Inc gated ref. spectrum
      fCohSpectrum = new Double_t[fTaggerChannels];	// Array to hold Coh spectrum
      fEnhSpectrum = new Double_t[fTaggerChannels];	// Array to hold Enhancement spectrum
      fBadScalerChan = new Bool_t[fTaggerChannels];	// Array to hold bad scaler channels
      fBadGatedScalerChan = new Bool_t[fTaggerChannels];// Array to hold bad scaler channels of gated spectrum
      fRandSubtraction = new Double_t[fTaggerChannels]; // Array to hold random subtraction spectrum
      fAccPromptSpec = new Double_t[fTaggerChannels];       // Array to hold accumulated prompt spectrum of multiple buffer
      fAccRandSpec = new Double_t[fTaggerChannels];         // Array to hold accumulated rand spectrum of multiple buffer
      fGatedCurrEnhSpec = new Double_t[fTaggerChannels];   // Array to hold gated buffered enhancement spectrum
      
      fScalerCurr=fLadder->GetScalerCurr();
      fLadder->fScalerPromptCurr=fLadder->GetScalerPromptCurr();
      fLadder->fScalerRandCurr=fLadder->GetScalerRandCurr();

      LoadAmoRef(fRunRefFiles[0]);					//Load reference amorphous scaler scectrum
    }

    // Only do this if we have an event
    //    if(fIsPolLookupTable){
    // if(LoadPolLookupTable() != 0){
    //	fprintf(stderr,"Warning failed to load PolLookupTable, GetPol() will always return 0\n");
    //  }
    // }
      
    TA2DataManager::PostInit();	//call the Init fron the base class
    break;

  case ELpInitLevel1:	//2nd pass after display to get the ptrs to the histograms
    if((!fHaveTaggerApp)||(!fHaveLadderDet)) break;
    if(!f1Dhist) break;
    sprintf(histName,"%s_CohEdge",fName.Data());
    fHEdge=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdge!=NULL){
      fHEdge->SetStats(kFALSE);
      fHEdge->GetXaxis()->SetTitle("Scaler Reads");
      fHEdge->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdge->SetMinimum(fEdgeMin);
      fHEdge->SetMaximum(fEdgeMax);
    }
    sprintf(histName,"%s_CohEdgePerp",fName.Data());
    fHEdgePerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgePerp!=NULL){
      fHEdgePerp->SetStats(kFALSE);
      fHEdgePerp->GetXaxis()->SetTitle("Scaler Reads");
      fHEdgePerp->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgePerp->SetMinimum(fEdgeMin);
      fHEdgePerp->SetMaximum(fEdgeMax);
      fHEdgePerp->SetLineColor(4);
    }
    sprintf(histName,"%s_CohEdgePara",fName.Data());
    fHEdgePara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgePara!=NULL){
      fHEdgePara->SetStats(kFALSE);
      fHEdgePara->GetXaxis()->SetTitle("Scaler Reads");
      fHEdgePara->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgePara->SetMinimum(fEdgeMin);
      fHEdgePara->SetMaximum(fEdgeMax);
      fHEdgePara->SetLineColor(2);
    }
    sprintf(histName,"%s_CohEdgeDistPerp",fName.Data());
    fHEdgeDistPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeDistPerp!=NULL){
      fHEdgeDistPerp->SetStats(kFALSE);
      fHEdgeDistPerp->GetXaxis()->SetTitle("Coherent Edge Position (MeV)");
      fHEdgeDistPerp->GetYaxis()->SetTitle("Scaler Reads");
      fHEdgeDistPerp->GetXaxis()->Set(fHEdgeDistPerp->GetNbinsX(),fEdgeMin,fEdgeMax);
      fHEdgeDistPerp->SetLineColor(4);
    }
    sprintf(histName,"%s_CohEdgeDistPara",fName.Data());
    fHEdgeDistPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeDistPara!=NULL){
      fHEdgeDistPara->SetStats(kFALSE);
      fHEdgeDistPara->GetXaxis()->SetTitle("Coherent Edge Position (MeV)");
      fHEdgeDistPara->GetYaxis()->SetTitle("Scaler Reads");
      fHEdgeDistPara->GetXaxis()->Set(fHEdgeDistPara->GetNbinsX(),fEdgeMin,fEdgeMax);
      fHEdgeDistPara->SetLineColor(2);
    }

    sprintf(histName,"%s_CohEdgeEpics",fName.Data());
    fHEdgeEpics=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeEpics!=NULL){
      fHEdgeEpics->SetStats(kFALSE);
      fHEdgeEpics->GetXaxis()->SetTitle("Epics Reads");
      fHEdgeEpics->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgeEpics->SetMinimum(fEdgeMin);
      fHEdgeEpics->SetMaximum(fEdgeMax);
    }
    sprintf(histName,"%s_CohEdgePerpEpics",fName.Data());
    fHEdgePerpEpics=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgePerpEpics!=NULL){
      fHEdgePerpEpics->SetStats(kFALSE);
      fHEdgePerpEpics->GetXaxis()->SetTitle("Epics Reads");
      fHEdgePerpEpics->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgePerpEpics->SetMinimum(fEdgeMin);
      fHEdgePerpEpics->SetMaximum(fEdgeMax);
      fHEdgePerpEpics->SetLineColor(4);
    }
    sprintf(histName,"%s_CohEdgeParaEpics",fName.Data());
    fHEdgeParaEpics=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeParaEpics!=NULL){
      fHEdgeParaEpics->SetStats(kFALSE);
      fHEdgeParaEpics->GetXaxis()->SetTitle("Epics Reads");
      fHEdgeParaEpics->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgeParaEpics->SetMinimum(fEdgeMin);
      fHEdgeParaEpics->SetMaximum(fEdgeMax);
      fHEdgeParaEpics->SetLineColor(2);
    }
    sprintf(histName,"%s_CohEdgeDistPerpEpics",fName.Data());
    fHEdgeDistPerpEpics=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeDistPerpEpics!=NULL){
      fHEdgeDistPerpEpics->SetStats(kFALSE);
      fHEdgeDistPerpEpics->GetXaxis()->SetTitle("Coherent Edge Position (MeV)");
      fHEdgeDistPerpEpics->GetYaxis()->SetTitle("Epics Reads");
      fHEdgeDistPerpEpics->GetXaxis()->Set(fHEdgeDistPerpEpics->GetNbinsX(),fEdgeMin,fEdgeMax);
      fHEdgeDistPerpEpics->SetLineColor(4);
    }
    sprintf(histName,"%s_CohEdgeDistParaEpics",fName.Data());
    fHEdgeDistParaEpics=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeDistParaEpics!=NULL){
      fHEdgeDistParaEpics->SetStats(kFALSE);
      fHEdgeDistParaEpics->GetXaxis()->SetTitle("Coherent Edge Position (MeV)");
      fHEdgeDistParaEpics->GetYaxis()->SetTitle("Epics Reads");
      fHEdgeDistParaEpics->GetXaxis()->Set(fHEdgeDistParaEpics->GetNbinsX(),fEdgeMin,fEdgeMax);
      fHEdgeDistParaEpics->SetLineColor(2);
    }

    if((!fHaveTaggerApp)||(!fHaveLadderDet)) break;
    sprintf(histName,"%s_Incoherent",fName.Data());
    fHInc=(TH1F *)f1Dhist->FindObject(histName);
    if(fHInc!=NULL){
      fHInc->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHInc->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedIncoherent",fName.Data());
    fHGatedInc=(TH1F *)f1Dhist->FindObject(histName);
    if(fHInc!=NULL){
      fHGatedInc->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedInc->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_Coherent",fName.Data());
    fHCoh=(TH1F *)f1Dhist->FindObject(histName);
    if(fHCoh!=NULL){
      fHCoh->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHCoh->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_Enhancement",fName.Data());
    fHEnh=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEnh!=NULL){
      fHEnh->SetStats(kFALSE);
      fHEnh->SetMinimum(80.0);
      fHEnh->SetMaximum(300.0);
      fHEnh->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHEnh->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHEnh->SetLineColor(1);
    }
    sprintf(histName,"%s_CoherentPara",fName.Data());
    fHCohPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHCohPara!=NULL){
      fHCohPara->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHCohPara->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHCohPara->SetLineColor(2);
    }
    sprintf(histName,"%s_EnhancementPara",fName.Data());
    fHEnhPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEnhPara!=NULL){
      fHEnhPara->SetStats(kFALSE);
      fHEnhPara->SetMinimum(80.0);
      fHEnhPara->SetMaximum(300.0);
      fHEnhPara->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHEnhPara->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHEnhPara->SetLineColor(2);
    }

    sprintf(histName,"%s_CoherentPerp",fName.Data());
    fHCohPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHCohPerp!=NULL){
      fHCohPerp->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHCohPerp->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHCohPerp->SetLineColor(4);
    }
    sprintf(histName,"%s_EnhancementPerp",fName.Data());
    fHEnhPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEnhPerp!=NULL){
      fHEnhPerp->SetStats(kFALSE);
      fHEnhPerp->SetMinimum(80.0);
      fHEnhPerp->SetMaximum(300.0);
      fHEnhPerp->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHEnhPerp->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHEnhPerp->SetLineColor(4);
    }
    
    if(fHaveIncScaler){	// if we have a reference
      if(fHInc!=NULL){	// and we have setup a histogram for it
	for(int n=0;n<fTaggerChannels;n++){
	  fHInc->Fill(fEnergyBins[n],fIncSpectrum[n]);	//fill the histogram
	}
	fHInc->Sumw2();
      }
    }
    if(fHaveIncGatedScaler){	// if we have a gated reference
      if(fHGatedInc!=NULL){	// and we have setup a histogram for it
	for(int n=0;n<fTaggerChannels;n++){
	  fHGatedInc->Fill(fEnergyBins[n],fIncGatedSpectrum[n]);	//fill the histogram
	}
	fHGatedInc->Sumw2();
      }
    }
    if(fHavePolTable){     //If we have pol lookup tables, make the relevant histograms
      fHistE        = new TH1F("PolTableEnhancement", "Pol Table Enhancement", fTaggerChannels,fEnergyBins );
      fHistP        = new TH1F("PolTablePol", "Pol Table Polarization",fTaggerChannels,fEnergyBins);
      fHistE->SetMinimum(0);
      fHistP->SetMinimum(0);
      fHistP->SetMaximum(1);
      fWeightHist   = new TH1F("weightHist",  "weightHist", THETASTEPS+1, 0, THETASTEPS+1 );
      fThetaPol     = new TH2F("thetaPol",    "thetaPol",   fHistE->GetNbinsX(), fHistE->GetXaxis()->GetXbins()->GetArray(), THETASTEPS+1,0, THETASTEPS+1);
      fThetaItot    = new TH2F("thetaItot",   "thetaItot",  fHistE->GetNbinsX(), fHistE->GetXaxis()->GetXbins()->GetArray(), THETASTEPS+1,0, THETASTEPS+1);
      
    }
    
    sprintf(histName,"%s_PolTableEnh",fName.Data());
    fHPolTableEnh=(TH1F *)f1Dhist->FindObject(histName);
    if(fHPolTableEnh!=NULL){
      fHPolTableEnh->SetStats(kFALSE);
      fHPolTableEnh->SetMinimum(0.0);
      fHPolTableEnh->SetMaximum(6.0);
      fHPolTableEnh->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHPolTableEnh->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHPolTableEnh->SetLineColor(4);
    }
    sprintf(histName,"%s_PolTablePol",fName.Data());
    fHPolTablePol=(TH1F *)f1Dhist->FindObject(histName);
    if(fHPolTablePol!=NULL){
      fHPolTablePol->SetStats(kFALSE);
      fHPolTablePol->SetMinimum(0.0);
      fHPolTablePol->SetMaximum(1.0);
      fHPolTablePol->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHPolTablePol->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHPolTablePol->SetLineColor(4);
    }
    sprintf(histName,"%s_PolMean",fName.Data());
    fHPolMean=(TH1F *)f1Dhist->FindObject(histName);
    if(fHPolMean!=NULL){
      fHPolMean->SetStats(kFALSE);
      fHPolMean->SetMinimum(0.0);
      fHPolMean->SetMaximum(1.0);
      fHPolMean->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHPolMean->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_PolCount",fName.Data());
    fHPolCount=(TH1F *)f1Dhist->FindObject(histName);
    if(fHPolCount!=NULL){
      fHPolCount->SetStats(kFALSE);
      fHPolCount->SetMinimum(0.0);
      fHPolCount->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHPolCount->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedCoherent",fName.Data());
    fHGatedCoherent=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCoherent!=NULL){
      fHGatedCoherent->SetStats(kTRUE);
      fHGatedCoherent->SetMinimum(0.0);
      fHGatedCoherent->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCoherent->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedCoherentPara",fName.Data());
    fHGatedCoherentPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCoherentPara!=NULL){
      fHGatedCoherentPara->SetStats(kTRUE);
      fHGatedCoherentPara->SetMinimum(0.0);
      fHGatedCoherentPara->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCoherentPara->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedCoherentPerp",fName.Data());
    fHGatedCoherentPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCoherentPerp!=NULL){
      fHGatedCoherentPerp->SetStats(kTRUE);
      fHGatedCoherentPerp->SetMinimum(0.0);
      fHGatedCoherentPerp->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCoherentPerp->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedCurrEnh",fName.Data());
    fHGatedCurrEnh=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCurrEnh!=NULL){
      fHGatedCurrEnh->SetStats(kTRUE);
      fHGatedCurrEnh->SetMinimum(0.0);
      fHGatedCurrEnh->SetMaximum(300.0);
      fHGatedCurrEnh->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCurrEnh->GetXaxis()->SetTitle("Photon Energy (MeV)");
    }
    sprintf(histName,"%s_GatedCurrEnhPara",fName.Data());
    fHGatedCurrEnhPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCurrEnhPara!=NULL){
      fHGatedCurrEnhPara->SetStats(kFALSE);
      fHGatedCurrEnhPara->SetMinimum(0.0);
      fHGatedCurrEnhPara->SetMaximum(300.0);
      fHGatedCurrEnhPara->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCurrEnhPara->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHGatedCurrEnhPara->SetLineColor(2);
      fHGatedCurrEnhPara->SetLineStyle(2);
    }
    sprintf(histName,"%s_GatedCurrEnhPerp",fName.Data());
    fHGatedCurrEnhPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHGatedCurrEnhPerp!=NULL){
      fHGatedCurrEnhPerp->SetStats(kFALSE);
      fHGatedCurrEnhPerp->SetMinimum(0.0);
      fHGatedCurrEnhPerp->SetMaximum(300.0);
      fHGatedCurrEnhPerp->GetXaxis()->Set(fTaggerChannels,fEnergyBins);
      fHGatedCurrEnhPerp->GetXaxis()->SetTitle("Photon Energy (MeV)");
      fHGatedCurrEnhPerp->SetLineColor(4);
      fHGatedCurrEnhPerp->SetLineStyle(2);
    }
    sprintf(histName,"%s_CohEdgeGated",fName.Data());
    fHEdgeGated=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeGated!=NULL){
      fHEdgeGated->SetStats(kFALSE);
      fHEdgeGated->GetXaxis()->SetTitle("Scaler Reads");
      fHEdgeGated->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgeGated->SetMinimum(fEdgeMin);
      fHEdgeGated->SetMaximum(fEdgeMax);
    }
    sprintf(histName,"%s_CohEdgeGatedPerp",fName.Data());
    fHEdgeGatedPerp=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeGatedPerp!=NULL){
      fHEdgeGatedPerp->SetStats(kFALSE);
      fHEdgeGatedPerp->GetXaxis()->SetTitle("Scaler Reads");
      fHEdgeGatedPerp->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgeGatedPerp->SetMinimum(fEdgeMin);
      fHEdgeGatedPerp->SetMaximum(fEdgeMax);
      fHEdgeGatedPerp->SetLineColor(4);
      fHEdgeGatedPerp->SetLineStyle(2);
    }
    sprintf(histName,"%s_CohEdgeGatedPara",fName.Data());
    fHEdgeGatedPara=(TH1F *)f1Dhist->FindObject(histName);
    if(fHEdgeGatedPara!=NULL){
      fHEdgeGatedPara->SetStats(kFALSE);
      fHEdgeGatedPara->GetXaxis()->SetTitle("Scaler Reads");
      fHEdgeGatedPara->GetYaxis()->SetTitle("Edge Position (MeV)");
      fHEdgeGatedPara->SetMinimum(fEdgeMin);
      fHEdgeGatedPara->SetMaximum(fEdgeMax);
      fHEdgeGatedPara->SetLineColor(2);
      fHEdgeGatedPara->SetLineStyle(2);
    }
    
  default:
    break;
  }
  fInitLevel++;
  fIsInit=ETrue;  
}

//-----------------------------------------------------------------------------
void TA2LinearPolEpics::Reconstruct( ){
  
  Double_t normValue=0.0;
  Double_t normValueGated=0.0;
  int ncount=0;
  int ngcount=0;
  TF1 *edgeFit=NULL;
  TF1 *edgeFitGated=NULL;
  Double_t coh_sum=0.0;
  int binx=0;
  Double_t xmax=0.0; 
  Double_t ymax=0.0;
  Double_t errordummy1, errordummy2;
  
  fLastPolRangeIndex[ETablePara]=-1;
  fLastPolRangeIndex[ETablePerp]=-1;
  fLastRunRangeIndex=-1;
  
  
  if(gAR->IsEpicsRead()||gAR->IsScalerRead()){
  
    if(strcmp(gAR->GetFileName(),fCurrentRunFileName)!=0){    //see if it's a new file
      strcpy(fCurrentRunFileName,gAR->GetFileName()); 
      fIsNewFile=ETrue;	//this stays flagged until we get a scaler or epics read
      
      fPolRangeIndex[ETablePara]=-1;
      fPolRangeIndex[ETablePerp]=-1;
      fRunRangeIndex=-1;
      
      sscanf(strrchr(fCurrentRunFileName,'_'),"_%d.dat",&fRunNo);  //get the run no from filename    
      if(fHavePolTable){
	for(int index=0;index<fPolTableNRanges[ETablePara];index++){ //find the pol table run range for this run
	  if((fRunNo>=fPolTableRangesMin[index][ETablePara])&&(fRunNo<=fPolTableRangesMax[index][ETablePara])){
	    fPolRangeIndex[ETablePara]=index;
	    break;
	  }
	}
	if(fPolRangeIndex[ETablePara]<0){
	fprintf(stderr,"Fatal Error: No para pol Table run range for run %d",fRunNo);
	// not a good way to exit (it will segfault)
	exit(1);
	}
	for(int index=0;index<fPolTableNRanges[ETablePerp];index++){ //find the pol table run range for this run
	  if((fRunNo>=fPolTableRangesMin[index][ETablePerp])&&(fRunNo<=fPolTableRangesMax[index][ETablePerp])){
	    fPolRangeIndex[ETablePerp]=index;
	    break;
	  }
	}
	if(fPolRangeIndex[ETablePerp]<0){
	fprintf(stderr,"Fatal Error: No perp Pol Table run range for run %d",fRunNo);
	// not a good way to exit (it will segfault)
	exit(1);
	}
      }
      for(int index=0;index<fNRunRanges;index++){ //find the run table run range for this run
	if((fRunNo>=fRunRangeMin[index])&&(fRunNo<=fRunRangeMax[index])){
	  fRunRangeIndex=index;
	  break;
	}
      }
      if(fRunRangeIndex>-1) {
	LoadAmoRef(fRunRefFiles[fRunRangeIndex]);  //Load new reference amorphous scaler scectrum 
	if(fForcePlane){
	  fPlane=fRunPlanes[fRunRangeIndex];
	}
      }
      else{
		fprintf(stderr,"Fatal Error: No plane info range for run %d",fRunNo);
		// not a good way to exit (it will segfault)
		exit(1);
      }
    }
    
    if(fIsEpics){   //if wer're using epics buffers to get coherent brem info
      ////////////////////////////////////////////////////////////////////////////
      // Deal with EPICS event
      //
      ////////////////////////////////////////////////////////////////////////////
      
      if (gAR->IsEpicsRead()){                   // check if epics buffer
	if(fEpicsInit == kFALSE){                // initialise if not done
	  fNEpics = gAR->GetNEpics();
	  fEpicsBuff = gAR->GetEpicsBuffer();
	  fEpicsIndex = gAR->GetEpicsIndex();
	  fEpics = new TEPICSmodule();
	  fEpicsInit = kTRUE;
	}
	
	// the rest is specific to what epics buffer and channels you need to read.
	if(fEpicsIndex[fA2LinPolEpicsIndex]){  // if the epics buffer has the index required for lin pol data
	  
	  //loop over all the variables 
	  for(Int_t nepics=0;nepics<fNEpicsVariables;nepics++){
	    //returns pointer to the data for the channel if you want to do something smart
	    fEpicsChannelBuffer = fEpics->GetChannel((Char_t*)fLinPolEpicsPVNames[nepics],     //pv name
						     &fEpicsType,                     //pv type
						     fEpicsBuff[fA2LinPolEpicsIndex], //start of epics buffer
						     fLinPolEpicsVariables[nepics],   //address of the variable to be filled
						     &fEpicsNElem);                   //no of elements in the channel (usually singles)
	  }
	  //fEpics->DumpBuffer(fEpicsBuff[fA2LinPolEpicsIndex]);
	}
	
	if(!fForcePlane){                              // if not forcing plane in setup file 
	  fPlane=(Int_t)fA2LinPolCohPlaneEpics;        //set plane and rad from epics
	  fRadiator=(Int_t)fA2LinPolCohRadiatorEpics;  
	  if(fRadiator==fDiamondID){
	    fIsDiamond=kTRUE;
	  }
	  else{
	    fIsDiamond=kFALSE;
	  }
	}
	if(!fForceFit){                              // if not fitting enhancements 
	  fEdge=fA2LinPolCohEdgeEpics;               // get edge from epics
	}
      }
      
      
      if(fHEdgeEpics){
	if(fEpicsCount%(fHEdgeEpics->GetNbinsX()-1)==0){
	  fHEdgeEpics->Reset("ICE");
	}
	fHEdgeEpics->Fill(fEpicsCount%(fHEdgeEpics->GetNbinsX()-1),fA2LinPolCohEdgeEpics);
      }
      if(fHEdgeParaEpics){
	if(fEpicsCount%(fHEdgeEpics->GetNbinsX()-1)==0){
	  fHEdgeParaEpics->Reset("ICE");
	}
	if(fPlane==ETablePara){
	  fHEdgeParaEpics->Fill(fEpicsCount%(fHEdgeParaEpics->GetNbinsX()-1),fA2LinPolCohEdgeEpics);
	}
	else{
	  fHEdgeParaEpics->Fill(fEpicsCount%(fHEdgeParaEpics->GetNbinsX()-1),0);
	}
	
      }
      if(fHEdgePerpEpics){
	if(fEpicsCount%(fHEdgePerpEpics->GetNbinsX()-1)==0){
	  fHEdgePerpEpics->Reset("ICE");
	}
	if(fPlane==ETablePerp){
	  fHEdgePerpEpics->Fill(fEpicsCount%(fHEdgePerpEpics->GetNbinsX()-1),fA2LinPolCohEdgeEpics);
	}
	else{
	  fHEdgePerpEpics->Fill(fEpicsCount%(fHEdgePerpEpics->GetNbinsX()-1),0);
	}
      }
      if((fPlane==ETablePara)&&(fHEdgeDistParaEpics)){
	fHEdgeDistParaEpics->Fill(fA2LinPolCohEdgeEpics);
      }
      else if((fPlane==ETablePerp)&&(fHEdgeDistPerpEpics)){
	fHEdgeDistPerpEpics->Fill(fA2LinPolCohEdgeEpics);
      }
      
      
      fIsNewFile=EFalse;                              //no longer a new file
      fEpicsCount++;                                  //increment the no of epics events
    } 
    // end of epics section
    
    if(gAR->IsScalerRead()&&(!fIsEpics)){            //if scaler read, and no epics events
      fIsNewFile=EFalse;	                           //no longer a new file
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    ///                                                                                       ///
    ///                       only do this bit for scaler events                              ///
    ///                                                                                       ///
    /////////////////////////////////////////////////////////////////////////////////////////////
    
    if((fDoingScalers)&&(gAR->IsScalerRead())){ 	   
      
      //Fill the various arrays in ascending E_g order
      if((fScalerEvent%fNScalerBuffers)==0){
	for(int n=0;n<fNScalerBuffers;n++){
	  for(UInt_t m=0;m<fLadder->GetNelem();m++){
	    fAccScaler[n][m]=0.0;
	    fAccPromptScaler[n][m]=0.0;
	    fAccRandScaler[n][m]=0.0;
	  }
	}
      }
      
      for(int n=0;n<fTaggerChannels;n++){	           //fill various arrays for hists (in ascending E_g order)
	fAccScaler[fScalerEvent%fNScalerBuffers][fTaggerChannels-1-n]=fScalerCurr[n];
	fAccPromptScaler[fScalerEvent%fNScalerBuffers][fTaggerChannels-1-n]=fLadder->fScalerPromptCurr[n];
	fAccRandScaler[fScalerEvent%fNScalerBuffers][fTaggerChannels-1-n]=fLadder->fScalerRandCurr[n];
      }
      
      for(int n=0;n<fTaggerChannels;n++){
	fCohSpectrum[n]=0.0;
	fAccPromptSpec[n]=0.0;
	fAccRandSpec[n]=0.0;
	fRandSubtraction[n]=0.0;
	fGatedCurrEnhSpec[n]=0.0;
      }
      for(int b=0;b<fNScalerBuffers;b++){
	for(int n=0;n<fTaggerChannels;n++){
	  fCohSpectrum[n]+=fAccScaler[b][n];
	  coh_sum+=fAccScaler[b][n];
	}
      }
      
      for(int b=0;b<fNScalerBuffers;b++){
	for(int n=0;n<fTaggerChannels;n++){
	  fAccPromptSpec[n]+=fAccPromptScaler[b][n];
	  fAccRandSpec[n]+=fAccRandScaler[b][n];
	  fRandSubtraction[n]=fAccPromptSpec[n]-fAccRandSpec[n];
	}
      }

      
      if((fScalerEvent%fNScalerBuffers)==0){                 /////only do this every N buffers
      
	for(int v=fNormChannel-10;v<fNormChannel+10;v++){
	  if((fCohSpectrum[v]>1.0)&&(fIncSpectrum[v]>1.0)){
	    normValue+=100.0/(fCohSpectrum[v]/fIncSpectrum[v]);
	    ncount++;
	  }
	  if((fRandSubtraction[v]>1.0)&&(fIncGatedSpectrum[v]>1.0)){
	    normValueGated+=100.0/(fRandSubtraction[v]/fIncGatedSpectrum[v]);
	    ngcount++;
	  }
	}
	normValue/=ncount;
	normValueGated/=ngcount;
      
	if(fHCoh!=NULL)fHCoh->Reset("ICE");	//These have to be reset (ie zeroed) first
	if(fHEnh!=NULL)fHEnh->Reset("ICE");
	if((fHCohPara!=NULL)&&(fPlane==ETablePara))fHCohPara->Reset("ICE");
	if((fHEnhPara!=NULL)&&(fPlane==ETablePara))fHEnhPara->Reset("ICE");
	if((fHCohPerp!=NULL)&&(fPlane==ETablePerp))fHCohPerp->Reset("ICE");
	if((fHEnhPerp!=NULL)&&(fPlane==ETablePerp))fHEnhPerp->Reset("ICE");
	if(fHGatedCoherent!=NULL)fHGatedCoherent->Reset("ICES");
	if(fHGatedCoherentPara!=NULL)fHGatedCoherentPara->Reset("ICES");
	if(fHGatedCoherentPerp!=NULL)fHGatedCoherentPerp->Reset("ICES");
	if(fHGatedCurrEnh!=NULL)fHGatedCurrEnh->Reset("ICES");
	if(fHGatedCurrEnhPara!=NULL)fHGatedCurrEnhPara->Reset("ICE");
	if(fHGatedCurrEnhPerp!=NULL)fHGatedCurrEnhPerp->Reset("ICE");     
      
	for(int n=0;n<fTaggerChannels;n++){     /////////////////////////////////////// calculate/fill various hists
	  if(fHCoh!=NULL){
	    fHCoh->Fill(fEnergyBins[n],fCohSpectrum[n]);		//main raw one
	  }
	  if((fHCohPara!=NULL)&&(fPlane==ETablePara)){		//para raw one
	    fHCohPara->Fill(fEnergyBins[n],fCohSpectrum[n]);
	  }
	  if((fHCohPerp!=NULL)&&(fPlane==ETablePerp)){		//perp raw one
	    fHCohPerp->Fill(fEnergyBins[n],fCohSpectrum[n]);
	  }

	  //random Subtraction
	  if(fHGatedCoherent!=NULL){
	    fHGatedCoherent->Fill(fEnergyBins[n],fRandSubtraction[n]);
	  }
	  if((fHGatedCoherentPara!=NULL)&&(fPlane==ETablePara)){		//para
	    fHGatedCoherentPara->Fill(fEnergyBins[n],fRandSubtraction[n]);
	  }
	  if((fHGatedCoherentPerp!=NULL)&&(fPlane==ETablePerp)){		//perp
	    fHGatedCoherentPerp->Fill(fEnergyBins[n],fRandSubtraction[n]);
	  }

	  if(fHaveIncScaler){		//Now normalise if we have reference
	    if((fCohSpectrum[n]<1.0)||(fIncSpectrum[n]<1.0)||(fBadScalerChan[n])){
	      if(n==0)fEnhSpectrum[n]=0;
	      else fEnhSpectrum[n]=  fEnhSpectrum[n-1];
	    }
	    else{
	      fEnhSpectrum[n]=normValue*fCohSpectrum[n]/fIncSpectrum[n];
	    }		  
	  }
	  if(fHaveIncGatedScaler){             //Now normalise if we have gated reference
	    if((fRandSubtraction[n]<1.0)||(fIncGatedSpectrum[n]<1.0)||(fBadGatedScalerChan[n])){ //calculate normalised gated enh
	      if(n==0)fGatedCurrEnhSpec[n]=0;
	      else fGatedCurrEnhSpec[n]=fGatedCurrEnhSpec[n-1];
	    }
	    else{
	      fGatedCurrEnhSpec[n]=normValueGated*fRandSubtraction[n]/fIncGatedSpectrum[n];
	    }
	  }
	} //end loop over all tagger channels for filling

	// make enhancements
	if(fHEnh!=NULL)  fHEnh->Divide(fHCoh,fHInc,normValue);
	switch(fPlane){	//now fill hists etc according to mode
	case ETablePara:
	  if(fHEnhPara!=NULL){	//para enhancement one
	    fHEnhPara->Divide(fHCoh,fHInc,normValue);	
	  }
	  break;
	case ETablePerp:
	  if(fHEnhPerp!=NULL){	//para enhancement one
	    fHEnhPerp->Divide(fHCoh,fHInc,normValue);
	  }
	  break;
	default:
	  break;
	}
	
	if(fHGatedCurrEnh!=NULL)  fHGatedCurrEnh->Divide(fHGatedCoherent,fHGatedInc,normValueGated);
	switch(fPlane){	//now fill hists etc according to mode
	case ETablePara:
	  if(fHGatedCurrEnhPara!=NULL){	//para enhancement one
	    fHGatedCurrEnhPara->Divide(fHGatedCoherent,fHGatedInc,normValueGated);	
	  }
	  break;
	case ETablePerp:
	  if(fHGatedCurrEnhPerp!=NULL){	//para enhancement one
	    fHGatedCurrEnhPerp->Divide(fHGatedCoherent,fHGatedInc,normValueGated);	
	  }
	  break;
	default:
	  break;
	}
	
      
	if((fHaveIncScaler)&&(fHEnh)){			//find the coherent edge
		 
	  if(fPlane == ETableAmo){
	    fEdge = 0;
	    fEdgeError = 0;
	  }
	  else
	    {
	    
	      binx=0;
	      ymax=0;
	      for(int b=fHEnh->FindBin(fEdgeMin);b<fHEnh->FindBin(fEdgeMax);b++){
		if(fHEnh->GetBinContent(b)>ymax){
		  ymax=fHEnh->GetBinContent(b);
		  binx=b;
		}
	      }
	      xmax=fHEnh->GetBinCenter(binx);
	      ymax=fHEnh->GetBinContent(binx);
	    
	    
	      if(!edgeFit)edgeFit=new TF1("edgeFit",GausOnBase,0,100,4);
	    
	      edgeFit->SetRange(xmax,xmax+40.0);
	      edgeFit->SetParameter(1,xmax);
	      edgeFit->SetParameter(2,10.0);
	      edgeFit->SetParameter(3,100.0);
	      fHEnh->Fit(edgeFit,"QNR");
	 
	    
	      errordummy1 = edgeFit->GetParError(1);
	      errordummy2 = edgeFit->GetParError(2); 
	    
	      fEdge = (edgeFit->GetParameter(1)) + abs(edgeFit->GetParameter(2));
	      fEdgeError = sqrt(errordummy1*errordummy1 + errordummy2*errordummy2);

	    }
	  if(fHEdge){
	    if(fScalerEvent%(fHEdge->GetNbinsX()-1)==0){
	      fHEdge->Reset("ICE");
	    }
	    fHEdge->SetBinContent(fScalerEvent%(fHEdge->GetNbinsX()-1),fEdge);
	    fHEdge->SetBinError(fScalerEvent%(fHEdge->GetNbinsX()-1),fEdgeError);
	  }
	  if(fHEdgePara){
	    if(fScalerEvent%(fHEdgePara->GetNbinsX()-1)==0){
	      fHEdgePara->Reset("ICE");
	    }
	    if(fPlane==ETablePara){
	      fHEdgePara->SetBinContent(fScalerEvent%(fHEdgePara->GetNbinsX()-1),fEdge);
	      fHEdgePara->SetBinError(fScalerEvent%(fHEdge->GetNbinsX()-1),fEdgeError);
	    }
	    else{
	      fHEdgePara->Fill(fScalerEvent%(fHEdgePara->GetNbinsX()-1),0);
	    }	  
	  }
	  if(fHEdgePerp){
	    if(fScalerEvent%(fHEdgePerp->GetNbinsX()-1)==0){
	      fHEdgePerp->Reset("ICE");
	    }
	    if(fPlane==ETablePerp){
	      fHEdgePerp->SetBinContent(fScalerEvent%(fHEdgePerp->GetNbinsX()-1),fEdge);
	      fHEdgePerp->SetBinError(fScalerEvent%(fHEdge->GetNbinsX()-1),fEdgeError);
	    }
	    else{
	      fHEdgePerp->Fill(fScalerEvent%(fHEdgePerp->GetNbinsX()-1),0);
	    }
	  }
	  if((fPlane==ETablePara)&&(fHEdgeDistPara)){
	    fHEdgeDistPara->Fill(fEdge);
	  }
	  else if((fPlane==ETablePerp)&&(fHEdgeDistPerp)){
	    fHEdgeDistPerp->Fill(fEdge);
	  }
	  if(!fForceFit){                              // if not fitting enhancements 
	    fEdge=fA2LinPolCohEdgeEpics;               // get edge from epics
	  }
	  GetPolDegree(200.0);                         //force it to set up a table
	
	}//end find edge part

	if((fHaveIncGatedScaler)&&(fHGatedCurrEnh)){	       	//find the gated coherent edge		 
	  if(fPlane == ETableAmo){
	    fEdgeGated = 0;
	    fEdgeGatedError = 0;
	  }
	  else
	    {
	      binx=0;
	      ymax=0;
	      for(int b=fHGatedCurrEnh->FindBin(fEdgeMin);b<fHGatedCurrEnh->FindBin(fEdgeMax);b++){
		if(fHGatedCurrEnh->GetBinContent(b)>ymax){
		  ymax=fHGatedCurrEnh->GetBinContent(b);
		  binx=b;
		}
	      }
	      xmax=fHGatedCurrEnh->GetBinCenter(binx);
	      ymax=fHGatedCurrEnh->GetBinContent(binx);
	    
	    
	      if(!edgeFitGated)edgeFitGated=new TF1("edgeFitGated",GausOnBase,0,100,4);
	    
	      edgeFitGated->SetRange(xmax-5.0,xmax+40.0);
	      edgeFitGated->SetParameter(0,ymax/10.0);
	      edgeFitGated->SetParameter(1,xmax);
	      edgeFitGated->SetParLimits(1,xmax-20,xmax+20);
	      edgeFitGated->SetParameter(2,10.0);
	      edgeFitGated->SetParLimits(2,0,50);
	      edgeFitGated->SetParameter(3,100.0);
	      fHGatedCurrEnh->Fit(edgeFitGated,"QNR");
	    
	      errordummy1 = edgeFitGated->GetParError(1);
	      errordummy2 = edgeFitGated->GetParError(2);
	    
	      fEdgeGated = (edgeFitGated->GetParameter(1)) + abs(edgeFitGated->GetParameter(2));
	      fEdgeGatedError = sqrt(errordummy1*errordummy1 + errordummy2*errordummy2);
	    }
	  if(fHEdgeGated){
	    if(fScalerEvent%(fHEdgeGated->GetNbinsX()-1)==0){
	      fHEdgeGated->Reset("ICE");
	    }
	    fHEdgeGated->SetBinContent(fScalerEvent%(fHEdgeGated->GetNbinsX()-1),fEdgeGated);
	    fHEdgeGated->SetBinError(fScalerEvent%(fHEdgeGated->GetNbinsX()-1),fEdgeGatedError);
	  }
	  if(fHEdgeGatedPara){
	    if(fScalerEvent%(fHEdgeGatedPara->GetNbinsX()-1)==0){
	      fHEdgeGatedPara->Reset("ICE");
	    }
	    if(fPlane==ETablePara){
	      fHEdgeGatedPara->SetBinContent(fScalerEvent%(fHEdgeGatedPara->GetNbinsX()-1),fEdgeGated);
	      fHEdgeGatedPara->SetBinError(fScalerEvent%(fHEdgeGated->GetNbinsX()-1),fEdgeGatedError);
	    }
	    else{
	      fHEdgeGatedPara->Fill(fScalerEvent%(fHEdgeGatedPara->GetNbinsX()-1),0);
	    }	  
	  }
	  if(fHEdgeGatedPerp){
	    if(fScalerEvent%(fHEdgeGatedPerp->GetNbinsX()-1)==0){
	      fHEdgeGatedPerp->Reset("ICE");
	    }
	    if(fPlane==ETablePerp){
	      fHEdgeGatedPerp->SetBinContent(fScalerEvent%(fHEdgeGatedPerp->GetNbinsX()-1),fEdgeGated);
	      fHEdgeGatedPerp->SetBinError(fScalerEvent%(fHEdgeGated->GetNbinsX()-1),fEdgeGatedError);
	    }
	    else{
	      fHEdgeGatedPerp->Fill(fScalerEvent%(fHEdgeGatedPerp->GetNbinsX()-1),0);
	    }
	  }
        
	  if(!fForceFit){                              // if not fitting enhancements 
	    fEdgeGated=fA2LinPolCohEdgeEpics;               // get edge from epics
	  }
	  GetPolDegree(200.0);                         //force it to set up a table
	
	}//end find gated edge

      
	/////////////////////////////////
	/////////Fit Enhancement/////////
	/////////////////////////////////
	/* if((fScalerEvent%30)==0){
	   FitEnhancement(fHGatedCurrEnh, 0.006, 2);
	   }*/

	
	
      }//end if clause for buffer size
      fScalerEvent++;
    } //end scaler part
  }

  else{
    if(fIsNewFile==ETrue) return;
    FillPolArray();
  }
}


void TA2LinearPolEpics::ParseMisc(char *line){	// read parameters in the setup file

  Char_t miscType[10];
  Char_t tempString[80];
  Char_t planeString[10];
  Int_t type=-1;
  Int_t plane=-1;
  Int_t rangeMin=-1;
  Int_t rangeMax=-1;

  // Use "wildcard" input line to read params for linear pol monitoring
  if(sscanf(line, "%s", miscType) != 1) PrintError( line, "Linear Pol  parameters" );

  type=Map2Key(miscType,kValidMiscParams);
  switch(type){
  case ELpMiscApp:
    if(sscanf(line, "%*s%lf%lf%lf%s%s%s%d%s%s%lf",&fNormEnergy,&fEdgeMin,&fEdgeMax,fTaggerName,fLadderName,
	      fRunRefFiles[0],&fNScalerBuffers,fEdgeString,fPlaneString,&fDeadband)!=10){
      PrintError( line, "Linear Pol app  parameters" );
    }
    fHaveApp=kTRUE;
    fDoingScalers = kTRUE;
    fHaveScaler   = kTRUE;
    if(strcmp(fEdgeString,"fit")==0){	// get edge from fit
      fForceFit=kTRUE;
    }
    else{
      fForceFit=kFALSE;
    }
    if(strcmp(fPlaneString,"file")==0){	// get plane from setup file
      fForcePlane=kTRUE;
    }
    else{
      fForcePlane=kFALSE;
    }
    if((fForceFit)&&(fForcePlane)){    //  in this case we don't use any epics
      fIsEpics=kFALSE;
    }
    fHaveCbrem = kTRUE;
    break;
    
  case ELpMiscRunRanges:
    if(sscanf(line, "%*s%d%d%s%s%lf%lf",&fRunRangeMin[fNRunRanges],&fRunRangeMax[fNRunRanges],
	      fRunRefFiles[fNRunRanges],planeString,&fEdgeSetting, &fEdgeRange)!=6){
      PrintError( line, "Linear Pol Run Range parameters" );
    }
    if(strcmp(planeString,"para")==0){
      fRunPlanes[fNRunRanges]=ETablePara;
    }
    else if(strcmp(planeString,"perp")==0){
      fRunPlanes[fNRunRanges]=ETablePerp;
    }
    else{
      fRunPlanes[fNRunRanges]=ETableAmo;
    }
    fNRunRanges++;
    
    break;
    
  case ELpMiscPolCuts:
    if(sscanf(line, "%*s%lf%lf%lf%lf",&fBeforeEdge,&fAfterEdge,&fPolMin,&fDeadband)!=4){
      PrintError( line, "Linear Pol PolLookupTable parameters" );
    }
    break;
    
  case ELpMiscPolLookupTable:
    //like this:
    //#Misc:   poltable  params       12345     12346 PARA 0.001790 0.000223 6.339087 0.008871 1557.000000 2.000000 6.045563 1.341427 0.000000 0.000000 4.271005
    //#Misc:   poltable  filename.dat 12345     12346
    
    if(sscanf(line, "%*s%d%d%s",&rangeMin,&rangeMax,tempString)!=3){
      PrintError( line, "Linear Pol PolLookupTable parameters" );
    }
    
    if(strcmp(tempString,"params")==0){
      sscanf(line,"%*s%*d%*d%*s%s",tempString);
      if(strcmp(tempString,"para")==0){
	plane=ETablePara;
      }
      else{
	plane=ETablePerp;
      }
      
      fPolTableTypes[fPolTableNRanges[plane]][plane]=ELpPolFromParams;
      fPolTableRangesMin[fPolTableNRanges[plane]][plane]=rangeMin;
      fPolTableRangesMax[fPolTableNRanges[plane]][plane]=rangeMax;
      
      sscanf(line,"%*s%*d%*d%*s%*s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
	     &fPolTableParams[fPolTableNRanges[plane]][plane][0],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][1],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][2],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][3],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][4],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][5],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][6],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][7],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][8],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][9],
	     &fPolTableParams[fPolTableNRanges[plane]][plane][10]);
    }
    else{
      fPolTableTypes[fPolTableNRanges[plane]][plane]=ELpPolFromFile;
      //do file stuff
    }
    
    fPolTableNRanges[plane]++;  //increment the no of ranges for this plane
    fHavePolTable = kTRUE;
    
    break;
  case ELpMiscPVNames:
    if(sscanf(line,"%*s%s%s%s%s%s%s%s%s",       //get read into these variables
	      fLinPolEpicsPVNames[0],           //&fA2LinPolCohEdgeEpics
	      fLinPolEpicsPVNames[1],           //&fA2LinPolCohRadiatorEpics
	      fLinPolEpicsPVNames[2],           //&fA2LinPolCohPlaneEpics
	      fLinPolEpicsPVNames[3],           //&fA2GoniAxis1Epics1
	      fLinPolEpicsPVNames[4],           //&fA2GoniAxis1Epics2
	      fLinPolEpicsPVNames[5],           //&fA2GoniAxis1Epics3
	      fLinPolEpicsPVNames[6],           //&fA2GoniAxis1Epics4
	      fLinPolEpicsPVNames[7])!=8){      //&fA2GoniAxis1Epics5
      PrintError( line, "Linear Pol pvname parameters" );
    }
    fHavePVNames = kTRUE;    
    break;
  case ELpMiscRads:
    if(sscanf(line,"%*s%d%d",&fDiamondID,&fAmoID)!=2){
      PrintError( line, "Linear Pol rads parameters" );
    }
    fHaveRads = kTRUE;
    break;
  default:
    break;
  }
}


int TA2LinearPolEpics::LoadAmoRef(Char_t *refFileName){
  FILE *fp;
  Char_t line[82];
  Int_t channel;
  Double_t content;
  Double_t gateRef; //difference between prompt and rand ref
  Bool_t IsFlagged; //File has a column with gated reference

  if((fp=fopen(refFileName,"r"))!=NULL){
    channel=0;
    for(;;){                            		// scan lines from scaler dump file
      if(fgets(line,80,fp) == NULL) break;		// check got a line from file
      if((line[0] == '*')||(line[0] == '#')) continue;
      if((line[0] == 'F')){
	fprintf(stderr,"Flag detected! Collimated data in reference file!\n");
	IsFlagged=kTRUE;
	continue;
      }
      if(!IsFlagged){
	sscanf(line,"%*d%lf",&content);         // get chan and contents
	
	if(channel>=fTaggerChannels) break; 		// break if past last chan
	fIncSpectrum[fTaggerChannels-1-channel]=content; //fill array
      }
      if(IsFlagged){
       	sscanf(line,"%*d%lf%lf",&content,&gateRef);

	if(channel>=fTaggerChannels) break; 		// break if past last chan
	fIncSpectrum[fTaggerChannels-1-channel]=content; //fill arrays
        fIncGatedSpectrum[fTaggerChannels-1-channel]=gateRef;
      }
      channel++;
    } 						 //(increasing E_g order)
    fHaveIncScaler=ETrue;	// flag that we have the Inc. scaler reference
    if(IsFlagged) fHaveIncGatedScaler=ETrue; // flag that we have the Inc. gated scaler reference
    fclose(fp);		// close the scaler dump file
    
    //run though the tagger scalers to find bad channels
    for(int n=0;n<fTaggerChannels-4;n++){ //flag if less that 1/9 of neighbours	
      if((18*fIncSpectrum[n])<(fIncSpectrum[n+2]+fIncSpectrum[n+3]+fIncSpectrum[n+4])){
	fBadScalerChan[n]=kTRUE;
      }
      else{
	fBadScalerChan[n]=kFALSE;
      }
      if(fIncSpectrum[n]<10.0)fBadScalerChan[n]=ETrue; // or less than 10.0

      if((18*fIncGatedSpectrum[n])<(fIncGatedSpectrum[n+2]+fIncGatedSpectrum[n+3]+fIncGatedSpectrum[n+4])){
	fBadGatedScalerChan[n]=kTRUE;
      }
      else{
	fBadGatedScalerChan[n]=kFALSE;
      }
      if(fIncGatedSpectrum[n]<10.0)fBadGatedScalerChan[n]=ETrue; // or less than 10.0
    }
    // work out the normalisation channel based on the norm energy
    
    for(int n=0;n<fTaggerChannels;n++){
      if(fEnergyCalib[n]>=fNormEnergy){
	fNormChannel=n;
	break;
      }
    }
    while(fIncSpectrum[fNormChannel--]<10.0); 
    fprintf(stderr,"Using %lf as the fNormEnergy\n",fEnergyCalib[fNormChannel]);
  }
  
  else {
    fprintf(stderr,"Warning: Couldn't open %s \n",refFileName);
    return -1;
  }
      
  return 0;
}

Double_t TA2LinearPolEpics::GetPolDegree(Double_t energy){
  Double_t pol;
  if(fIsNewFile) return -1;	//no meaningful pol info until 1st scaler read,
  if((fPlane!=ETablePara)&&(fPlane!=ETablePerp)) return -1; //no meanungful plane 
  // if((fEdge<(fLastEdge-fDeadband))||(fEdge>(fLastEdge+fDeadband))){
  //    fLastEdge=fEdge;
  //    enhFromParams();
  //}
  //return -1.0 if too far from edge.
  if((energy<fEdge-fBeforeEdge)||(energy>fEdge+fAfterEdge)) return -1;  
  pol = fHistP->GetBinContent(fHistP->FindBin(energy));
  //return -1 if below threshold
  if(pol<fPolMin) return -1.0;
  return pol;
}


Double_t *TA2LinearPolEpics::FillPolArray(){
  Int_t n=0;
  Int_t bin=-1;
  Int_t counter=-1;
  Double_t oldmean=-1.0;
  Double_t pol=-1.0;
  
  if(fIsNewFile) return NULL;	//no meaningful pol info until 1st scaler read,

  //if no valid plane or edge
  if(((fPlane!=ETablePara)&&(fPlane!=ETablePerp))|| ((fEdge<(fEdgeSetting-fEdgeRange))||(fEdge>(fEdgeSetting+fEdgeRange)))){
    while(fLadderHits[n]!=(Int_t)ENullHit){
      fPolArray[n]=-1.0;
      fPolArrayM[n++]=-1.0;
    }
    fPolArray[n]=(Double_t)ENullHit;
    return NULL;
  }
  
  if((fEdge<(fLastEdge-fDeadband))||(fEdge>(fLastEdge+fDeadband))){
    fLastEdge=fEdge;
    fBeforeEdgeBin=fHistP->FindBin(fEdge-fBeforeEdge); //find channels
    fAfterEdgeBin=fHistP->FindBin(fEdge+fAfterEdge);
    enhFromParams();
  }
  while(fLadderHits[n]!=(Int_t)ENullHit){
    //ladder bins go from 0->351 in reverse ord from hist bins, going from 1-352 
    bin=fHistP->GetNbinsX()-fLadderHits[n];
    if((bin<fBeforeEdgeBin)||(bin>fAfterEdgeBin)){
      fPolArray[n]=-1.0;
    } 
    else{
      pol=fHistP->GetBinContent(bin);
      if(pol<fPolMin){
	fPolArray[n]=-1.0;
      }
      else{
	fPolArray[n]=fHistP->GetBinContent(bin); //fill
	if((fHPolCount)&&(fHPolMean)){           //if the hists are there, fill the meam and count
	  counter = fHPolCount->GetBinContent(bin);
	  oldmean = fHPolMean->GetBinContent(bin);
	  fHPolMean->SetBinContent(bin, oldmean + ((fHistP->GetBinContent(bin) - oldmean)/(counter+1)));
	  fHPolCount->SetBinContent(bin,counter+1);
	}
      }
    }
    n++;
  }
  fPolArray[n]=(Double_t)ENullHit;
  
  return fPolArray; 
}

void  TA2LinearPolEpics::enhFromParams(){
  //make an enhancement and corresponding polarization from some the parameters as defined in the CLAS note.
  //this function is can be called stand alone, but will also ba called many times from the fitting function
  Double_t *par=NULL;
  Double_t xd[10];
  Double_t xc[10];
  Double_t Q[10];
  Double_t cohContrib;
  Double_t cohTotal;
  Double_t phiTotal;
  Double_t etotal;
  Double_t ptotal;
  Double_t x=0.0;
  Int_t    g=0;
  Double_t weight=0.0;
  Double_t weightSum=0.0;
  Double_t polSum=0.0;
  Double_t phi,chi,cd;
  Double_t amo;
  Int_t jbin=0;
  Double_t E0=fBeamEnergy;
  Double_t Eg=fEdge;

  if(par==NULL){
    par=fPolTableParams[fPolRangeIndex[fPlane]][fPlane];

    //get theta for the current edge position
    par[THETA]  = k/(2.0*E0*E0*((1/Eg)-(1/E0)));        //theta from edge and beam energy
  }

  //reset them all for fresh filling
  fHistE->Reset("ICE");
  fHistP->Reset("ICE");
  fThetaPol->Reset("ICE");
  fThetaItot->Reset("ICE");
  fWeightHist->Reset("ICE");


  for(Double_t j=par[THETA]-3.0*par[SIGMA];j<=par[THETA]+3.001*par[SIGMA];j+=(6.0*par[SIGMA])/THETASTEPS){
    
    weight=TMath::Gaus(j,par[THETA],par[SIGMA]);   //get the weight from the gaussian
    weightSum+=weight;                             //add to sum      
    
    //find the discontinuity for each vector
    for(int v=0;v<par[NVEC];v++){
      g=VECTORS[v];
      xd[v]=1.0/((k/(g*par[E0MEV]*j))+1.0);
      Q[v]=(1.0-xd[v])/xd[v];
      xc[v]=xd[v]/(1+((par[THETAR]*par[THETAR])*(1-xd[v])));
    }

    //loop over all bins in the histogram
    for(int bin=1;bin<=fHistE->GetNbinsX();bin++){
      x=fHistE->GetBinCenter(bin)/par[E0MEV];            //find the value of the bin
      amo=1/x;                                    //assume amo = inc = 1/x over regio of interest
      
      cohTotal=0.0;
      phiTotal=0.0;
      
      //loop over all the vectors
      for(int v=0;v<par[NVEC];v++){
	if(x>xd[v]) continue;           //only do up to x_dg
	 
	//work out chi and phi
	phi=(2*Q[v]*Q[v]*x*x)/((1-x)*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1))));
	chi=((Q[v]*Q[v]*x)/(1-x))*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1)));
	//	cout << j  << "  " << chi << endl;
	cd=0.5*(1+TMath::Erf((x-xc[v])/(TMath::Sqrt(2)*par[SIGMAR])));

	//get coherent contrib for the vector
	cohContrib=cd*par[IVEC+v]*chi;

	//add to the total and update the phi total
	cohTotal+=cohContrib;
	phiTotal+=cohContrib*phi;

      }
      if(cohTotal>0.0) {
	phiTotal/=cohTotal;   //divide by the cohTotal to get the weighted dmean phi
	//cout << x << " " << phiTotal << " " << cohTotal << " " << weight << endl;	 
      }

      //enhancement = coherent total + inc (or amo).
      etotal=(amo+cohTotal)/amo;
      //and pol like this
      //      ptotal=phiTotal*cohTotal/(cohTotal + amo);
      ptotal=phiTotal*cohTotal;

      //add the weighted contribution to the enhancement
      fHistE->Fill(x*par[E0MEV],weight*etotal);

      //keep the pol for this x,theta coord
      fThetaPol->Fill(x*par[E0MEV],jbin,ptotal);

      //keep the total intensity for this x,theta coord
      fThetaItot->Fill(x*par[E0MEV],jbin,cohTotal+amo);
    }
    
    //save the weight for this theta point
    fWeightHist->Fill(jbin,weight);
    jbin++;

  }
  //normalize the sum of the weighted enhancements
  fHistE->Scale(1.0/weightSum);
  
  
  //loop over each x bin, adding the weighted contribs from each theta pos
  for(int bin=1; bin<=fHistP->GetNbinsX(); bin++){
    weightSum=0.0;
    polSum=0.0;
    
    for(int jb=1;jb<=fWeightHist->GetNbinsX();jb++){
      weight=fWeightHist->GetBinContent(jb);

      //      polSum+=fThetaPol->GetBinContent(bin,jb)*fThetaItot->GetBinContent(bin,jb)*weight;
      polSum+=fThetaPol->GetBinContent(bin,jb)*weight;
      weightSum+=fThetaItot->GetBinContent(bin,jb)*weight;
      //polSum+=fThetaPol->GetBinContent(bin,jb)*weight;
      //weightSum+=weight;
    }
    polSum/=weightSum;
    fHistP->Fill(fHistP->GetBinCenter(bin),polSum);
  } 
  //
  for(int n=0;n<=fTaggerChannels;n++){
    if(fHPolTableEnh) fHPolTableEnh->SetBinContent(n+1,fHistE->GetBinContent(n+1));
    if(fHPolTablePol)fHPolTablePol->SetBinContent(n+1,fHistP->GetBinContent(n+1));
    fCurrentPolTable[n]	=fHistP->GetBinContent(n+1);
    fCurrentEnhTable[n]	=fHistE->GetBinContent(n+1);      
    
    fCurrentPolTable_TC[fTaggerChannels-n]=fHistP->GetBinContent(n+1);
    fCurrentEnhTable_TC[fTaggerChannels-n]=fHistE->GetBinContent(n+1);
  }
}

void  TA2LinearPolEpics::FitInit(const TH1F *histD){
  fFitEnhData = (TH1F*) histD->Clone("FitEnhData");
  fFitEnh = (TH1F*) fFitEnhData->Clone("FitEnh");
  fFitPol = (TH1F*) fFitEnhData->Clone("FitPol");
  fFitInitialised = kTRUE;
}

void  TA2LinearPolEpics::FitEnhancement(const TH1F *histD, const double scalingN, const int nVec){

  Double_t beamMeV = fBeamEnergy;
  Double_t colliDist_m = 2.5;
  Double_t colliRad_mm = 3.0;

  const Int_t VECTORS[]={2,4,6,8,10};    //list of the vectors to be included (022,044);

  if(!fFitInitialised) FitInit(histD);
  
  if(fFitEnhData!=NULL)fFitEnhData->Reset("ICES");
  if(fFitEnh!=NULL)fFitEnh->Reset("ICES");
  if(fFitPol!=NULL)fFitPol->Reset("ICES");
  
  for(int i=0;i<=histD->GetNbinsX();i++){
    fFitEnhData->SetBinContent(i,histD->GetBinContent(i));
  }

  
  UInt_t nBins = fFitEnhData->GetNbinsX();
  Double_t diff1, diff2, lowmean,scalefac;
  Double_t par[10];
  ROOT::Math::Minimizer* min;
  Char_t name[30];
  
  TF1 *gausFit=new TF1("gausFit",GausOnBase,0,100,4);

  
  //Get rid of zeros
  for(UInt_t n=1;n<=nBins-1;n++){
    if(fFitEnhData->GetBinContent(n)<0.1)fFitEnhData->SetBinContent(n,fFitEnhData->GetBinContent(n+1));
  }
  //Get rid of zeros 2nd pass
  for(UInt_t n=1;n<=nBins-1;n++){
    if(fFitEnhData->GetBinContent(n)<0.1)fFitEnhData->SetBinContent(n,fFitEnhData->GetBinContent(n+1));
  }
  //  Get rid of spikes up and down
  for(UInt_t n=2;n<=nBins-1;n++){
    diff1=(fFitEnhData->GetBinContent(n)-fFitEnhData->GetBinContent(n-1))/fFitEnhData->GetBinContent(n-1);
    diff2=(fFitEnhData->GetBinContent(n)-fFitEnhData->GetBinContent(n+1))/fFitEnhData->GetBinContent(n+1);

    if (((fabs(diff1)>0.03)&&(fabs(diff2)>0.03))&&(fabs(diff1-diff2)<0.1)){
      fFitEnhData->SetBinContent(n,0.5*(fFitEnhData->GetBinContent(n-1)+fFitEnhData->GetBinContent(n+1)));
    }
  }

  // find a reasonable minumum spot to set to 1 for the baseline.
  // the lowest 5 channel mean between 0.2 and 0.95 of the range
  lowmean=1000000.0;
  for(int n=(int)(0.15*(float)nBins);n<=(int)(0.95*(float)nBins);n++){
    if((fFitEnhData->Integral(n-2,n+2)<lowmean)){
      lowmean=fFitEnhData->Integral(n-2,n+2);
      std::cout << "lowmean: " << lowmean << " chan: " << n << std::endl;
    }
  }
  if(lowmean<1) lowmean=500.0;
  fFitEnhData->Scale(5.0/(lowmean));

  //energy dependent scaling to improve fit
  for(UInt_t n=1;n<=nBins-1;n++){
    Double_t binenergy = fFitEnhData->GetBinCenter(n);
    Double_t bintemp = fFitEnhData->GetBinContent(n);
    if(binenergy==0) continue;
    bintemp = bintemp*TMath::Power((1/binenergy),scalingN);
    fFitEnhData->SetBinContent(n,bintemp);
  }
  
  fFitEnhData->GetXaxis()->SetRange(100,300);
  fFitEnhData->SetMaximum(1.2*fFitEnhData->GetMaximum());
  fFitEnhData->SetMinimum(0.0);
  fFitEnh->SetMaximum(1.2*fFitEnhData->GetMaximum());
  fFitEnh->SetMinimum(0.0);
  fFitPol->SetMaximum(1);
  fFitEnhData->GetXaxis()->SetRange();
  //fFitEnhData->Draw("P");

  //Fit
  //Now try to make some guesses at the initial parameters
  fFitEnhData->GetXaxis()->SetRange(100,300);
  gausFit->SetRange(fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin()),fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin())+100.0);
  gausFit->SetParameter(1,fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin()));
  gausFit->SetParameter(2,10.0);
  gausFit->SetParameter(3,1.0);
  fFitEnhData->Fit(gausFit,"rN");
  fFitEnhData->GetXaxis()->SetRange();

  //Get the edge from the fit
  fFitedge=(gausFit->GetParameter(1)) + abs(gausFit->GetParameter(2));


  std::cout << beamMeV << std::endl;
  std::cout << fFitedge << std::endl;
  std::cout << gausFit->GetParameter(2) << std::endl;

  //Now we have enough information to set the basic parameters
  parFromHuman(fFitedge,gausFit->GetParameter(2),colliDist_m,colliRad_mm,nVec,par);

  //set the intensities
  for(int v=0;v<par[NVEC];v++){                                               //give the vectors intensities
    par[IVEC+v] = fFitEnhData->GetMaximum()*2.0/((Double_t)VECTORS[v]*(Double_t)VECTORS[v]);      //tailing off as 1/VECTORS[v]^2
  }
    
  enhFromParams(par,fFitedge);

  //Redo the intensities according to a the calc / data ration
  scalefac=fFitEnhData->GetMaximum()/fFitEnh->GetMaximum();
  for(int v=0;v<par[NVEC];v++){                                               //give the vectors intensities
    par[IVEC+v]*=scalefac;
  }
  enhFromParams(par,fFitedge);
  fFitEnh->SetLineColor(2);
  //fFitEnh->Draw("same");    

  fFitPol->SetLineColor(2);
  //fFitPol->Draw();
  gSystem->ProcessEvents();
  

  min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Simple");


  // set tolerance , etc...
  fFitEnhData->GetXaxis()->SetRange(100,300);
  fFitMinBin=fFitEnh->FindBin((int)fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin())-fLOWFIT);
  fFitMaxBin=40+fFitEnh->FindBin(par[E0MEV]/((((2.0/4.0)*((par[E0MEV]/fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin()))-1.0))+1.0)));
  fFitEnhData->GetXaxis()->SetRange();
  
  std::cout << "fitMinBin " << fFitMinBin << std::endl;
  std::cout << "BinCenter " << fFitEnhData->GetBinCenter(fFitEnhData->GetMaximumBin()) << std::endl;
  std::cout << "MaxBin " << fFitEnhData->GetMaximumBin() << std::endl;
  std::cout << "fLowfit " << fLOWFIT << std::endl;
  std::cout << "fitMaxBin " << fFitMaxBin << std::endl;
  
  min->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2 
  min->SetMaxIterations(10000);  // for GSL 
  min->SetTolerance(0.001);
  min->SetPrintLevel(1);
    
  ROOT::Math::Functor ft(this, &TA2LinearPolEpics::efit, IVEC+nVec);
  min->SetFunction(ft);
 
  //Now set the variables
  min->SetLimitedVariable(THETA,   "Theta",   par[THETA],      par[THETA]/100.0,  0.95*par[THETA], 1.05*par[THETA]);
  min->SetLimitedVariable(SIGMA,   "Sigma",   2.5*par[SIGMA],  par[SIGMA]/100.0,  par[SIGMA],  5.0*par[SIGMA]);
  min->SetLimitedVariable(THETAR,  "Thetar",  2.0*par[THETAR],     par[THETAR]/100.0, 0.2*par[THETAR], 5.0*par[THETAR]);
  min->SetLimitedVariable(SIGMAR,  "Sigmar",  0.5*par[SIGMAR], par[SIGMAR]/100.0, 0.1*par[SIGMAR], 20.0*par[SIGMAR]);
  min->SetFixedVariable(E0MEV,     "E0MeV",   par[E0MEV]);  //no of vectors
  min->SetFixedVariable(NVEC,      "Nvec",    par[NVEC]);  //no of vectors
  for(int v=0;v<nVec;v++){
    sprintf(name,"Vec0%d%d", VECTORS[v],VECTORS[v]);
    min->SetVariable(v+IVEC, name, par[v+IVEC], par[v+IVEC]/100.0);
 
  }

  fbestChisq=100000.00;   //set this high for starters

  min->Minimize(); 
  
  //enhFromParams(fbestPar,fFitedge);



  // fFitEnhData->Draw("P");
  fFitEnh->SetLineColor(2);
  //  fFitEnh->Draw("same");    

  fFitPol->SetLineColor(2);
  //  fFitPol->Draw();
  gSystem->Sleep(500);


  std::cout << "Fit procedure done!" << std::endl;
}

void  TA2LinearPolEpics::parFromHuman(Double_t edgeMeV, Double_t spreadMeV, 
		  Double_t colliDist_m, Double_t colliRad_mm, Int_t nVec, Double_t *par){

   //takes some physical quantities and makes them into parameters, then calls the 
  //enhFromParams function.
  
  Int_t g = 2;                                                                //variables used in CLAS note
  Double_t E0 = fBeamEnergy;
  Double_t Eg = edgeMeV;
  
  
  par[THETA]  = k/(g*E0*E0*((1/Eg)-(1/E0)));                                  //theta from edge and beam energy
  par[SIGMA]  = (par[THETA]-(k/(g*E0*E0*((1/(Eg-spreadMeV))-(1/E0)))))/3.0;   //spread in theta from spread in edge 
  par[THETAR] = E0*0.001*5.0*colliRad_mm/colliDist_m;                         //cut from collimator
  par[SIGMAR] = par[THETAR]*par[SIGMA]/par[THETA];                            //smear in above same fractional sigma as above
  par[E0MEV]  = E0;                                                           //beam energy
  par[NVEC]   = (Double_t)nVec;                                                         //no of harmonics

  for(int v=0;v<par[NVEC];v++){                                               //give the vectors intensities
    par[IVEC+v] = 2.0/(Double_t)VECTORS[v];                                   //tailing off as 1/VECTORS[v]
    std::cout << IVEC+v << "  v   " << par[IVEC+v] << std::endl; 
  }
  std::cout << "Theta: " << par[THETA] << std::endl;
  std::cout << "Sigma: " << par[SIGMA] << std::endl;
  std::cout << "Thetar: " << par[THETAR] << std::endl;
  std::cout << "Sigmar: " << par[SIGMAR] << std::endl;
  std::cout << "E0: " << par[E0MEV] << std::endl;
  std::cout << "Nvec: " << par[NVEC] << std::endl;
}

Double_t  TA2LinearPolEpics::efit(const Double_t *parms){

  bool verbose = false;
  Double_t chisq = 1.0;
  Double_t delta;
  Double_t b1,b2;
  Double_t err;
  Double_t *par = (Double_t*)parms;
  Int_t counter=1;
  

  //call the function to make the enhancement and polarization
  enhFromParams(par,fFitedge);

  chisq = 1.0;
  //loop over all the required bins in the histogram to work out a chisq
  for(int n=fFitMinBin;n<=fFitMaxBin;n++){
    b1=fFitEnh->GetBinContent(n);
    b2=fFitEnhData->GetBinContent(n);
    err=1.0;
    delta=(b1-b2)/err;
    chisq+=(delta*delta);
    //note - not a proper chisq because its an enhancement
  }
   
  fprintf(stderr,"Chisq: \t%6.2f\t\r",chisq);


  if(chisq<fbestChisq){
    fbestChisq=chisq;
    for(int n=0;n<10;n++){
       fbestPar[n]=par[n];
    }
    if(verbose){
      if(10%(counter++)){
	//if verbose, draw this on the canvas for every iteration to see how it's going

	//	fFitEnhData->Draw("P");

	//	fFitEnh->Draw("same");
	
	std::cout << "HI" << std::endl;

	//	fFitPol->Draw();
	fFitPol->SetMinimum(0);
	fFitPol->SetMaximum(1);

	gSystem->ProcessEvents();
	counter=1;
      }
    }
  }
  return chisq; 
}

void  TA2LinearPolEpics::enhFromParams(Double_t *par, Double_t edge){
  //make an enhancement and corresponding polarization from some the parameters as defined in the CLAS note.
  //this function is can be called stand alone, but will also ba called many times from the fitting function

  Double_t xd[10];
  Double_t xc[10];
  Double_t Q[10];
  Double_t cohContrib;
  Double_t cohTotal;
  Double_t phiTotal;
  Double_t etotal;
  Double_t ptotal;
  Double_t x=0.0;
  Int_t    g=0;
  Double_t weight=0.0;
  Double_t weightSum=0.0;
  Double_t polSum=0.0;
  Double_t phi,chi,cd;
  Double_t amo;
  Int_t jbin=0;
  Double_t E0=fBeamEnergy;
  Double_t Eg=edge;

  if(par==NULL){
    par=fPolTableParams[fPolRangeIndex[fPlane]][fPlane];

    //get theta for the current edge position
    par[THETA]  = k/(2.0*E0*E0*((1/Eg)-(1/E0)));        //theta from edge and beam energy
  }

  //reset them all for fresh filling
  fFitEnh->Reset("ICE");
  fFitPol->Reset("ICE");
  fThetaPol->Reset("ICE");
  fThetaItot->Reset("ICE");
  fWeightHist->Reset("ICE");


  for(Double_t j=par[THETA]-3.0*par[SIGMA];j<=par[THETA]+3.001*par[SIGMA];j+=(6.0*par[SIGMA])/THETASTEPS){
    
    weight=TMath::Gaus(j,par[THETA],par[SIGMA]);   //get the weight from the gaussian
    weightSum+=weight;                             //add to sum      
    
    //find the discontinuity for each vector
    for(int v=0;v<par[NVEC];v++){
      g=VECTORS[v];
      xd[v]=1.0/((k/(g*par[E0MEV]*j))+1.0);
      Q[v]=(1.0-xd[v])/xd[v];
      xc[v]=xd[v]/(1+((par[THETAR]*par[THETAR])*(1-xd[v])));
    }

    //loop over all bins in the histogram
    for(int bin=1;bin<=fFitEnh->GetNbinsX();bin++){
      x=fFitEnh->GetBinCenter(bin)/par[E0MEV];            //find the value of the bin
      amo=1/x;                                    //assume amo = inc = 1/x over regio of interest
      
      cohTotal=0.0;
      phiTotal=0.0;
      
      //loop over all the vectors
      for(int v=0;v<par[NVEC];v++){
	if(x>xd[v]) continue;           //only do up to x_dg
	 
	//work out chi and phi
	phi=(2*Q[v]*Q[v]*x*x)/((1-x)*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1))));
	chi=((Q[v]*Q[v]*x)/(1-x))*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1)));
	//	cout << j  << "  " << chi << endl;
	cd=0.5*(1+TMath::Erf((x-xc[v])/(TMath::Sqrt(2)*par[SIGMAR])));

	//get coherent contrib for the vector
	cohContrib=cd*par[IVEC+v]*chi;

	//add to the total and update the phi total
	cohTotal+=cohContrib;
	phiTotal+=cohContrib*phi;

      }
      if(cohTotal>0.0) {
	phiTotal/=cohTotal;   //divide by the cohTotal to get the weighted dmean phi
	//cout << x << " " << phiTotal << " " << cohTotal << " " << weight << endl;	 
      }

      //enhancement = coherent total + inc (or amo).
      etotal=(amo+cohTotal)/amo;
      //and pol like this
      //      ptotal=phiTotal*cohTotal/(cohTotal + amo);
      ptotal=phiTotal*cohTotal;

      //add the weighted contribution to the enhancement
      fFitEnh->Fill(x*par[E0MEV],weight*etotal);

      //keep the pol for this x,theta coord
      fThetaPol->Fill(x*par[E0MEV],jbin,ptotal);

      //keep the total intensity for this x,theta coord
      fThetaItot->Fill(x*par[E0MEV],jbin,cohTotal+amo);
    }
    
    //save the weight for this theta point
    fWeightHist->Fill(jbin,weight);
    jbin++;

  }
  //normalize the sum of the weighted enhancements
  fFitEnh->Scale(1.0/weightSum);
  
  
  //loop over each x bin, adding the weighted contribs from each theta pos
  for(int bin=1; bin<=fFitPol->GetNbinsX(); bin++){
    weightSum=0.0;
    polSum=0.0;
    
    for(int jb=1;jb<=fWeightHist->GetNbinsX();jb++){
      weight=fWeightHist->GetBinContent(jb);

      //      polSum+=fThetaPol->GetBinContent(bin,jb)*fThetaItot->GetBinContent(bin,jb)*weight;
      polSum+=fThetaPol->GetBinContent(bin,jb)*weight;
      weightSum+=fThetaItot->GetBinContent(bin,jb)*weight;
      //polSum+=fThetaPol->GetBinContent(bin,jb)*weight;
      //weightSum+=weight;
    }
    polSum/=weightSum;
    fFitPol->Fill(fFitPol->GetBinCenter(bin),polSum);
  } 
}

ClassImp(TA2LinearPolEpics)
