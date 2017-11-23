//--Author	JRM Annand   25th Jan 2006
//--Rev 	JRM Annand
//--Rev 	JRM Annand    8th Jan 2008 No write to hardware in InitRAM
//--Rev 	B.Oussena    26th Oct 2009 Bug fix reg offset InitRAM
//--Update	JRM Annand    5th Dec 2010 F9,F0's then RAM, no CNAF command 
//
//--Description
//
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_4508
// Dual,  8-bit input, 8-bit output Memory Lookup Unit (MLU)
// Latched input patterns may be read out via F0-A0,A1
// Setup of readout now hard wired.
// Setup of memory via "RAM:" commands, CAMAC "CNAF:" commands not recognised

#include "TCAMAC_4508.h"


enum { E4508RAM=200 };
static Map_t k4508Keys[] = {
  { "RAM:",         E4508RAM },
  { NULL,           -1 }
};

//-----------------------------------------------------------------------------
TCAMAC_4508::TCAMAC_4508( Char_t* name, Char_t* input, FILE* log,
			  Char_t* line ):
  TCAMACmodule( name, input, log, line )
{
  // Basic initialisation
  fType = EDAQ_ADC & EDAQ_SlowCtrl;         // slow control, also has read out
  AddCmdList( k4508Keys );                  // camac-specific cmds
  fRam0 = fRam1 = NULL;
  // 4 (F9) + 2 x 256 memory locations
  fNInitReg = E4508_INIT + (E4508_MEM * 2);
  fNIRQReg = 2;                            // 2 readout reg. F0-A0,A1
  fMaxReg = fNreg = fNInitReg + fNIRQReg;  // 2 extra for readout of patterns
}

//-----------------------------------------------------------------------------
TCAMAC_4508::~TCAMAC_4508( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMAC_4508::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  switch( key ){
  case E4508RAM:
    // Setup the internal RAM of the MLU
    InitRAM( line );
    break;
  default:
    // default try commands of TDAQmodule
    // note that standard CAMAC CNAF commands are not recognised
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TCAMAC_4508::Init( )
{ 
  // General purpose CAMAC initialisation
}

//-----------------------------------------------------------------------------
void TCAMAC_4508::InitRAM( Char_t* line	)
{
  // Write values into the internal memory of the MLU
  UShort_t addr,datum;	  
  Int_t ram,i,j;
  
  // Calculate write-access addresses of the internal memories
  // 1st include F9 A0-3 to make a complete reset before loading memories
  if( !fRam0 ){
    fRam0 = CNAF(fC,fN,2,16);       // C,N,2,16 Ram 0
    fRam1 = CNAF(fC,fN,3,16);       // C,N,3,16 Ram 1
    for( i=0; i<E4508_INIT; i++ ){  // Give F9 A0-3 to reset everything
      fReg[i] = CNAF(fC,fN,i,9);
      fDW[i] = 2;
      fData[i] = 0xfffff;
    }
    // Setup the CNAFS to read the input patterns
    // Only used if the modules is marked down as an ADC
    for( i=0; i<E4508_RD; i++ ){   // Setup the pattern readout address
      j = E4508_INIT + (2 * E4508_MEM) + i;
      fReg[j] = CNAF(fC,fN,i,0);
      fDW[j] = 2;
      fData[j] = 0;
    }
  }
  // Read RAM#, offset address inside RAM, and datum to write at that address
  // An address of 0xffff means write datum to all 256 memory addresses
  if(sscanf(line,"%d%hx%hx",&ram,&addr,&datum) != 3){
    PrintError(line,"<RAM setup parse error>");
    return;
  }
  UShort_t* pRam;
  switch(ram){
  default:
    PrintError(line,"<Invalid RAM>");
    break;
  case 0:
    pRam = fRam0;
    break;
  case 1:
    pRam = fRam1;
    break;
  }
  // All memory addresses
  if( addr == 0xffff ){
    for(i=0; i<E4508_MEM; i++){
      addr = ConvAddr( (UShort_t)i, datum );
      j = i + ram*E4508_MEM + E4508_INIT;
      fReg[j] = pRam;
      fDW[j] = 2;
      fData[j] = addr;
    }
  }
  // Single memory address
  else {
    i = addr;
    addr = ConvAddr( addr, datum );
    j = i + ram*E4508_MEM + E4508_INIT;
    fReg[j] = pRam;
    fDW[j] = 2;
    fData[j] = addr;
  }
}

ClassImp(TCAMAC_4508)
