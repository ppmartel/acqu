//--Author	PP Martel    10th Oct 2010
//--Update	PP Martel    19th Feb 2011
//
// TA2PhysicsPMartel
//
// Reconstruction of Compton and Pi0 kinematics. 

#include "TA2PhysicsPMartel.h"

// Valid Keywords for command-line setup of Compton
enum { EPhotoMassLimits = 1000, EPromRandWindows, ESaveTree,
       ERecordRaw, ECherenkov, ECutNaIPID, ECutNaIMWPC, ECutPIDMWPC};
static const Map_t kPhotoKeys[] = {
  {"Mass-Limits:",			EPhotoMassLimits},
  {"SaveTree:", 	                ESaveTree},
  {"RecordRaw:",	                ERecordRaw},
  {"Cherenkov:",	                ECherenkov},
  {"CutNaIPID:",                        ECutNaIPID},
  {"CutNaIMWPC:",                       ECutNaIMWPC},
  {"CutPIDMWPC:",                       ECutPIDMWPC},
  {NULL,                                  -1}
};

ClassImp(TA2PhysicsPMartel)

//-----------------------------------------------------------------------------
TA2PhysicsPMartel::TA2PhysicsPMartel( const char* name, TA2Analysis* analysis ):TA2AccessSQL( name, analysis ) {
  // Initialise Compton variables here
  // Default null pointers, zeroed variables

  fTreeSave = false;

  fIsCutNaIPID = false;
  fCanCutNaIPID = false;
  fIsCutNaIMWPC = false;
  fCanCutNaIMWPC = false;
  fIsCutPIDMWPC = false;
  fCanCutPIDMWPC = false;

  fTableCB = false;
  fTblTAPS = false;

  fCB = NULL;
  fNaI = NULL;
  fPID = NULL;
  fMWPC = NULL;
  fTAGG = NULL;
  fLADD = NULL;
  fTAPS = NULL;
  fBaF2 = NULL;
  fVeto = NULL;
  
  fPARTtagg = NULL; 
  fPARTneut = NULL;
  fPARTchar = NULL;
  
  fPARTpi0 = NULL;
  fPARTeta = NULL;
  fPARTgprime = NULL;

  fTgRefTDC = ENullHit;
  fCBRefTDC = ENullHit;
  fSynchDif = ENullHit;

  fCherADC = ENullHit;
  fCherTDC0 = ENullHit;
  fCherTDC1 = ENullHit;
  fCherTDC2 = ENullHit;

  fBeamChk = 0;
  fBeamPol = 0;
  fCBESum = 0.0;
  fDumpN = 0;
  fEvnN = 0;

  fScalAcc = NULL;
  fScalCur = NULL;

  fNneut = 0;
  fNchar = 0;
  fNtagg = 0;

  fMaxTagg = 0;
  fNmultS = 0;
  fNmult = 0;

  fNpi0 = 0;
  fNeta = 0;
  fNgprime = 0;
  
  fMassDpi0 = NULL;
  fMassDeta = NULL;
  fMassIJ = NULL;
  fMassIpi0 = NULL;
  fMassIeta = NULL;
  fIsMesonIndex = NULL;
  
  fMaxMDpi0 = 0.0;
  fMaxMDeta = 0.0;
  
  fM2g = ENullHit;
  fM6g = ENullHit;

  fDetNaI = NULL;
  fDetPID = NULL;
  fDetMWPC = NULL;

  fDetNaITot = 0;
  fDetPIDTot = 0;
  fDetMWPCTot = 0;

  fENaI = NULL;
  fEPID = NULL;
  fEMWPC = NULL;

  fCheckNaIPID = NULL;
  fCheckNaIMWPC = NULL;
  fCheckPIDMWPC = NULL;
  fCheckCharged = NULL;

  fCheckNaIPIDTot = 0;
  fCheckNaIMWPCTot = 0;
  fCheckPIDMWPCTot = 0;
  fCheckChargedTot = 0;

  fEvnID = -1;
  fEvnIDList = NULL;

  fRecordRaw = 0;
  fCherenkov = 0;

  fNADCsNaI = 0;
  fADCsNaI = NULL;
  fADCsRawNaI = NULL;
  fADCsHitNaI = NULL;

  fNTDCsNaI = 0;
  fTDCsNaI = NULL;
  fTDCsRawNaI = NULL;
  fTDCsHitNaI = NULL;

  fNHitsNaI = 0;
  fHitsNaI = NULL;

  fNADCsPID = 0;
  fADCsPID = NULL;
  fADCsRawPID = NULL;
  fADCsHitPID = NULL;

  fNTDCsPID = 0;
  fTDCsPID = NULL;
  fTDCsRawPID = NULL;
  fTDCsHitPID = NULL;

  fNHitsPID = 0;
  fHitsPID = NULL;

  fNHitsMWPC = 0;
  fHitsMWPC = NULL;

  fNADCsBaF2 = 0;
  fADCsBaF2 = NULL;
  fADCsRawBaF2 = NULL;
  fADCsHitBaF2 = NULL;

  fNTDCsBaF2 = 0;
  fTDCsBaF2 = NULL;
  fTDCsRawBaF2 = NULL;
  fTDCsHitBaF2 = NULL;

  fNHitsBaF2 = 0;
  fHitsBaF2 = NULL;

  fNADCsVeto = 0;
  fADCsVeto = NULL;
  fADCsRawVeto = NULL;
  fADCsHitVeto = NULL;

  fNTDCsVeto = 0;
  fTDCsVeto = NULL;
  fTDCsRawVeto = NULL;
  fTDCsHitVeto = NULL;

  fNHitsVeto = 0;
  fHitsVeto = NULL;
  
  // Hardware errors

  fNError = 0;
  fErrorBlock = 0;
  fError = 0;
  fErrorHead = NULL;
  fErrorMoID = NULL;
  fErrorMoIn = NULL;
  fErrorCode = NULL;
  fErrorTrlr = NULL;

  fNVert = 0;
  fVertX = NULL;
  fVertY = NULL;
  fVertZ = NULL;

  // Requires at least one photon and one proton
  
  fNeutCB = NULL;
  fNeutCh = NULL;
  fNeutCl = NULL;
  fNeutEk = NULL;
  fNeutPx = NULL;
  fNeutPy = NULL;
  fNeutPz = NULL;
  fNeutTh = NULL;
  fNeutPh = NULL;
  fNeutTm = NULL;
  fNeutMi = NULL;
  
  fCharCB = NULL;
  fCharPC = NULL;
  fCharCh = NULL;
  fCharCl = NULL;
  fCharVI = NULL;
  fCharVE = NULL;
  fCharWI = NULL;
  fCharWO = NULL;
  fCharWE = NULL;
  fCharUn = NULL;
  fCharEk = NULL;
  fCharPx = NULL;
  fCharPy = NULL;
  fCharPz = NULL;
  fCharTh = NULL;
  fCharPh = NULL;
  fCharTm = NULL;
  
  fCharWC = NULL;
  fCharVx = NULL;
  fCharVy = NULL;
  fCharVz = NULL;

  fPionEk = NULL;
  fPionPx = NULL;
  fPionPy = NULL;
  fPionPz = NULL;
  fPionMa = NULL;
  fPionTh = NULL;
  fPionPh = NULL;
  fPionTm = NULL;
  fPionOA = NULL;

  fEtaEk = NULL;
  fEtaPx = NULL;
  fEtaPy = NULL;
  fEtaPz = NULL;
  fEtaMa = NULL;
  fEtaTh = NULL;
  fEtaPh = NULL;
  fEtaTm = NULL;
  fEtaOA = NULL;

  fTaggCh = NULL;
  fTaggEk = NULL;
  fTaggTm = NULL;
  fTaggMH = NULL;

  AddCmdList( kPhotoKeys );       // command-line recognition for SetConfig()
}


//-----------------------------------------------------------------------------
TA2PhysicsPMartel::~TA2PhysicsPMartel() {
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
  
  delete fScalTree;
  delete fPartTree;
  delete fTreeFile;
  
}

//---------------------------------------------------------------------------
void TA2PhysicsPMartel::Finish() {

  if ( fTAGG ) {
    TH1D *hScalAcc = new TH1D("hScalAcc","hScalAcc",352,0,352);
    fScalAcc = fLADD->GetScalerAcc();
    for(Int_t i=0; i<(fLADD->GetNelement()); i++) hScalAcc->SetBinContent(i+1,fScalAcc[i]);
  }

  if ( fTreeSave ) {
    fTreeFile->cd();
    fPartTree->Write();
    fScalTree->Write();
    fTreeFile->Close();
    printf("\nTrees saved to %s\n",fTreeFileName);
  }  

  cout << "DetNaI " << fDetNaITot << endl;
  cout << "DetPID " << fDetPIDTot << endl;
  cout << "DetMWPC " << fDetMWPCTot << endl;

  cout << "CheckNaIPID " << fCheckNaIPIDTot << endl;
  cout << "CheckNaIMWPC " << fCheckNaIMWPCTot << endl;
  cout << "CheckPIDMWPC " << fCheckPIDMWPCTot << endl;
  cout << "CheckCharged " << fCheckChargedTot << endl << endl;

  TA2AccessSQL::Finish();

}

//-----------------------------------------------------------------------------
void TA2PhysicsPMartel::ParseMisc(char* line) {
  TA2AccessSQL::ParseMisc(line);
}

//-----------------------------------------------------------------------------
void TA2PhysicsPMartel::SetConfig(Char_t* line, Int_t key) {
  // Any special command-line input for Crystal Ball apparatus
  
  switch (key){
  case EPhotoMassLimits:
    //  Invariant mass limits
    if ( sscanf( line, "%lf%lf", &fMaxMDpi0, &fMaxMDeta ) != 2 ) {
      PrintError( line, "<Compton meson invariant mass limits>");
      return;
    }
    break;
  case ESaveTree:
    //  Output Trees to ROOT file
    if ( sscanf( line, "%s", fTreeFileName) != 1 ) {
      PrintError( line, "<Output Trees to ROOT file>");
      return;
    }
    else fTreeSave = true;
    break;
  case ERecordRaw:
    //  Record Raw Values to Tree
    if ( sscanf( line, "%d", &fRecordRaw) != 1 ) {
      PrintError( line, "<Record Raw Values to Tree>");
      return;
    }
    break;
  case ECherenkov:
    //  Record Cherenkov to Tree
    if ( sscanf( line, "%d", &fCherenkov) != 1 ) {
      PrintError( line, "<Record Cherenkov to Tree>");
      return;
    }
    break;
  case ECutNaIPID:
    //  Establish cut for events in NaI and PID
    if ( sscanf( line, "%s%s", fCutNaIPIDName, fCutNaIPIDFile) != 2 ) {
      PrintError( line, "<Cut on events in NaI and PID>");
      return;
    }
    else fIsCutNaIPID = true;
    break;
  case ECutNaIMWPC:
    //  Establish cut for events in NaI and MWPC
    if ( sscanf( line, "%s%s", fCutNaIMWPCName, fCutNaIMWPCFile) != 2 ) {
      PrintError( line, "<Cut on events in NaI and MWPC>");
      return;
    }
    else fIsCutNaIMWPC = true;
    break;
  case ECutPIDMWPC:
    //  Establish cut for events in PID and MWPC
    if ( sscanf( line, "%s%s", fCutPIDMWPCName, fCutPIDMWPCFile) != 2 ) {
      PrintError( line, "<Cut on events in PID and MWPC>");
      return;
    }
    else fIsCutPIDMWPC = true;
    break;
  default:
    // default main apparatus SetConfig()
    TA2AccessSQL::SetConfig( line, key );
    break;
  }
}

//---------------------------------------------------------------------------
void TA2PhysicsPMartel::PostInit() {
  // Initialise arrays to contain 4 momenta and plotable scaler variables
  // Missing mass, missing energy, cm momentum, energies, angles
  // Initialisation will abort if CB or Tagger not initialised
  // TAPS is optional

  fCB = (TA2CentralApparatus*)((TA2Analysis*)fParent)->GetChild("CB");
  if ( !fCB ) PrintError("Warning!!!","<No CB class found in annalysis>");
  else {
    fCBpart = fCB->GetParticleInfo();
    fNaI = (TA2CalArray*)((TA2Analysis*)fParent)->GetGrandChild("NaI");
    fPID = (TA2PlasticPID*)((TA2Analysis*)fParent)->GetGrandChild("PID");
    fMWPC = (TA2CylMwpc*)((TA2Analysis*)fParent)->GetGrandChild("CylMWPC");
  }

  fTAGG = (TA2Tagger*)((TA2Analysis*)fParent)->GetChild("TAGG");
  if ( !fTAGG ) PrintError("Warning!!!","<No Tagger class found in analysis>");
  else {
    fTAGGpart = fTAGG->GetParticles();
    fLADD = (TA2Ladder*)((TA2Analysis*)fParent)->GetGrandChild("FPD");
    if ( !fLADD ) PrintError("","<No Ladder class found in analysis>",EErrFatal);
  }
  
  fTAPS = (TA2Taps*)((TA2Analysis*)fParent)->GetChild("TAPS");
  if ( !fTAPS ) PrintError("Warning!!!","<No TAPS class found in annalysis>");
  else {
    fTAPSpart = fTAPS->GetParticles();
    fBaF2 = (TA2TAPS_BaF2*)((TA2Analysis*)fParent)->GetGrandChild("BaF2");
    fVeto = (TA2TAPS_Veto*)((TA2Analysis*)fParent)->GetGrandChild("Veto");
  }

  Int_t i;
  TA2Particle* part;
  
  // Maximum # of reaction particles
  Int_t maxparticle = 0;
  if ( fCB ) maxparticle += fCB->GetMaxParticle();
  if ( fTAPS ) maxparticle += fTAPS->GetMaxParticle();

  fDetNaI = new Int_t[maxparticle];
  fDetPID = new Int_t[maxparticle];
  fDetMWPC = new Int_t[maxparticle];

  fENaI = new Double_t[maxparticle];
  fEPID = new Double_t[maxparticle];
  fEMWPC = new Double_t[maxparticle];

  fCheckNaIPID = new Int_t[maxparticle];
  fCheckNaIMWPC = new Int_t[maxparticle];
  fCheckPIDMWPC = new Int_t[maxparticle];
  fCheckCharged = new Int_t[maxparticle];

  fEvnIDList = new Int_t[24];

  fADCsNaI = new Int_t[720];
  fADCsRawNaI = new Int_t[720];
  fADCsHitNaI = new Bool_t[720];
  fTDCsNaI = new Int_t[720];
  fTDCsRawNaI = new Int_t[720];
  fTDCsHitNaI = new Bool_t[720];
  fHitsNaI = new Int_t[720];

  fADCsPID = new Int_t[24];
  fADCsRawPID = new Int_t[24];
  fADCsHitPID = new Bool_t[24];
  fTDCsPID = new Int_t[24];
  fTDCsRawPID = new Int_t[24];
  fTDCsHitPID = new Bool_t[24];
  fHitsPID = new Int_t[24];

  fHitsMWPC = new Int_t[860];

  fADCsBaF2 = new Int_t[438];
  fADCsRawBaF2 = new Int_t[438];
  fADCsHitBaF2 = new Bool_t[438];
  fTDCsBaF2 = new Int_t[438];
  fTDCsRawBaF2 = new Int_t[438];
  fTDCsHitBaF2 = new Bool_t[438];
  fHitsBaF2 = new Int_t[438];

  fADCsVeto = new Int_t[438];
  fADCsRawVeto = new Int_t[438];
  fADCsHitVeto = new Bool_t[438];
  fTDCsVeto = new Int_t[438];
  fTDCsRawVeto = new Int_t[438];
  fTDCsHitVeto = new Bool_t[438];
  fHitsVeto = new Int_t[438];

  fErrorHead = new UInt_t[500];
  fErrorMoID = new Int_t[500];
  fErrorMoIn = new Int_t[500];
  fErrorCode = new Int_t[500];
  fErrorTrlr = new UInt_t[500];

  fVertX = new Float_t[maxparticle];
  fVertY = new Float_t[maxparticle];
  fVertZ = new Float_t[maxparticle];

  fNeutCB = new Bool_t[maxparticle];
  fNeutCh = new Int_t[maxparticle];
  fNeutCl = new Int_t[maxparticle];
  fNeutEk = new Float_t[maxparticle];
  fNeutPx = new Float_t[maxparticle];
  fNeutPy = new Float_t[maxparticle];
  fNeutPz = new Float_t[maxparticle];
  fNeutTh = new Float_t[maxparticle];
  fNeutPh = new Float_t[maxparticle];
  fNeutTm = new Float_t[maxparticle];
  fNeutMi = new Int_t[maxparticle];

  fCharCB = new Bool_t[maxparticle];
  fCharPC = new Bool_t[maxparticle];
  fCharCh = new Int_t[maxparticle];
  fCharCl = new Int_t[maxparticle];
  fCharVI = new Int_t[maxparticle];
  fCharVE = new Float_t[maxparticle];
  fCharWI = new Int_t[maxparticle];
  fCharWO = new Int_t[maxparticle];
  fCharWE = new Float_t[maxparticle];
  fCharUn = new Float_t[maxparticle];
  fCharEk = new Float_t[maxparticle];
  fCharPx = new Float_t[maxparticle];
  fCharPy = new Float_t[maxparticle];
  fCharPz = new Float_t[maxparticle];
  fCharTh = new Float_t[maxparticle];
  fCharPh = new Float_t[maxparticle];
  fCharTm = new Float_t[maxparticle];

  fCharWC = new Bool_t[maxparticle];
  fCharVx = new Float_t[maxparticle];
  fCharVy = new Float_t[maxparticle];
  fCharVz = new Float_t[maxparticle];

  fPionEk = new Float_t[maxparticle];
  fPionPx = new Float_t[maxparticle];
  fPionPy = new Float_t[maxparticle];
  fPionPz = new Float_t[maxparticle];
  fPionMa = new Float_t[maxparticle];
  fPionTh = new Float_t[maxparticle];
  fPionPh = new Float_t[maxparticle];
  fPionTm = new Float_t[maxparticle];
  fPionOA = new Float_t[maxparticle];

  fEtaEk = new Float_t[maxparticle];
  fEtaPx = new Float_t[maxparticle];
  fEtaPy = new Float_t[maxparticle];
  fEtaPz = new Float_t[maxparticle];
  fEtaMa = new Float_t[maxparticle];
  fEtaTh = new Float_t[maxparticle];
  fEtaPh = new Float_t[maxparticle];
  fEtaTm = new Float_t[maxparticle];
  fEtaOA = new Float_t[maxparticle];

  // Maximum # of tagger hits
  Int_t maxtagg = 0;
  if ( fTAGG ) {
    maxtagg = fTAGG->GetMaxParticle() + 1;
    fMaxTagg = maxtagg;
    if ( gAR->IsOnline() ) fNmultS = fLADD->GetNMultihit();
    if ( fNmultS == 0 ) fNmultS = 1;
    
    fTaggTLo = fLADD->GetElement(0)->GetTimeLowThr();
    fTaggTHi = fLADD->GetElement(0)->GetTimeHighThr();
    fTaggOff = fLADD->GetTimeOffset();
    
    // get the electron beam energy
    fEBeamE = fTAGG->GetBeamEnergy();
    
    // get the channel electron energies
    fTaggCal = fLADD->GetECalibration();
  }
    
  fTaggCh = new Int_t[maxtagg*fNmultS];
  fTaggEk = new Float_t[maxtagg*fNmultS];
  fTaggTm = new Float_t[maxtagg*fNmultS];
  fTaggMH = new Int_t[fNmultS];

  // Particle from detectors
  fPARTneut = new TA2Particle*[maxparticle];
  fPARTchar = new TA2Particle*[maxparticle];
  fPARTtagg = new TA2Particle*[maxtagg*fNmultS];
 
  // Pi0
  fPARTpi0 = new TA2Particle*[maxparticle];
  part = new TA2Particle[maxparticle];
  for ( i=0; i<maxparticle; i++ ) fPARTpi0[i] = part + i;
  
  // Eta
  fPARTeta = new TA2Particle*[maxparticle];
  part = new TA2Particle[maxparticle];
  for ( i = 0; i < maxparticle; i++ ) fPARTeta[i] = part + i;
  
  // Gamma prime
  fPARTgprime =  new TA2Particle*[maxparticle];
  part = new TA2Particle[maxparticle];
  for ( i=0; i<maxparticle; i++ ) fPARTgprime[i] = part + i;

  // Arrays used to combine photons to mesons
  Int_t maxperm = 0;
  for ( i=1; i<=maxparticle; i++ ) maxperm += i;
  fMassDpi0 = new Double_t[maxperm];
  fMassDeta = new Double_t[maxperm];
  fMassIJ = new Int_t[maxperm];
  fMassIpi0 = new Int_t[maxperm];
  fMassIeta = new Int_t[maxperm];
  fIsMesonIndex = new Bool_t[maxparticle];

  fTableCB = MakeTable("CB");
  fTblTAPS = MakeTable("TAPS");

  if ( fTreeSave ) {
    TString sDir = gAR->GetTreeDir();
    TString sFile;
    if ( gAR->IsOnline() ) sFile = gAR->GetFileName();
    else sFile = gAR->GetTreeFileList(0);
    while(sFile.Contains("/")) sFile.Remove(0,1+sFile.Index("/"));
    sFile.ReplaceAll(".root",".tree.root");
    sFile.ReplaceAll(".dat",".tree.root");
    sFile.Prepend(sDir);
    cout << sFile << endl;

    fTreeFile = new TFile(sFile, "RECREATE", "TreeFile", 6);
    //fTreeFile = new TFile(fTreeFileName, "RECREATE", "TreeFile", 6);
    fPartTree = new TTree("PartTree", "Kinematics");
    
    if ( fCB || fTAPS ) {
      fPartTree->Branch("NNeut", &fNneut, "NNeut/I");

      fPartTree->Branch("NeutCB", fNeutCB, "NeutCB[NNeut]/O");
      fPartTree->Branch("NeutCh", fNeutCh, "NeutCh[NNeut]/I");
      fPartTree->Branch("NeutCl", fNeutCl, "NeutCl[NNeut]/I");
      fPartTree->Branch("NeutEk", fNeutEk, "NeutEk[NNeut]/F");
      fPartTree->Branch("NeutTh", fNeutTh, "NeutTh[NNeut]/F");
      fPartTree->Branch("NeutPh", fNeutPh, "NeutPh[NNeut]/F");
      fPartTree->Branch("NeutTm", fNeutTm, "NeutTm[NNeut]/F");
      fPartTree->Branch("NeutMi", fNeutMi, "NeutMi[NNeut]/I");

      fPartTree->Branch("NChar", &fNchar, "NChar/I");

      fPartTree->Branch("CharCB", fCharCB, "CharCB[NChar]/O");
      fPartTree->Branch("CharPC", fCharPC, "CharPC[NChar]/O");
      fPartTree->Branch("CharCh", fCharCh, "CharCh[NChar]/I");
      fPartTree->Branch("CharCl", fCharCl, "CharCl[NChar]/I");
      fPartTree->Branch("CharVI", fCharVI, "CharVI[NChar]/I");
      fPartTree->Branch("CharVE", fCharVE, "CharVE[NChar]/F");
      fPartTree->Branch("CharEk", fCharEk, "CharEk[NChar]/F");
      fPartTree->Branch("CharTh", fCharTh, "CharTh[NChar]/F");
      fPartTree->Branch("CharPh", fCharPh, "CharPh[NChar]/F");
      fPartTree->Branch("CharTm", fCharTm, "CharTm[NChar]/F");

      fPartTree->Branch("CharWC", fCharWC, "CharWC[NChar]/O");
      fPartTree->Branch("CharVx", fCharVx, "CharVx[NChar]/F");
      fPartTree->Branch("CharVy", fCharVy, "CharVy[NChar]/F");
      fPartTree->Branch("CharVz", fCharVz, "CharVz[NChar]/F");

      fPartTree->Branch("NPion", &fNpi0, "NPion/I");

      fPartTree->Branch("PionEk", fPionEk, "PionEk[NPion]/F");
      fPartTree->Branch("PionMa", fPionMa, "PionMa[NPion]/F");
      fPartTree->Branch("PionTh", fPionTh, "PionTh[NPion]/F");
      fPartTree->Branch("PionPh", fPionPh, "PionPh[NPion]/F");
      fPartTree->Branch("PionTm", fPionTm, "PionTm[NPion]/F");

      fPartTree->Branch("NEta", &fNeta, "NEta/I");

      fPartTree->Branch("EtaEk", fEtaEk, "EtaEk[NEta]/F");
      fPartTree->Branch("EtaMa", fEtaMa, "EtaMa[NEta]/F");
      fPartTree->Branch("EtaTh", fEtaTh, "EtaTh[NEta]/F");
      fPartTree->Branch("EtaPh", fEtaPh, "EtaPh[NEta]/F");
      fPartTree->Branch("EtaTm", fEtaTm, "EtaTm[NEta]/F");
    }

    if ( fTAGG ) {
      fPartTree->Branch("NTagg", &fNtagg, "NTagg/I");

      fPartTree->Branch("TaggCh", fTaggCh, "TaggCh[NTagg]/I");
      fPartTree->Branch("TaggTm", fTaggTm, "TaggTm[NTagg]/F");
 
      fPartTree->Branch("NMult", &fNmult, "NMult/I");
      fPartTree->Branch("TaggMH", fTaggMH, "TaggMH[NMult]/I");

      fPartTree->Branch("TgRefTDC", &fTgRefTDC, "TgRefTDC/I");
      fPartTree->Branch("CBRefTDC", &fCBRefTDC, "CBRefTDC/I");
    }

    if ( fCherenkov ) {
      fPartTree->Branch("CherADC", &fCherADC, "CherADC/I");
      fPartTree->Branch("CherTDC0", &fCherTDC0, "CherTDC0/I");
      fPartTree->Branch("CherTDC1", &fCherTDC1, "CherTDC1/I");
      fPartTree->Branch("CherTDC2", &fCherTDC2, "CherTDC2/I");
    }

    fPartTree->Branch("BeamChk", &fBeamChk, "BeamChk/O");
    fPartTree->Branch("BeamPol", &fBeamPol, "BeamPol/O");
    fPartTree->Branch("CBESum", &fCBESum, "CBESum/F");

    fPartTree->Branch("EvnID", &fEvnID, "EvnID/I");
    fPartTree->Branch("EvnIDList", fEvnIDList, "EvnIDList[24]/I");

    if ( fNaI ) {
      if ( fNaI->IsRawHits() ) {
	fPartTree->Branch("NADCsNaI", &fNADCsNaI, "NADCsNaI/I");
	fPartTree->Branch("ADCsNaI", fADCsNaI, "ADCsNaI[NADCsNaI]/I");
	if ( fRecordRaw ) fPartTree->Branch("ADCsRawNaI", fADCsRawNaI, "ADCsRawNaI[720]/I");
	fPartTree->Branch("ADCsHitNaI", fADCsHitNaI, "ADCsHitNaI[NADCsNaI]/O");

	fPartTree->Branch("NTDCsNaI", &fNTDCsNaI, "NTDCsNaI/I");
	fPartTree->Branch("TDCsNaI", fTDCsNaI, "TDCsNaI[NTDCsNaI]/I");
	if ( fRecordRaw ) fPartTree->Branch("TDCsRawNaI", fTDCsRawNaI, "TDCsRawNaI[720]/I");
	fPartTree->Branch("TDCsHitNaI", fTDCsHitNaI, "TDCsHitNaI[NTDCsNaI]/O");
      }
      fPartTree->Branch("NHitsNaI", &fNHitsNaI, "NHitsNaI/I");
      fPartTree->Branch("HitsNaI", fHitsNaI, "HitsNaI[NHitsNaI]/I");
    }

    if ( fPID ) {
      if ( fPID->IsRawHits() ) {
	fPartTree->Branch("NADCsPID", &fNADCsPID, "NADCsPID/I");
	fPartTree->Branch("ADCsPID", fADCsPID, "ADCsPID[NADCsPID]/I");
	if ( fRecordRaw ) fPartTree->Branch("ADCsRawPID", fADCsRawPID, "ADCsRawPID[24]/I");
	fPartTree->Branch("ADCsHitPID", fADCsHitPID, "ADCsHitPID[NADCsPID]/O");
      
	fPartTree->Branch("NTDCsPID", &fNTDCsPID, "NTDCsPID/I");
	fPartTree->Branch("TDCsPID", fTDCsPID, "TDCsPID[NTDCsPID]/I");
	if ( fRecordRaw ) fPartTree->Branch("TDCsRawPID", fTDCsRawPID, "TDCsRawPID[24]/I");
	fPartTree->Branch("TDCsHitPID", fTDCsHitPID, "TDCsHitPID[NTDCsPID]/O");
      }
      fPartTree->Branch("NHitsPID", &fNHitsPID, "NHitsPID/I");
      fPartTree->Branch("HitsPID", fHitsPID, "HitsPID[NHitsPID]/I");
    }

    if ( fMWPC ) {
      fPartTree->Branch("NHitsMWPC", &fNHitsMWPC, "NHitsMWPC/I");
      fPartTree->Branch("HitsMWPC", fHitsMWPC, "HitsPID[NHitsMWPC]/I");
    }

    if ( fBaF2 ) {
      if ( fBaF2->IsRawHits() ) {
	fPartTree->Branch("NADCsBaF2", &fNADCsBaF2, "NADCsBaF2/I");
	fPartTree->Branch("ADCsBaF2", fADCsBaF2, "ADCsBaF2[NADCsBaF2]/I");
	if ( fRecordRaw ) fPartTree->Branch("ADCsRawBaF2", fADCsRawBaF2, "ADCsRawBaF2[438]/I");
	fPartTree->Branch("ADCsHitBaF2", fADCsHitBaF2, "ADCsHitBaF2[NADCsBaF2]/O");
	
	fPartTree->Branch("NTDCsBaF2", &fNTDCsBaF2, "NTDCsBaF2/I");
	fPartTree->Branch("TDCsBaF2", fTDCsBaF2, "TDCsBaF2[NTDCsBaF2]/I");
	if ( fRecordRaw ) fPartTree->Branch("TDCsRawBaF2", fTDCsRawBaF2, "TDCsRawBaF2[438]/I");
	fPartTree->Branch("TDCsHitBaF2", fTDCsHitBaF2, "TDCsHitBaF2[NTDCsBaF2]/O");
      }
      fPartTree->Branch("NHitsBaF2", &fNHitsBaF2, "NHitsBaF2/I");
      fPartTree->Branch("HitsBaF2", fHitsBaF2, "HitsBaF2[NHitsBaF2]/I");
    }
     
    if ( fVeto ) {
      if ( fVeto->IsRawHits() ) {
	fPartTree->Branch("NADCsVeto", &fNADCsVeto, "NADCsVeto/I");
	fPartTree->Branch("ADCsVeto", fADCsVeto, "ADCsVeto[NADCsVeto]/I");
	if ( fRecordRaw ) fPartTree->Branch("ADCsRawVeto", fADCsRawVeto, "ADCsRawVeto[438]/I");
	fPartTree->Branch("ADCsHitVeto", fADCsHitVeto, "ADCsHitVeto[NADCsVeto]/O");
	
	fPartTree->Branch("NTDCsVeto", &fNTDCsVeto, "NTDCsVeto/I");
	fPartTree->Branch("TDCsVeto", fTDCsVeto, "TDCsVeto[NTDCsVeto]/I");
	if ( fRecordRaw ) fPartTree->Branch("TDCsRawVeto", fTDCsRawVeto, "TDCsRawVeto[438]/I");
	fPartTree->Branch("TDCsHitVeto", fTDCsHitVeto, "TDCsHitVeto[NTDCsVeto]/O");
      }
      fPartTree->Branch("NHitsVeto", &fNHitsVeto, "NHitsVeto/I");
      fPartTree->Branch("HitsVeto", fHitsVeto, "HitsVeto[NHitsVeto]/I");
    }

    fPartTree->Branch("NError", &fNError, "NError/I");
    fPartTree->Branch("ErrorHead", fErrorHead, "ErrorHead[NError]/i");
    fPartTree->Branch("ErrorMoID", fErrorMoID, "ErrorMoID[NError]/I");
    fPartTree->Branch("ErrorMoIn", fErrorMoIn, "ErrorMoIn[NError]/I");
    fPartTree->Branch("ErrorCode", fErrorCode, "ErrorCode[NError]/I");
    fPartTree->Branch("ErrorTrlr", fErrorTrlr, "ErrorTrlr[NError]/i");

    if ( fCB ) {
      fPartTree->Branch("NVert", &fNVert, "NVert/I");
      fPartTree->Branch("VertX", fVertX, "VertX[NVert]/F");
      fPartTree->Branch("VertY", fVertY, "VertY[NVert]/F");
      fPartTree->Branch("VertZ", fVertZ, "VertZ[NVert]/F");
    }

    fScalTree = new TTree("ScalTree", "Kinematics");
    fScalTree->Branch("EvnN", &fEvnN, "EvnN/I");
    fScalTree->Branch("EvnID", &fEvnID, "EvnID/I");
    fScalTree->Branch("Scaler", fScaler, Form("Scaler[%d]/i",GetMaxScaler()));
  }

  if ( fIsCutNaIPID ) {
    TFile *CutFile = new TFile(fCutNaIPIDFile, "READ");
    fCutNaIPID = (TCutG*)CutFile->Get(fCutNaIPIDName);
    cout << "Cut enabled for NaI and PID events." << endl;
    CutFile->Close();
    delete CutFile;
  }
  if ( fIsCutNaIMWPC ) {
    TFile *CutFile = new TFile(fCutNaIMWPCFile, "READ");
    fCutNaIMWPC = (TCutG*)CutFile->Get(fCutNaIMWPCName);
    cout << "Cut enabled for NaI and MWPC events." << endl;
    CutFile->Close();
    delete CutFile;
  }
  if ( fIsCutPIDMWPC ) {
    TFile *CutFile = new TFile(fCutPIDMWPCFile, "READ");
    fCutPIDMWPC = (TCutG*)CutFile->Get(fCutPIDMWPCName);
    cout << "Cut enabled for PID and MWPC events." << endl;
    CutFile->Close();
    delete CutFile;
  }

  gROOT->cd();

  // Default physics initialisation
  TA2AccessSQL::PostInit();
}

//-----------------------------------------------------------------------------
void TA2PhysicsPMartel::LoadVariable( ) {
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //	array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //				 EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //				 MultiX  for a multi-valued variable
  
  TA2AccessSQL::LoadVariable();
  TA2DataManager::LoadVariable("TgRefTDC", &fTgRefTDC, EISingleX);
  TA2DataManager::LoadVariable("CBRefTDC", &fCBRefTDC, EISingleX);
  TA2DataManager::LoadVariable("SynchDif", &fSynchDif, EISingleX);

  TA2DataManager::LoadVariable("Nneut", &fNneut, EISingleX);
  TA2DataManager::LoadVariable("Nchar", &fNchar, EISingleX);
  TA2DataManager::LoadVariable("Ntagg", &fNtagg, EISingleX);

  TA2DataManager::LoadVariable("Npi0", &fNpi0, EISingleX);
  TA2DataManager::LoadVariable("Neta", &fNeta, EISingleX);
  TA2DataManager::LoadVariable("Ngprime", &fNgprime, EISingleX);

  TA2DataManager::LoadVariable("M2g", &fM2g, EDSingleX);
  TA2DataManager::LoadVariable("M6g", &fM6g, EDSingleX);

  TA2DataManager::LoadVariable("DetNaI", fDetNaI, EIMultiX);
  TA2DataManager::LoadVariable("DetPID", fDetPID, EIMultiX);
  TA2DataManager::LoadVariable("DetMWPC", fDetMWPC, EIMultiX);

  TA2DataManager::LoadVariable("ENaI", fENaI, EDMultiX);
  TA2DataManager::LoadVariable("EPID", fEPID, EDMultiX);
  TA2DataManager::LoadVariable("EMWPC", fEMWPC, EDMultiX);

  TA2DataManager::LoadVariable("CheckNaIPID", fCheckNaIPID, EIMultiX);
  TA2DataManager::LoadVariable("CheckNaIMWPC", fCheckNaIMWPC, EIMultiX);
  TA2DataManager::LoadVariable("CheckPIDMWPC", fCheckPIDMWPC, EIMultiX);
  TA2DataManager::LoadVariable("CheckCharged", fCheckCharged, EIMultiX);

  return;
}

//-----------------------------------------------------------------------------
void TA2PhysicsPMartel::Reconstruct() {
  // General reconstruction of reaction kinematics in Mainz tagged-photon
  // meson production experiments.
  // Use 4-momenta and PDG-index information from apparati to reconstruct
  // reaction kinematics. The PDG index (and 4-momentum) assigned by the
  // apparatus is not considered binding, e.g. in cases where n/gamma
  // discrimination by an apparatus is not possible, in which case it
  // defaults to kGamma. The method TA2ParticleID->SetMassP4( *p4, ipdg )
  // may be used to reset the rest-mass of an existing 4 momentum *p4 to that
  // corresponding to PDG index ipdg.
  // This one deals with pion and eta photoproduction on the nucleon.

  Int_t i, j, k, l;

  Int_t ncb = 0;
  if ( fCB ) ncb = fCB->GetNParticle();        // # particles in CB
  Int_t ntaps = 0;
  if ( fTAPS ) ntaps = fTAPS->GetNParticle();  // # particles in TAPS

  fTgRefTDC = ENullHit;
  fCBRefTDC = ENullHit;
  fSynchDif = ENullHit;
  
  fCherADC = ENullHit;
  fCherTDC0 = ENullHit;
  fCherTDC1 = ENullHit;
  fCherTDC2 = ENullHit;

  fCBESum = 0.0;
  
  // zero particle counters
  fNneut = 0;
  fNchar = 0;
  fNtagg = 0;

  fNpi0 = 0;
  fNeta = 0;
  fNgprime = 0;

  fM2g = ENullHit;                             // zero 2-gamma inv. mass
  fM6g = ENullHit;                             // zero 6-gamma inv. mass
  
  fTgRefTDC = fADC[2000];
  fCBRefTDC = fADC[1400];
  if ( fTAGG && fCB ) fSynchDif = (fTgRefTDC - fCBRefTDC);
  else fSynchDif = 0;

  if ( fCherenkov ) {
    fCherADC = fADC[125];
    fCherTDC0 = fMulti[2001]->GetHit(0);
    fCherTDC1 = fMulti[2001]->GetHit(1);
    fCherTDC2 = fMulti[2001]->GetHit(2);
  }

  fBeamChk = false;
  fBeamPol = false;

  if ( fADC[6] ) {
    // Older helicity bit
    if ( !(fADC[6] & 1<<0) && !(fADC[6] & 1<<1) && (fADC[6] & 1<<2) && (fADC[6] & 1<<3) && !(fADC[6] & 1<<4) ) {
      fBeamChk = true;
      fBeamPol = false;
    }
    else if ( !(fADC[6] & 1<<0) && (fADC[6] & 1<<1) && !(fADC[6] & 1<<2) && !(fADC[6] & 1<<3) && (fADC[6] & 1<<4) ) {
      fBeamChk = true;
      fBeamPol = true;
    }
  }
  else if ( fADC[7] ) {
    // Newer helicity bit
    if ( !(fADC[7] & 1<<0) && !(fADC[7] & 1<<1) && !(fADC[7] & 1<<2) && (fADC[7] & 1<<3) ) {
      fBeamChk = true;
      fBeamPol = false;
    }
    else if ( (fADC[7] & 1<<0) && !(fADC[7] & 1<<1) && (fADC[7] & 1<<2) && !(fADC[7] & 1<<3) ) {
      fBeamChk = true;
      fBeamPol = true;
    }
  }

  fEvnID = gAN->GetNDAQEvent();
  if ( fMulti[400] ) for(i=0; i<24; i++) fEvnIDList[i] = (fMulti[400]->GetHit(i));

  if ( !(gAR->GetProcessType() == EMCProcess) && (gAR->IsScalerRead()) && (fTreeSave) ) fScalTree->Fill();
  fEvnN++;  

  if ( fNaI ) {
    fCBESum = (Float_t)(fNaI->GetTotalEnergy());
    
    if ( fNaI->IsRawHits() ) {
      fNADCsNaI = fNaI->GetNADChits();
      fTempHits = fNaI->GetRawEnergyHits();
      for(i=0; i<720; i++) fADCsRawNaI[i] = 0;
      for(i=0; i<fNADCsNaI; i++){
	fADCsNaI[i] = fTempHits[i];
	if ( fRecordRaw ) fADCsRawNaI[fADCsNaI[i]] = (Int_t)fNaI->GetElement(fADCsNaI[i])->GetRawADCValue();
	fADCsHitNaI[i] = fNaI->GetElement(fADCsNaI[i])->CheckEnergy();
      }

      fNTDCsNaI = fNaI->GetNTDChits();
      fTempHits = fNaI->GetRawTimeHits();
      for(i=0; i<720; i++) fTDCsRawNaI[i] = 0;
      for(i=0; i<fNTDCsNaI; i++){
	fTDCsNaI[i] = fTempHits[i];
	if ( fRecordRaw ) fTDCsRawNaI[fTDCsNaI[i]] = (Int_t)fNaI->GetElement(fTDCsNaI[i])->GetRawTDCValue();
	fTDCsHitNaI[i] = fNaI->GetElement(fTDCsNaI[i])->CheckTime();
      }
    }
    fNHitsNaI = fNaI->GetNhits();
    for(i=0; i<fNHitsNaI; i++){
      fHitsNaI[i] = fNaI->GetHits(i);
    }

    fNVert = fCB->GetNvertexes();
    const TVector3 *fVert = fCB->GetVertexes();
    for(i=0; i<fNVert; i++){
      fVertX[i] = fVert[i].X();
      fVertY[i] = fVert[i].Y();
      fVertZ[i] = fVert[i].Z();
    }
  }

  if ( fPID ) {
    if ( fPID->IsRawHits() ) {
      fNADCsPID = fPID->GetNADChits();
      fTempHits = fPID->GetRawEnergyHits();
      for(i=0; i<24; i++) fADCsRawPID[i] = 0;
      for(i=0; i<fNADCsPID; i++){
	fADCsPID[i] = fTempHits[i];
	if ( fRecordRaw ) fADCsRawPID[fADCsPID[i]] = (Int_t)fPID->GetElement(fADCsPID[i])->GetRawADCValue();
	fADCsHitPID[i] = fPID->GetElement(fADCsPID[i])->CheckEnergy();
      }

      fNTDCsPID = fPID->GetNTDChits();
      fTempHits = fPID->GetRawTimeHits();
      for(i=0; i<24; i++) fTDCsRawPID[i] = 0;
      for(i=0; i<fNTDCsPID; i++){
	fTDCsPID[i] = fTempHits[i];
	if ( fRecordRaw ) fTDCsRawPID[fTDCsPID[i]] = (Int_t)fPID->GetElement(fTDCsPID[i])->GetRawTDCValue();
	fTDCsHitPID[i] = fPID->GetElement(fTDCsPID[i])->CheckTime();
      }
    }
    fNHitsPID = fPID->GetNhits();
    for(i=0; i<fNHitsPID; i++){
      fHitsPID[i] = fPID->GetHits(i);
    }
  }

  if ( fMWPC ) {
    fNHitsMWPC = fMWPC->GetNhits();
    for(i=0; i<fNHitsMWPC; i++){
      fHitsMWPC[i] = fMWPC->GetHits(i);
    }
  }

  if ( fBaF2 ) {
    if ( fBaF2->IsRawHits() ) {
      fNADCsBaF2 = fBaF2->GetNADChits();
      fTempHits = fBaF2->GetRawEnergyHits();
      for(i=0; i<438; i++) fADCsRawBaF2[i] = 0;
      for(i=0; i<fNADCsBaF2; i++){
	fADCsBaF2[i] = fTempHits[i];
	if ( fRecordRaw ) fADCsRawBaF2[fADCsBaF2[i]] = (Int_t)fBaF2->GetElement(fADCsBaF2[i])->GetRawADCValue();
	fADCsHitBaF2[i] = fBaF2->GetElement(fADCsBaF2[i])->CheckEnergy();
      }
      
      fNTDCsBaF2 = fBaF2->GetNTDChits();
      fTempHits = fBaF2->GetRawTimeHits();
      for(i=0; i<438; i++) fTDCsRawBaF2[i] = 0;
      for(i=0; i<fNTDCsBaF2; i++){
	fTDCsBaF2[i] = fTempHits[i];
	if ( fRecordRaw ) fTDCsRawBaF2[fTDCsBaF2[i]] = (Int_t)fBaF2->GetElement(fTDCsBaF2[i])->GetRawTDCValue();
	fTDCsHitBaF2[i] = fBaF2->GetElement(fTDCsBaF2[i])->CheckTime();
      }
    }
    fNHitsBaF2 = fBaF2->GetNhits();
    for(i=0; i<fNHitsBaF2; i++){
      fHitsBaF2[i] = fBaF2->GetHits(i);
    }
  }

  if ( fVeto ) {
    if ( fVeto->IsRawHits() ) {
      fNADCsVeto = fVeto->GetNADChits();
      fTempHits = fVeto->GetRawEnergyHits();
      for(i=0; i<438; i++) fADCsRawVeto[i] = 0;
      for(i=0; i<fNADCsVeto; i++){
	fADCsVeto[i] = fTempHits[i];
	if ( fRecordRaw ) fADCsRawVeto[fADCsVeto[i]] = (Int_t)fVeto->GetElement(fADCsVeto[i])->GetRawADCValue();
	fADCsHitVeto[i] = fVeto->GetElement(fADCsVeto[i])->CheckEnergy();
      }
      
      fNTDCsVeto = fVeto->GetNTDChits();
      fTempHits = fVeto->GetRawTimeHits();
      for(i=0; i<438; i++) fTDCsRawVeto[i] = 0;
      for(i=0; i<fNTDCsVeto; i++){
	fTDCsVeto[i] = fTempHits[i];
	if ( fRecordRaw ) fTDCsRawVeto[fTDCsVeto[i]] = (Int_t)fVeto->GetElement(fTDCsVeto[i])->GetRawTDCValue();
	fTDCsHitVeto[i] = fVeto->GetElement(fTDCsVeto[i])->CheckTime();
      }
    }
    fNHitsVeto = fVeto->GetNhits();
    for(i=0; i<fNHitsVeto; i++){
      fHitsVeto[i] = fVeto->GetHits(i);
    }
  }

  fNError = gAR->GetHardError();
  fErrorBlock = gAR->GetHardwareError();
  for( i=0; i<fNError; i++ ){
    fError = fErrorBlock + i;
    fErrorHead[i] = fError->fHeader;
    fErrorMoID[i] = fError->fModID;
    fErrorMoIn[i] = fError->fModIndex;
    fErrorCode[i] = fError->fErrCode;
    fErrorTrlr[i] = fError->fTrailer;
  }

  MarkEndBuffer();

  // Sort 4-momenta provided by apparati according to particle type

  fNtagg = 0;
  fNmult = 0;
  for ( i = 0; i < fNmultS; i++ ) {
    if ( fNmultS > 1 ) {
      fNtagg += fLADD->GetNhitsM(i);
      if ( (fLADD->GetNhitsM(i)) > 0 ) {
	fTaggMH[i] = (fLADD->GetNhitsM(i));
	fNmult++;
      }
    }
    else{
      fNtagg += fLADD->GetNhits();
      if ( (fLADD->GetNhits()) > 0 ) {
	fTaggMH[0] = (fLADD->GetNhits());
	fNmult++;
      }
    }
  }

  fMTime = fLADD->GetTimeM();

  // loop over hit multiplicity
  k = 0;
  for (i = 0; i < fNmultS; i++) {   
    if ( fNmultS > 1 ) {
      fNMHits = fLADD->GetNhitsM(i);
      fMHits = fLADD->GetHitsM(i);
      fMTmOR = fLADD->GetTimeORM(i);
    }
    else {
      fNMHits = fLADD->GetNhits();
      fMHits = fLADD->GetHits();
      fMTmOR = fLADD->GetTimeOR();
    }

    // loop over hits of current multiplicity
    for (j = 0; j < fNMHits; j++) {
      // set hit element, time and energy
      fTaggCh[k] = fMHits[j];
      fTaggEk[k] = fEBeamE - fTaggCal[fMHits[j]];
      fTaggTm[k] = fMTmOR[j];

      if ( !(gAR->GetProcessType() == EMCProcess) ) {
	l = 0;
	while((fTaggTm[k]==(fTaggOff-1)) || (fTaggTm[k] < (fTaggTLo+fTaggOff)) || (fTaggTm[k] >= (fTaggTHi+fTaggOff))) fTaggTm[k] = (fMTime[l++][fMHits[j]]+fTaggOff);
      }
      else fTaggTm[k] = 0;
      k++;
    }
  }
  //if ( (gAR->GetProcessType() == EMCProcess) && (fNtagg == 2) ) fNtagg = 1;

  if ( fCB ) {
    // CB
    const TA2CentralTrack *fTracks = fCB->GetTracks();
    
    for ( i=0; i<ncb; i++ ) {
      switch ( (fCBpart+i)->GetParticleID() ) {   // PDG code
      case kGamma:                                // neutral
	fPARTneut[fNneut] = fCBpart+i;
	fNeutCB[fNneut] = kTRUE;
	fNeutMi[fNneut] = 0;
	fNneut++;
	break;
      default:                                    // charged
	fPARTchar[fNchar] = fCBpart+i;
	fCharCB[fNchar] = kTRUE;
	fCharWC[fNchar] = fTracks[i].HasMwpc();
	fCharVx[fNchar] = fTracks[i].GetPsVertex().X();
	fCharVy[fNchar] = fTracks[i].GetPsVertex().Y();
	fCharVz[fNchar] = fTracks[i].GetPsVertex().Z();
	SortCharged();
	fNchar++;
      }
    }
  }

  if ( fTAPS ) {
    // TAPS
    for ( i=0; i<ntaps; i++ ) {
      switch ( (fTAPSpart+i)->GetParticleID() ) { // PDG code
      case kGamma:                                // neutral
	fPARTneut[fNneut] = fTAPSpart+i;
	fNeutCB[fNneut] = kFALSE;
	fNeutMi[fNneut] = 0;
	fNneut++;
	break;
      case kProton:                               // proton
	fPARTchar[fNchar] = fTAPSpart+i;
	fCharCB[fNchar] = kFALSE;
	fCharWC[fNchar] = kFALSE;
	fCharVx[fNchar] = 0;
	fCharVy[fNchar] = 0;
	fCharVz[fNchar] = 0;
	SortCharged();
	fCharPC[fNchar] = kTRUE;
	fNchar++;
	break;
      default:                                    // charged
	fPARTchar[fNchar] = fTAPSpart+i;
	fCharCB[fNchar] = kFALSE;
	fCharWC[fNchar] = kFALSE;
	fCharVx[fNchar] = 0;
	fCharVy[fNchar] = 0;
	fCharVz[fNchar] = 0;
	SortCharged();
	fCharPC[fNchar] = kFALSE;
	fNchar++;                          
      }
    }
  }

  // Check if detected photons combine to give pi0 or eta
  TLorentzVector p4;
  switch ( fNneut ) {
  case 1:
    // Just 1 photon....assume it is a gamma-prime
    fPARTgprime[0] = fPARTneut[0];
    fNgprime = 1;
    break;
  case 2:
    // 2 photons detected, fast check if they make a pi0 or eta
    Sort2Photon();
    break;
  default:
    // More than 2 photons 
    SortNPhoton();
    // Check for 3-pi0 eta decay mode
    if ( fNpi0==3 ) {
      p4 = (*fPARTpi0[0]).GetP4() + (*fPARTpi0[1]).GetP4()
	+ (*fPARTpi0[2]).GetP4();
      fMassDpi0[0] = TMath::Abs( p4.M() - fParticleID->GetMassMeV( kEta));
      if ( fMassDpi0[0]<fMaxMDeta ) {
	(*fPARTeta[0]).GetP4() = p4;
	fNeta = 1;
	fNpi0 = 0;
	// Need to re-adjust the Neutrals Meson index here
      }
    }
    break;
  }

  TA2Particle fNeutral;
  TA2Particle fCharged;
  TA2Particle fPion;
  TA2Particle fEta;
  TA2Particle fDecay1;
  TA2Particle fDecay2;

  for ( i=0; i<fNneut; i++ ) {
    
    fNeutral = *fPARTneut[i];
    
    fNeutCh[i] = fNeutral.GetCentralIndex();
    fNeutCl[i] = fNeutral.GetClusterSize();
    fNeutEk[i] = (Float_t)fNeutral.GetT();
    fNeutPx[i] = (Float_t)fNeutral.GetPx();
    fNeutPy[i] = (Float_t)fNeutral.GetPy();
    fNeutPz[i] = (Float_t)fNeutral.GetPz();
    fNeutTh[i] = (Float_t)fNeutral.GetThetaDg();
    fNeutPh[i] = (Float_t)fNeutral.GetPhiDg();
    fNeutTm[i] = (Float_t)fNeutral.GetTime();

  }

  for ( i=0; i<fNchar; i++ ) {
    
    fCharged = *fPARTchar[i];
    
    Float_t TempT, CorrT, LossT;
    
    fCharCh[i] = fCharged.GetCentralIndex();
    fCharCl[i] = fCharged.GetClusterSize();
    if (fCharCl[i] < 0 ) fCharCl[i] = 0;
    fCharVI[i] = fCharged.GetVetoIndex();
    fCharVE[i] = (Float_t)fCharged.GetVetoEnergy();
    //fCharWI[i] = fCharged.GetIinterMwpc(0);
    //fCharWO[i] = fCharged.GetIinterMwpc(1);
    //fCharWE[i] = (Float_t)fCharged.GetEtrackMwpc();
    fCharWI[i] = 0;
    fCharWO[i] = 0;
    fCharWE[i] = 0;
    
    fCharUn[i] = (Float_t)fCharged.GetT();
    TempT = fCharUn[i];
    
    if ( fTableCB && fCharCB[i] && (fCharCh[i] >= 0) ) {
      if ( gAR->GetProcessType() == EMCProcess ) CorrT = 0;
      else CorrT = fECorrCB->Eval(TempT);
      TempT = (TempT+CorrT);
      LossT = fELossCB[fCharCh[i]]->Eval(TempT);
      TempT = (TempT+LossT);
      fCharged.SetKinetic(TempT);
    }
    else if ( fTblTAPS && !fCharCB[i] ) {
      if ( gAR->GetProcessType() == EMCProcess ) CorrT = 0;
      else CorrT = fECoTAPS->Eval(TempT);
      TempT = (TempT+CorrT);
      LossT = fELoTAPS[fCharCh[i]]->Eval(TempT);
      TempT = (TempT+LossT);
      fCharged.SetKinetic(TempT);
    }
    
    fCharEk[i] = (Float_t)fCharged.GetT();
    if ( fCharEk[i] < 0 ) fCharEk[i] = 0;
    fCharPx[i] = (Float_t)fCharged.GetPx();
    fCharPy[i] = (Float_t)fCharged.GetPy();
    fCharPz[i] = (Float_t)fCharged.GetPz();
    fCharTh[i] = (Float_t)fCharged.GetThetaDg();
    fCharPh[i] = (Float_t)fCharged.GetPhiDg();
    fCharTm[i] = (Float_t)fCharged.GetTime();
    
  }

  for ( i=0; i<fNpi0; i++ ) {
    
    fPion = *fPARTpi0[i];
    
    k = fNneut;
    for ( j=0; j<fNneut; j++ ) {
      if ( ( fNeutMi[j]==(i+1) ) && ( j<k ) ) {
	fDecay1 = *fPARTneut[j];
	k = j;
      }
      else if ( ( fNeutMi[j]==(i+1) ) && ( j>k ) ) {
	fDecay2 = *fPARTneut[j];
      }
    }
    
    fPionEk[i] = (Float_t)fPion.GetT();
    fPionPx[i] = (Float_t)fPion.GetPx();
    fPionPy[i] = (Float_t)fPion.GetPy();
    fPionPz[i] = (Float_t)fPion.GetPz();
    fPionMa[i] = (Float_t)fPion.GetM();
    fPionTh[i] = (Float_t)fPion.GetThetaDg();
    fPionPh[i] = (Float_t)fPion.GetPhiDg();
    fPionTm[i] = (Float_t)fPion.GetTime();
    fPionOA[i] = (Float_t)(fDecay1.GetVect()).Angle(fDecay2.GetVect())*TMath::RadToDeg();
  }

  for ( i=0; i<fNeta; i++ ) {
    
    fEta = *fPARTeta[i];
    
    k = fNneut;
    for ( j=0; j<fNneut; j++ ) {
      if ( ( fNeutMi[j]==(i+1) ) && ( j<k ) ) {
	fDecay1 = *fPARTneut[j];
	k = j;
      }
      else if ( ( fNeutMi[j]==(i+1) ) && ( j>k ) ) {
	fDecay2 = *fPARTneut[j];
      }
    }
    
    fEtaEk[i] = (Float_t)fEta.GetT();
    fEtaPx[i] = (Float_t)fEta.GetPx();
    fEtaPy[i] = (Float_t)fEta.GetPy();
    fEtaPz[i] = (Float_t)fEta.GetPz();
    fEtaMa[i] = (Float_t)fEta.GetM();
    fEtaTh[i] = (Float_t)fEta.GetThetaDg();
    fEtaPh[i] = (Float_t)fEta.GetPhiDg();
    fEtaTm[i] = (Float_t)fEta.GetTime();
    fEtaOA[i] = (Float_t)(fDecay1.GetVect()).Angle(fDecay2.GetVect())*TMath::RadToDeg();
  }

  if ( fTreeSave ) fPartTree->Fill();
  
  MarkEndBuffer();

}
