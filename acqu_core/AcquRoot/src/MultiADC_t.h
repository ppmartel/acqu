//--Author	JRM Annand   9th Mar 2003
//--Update	JRM Annand  24th Jan 2004 Exclude multiple entries during event
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
// MultiADC
// Multi-hit ADC info handler. All functions inlined in this header file
// fNtry added in case a the # multiple hits exceeds those defined in the
// storage array

#ifndef __MultiADC_h__
#define __MultiADC_h__

#include "EnumConst.h"                    // general constants
#include "DataFormats.h"                  // Acqu data formats
#include <Rtypes.h>

class MultiADC_t
{
protected:
  Short_t* fStore;                        // -> stored ADC values for 1 event
  UInt_t fIadc;                           // adc  index
  UInt_t fChan;                           // # of channels
  UInt_t fNstore;                         // # of channels stored in event
  UInt_t fNtry;                          // channels stored this entry
  Bool_t fRefApplied;
 public:
  MultiADC_t( Int_t index, Int_t chan ){
    // store ADC index and max # of hist (chan), create hit storage array
    fIadc = index;
    fChan = chan;
    fStore = new Short_t[chan];
    for(UInt_t i=0; i<fChan; i++ ) fStore[i] = ENullStore;
    fNstore = 0;
    fRefApplied = false;
  }
  virtual ~MultiADC_t(){
    // delete storage arrays...previously "newed" at construct
    if(fStore) delete fStore;
  }
  virtual void Fill( AcquBlock_t* d ){
    // Fill the multi-ADC array until it is completely filled
    // or until the ADC index on the input stream doesn't match fIadc.
    // The multi-ADC values are assumed to come consecutively.
    // If there are fewer than fChan values on the input stream
    // zero the remainder of the storage array
    fNtry = 0;
    while( d->id == fIadc ){
      if( fNstore < fChan ) {
        fStore[fNstore] = d->adc;
        fNstore++;
      }
      fNtry++;
      d++;
    }
  }

  // by default, we don't have a reference hit
  // so do nothing
  virtual void ApplyRef() {}

  virtual void Flush(){
    // Flush the store array...if anything stored
    // called at the end of an event
    if( !fNstore ) return;
    for( UInt_t i=0; i<fNstore; i++ )fStore[i] = ENullStore;
    fNstore = 0;
    fRefApplied = false;
  }

  // Getters for read-only variables
  UInt_t GetChan(){ return fChan; }
  UInt_t GetNstore(){ return fNstore; }
  UInt_t GetNtry(){ return fNtry; }
  UInt_t GetIadc(){ return fIadc; }
  Short_t* GetStore(){ return fStore; }
  Short_t GetHit( UInt_t i ){                  // Get individual array element
    if( (UInt_t)i < fNstore ) return fStore[i];
    else return (Short_t)ENullADC;             // Not defined this event
  }
  Short_t* GetHitPtr( UInt_t i ){              // array element ptr (hist)
    if( (UInt_t)i < fChan ) return (fStore + i);
    else return NULL;                           // out with the array
  }
};

#endif
