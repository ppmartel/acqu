#ifndef __HitClusterTrad_t_h__
#define __HitClusterTrad_t_h__

#include "HitCluster_t.h"

class HitClusterTrad_t : public HitCluster_t
{

protected:
    Double_t fEWgt;                 // energy weighting factor
    Int_t fLEWgt;                   // energy weighting factor switch

public:
    HitClusterTrad_t(Char_t* line, UInt_t index, Int_t sizefactor = 1,
                     Double_t ewgt = 0.0, Int_t lewgt = 0.0);
    virtual ~HitClusterTrad_t() { }

    virtual void ClusterDetermine(TA2ClusterDetector* cl);

    virtual void Merge(HitCluster_t* cl);
    virtual void MoreNeighbours(TA2ClusterDetector* cl);
};

#endif

