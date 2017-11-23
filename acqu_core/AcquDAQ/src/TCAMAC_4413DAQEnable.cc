//--Author	JRM Annand   11th Sep 2010 Cut-down version of spill
//--Rev 	
//--Update      	
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_4413DAQEnable
// Implement programable pulse generator from  LRS 4413 discriminator
// Used to set/reset the trigger-inhibit latch....no TCS stuff

#include "TCAMAC_4413DAQEnable.h"


//-----------------------------------------------------------------------------
TCAMAC_4413DAQEnable::TCAMAC_4413DAQEnable( Char_t* name, Char_t* input, 
					    FILE* log, Char_t* line ):
  TCAMAC_4413SPILL( name, input, log, line )
{
  // Basic initialisation
  fTCSresetDelay = 0; 
}

//-----------------------------------------------------------------------------
TCAMAC_4413DAQEnable::~TCAMAC_4413DAQEnable( )
{
  // Clean up
}

//-------------------------------------------------------------------------
void TCAMAC_4413DAQEnable::RunTrigCtrl()
{
  // Enable DAQ Latch Only
  EnableTrigCtrl();           // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TCAMAC_4413DAQEnable::EndTrigCtrl()
{
  // Disable DAQ Latch Only
  DisableTrigCtrl();
  return;
}

//--------------------------------------------------------------------------

ClassImp(TCAMAC_4413DAQEnable)
