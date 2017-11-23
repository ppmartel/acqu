#ifndef __TA2CentralTrack_h__
#define __TA2CentralTrack_h__

// AcquRoot
#include "EnumConst.h"
#include "TA2TrackLine.h"

class TA2CentralTrack : public TA2TrackLine {
  protected:
    static const Int_t kNullHit = ENullHit;
    static const Double_t kNullFloat = ENullFloat; // TODO Think better about global consts
    Int_t    fIhitPid;		// Index of a hit in the PID
    Double_t fEhitPid;		// Pulse amplitude of the Pid hit
    Double_t fEhitPidCorr;	// Corrected pulse amplitude of the Pid hit
    Double_t fThitPid;		// Time of the Pid hit
    Double_t fPhiPid;		// Phi of the the PID hit
    Int_t    fIintersMwpc[2];	// Indexes of intersections in the internal (0) and external (1) MWPC
    Double_t fEtrackMwpc;	// Sum of MWPCs intersections pulse amplitudes
    Double_t fEtrackMwpcCorr;   // TODO
    Double_t fTtrackMwpc;	// TODO
    Double_t fEtrackMwpc0;	// MWPC 0 pulse amplitudes
    Double_t fEtrackMwpc1;	// MWPC 1 pulse amplitudes
    Int_t    fIclNaI;		// Index of a cluster in the NaI
    Double_t fEclNaI;		// Sum of crystals pulse amplitudes
    Double_t fEclNaICorr;	// Corrected sum of crystals pulse amplitudes
    Double_t fTclNaI;		// NaI cluster time
    Int_t    fMclNaI;		// Size of the NaI cluster
    Int_t    fCentralIndexNaI;	// Index of the central crystal in the cluster
    Double_t fThetaNaI;		// Phi of the the NaI claster
    Double_t fPhiNaI;		// Phi of the the NaI claster
    Double_t fSigmaTimeNaI;
    Double_t fSigmaEnergyNaI;
    Double_t fSigmaPhiNaI;
    Double_t fSigmaThetaNaI;
    void SetType();
  public:
    TA2CentralTrack();
    virtual ~TA2CentralTrack() {}
    virtual void Reset();
    void SetTrack(const Int_t, const Int_t, const Int_t, const Int_t, const TVector3&, const TVector3&, const TVector3&);
    void SetIhitPid(const Int_t);
    void SetEhitPid(const Double_t &e) { fEhitPid = e; }
    void SetEhitPidCorr(const Double_t &e) { fEhitPidCorr = e; }
    void SetThitPid(const Double_t &t) { fThitPid = t; }
    void SetPhiPid(const Double_t &phi) { fPhiPid = phi; }
    void SetIinterMwpc(const Int_t, const Int_t);
    void SetIintersMwpc(const Int_t, const Int_t);
    void SetEtrackMwpc(const Double_t &e) { fEtrackMwpc = e; }
    void SetEtrackMwpcCorr(const Double_t &e) { fEtrackMwpcCorr = e; }
    void SetEtrackMwpc0(const Double_t &e) { fEtrackMwpc0 = e; }
    void SetEtrackMwpc1(const Double_t &e) { fEtrackMwpc1 = e; }
    void SetTtrackMwpc(const Double_t &t) { fTtrackMwpc = t; }
    void SetIclNaI(const Int_t);
    void SetEclNaI(const Double_t &e) { fEclNaI = e; }
    void SetEclNaICorr(const Double_t &e) { fEclNaICorr = e; }
    void SetTclNaI(const Double_t &t) { fTclNaI = t; }
    void SetMclNaI(const Int_t &m) { fMclNaI = m; }
    void SetCentralIndexNaI(const Int_t &i) { fCentralIndexNaI = i; }
    void SetThetaNaI(const Double_t &theta) { fThetaNaI = theta; }
    void SetPhiNaI(const Double_t &phi) { fPhiNaI = phi; }
    void SetSigmaTimeNaI(const Double_t &t) { fSigmaTimeNaI = t; }
    void SetSigmaEnergyNaI(const Double_t &e) { fSigmaEnergyNaI = e; }
    void SetSigmaPhiNaI(const Double_t &phi) { fSigmaPhiNaI = phi; }
    void SetSigmaThetaNaI(const Double_t &tet) { fSigmaThetaNaI = tet; }
    static Int_t CalcTrackType(const Bool_t, const Bool_t, const Bool_t, const Bool_t);
    Int_t    GetIhitPid() const { return fIhitPid; }
    Double_t GetEhitPid() const { return fEhitPid; }
    Double_t GetEhitPidCorr() const { return fEhitPidCorr; }
    Double_t GetThitPid() const { return fThitPid; }
    Double_t GetPhiPid() const { return fPhiPid; }
    Int_t    GetIinterMwpc(const Int_t iCh) const { return fIintersMwpc[iCh]; }
    const Int_t *GetIintersMwpc() const { return fIintersMwpc; }
    Double_t GetEtrackMwpc() const { return fEtrackMwpc; }
    Double_t GetEtrackMwpcCorr() const { return fEtrackMwpcCorr; }
    Double_t GetEtrackMwpc0() const { return fEtrackMwpc0; }
    Double_t GetEtrackMwpc1() const { return fEtrackMwpc1; }
    Double_t GetTtrackMwpc() const { return fTtrackMwpc; }
    Int_t    GetIclNaI() const { return fIclNaI; }
    Double_t GetEclNaI() const { return fEclNaI; }
    Double_t GetEclNaICorr() const { return fEclNaICorr; }
    Double_t GetTclNaI() const { return fTclNaI; }
    Int_t    GetMclNaI() const { return fMclNaI; }
    Int_t    GetCentralIndexNaI() const { return fCentralIndexNaI; }
    Double_t GetThetaNaI() const { return fThetaNaI; }
    Double_t GetPhiNaI() const { return fPhiNaI; }
    Double_t GetSigmaTimeNaI() const { return fSigmaTimeNaI; }
    Double_t GetSigmaEnergyNaI() const { return fSigmaEnergyNaI; }
    Double_t GetSigmaPhiNaI() const { return fSigmaPhiNaI; }
    Double_t GetSigmaThetaNaI() const { return fSigmaThetaNaI; }
    Bool_t HasPid() const { return fIhitPid != kNullHit; }
    Bool_t HasMwpc(const Int_t iCh) const { return fIintersMwpc[iCh] != kNullHit; }
    Bool_t HasMwpc() const { return HasMwpc(0) || HasMwpc(1); }
    Int_t  HasSingleMwpc() const;
    Bool_t HasNaI() const { return fIclNaI != kNullHit; }
    Bool_t IsNeutral() const;
    
    ClassDef(TA2CentralTrack,1) // A2CB central track (PID-MWPC-NaI)
};

//_________________________________________________________________________________________
inline void TA2CentralTrack::Reset()
{
  // Set default track info
  
  // Reset inherited data-members
  TA2TrackLine::Reset();
  
  //
  fIhitPid = kNullHit;
  fEhitPid = kNullFloat;
  fThitPid = kNullFloat;
  fIintersMwpc[0] = kNullHit;
  fIintersMwpc[1] = kNullHit;
  fEtrackMwpc = kNullFloat;
  fEtrackMwpc0 = kNullFloat;  
  fEtrackMwpc1 = kNullFloat;    
  fTtrackMwpc = kNullFloat;
  fIclNaI = kNullHit;
  fEclNaI = kNullFloat;
  fTclNaI = kNullFloat;
  fMclNaI = kNullHit;
  fCentralIndexNaI = kNullHit;
  fSigmaTimeNaI   = kNullFloat;
  fSigmaEnergyNaI = kNullFloat;
  fSigmaPhiNaI    = kNullFloat;
  fSigmaThetaNaI  = kNullFloat;
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetTrack(const Int_t iHitPid, const Int_t iInterMwpc0, const Int_t iInterMwpc1, const Int_t iClNaI, const TVector3 &orig, const TVector3 &dircos, const TVector3 &psvertex)
{
  // Set all data members of the track
  
  // Set hit index of PID
  SetIhitPid(iHitPid);
  
  // Set indexes of MWPC intersections
  SetIintersMwpc(iInterMwpc0,iInterMwpc1);
  
  // Set NaI claster index
  SetIclNaI(iClNaI);
  
  // Set track line
  SetOrigin(orig);
  SetDirCos(dircos);
  SetPsVertex(psvertex);
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetIhitPid(const Int_t i)
{
  fIhitPid = i;
  SetType();
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetIintersMwpc(const Int_t i0, const Int_t i1)
{
  fIintersMwpc[0] = i0;
  fIintersMwpc[1] = i1;
  SetType();
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetIinterMwpc(const Int_t iCh, const Int_t i)
{
  fIintersMwpc[iCh] = i;
  SetType();
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetIclNaI(const Int_t i)
{
  fIclNaI = i;
  SetType();
}

//_________________________________________________________________________________________
inline Int_t TA2CentralTrack::CalcTrackType(const Bool_t hasNaI, const Bool_t hasMwpc1, const Bool_t hasMwpc0, const Bool_t hasPid)
{
  // Calculate track type
  // bit = 0(no), = 1(yes)
  // bit#	detector	param
  // 0		PID		hasPid
  // 1		MWPC0		hasMwpc0
  // 2		MWPC1		hasMwpc1
  // 3		NaI		hasNaI
  
  return hasNaI << 3 | hasMwpc1 << 2 | hasMwpc0 << 1 | hasPid;
}

//_________________________________________________________________________________________
inline void TA2CentralTrack::SetType()
{
  // Set track type based on the current state
  TA2TrackLine::SetType(CalcTrackType(HasNaI(),HasMwpc(1),HasMwpc(0),HasPid()));
}

//_________________________________________________________________________________________
inline Int_t TA2CentralTrack::HasSingleMwpc() const
{
  // Check if the track is based on a single MWPC
  // Return iCh+1 if the track contains only MWPC # iCh. Otherwise, return 0
  
  // Check if the track contains 2 MWPCs or does not contain any MWPC
  if ( HasMwpc(0) == HasMwpc(1) ) return 0;
  
  // Check if the track has MWPC0 otherwise return 2 (means MWPC1)
  if (HasMwpc(0))
    return 1;
  else
    return 2;
}

//_________________________________________________________________________________________
inline Bool_t TA2CentralTrack::IsNeutral() const
{
  return HasNaI() && !HasMwpc() && !HasPid();
}

#endif
