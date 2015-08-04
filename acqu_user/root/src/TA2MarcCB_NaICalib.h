// Author: Marc Unverzagt
// Date:   18.01.12
//
// Previous Version of this class (TA2MarcCB_NaI) was made by the copy
// and paste method. This class was derived from TA2CalArray.
// This class represents the NaI array of the CB. In its decode method the
// calibration data with Am/Be are analysed.

#ifndef __TA2MarcCB_NaICalib_h__
#define __TA2MarcCB_NaICalib_h__

#include "TA2CalArray.h"

class TA2MarcCB_NaICalib : public TA2CalArray { 
 private:
  Double_t*  fCalEn;                 // Calibration "energy" for all clusters
  Double_t*  fCalEnSingle;           // Calibration "energy" for events with one cluster
  Double_t*  fCalEnSingleHit;        // Calibration "energy" for single hit clusters
  UInt_t* fClNHits;                   // Number of crystals in cluster
 public:
  TA2MarcCB_NaICalib( const char*, TA2System* );// Normal use  
  virtual void PostInit( );            // initialise using setup info
  virtual void Decode( );              // hits -> energy procedure
  virtual void LoadVariable( );        // display setup

  ClassDef(TA2MarcCB_NaICalib,1)
};

#endif
