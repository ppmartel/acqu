//--Author	JRM Annand    1st Jun 2005
//--Update	JRM Annand
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Main program to run the MC reaction event generator
//

#include "TMCUserGenerator.h"
#include "TMCPhotoPSGenerator.h"

//TMCUserGenerator* MC;
//TMCFoamGenerator* MC;
//TMCHe4Phase* MC;

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  TMCGenerator* MC;
  // Run the Acqu-Root Monte Carlo reaction event generator
  Char_t setfile[128];
  strcpy( setfile, "MCsetup.dat" );
  MC = NULL;
  // Handle any command-line option here online/offline or setup file
  for( int i=1; i<argc; i++ ){
    if( strncmp("--", argv[i],2) != 0 ) strcpy( setfile, argv[i] );
     else if( strcmp("--PS", argv[i]) == 0 )
       MC = new TMCPhotoPSGenerator( "MC_PS_Meson" );
  }
  if( !MC ) MC = new TMCUserGenerator( "MC_User" );
  MC->FileConfig(setfile);
  MC->PostInit();
  MC->Generate();
}

