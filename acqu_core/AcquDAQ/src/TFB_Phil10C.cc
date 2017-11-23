//--Author	JRM Annand     29th Jul 2010 Start
//--Rev
//--Update
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TFB_Phil10C
// Class to decode data read out from Phillips 10C QDCs and TDCs
// These should all employ the same data format
//

#include "TFB_Phil10C.h"
#include "TFB_1821SMI.h"
#include "TDAQexperiment.h"


//-----------------------------------------------------------------------------
TFB_Phil10C::TFB_Phil10C( Char_t* name, Char_t* file, FILE* log,
			  Char_t* line ):
TFBmodule( name, file, log, line )
{
  // Basic initialisation 
  // Need to check this
    
  fC0_Reset = 0x80000000;
  fC0_Config = 0x08000000;
  fC0_Test = 0x0;
  fC0_Acqu = 0x40000000;
  fC0_Acqu_M = 0x10000000;
  fC0_Acqu_S = 0x20000000;
  fC0_Acqu_E = 0x0;
}

//-----------------------------------------------------------------------------
TFB_Phil10C::~TFB_Phil10C( )
{
  // Clean up
}

//-------------------------------------------------------------------------
void TFB_Phil10C::Decode(
			 UInt_t **I_buff,	// buffer which contains raw data 
			 void **P_buff,	// write buffer for refined data 
			 Int_t *n_word		// no. of words in I_buff 
			 )
{
  // Function to decode output from LeCroy 1880 series QDC's and TDC's
  // (held in buffer I_buff). Decoded output into P_buff.
  // This is for low range operation.
  // If in auto range and the range bit is set, then channel = 0xfff
  // ie full scale..equivalent to an overflow.
  //
  UShort_t *buff = (UShort_t*)(*P_buff);  // temp buffer pointer
  UShort_t *ibuff;
  UInt_t buff10C;	                  // raw datum word
  UShort_t a;		                  // read subaddr
  UShort_t datum;	                  // temp parm for storage
  Int_t na;                               // no. channels read

  // 1st CHECK FOR ANY READ ERRORS
  if( (buff10C = **I_buff) == EReadError){
    fSMIctrl-> smi_error(I_buff,P_buff,n_word);
    return;
  }
  // Check station number and no. words read
  if( ((buff10C & 0x1f0000)>>16) == (fN & 0x1f) || n_word <= 0 ) return;
  na = (buff10C & 0xfc00) >> 10;      // no. of channels
  (*I_buff)++;
  ibuff = (UShort_t *)(*I_buff);
  if(na & 0x1)			      // odd number ?
    na++;			      // account for dummy which is read
  *n_word -= (na>>1) + 1;	      // decr total words from module
  (*I_buff)++;
  // loop round channels
  while(na-- > 0) {
    datum = *ibuff++;	              // raw datum
    if(datum & 0x8000)	              // check if dummy word
      continue;
    a = ((datum & 0x7c00)>>10) + fBaseIndex;
    datum = datum & 0x3ff;
    *buff++ = datum;	              // save ADC/TDC data
    *buff++ = a;		      // save channel index
  }
  *I_buff = (UInt_t*)ibuff;
  *P_buff = buff;
  return;
}

ClassImp(TFB_Phil10C)
