//--Author	JRM Annand   6th Jul 2012 Inherit from V792
//--Rev 	JRM Annand
//--Update	JRM Annand   2nd Nov 2017 DMA support
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V965
// CAEN VMEbus 16-channel dual-range QDC
//

#ifndef __TVME_V965_h__
#define __TVME_V965_h__

#include "TVME_V792.h"

//
class TVME_V965 : public TVME_V792 {
 protected:
 public:
  TVME_V965( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_V965();
  void ReadIRQ( void** );
  virtual Bool_t CheckHardID();
  ClassDef(TVME_V965,1)   
    };

#endif

