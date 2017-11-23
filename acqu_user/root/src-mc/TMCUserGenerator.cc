//--Author	JRM Annand   19th Oct 2005  Skeleton user version	
//--Rev         
//--Update      JRM Annand
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCUserGenerator
//
// Skeleton User Monte Carlo Kinematics Generator
// Users can redefine behaviour of TMCGenerator class here

#include "TMCUserGenerator.h"


// Recognised User configure commands
enum { EMCUserMisc = 200 };
// Main command options
static const Map_t kMCUserCmd[] = {
  {"Misc:",          EMCUserMisc},
  {NULL,             -1}
};

//-----------------------------------------------------------------------------
TMCUserGenerator::TMCUserGenerator( const Char_t* name, Int_t seed )
  :TMCFoamGenerator( name, seed  )
{
  // Add any local configure commands to list
  AddCmdList( kMCUserCmd );
}

//---------------------------------------------------------------------------
TMCUserGenerator::~TMCUserGenerator()
{
  // Free up allocated memory
  TMCFoamGenerator::Flush();
}

//-----------------------------------------------------------------------------
void TMCUserGenerator::SetConfig(Char_t* line, Int_t key)
{
  // Parse a line of configuration information.
  // Recognised command strings are listed in Map_t kMCcmd[] array above

  switch (key){
  case EMCUserMisc:
    // Any local commands here
    PrintError(line,"<Command not yet implemented>");
    break;
  default:
    // Default TMCGenerator config
    TMCFoamGenerator::SetConfig(line,key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TMCUserGenerator::PostInit( )
{
  // Finish initialisation
  TMCFoamGenerator::PostInit();
}

ClassImp(TMCUserGenerator)
