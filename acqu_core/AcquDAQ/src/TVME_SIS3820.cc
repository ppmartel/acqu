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
// TVME_SIS3820
// CES CBD 8210 CAMAC Parallel Branch Driver
//

#include "TVME_SIS3820.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"
//-----------------------------------------------------------------------
// Device registers for SIS3820 Latch-Counter
//-----------------------------------------------------------------------
VMEreg_t SIS3820_reg[] = {
  //off        data  l=D32	init-write=1	
  {0x0,        0x0,	'l',	0},	        // Control & status register
  {0x4,	       0x0,	'l',	0},	        // Module & firmware ID
  {0x8,	       0x0,	'l',	0},	        // Interrupt configuration 
  {0xC,	       0x0,	'l',	0},	        // Interrupt control/status
  {0x10,       0x0,	'l',	0},	        // Direct output data register
  {0x14,       0x0,	'l',	0},	        // J/K output data register
  {0x20,       0x0,	'l',	0},	        // Direct input register
  {0x24,       0x0,	'l',	0},	        // Latched input register
  {0x30,       0x0,	'l',	0},             // Counter 1-4, offset 0x30-3c
  {0x40,       0xf,	'l',	1},             // Clear counters
  {0x60,       0x0,	'l',	0},             // Reset module
  {0xffffffff, 0,        0,     0}              // terminator
};



enum { ESIS_XXX=200, ESIS_YYY };
static Map_t kSISKeys[] = {
  {"XXX:",                ESIS_XXX},
  {"YYY:",                ESIS_YYY},
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TVME_SIS3820::TVME_SIS3820( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = new TDAQcontrol( this );         // tack on control functions
  fType = EDAQ_SlowCtrl;                   // secondary controller
  fHardID = 0x3820;                        // hardware identity
  fNreg = ESIS_Reset + 2;                  // no. of registers
  AddCmdList( kSISKeys );                  // CBD-specific setup commands
}

//-----------------------------------------------------------------------------
TVME_SIS3820::~TVME_SIS3820( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_SIS3820::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file  
  switch( key ){
  case ESIS_XXX:
    break;
  case ESIS_YYY:
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_SIS3820::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  InitReg( SIS3820_reg );
  TVMEmodule::PostInit();
  Write(ESIS_CSR,0xfb);    // Enable control inputs, user LED on
  return;
}

//-------------------------------------------------------------------------
Bool_t TVME_SIS3820::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = (Read(ESIS_ModID))>>16;
  fprintf(fLogStream,"SIS3820 firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<SIS3820 firmware ID error>",EErrFatal);
  return kFALSE;
}
ClassImp(TVME_SIS3820)
