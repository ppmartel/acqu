//--Author	JRM Annand    4th Apr 2003
//--Rev 	JRM Annand... 8th Jan 2004..TA2Control mod.
//--Update	JRM Annand...18th Feb 2005..Run to TA2Control
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Main program to run the system
//
#include "TDAQexperiment.h"
#include "TApplication.h"

TDAQexperiment* gEXP;         // DAQ container class

//______________________________________________________________________________
int main(int argc, char **argv)
{
  // Run the Acqu-Root analyser
  // TA2Control constructor sets up the AcquRoot class
  // StartAnalyser() sets up the TA2Control  class
  // and starts the analyser running in a separate thread
  // Run() starts a concurrent ROOT shell to display/manipulate the analysis

  Char_t* file;
  if( argc > 1 ) file = argv[1];
  else file = (Char_t*)"AcquDAQ.dat";
  Int_t initLevel = 0;
  if( argc > 2 ) sscanf( argv[2],"%d",&initLevel );
  //  TApplication theApp("app",&argc,argv);
  gEXP = new TDAQexperiment( (Char_t*)"Acqu-DAQ", file, NULL,
			     NULL, initLevel );
  gEXP->FileConfig();
  gEXP->PostInit();
if (initLevel != 2) //<<------------------------------- Baya
{

  gEXP->StartExperiment();
  //  a2ctrl->Run();
  //  theApp.Run();
  while( !gEXP->IsError() ){
    sleep(1);
  }
  delete gEXP ;
}   //<<------------------------------------------Baya
else {
   // in init mode, output some success message for
   // AcquControl checking routines
   printf("\n<Initialization completely done>\n");
}
}

