//--Author	JRM Annand	21st Dec 2005
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	B. Oussena       May 2009..fix a bug in PostInit()
//--Update	JRM Annand  17th May 2010..It should be A24 addressing
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CBD8210
// CES CBD 8210 CAMAC Parallel Branch Driver
//

#include "TVME_CBD8210.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


enum { ECBD_XXX=200, ECBD_YYY };
static Map_t kCBDKeys[] = {
  {"XXX:",                ECBD_XXX},
  {"YYY:",                ECBD_YYY},
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TVME_CBD8210::TVME_CBD8210( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = new TDAQcontrol( this );         // tack on control functions
  fType = EDAQ_SCtrl;                      // secondary controller
  AddCmdList( kCBDKeys );                  // CBD-specific setup commands
}

//-----------------------------------------------------------------------------
TVME_CBD8210::~TVME_CBD8210( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_CBD8210::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file  
  switch( key ){
  case ECBD_XXX:
    break;
  case ECBD_YYY:
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_CBD8210::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  if( !fReg ){
    fMaxReg = 2;
    fReg = new void*[fMaxReg];
    fDW = new Int_t[fMaxReg];
    fData = new UInt_t[fMaxReg];
    fAM = new Int_t[fMaxReg];
    fReg[0] = ( (Char_t*)fBaseAddr + 0xe802 );
    fReg[1] = ( (Char_t*)fBaseAddr + 0xe812 );
    fDW[0] = fDW[1] = 2;
    //    fAM[0] = fAM[1] = 0x9;
    fAM[0] = fAM[1] = EVME_A24;
  }
  //fCSR = (UShort_t*)fReg[0]; // Baya: moved after TVMEmodule::PostInit();
  //fIFR = (UShort_t*)fReg[1];
  TVMEmodule::PostInit();
  fCSR = (UShort_t*)fReg[0];
  fIFR = (UShort_t*)fReg[1];
  return;
}

ClassImp(TVME_CBD8210)
