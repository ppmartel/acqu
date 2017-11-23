//--Author	JRM Annand   13th Feb 2007  Adapt from TMCParicle	
//--Rev         JRM Annand
//--Rev         JRM Annand   16th Apr 2008  Special 3-body decay
//--Rev         JRM Annand   30th Jun 2008  Bug fix pion asimuthal angle
//--Update      JRM Annand   25th Jan 2008  Redo constructers

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCdS5MDMParticle
//
// Generate 4-momenta according to "foam method"
// Specialist class to handle radiative PS-meson photoproduction.
// 5-fold differential cross section calculated according to
// Unitary Model or Effective-field-theory Model, tabulated and used
// to generate foam.

#include "TMCdS5MDMParticle.h"
#include "ARFile_t.h"
#include "TA2System.h"


//-----------------------------------------------------------------------------
TMCdS5MDMParticle::TMCdS5MDMParticle( const Char_t* name, TRandom* rand, 
				  TParticlePDG* pdg,
				  Int_t fnopt, Char_t* file, TA2System* sys )
  :TMCFoamParticle( name, rand, pdg, fnopt, file, sys )
{
  //
}

//-----------------------------------------------------------------------------
TMCdS5MDMParticle::TMCdS5MDMParticle( const Char_t* name, TMCParticle* p,
				  Double_t* xscale, Double_t* scale,
				  Int_t* opt )
  :TMCFoamParticle( name, p, xscale, scale, opt )
{
  // Anything special goes in here
}

//---------------------------------------------------------------------------
TMCdS5MDMParticle::~TMCdS5MDMParticle()
{
  Flush();
}

//-----------------------------------------------------------------------------
void TMCdS5MDMParticle::Flush( )
{
  // Free up allocated memory

  TMCFoamParticle::Flush();
}

//-----------------------------------------------------------------------------
void TMCdS5MDMParticle::Init( )
{
  // Set default initialise options
  // See TFoam reference at http://root.cern.ch
  // The vales here are the default ones which are used unless reset by
  // "Foam-Initialise:" command-line input
  // Work in progress!!!!
  TMCFoamParticle::Init();
}
//----------------------------------------------------------------------------
Bool_t TMCdS5MDMParticle::GenNDecay( ) 
{
  //  Generate an 3-Body final state.
  //  According to 5-fold differential cross section
  //  This is for radiative meson photoproduction where particle indices
  //  0 gamma'
  //  1 meson
  //  2 nucleon
  //  In principle should work for any 5-fold cross section differential in
  //  incident energy, energy & polar/asimuthal angle particle 0 (cm frame)
  //  and polar angle of particle 1 (cm frame). Given in terms of
  //  zero asimuthal angle of particle 1.

  Double_t M = fP4->M();                  // total mass
  //  Double_t E = fP4->E();                  // and energy
  TIter next( fDecayList );               // list of "decay" products
  TMCParticle* d0 = (TMCParticle*)next(); // 1st particle
  TMCParticle* d1 = (TMCParticle*)next(); // 2nd particle
  TMCParticle* d2 = (TMCParticle*)next(); // 3rd particle

  // Radiated gamma (or 1st decay particle)
  Double_t e0 = d0->Energy();
  Double_t c0 = d0->CosTheta();
  Double_t s0 = TMath::Sqrt(1.-c0*c0);
  Double_t ph0 = d0->Phi();
  Double_t m0 = d0->Mass();
  Double_t p0 = TMath::Sqrt(e0*e0 - m0*m0);    // magnitude momentum
  Double_t u0 = s0 * TMath::Cos(ph0);          // direction cosine u0
  Double_t v0 = s0 * TMath::Sin(ph0);          // direction cosine v0
  Double_t w0 = c0;                            // direction cosine w0
  // Photoproduced Meson (or 2nd decay particle)
  // Diff. cross section formulated in terms of pion produced in x-z plane
  Double_t c1 = d1->CosTheta();
  Double_t m1 = d1->Mass();
  Double_t u1 = TMath::Sqrt(1.- c1*c1);
  Double_t v1 = 0.0;
  Double_t w1 = c1;
  // Recoil Nucleon (or 3rd decay particle)
  Double_t m2 = d2->Mass();
  // Solve for p1, momentum magnitude of 2nd particle
  Double_t Ep = M - e0;                       // energy particles 1+2
  Double_t B = 2*p0*(u0*u1+w0*w1);
  Double_t C = p0*p0 - Ep*Ep - m1*m1 + m2*m2;
  Double_t a = B*B - 4*Ep*Ep;
  Double_t b = B*C;
  Double_t c = C*C - 4*Ep*Ep*m1*m1; 
  // Check which of 2 roots
  Double_t p1,p1a,p1b;
  p1a = (-b + TMath::Sqrt(b*b - a*c))/a;
  p1b = (-b - TMath::Sqrt(b*b - a*c))/a;
  if( p1a < 0 ){
    if( p1b < 0 ) return kFALSE;
    p1 = p1b;
  }
  else if( p1b < 0 ){
    p1 = p1a;
  }
  else{
    Double_t e1a = TMath::Sqrt(p1a*p1a + m1*m1);
    Double_t e1b = TMath::Sqrt(p1b*p1b + m1*m1);
    Double_t e2x = Ep - e1a;
    if( e2x < m2 ){
      e2x = Ep - e1b;
      if( e2x < m2 ){
	return kFALSE;
      }
      p1 = p1b;
    }
    else p1 = p1a;
  }    
  Double_t e1 = TMath::Sqrt(p1*p1 + m1*m1);
  // Save 4-momenta
  fP4d[0]->SetXYZT(p0*u0,p0*v0,p0*w0,e0);
  fP4d[1]->SetXYZT(p1*u1,p1*v1,p1*w1,e1);
  TLorentzVector CM(0.,0.,0.,M);
  *fP4d[2] = CM - *fP4d[0] - *fP4d[1];
  // Sample an asimuthal pion angle and rotate all 4-momenta
  // about the z-axis by this angle
  Double_t phi1 = d1->Phi();
  for( Int_t i=0; i<3; i++ ) fP4d[i]->RotateZ(phi1);

  // Boost CM -> Lab. Boost vector from 4-momentum of parent particle
  BoostLab( );
  return kTRUE;
}
ClassImp(TMCdS5MDMParticle)
