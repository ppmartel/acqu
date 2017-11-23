//--Author	JRM Annand   3rd Sep 2013
//--Rev 	JRM Annand
//--Update	JRM Annand  17th Sep 2013  Mods for new firmware
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VITEC
// VITEC piggy back card to the KPh i686 VMEbus SBC
// Used for "interrupt" and event-ID processing
//

#include "TVME_VITEC.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


enum { EVIT_ReadoutPatternOffset=200, EVIT_YYY };
static Map_t kVITECKeys[] = {
  {"ReadoutPatternOffset:", EVIT_ReadoutPatternOffset},
  {"YYY:",                  EVIT_YYY},
  {NULL,                    -1}
};

// Internal register offsets
VMEreg_t VITECreg[] = {
  {0x0000,      0x0,  'w', 0},     // ECL input
  {0x0002,      0x0,  'w', 0},     // ECL output
  {0x0004,      0x0,  'w', 0},     // NIM input
  {0x0006,      0x0,  'w', 0},     // NIM output
  {0x0008,      0x0,  'w', 0},     // Event ID LSB
  {0x000a,      0x0,  'w', 0},     // Event ID MSB
  {0x000c,      0x0,  'w', 0},     // Event ID status
  {0x000e,      0x0,  'w', 0},     // Firmware ID
  {0x0010,      0x0,  'w', 0},     // Bitpattern LSB
  {0x0012,      0x0,  'w', 0},     // Bitpattern MSB
  {0xffffffff,  0x0,  'w', 0},     // end of list
};

//-----------------------------------------------------------------------------
TVME_VITEC::TVME_VITEC( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = new TDAQcontrol( this );         // tack on control functions
  fType = EDAQ_SCtrl;                      // secondary controller
  AddCmdList( kVITECKeys );                // specific setup commands
  fNreg = EVIT_BitPatternHigh + 2;   // control and scaler registers
  fReadoutPatternOffset = 0;
  fReadoutPatternStatus = 1; // error flag high by default
  fReadoutPattern = 0;  
}

//-----------------------------------------------------------------------------
TVME_VITEC::~TVME_VITEC( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_VITEC::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file  
  switch( key ){
  case EVIT_ReadoutPatternOffset:
    if(sscanf(line,"%d",&fReadoutPatternOffset) != 1)
      PrintError(line,"<ReadoutPatternOffset>",EErrFatal);
    break;
  case EVIT_YYY:
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VITEC::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  InitReg( VITECreg );
  TVMEmodule::PostInit();
  SetIRQ();
  ResetIRQ();
  return;
}

//-------------------------------------------------------------------------
Bool_t TVME_VITEC::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read(EVIT_FwID);
  fprintf(fLogStream,"VITEC firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VITEC firmware ID error>",EErrFatal);
  return kFALSE;
}
ClassImp(TVME_VITEC)
