//--Author	JRM Annand    4th Apr 2003
//--Update	JRM Annand... 8th Jan 2004..TA2Control mod.
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Main program to run the system
//
#include "TA2UserControl.h"

//______________________________________________________________________________
int main(int argc, char **argv)
{
  // Run the Acqu-Root analyser
  // TA2Control constructor sets up the AcquRoot class
  // StartAnalyser() sets up the TA2Control  class
  // and starts the analyser running in a separate thread
  // Run() starts a concurrent ROOT shell to display/manipulate the analysis

  TA2Control* a2ctrl = new TA2UserControl( "Acqu-Root Analyser", &argc, argv );
  a2ctrl->StartAnalyser();
  a2ctrl->Run();
  delete a2ctrl ;
}

