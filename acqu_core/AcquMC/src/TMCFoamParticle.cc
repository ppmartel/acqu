//--Author	JRM Annand   13th Feb 2007  Adapt from TMCParicle	
//--Rev         JRM Annand
//--Update      JRM Annand   25th Jan 2009  Redo constructers

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCFoamParticle
//
// Generate 4-momenta according to "foam method"
// Root class TFoam
//   S. Jadach and P.Sawicki
//   Institute of Nuclear Physics, Cracow, Poland
//   Stanislaw. Jadach@ifj.edu.pl, Pawel.Sawicki@ifj.edu.pl
// Beam + Target -> Reaction -> Products
// 4 momenta generated accoring to N-way differential cross section
// Sampled (in N dimensions) by the TFoam class
// Input function returning value of sigma(X1,X2,...XN) i=1,N
// for a given set of Xi
// Store
// 1) Interaction vertex in target (x,y,z)
// 2) Beam 4-momentum
// 3) 4-momenta of reaction products
// Output as ROOT tree (decodable by macro MCReplay.C)
// OR as HBOOK ntuple, in identical format to mkin for input
// to CB/TAPS GEANT-3 model cbsim

#include "TMCFoamParticle.h"
#include "ARFile_t.h"
#include "TA2System.h"


//-----------------------------------------------------------------------------
TMCFoamParticle::TMCFoamParticle( const Char_t* name, TRandom* rand, 
				  TParticlePDG* pdg,
				  Int_t fnopt, Char_t* file, TA2System* sys )
  :TMCParticle( name, rand, pdg )
{
  fITheta = fICosTheta = fIPhi = fIEnergy = fIMomentum = -1;
}

//-----------------------------------------------------------------------------
TMCFoamParticle::TMCFoamParticle( const Char_t* name, TMCParticle* p,
				  Double_t* xscale, Double_t* scale,
				  Int_t* opt )
  :TMCParticle( name, p )
{
  // "Copy" constructor, copy TMCParticle properties
  // and then foam stuff
  // opt[0-4] are the indices of the foam vector for energy, cos(th), phi

  fXscale = xscale;
  fScale = scale;
  SetFoamLink( opt );
}

//---------------------------------------------------------------------------
TMCFoamParticle::~TMCFoamParticle()
{
  Flush();
}

//-----------------------------------------------------------------------------
void TMCFoamParticle::Flush( )
{
  // Free up allocated memory

  TMCParticle::Flush();
}

//-----------------------------------------------------------------------------
void TMCFoamParticle::SetFoamLink( Int_t* opt )
{
  // Linkage of arbitrary particle to foam
  // If non-zero iopt supplied assume
  // opt[] are the indices of the foam vector for kinematic variables
  // if opt[i] = -1 then that particular kinematic variable is not generated
  // by TFoam
  //

  // Momentum
  fIMomentum = opt[EIMomentum];      // momentum foam-vector index
  if( fIMomentum != -1 ){
    fTLow = fScale[2*fIMomentum];
    fTHigh = fScale[2*fIMomentum+1];
  }
  // Kinetic energy
  fIEnergy = opt[EIEnergy];          // energy foam-vector index
  if( fIEnergy != -1 ){
    fTLow = fScale[2*fIEnergy];
    fTHigh = fScale[2*fIEnergy+1];
  }
  // Asimuthal angle
  fIPhi = opt[EIPhi];                // phi foam-vector index
  if( fIPhi != -1 ){
    fPhiLow = fScale[2*fIPhi];
    fPhiHigh = fScale[2*fIPhi+1];
  }
  // Cosine polar angle
  fICosTheta = opt[EICosTheta];      // cos(theta) foam-vector index
  if( fICosTheta != -1 ){
    fCosThLow = fScale[2*fICosTheta];
    fCosThHigh = fScale[2*fICosTheta+1];
  }
  // Polar angle
  fITheta = opt[EITheta];            // theta foam-vector index
  if( fITheta != -1 ){
    fThetaLow = fScale[2*fITheta];
    fThetaHigh = fScale[2*fITheta+1];
  }
}

//-----------------------------------------------------------------------------
void TMCFoamParticle::Init( )
{
  // Set default initialise options
  // See TFoam reference at http://root.cern.ch
  // The vales here are the default ones which are used unless reset by
  // "Foam-Initialise:" command-line input
  // Work in progress!!!!
  TMCParticle::Init();
}

ClassImp(TMCFoamParticle)
