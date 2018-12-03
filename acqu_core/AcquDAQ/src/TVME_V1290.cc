//--Author	JRM Annand   1st Feb 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand  11th Jan 2008  1st working version
//--Rev 	JRM Annand...3rd Jun 2008..const Char_t*...gcc 4.3
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..24th Aug 2012..Start debugging
//--Rev 	JRM Annand..25th Sep 2012.."Working version"
//--Rev 	JRM Annand..25th Sep 2013..Edge detection control
//--Rev 	JRM Annand..12th Feb 2017..Explicit class for V1290 TDC
//--Update	JRM Annand...2nd Nov 2017..DMA support.
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V1290
// CAEN VMEbus 32-channel, 25-ps TDC
// Inherits most of setting up from TVME_V1190
//

#include "TVME_V1290.h"
#include "TDAQexperiment.h"
//
ClassImp(TVME_V1290)

//-----------------------------------------------------------------------------
TVME_V1290::TVME_V1290( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
TVME_V1190( name, file, log, line )
{
  // See TVME_V1190 for the bulk of the setting up
  // 
  fHardID = 0xa0500;                       // ID read from hardware
  fNBits = 16;                             // 16-bit ADC
}

//-----------------------------------------------------------------------------
TVME_V1290::~TVME_V1290( )
{
  // Clean up
}

//------------------------------------------------------------------------------
void TVME_V1290::ReadIRQ( void** outBuffer )
{
  // Read and decode the memory of the V1190
  // Store the decoded ADC index and value in *outBuffer
  // Errors in contents of header word considered fatal
  // This version decodes a single event and then resets the data buffer
  //
  if(fIsDMA){
    UInt_t* buff;
    if(fDMAnPkt == 1){
      if(!IsDataReady()){
	ResetData();
	return;
      }
      buff = (UInt_t*)fPCtrlMod->DMAread();    // perform DMA
    }
    buff = (UInt_t*)fMemDMA;                   // DMA buffer with data offset
    UInt_t datum = *buff++;              // data header
    if( !(datum & 0x40000000) ){   // check its a header word
      ErrorStore(outBuffer, EErrDataFormat);
      ResetData();
      return;
    }
    //
    Int_t adcVal, adcIndex;                // adc value and index
    for(Int_t i=0; i<=fMaxHits; i++ ){
      datum = *buff++;
      if( (datum & 0x80000000) ) break;       // global trailer
      adcVal = datum & 0x1fffff;              // ADC value 21 bits
      adcIndex = (datum & 0x3e00000) >> 21;   // ADC subaddress
      adcIndex += fBaseIndex;                 // index offset
      if( fEdgeDet == 0 ){                    // pair mode
	UShort_t val1 = adcVal & 0xfff;
	UShort_t val2 = adcVal>>12;
	ADCStore(outBuffer,val1,adcIndex);
	ADCStore(outBuffer,val2,adcIndex);
      }
      else ADCStore( outBuffer, adcVal, adcIndex );// store values
    } 
    if( (datum & 0x7000000) ){   // Check trailer word
      ErrorStore(outBuffer, EErrDataFormat);
    }
    ResetData();
    return;
  }
  else if(fIsBlk){
    // transfer data from virtual memory mapped data buffer
    // to local buffer in one block
    if(!IsDataReady()){
      ResetData();
      return;
    }
    memcpy(fBlkBuff,fReg[EV1190_OutBuff],fBlkSize); // block read
    UInt_t* buff = (UInt_t*)fBlkBuff;
    UInt_t datum = *buff++;              // data header
    if( !(datum & 0x40000000) ){         // check its a header word
      ErrorStore(outBuffer, EErrDataFormat);
      ResetData();
      return;
    }
    //
    Int_t adcVal, adcIndex;                // adc value and index
    for(Int_t i=0; i<=fMaxHits; i++ ){
      datum = *buff++;
      if( (datum & 0x80000000) ) break;       // global trailer
      adcVal = datum & 0x1fffff;              // ADC value 21 bits
      adcIndex = (datum & 0x3e00000) >> 21;   // ADC subaddress
      adcIndex += fBaseIndex;                 // index offset
      if( fEdgeDet == 0 ){                    // pair mode
	UShort_t val1 = adcVal & 0xfff;
	UShort_t val2 = adcVal>>12;
	ADCStore(outBuffer,val1,adcIndex);
	ADCStore(outBuffer,val2,adcIndex);
      }
      else ADCStore( outBuffer, adcVal, adcIndex );// store values
    } 
    if( (datum & 0x7000000) ){   // Check trailer word
      ErrorStore(outBuffer, EErrDataFormat);
    }
    ResetData();
    return;
  }
  // Normal read
  UShort_t status = Read(EV1190_Status);   // read status reg. for data avail
  if( !(status & 0x1) ) return;            // nothing there, return
  Int_t ich = EV1190_OutBuff;              // start register (data buffer)
  Int_t imax = ich + 1024;                 // max posible data extent
  UInt_t datum = Read(ich++);              // data header
  if( !(datum & 0x40000000) ){             // check its a header word
    ErrorStore(outBuffer, EErrDataFormat);
    ResetData();
    return;
  }
  //
  Int_t adcVal, adcIndex;                   // adc value and index
  for(Int_t i=ich; i<imax; i++ ){
    datum = Read(i);
    if( (datum & 0x80000000) ) break;       // global trailer
    adcVal = datum & 0x1fffff;              // ADC value 21 bits
    adcIndex = (datum & 0x3e00000) >> 21;   // ADC subaddress
    adcIndex += fBaseIndex;                 // index offset
    if( fEdgeDet == 0 ){                    // pair mode
      UShort_t val1 = adcVal & 0xfff;
      UShort_t val2 = adcVal>>12;
      ADCStore(outBuffer,val1,adcIndex);
      ADCStore(outBuffer,val2,adcIndex);
    }
    else ADCStore( outBuffer, adcVal, adcIndex );// store values
  } 
  if( (datum & 0x7000000) ){   // Check trailer word
    ErrorStore(outBuffer, EErrDataFormat);
  }
  ResetData();
  return;
}
