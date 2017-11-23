//--Author	JRM Annand	11th Jan 2008
//--Rev 	JRM Annand
//--Update	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V775
// CAEN VMEbus 32-channel TDC (analogue TAC front end)
//

#include "TVME_V775.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


//-----------------------------------------------------------------------------
TVME_V775::TVME_V775( Char_t* name, Char_t* file, FILE* log,
		      Char_t* line ):
TVME_V792( name, file, log, line )
{
  // Add any specialist 775 stuff here
  fHardID = 775;
}

//-----------------------------------------------------------------------------
TVME_V775::~TVME_V775( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_V775::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file. Add any specialist stuff here 
  switch( key ){
  default:
    // default use commands of V792
    TVME_V792::SetConfig(line, key);
    break;
  }
}

ClassImp(TVME_V775)
