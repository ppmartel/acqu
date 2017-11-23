//--Author	JRM Annand    4th Apr 2003
//--Rev		JRM Annand...
//--Update	JRM Annand... 1st Dec 2004  Remove MC analysis class
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data.
//
// TA2UserControl
//
// Provide a ROOT command-line interface for Acqu-Root analyser.
// Create TAcquRoot Acqu-Root interface and TA2Analysis data analyser.
// Attempt to configure and run the system as a separate thread.
// Allow user defined analyses....enter analysis class in Map_t kKnownAnalysis
// and in CreateAnalyser() method

//<<<<<<< HEAD
//#include "TROOT.h"
//#include "TA2UserControl.h"
//#include "TInterpreter.h"
//#include "TAcquRoot.h"
//#include "TA2Analysis.h"
//#include "TA2UserAnalysis.h"

// Recognised apparatus classes. Anything not in this table must be
// instantiated outside of TA2Analysis and then added
//=======
#include "TA2UserControl.h"

//Global pointer to main class
TA2UserAnalysis* gUAN = NULL;

//Recognised apparatus classes.
//Anything not in this table must be instantiated outside of TA2Analysis and then added
//>>>>>>> user/schumann
enum{ EA2Analysis, EA2UserAnalysis };
static const Map_t kKnownAnalysis[] = {
  {"TA2Analysis",      EA2Analysis},
  {"TA2UserAnalysis",  EA2UserAnalysis},
  {NULL,             -1}
};


//--------------------------------------------------------------------------

TA2UserControl::TA2UserControl(const char* appClassName, int* argc, char** argv, void* options, int numOptions, Bool_t noLogo )
               :TA2Control(appClassName, argc, argv, options, numOptions, noLogo)
{
  //This resolves an odd error with the uninitialised fFiles pointer in TApplication...
  GetOptions(0, NULL);
}

//---------------------------------------------------------------------------
void TA2UserControl::CreateAnalyser()
{
  //Create a user-defined analysis class
  //Add line with analysis name and key to kKnownAnalysis Map_t at
  //the top of TA2UserControl.cc and make entry to create the
  //specific analysis class in the in the switch(analysis){} block below

  Int_t analysis = GetAnalyser(kKnownAnalysis);
  switch(analysis){
  default:
    printf(" FATAL ARROR: Unknown analysis class");
    gAN = NULL;
    gUAN = NULL;
    break;
  case EA2Analysis:
    //Create the base analyser
    gAN = new TA2Analysis("Base-Analysis");
    gUAN = NULL;
    break;
  case EA2UserAnalysis:
    //Analysis of neutral meson events
    gAN = new TA2UserAnalysis("Meson-Analysis");
    gUAN = (TA2UserAnalysis*)gAN;
    break;
  }
}

ClassImp(TA2UserControl)
