//--Author	C Collicott   14th Nov 2004   Most Basic Form
//
// PDG codes of particles generlly observed MAMI-C
// kElectron 11,     kPositron -11
// kMuonMinus 13     kMuonPlus -13      kGamma 22
// kPi0 111          kPiPlus 211        kPiMinus -211       kEta 221
// kProton 2212      kNeutron 2112
// 

#ifndef __TA2OnlinePhys_h__
#define __TA2OnlinePhys_h__

#include "TAcquRoot.h"
#include "TAcquFile.h"
#include "TA2Physics.h"
#include "TA2Analysis.h"
#include "TA2Tagger.h"
#include "TA2CrystalBall.h"
#include "TA2CentralApparatus.h"
#include "TA2Taps.h"
#include "TA2Ladder.h"
//#include "TA2PhotoPhysics.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include <iostream>

class TA2Tagger;

class TA2OnlinePhys : public TA2Physics {

	protected:

// Begin by initialising Detectors
	TA2Tagger*		fTAGG;	// Glasgow photon tagger
	TA2Ladder*		fLadder;	// Ladder
	TA2CrystalBall*	fCB;	// CrystalBall (PID, MWPCs, NaI)
	TA2Calorimeter*	fTAPS;  // TAPS

// OnlinePhys Class Variables

	// Particle Information

	UInt_t 			fCBMaxNParticle; 	// Max# Particle from CentApp
	UInt_t			fTAPSMaxNParticle; 	// Max# Particle from TAPS 
	UInt_t 			fMaxNParticle;		// Max # Particle (CB + TAPS)

	TA2Particle* 	fTAGGParticles;	   	// Particles from Tagger
    TA2Particle* 	fCBParticles;	   	// Particles from CB

	UInt_t 			fCBNParticle;    	// # Particle from CentApp
	UInt_t			fTAPSNParticle;    	// # Particle from TAPS
	UInt_t			fTAGGNParticle; 	// # Particle from Tagger
	UInt_t 			fNParticle;			// # of Particles (CB + TAPS)

	// Particle Arrays

	TA2Particle**   fTaggedPhoton;		// Array to hold Tagger photons

	UInt_t			fNTagg;
	Int_t*			fTaggerChannel;
	Double_t*		fTaggerTime;
	
	TLorentzVector* fParticleP4;
	Int_t*			fParticleApp;
    Double_t*		fParticleTime;

    Int_t  	 	fNHelicityBits;
    Bool_t  	fHelicity;
    Bool_t  	fHelicityInverted;
    Bool_t  	fHelicitySet;
    Int_t  	 	fHelicityADC;

    Char_t  	fHelicityBits[8][8];
    Bool_t  	fHelicityInhibit[8];
    Bool_t  	fHelicityBeam[8];

// Histograms
        TH1*    IM_2g_TAGG;
        TH1*    IM_2g_TAGGpr;
        TH1*    IM_2g_TAGGr;
	TH1* 	IM_2g;
	TH1*	IM_2g_CB;
	TH1*	IM_2g_TAPS;	
	TH1*	IM_2g_mix;

	TH1*	IM_3g;
	TH1*	IM_3g_CB;
	TH1*	IM_3g_E300;
	
	TH1*	IM_6g;
	TH1*	IM_6g_CB;

    TH1*    CB_Display_R;
    TH1*    CB_Display_G;
    TH1*    CB_Display_B;
    TH1*    CB_Display_T;

    TH3*	Pi0P_Hel0;
    TH3*	Pi0P_Hel1;
	
	protected:
		void ZeroCounters();
		void GetCBParticles();
		void GetTAPSParticles();
		void GetTagger();
		void DefineHistograms();
		void BasicPhysCheck();
        void CBDisplayCheck();
        void BeamHelicCheck();
		void FAsymPi0PCheck();

	public:

	TA2OnlinePhys( const char*, TA2Analysis* );
		virtual ~TA2OnlinePhys();
		virtual void LoadVariable();	// variables for display/cuts
		virtual void PostInit( );	// init after parameter input
		virtual void SetConfig(Char_t*, Int_t);
		virtual void Reconstruct();	// reconstruct detector info
		virtual TA2DataManager* CreateChild( const char*, Int_t ){ return NULL;}		

	ClassDef(TA2OnlinePhys,1)
};

#endif
