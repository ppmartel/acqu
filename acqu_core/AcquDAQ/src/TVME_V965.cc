//--Author	JRM Annand   6th Jul 2012 Inherit from V792
//--Rev 	JRM Annand
//--Update	JRM Annand   2nd Nov 2017 DMA support
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V965
// CAEN VMEbus 16-channel dual-range QDC
// 1st 16 channels for low range convertion 100 pC full scale
// 2nd 16 channels for high range convertion 900 pC full scale
//

#include "TVME_V965.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


//-----------------------------------------------------------------------------
TVME_V965::TVME_V965( Char_t* name, Char_t* file, FILE* log, Char_t* line )
:TVME_V792( name, file, log, line )
{
  // Basic initialisation 
  fHardID = 965;                           // ID read from hardware
}

//-----------------------------------------------------------------------------
TVME_V965::~TVME_V965( )
{
  // Clean up
}

//---------------------------------------------------------------------------
void TVME_V965::ReadIRQ( void** outBuffer )
{
  // Read and decode the memory of the V965
  // Store the decoded ADC index and value in *outBuffer
  // Errors in contents of header word considered fatal
  // This version decodes a single event and then resets the data buffer
  //
  // DMA readout
  if(fIsDMA){
    UInt_t* buff;
    if(fDMAnPkt == 1){
      if(!CheckStatus())
	return;
      buff = (UInt_t*)fPCtrlMod->DMAread();    // perform DMA
    }
    buff = (UInt_t*)fMemDMA;                   // DMA buffer with data offset
    UInt_t datum = *buff++;                    // data header
    if( (datum & 0x7000000) != 0x2000000 ){    // check its a header word
      ErrorStore(outBuffer, EErrDataFormat);
      ResetData();
      return;
    }
    Int_t nword = (datum & 0x3f00) >> 8;    // # data words
    //
    UInt_t range,oflow;
    UShort_t adcVal, adcIndex;                // adc value and index
    for(Int_t i=0; i<nword; i++ ){
      datum = *buff++;
      range = (datum & 0x10000) >> 16;        // high or low range QDC
      oflow = (datum & 0x1000) >> 12;         // overflow bit
      //if( range )
      //  continue;                           // take high range for now
      adcVal = datum & 0xfff;                 // ADC value
      if(oflow)
	adcVal = 4095;
      adcIndex = (datum & 0x1e0000) >> 17;    // ADC subaddress
      adcIndex += fBaseIndex;                 // index offset
      if(!range)
	adcIndex += 16;                       // high range 2nd set 16 chan
      ADCStore( outBuffer, adcVal, adcIndex );// store values
    }
    datum = *buff++; 
    if( (datum & 0x7000000) != 0x4000000 ){   // Check trailer word
      ErrorStore(outBuffer, EErrDataFormat);
    }
    ResetData();
    return;
  }
  //
  // Standard readout
  UShort_t status1 = Read(EV7XX_Status1);    // read status reg. for data avail
  if( !(status1 & 0x1) || (status1 & 0x4) ){ // is there any data?
    ResetData();
    return;
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
  UInt_t range,oflow;
  UShort_t adcVal, adcIndex;                // adc value and index
  for( i=1; i<=nword; i++ ){
    datum = Read(i);
    range = (datum & 0x10000) >> 16;        // high or low range QDC
    oflow = (datum & 0x1000) >> 12;         // overflow bit
    //if( range )
    //  continue;                           // take high range for now
    adcVal = datum & 0xfff;                 // ADC value
    if(oflow)
      adcVal = 4095;
    adcIndex = (datum & 0x1e0000) >> 17;    // ADC subaddress
    adcIndex += fBaseIndex;                 // index offset
    if(!range)
      adcIndex += 16;                       // high range 2nd set 16 chan
    ADCStore( outBuffer, adcVal, adcIndex );// store values
  }
  datum = Read(i); 
  if( (datum & 0x7000000) != 0x4000000 ){   // Check trailer word
    ErrorStore(outBuffer, EErrDataFormat);
  }
  ResetData();
  return;
}

//-------------------------------------------------------------------------
Bool_t TVME_V965::CheckHardID( )
{

  // Read module ID from internal ROM
  Int_t id = Read(EV7XX_ID2) & 0xff;
  id |= (Read(EV7XX_ID1) & 0xff) << 8;
  id |= (Read(EV7XX_ID0) & 0xff) << 16;
  Int_t fw = Read(EV7XX_Firmware) & 0xffff;
  fprintf(fLogStream,"V965 ID Read: %d  Expected: %d  Firmware: %x\n",
	  id,fHardID,fw);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<CAEN V965 ADC hardware ID read error>",EErrFatal);
  return kFALSE;
}

ClassImp(TVME_V965)
