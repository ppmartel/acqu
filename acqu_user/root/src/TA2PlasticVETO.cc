//--Author	JRM Annand   30th Sep 2003
//--Rev 	JRM Annand...15th Oct 2003 ReadDecoded...MC data
//--Rev 	JRM Annand... 5th Feb 2004 3v8 compatible
//--Rev 	JRM Annand...19th Feb 2004 User code
//--Update	JRM Annand...16th May 2005 ReadDecoded (bug G3 output)
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2PlasticVETO
//
// Internal Particle Identification Detector for the Crystal Ball
// Cylindrical array of plastic scintillators
//

#include "TA2PlasticVETO.h"


//---------------------------------------------------------------------------
TA2PlasticVETO::TA2PlasticVETO( const char* name, TA2System* apparatus )
  :TA2Detector(name, apparatus)
{ //printf("print\n");

  // Default detector initialisation
}

//---------------------------------------------------------------------------
TA2PlasticVETO::~TA2PlasticVETO()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  DeleteArrays();
}

//-----------------------------------------------------------------------------
void TA2PlasticVETO::LoadVariable( )
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

  //                            name     pointer          type-spec
  //  TA2DataManager::LoadVariable("Energy", &fTotalEnergy,   EDSingleX);

  // Call the standard detector
  // name-variable load
  TA2Detector::LoadVariable();
  
}

//---------------------------------------------------------------------------
void TA2PlasticVETO::SaveDecoded( )
{
  // Save decoded info to Root Tree file
}
ClassImp(TA2PlasticVETO)
