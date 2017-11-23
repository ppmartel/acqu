//--Author	D Glazier    XXth Jun 2007	
//--Rev         JRM Annand   20th Jul 2007  Adapt for system foam   
//--Rev         JRM Annand   25th Jun 2008  Remove brem-weight allusions   
//--Update      DI Glazier   12th Feb 2009  Add SaveEventAsParticles,rem fPSInt
//--Update      DI Glazier   30th Feb 2010  include GP4 to correct for boost rot.

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCPhotoPSGenerator
//
// Read E/theta dependence from data base file generated from SAID
// partial wave analysis of pseudo-scalar photoproduction data
// Calculate all 16 observables and use to generate lab 4-momenta

#include "TMCPhotoPSGenerator.h"
#include "MCNtuple.h"
#include "ARFile_t.h"
#include "TParticle.h"


// Recognised Said configure commands
enum { EMCPhotoPSMisc = 200 };
// Main command options
static const Map_t kMCSAIDCmd[] = {
  {"Misc:",                 EMCPhotoPSMisc},
  {NULL,             -1}
};

//-----------------------------------------------------------------------------
TMCPhotoPSGenerator::TMCPhotoPSGenerator( const Char_t* name, Int_t seed )
  :TMCFoamGenerator( name, seed  )
{
  AddCmdList( kMCSAIDCmd );
}

//---------------------------------------------------------------------------
TMCPhotoPSGenerator::~TMCPhotoPSGenerator()
{
  // Free up allocated memory
  TMCFoamGenerator::Flush();
}

void TMCPhotoPSGenerator::SaveEventAsParticles(  )
{
  // Save Event in TParticle format if this is enabled at configuration
  // if not Check if HBOOK ntuple output enabled
  // This one checks if the particle is a recoiling baryon...if so it
  // calculates the recoil polarisation and saves it in TParticle

  if( fIsNtupleOut ) SaveNtuple();
  if( !fIsPartOut ) return;
  if( fIsTreeOut ) return;

  TMCParticle* particle;
  TIter next( fParticle );
  TLorentzVector* P4;
  Double_t vx = fVertex->X();
  Double_t vy = fVertex->Y();
  Double_t vz = fVertex->Z();
  Int_t nTrack = 0;
  TParticle* part=(TParticle*)fClonePtcl->At(nTrack);
  part->SetMomentum(fP4Beam->X(),fP4Beam->Y(),fP4Beam->Z(),fP4Beam->E());
  //  part->SetProductionVertex(vx,vy,vz,0);
  part->SetProductionVertex(fBeamCentre->X(),fBeamCentre->Y(),
			    fBeamCentre->Z(),0);
  nTrack++;
  while( (particle = (TMCParticle*)next()) ){
    if( particle->IsTrack() ){
      P4 = particle->GetP4();
      part = (TParticle*)fClonePtcl->At(nTrack);
      part->SetMomentum(P4->X(), P4->Y(), P4->Z(), P4->E());
      part->SetProductionVertex(vx,vy,vz,0);
      // Check if the particle is a baryon whose parent is the
      // "reaction particle.....ie a recoiling baryon
      if( (particle->GetPDG()->ParticleClass() == TString("Baryon")) && 
	  (particle->GetParent()->GetName() == TString("Reaction")) ) {
	TMCPhotoPSInt* psint=static_cast<TMCPhotoPSInt*>(fInt);
 	psint->RecoilPol(-P4->Phi());//needs the meson phi angle in cm!
	// Save the polarisation angles in TParticle
	part->SetPolarisation(psint->GetRecPol());
	// Save the polarisation magnitude in the weight parameter
	part->SetWeight(psint->GetRecPol().Mag());
      }
      nTrack++;
    }
  }
  fTree->Fill();
}

ClassImp(TMCPhotoPSGenerator)
