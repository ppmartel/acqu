//--Author	JRM Annand   25th Jan 2012
//--Rev 	JRM Annand
//--Update	JRM Annand
//
//--Description
//
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMAC_GSI4800
// 48 channel, 24 bit CAMAC scaler from GSI

#include "TCAMAC_GSI4800.h"


//-----------------------------------------------------------------------------
TCAMAC_GSI4800::TCAMAC_GSI4800( Char_t* name, Char_t* input, FILE* log,
			  Char_t* line ):
  TCAMACmodule( name, input, log, line )
{
  // Basic initialisation
  fType = EDAQ_Scaler;                     // Scaler module
  fNInitReg = 2;                           // 2 init reg.
  fNIRQReg = 2;                            // 2 readout..hard wired
  fNTestReg = 0;                           // 0 test registers
  fNreg = fMaxReg = 5;                     // 5 total, include load start chan
}

//-----------------------------------------------------------------------------
TCAMAC_GSI4800::~TCAMAC_GSI4800( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMAC_GSI4800::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  switch( key ){
  default:
    // default try commands of TDAQmodule
    // note that standard CAMAC CNAF commands are not recognised
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TCAMAC_GSI4800::PostInit()
{
  // Hardwire the employed CNAFS
  // into the device registers
  // then call standard CAMAC PostInit

  fReg[EGSI4800_Reset] = CNAF(fC,fN,0,9);        // A0 F9 reset
  fReg[EGSI4800_Enable] = CNAF(fC,fN,0,1);       // A0 F1 enable
  fReg[EGSI4800_RdLSB] = CNAF(fC,fN,0,0);        // A0 F0 read least sig. bits
  fReg[EGSI4800_RdMSB] = CNAF(fC,fN,1,0);        // A1 F0 read most sig. bits
  fReg[EGSI4800_Load] = CNAF(fC,fN,0,16);        // A0 F16 load start channel
  for( Int_t i=0; i<fNreg; i++ ){
    fDW[i] = 2; 
    fData[i] = 0xfffff;                          // force a read operation
  }
  fData[EGSI4800_Load] = 0x0;   // start channel 0...this is a write
  TCAMACmodule::PostInit();
}

//-----------------------------------------------------------------------------
void TCAMAC_GSI4800::ReadIRQScaler( void** outBuffer )
{
  // This a special read for this hardware module only
  // Load start channel 0
  // Loop channel 0 - 47, read 8 msb and 16 lsb in two CNAF operations
  // Combine to a 24 bit word and store
  //
  Int_t i;
  UInt_t datum;
  Write(EGSI4800_Load);
  for(i=0; i<EGSI4800_NChan; i++){
    Read(EGSI4800_RdMSB);        // read 8 MSB (A1F0)
    Read(EGSI4800_RdLSB);        // read 16 LSB (A0F0)
    datum = (0xffff & fData[EGSI4800_RdLSB]) + 
      (fData[EGSI4800_RdMSB]<<16);           // combine 24b
    // Store 24 bit word
    ScalerStore( outBuffer, datum, fBaseIndex+i );
  }
  Read(EGSI4800_Reset);
  Read(EGSI4800_Enable);
}

ClassImp(TCAMAC_GSI4800)
