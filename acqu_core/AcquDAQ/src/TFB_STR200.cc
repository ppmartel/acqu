//--Author	Baya Oussena     15th Mar 2010
//--Rev 	Baya Oussena
//--Update      JRM Annand       29th Jul 2010 Remove extraneous stuff
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TFB_STR200
// Struck Fastbus 32-channel, 32-bit scaler
//

#include "TFB_STR200.h"
#include "TDAQexperiment.h"


//-----------------------------------------------------------------------------
TFB_STR200::TFB_STR200( Char_t* name, Char_t* file, FILE* log,
			Char_t* line ):
TFBmodule( name, file, log, line )
{
  // Basic initialisation 
    
  fC0_Reset = 0xc0000000;
  fC0_Config = 0x00000000;
  fC0_Test = 0xc00;
  fC0_Acqu = 0x00400000;
  fC0_Acqu_M = 0x0;
  fC0_Acqu_S = 0x0;
  fC0_Acqu_E = 0x0;
}

//-----------------------------------------------------------------------------
TFB_STR200::~TFB_STR200( )
{
  // Clean up
}
ClassImp(TFB_STR200)
