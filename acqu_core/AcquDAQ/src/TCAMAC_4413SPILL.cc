//--Author	Baya Oussena 13th Jul 2010
//--Rev 	Baya Oussena
//--Rev         JRM Annand   20th Jul 2010 Implement Run/End/Reset/Enab/Disab	
//--Rev         JRM Annand    6th Sep 2010 TrigSet & TrigExe after mapping	
//--Rev         JRM Annand    8th Sep 2010 TCS reset, pause between stop-start	
//--Update      JRM Annand   10th Sep 2010 Add fEXP->PostReset()...temp measure
//                                         Change fReg order F26 1st	
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_4413SPILL
// Implement programable pulse generator from  LRS 4413 discriminator
// Used to set/reset the trigger-inhibit latch and start/stop the CERN optical
// trigger control system

#include "TCAMAC_4413SPILL.h"



enum {E4413SPILLreset=200 };
static Map_t k4413SPILLKeys[] = {
  {"Reset-delay:",  E4413SPILLreset },
  { NULL,           -1 }
};


//-----------------------------------------------------------------------------
TCAMAC_4413SPILL::TCAMAC_4413SPILL( Char_t* name, Char_t* input, FILE* log,
				    Char_t* line ):
  TCAMACmodule( name, input, log, line )
{
  // Basic initialisation
  fType = EDAQ_SlowCtrl;                    // slow control
  AddCmdList( k4413SPILLKeys );             // module-specific setup
  fTrigSet = fTrigExe = NULL;
  fNInitReg = fNreg = fMaxReg = 0;
  fTCSresetDelay = 500;                     // default TCS wait 500 cycles
}

//-----------------------------------------------------------------------------
TCAMAC_4413SPILL::~TCAMAC_4413SPILL( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMAC_4413SPILL::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  //
  switch( key ){
  case E4413SPILLreset:
    // Set wait period for trigger control system to reset
    // The unit is in usleep periods. The default setting is usleep(500)
    if(sscanf(line,"%d",&fTCSresetDelay) != 1)
      PrintError(line,"<Invalid TCD reset delay period>",EErrFatal);
    break;
  default:
    // default try commands of TCAMACmodule
    TCAMACmodule::SetConfig(line, key);
    break;
  }
}
 
//-------------------------------------------------------------------------
//  Initialise 4413 to give beginning/end of spill signals for CATCH systems
//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::PostInit()
{
  // Assuming the pulser is a 4413 discriminator
  // fTrigSet = F16A0, write mask register (turn off all but desired channel)
  // fTrigExe = F25A0, test pulse
  fReg[1] = CNAF(fC,fN,0,16);
  fReg[2] = CNAF(fC,fN,0,25);
  TCAMACmodule::PostInit();
  fTrigSet = (UShort_t*)fReg[1];
  fTrigExe = (UShort_t*)fReg[2];
  return;
}

//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::RunTrigCtrl()
{
  // Start trigger control system and enable triggers
  // mask all channels except 0 (0-15) and execute F25 test
  // beginning of spill
  UShort_t data = 0xfffe;
  Write( fTrigSet, &data );   // mask all channels except 0
  Read( fTrigExe, &data );    // execute F25 test...generate pulse
  usleep(fTCSresetDelay);     // allow time TCS to reset itself
  fEXP->PostReset();          // any additional resetting after start
  EnableTrigCtrl();           // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::EndTrigCtrl()
{
  // Stop trigger control system and mark "end of spill"
  // and set overall inhibit
  DisableTrigCtrl();
  usleep(10);
  UShort_t data = 0xfffd;
  Write( fTrigSet, &data );   // mask all channels except 1 (0-15)
  Read( fTrigExe, &data );    // execute F25 test... generate pulse
  return;
}

//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::ResetTrigCtrl()
{
  // Disable triggers and reset the trigger control system.
  // Issue a stop followed by start pulse
  //
  DisableTrigCtrl();
  // Tell TCS end of spill
  UShort_t data = 0xfffd;
  Write( fTrigSet, &data );   // mask all channels except 1 (0-15)
  Read( fTrigExe, &data );    // execute F25 test... generate pulse
  usleep(1);                  // pause micro-sec
  // Tell TCS beginning of spill
  data = 0xfffe;
  Write( fTrigSet, &data );   // mask all channels except 0 (0-15)
  Read( fTrigExe, &data );    // execute F25 test... generate pulse
  usleep(fTCSresetDelay);     // allow time TCS to reset itself
  fEXP->PostReset();          // any additional resetting after start
  EnableTrigCtrl();           // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::EnableTrigCtrl()
{
  // Unset inhibit flip-flop to enable triggers
  //
  UShort_t data = 0xfffb;
  Write( fTrigSet, &data );   // mask all channels except 2 (0-15)
  Read( fTrigExe, &data );    // execute F25 test... generate pulse
}

//-------------------------------------------------------------------------
void TCAMAC_4413SPILL::DisableTrigCtrl()
{
  // Set inhibit flip-flop to disable triggers
  //
  UShort_t data = 0xfff7;
  Write( fTrigSet, &data );   // mask all channels except 3 (0-15)
  Read( fTrigExe, &data );    // execute F25 test... generate pulse
}
//--------------------------------------------------------------------------

ClassImp(TCAMAC_4413SPILL)
