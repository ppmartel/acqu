//--Author	JRM Annand    4th Apr 2003
//--Rev 	JRM Annand... 8th Jan 2004..TA2Control mod.
//--Update	JRM Annand...18th Feb 2005..Run to TA2Control
//--Update  A Neiser...   6th June 2013  Make char_t* const
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Main program to run HV systems stand alone
//
#include "TDAQ_SY1527.h"

TDAQ_SY1527* HV_1527;         // HV class
void CommandLoop();

//______________________________________________________________________________
int main(int argc, char **argv)
{
  // Run AcquHV

  const Char_t* file;
  FILE* logfile = fopen("AcquHV.log","w");
  if( argc > 1 ) file = argv[1];
  else file = "AcquHV.dat";
  HV_1527 = new TDAQ_SY1527( "Tagger", file, logfile );
  HV_1527->FileConfig();
  HV_1527->PostInit();
  HV_1527->CommandLoop();
  sleep(1);
  delete HV_1527 ;
}

void CommandLoop()
{
  // Input from terminal
  //  while( comm ){
  //    switch 
  //      }
  return;
}

