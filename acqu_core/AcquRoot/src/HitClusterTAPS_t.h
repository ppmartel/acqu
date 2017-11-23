//--Author	JRM Annand   30th Aug 2003
//--Rev 	A.Starostin..24th Jan 2004  add theta,phi
//--Rev 	JRM Annand   30th Jan 2004  mod neighbour determination
//--Update	JRM Annand   21st Oct 2004  cluster time
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// THitCluster
// Specimen for hit cluster determination in a segmented calorimeter
// Mean cluster position obtained from the sqrt(E) weighted sum
//
//---------------------------------------------------------------------------
#ifndef __THitClusterTAPS_h__
#define __THitClusterTAPS_h__

#include "HitCluster_t.h"       // base hit cluster determination

class TA2ClusterDetector;

class HitClusterTAPS_t : public HitCluster_t
{
 public:
  HitClusterTAPS_t( char*, UInt_t );
  virtual ~HitClusterTAPS_t() { }
  virtual void ClusterDetermine(TA2ClusterDetector*);
};

//---------------------------------------------------------------------------

#endif

