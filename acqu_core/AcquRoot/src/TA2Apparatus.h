//--Author	JRM Annand    4th Feb 2003
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...30th Sep 2003...Incorporate TA2DataManager
//--Rev 	JRM Annand...27th Sep 2004...Incorporate some PDG stuff
//--Rev 	JRM Annand...18th Oct 2004...Periodic procedures
//--Rev 	JRM Annand... 1st Dec 2004...DeleteArrays
//--Rev 	JRM Annand...14th Jan 2005...fTreeFileName
//--Rev 	JRM Annand...22nd Mar 2005...PostInit order, particle ID
//--Rev 	JRM Annand...15th Apr 2005...General-purpose particle ID cuts
//--Rev 	JRM Annand...20th Apr 2005   Add local 4-mom & PDG-code arrays
//--Rev 	JRM Annand...21st Jan 2007   4v0 update
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand...31st Jan 2007   Particle-ID cuts
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Update	JRM Annand...24th Nov 2008...add TA2Particle
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Apparatus
//
// Base case for apparatus classes...cannont be instantiated itself
// Specify set of procedures which user-defined derivatives must implement
// User-defined apparatus classes must inherit from this
//

#ifndef __TA2Apparatus_h__
#define __TA2Apparatus_h__

#include "TA2HistManager.h"
#include "TA2Detector.h"
#include "TList.h"
#include "TLorentzVector.h"
#include "TSystem.h"

#include "TA2ParticleID.h"             // PDG particle attributes

enum{ ECutSectorIndex, ECutPDGIndex }; // cut options

R__EXTERN TA2Analysis*  gAN;

class TA2Detector;
class TA2Particle;

class TA2Apparatus : public TA2HistManager {
private:
 protected:
  TLorentzVector* fP4;                  // 4 momenta of ID'ed particles
  TA2Particle* fParticleInfo;           // general particle info
  Int_t* fPDGtype;                      // considered particle IDs
  Int_t* fPDG_ID;                       // ID integers of particles
  TLorentzVector fP4tot;                // Sumed 4-momentum
  TA2ParticleID* fParticleID;           // local store particle attributed
  Double_t fMtot;                       // total invariant mass
  Double_t* fMinv;                      // individual invariant masses
  Double_t fEtot;                       // total energy
  Int_t fMaxParticle;                   // max # particles per event
  Int_t fNparticle;                     // # particles detected
  void* fROOToutput;                    // buffer for root tree writing
  TA2Cut**  fPCut;                      // array particle-ID cut pointers
  Int_t     fNCut;                      // total # p-ID cuts
  Int_t     fNSector;                   // # Sectors to apply p-ID cuts
  Int_t*    fPCutStart;                 // cut start index for each sector
  Int_t*    fNSectorCut;                // # cuts associated with each sector

public:
  TA2Apparatus( const char*, TA2System*, const Map_t*, const Map_t* = NULL );
  virtual ~TA2Apparatus();
  virtual void SetConfig( char*, int );   // setup decode in implement
  virtual void LoadVariable( );           // setup data filter conditions
  virtual void PostInit( );               // init after parameter input
  virtual TA2DataManager* CreateChild( const char*, Int_t ) = 0;
  virtual void Reconstruct() = 0;         // reconstruct detector info
  virtual void InitSaveTree();            // setup ROOT data output
  virtual void SaveEvent();               // save ROOT dats
  virtual void DeleteArrays();            // flush new'ed stuff
  virtual void SortParticleIDCuts();      // particle ID from misc cuts list

  TLorentzVector* GetP4(){ return fP4; }        // 4 momenta of ID'ed particles
  TA2Particle* GetParticleInfo(){ return fParticleInfo; }
  TLorentzVector GetP4tot(){ return fP4tot; }   // Sumed 4-momentum
  Int_t* GetPGDtype(){ return fPDGtype; }
  Int_t* GetPDG_ID(){ return fPDG_ID; }
  Double_t GetMtot(){ return fMtot; }           // total invariant mass
  Double_t* GetMinv(){ return fMinv; }          // individual invariant masses
  Double_t GetEtot(){ return fEtot; }           // total energy
  Int_t GetMaxParticle(){ return fMaxParticle; }// max # particles
  Int_t GetNParticle(){ return fNparticle; }    // # particles detected (allias)
  TA2Cut** GetPCut(){ return fPCut; }           // array particle-ID cut ptrs
  TA2Cut* GetPCut(Int_t i){ return fPCut[i]; }  // ith particle-ID cut
  Int_t GetNCut(){ return fNCut; }              // total # p-ID cuts
  Int_t GetNSector(){ return fNSector; }        // # Sectors to apply p-ID cuts
  Int_t* GetPCutStart(){ return fPCutStart; }   // cut start index each sect
  Int_t GetPCutStart(Int_t i){ return fPCutStart[i]; } // ith start index
  Int_t* GetNSectorCut(){ return fNSectorCut; } // # cuts for each sector
  Int_t GetNSectorCut(Int_t i){ return fNSectorCut[i]; } // ith # cuts
  // access wildcard cut options
  Double_t GetCutOpt(Int_t i, Int_t j){ return fPCut[i]->GetOpt(j); }
  Int_t GetCutSectorIndex( Int_t i )
  { return (Int_t)fPCut[i]->GetOpt(ECutSectorIndex); }
  Int_t GetCutPDGIndex( Int_t i )
  { return (Int_t)fPCut[i]->GetOpt(ECutPDGIndex); }
  
  ClassDef(TA2Apparatus,1)
};

//-----------------------------------------------------------------------------
inline void TA2Apparatus::SaveEvent( )
{
  // Transfer 4-vector info to output buffer
  // and write buffer to ROOT tree
  //
  if( !fIsSaveEvent ) return;
  Int_t* iptr = (Int_t*)fROOToutput;
  *iptr++ = fNparticle;
  Double_t* dptr = (Double_t*)((Int_t*)fROOToutput + fNparticle + 1 );
  for( Int_t i=0; i<fNparticle; i++ ){
    iptr[i] = fPDGtype[i];              // particle ID
    dptr[i] = fP4[i].Px();              // x-momentum
    dptr[i+fNparticle] = fP4[i].Py();   // y-momentum
    dptr[i+2*fNparticle] = fP4[i].Pz(); // z-momentum
    dptr[i+3*fNparticle] = fP4[i].E();  // energy
  }
  fTree->Fill();
}
#endif
