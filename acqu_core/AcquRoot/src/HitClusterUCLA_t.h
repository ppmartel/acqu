//--Author	JRM Annand   30th Aug 2003
//--Rev 	A.Starostin..24th Jan 2004  add theta,phi
//--Rev 	JRM Annand   30th Jan 2004  mod neighbour determination
//--Rev 	JRM Annand   21st Oct 2004  cluster time
//--Rev 	JRM Annand    9th Mar 2005  protected instead of private vars
//--Rev 	JRM Annand   14th Apr 2005  energy fraction central crystal
//--Rev 	JRM Annand    6th Jun 2005  up to 48 nearest neighbours
//--Rev 	JRM Annand   13th Jul 2005  add Merge function
//--Rev 	JRM Annand   19th Oct 2005  up to 64 nearest neighbours
//--Rev 	JRM Annand   12th May 2007  mean radius
//--Update	D Glazier    24th Aug 2007  Add IsTime check
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// HitClusterUCLA_t
// Specimen for hit cluster determination in a segmented calorimeter
// Mean cluster position obtained from the sqrt(E) weighted sum
//
//---------------------------------------------------------------------------
#ifndef __HitClusterUCLA_t_h__
#define __HitClusterUCLA_t_h__

#include "TMath.h"
#include "TVector3.h"
#include "EnumConst.h"
#include "HitCluster_t.h"

//---------------------------------------------------------------------------

class TA2ClusterDetector;

class HitClusterUCLA_t : public HitCluster_t
{
 private:
 protected:
  TVector3* fMeanPosUp;                // energy-weighted mean pos. of clust. *sp*
  TVector3* fMeanPosDn;                // energy-weighted mean pos. of clust. *sp*
  Double_t fSqrtEtUp;                  // *sp*
  Double_t fSqrtEtDn;                  // *sp*
public:
  HitClusterUCLA_t(char*, UInt_t, Int_t = 1);
  virtual ~HitClusterUCLA_t();
  virtual void     Cleanup();
  virtual void     ClusterDetermine(TA2ClusterDetector*);
  virtual Bool_t   ClusterDetermine2(TA2ClusterDetector*);
  virtual Double_t ClusterRadiusUCLA(TA2ClusterDetector*);
  TVector3* GetMeanPosUp(){ return fMeanPosUp; }
  TVector3* GetMeanPosDn(){ return fMeanPosDn; }
  Double_t  GetSqrtEtUp() { return fSqrtEtUp; }
  Double_t  GetSqrtEtDn() { return fSqrtEtDn; }
};

//---------------------------------------------------------------------------

inline void HitClusterUCLA_t::Cleanup()
{
  // End-of-Event cleanup
  fSqrtEtot = (Double_t)ENullHit;
  fSqrtEtUp = 0.;
  fSqrtEtDn = 0.;
  fRadius = (Double_t)ENullHit;
  HitCluster_t::Cleanup();
}

//---------------------------------------------------------------------------

#endif
