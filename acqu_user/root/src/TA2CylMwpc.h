#ifndef __TA2CylMwpc_h__
#define __TA2CylMwpc_h__

// STD
#include <algorithm>
using std::equal;
#include <vector>
using std::vector;
#include <map>
using std::map;
using std::pair;
using std::make_pair;
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <fstream>

// ROOT
#include <TGraph2D.h>

// AcquRoot
#include "TA2WireChamber.h"
#include "TA2CylMwpcStrip.h"
#include "TA2CylMwpcWire.h"
#include "TA2Math.h"
#include "TA2MwpcTrack.h"
#include "TA2MwpcIntersection.h"

class TA2CylMwpc : public TA2WireChamber {
public:
  Bool_t fWait;
  virtual void Test(Bool_t); // for test

// Display
public:
  void InitGeometry();
private:
  TCanvas *c, *c2;
  TH2F * h, *h2;
  Bool_t fMwpcDisplay;

// automatic procedure for MWPC calibration
  Bool_t fIfCalib;
  std::ifstream parfile;

protected:
  // Geometrical constants
  Double_t	*fR;	// chamber wire radii
  Double_t	*fRtI;	// chamber inner strip radii
  Double_t	*fRtE;	// chamber external strip radii
  Double_t	*fC1;	// some const
  Double_t	*fC2;	// some const
  
  // Parametrs of the analysis
  static const Int_t kNullHit;
  static const Double_t kNullFloat;     // TODO Think better about global consts
  static const UInt_t kMaxNsolIE;	// Max number of the strip-strip solutions
  Double_t	fMaxDphiWIE;		// max w-ie intersections dPhi
  Double_t	fMaxDr;			// max distance between 2 intersections forming a track
public:
  // Parameters
  virtual const Double_t *GetR() const { return fR; }
  
protected:
  // Layers
  TA2CylMwpcStrip **fI;		// ptr to an internal strips layer
  TA2CylMwpcStrip **fE;		// ptr to an external strips layer
  TA2CylMwpcWire  **fW;		// ptr to a wires layer
  Bool_t **fUsedClW;	// a used/unused wires clusters puttern
  Bool_t **fUsedClI;	// a used/unused internal strips clusters puttern
  Bool_t **fUsedClE;	// a used/unused external strips clusters puttern
public:
  virtual TA2CylMwpcStrip *GetStripsI(const Int_t) const;
  virtual TA2CylMwpcStrip *GetStripsE(const Int_t) const;
  virtual TA2CylMwpcWire  *GetWires(const Int_t) const;
  virtual void ResetWiresAndStripsUsage(const Int_t);
  virtual void ResetWiresAndStripsUsage();
  virtual Bool_t IsUsedClW(const Int_t, const Int_t) const;
  virtual Bool_t IsUsedClI(const Int_t, const Int_t) const;
  virtual Bool_t IsUsedClE(const Int_t, const Int_t) const;
  virtual void SetUsedClW(const Int_t, const Int_t);
  virtual void SetUsedClI(const Int_t, const Int_t);
  virtual void SetUsedClE(const Int_t, const Int_t);
  virtual Bool_t IsUsedWIE(const Int_t, const Int_t, const Int_t, const Int_t) const;
  virtual Bool_t IsUsedWIE(const Int_t, const Int_t) const;
  virtual Bool_t IsUsedWIE(const Int_t) const;
  virtual void SetUsedWIE(const Int_t, const Int_t, const Int_t, const Int_t);
  virtual void SetUsedWIE(const Int_t, const Int_t);
  virtual void SetUsedWIE(const Int_t);

// Intersections
protected:
  Int_t *fNinters;		// # of all intersections true + # of all intersection candidates
  TA2MwpcIntersection **fInters;// intersections + intersection candidates
  Int_t  *fNintersTrue;		// # of strip-strip-wire intersections
  Int_t **fIintersTrue;		// strip-strip-wire intersections indexes
  Int_t  *fNintersCandidate;	// # of intersection candidates (strip-wire or strip-strip intersections)
  Int_t **fIintersCandidate;	// intersection candidates (strip-wire or strip-strip intersections) indexes
  // IE intersections
  Int_t **fIclI;
  Int_t **fIclE;
  Double_t **fZie;
  Double_t **fPhiIE;
  Int_t *fNie;
  // WIE intersections
  map<Double_t,pair<Int_t,Int_t> > *fDphiWIE;
  // Geometrical corrections
  Double_t *fPhiCorrEI[3];
  Double_t *fPhiCorr;
  Double_t *fXcorr;
  Double_t *fYcorr;
  Double_t *fZcorr[8];
  // For histograms
  Int_t		**fTypeInter;	// intersection type
  Double_t	**fZInter;	// Z for a given intersection
  Double_t	**fPhiInter;	// Phi for a given intersection
  Double_t	**fDphiWS;	// w-ss intersections dPhi
  Double_t	**fCGclI;
  Double_t	**fCGclW;
  Double_t	**fCGclE;
  Double_t	**fAclIE;	// Sum of the strip signals amplitudes corresponding to the intersection
public:
  virtual void AddIntersection(const Int_t, const TA2MwpcIntersection::EType, const Int_t, const Int_t, const Int_t, const Double_t&, const Double_t&, const Double_t &, const Double_t&);
  virtual Bool_t TooManyInters(const Int_t iCh) const { return fNinters[iCh] >= fMaxIntersect; }
  // TODO Move to private
  virtual Double_t PhiCorrEI(const Int_t, const Double_t&, const Double_t&) const;
  virtual Double_t ZcorrEI(const Int_t, const Double_t&) const;
  virtual Double_t ZcorrEW(const Int_t, const Double_t&, const Double_t&) const;
  virtual Double_t ZinterIE(const Int_t, const Double_t&, const Double_t&, const Int_t, const Int_t) const;
  virtual Double_t PhiInterIE(const Int_t, const Double_t&, const Double_t&, const Int_t, const Int_t) const;
  virtual Double_t ZinterWI(const Int_t, const Double_t&, const Double_t&, const Int_t) const;
  virtual Double_t ZinterWE(const Int_t, const Double_t&, const Double_t&, const Int_t) const;
  virtual Double_t Xinter(const Int_t, const Double_t&) const;
  virtual Double_t Yinter(const Int_t, const Double_t&) const;
  virtual Double_t Zinter(const Int_t, Double_t) const;
  // All
  virtual const Int_t *GetNinters() const { return fNinters; }
  virtual const Int_t &GetNinters(const Int_t i) const { return fNinters[i]; }
  virtual const TA2MwpcIntersection *GetInters(const Int_t i) const { return fInters[i]; }
  virtual const TA2MwpcIntersection *GetInters(const Int_t i, const Int_t j) const { return fInters[i]+j; } // fInters[iCh][iInter]
  // True
  virtual const Int_t *GetNintersTrue() const { return fNintersTrue; }
  virtual const Int_t &GetNintersTrue(const Int_t i) const { return fNintersTrue[i]; }
  virtual const Int_t *GetIintersTrue(const Int_t i) const { return fIintersTrue[i]; }
  virtual const Int_t &GetIintersTrue(const Int_t i, const Int_t j) const { return fIintersTrue[i][j]; }
  virtual const TA2MwpcIntersection *GetInterTrue(const Int_t i, const Int_t j) const { return GetInters(i,fIintersTrue[i][j]); }
  // Candidate
  virtual const Int_t *GetNintersCandidate() const { return fNintersCandidate; }
  virtual const Int_t &GetNintersCandidate(const Int_t i) const { return fNintersCandidate[i]; }
  virtual const Int_t *GetIintersCandidate(const Int_t i) const { return fIintersCandidate[i]; }
  virtual const Int_t &GetIintersCandidate(const Int_t i, const Int_t j) const { return fIintersCandidate[i][j]; }
  virtual const TA2MwpcIntersection *GetInterCandidate(const Int_t i, const Int_t j) const { return GetInters(i,fIintersCandidate[i][j]); }
  //
  const Int_t *GetIclI(const Int_t i) const { return fIclI[i]; }
  const Int_t *GetIclE(const Int_t i) const { return fIclE[i]; }
  const Double_t *GetZie(const Int_t i) const { return fZie[i]; }
  const Double_t *GetPhiIE(const Int_t i) const { return fPhiIE[i]; }
  const Int_t *GetNie() const { return fNie; }
  const map<Double_t,pair<Int_t,Int_t> > *GetDphiWIE() const { return fDphiWIE; }
  //
  
// Tracks
protected:
  // TODO All map<> should be changed to multimap<>
  TA2MwpcTrack *fTracks;			// all possible tracks
  Int_t fNtracksTrue;				// # of possible true tracks
  map<Double_t,Int_t> fTracksTrue;		// sorted array of the possible true tracks
  Int_t fNtracksTrueCandidate;			// # of possible true candidate tracks
  map<Double_t,Int_t> fTracksTrueCandidate;	// sorted array of the possible true candidates tracks
  Int_t fNtracksCandidate;			// # of possible candidate tracks
  map<Double_t,Int_t> fTracksCandidate;		// sorted array of the possible candidates tracks
  Int_t fNtracksTrueBest;			// # of the best true tracks
  map<Double_t,Int_t> fTracksTrueBest;		// sorted array of the best true tracks
  Int_t fNtracksTrueCandidateBest;		// # of the best true candidate tracks
  map<Double_t,Int_t> fTracksTrueCandidateBest;	// sorted array of the best true candidates tracks
  Int_t fNtracksCandidateBest;			// # of best candidate tracks
  map<Double_t,Int_t> fTracksCandidateBest;	// sorted array of the best candidates tracks
  // For histograms
  Int_t    *fTypeTrack;		// a track type
  Double_t *fMagTrack;		// a track lenght
  Double_t *fThetaTrack;	// a track theta angle
  Double_t *fPhiTrack;		// a track phi angle
  Double_t *fDPhiInters;	// dPhi of the intersections forming a given track
  Double_t *fDZInters;		// dZ of the intersections forming a given track
  Double_t *fPsVertex[3];	// a pseudo vertex {x,y,z} for a given track
  Double_t *fPsVertexR;		// a pseudo vertex R for a given track
  Double_t *fEtrack;		// Sum of the strip signals amplitudes corresponding to the track
  
  void AddTrackTo(map<Double_t,Int_t>&);
public:
  virtual void MakeTracks(); // Make all possible tracks
  virtual void SelectBestTracks(const map<Double_t,Int_t>&, map<Double_t, Int_t>&);
  virtual Bool_t TooManyTracks() const { return fNTrack >= fMaxTrack; }
  //
  virtual const Int_t		 	*GetNtracksPtr() const { return &fNTrack; }
  virtual const Int_t		 	&GetNtracks() const { return fNTrack; }
  virtual const TA2MwpcTrack		*GetTracks() const { return fTracks; }
  virtual const TA2MwpcTrack		*GetTrack(const Int_t i) const { return fTracks+i; }
  virtual const TA2MwpcTrack		*FindTrack(const Int_t *inters) const; // Among fTracks, find a track contaning all intersections from inters
  virtual const TA2MwpcTrack		*FindTrack(const Int_t, const Int_t) const; // The above function for Nch = 2
  // True tracks (all possible)
  virtual const Int_t		 	*GetNtracksTruePtr() const { return &fNtracksTrue; }
  virtual const Int_t		 	&GetNtracksTrue() const { return fNtracksTrue; }
  virtual const map<Double_t,Int_t>	*GetTracksTrue() const { return &fTracksTrue; }
  // True candidate tracks (all possible)
  virtual const Int_t		 	*GetNtracksTrueCandidatePtr() const { return &fNtracksTrueCandidate; }
  virtual const Int_t		 	&GetNtracksTrueCandidate() const { return fNtracksTrueCandidate; }
  virtual const map<Double_t,Int_t>	*GetTracksTrueCandidate() const { return &fTracksTrueCandidate; }
  // Candidate tracks (all possible)
  virtual const Int_t		 	*GetNtracksCandidatePtr() const { return &fNtracksCandidate; }
  virtual const Int_t		 	&GetNtracksCandidate() const { return fNtracksCandidate; }
  virtual const map<Double_t,Int_t>	*GetTracksCandidate() const { return &fTracksCandidate; }
  // Best true tracks
  virtual const Int_t		 	*GetNtracksTrueBestPtr() const { return &fNtracksTrueBest; }
  virtual const Int_t		 	&GetNtracksTrueBest() const { return fNtracksTrueBest; }
  virtual const map<Double_t,Int_t>	*GetTracksTrueBest() const { return &fTracksTrueBest; }
  // Best true candidate tracks
  virtual const Int_t		 	*GetNtracksTrueCandidateBestPtr() const { return &fNtracksTrueCandidateBest; }
  virtual const Int_t		 	&GetNtracksTrueCandidateBest() const { return fNtracksTrueCandidateBest; }
  virtual const map<Double_t,Int_t>	*GetTracksTrueCandidateBest() const { return &fTracksTrueCandidateBest; }
  // Best candidate tracks
  virtual const Int_t		 	*GetNtracksCandidateBestPtr() const { return &fNtracksCandidateBest; }
  virtual const Int_t		 	&GetNtracksCandidateBest() const { return fNtracksCandidateBest; }
  virtual const map<Double_t,Int_t>	*GetTracksCandidateBest() const { return &fTracksCandidateBest; }

// Vertexes
protected:
  TVector3		*fVertexes;	// Vertexes
  pair<Int_t,Int_t>	*fItracks;	// vertex-tracks cross indexes
  // For the output hists
  Double_t	*fVert[3];	// a vertex {x,y,z}
  Double_t	*fVertR;	// a vertex R
  Double_t	*fTrackDist;	// distance between 2 tracks forming a given vertex
  //
  virtual void MakeVertexes(const map<Double_t,Int_t>&);	// Make vertexes
public:
  virtual const Int_t		&GetMaxVertex() const { return fMaxVertex; }
  virtual const Int_t		&GetNvertexes() const { return fNVertex; }
  virtual const Int_t		*GetNvertexPtr() const { return &fNVertex; }
  virtual const TVector3 	*GetVertex() const { return fVertexes; }
  virtual const pair<Int_t,Int_t> *GetItracks() const { return fItracks; }
  
// Simulation
protected:
  Bool_t fReadGeneratedKinematics; // when "true" run ReadGeneratedKinematics() instead of ReadMC()
  TGraph2D **fSigmaZ;		// sigmaZ(phi,z)
  TGraph2D **fSigmaPhi;		// sigmaPhi(phi,z)
  map<Int_t,TGraph2D*> *fDetEff;// Detection efficiency
  // For the output hists
  Double_t	fVertSim[3];	// simulated vertex {x,y,z}
  Double_t     *fDrHitsSim;
public:
  virtual Double_t	SmearZ(const Int_t, const Double_t&, const Double_t&) const;
  virtual Double_t	SmearPhi(const Int_t, const Double_t&, const Double_t&) const;
  
//
protected:
  virtual void	 DeleteArrays();         	// flush local new store
  virtual void	 IntersectLayers(Int_t);	// Find Wire-Strip-Strip intersections
  virtual void	 MarkEndBuffers();		// Mark EndBuffer for the output arrays
public:
  TA2CylMwpc( const char*, TA2System* );
  virtual ~TA2CylMwpc();
  virtual void	PostInit();
  virtual void	LoadVariable();
  virtual void	SetChamberParm(Int_t, Double_t*);
  virtual void  ReadDecoded();	// read back previous analysis
  virtual void  ReadGeneratedKinematics();
  virtual void  ReadMC();
  virtual void	Decode();
  virtual void	SetConfig( char*, Int_t );
  virtual void	Cleanup();	// Reset event
  
  // TODO: сделать заглушки на накоторые ненужные члены и методы, наследованные от TA2WireChamber (see TObject::MayNotUse())
  
  ClassDef(TA2CylMwpc,1)  // Cylindrical MWPC events reconstructions
};

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::PhiCorrEI(const Int_t iCh, const Double_t &zEI, const Double_t &phiEI) const
{
  // Return Phi correction for rotation of E relative to I as a function of not corrected Zei and PhiEI
  return fPhiCorrEI[0][iCh] + fPhiCorrEI[1][iCh]*zEI + fPhiCorrEI[2][iCh]*phiEI;
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::ZcorrEI(const Int_t iCh, const Double_t &phiCorrEI) const
{
  // Return Z correction of E relative to I
  
//   return fR[iCh]*fPhiCorrEI[0][iCh]; // Susanna
  return fRtE[iCh]*phiCorrEI;
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::ZcorrEW(const Int_t iCh, const Double_t &zEW, const Double_t &phiW) const
{
  // Return Z correction of E relative to I as a function of not corrected zEW and phiW
  return 2.*fRtE[iCh]*(fPhiCorrEI[0][iCh] + fPhiCorrEI[1][iCh]*zEW + fPhiCorrEI[2][iCh]*phiW)/(1.+fRtE[iCh]*fPhiCorrEI[1][iCh]);
}

//_________________________________________________________________________________________
inline Double_t TA2CylMwpc::ZinterIE(const Int_t iCh, const Double_t &phiI, const Double_t &phiE, const Int_t j, const Int_t k) const
{
  // Returns Z coordinate of the internal-external strips intersection
  // (in intrinsic frame of the iCh-th MWPC)
  return fC2[iCh]*(phiI+phiE+j+k);	// not corrected Z IE intersection
}

//_________________________________________________________________________________________
inline Double_t TA2CylMwpc::PhiInterIE(const Int_t iCh, const Double_t &phiI, const Double_t &phiE, const Int_t j, const Int_t k) const
{
  // Returns phi coordinate of the internal-external strips intersection
  // (in intrinsic frame of the iCh-th MWPC)
  return fC1[iCh]*(fRtE[iCh]*(phiE+k) - fRtI[iCh]*(phiI+j));	// not corrected Phi IE intersection
}

//_________________________________________________________________________________________
inline Double_t TA2CylMwpc::ZinterWI(const Int_t iCh, const Double_t &phiW, const Double_t &phiI, const Int_t m) const
{
  // Returns Z coordinate of wire and internal strip intersection
  // (in intrinsic frame of the iCh-th MWPC)
  return fRtI[iCh]*phiW + 2.*kPi*fRtI[iCh]*(phiI+m);
}

//_________________________________________________________________________________________
inline Double_t TA2CylMwpc::ZinterWE(const Int_t iCh, const Double_t &phiW, const Double_t &phiE, const Int_t k) const
{
  // Returns Z coordinate of wire and external strip intersection
  // (in intrinsic frame of the iCh-th MWPC)
  return -fRtE[iCh]*phiW + 2.*kPi*fRtE[iCh]*(phiE+k);	// not corrected Z WE intersection
}

//________________________________________________________________________________________
inline void TA2CylMwpc::AddIntersection(const Int_t iCh, const TA2MwpcIntersection::EType type, const Int_t iClI, const Int_t iClW, const Int_t iClE, const Double_t &z,const Double_t &phi, const Double_t &dPhiWIE, const Double_t &ampl)
{
  // Add a new intersection to the fInters[iCh] and increase fNinters[iCh] by 1
  
  // Adding
  TA2MwpcIntersection &inter = fInters[iCh][fNinters[iCh]];
  inter.Set(Xinter(iCh,phi),
	    Yinter(iCh,phi),
	    Zinter(iCh,z),
	    iClI,
	    iClW,
	    iClE,
	    type,
	    dPhiWIE,
	    ampl);
  
  // Fill arrays of "true" intersections or "candidate" intersections
  if (type == TA2MwpcIntersection::kEWI)
  {
    fIintersTrue[iCh][fNintersTrue[iCh]++] = fNinters[iCh];
    SetUsedWIE(iCh,iClI,iClW,iClE); // Mark the strips and wire clusters as "used"
  }
  else
  {
    fIintersCandidate[iCh][fNintersCandidate[iCh]++] = fNinters[iCh];
  }
  
  // Histograms
  fTypeInter[iCh][fNinters[iCh]] = inter.GetType();
  fZInter[iCh][fNinters[iCh]]    = inter.GetPosition()->Z();
  fPhiInter[iCh][fNinters[iCh]]  = TVector2::Phi_0_2pi(inter.GetPosition()->Phi())*kRadToDeg;
  fDphiWS[iCh][fNinters[iCh]]    = inter.GetDphiWIE()*kRadToDeg;
  fCGclI[iCh][fNinters[iCh]] = (iClI != kNullHit) ? fI[iCh]->GetCGClust(inter.GetIclI())*fI[iCh]->GetNElement() : kNullFloat;
  fCGclW[iCh][fNinters[iCh]] = (iClW != kNullHit) ? fW[iCh]->GetCGClust(inter.GetIclW())*fW[iCh]->GetNElement()/(2.*kPi) : kNullFloat;
  fCGclE[iCh][fNinters[iCh]] = (iClE != kNullHit) ? fE[iCh]->GetCGClust(inter.GetIclE())*fE[iCh]->GetNElement() : kNullFloat;
  fAclIE[iCh][fNinters[iCh]] = inter.GetAclIE();
  
  //
  ++fNinters[iCh];
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::Xinter(const Int_t iCh, const Double_t &phi) const
{
  // Return an absolute X coordinate for an intersection in the iCh-th MWPC
  // Corrections fPhiCorr and fXcorr are assumed to be constant for a given MWPC
  return fR[iCh]*TMath::Cos(phi+fPhiCorr[iCh]) + fXcorr[iCh];
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::Yinter(const Int_t iCh, const Double_t &phi) const
{
  // Return an absolute Y coordinate for an intersection in the iCh-th MWPC
  // Corrections fPhiCorr and fYcorr are assumed to be constant for a given MWPC
  return fR[iCh]*TMath::Sin(phi+fPhiCorr[iCh]) + fYcorr[iCh];
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::Zinter(const Int_t iCh, Double_t z) const
{
  // Return an absolute Z coordinate for an intersection in the iCh-th MWPC
  // Correction fZcorr depends on z of intersection
  
//   for (Int_t i=0;i<8;++i)
//   {
//     z += fZcorr[i][iCh]*TMath::Power(z,i);
//   }
  
  for (Int_t i=0; i<8; i+=2)
  {
    z += fZcorr[i][iCh] + fZcorr[i+1][iCh]*z;
  }
  
  return z;
}

//________________________________________________________________________________________
inline TA2CylMwpcStrip *TA2CylMwpc::GetStripsI(const Int_t iCh) const
{
  // Return pointer to the internal strips layer of the iCh-th chamber
  return fI[iCh];
}

//________________________________________________________________________________________
inline TA2CylMwpcStrip *TA2CylMwpc::GetStripsE(const Int_t iCh) const
{
  // Return pointer to the external strips layer of the iCh-th chamber
  return fE[iCh];
}

//________________________________________________________________________________________
inline TA2CylMwpcWire *TA2CylMwpc::GetWires(const Int_t iCh) const
{
  // Return pointer to the wires layer of the iCh-th chamber
  return fW[iCh];
}

//________________________________________________________________________________________
inline void TA2CylMwpc::ResetWiresAndStripsUsage(const Int_t iCh)
{
  // Reset wires and strips usage pattern for chamber iCh
  std::fill(fUsedClI[iCh],fUsedClI[iCh]+fI[iCh]->GetMaxClust(),kFALSE);
  std::fill(fUsedClW[iCh],fUsedClW[iCh]+fW[iCh]->GetMaxClust(),kFALSE);
  std::fill(fUsedClE[iCh],fUsedClE[iCh]+fE[iCh]->GetMaxClust(),kFALSE);
}

//________________________________________________________________________________________
inline void TA2CylMwpc::ResetWiresAndStripsUsage()
{
  // Reset wires and strips usage pattern for all chambers
  for (Int_t i=0; i<fNchamber; ++i) ResetWiresAndStripsUsage(i);
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedClW(const Int_t iCh, const Int_t iClW) const
{
  // Return TRUE if iClW-th wire cluster of iCh-th chamber is used or iClW = kNullHit
  return iClW != kNullHit && fUsedClW[iCh][iClW];
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedClW(const Int_t iCh, const Int_t iClW)
{
  // If iClW != kNullHit mark the iClW-th wire cluster of iCh-th chamber "used"
  if (iClW != kNullHit) fUsedClW[iCh][iClW] = kTRUE;
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedClI(const Int_t iCh, const Int_t iClI) const
{
   // Return TRUE if iClI-th internal strip cluster of iCh-th chamber is used or iClI = kNullHit
  return iClI != kNullHit && fUsedClI[iCh][iClI];
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedClI(const Int_t iCh, const Int_t iClI)
{
  // If iClI != kNullHit mark the iClI-th internal strip cluster of iCh-th chamber "used"
  if (iClI != kNullHit) fUsedClI[iCh][iClI] = kTRUE;
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedClE(const Int_t iCh, const Int_t iClE) const
{
  // Return TRUE if iClE-th external strip cluster of iCh-th chamber is used or iClE = kNullHit
  return iClE != kNullHit && fUsedClE[iCh][iClE];
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedClE(const Int_t iCh, const Int_t iClE)
{
  // If iClE != kNullHit mark the iClE-th external strip cluster of iCh-th chamber "used"
  if (iClE != kNullHit) fUsedClE[iCh][iClE] = kTRUE;
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedWIE(const Int_t iCh, const Int_t iClI, const Int_t iClW, const Int_t iClE) const
{
  // Check if wire, or external, or internal strip cluster of iCh-th chamber is used
  return IsUsedClW(iCh,iClW) || IsUsedClI(iCh,iClI) || IsUsedClE(iCh,iClE);
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedWIE(const Int_t iCh, const Int_t iInter) const
{
  // Check if wire, or external, or internal strip cluster forming the intersection iInter of iCh-th chamber is used
  return IsUsedWIE(iCh,fInters[iCh][iInter].GetIclI(),fInters[iCh][iInter].GetIclW(),fInters[iCh][iInter].GetIclE());
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedWIE(const Int_t iCh, const Int_t iClI, const Int_t iClW, const Int_t iClE)
{
  // Mark iClW-th wire cluster, iClI-th internal strip cluster and iClE-th external strip cluster of iCh-th chamber "used"
  SetUsedClW(iCh,iClW);
  SetUsedClI(iCh,iClI);
  SetUsedClE(iCh,iClE);
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedWIE(const Int_t iCh, const Int_t iInter)
{
  // Mark iClW-th wire cluster, iClI-th internal strip cluster and iClE-th external strip cluster of iCh-th chamber "used"
  SetUsedWIE(iCh,fInters[iCh][iInter].GetIclI(),fInters[iCh][iInter].GetIclW(),fInters[iCh][iInter].GetIclE());
}

//________________________________________________________________________________________
inline Bool_t TA2CylMwpc::IsUsedWIE(const Int_t iTrack) const
{
  // Check if wire, or external, or internal strip clusters, forming the iTrack-th track, is used
  for (Int_t i=0; i<fNchamber; ++i)
  {
    if (IsUsedWIE(i,fTracks[iTrack].GetIinter(i))) return kTRUE;
  }
  return kFALSE;
}

//________________________________________________________________________________________
inline void TA2CylMwpc::SetUsedWIE(const Int_t iTrack)
{
  // Mark as "used" the iClW-th wire clusters, iClI-th internal strip clusters and iClE-th external strip clusters, forming the iTrack-th track
  for (Int_t i=0; i<fNchamber; ++i) SetUsedWIE(i,fTracks[iTrack].GetIinter(i));
}

//________________________________________________________________________________________
inline void TA2CylMwpc::AddTrackTo(map<Double_t,Int_t> &mapTracks)
{
  // Add an index of the current track into the map
  
  // Adding
  mapTracks[fTracks[fNTrack].GetDirCos().Mag()] = fNTrack;
  
  // Histograms
  fPsVertex[0][fNTrack] = fTracks[fNTrack].GetPsVertex().X();
  fPsVertex[1][fNTrack] = fTracks[fNTrack].GetPsVertex().Y();
  fPsVertex[2][fNTrack] = fTracks[fNTrack].GetPsVertex().Z();
  fPsVertexR[fNTrack]   = fTracks[fNTrack].GetPsVertex().Perp();
  fTypeTrack[fNTrack]   = fTracks[fNTrack].GetType();
  fMagTrack[fNTrack]    = fTracks[fNTrack].GetDirCos().Mag();
  fEtrack[fNTrack]      = fInters[0][fTracks[fNTrack].GetIinter(0)].GetAclIE() + fInters[1][fTracks[fNTrack].GetIinter(1)].GetAclIE();
  fThetaTrack[fNTrack]  = fTracks[fNTrack].GetDirCos().Theta()*kRadToDeg;
  fPhiTrack[fNTrack]    = TVector2::Phi_0_2pi(fTracks[fNTrack].GetDirCos().Phi())*kRadToDeg;
  fDPhiInters[fNTrack]  = (fInters[0][fTracks[fNTrack].GetIinter(0)].GetPosition()->Phi() - fInters[1][fTracks[fNTrack].GetIinter(1)].GetPosition()->Phi())*kRadToDeg;
  fDZInters[fNTrack]    = fInters[0][fTracks[fNTrack].GetIinter(0)].GetPosition()->Z() - fInters[1][fTracks[fNTrack].GetIinter(1)].GetPosition()->Z();
}

//________________________________________________________________________________________
inline const TA2MwpcTrack *TA2CylMwpc::FindTrack(const Int_t *ii) const
{
  // Find a track build of the intersections ii[]
  
  const Int_t *iInters;
  for (Int_t i=0; i<fNTrack; ++i)
  {
    iInters = fTracks[i].GetIinters();
    if ( equal(iInters, iInters+fNchamber, ii) )
    {
      // Found
      return fTracks+i;
    }
  }
  
  // Not found
  return NULL;
}

//________________________________________________________________________________________
inline const TA2MwpcTrack *TA2CylMwpc::FindTrack(const Int_t i1, const Int_t i2) const
{
  // Find a track build of the intersections i1 and i2
  
  Int_t i[2] = {i1, i2};
  return FindTrack(i);
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::SmearZ(const Int_t iCh, const Double_t &phi, const Double_t &z) const
{
  // Return a smeared value of the Z coordinate for the iCh-th chamber
  
  Double_t dZ = 0.;
  if (fSigmaZ[iCh])
  {
    dZ = gRandom->Gaus(0.,fSigmaZ[iCh]->Interpolate(phi,z));
  }
  
  //
  return z + dZ;
}

//________________________________________________________________________________________
inline Double_t TA2CylMwpc::SmearPhi(const Int_t iCh, const Double_t &phi, const Double_t &z) const
{
  // Return a smeared value of the Phi coordinate for the iCh-th chamber
  
  Double_t dPhi = 0.;
  if (fSigmaPhi[iCh])
  {
    dPhi = gRandom->Gaus(0.,fSigmaPhi[iCh]->Interpolate(phi,z));
  }
  
  //
  return TVector2::Phi_0_2pi(phi + dPhi);
}

#endif
