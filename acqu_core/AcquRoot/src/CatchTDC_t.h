//--Author	JRM Annand   9th March 2003
//--Rev 	JRM Annand
//--Update      D. Krambrich 5th April 2004 TDC overflow handler
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
// CatchTDC
// Catch multi-hit TDC info handler. All functions inlined in this header file

#ifndef __CatchTDC_h__
#define __CatchTDC_h__

#include "MultiADC_t.h"


enum { ETDCOverflow = 62054 };

class CatchTDC_t : public MultiADC_t
{
protected:
  UShort_t* fRef;                         // -> to storage area
public:
  CatchTDC_t( Int_t index, Int_t chan, UShort_t* refaddr ):
    MultiADC_t(index, chan){
    fRef = refaddr;
  }
  virtual ~CatchTDC_t(){
    // delete storage arrays...previously "newed" at construct
    if(fStore) delete fStore;
  }

  virtual void ApplyRef() {
    if( fRefApplied ) return; // Reference has already been subtracted

    // A simple algorithm is added to find the correct time difference
    // taking account of possible overflows in either the reference
    // or current channel (if neither or both overflow there is no problem)
    for(UInt_t i=0;i<fNstore; i++) {
      // this typecast back to UShort_t of fStore is VERY important!
      Int_t diff   = (UShort_t)fStore[i] - *fRef;
      Int_t diff_p = diff + ETDCOverflow;    // + overflow
      Int_t diff_m = diff - ETDCOverflow;    // - overflow
      Int_t min = (abs(diff) < abs(diff_p)) ? diff : diff_p;
      min = (abs(min)  < abs(diff_m)) ? min  : diff_m;
      fStore[i] = Short_t( min );
    }
    fRefApplied = true;
  }

};

#endif
