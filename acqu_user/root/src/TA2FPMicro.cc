//--Author	JRM Annand   17th Mar 2006
//--Rev 	
//--Update	
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2FPMicro
//
// Tests of Gla/Mz tagger with high-resolution FP detectors
//

#include "TA2FPMicro.h"


//---------------------------------------------------------------------------
TA2FPMicro::TA2FPMicro( const char* name, TA2System* apparatus )
  :TA2Detector(name, apparatus)
{
  // Set variables uninitialised
  fFPhits = NULL;
  fNFPhits = 0;
  fNFPelem = 0;
}

//---------------------------------------------------------------------------
TA2FPMicro::~TA2FPMicro()
{
  // Free up all allocated memory
}

//-----------------------------------------------------------------------------
void TA2FPMicro::ParseDisplay( char* line )
{
  // Input private histogram spec to general purpose parse
  // and setup routine

  //  const Name2Variable_t hist[] = {
  // Name          ->variable  single/mult    Fill-condition
  //    {"Nphoton_Minv", fM_Nphoton, EHistSingleX},
  //    {NULL,          0,         0}
  //  };
  // Do not remove the final NULL line

  TA2Detector::ParseDisplay(line);
  return;
}

//---------------------------------------------------------------------------
void TA2FPMicro::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Variables for overlap-hit arrays

  fNFPelem = 2*(fNelem - 1) + 1;
  fFPhits = new Int_t[fNFPelem];
  TA2Detector::PostInit();
}

//---------------------------------------------------------------------------
void TA2FPMicro::SaveDecoded( )
{
  // Save decoded info to Root Tree file
}
//-----------------------------------------------------------------------------
void TA2FPMicro::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  TA2Detector::LoadVariable();
  TA2DataManager::LoadVariable("NFPhits",     &fNFPhits,     EISingleX);
  TA2DataManager::LoadVariable("FPhits",      fFPhits,       EIMultiX);
}

ClassImp(TA2FPMicro)
