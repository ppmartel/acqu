//--Author	JRM Annand	11th Jan 2008
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Update	JRM Annand.. 3rd Nov 2016..cloan from TVME_V775
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V785
// CAEN VMEbus 32-channel VDC (analogue TAC front end)
//

#include "TVME_V785.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


//-----------------------------------------------------------------------------
TVME_V785::TVME_V785( Char_t* name, Char_t* file, FILE* log,
		      Char_t* line ):
TVME_V792( name, file, log, line )
{
  // Add any specialist 785 stuff here
  fHardID = 785;
}

//-----------------------------------------------------------------------------
TVME_V785::~TVME_V785( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_V785::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file. Add any specialist stuff here 
  switch( key ){
  default:
    // default use commands of V792
    TVME_V792::SetConfig(line, key);
    break;
  }
}

ClassImp(TVME_V785)
