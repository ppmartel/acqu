//--Author	Cristina Collicott   June 2013   Compton and Pi0 analysis

#include "TA2OnlinePhys.h"

enum {EG_HELICITY};
static const Map_t kInputs[] = {
    {"Beam-Helicity:", EG_HELICITY},
    {NULL,          -1}
};

ClassImp(TA2OnlinePhys)

//-----------------------------------------------------------------------------
TA2OnlinePhys::TA2OnlinePhys( const char* name, TA2Analysis* analysis )
	:TA2Physics( name, analysis ) 
{
// Initialise Detectors
	fTAGG		= NULL; // Tagger
	fLadder		= NULL; // Tagger ladder
	fCB			= NULL; // CB (PID, MWPC, NaI)
	fTAPS		= NULL; // TAPS

	// Particle arrays
	fTaggedPhoton	= NULL;
    fCBParticles    = NULL;

	fNTagg			= 0;
	fTaggerChannel	= NULL;
	fTaggerTime		= NULL;
	
	fNParticle		= 0;
	fParticleP4		= NULL;
	fParticleApp	= NULL;

	fNHelicityBits = 0;
	fHelicity = 0;
	fHelicityInverted = 0;
	fHelicitySet = 0;
	fHelicityADC = 0;

	AddCmdList(kInputs);
}


//-----------------------------------------------------------------------------
TA2OnlinePhys::~TA2OnlinePhys()
{
}
	
//-----------------------------------------------------------------------------
void TA2OnlinePhys::SetConfig(Char_t* line, Int_t key)
{
	// Any special command-line input for Crystal Ball apparatus

	switch (key){
    case EG_HELICITY:
        fNHelicityBits = sscanf(line, "%i%s%s%s%s%s%s%s%s", &fHelicityADC, fHelicityBits[0], fHelicityBits[1], fHelicityBits[2], fHelicityBits[3], fHelicityBits[4], fHelicityBits[5], fHelicityBits[6], fHelicityBits[7]);
        fNHelicityBits--;
        if(fNHelicityBits < 2) Error("SetConfig", "Not enough information to construct beam helicity!");
        else
        {
            //printf("Helicity");
            for(Int_t i=0; i<fNHelicityBits; i++)
            {
                fHelicityInhibit[i] = false;
                if(!strcmp(fHelicityBits[i],"I") || !strcmp(fHelicityBits[i],"i")) fHelicityInhibit[i] = true;
                else if(!strcmp(fHelicityBits[i],"L") || !strcmp(fHelicityBits[i],"l")) fHelicityBeam[i] = false;
                else if(!strcmp(fHelicityBits[i],"H") || !strcmp(fHelicityBits[i],"h")) fHelicityBeam[i] = true;
                //printf(" - %s %i %i",fHelicityBits[i], fHelicityInhibit[i], fHelicityBeam[i]);
            }
            //printf("\n");
        }
        break;
    default:
        // default main apparatus SetConfig()
        TA2Physics::SetConfig( line, key );
        break;
    }
}

//---------------------------------------------------------------------------
void TA2OnlinePhys::PostInit()
{

// Introduce Detectors

	// Tagger
	fTAGG = (TA2Tagger*)((TA2Analysis*)fParent)->GetChild("TAGG");
	if ( !fTAGG) PrintError("","<No Tagger class found>",EErrFatal);
	else {  printf("Tagger included in analysis\n");
		fTAGGParticles = fTAGG->GetParticles(); }

	// Ladder
	fLadder = (TA2Ladder*)((TA2Analysis*)fParent)->GetGrandChild( "FPD");
	if (!fLadder) PrintError( "", "<No Ladder class found>", EErrFatal);

	// Central Apparatus
	fCB = (TA2CrystalBall*)((TA2Analysis*)fParent)->GetChild("CB");	
	if (!fCB) PrintError( "", "<No CrystalBall class found>", EErrFatal);
	else {  printf("CB system included in analysis\n"); }

	// TAPS
	fTAPS = (TA2Calorimeter*)((TA2Analysis*)fParent)->GetChild("TAPS");
	if ( !fTAPS) printf("TAPS *NOT* included in analysis\n");
	else {  printf("TAPS included in analysis\n"); }

	printf("\n");

// Get max # of Particles from detectors, used for defining array sizes

	fCBMaxNParticle = fCB->GetMaxParticle();	
	if (fTAPS)  	fTAPSMaxNParticle = fTAPS->GetMaxParticle(); 
	else 			fTAPSMaxNParticle = 0;
	fMaxNParticle = fCBMaxNParticle + fTAPSMaxNParticle;  

	// Create arrays to hold Particles
    fCBParticles    = new TA2Particle[fCBMaxNParticle];
	fParticleP4		= new TLorentzVector[fMaxNParticle];
	fParticleApp	= new Int_t[fMaxNParticle];
    fParticleTime	= new Double_t[fMaxNParticle];

	// Make 2x the room (enough) for tagger for multi-hit

	fTaggedPhoton	= new TA2Particle*[352*2];
	fTaggerTime		= new Double_t[352*2];
	fTaggerChannel	= new Int_t[352*2];

	// Define hard coded histograms
	DefineHistograms();

	// Default physics initialisation
	TA2Physics::PostInit();
}

//-----------------------------------------------------------------------------
void TA2OnlinePhys::LoadVariable( )
{

// Input name - variable pointer associations for any subsequent cut/histogram setup

	TA2Physics::LoadVariable();

	return;
}

//-----------------------------------------------------------------------------
void TA2OnlinePhys::Reconstruct() 
{
	
	ZeroCounters();
	
	GetCBParticles();
	GetTAPSParticles();
	GetTagger();
	
	BasicPhysCheck();
    CBDisplayCheck();
    if(fNHelicityBits>=2) BeamHelicCheck();
	FAsymPi0PCheck();

}

void TA2OnlinePhys::DefineHistograms()
{
	// Hard coded histograms
	gROOT->cd();
	
	IM_2g 		= new TH1D("PHYS_IM_2g", 		"IM of 2 photon events", 1000, 0, 1000);
	IM_2g_CB 	= new TH1D("PHYS_IM_2g_CB", 	"IM of 2 photon events - CB only", 1000, 0, 1000);
	IM_2g_TAPS 	= new TH1D("PHYS_IM_2g_TAPS", 	"IM of 2 photon events - TAPS only", 1000, 0, 1000);
	IM_2g_mix 	= new TH1D("PHYS_IM_2g_mix", 	"IM of 2 photon events - CB & TAPS", 1000, 0, 1000);

	IM_3g 		= new TH1D("PHYS_IM_3g", 		"IM of 3 photon events", 1000, 0, 1000);
	IM_3g_CB 	= new TH1D("PHYS_IM_3g_CB", 	"IM of 3 photon events - CB only", 1000, 0, 1000);
	IM_3g_E300	= new TH1D("PHYS_IM_3g_E300", 	"IM of 3 photon events - all E > 300 MeV", 1000, 0, 1000);
	
	IM_6g 		= new TH1D("PHYS_IM_6g", 		"IM of 6 photon events", 1000, 0, 1000);
	IM_6g_CB 	= new TH1D("PHYS_IM_6g_CB", 	"IM of 6 photon events - CB only", 1000, 0, 1000);

    CB_Display_R = new TH1D("PHYS_CB_Display_R",	"CB Hit Pattern for Display - Red", 720, 0, 719);
    CB_Display_G = new TH1D("PHYS_CB_Display_G",	"CB Hit Pattern for Display - Green", 720, 0, 719);
    CB_Display_B = new TH1D("PHYS_CB_Display_B",	"CB Hit Pattern for Display - Blue", 720, 0, 719);
    CB_Display_T = new TH1D("PHYS_CB_Display_T",	"CB Hit Pattern for Display - Time", 720, 0, 719);

    Pi0P_Hel0	= new TH3D("PHYS_Pi0P_Hel0",	"Pi0P Events;#theta_{#pi0} (deg);t_{#pi0} (ns);m_{miss} (MeV)", 36, 0, 180, 200, -200, 200, 150, 800, 1100);
    Pi0P_Hel1	= new TH3D("PHYS_Pi0P_Hel1",	"Pi0P Events;#theta_{#pi0} (deg);t_{#pi0} (ns);m_{miss} (MeV)", 36, 0, 180, 200, -200, 200, 150, 800, 1100);

		
}

void TA2OnlinePhys::FAsymPi0PCheck()
{
    TLorentzVector p4Beam(0,0,0,0);
    TLorentzVector p4Target(0,0,0,938.27);
    TLorentzVector p4Pi0(0,0,0,0);
    TLorentzVector p4Miss(0,0,0,0);

    Double_t timePi0 = 0;

    // Possible pi0 candidate with two tracks (lost recoil)
    if(fNParticle == 2)
    {
        p4Pi0 = fParticleP4[0] + fParticleP4[1];
        timePi0 = 0.5*(fParticleTime[0] + fParticleTime[1]);
    }
    // Possible pi0 candidate with three tracks (detect recoil)
    else if(fNParticle == 3)
    {
        TLorentzVector p4_1 = fParticleP4[0] + fParticleP4[1];
        TLorentzVector p4_2 = fParticleP4[0] + fParticleP4[2];
        TLorentzVector p4_3 = fParticleP4[1] + fParticleP4[2];

        Double_t md_1 = TMath::Abs(p4_1.M()-134.98);
        Double_t md_2 = TMath::Abs(p4_2.M()-134.98);
        Double_t md_3 = TMath::Abs(p4_3.M()-134.98);

        if(md_1 < md_2 && md_1 < md_3)
        {
            p4Pi0 = p4_1;
            timePi0 = 0.5*(fParticleTime[0] + fParticleTime[1]);
        }
        else if(md_2 < md_1 && md_2 < md_3)
        {
            p4Pi0 = p4_2;
            timePi0 = 0.5*(fParticleTime[0] + fParticleTime[2]);
        }
        else if(md_3 < md_1 && md_3 < md_2)
        {
            p4Pi0 = p4_3;
            timePi0 = 0.5*(fParticleTime[1] + fParticleTime[2]);
        }
    }
    // Reasonable pi0 candidate, and helicity bit is set correctly
    if(TMath::Abs(p4Pi0.M()-134.98) < 20 && fHelicitySet)
    {
        for(UInt_t i=0; i<fNTagg; i++)
        {
            p4Beam = fTaggedPhoton[i]->GetP4();
            if(p4Beam.E()<145 || p4Beam.E()>=300) continue;
            if((TMath::Abs(p4Pi0.Phi()*TMath::RadToDeg())<45) || (TMath::Abs(p4Pi0.Phi()*TMath::RadToDeg())>=135)) continue;
	    p4Miss = p4Beam+p4Target-p4Pi0;
	    if((fHelicity && (p4Pi0.Phi()>0)) || (!fHelicity && (p4Pi0.Phi()<0))) Pi0P_Hel1->Fill((p4Pi0.Theta()*TMath::RadToDeg()),(timePi0-fTaggerTime[i]),p4Miss.M());
            else Pi0P_Hel0->Fill((p4Pi0.Theta()*TMath::RadToDeg()),(timePi0-fTaggerTime[i]),p4Miss.M());
        }
    }
}


void TA2OnlinePhys::BeamHelicCheck()
{
    Bool_t fHelicityBit;
    fHelicity = true;
    fHelicityInverted = true;
    fHelicitySet = true;
    for(Int_t i=0; i<fNHelicityBits; i++)
    {
        fHelicityBit = (fADC[fHelicityADC] & 1<<i);
        if(fHelicityInhibit[i] && fHelicityBit)
        {
            fHelicitySet = false;
            break;
        }
        else if(fHelicityInhibit[i]) continue;
        fHelicity = (fHelicity && (fHelicityBeam[i] == fHelicityBit));
        fHelicityInverted = (fHelicityInverted && (fHelicityBeam[i] != fHelicityBit));
        if(fHelicity == fHelicityInverted)
        {
            fHelicitySet = false;
            break;
        }
    }
}


void TA2OnlinePhys::BasicPhysCheck()
{
	// 2 Gamma invariant mass	
	if(fNParticle == 2)
	{
		TLorentzVector p4 = fParticleP4[0] + fParticleP4[1]; 
		
		IM_2g->Fill(p4.M());
		
		if((fParticleApp[0] == 1) && (fParticleApp[1] == 1)) 
		IM_2g_CB->Fill(p4.M());
		
		else if((fParticleApp[0] == 2) && (fParticleApp[1] == 2)) 
		IM_2g_TAPS->Fill(p4.M());
		
		else 
		IM_2g_mix->Fill(p4.M());
	}

	// 3g case
	if(fNParticle == 3)
	{
		TLorentzVector p4 = fParticleP4[0] 
						  + fParticleP4[1]  
						  +	fParticleP4[2];
		
		IM_3g->Fill(p4.M());
		
		if((fParticleApp[0] == 1) 
		&& (fParticleApp[1] == 1) 
		&& (fParticleApp[2] == 1) )
		IM_3g_CB->Fill(p4.M());
		
		if((fParticleP4[0].T() >= 300.0) 
		&& (fParticleP4[1].T() >= 300.0) 
		&& (fParticleP4[2].T() >= 300.0) )
		IM_3g_E300->Fill(p4.M());		
		
	}		
	
	// 6g case
	if(fNParticle == 6)
	{
		TLorentzVector p4 = fParticleP4[0] + fParticleP4[1] 
						  +	fParticleP4[2] + fParticleP4[3]
						  +	fParticleP4[4] + fParticleP4[5];
		
		IM_6g->Fill(p4.M());
		
		if((fParticleApp[0] == 1) && (fParticleApp[1] == 1) 
		&& (fParticleApp[2] == 1) && (fParticleApp[3] == 1)
		&& (fParticleApp[4] == 1) && (fParticleApp[5] == 1)	)
		IM_6g_CB->Fill(p4.M());
		
	}	
}

void TA2OnlinePhys::ZeroCounters()
{
	fNParticle = 0;
}

void TA2OnlinePhys::GetCBParticles()
{
	fCBNParticle = fCB->GetNParticle();
	for (UInt_t i = 0; i < fCBNParticle; i++ ) {
        fCBParticles[i] = fCB->GetParticles(i);

		 TLorentzVector* p4cb = fCB->GetP4();
		 
		fParticleP4[fNParticle+i]  = p4cb[i];
		fParticleApp[fNParticle+i] = 1;
        fParticleTime[fNParticle+i] = fCB->GetParticles(i).GetTime();
	}
	
	// update # of particles
	fNParticle+=fCBNParticle;
}

void TA2OnlinePhys::CBDisplayCheck()
{
    // For CB Display purposes
    //cout << "New Event" << endl;

    CB_Display_R->Reset();
    CB_Display_G->Reset();
    CB_Display_B->Reset();
    CB_Display_T->Reset();

    HitCluster_t *cluster;
    UInt_t *hits;
    for (UInt_t i = 0; i < fCBNParticle; i++ ) {
        //cout << "\tParticle " << i << "\t" << fCBParticles[i].GetCentralIndex() << "\t" << fCBParticles[i].GetVetoEnergy() << "\t" << fCBParticles[i].GetVetoIndex() << endl;
        cluster = fCB->GetNaI()->GetCluster(fCBParticles[i].GetCentralIndex());
        hits = cluster->GetHits();
        for (UInt_t j = 0; j < (cluster->GetNhits()); j++ ) {
            //cout << "\t\t" << hits[j] << "\t" << fCB->GetNaI()->GetTime(hits[j]) << endl;
            if((fCBParticles[i].GetVetoIndex() > 0) && (fCBParticles[i].GetVetoEnergy() > 1.5)) CB_Display_R->SetBinContent(hits[j]+1,255);
            else if((fCBParticles[i].GetVetoIndex() > 0) && (fCBParticles[i].GetVetoEnergy() > 0)) CB_Display_G->SetBinContent(hits[j]+1,255);
            else CB_Display_B->SetBinContent(hits[j]+1,255);
            CB_Display_T->SetBinContent(hits[j]+1,fCB->GetNaI()->GetTime(hits[j]));
        }
    }
}

void TA2OnlinePhys::GetTAPSParticles()
{
	fTAPSNParticle = fTAPS->GetNParticle();
	for (UInt_t i = 0; i < fTAPSNParticle; i++ ) {
		
		 TLorentzVector* p4taps = fTAPS->GetP4();
		 
		fParticleP4[fNParticle+i]  = p4taps[i];
		fParticleApp[fNParticle+i] = 2;
        fParticleTime[fNParticle+i] = (fTAPS->GetCal()->GetClTimeOR())[i];
    }
	
	// update # of particles
	fNParticle+=fTAPSNParticle;	
}

void TA2OnlinePhys::GetTagger()
{
	// Tagger
	if(fTAGG && fLadder)
	{
		// Collect Tagger M0 Hits
		fNTagg	= fLadder->GetNhits();
		for(UInt_t i=0; i<fNTagg; i++)
		{
			fTaggerChannel[i]	= fLadder->GetHits(i);
			fTaggerTime[i]		= (fLadder->GetTimeOR())[i];
			fTaggedPhoton[i] 	= fTAGGParticles+i;
		}
	
		// Collect Tagger M+ Hits
		for(UInt_t m=1; m<fLadder->GetNMultihit(); m++)
		{
			for(UInt_t i=0; i<fLadder->GetNhitsM(m); i++)
			{
				fTaggerChannel[fNTagg+i] 	= (fLadder->GetHitsM(m))[i];
				fTaggerTime[fNTagg+i]	 	= (fLadder->GetTimeORM(m))[i];
			}
			fNTagg	+= fLadder->GetNhitsM(m);
		}
	}
	else fNTagg = 0;	
	
}
