//--Author	JRM Annand    10th Feb 2007
//--Rev 	JRM Annand
//--Update	Baya Oussena  24th Nov 2011 left shift instead of right InitDelay
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_2323
// Dual,  programable gate and delay generator
// F9A0-1 Stop channels         F25A0-1 Start channels
// F1A0-1 Read delay registers  F17A0-1 Write delay registers

#include "TCAMAC_2323.h"


enum { E2323Delay=200 };
static Map_t k2323Keys[] = {
  { "Gate-Delay:",  E2323Delay },
  { NULL,           -1 }
};

//-----------------------------------------------------------------------------
TCAMAC_2323::TCAMAC_2323( Char_t* name, Char_t* input, FILE* log,
			  Char_t* line ):
  TCAMACmodule( name, input, log, line )
{
  // Basic initialisation
  fType = EDAQ_SlowCtrl;                    // slow control
  AddCmdList( k2323Keys );                  // module-specific cmds
}

//-----------------------------------------------------------------------------
TCAMAC_2323::~TCAMAC_2323( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMAC_2323::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file

  switch( key ){
  case E2323Delay:
    // load internal lookup memory
    InitDelay( line );
    break;
  default:
    // default try commands of TCAMACmodule
    TCAMACmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TCAMAC_2323::InitDelay( Char_t* line )
{
  // Construct delay register contents
  // Bits 0-9 delay in ns
  // Bits 10-12 exponent delay * 10**n
  // Bit 13 Set Latched, clear unlatched
  // Bits 14-15 delayed output width 0-10ns, 1-30ns, 2-100ns, 3-300ns
  // Errors in spec. considered fatal

  UInt_t index, latch, delay, exp, dwidth;

  if( sscanf(line,"%d%d%d%d%d",&index, &latch, &delay, &exp, &dwidth) < 5 )
    PrintError(line,"<Parse delay specification>", EErrFatal);
  if( (index>1) || (latch>1) || (delay>1023) || (exp>3) || (dwidth>3) )
    PrintError(line,"<Delay specification, value out of range>",EErrFatal);

  fData[index] = delay | (exp<<10) | (latch<<13) | (dwidth<<14);

}

ClassImp(TCAMAC_2323)
