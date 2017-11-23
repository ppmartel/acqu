//--Author	JRM Annand   19th Oct 2005  Skeleton user version
//--Rev
//--Update
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TMCUserParticle
//
// Skeleton User Particle specification and kinematics generation for
// the Monte Carlo event generator TMCGenerator.
// Users can redefine the behaviour of TMCParticle here

#include "TMCUserParticle.h"


//-----------------------------------------------------------------------------
TMCUserParticle::TMCUserParticle(const Char_t* name, TRandom* rand, Double_t mass, Bool_t decay, Bool_t track)
                :TMCParticle(name, rand, mass, decay, track)
{
  // No PDG data base supplied, read in particle mass
}

//-----------------------------------------------------------------------------
//TMCUserParticle::TMCUserParticle(const Char_t* name, TRandom* rand, TDatabasePDG* pdg, Int_t ipdg, Bool_t decay, Bool_t track)
//                :TMCParticle(name, rand, pdg, ipdg, decay, track)
//{
//  // Use PDG data base, read in PDG index
//}
//
//-----------------------------------------------------------------------------
TMCUserParticle::TMCUserParticle(const Char_t* name, TMCUserParticle* p)
                :TMCParticle(name, p)
{
  // "Copy" constructor, copy particle properties
  // and then execute "NULL" initialisation
}

//---------------------------------------------------------------------------
TMCUserParticle::~TMCUserParticle()
{
  // Free up allocated memory
  TMCParticle::Flush();
}

//-----------------------------------------------------------------------------
void TMCUserParticle::Init( )
{
  // Basic "NULL" initialisation...common to different constructors
  TMCParticle::Init();
}

//-----------------------------------------------------------------------------
void TMCUserParticle::PostInit( )
{
  // Construct parameter arrays for decay-particle for N-body phase-space
  // decay generator. Call PostInit for all decay particles
  TMCParticle::PostInit();
}
ClassImp(TMCUserParticle)
