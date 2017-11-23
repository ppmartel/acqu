//--Author	JRM Annand	25th Jan 2006
//--Rev 	JRM Annand
//--Update	B Oussena       ????????????? 
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_2373
// Dual,  8-bit input, 8-bit output Memory Lookup Unit (MLU)
// Latched input patterns may be read out

#include "TCAMAC_2373.h"


enum { E2373RAM=200, E2373CCR };
static Map_t k2373Keys[] = {
  { "RAM:",         E2373RAM },
  { "CCR:",         E2373CCR },
  { NULL,           -1 }
};

//-----------------------------------------------------------------------------
TCAMAC_2373::TCAMAC_2373( Char_t* name, Char_t* input, FILE* log,
			  Char_t* line ):
  TCAMACmodule( name, input, log, line )
{
  // Basic initialisation
  fType = EDAQ_ADC & EDAQ_SlowCtrl;         // slow control, also has read out
  AddCmdList( k2373Keys );                  // camac-specific cmds
  fCCR = fData = fAddr = NULL;
}

//-----------------------------------------------------------------------------
TCAMAC_2373::~TCAMAC_2373( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMAC_2373::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  switch( key ){
  case E2373RAM:
    // load internal lookup memory
    InitRAM( line );
    break;
  case E2373CCR:
    // Set CCR register
    if( sscanf( line, "%hx", &fCCRDatum ) != 1 )
      PrintError(line,"<CCR parse>");
    break;
  default:
    // default try commands of TDAQmodule
    TCAMACmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TCAMAC_2373::PostInit( )
{ 
  // General purpose CAMAC initialisation
  if( !fCCR ){
    PrintError("","<Not pre-initialised>");
    return;
  }
  //Write( fCCR, &fCCRDatum );    // config CCR for running <--- Baya Moved
  TCAMACmodule::PostInit();

  fCCR =  (UShort_t*)fReg[0];    // <<--- baya
  fData = (UShort_t*)fReg[1];   // <<--- baya
  fAddr = (UShort_t*)fReg[2];  // << --- baya

  //InitRAM2373();    // << --- baya

  Write( fCCR, &fCCRDatum );    // <<------     BAYA
}

//-----------------------------------------------------------------------------
void TCAMAC_2373::InitRAM( Char_t* line	)
{
  // Write values into the internal memory of the MLU
  UShort_t addr,datum;	  ;
  
  // Calculate write-access addresses of the internal memories
  if( !fCCR ){
    fReg[0] = fCCR = CNAF(fC,fN,2,16);    // C,N,2,16 CCR reg
    fReg[1] = fData = CNAF(fC,fN,0,16);   // load data reg.
    fReg[2] = fAddr = CNAF(fC,fN,1,16);   // memory address reg
    fNreg = fNInitReg = 3;                // general housekeeping
  }
  // Read offset address inside RAM, and datum to write at that address
  // An address of 0xffff means write datum to all 16384 memory addresses
  if(sscanf(line,"%hx%hx",&addr,&datum) != 2){
    PrintError(line,"<Parse Error>");
    return;
  }
  
}

//-----------------------------------------------------------------------------
void TCAMAC_2373::InitMEM2373()
{

  // Write values into the internal memory of the MLU
  UShort_t datum = 0x50;
  Write( fCCR, &datum );    // config CCR for memory setup
  

  // All memory addresses
  if( addr == 0xffff ){
    addr=0;
    for(Int_t i=0; i<E2373_MEM; i++){
      Write( fAddr, &addr );
      Write( fData, &datum );
      addr++;
    }
  }
  // Single memory address
  else {
    Write( fAddr, &addr );
    Write( fData, &datum );
  }

}

ClassImp(TCAMAC_2373)
