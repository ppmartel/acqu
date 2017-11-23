//--Author	JRM Annand    1st Jun 2005
//--Rev 	JRM Annand
//--Update	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Main program to run the MC reaction event generator
//

#include "TMCFoamGenerator.h"

TMCFoamGenerator* MC;

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  // Run the Acqu-Root Monte Carlo reaction event generator

  char setfile[128];
  strcpy( setfile, "MCsetup.dat" );
  // Handle any command-line option here online/offline or setup file
  for( int i=1; i<argc; i++ ){
    if( strcmp("--", argv[i]) != 0 ) strcpy( setfile, argv[i] );
  }
  MC = new TMCFoamGenerator( (Char_t*)"MC" );
  MC->FileConfig((Char_t*)"setfile");
  MC->Generate();
}

