//--Author	JRM Annand    4th Apr 2003
//--Rev 	JRM Annand... 1st Oct 2003  Interface TA2Analysis
//--Rev 	JRM Annand... 8th Jan 2004  More flexible analyser choice
//--Rev 	JRM Annand... 9th Mar 2004  default online
//--Rev 	JRM Annand...18th Oct 2004  banner print, gAR->SetIsOnline
//--Rev 	JRM Annand... 1st Dec 2004  MC analysis no longer needed
//--Rev 	JRM Annand...18th Feb 2005  Mods for batch processing
//--Rev 	JRM Annand... 3rd Mar 2005  Mods for batch processing
//--Rev 	JRM Annand...11th Mar 2005  Mod default TRint call
//--Rev 	JRM Annand...22nd Jan 2007  4v0 changes
//--Rev 	JRM Annand...17th May 2007  Offline...check LinkADC worked
//--Rev 	JRM Annand... 6th Jul 2007  MC check manual ADC initialisation
//--Rev 	JRM Annand...20th Nov 2007  MC=offline, into TDAQexperiment
//--Rev 	JRM Annand...29th Nov 2007  LinkDataServer() to gAR->Start()
//--Update	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Control
//
// Provide a ROOT command-line interface for Acqu-Root analyser.
// Create TAcquRoot Acqu-Root interface and TA2Analysis data analyser.
// Attempt to configure and run the system as a separate thread. 

#include "TROOT.h"
#include "TA2Control.h"
#include "TInterpreter.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2DataServer.h"
#include "TDAQexperiment.h"
#include "CMakeConfig.h"


// Global pointers to main Classes
TAcquRoot* gAR = NULL;
TA2Analysis* gAN = NULL;
TA2DataServer* gDS = NULL;
TDAQexperiment* gDAQ = NULL;
TA2Control* gCTRL = NULL;

TA2Control* TA2Control::fExists = NULL;  //Pointer to this interface

// Recognised apparatus classes. Anything not in this table must be
// instantiated outside of TA2Analysis and then added
enum{ EA2Analysis };
static const Map_t kKnownAnalysis[] = {
  {"TA2Analysis",      EA2Analysis},
  {NULL,             -1}
};



//--------------------------------------------------------------------------
TA2Control::TA2Control( const char* appClassName, int* argc, char** argv, 
			    void* options, int numOptions, Bool_t noLogo ) :
  TRint( appClassName, argc, argv, options, numOptions, noLogo )
{
  // Create the Acqu-Root interface (global pointer gAR),
  // the Acqu-Root analyser (global pointer gAN)
  // and optionally the Acqu-Root Data Server

  if( fExists ) {
    Error("TA2Control", "only one instance of TA2Control allowed");
    MakeZombie();
    return;
  }
  gCTRL = this;
  // Default offline running, using file ROOTsetup.dat for configuring
  // analysis session
  Bool_t online = ETrue;
  Bool_t logfiles = kTRUE;
  fBatch = EFalse;
  char setfile[128] = "";
  char datafile[256] = "";
  char directory[256] = "./";
  char batchdir[256] = "./";

  // Handle any command-line option here online/offline or setup file
  for( int i=1; i<*argc; i++ ){
    if( strcmp("--offline", argv[i]) == 0 ) online = EFalse;
    else if( strcmp("--rootfile", argv[i]) == 0 ) online = EFalse;
    else if( strcmp("--batch", argv[i]) == 0 ) fBatch = ETrue;
    else if( strcmp("--nolog", argv[i]) == 0 ) logfiles = kFALSE;
    else if( strcmp("--datafile", argv[i]) == 0 ) strcpy( datafile, argv[++i] );
    else if( strcmp("--directory", argv[i]) == 0 ) strcpy( directory, argv[++i] );
    else if( strcmp("--batchdir", argv[i]) == 0 ) strcpy( batchdir, argv[++i] );
    else if( strcmp("--", argv[i]) != 0 ) strcpy( setfile, argv[i] );
  }
  
  // use some default top-level config file,
  // depending on MC mode or not...yeah, it's weird here...
  if(!strcmp(setfile, "")) {
    if(online)
      strcpy( setfile, "ROOTsetup.dat" );
    else
      strcpy( setfile, "ROOTsetupMC.dat" );
  }
  
  if( !noLogo ) PrintLogo();
  SetPrompt("Acqu-Root:%d> ");

  gAR = new TAcquRoot("A2Acqu", fBatch, logfiles); // Acqu - Root interface
  if( online ) gAR->SetIsOnline();               // AcquRoot online procedures

  // Set output and batch log files directories before
  // using config file to give command line preference
  if (strcmp(directory, "./")) gAR->SetTreeDir(gAR->BuildName(directory));
  if (fBatch && strcmp(batchdir, "./")){
    gAR->SetBatchDir(gAR->BuildName(batchdir));
    if(logfiles)
      gAR->SetLogFile(gAR->BuildName(batchdir, "AcquRoot.log"));
  }

  // Perform remaining config from file
  gAR->FileConfig( setfile );                    // user-def setup
  if( gAR->GetProcessType() == EMCProcess ) online = EFalse;

  // If batch directory was not set by command line or file
  // give default to avoid crashing when running batch
  if (fBatch && ((gAR->GetBatchDir()) == NULL)){
    gAR->SetBatchDir(gAR->BuildName(batchdir));
    if(logfiles)
      gAR->SetLogFile(gAR->BuildName(batchdir, "AcquRoot.log"));
  }

  // "online" means receiving data from TA2DataServer which has 3 options
  // 1) network input from remote (usually DAQ) node
  // 2) input read from local file
  // 3) input from "local" DAQ system run as a part of AcquRoot
  // The old method of a separate DataServer program is no longer supported
  //
  if( online ){
    // Optional local DAQ
    if( gAR->IsLocalDAQ() ){
      gDAQ = 
	new TDAQexperiment((Char_t*)"Acqu-DAQ",gAR->GetLocalDAQSetup(),
			   (Char_t*)"AcquDAQ", gAR);
      gDAQ->FileConfig();
      gDAQ->PostInit();
      gDAQ->StartExperiment();
    }
    // TA2DataServer data server input must be setup for online running
    if( gAR->GetDataServerSetup() ){
      gDS = new TA2DataServer("DataServer",gAR);
      gDS->FileConfig( gAR->GetDataServerSetup() );  // configure it
      if (strcmp(datafile, "")){
	gDS->GetDataSource(0)->ClearList();
	gDS->GetDataSource(0)->InputList(datafile, 0, 0);
      }
      gDS->StartSources();                   // start front-end I/O threads
      gDS->Start();                          // start DataServer thread
      //      gAR->LinkDataServer( gDS );            // Header, if in data
    }
    else{
      printf(" Cannot run online mode without TA2DataServer setup\n\n");
      exit(-1);
    }
  }
  // offline running means read from ROOT Tree files...
  // Look for ADC info as a branch of the Tree
  else if( gAR->GetProcessType() == EMCProcess ){
    if (strcmp(datafile, "")){
      gAR->SetNTreeFiles(0);
      gAR->SaveTreeFile(datafile);
    }
    if( !gAR->GetADC() ){
      Error("TA2Control","Manual ADC setup required for MC running>");
      exit(-1);
    }
  }
  else if(!gAR->LinkADC()){
    Error("TA2Control", "<Unable to link to Tree to find ADC info>");
    exit(-1);
  }
  fExists = this;
}

//---------------------------------------------------------------------------
TA2Control::~TA2Control()
{
  if( fExists == this ) {
    fExists = NULL;
    if( gAR ) delete gAR;
    if( gAN ) delete gAN;
    if( gDS ) delete gDS;
    if( gDAQ ) delete gDAQ;
  }
}

//---------------------------------------------------------------------------
void TA2Control::PrintLogo()
{
  // Banner showing versions and dates

  Printf("\n\n");
  Printf("  ****** Acqu - Root Analyser ******\n");
  Printf("          Version: %s",ENV_OR_CMAKE("acquversion", CMAKE_ACQU_VERSION));
  Printf("            Dated: %s",ENV_OR_CMAKE("acqudate", CMAKE_ACQU_DATE));
  Printf("   Linked to Root: %s\n\n",gROOT->GetVersion());
  Printf("   See .log files for all AcquRoot progress/error messages\n\n");

  //   gInterpreter->PrintIntro();

}

//---------------------------------------------------------------------------
Int_t TA2Control::GetAnalyser( const Map_t* knownAnalysis )
{
  // Get the name of the analysis class from TAcquRoot
  // check its validity and return corresponding integer key

  Char_t* analysis = gAR->GetAnalysisType();
  const char* f;
  Int_t type;
  for( Int_t i=0; ; i++ ){
    f = knownAnalysis[i].fFnName;
    if( !f ){
      type = -1;
      break;
    }
    if( !strcmp(analysis, f)  ){
      type = knownAnalysis[i].fFnKey;
      break;
    }
  }
  return type;
}

//---------------------------------------------------------------------------
void TA2Control::CreateAnalyser()
{
  // Get key corresponding to specified (in TAcquRoot setup) analysis class
  // and create analysis class

  Int_t analysis = GetAnalyser(kKnownAnalysis);
  switch(analysis){
  default:
    printf(" FATAL ARROR: Unknown analysis class" );
    gAN = NULL;
    break;
  case EA2Analysis:
    // Create the base analyser 
    gAN = new TA2Analysis( "Base-Analysis" );
    break;
  }
}

//---------------------------------------------------------------------------
void TA2Control::StartAnalyser()
{
  // Check if internal threaded Data Server loaded
  // Create analysis class, link with TAcquRoot
  // Configure the analyser and start it running
  // Check if batch mode....
  // if so turn off graphics and wait for sorting to finish
  // if not start the interactive ROOT command-line interface

  CreateAnalyser();
  gAR->SetAnalysis( gAN );                       // save analysis to AcquRoot
  gAR->SetDataServer( gDS );
  //  gAN->FileConfig( gAR->GetAnalysisSetup() );    // config from file
  gAR->Start();                                  // analysis separate thread
  if( fBatch ){
    MakeBatch();
    while( !gAR->IsFinished() ) sleep(1);
    exit(0);
  }
  else{
    Run();
    //    while( !gAR->IsFinished() ) sleep(1);
  }
}

ClassImp(TA2Control)
