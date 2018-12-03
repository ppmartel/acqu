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

#ifndef __TVME_V1290_h__
#define __TVME_V1290_h__

#include "TVME_V1190.h"

class TVME_V1290 : public TVME_V1190 {
 protected:
 public:
  TVME_V1290( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_V1290();
  virtual void ReadIRQ( void** );
  //
  ClassDef(TVME_V1290,1)
    };

//-----------------------------------------------------------------------------

#endif

