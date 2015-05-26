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
//--Rev 	D Glazier    24th Aug 2007  Add IsTime check
//--Rev 	JRM Annand   20th Nov 2009  Adjustable energy weighting
//--Update	JRM Annand   17th Sep 2011  log energy weighting
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// HitCluster_t
// Specimen for hit cluster determination in a segmented calorimeter
// Mean cluster position obtained from the sqrt(E) weighted sum
//
//---------------------------------------------------------------------------
#ifndef __HitCluster_t_h__
#define __HitCluster_t_h__

#include "TMath.h"
#include "TVector3.h"
#include "EnumConst.h"
#include <vector>

class TA2ClusterDetector;
class crystal_t;

class HitCluster_t{
protected:
  TVector3* fMeanPosition;             // energy-weighted mean pos. of clust.
  UInt_t* fNeighbour;                  // indices of neighbouring elements
  UInt_t fIndex;                       // index of central element
  Double_t fEnergy;                    // Total energy deposited in cluster
  Double_t fCentralFrac;               // Fractional energy in central crystal
  Double_t fTime;                      // Time of max-energy element of cluster
  Double_t fTheta;                     // Cluster's theta
  Double_t fPhi;                       // Cluster's phi
  Double_t fRadius;                    // energy-weighted "radius" of cluster
  UInt_t fNNeighbour;                  // # neighbour elements in array
  Int_t fMaxHits;                      // size of hits array
  UInt_t* fHits;                       // indices of hit elements
  Double_t* fEnergies;                 // energies of hit elements  
  Double_t* fTimes;                    // times of hit elements  
  Int_t* fTimeMultiplicities;          // multiplicity of each crystal hit times
  UInt_t fNhits;                       // # of hits in cluster
  Double_t fMoliereRadius;             // Moliere Radius of crystal with fIndex
public:
  HitCluster_t(Char_t* line, UInt_t index);
  virtual ~HitCluster_t();
  virtual void Cleanup();
  virtual Bool_t IsNeighbour( UInt_t );
  //
  TVector3* GetMeanPosition() { return fMeanPosition; }
  UInt_t* GetNeighbour(){ return fNeighbour; }
  UInt_t GetIndex(){ return fIndex; }
  Double_t GetEnergy(){ return fEnergy; }
  Double_t GetCentralFrac(){ return fCentralFrac; }
  Double_t GetTime(){ return fTime; }
  Double_t GetTheta(){ return fTheta;}
  Double_t GetPhi(){ return fPhi;}
  UInt_t GetNNeighbour(){ return fNNeighbour; }
  Int_t GetMaxHits(){ return fMaxHits; }
  UInt_t* GetHits(){ return fHits; }
  Double_t* GetEnergies(){ return fEnergies; }  
  Double_t* GetTimes(){ return fTimes; }  
  Int_t* GetTimeMultiplicities(){ return fTimeMultiplicities; }    
  UInt_t GetNhits(){ return fNhits; }
  //
  UInt_t* GetNhitsPtr(){ return &fNhits; }
  Double_t* GetEnergyPtr(){ return &fEnergy; }
  Double_t* GetTimePtr(){ return &fTime; }
  Bool_t SetFields(const std::vector<crystal_t> &cluster, // hit indices in cluster
      Double_t Energy,                    // Total energy deposited in cluster
      const TVector3& MeanPosition
      );
  void SetMoliereRadius(Double_t moliere) { fMoliereRadius = moliere; }
  Double_t GetMoliereRadius() { return fMoliereRadius; }
};

#endif
