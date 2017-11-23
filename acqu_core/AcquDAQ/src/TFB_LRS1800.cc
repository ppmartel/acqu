//--Author	B. Oussena     5th Feb 2010  Start updates for KpHBoard
//--Rev         B. Oussena     10th Mar 2010 futher development
//--Rev 	JRM Annand     27th Jul 2010 Generalise 1800 module class
//--Update	JRM Annand      9th Sep 2010 Update Decode()
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TFB_LRS1800
// Class to decode data read out from LeCroy 1800 series of QDCs and TDCs.
// These should all employ the same data format
//

#include "TFB_LRS1800.h"
#include "TFB_1821SMI.h"
#include "TDAQexperiment.h"


//-----------------------------------------------------------------------------
TFB_LRS1800::TFB_LRS1800( Char_t* name, Char_t* file, FILE* log,
			  Char_t* line ):
TFBmodule( name, file, log, line )
{
  // Basic initialisation 
    
  fC0_Reset = 0x80000000;
  fC0_Config = 0x08000000;
  fC0_Test = 0x0;
  fC0_Acqu = 0x40000000;
  fC0_Acqu_M = 0x10000000;
  fC0_Acqu_S = 0x20000000;
  fC0_Acqu_E = 0x0;
}

//-----------------------------------------------------------------------------
TFB_LRS1800::~TFB_LRS1800( )
{
  // Clean up
}

//-------------------------------------------------------------------------
void TFB_LRS1800::Decode( UInt_t **outBuff, void **P_buff, Int_t *n_word )
{
  // Function to decode output from LeCroy 1880 series QDC's and TDC's
  // (held in buffer outBuff). Decoded output into P_buff.
  // This is for low range operation.
  // If in auto range and the range bit is set, then channel = 0xfff
  // ie full scale..equivalent to an overflow.
  //
  UInt_t buff_1880;	/* 1880 raw datum word */
  UShort_t a;		/* read subaddr */
  UShort_t datum;	/* temp parm for storage */

  // 1st CHECK FOR ANY READ ERRORS
  if( (buff_1880 = **outBuff) == EReadError){
    ErrorStore( (void**)outBuff, 1 );
    return;
  }
 
  // Loop round reading data from primary buffer as long as Geographic
  // address bits in data word match station no.
  // Then get channel address and add to base index
  while( ((buff_1880 & 0xf8000000)>>27) == (fN & 0x1f)){
    a = ((buff_1880 & 0x007f0000)>>16) + fBaseIndex;   
    datum = buff_1880 & 0xfff;	// actual adc/tdc datum
    ADCStore( P_buff, datum, a );
    (*n_word)--;	        // word counter
    if(*n_word <= 0)
      break;                    // all channels decoded
    (*outBuff)++;                // next input datum
    buff_1880 = **outBuff;
  }
  return;
}
ClassImp(TFB_LRS1800)
