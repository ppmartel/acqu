//--Author	JRM Annand   30th Sep 2003  Read MC data
//--Rev 	JRM Annand...15th Sep 2003  Generalise methods
//--Rev 	JRM Annand....9th Mar 2004  New OR variables
//--Rev 	JRM Annand....9th Mar 2005  protected instead of private vars
//--Rev 	JRM Annand...13th Jul 2005  split offs, time OR
//--Rev 	JRM Annand...25th Jul 2005  SetConfig hierarchy
//--Rev 	JRM Annand...20th Oct 2005  Iterative neighbour find (TAPS)
//--Rev 	JRM Annand... 9th Dec 2005  Change ambigous split-off thresh
//--Rev 	JRM Annand... 6th Feb 2006  Bug in SplitSearch
//--Rev 	JRM Annand...21st Apr 2006  Command-key enum to .h
//--Rev 	JRM Annand...22nd Jan 2007  4v0 update
//--Rev 	JRM Annand...12th May 2007  Central-frac and radius
//--Rev 	JRM Annand...18th Jan 2009  TMath::Sort (Root v5.22)
//--Update	JRM Annand   17th Sep 2011  log energy weighting
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2ClusterDetector
//
// Decoding and calibration methods for EM calorimeters or similar systems
// where a shower or showers of secondary particles fire a cluster or
// clusters neighbouring calorimeter elements,
// e.g. Crystal Ball NaI(Tl) array
//

#ifndef __TA2ClusterDetector_h__
#define __TA2ClusterDetector_h__

#include "TA2Detector.h"
#include "HitCluster_t.h"       // hit cluster determination

#include <list>
#include <vector>


#include "a2display.h"
#define MAX_DISP_CLUSTERS 8


// constants for command-line maps
enum {
  EClustDetMaxCluster = 100, EClustDetNeighbour, EClustDetMoliereRadius,
  EClustDetWeighting,
  EClustDetEnergy, EClustDetTime, EClustDetCentFrac, EClustDetRadius,
  EClustDetHits, EClustDetMulti
};

struct crystal_t  {
  UInt_t Index;
  Double_t Energy;
  Double_t Time;
  TVector3 Position;
  std::vector<UInt_t> NeighbourIndices; // potential neighbours
  Double_t MoliereRadius;
  crystal_t(const UInt_t index,
            const Double_t energy,
            const Double_t time,
            const TVector3& position,
            const UInt_t nNeighbours,
            const UInt_t* neighbours,
            const Double_t moliere
            ) :
    Index(index),
    Energy(energy),
    Time(time),
    Position(position),
    MoliereRadius(moliere)
  {
    NeighbourIndices.assign(neighbours,neighbours+nNeighbours);
  }
};

inline bool operator< (const crystal_t& lhs, const crystal_t& rhs){
  return lhs.Energy>rhs.Energy;
}

inline std::ostream& operator<< (std::ostream& o, const TVector3& c) {
  return o << "(" << c.X() << "," << c.Y() << "," << c.Z() << ")";
}

inline std::ostream& operator<< (std::ostream& o, const crystal_t& c) {
  return o << "Crystal Index=" << c.Index
           << " Energy=" << c.Energy
           << " Position " << c.Position;
}

class TA2ClusterDetector : public TA2Detector {
protected:
  HitCluster_t** fCluster;              // Clusters of hits
  UInt_t* fClustHit;                    // Cluster indices
  UInt_t fNCluster;                     // # of clusters
  UInt_t fMaxCluster;                   // Max # of clusters
  UInt_t* fNClustHitOR;                 // OR of #hits in individual clusters
  Double_t* fTheta;                     // theta of cluster hit
  Double_t* fPhi;                       // phi of cluster hit
  Double_t* fClEnergyOR;                // OR of cluster energies
  Double_t* fClTimeOR;                  // OR of cluster times
  Double_t* fClCentFracOR;              // OR of energy ratios in central elem.
  Double_t* fClRadiusOR;                // OR E-weighted cluster radii
  Double_t fEthresh;                    // generic threshold energy for cluster
  UInt_t   fClusterWeightingType;
  Double_t fClusterWeightingPar1;



  Bool_t fDispClusterEnable;
  TH2Crystals*  fDispClusterHitsAll;
  TH2Crystals** fDispClusterHitsSingle;
  TH2Crystals*  fDispClusterHitsEnergy;
  void DisplayClusters();

public:

  TA2ClusterDetector( const char*, TA2System* );// Normal use
  virtual ~TA2ClusterDetector();
  virtual void SetConfig( char*, int );// decode and load setup info
  virtual void ParseDisplay( char* );  // decode histogram setup lines
  virtual void LoadVariable(  );       // name-variable association
  virtual void PostInit( );            // initialise using setup info
  virtual void Decode( );              // hits -> energy procedure
  virtual void DecodeCluster( );       // determine clusters
  virtual void DecodeSaved( );         // decode previously written data
  virtual void Cleanup( );             // end-of-event cleanup
  virtual void SaveDecoded( ) = 0;     // specialist
  virtual void ReadDecoded( ) = 0;     // specialist
  virtual void DeleteClusterArrays();  // flush cluster-specific arrays

  HitCluster_t** GetCluster(){ return fCluster; }
  HitCluster_t* GetCluster( UInt_t i ){ return fCluster[i]; }
  UInt_t* GetClustHit(){ return fClustHit; }
  UInt_t* GetClustHit( UInt_t i ){ return fClustHit + i; }
  UInt_t GetNCluster(){ return fNCluster; }
  UInt_t GetMaxCluster(){ return fMaxCluster; }
  UInt_t* GetNClustHitOR(){ return fNClustHitOR; }
  Double_t* GetTheta(){ return fTheta; }
  Double_t* GetPhi(){ return fPhi; }
  Double_t* GetClEnergyOR(){ return fClEnergyOR; }
  Double_t* GetClTimeOR(){ return fClTimeOR; }
  Double_t* GetClCentFracOR(){ return fClCentFracOR; }
  Double_t* GetClRadiusOR(){ return fClRadiusOR; }
  Double_t GetClusterThreshold(){ return fEthresh; }

  ClassDef(TA2ClusterDetector,1)

};


#endif
