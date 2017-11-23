//--Author	JRM Annand   24th May 2005	       
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3 
//--Rev 	JRM Annand...25th Jan 2009...redo constructer
//--Update	JRM Annand...27th Jan 2010...copy constructer option

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCResonance
//
// Hadronic resonance specification for Monte Carlo
// Inherits from TMCParticle and in addition has a Breit-Wigner
// distribution of mass.

#include "TMCResonance.h"
#include "TMCGenerator.h"
//#include "SysUtil.h"


//-----------------------------------------------------------------------------
TMCResonance::TMCResonance( Char_t* name, TMCParticle* p,
			    Double_t w, Double_t l, Double_t h )
  :TMCParticle( name, p )
{
  // Use TMCParticle copy constructer...with optional rename
  // w = width, l = low mass limit, h = high mass limit
  // if sampling mass from a Breit-Wigner distribution
  // If the supplied particle happens to be a resonance then just copy
  // its properties over and rename. Any supplied width etc. is ignored
  //
  if( p->IsResonance() ){
    *this = *(TMCResonance*)p;
    this->SetName(name);
  }
  else{
    if( w )
      fGamma = w;                  // width supplied
    else if( fPDG )
      fGamma = fPDG->Width();      // width from PDG database
    else fGamma = 0;               // no width info
    if( fGamma ){
      if( l ) fMLow = l;
      else fMLow = fMass - 5.0*fGamma;
      if( h ) fMHigh = h;
      else fMHigh = fMass + 5.0*fGamma;
      Double_t A[8];
      A[0] = fMass; A[1] = fGamma;
      SetDist( EMCParticleMDist, (Char_t*)"TMath::BreitWigner(x,%f,%f)", A,
	       500, fMLow, fMHigh );
    }
    fIsResonance = kTRUE;
  }
}

//---------------------------------------------------------------------------
TMCResonance::~TMCResonance()
{
  // Free up allocated memory
  if( fMassDist ) delete fMassDist;
}

//----------------------------------------------------------------------------
TH1D** TMCResonance::FindDist( Int_t type, Char_t* name,
			       Double_t min, Double_t max )
{
  // Add on mass distribution for resonance
  // in addition to standard particle distribution types

  TH1D** d = TMCParticle::FindDist( type, name, min, max );
  if( d ) return d;
  else if( type == EMCParticleMDist ){
    strcat( name, "_MDist" );
    if( min != max ){ fMLow = min; fMHigh = max; }
    return &fMassDist;
  }
  else return NULL;
}

ClassImp(TMCResonance)
