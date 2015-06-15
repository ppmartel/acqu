#include "HitClusterNextGen_t.h"
#include "TA2ClusterDetector.h"

#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>
#include <limits>

//______________________________________________________________________________
HitClusterNextGen_t::HitClusterNextGen_t( Char_t* line, UInt_t index )
                 :HitCluster_t(line, index, 32)
{
  fTimeMultiplicities = new Int_t[ fMaxHits ];
  fMoliereRadius = 4.0; // not too bad for NaI, BaF2, PbWO4...
}

//______________________________________________________________________________
Bool_t HitClusterNextGen_t::SetFields(const std::vector< crystal_t >& cluster,
    Double_t Energy,
    const TVector3 &MeanPosition) {
  if(cluster.size()>(UInt_t)fMaxHits)
    return kFALSE;
  fNhits = cluster.size();
  for(size_t i=0;i<cluster.size();i++) {
    fHits[i] = cluster[i].Index;
    fEnergies[i] = cluster[i].Energy;
    fTimes[i] = cluster[i].Time;
    fTimeMultiplicities[i] = cluster[i].TimeMultiplicity;
  }
  fHits[fNhits] = EBufferEnd;

  fIndex = cluster[0].Index; // assume cluster is sorted by descending energy
  fEnergy = Energy;
  fCentralFrac = cluster[0].Energy / Energy;
  fTime = cluster[0].Time;
  Double_t xyz[3];
  MeanPosition.GetXYZ(xyz);
  fMeanPosition->SetXYZ(xyz[0], xyz[1], xyz[2]);
  // precalculate theta/phi...
  fTheta = fMeanPosition->Theta();
  fPhi = fMeanPosition->Phi();
  return kTRUE;
}

//______________________________________________________________________________
Double_t HitClusterNextGen_t::calc_total_energy(const vector< crystal_t >& cluster) {
  Double_t energy = 0;
  for(size_t i=0;i<cluster.size();i++) {
    energy += cluster[i].Energy;
  }
  return energy;
}

//______________________________________________________________________________
Double_t HitClusterNextGen_t::opening_angle(const crystal_t& c1, const crystal_t& c2)  {
  // use TMath::ACos since it catches some NaN cases due to double rounding issues
  return TMath::RadToDeg()*TMath::ACos(c1.Position.Unit()*c2.Position.Unit());
}

//______________________________________________________________________________
Double_t HitClusterNextGen_t::calc_energy_weight(const Double_t energy, const Double_t total_energy,
                                                 const cluster_config_t& cfg) {
  if(cfg.WeightingType == EClustDetWeightingTypeLog) {
    Double_t wgtE = cfg.WeightingPar1 + TMath::Log(energy / total_energy);
    return wgtE<0 ? 0 : wgtE; // no negative weights
  }
  else if(cfg.WeightingType == EClustDetWeightingTypeScaledLog) {
    Double_t wgtE = cfg.WeightingPar1 + TMath::Log(energy / cfg.WeightingPar2);
    return wgtE<0 ? 0 : wgtE; // no negative weights
  }
  else if(cfg.WeightingType == EClustDetWeightingTypePower) {
    return TMath::Power(energy/cfg.WeightingPar2, cfg.WeightingPar1);
  }
  else if(cfg.WeightingType == EClustDetWeightingTypeRelPower) {
    return TMath::Power(energy/total_energy, cfg.WeightingPar1);
  }
  return 0;
}

//______________________________________________________________________________
void HitClusterNextGen_t::calc_bump_weights(const vector<crystal_t>& cluster, bump_t& bump) {
  // use local Moliere radius (NaI=4.8, BaF2=3.4, PbWO4=2.2)
  // given by config file
  Double_t w_sum = 0;
  for(size_t i=0;i<cluster.size();i++) {
    Double_t r = (bump.Position - cluster[i].Position).Mag();
    Double_t w = cluster[i].Energy*TMath::Exp(-2.5*r/cluster[i].MoliereRadius); // originally 2.5
    bump.Weights[i] = w;
    w_sum += w;
  }
  // normalize weights and find index of highest weight
  // (important for merging later)
  Double_t w_max = 0;
  size_t i_max = 0;
  for(size_t i=0;i<cluster.size();i++) {
    bump.Weights[i] /= w_sum;
    if(w_max<bump.Weights[i]) {
      i_max = i;
      w_max = bump.Weights[i];
    }
  }
  bump.MaxIndex = i_max;
}

//______________________________________________________________________________
void HitClusterNextGen_t::update_bump_position(const vector<crystal_t>& cluster, bump_t& bump,
                                               const cluster_config_t& cfg) {
  Double_t bump_energy = 0;
  for(size_t i=0;i<cluster.size();i++) {
    bump_energy += bump.Weights[i] * cluster[i].Energy;
  }
  TVector3 position(0,0,0);
  Double_t w_sum = 0;
  for(size_t i=0;i<cluster.size();i++) {
    Double_t energy = bump.Weights[i] * cluster[i].Energy;
    Double_t w = calc_energy_weight(energy, bump_energy, cfg);
    position += w * cluster[i].Position;
    w_sum += w;
  }
  position *= 1.0/w_sum;
  bump.Position = position;
}

//______________________________________________________________________________
bump_t HitClusterNextGen_t::merge_bumps(const vector<bump_t> bumps) {
  bump_t bump = bumps[0];
  for(size_t i=1;i<bumps.size();i++) {
    bump_t b = bumps[i];
    for(size_t j=0;j<bump.Weights.size();j++) {
      bump.Weights[j] += b.Weights[j];
    }
  }
  // normalize
  Double_t w_max = 0;
  size_t i_max = 0;
  for(size_t i=0;i<bump.Weights.size();i++) {
    bump.Weights[i] /= bumps.size();
    if(w_max<bump.Weights[i]) {
      i_max = i;
      w_max = bump.Weights[i];
    }
  }
  bump.MaxIndex = i_max;
  return bump;
}

//______________________________________________________________________________
void HitClusterNextGen_t::split_cluster(const vector<crystal_t>& cluster,
                                        vector< vector<crystal_t> >& clusters,
                                        const cluster_config_t& cfg) {

  // make Voting based on relative distance or energy difference

  Double_t totalClusterEnergy = 0;
  vector<UInt_t> votes;
  votes.resize(cluster.size(), 0);
  // start searching at the second highest energy (i>0 case in next for loop)
  // since we know that the highest energy always has a vote
  votes[0]++;
  for(size_t i=0;i<cluster.size();i++) {
    totalClusterEnergy += cluster[i].Energy; // side calculation in this loop, but include i=0
    if(i==0)
      continue;

    // i>0 now...
    // for each crystal walk through cluster
    // according to energy gradient
    UInt_t currPos = i;
    bool reachedMaxEnergy = false;
    Double_t maxEnergy = 0;
    while(!reachedMaxEnergy) {
      // find neighbours intersection with actually hit clusters
      reachedMaxEnergy = true;
      vector<UInt_t> neighbours = cluster[currPos].NeighbourIndices;
      for(size_t j=0;j<cluster.size();j++) {
        for(UInt_t n=0;n<neighbours.size();n++) {
          if(neighbours[n] != cluster[j].Index)
            continue; // cluster element j not neighbour of element currPos, go to next
          Double_t energy = cluster[j].Energy;
          if(maxEnergy < energy) {
            maxEnergy = energy;
            currPos = j;
            reachedMaxEnergy = false;
          }
          break; // neighbour indices are unique, stop iterating
        }
      }
    }
    // currPos is now at max Energy
    votes[currPos]++;
  }

  // all crystals vote for highest energy
  // so this cluster should not be splitted,
  // just add it to the clusters
  if(votes[0] == cluster.size()) {
    clusters.push_back(cluster);
    return;
  }

  // find the bumps (crystals voted for)
  // and init the weights
  typedef list<bump_t> bumps_t;
  bumps_t bumps;
  for(size_t i=0;i<votes.size();i++) {
    if(votes[i]==0)
      continue;
    // initialize the weights with the position of the crystal
    bump_t bump;
    bump.Position = cluster[i].Position;
    bump.Weights.resize(cluster.size(), 0);
    calc_bump_weights(cluster, bump);
    bumps.push_back(bump);
  }

  // as long as we have overlapping bumps
  Bool_t haveOverlap = kFALSE;

  do {
    // converge the positions of the bumps
    UInt_t iterations = 0;
    bumps_t stable_bumps;
    const Double_t positionEpsilon = 0.01;
    while(!bumps.empty()) {
      for(bumps_t::iterator b=bumps.begin(); b != bumps.end(); ++b) {
        // calculate new bump position with current weights
        TVector3 oldPos = (*b).Position;
        update_bump_position(cluster, *b, cfg);
        Double_t diff = (oldPos - (*b).Position).Mag();
        // check if position is stable
        if(diff>positionEpsilon) {
          // no, then calc new weights with new position
          calc_bump_weights(cluster, *b);
          continue;
        }
        // yes, then save it and erase it from to-be-stabilized bumps
        stable_bumps.push_back(*b);
        b = bumps.erase(b);
      }
      // check max iterations
      iterations++;
      if(iterations<100)
        continue; // go on iterating...
      // ... or we iterated really long without convergence
      // discard the bumps which havn't converged
      bumps.clear();
    }

    // do we have any stable bumps?
    // Then just the use cluster as is
    if(stable_bumps.empty()) {
      clusters.push_back(cluster);
      return;
    }


    // stable_bumps are now identified, form clusters out of it
    // check if two bumps share the same crystal of highest energy
    // if they do, merge them

    typedef vector< vector< bump_t > > overlaps_t;
    overlaps_t overlaps(cluster.size()); // index of highest energy crystal -> corresponding stable bumps
    haveOverlap = kFALSE;
    for(bumps_t::iterator b=stable_bumps.begin(); b != stable_bumps.end(); ++b) {
      // remember the bump at its highest energy
      overlaps[b->MaxIndex].push_back(*b);
    }

    for(overlaps_t::iterator o=overlaps.begin(); o != overlaps.end(); ++o) {
      if(o->size()==0) {
        continue;
      }
      else if(o->size()==1) {
        bumps.push_back(o->at(0));
      }
      else { // size>1 more than one bump at index, then merge overlapping bumps
        haveOverlap = kTRUE;
        bumps.push_back(merge_bumps(*o));
      }
    }

  } while(haveOverlap);

  // bumps contain non-overlapping, stable bumps
  // try to build clusters out of it
  // we start with seeds at the position of the heighest weight in each bump,
  // and similarly to build_cluster iterate over the cluster's crystals


  // populate seeds and flags
  typedef vector< vector<size_t> > bump_seeds_t;
  bump_seeds_t b_seeds; // for each bump, we track the seeds independently
  b_seeds.reserve(bumps.size());
  typedef vector< set<size_t> > state_t;
  state_t state(cluster.size()); // at each crystal, we track the bump index
  for(bumps_t::iterator b=bumps.begin(); b != bumps.end(); ++b) {
    size_t i = b_seeds.size();
    state[b->MaxIndex].insert(i);
    // starting seed is just the max index
    vector<size_t> single;
    single.push_back(b->MaxIndex);
    b_seeds.push_back(single);
  }

  Bool_t noMoreSeeds = kFALSE;
  while(!noMoreSeeds) {
    bump_seeds_t b_next_seeds(bumps.size());
    state_t next_state = state;
    noMoreSeeds = kTRUE;
    for(size_t i=0; i<bumps.size(); i++) {
      // for each bump, do next neighbour iteration
      // so find intersection of neighbours of seeds with crystals inside the cluster
      vector<size_t> seeds = b_seeds[i];
      for(size_t j=0;j<cluster.size();j++) {
        // skip crystals in cluster which have already been visited/assigned
        if(state[j].size()>0)
          continue;
        for(size_t s=0; s<seeds.size(); s++) {
          crystal_t seed = cluster[seeds[s]];
          for(size_t n=0;n<seed.NeighbourIndices.size();n++) {
            if(seed.NeighbourIndices[n] != cluster[j].Index)
              continue;
            // for bump i, we found a next_seed, ...
            b_next_seeds[i].push_back(j);
            // ... and we assign it to this bump
            next_state[j].insert(i);
            // flag that we found more seeds
            noMoreSeeds = kFALSE;
            // neighbours is a list of unique items, we can stop searching
            break;
          }
        }
      }
    }

    // prepare for next iteration
    state = next_state;
    b_seeds = b_next_seeds;
  }

  // now, state tells us which crystals can be assigned directly to each bump
  // crystals are shared if they were claimed by more than one bump at the same neighbour iteration

  // first assign easy things and determine rough bump energy
  vector< vector<crystal_t> > bump_clusters(bumps.size());
  vector< Double_t > bump_energies(bumps.size(), 0);
  for(size_t j=0;j<cluster.size();j++) {
    if(state[j].size()==1) {
      // crystal claimed by only one bump
      size_t i = *(state[j].begin());
      bump_clusters[i].push_back(cluster[j]);
      bump_energies[i] += cluster[j].Energy;
    }
  }

  // then calc weighted bump_positions for those preliminary bumps
  vector<TVector3> bump_positions(bumps.size(), TVector3(0,0,0));
  for(size_t i=0; i<bump_clusters.size(); i++) {
    vector<crystal_t> bump_cluster = bump_clusters[i];
    Double_t w_sum = 0;
    for(size_t j=0;j<bump_cluster.size();j++) {
      Double_t w = calc_energy_weight(bump_cluster[j].Energy, bump_energies[i], cfg);
      bump_positions[i] += w * bump_cluster[j].Position;
      w_sum += w;
    }
    bump_positions[i] *= 1.0/w_sum;
  }

  // finally we can share the energy of crystals claimed by more than one bump
  // we use bump_positions and bump_energies to do that
  for(size_t j=0;j<cluster.size();j++) {
    if(state[j].size()==1)
      continue;
    // size should never be zero, aka a crystal always belongs to at least one bump

    vector<Double_t> pulls(bumps.size());
    Double_t sum_pull = 0;
    for(set<size_t>::iterator b=state[j].begin(); b != state[j].end(); ++b) {
      TVector3 r = cluster[j].Position - bump_positions[*b];
      Double_t pull = bump_energies[*b] * TMath::Exp(-r.Mag()/cluster[j].MoliereRadius);
      pulls[*b] = pull;
      sum_pull += pull;
    }

    for(set<size_t>::iterator b=state[j].begin(); b != state[j].end(); ++b) {
      crystal_t crys = cluster[j];
      crys.Energy *= pulls[*b]/sum_pull;
      bump_clusters[*b].push_back(crys);
    }
  }

  for(size_t i=0; i<bump_clusters.size(); i++) {
    vector<crystal_t> bump_cluster = bump_clusters[i];
    // always sort before adding to clusters
    sort(bump_cluster.begin(), bump_cluster.end());
    clusters.push_back(bump_cluster);
  }
}


//______________________________________________________________________________
void HitClusterNextGen_t::build_cluster(list<crystal_t>& crystals,
                                        vector<crystal_t> &cluster) {
  list<crystal_t>::iterator i = crystals.begin();

  // start with initial seed list
  vector<crystal_t> seeds;
  seeds.push_back(*i);

  // save i in the current cluster
  cluster.push_back(*i);
  // remove it from the candidates
  crystals.erase(i);

  while(seeds.size()>0) {
    // neighbours of all seeds are next seeds
    vector<crystal_t> next_seeds;

    for(vector<crystal_t>::iterator seed=seeds.begin(); seed != seeds.end(); seed++) {
      // find intersection of neighbours and seed
      for(list<crystal_t>::iterator j = crystals.begin() ; j != crystals.end() ; ) {
        bool foundNeighbour = false;
        for(size_t n=0;n<(*seed).NeighbourIndices.size();n++) {
          if((*seed).NeighbourIndices[n] != (*j).Index)
            continue;
          next_seeds.push_back(*j);
          cluster.push_back(*j);
          j = crystals.erase(j);
          foundNeighbour = true;
          // neighbours is a list of unique items, we can stop searching
          break;
        }
        // removal moves iterator one forward
        if(!foundNeighbour)
          ++j;
      }
    }
    // set new seeds, if any new found...
    seeds = next_seeds;
  }

  // sort it by energy
  sort(cluster.begin(), cluster.end());
}

