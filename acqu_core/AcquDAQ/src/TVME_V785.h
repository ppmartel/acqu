//--Author	JRM Annand	11th Jan 2008
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Update	JRM Annand.. 3rd Nov 2016..cloan from TVME_V775
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V785
// CAEN VMEbus 32-channel VDC (analogue TAC front end)
//

#ifndef __TVME_V785_h__
#define __TVME_V785_h__

#include "TVME_V792.h"

class TVME_V785 : public TVME_V792 {
 protected:
 public:
  TVME_V785( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_V785();
  virtual void SetConfig( Char_t*, Int_t );   // configure VME module

  ClassDef(TVME_V785,1)   

    };

#endif

