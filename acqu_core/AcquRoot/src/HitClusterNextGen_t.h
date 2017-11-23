#ifndef __THitClusterNextGen_h__
#define __THitClusterNextGen_h__

#include "HitCluster_t.h"       // base hit cluster determination

#include <list>
#include <vector>
#include <set>

using namespace std;

class TA2ClusterDetector;

// yeah, C++11 offers scoped enums with "enum class", I know...
enum {
  EClustDetWeightingTypeLog,
  EClustDetWeightingTypeScaledLog,
  EClustDetWeightingTypePower,
  EClustDetWeightingTypeRelPower
};

struct cluster_config_t {
  UInt_t   WeightingType;
  Double_t WeightingPar1;
  Double_t WeightingPar2;
  cluster_config_t(UInt_t weightingType, Double_t weightingPar1, Double_t weightingPar2) :
    WeightingType(weightingType),
    WeightingPar1(weightingPar1),
    WeightingPar2(weightingPar2) {}
};

struct crystal_t  {
  UInt_t Index;
  Double_t Energy;
  Double_t Time;
  Int_t TimeMultiplicity;
  TVector3 Position;
  std::vector<UInt_t> NeighbourIndices; // potential neighbours
  Double_t MoliereRadius;
  crystal_t(const UInt_t index,
            const Double_t energy,
            const Double_t time,
            const Int_t timeMultiplicity,
            const TVector3& position,
            const UInt_t nNeighbours,
            const UInt_t* neighbours,
            const Double_t moliere
            ) :
    Index(index),
    Energy(energy),
    Time(time),
    TimeMultiplicity(timeMultiplicity),
    Position(position),
    MoliereRadius(moliere)
  {
    NeighbourIndices.assign(neighbours,neighbours+nNeighbours);
  }
};

typedef struct {
  TVector3 Position;
  vector<Double_t> Weights;
  size_t MaxIndex; // index of highest weight
} bump_t;

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

class HitClusterNextGen_t : public HitCluster_t
{
private:
  Int_t* fTimeMultiplicities;          // multiplicity of each crystal hit times
  Double_t fMoliereRadius;             // Moliere Radius of crystal with fIndex
public:
  HitClusterNextGen_t( char*, UInt_t );
  virtual ~HitClusterNextGen_t() { }
  virtual void ClusterDetermine(TA2ClusterDetector*) { }

  Int_t* GetTimeMultiplicities(){ return fTimeMultiplicities; }
  Double_t GetMoliereRadius() { return fMoliereRadius; }

  Bool_t SetFields(const std::vector<crystal_t> &cluster, // hit indices in cluster
      Double_t Energy,                    // Total energy deposited in cluster
      const TVector3& MeanPosition
      );
  void SetMoliereRadius(Double_t moliere) { fMoliereRadius = moliere; }

  static Double_t calc_total_energy(const vector< crystal_t >& cluster);
  static Double_t opening_angle(const crystal_t& c1, const crystal_t& c2);
  static Double_t calc_energy_weight(const Double_t energy, const Double_t total_energy,
                                     const cluster_config_t& cfg);
  static void calc_bump_weights(const vector<crystal_t>& cluster, bump_t& bump);
  static void update_bump_position(const vector<crystal_t>& cluster, bump_t& bump,
                                   const cluster_config_t& cfg);
  static bump_t merge_bumps(const vector<bump_t> bumps);
  static void split_cluster(const vector<crystal_t>& cluster,
                            vector< vector<crystal_t> >& clusters,
                            const cluster_config_t& cfg);
  static void build_cluster(list<crystal_t>& crystals,
                            vector<crystal_t> &cluster);
};

#endif

