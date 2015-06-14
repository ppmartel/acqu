#include "HitClusterTrad_t.h"
#include "TA2ClusterDetector.h"

//______________________________________________________________________________
HitClusterTrad_t::HitClusterTrad_t(Char_t* line, UInt_t index, Int_t sizefactor,
                                   Double_t ewgt, Int_t lewgt)
    : HitCluster_t(line, index, sizefactor)
{
  fEWgt = ewgt;
  fLEWgt = lewgt;
}

//______________________________________________________________________________
void HitClusterTrad_t::ClusterDetermine(TA2ClusterDetector* cl)
{
  // Determine which hits form the cluster.
  // Basic algorithm: supply list of nearest neighbours for
  // each element in calorimeter. Any (not previously claimed) hit in one
  // of these neighbours is added to the cluster. Clusters are initially
  // centered on elements where the energy signal is a local maximum.
  // The fIsIterate = kTRUE flag signals that a further search for cluster-
  // connected hits which are NOT in the nearest neighbours list
  // should be made.
  // The position of the cluster is taken as the
  // sqrt(energy)-weighted mean of the individual positions of the
  // hit elements.
 
  UInt_t i,j,k;

  Double_t* energy = cl->GetEnergy();
  Double_t* time = cl->GetTime();
  UInt_t* hits = cl->GetTempHits();
  TVector3** pos = cl->GetPosition();
  UInt_t nhits = cl->GetNhits();
  Double_t wgtE;

  *fMeanPosition = *(pos[fIndex]);            // position = "centre"
  fEnergy = energy[fIndex];                   // energy in "central" element
  if(cl->IsTime())fTime = time[fIndex];       // time in central element
  if( !fEWgt ) wgtE = TMath::Sqrt(fEnergy);
  else if( !fLEWgt ) wgtE = TMath::Power( fEnergy, fEWgt );
  else wgtE = TMath::Log( fEnergy/fEWgt );
  fSqrtEtot = wgtE;
  *fMeanPosition = *(pos[fIndex]) * wgtE;    // position = "centre"
  TVector3 diff;
  fRadius = 0.0;
  fHits[0] = fIndex;
  
  // Accumulate weighted mean position
  for( i=0,k=1; i<nhits; i++ ){
    if( (j = hits[i]) == ENullHit ) continue; // was previously counted
    if( j == fIndex ){
      hits[i] = ENullHit;
      continue;
    }                                         // already got center
    if( IsNeighbour(j) ){                     // a neighbour of the center?
      hits[i] = ENullHit;                     // so its not double counted
      fHits[k] = j;                           // add to cluster hits collection
      if( !fEWgt ) wgtE = TMath::Sqrt(energy[j]);
      else if( !fLEWgt ) wgtE = TMath::Power( energy[j], fEWgt );
      else wgtE = TMath::Log( energy[j]/fEWgt );
      fEnergy += energy[j];
      fSqrtEtot += wgtE;
      *fMeanPosition += ( *(pos[j]) * wgtE );// root energy weighted pos
      diff = *(pos[fIndex]) - *(pos[j]);
      fRadius = fRadius + wgtE * diff.Mag();
      k++;
    }
  }
  fNhits = k;
  // If the iterate flag is TRUE check if any unused hits (ie NOT in the near
  // neighbour array) are connected to existing hits
  if( cl->IsIterate() ) MoreNeighbours( cl );

  fHits[fNhits] = EBufferEnd;                  // mark no more hits
  // Normalise weighted mean, get fraction total energy in central crystal,
  // calc circular polar coordinates of cluster center
  *fMeanPosition = (*fMeanPosition) * (1./fSqrtEtot);// normalise weighted mean
  fRadius = fRadius/fSqrtEtot;                // normalise weighted radius
  fCentralFrac = energy[fIndex]/fEnergy;      // fraction Etot in central elem
  fTheta = TMath::RadToDeg() * fMeanPosition->Theta();
  fPhi   = TMath::RadToDeg() * fMeanPosition->Phi();
}

//______________________________________________________________________________
void HitClusterTrad_t::MoreNeighbours( TA2ClusterDetector* cl ){
  // Assume 1st go at cluster member search complete.
  // Now scan for any other close-proximity detector hits for potential
  // additional members of the cluster
  // Adapted from HitClusterTAPS_t (F.Zehr, Basle 2005)

  UInt_t i,j,k,n;
  Double_t distApart;                          // distance between elements
  TVector3** pos = cl->GetPosition();          // element position array
  UInt_t* hits = cl->GetTempHits();            // hits array
  UInt_t nhits = cl->GetNhits();               // # hits
  Double_t* energy = cl->GetEnergy();          // array of energies
  UInt_t* nextIter;                            // -> relevent part fHits array
  UInt_t* hitsEnd = fHits + fMaxHits - 1;      // end-stop of fHits array
  Double_t wgtE;                              // square root energy weighting
  UInt_t pNhits = 0;                           // # previously processed hits

  // Iterate round while new cluster members still found
  do{
    nextIter = fHits + fNhits;                 // -> start new cluster members
    for( i=pNhits,n=0; i<fNhits; i++ ) {       // loop existing clust hits
      for ( j=0; j<nhits; j++ ) {              // loop total detector hits
	if ( (k = hits[j]) == ENullHit ) continue; // hit already spoken for
	// distance between cluster hit and potential new hit
	// if its within limits add the new hit to the cluster
	distApart = (*(pos[fHits[i]]) - *(pos[k])).Mag();
	if ( (distApart > cl->GetMinPosDiff()) &&
	     (distApart < cl->GetMaxPosDiff()) ) {
	  if( !fEWgt ) wgtE = TMath::Sqrt(energy[k]);
	  else wgtE = TMath::Power( energy[k], fEWgt );
	  fEnergy += energy[ k];
	  fSqrtEtot += wgtE;
	  *fMeanPosition += (*(pos[k])*wgtE);// pos root energy weighted
	  hits[j] = ENullHit;                 // mark hit as spoken for
	  *nextIter++ = k;                    // add index to cluster hits list
	  n++;                                // update # new clust members
	  // Check if space for futher cluster members, finish if not
	  if( nextIter >= hitsEnd ){
	    fNhits += n;
	    return;
	  }
	} 
      }
    }
    pNhits = fNhits;         // update previously processed hits
    fNhits += n;             // update total processed hits
  }while( n );               // iterate while new cluster members found
  return;
}

//______________________________________________________________________________
void HitClusterTrad_t::Merge(HitCluster_t* cl)
{
  // Merge cluster pointed to by cl with present cluster
  // merged-cluster position sqrt(Energy)-weighted sum of positions
  // merged-cluster Energy = sum of two energies

  Double_t sqE1,sqE2;
  if( !fEWgt ){
    sqE1 = TMath::Sqrt( fEnergy );
    sqE2 = TMath::Sqrt( cl->GetEnergy() );
  }
  else{
    sqE1 = TMath::Power( fEnergy, fEWgt );
    sqE2 = TMath::Power( cl->GetEnergy(), fEWgt );
  }
  TVector3 pos = (*fMeanPosition * sqE1) + (*(cl->GetMeanPosition()) * sqE2);
  *fMeanPosition = pos * (1./( sqE1 + sqE2 ));
  fTheta = TMath::RadToDeg() * fMeanPosition->Theta();
  fPhi   = TMath::RadToDeg() * fMeanPosition->Phi();
  Double_t totEnergy = fEnergy + cl->GetEnergy();
  fCentralFrac *= fEnergy/totEnergy;
  fEnergy = totEnergy;
}

