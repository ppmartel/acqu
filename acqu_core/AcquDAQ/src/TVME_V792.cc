//--Author	JRM Annand   1st Feb 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand  11th Jan 2008  1st working version
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand.. 2nd May 2011..no pedestal suppress option
//--Update	JRM Annand..13th Apr 2013..tidy up coding...fix pedestals
//--Update	D. Werthmueller..27th Jan 2014..added support for high threshold range
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V792
// CAEN VMEbus 32-channel QDC
// Also forms basis for classes to handle similar CAEN ADC hardware
//

#include "TVME_V792.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"

ClassImp(TVME_V792)

enum { ECAEN_Threshold=200, ECAEN_NoThreshold, ECAEN_IPed, ECAEN_FCWind, ECAEN_HighThrRange };
static Map_t kCAENKeys[] = {
  {"Threshold:",          ECAEN_Threshold},
  {"No-threshold:",       ECAEN_NoThreshold},
  {"Pedestal-Current:",   ECAEN_IPed},
  {"FC-Window:",          ECAEN_FCWind},
  {"High-Thr-Range:",     ECAEN_HighThrRange},
  {NULL,                  -1}
};

// Internal register offsets
VMEreg_t V792reg[] = {
  {0x0000,      0x0,  'l', 511},     // data buffer
  {0x1000,      0x0,  'w', 0},       // firmware version
  {0x1006,      0x0,  'w', 0},       // Bit set 1 register
  {0x1008,      0x0,  'w', 0},       // bit clear 1 register
  {0x100e,      0x0,  'w', 0},       // Status register 1
  {0x1010,      0x0,  'w', 0},       // Control register 1
  {0x1022,      0x0,  'w', 0},       // Status register 2
  {0x1024,      0x0,  'w', 0},       // Event counter low
  {0x1026,      0x0,  'w', 0},       // Event counter high
  {0x102e,      0x0,  'w', 0},       // Fast clear window
  {0x1032,      0x0,  'w', 0},       // Bit set 2 register
  {0x1034,      0x0,  'w', 0},       // bit clear 2 register
  {0x1040,      0x0,  'w', 0},       // Event counter reset
  {0x1060,      0x0,  'w', 0},       // Pedestal current
  {0x1080,      0x0,  'w', 31},      // Pedestal threshold registers
  {0x8036,      0x0,  'w', 0},       // Board ID MSB
  {0x803a,      0x0,  'w', 0},       // Board ID
  {0x803e,      0x0,  'w', 0},       // Board ID LSB
  {0xffffffff,  0x0,  'l', 0},       // end of list
};


//-----------------------------------------------------------------------------
TVME_V792::TVME_V792( Char_t* name, Char_t* file, FILE* log,
		      Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = NULL;                            // no control functions
  fType = EDAQ_ADC;                        // analogue to digital converter
  fNreg = EV7XX_ID2 + 1;                   // Last "hard-wired" register
  fHardID = 792;                           // ID read from hardware
  fNBits = 12;                             // 12-bit ADC
  fIsThreshold = kTRUE;                    // default suppress pedestals
  fIsHighThrRange = kFALSE;                // use low (thr x 2) threshold range 
  AddCmdList( kCAENKeys );                 // CAEN-specific setup commands
}

//-----------------------------------------------------------------------------
TVME_V792::~TVME_V792( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_V792::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  Int_t ithr, thr, i;
  switch( key ){
  case ECAEN_Threshold:
    // Specific setting of data threshold register
    if( sscanf(line,"%d%d",&ithr,&thr) != 2 ){
      PrintError(line,"<Parse threshold read>");
      break;
    }
    if( !fReg ){
      PrintError(line,"<Threshold setting, registers not yet initialised>");
      break;
    }
    if( ithr > fNChannel ){
      PrintError(line,"<Threshold index > # channels in module>");
      break;
    }
    if( fIsHighThrRange) fThresh[ithr] = thr>>4;   // divide by 16 (only 8 bits in register)
    else fThresh[ithr] = thr>>1;                   // divide by 2 (only 8 bits in register)
    break;
  case ECAEN_NoThreshold:
    // Turn off pedestal suppress
    fIsThreshold = kFALSE;
    break;
  case ECAEN_IPed:
    if( sscanf(line,"%d",&i) != 1 ){
      PrintError(line,"<Parse pedestal current read>");
      break;
    }
    fData[EV7XX_IPed] = i;
    break;
  case ECAEN_FCWind:
    if( sscanf(line,"%d",&i) != 1 ){
      PrintError(line,"<FC window read>");
      break;
    }
    fData[EV7XX_FCLRWind] = i;
    break;
  case ECAEN_HighThrRange:
    fIsHighThrRange = kTRUE;
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//---------------------------------------------------------------------------
void TVME_V792::ReadIRQ( void** outBuffer )
{
  // Read and decode the memory of the V792
  // Store the decoded ADC index and value in *outBuffer
  // Errors in contents of header word considered fatal
  // This version decodes a single event and then resets the data buffer
  //
  UShort_t status1 = Read(EV7XX_Status1);    // read status reg. for data avail
  Bool_t hasData = status1 & 0x1;
  Bool_t isBusy = status1 & 0x4;
  UInt_t nTries = 0;
  while(isBusy || !hasData){
    status1 = Read(EV7XX_Status1);
    hasData = status1 & 0x1;
    isBusy = status1 & 0x4;
    if(nTries>100){
      if(isBusy) ErrorStore(outBuffer, 1);
      if(!hasData) ErrorStore(outBuffer, 2);
      ResetData();
      return;
    }
    nTries++;
  }
  Int_t i = EV7XX_Outbuff;
  UInt_t datum = Read(i++);                 // data header
  if( (datum & 0x7000000) != 0x2000000 ){   // check its a header word
    ErrorStore(outBuffer, EErrDataFormat);
    ResetData();
    return;
  }
  Int_t nword = (datum & 0x3f00) >> 8;      // # data words
  //
  UShort_t adcVal, adcIndex;                // adc value and index
  for( i=1; i<=nword; i++ ){
    datum = Read(i);
    adcVal = datum & 0xfff;                 // ADC value
    adcIndex = (datum & 0x1f0000) >> 16;    // ADC subaddress
    adcIndex += fBaseIndex;                 // index offset
    ADCStore( outBuffer, adcVal, adcIndex );// store values
  }
  datum = Read(i); 
  if( (datum & 0x7000000) != 0x4000000 ){   // Check trailer word
    ErrorStore(outBuffer, EErrDataFormat);
    ResetData();
    return;
  }
  Reset();
  ResetData();
}

//-------------------------------------------------------------------------
void TVME_V792::PostInit( )
{
  // Check if any general register initialisation has been performed
  // Setup data memory, threshold registers and status registers.
  // Check if ID read from internal ROM matches the expected value
  // If OK carry out default init to perform write initialisation of registers
  if( fIsInit ) return;            // Init already done?
  InitReg(V792reg);
  if( fIsHighThrRange ) fData[EV7XX_BitSet2] = 0x4;
  else fData[EV7XX_BitSet2] = 0x104;
  // If no pedestal suppression set the "Low threshold enable bit 4
  if( !fIsThreshold )  fData[EV7XX_BitSet2] |= 0x10;
  fData[EV7XX_BitClr2] = 0x4;                       // clear data reset
  TVMEmodule::PostInit();
  // Software reset. This will reset many registers which potentially one
  // would want to program. The programming then has to be re-done after this
  // reset. Note that pedestal registers are not affected by a soft reset
  for(Int_t i=0; i<EV7XX_NThresh; i++){
    Write(EV7XX_Thresholds+i, fThresh[i]); // write pedestal threshold register
  }
  fData[EV7XX_BitSet1] = 0x80;             // software reset
  fData[EV7XX_BitClr1] = 0x80;             // clear software reset
  Reset();
  ResetData();
  return;
}

//-------------------------------------------------------------------------
Bool_t TVME_V792::CheckHardID( )
{

  // Read module ID from internal ROM
  Int_t id = Read(EV7XX_ID2) & 0xff;
  id |= (Read(EV7XX_ID1) & 0xff) << 8;
  id |= (Read(EV7XX_ID0) & 0xff) << 16;
  fprintf(fLogStream,"V792 ID Read: %d  Expected: %d\n",id,fHardID);
  Int_t fw = Read(EV7XX_Firmware) & 0xffff;
  fprintf(fLogStream,"Firmware version: %x\n",fw);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<CAEN V7** ADC/TDC hardware ID read error>",EErrFatal);
  return kFALSE;
}
