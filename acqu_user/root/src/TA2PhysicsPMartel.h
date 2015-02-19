//--Author	PP Martel    10th Oct 2010
//--Update	PP Martel    19th Feb 2011
//
// TA2PhysicsPMartel
//
// Reconstruction of Compton and Pi0 kinematics. 

#ifndef __TA2PhysicsPMartel_h__
#define __TA2PhysicsPMartel_h__

#include "TAcquRoot.h"
#include "TAcquFile.h"
#include "TA2Mk2Format.h"
#include "TA2AccessSQL.h"
#include "TA2Physics.h"
#include "TA2Analysis.h"
#include "TA2CentralApparatus.h"
#include "TA2CentralTrack.h"
#include "TA2CalArray.h"
#include "TA2PlasticPID.h"
#include "TA2CylMwpc.h"
#include "TA2Tagger.h"
#include "TA2Ladder.h"
#include "TA2Taps.h"
#include "TA2TAPS_BaF2.h"
#include "TA2TAPS_Veto.h"
#include "TA2Event.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include <fstream>

class TA2Apparatus;

class TA2PhysicsPMartel : public TA2AccessSQL {
  
 protected:

  //Root File and Tree Outputs
  Bool_t fTreeSave;              // Save Tree
  TFile* fTreeFile;              // Tree File
  Char_t fTreeFileName[100];
  TTree* fPartTree;              // Part Tree
  TTree* fScalTree;              // Scal Tree

  Char_t fCutNaIPIDName[100];
  Char_t fCutNaIPIDFile[100];
  TCutG *fCutNaIPID;
  Bool_t fIsCutNaIPID;
  Bool_t fCanCutNaIPID;

  Char_t fCutNaIMWPCName[100];
  Char_t fCutNaIMWPCFile[100];
  TCutG *fCutNaIMWPC;
  Bool_t fIsCutNaIMWPC;
  Bool_t fCanCutNaIMWPC;

  Char_t fCutPIDMWPCName[100];
  Char_t fCutPIDMWPCFile[100];
  TCutG *fCutPIDMWPC;
  Bool_t fIsCutPIDMWPC;
  Bool_t fCanCutPIDMWPC;
  
  TA2CentralApparatus* fCB;      // Crystal Ball
  TA2CalArray* fNaI;             // NaI
  TA2PlasticPID* fPID;           // PID
  TA2CylMwpc* fMWPC;             // MWPC
  TA2Tagger* fTAGG;              // Glasgow photon tagger
  TA2Ladder* fLADD;              // Ladder
  TA2Taps* fTAPS;                // TAPS
  TA2TAPS_BaF2* fBaF2;           // BaF2
  TA2TAPS_Veto* fVeto;           // Veto
  
  TA2Particle* fTAGGpart;        // TA2Particles from Tagger
  TA2Particle* fCBpart;          // TA2Particles from CB
  TA2Particle* fTAPSpart;        // TA2Particles from TAPS
  
  TA2Particle** fPARTtagg;       // TA2Particle tagger photon
  TA2Particle** fPARTneut;       // TA2Particle photon
  TA2Particle** fPARTchar;       // TA2Particle proton
  
  TA2Particle** fPARTpi0;	 // TA2Particle pi0
  TA2Particle** fPARTeta;        // TA2Particle eta
  TA2Particle** fPARTgprime;     // TA2Particle gprime
  
  TLorentzVector fP4photonTot;   // total 4-momentum of gammas

  Int_t fTgRefTDC;               // Tagger TDC for synch check
  Int_t fCBRefTDC;               // CB TDC for synch check
  Int_t fSynchDif;               // Synch check

  Int_t fCherADC;                // Cherenkov ADC
  Int_t fCherTDC0;               // Cherenkov TDC M0
  Int_t fCherTDC1;               // Cherenkov TDC M1
  Int_t fCherTDC2;               // Cherenkov TDC M2

  Bool_t fBeamChk;               // Beam Helicity
  Bool_t fBeamPol;               // Beam Helicity
  Float_t fCBESum;               // CB Energy Sum
  Int_t fDumpN;                  // Scaler Dump Number
  Int_t fEvnN;                   // Event Number

  Double_t* fScalAcc;            // Accumulated Scalers
  UInt_t* fScalCur;              // Current Scalers

  Int_t fNneut;                  // # neutral
  Int_t fNchar;                  // # charged
  Int_t fNtagg;                  // tagger hits

  Int_t fMaxTagg;                // max # tagger hits
  Int_t fNmultS;                 // Number of multi-hits stored
  Int_t fNmult;                  // Highest number of multiplicity
  Int_t fNMHits;                 // Number of hits
  Int_t* fMHits;                 // Tagger channels
  Double_t** fMTime;             // Tagger time
  Double_t* fMTmOR;              // Tagger time
  Double_t fTaggTLo;
  Double_t fTaggTHi;
  Double_t fTaggOff;

  Double_t fEBeamE;              // Electron Beam Energy
  const Double_t* fTaggCal;            // Tagger Calibrations

  Int_t fNpi0;                   // # pi0
  Int_t fNeta;                   // # eta
  Int_t fNgprime;                // # gprime
  
  Double_t* fMassDpi0;           // for meson ID by inv. mass
  Double_t* fMassDeta;           // for meson ID by inv. mass
  Int_t* fMassIJ;                // combinatorial indices
  Int_t* fMassIpi0;              // ditto
  Int_t* fMassIeta;              // ditto
  Bool_t* fIsMesonIndex;         // photon derived from a meson?
  Double_t fMaxMDpi0;            // mass-diff limit pi0
  Double_t fMaxMDeta;            // mass-diff limit eta
  
  Double_t fM2g;                 // 2-photon invariant mass
  Double_t fM6g;                 // 6-photon invariant mass

  Int_t* fDetNaI;
  Int_t* fDetPID;
  Int_t* fDetMWPC;

  Int_t fDetNaITot;
  Int_t fDetPIDTot;
  Int_t fDetMWPCTot;

  Double_t* fENaI;
  Double_t* fEPID;
  Double_t* fEMWPC;

  Int_t* fCheckNaIPID;
  Int_t* fCheckNaIMWPC;
  Int_t* fCheckPIDMWPC;
  Int_t* fCheckCharged;

  Int_t fCheckNaIPIDTot;
  Int_t fCheckNaIMWPCTot;
  Int_t fCheckPIDMWPCTot;
  Int_t fCheckChargedTot;

  Int_t fEvnID;
  Int_t* fEvnIDList;

  Int_t fRecordRaw;                // Record Raw values
  Int_t fCherenkov;                // Record Cherenkov

  Int_t* fTempHits;

  Int_t fNADCsNaI;
  Int_t* fADCsNaI;
  Int_t* fADCsRawNaI;
  Bool_t* fADCsHitNaI;

  Int_t fNTDCsNaI;
  Int_t* fTDCsNaI;
  Int_t* fTDCsRawNaI;
  Bool_t* fTDCsHitNaI;

  Int_t fNHitsNaI;
  Int_t* fHitsNaI;

  Int_t fNADCsPID;
  Int_t* fADCsPID;
  Int_t* fADCsRawPID;
  Bool_t* fADCsHitPID;

  Int_t fNTDCsPID;
  Int_t* fTDCsPID;
  Int_t* fTDCsRawPID;
  Bool_t* fTDCsHitPID;

  Int_t fNHitsPID;
  Int_t* fHitsPID;

  Int_t fNHitsMWPC;
  Int_t* fHitsMWPC;

  Int_t fNADCsBaF2;
  Int_t* fADCsBaF2;
  Int_t* fADCsRawBaF2;
  Bool_t* fADCsHitBaF2;

  Int_t fNTDCsBaF2;
  Int_t* fTDCsBaF2;
  Int_t* fTDCsRawBaF2;
  Bool_t* fTDCsHitBaF2;

  Int_t fNHitsBaF2;
  Int_t* fHitsBaF2;

  Int_t fNADCsVeto;
  Int_t* fADCsVeto;
  Int_t* fADCsRawVeto;
  Bool_t* fADCsHitVeto;

  Int_t fNTDCsVeto;
  Int_t* fTDCsVeto;
  Int_t* fTDCsRawVeto;
  Bool_t* fTDCsHitVeto;

  Int_t fNHitsVeto;
  Int_t* fHitsVeto;

  Int_t fNError;
  ReadErrorMk2_t *fErrorBlock;
  ReadErrorMk2_t *fError;
  UInt_t *fErrorHead;
  Int_t *fErrorMoID;
  Int_t *fErrorMoIn;
  Int_t *fErrorCode;
  UInt_t *fErrorTrlr;

  Int_t fNVert;
  Float_t* fVertX;
  Float_t* fVertY;
  Float_t* fVertZ;

  Bool_t* fNeutCB;                 // Neutral variables
  Int_t* fNeutCh;
  Int_t* fNeutCl;
  Float_t* fNeutEk;
  Float_t* fNeutPx;
  Float_t* fNeutPy;
  Float_t* fNeutPz;
  Float_t* fNeutTh;
  Float_t* fNeutPh;
  Float_t* fNeutTm;
  Int_t* fNeutMi;
  
  Bool_t* fCharCB;                 // Charged variables
  Bool_t* fCharPC;
  Int_t* fCharCh;
  Int_t* fCharCl;
  Int_t* fCharVI;
  Float_t* fCharVE;
  Int_t* fCharWI;
  Int_t* fCharWO;
  Float_t* fCharWE;
  Float_t* fCharUn;
  Float_t* fCharEk;
  Float_t* fCharPx;
  Float_t* fCharPy;
  Float_t* fCharPz;
  Float_t* fCharTh;
  Float_t* fCharPh;
  Float_t* fCharTm;
  
  Bool_t* fCharWC;
  Float_t* fCharVx;
  Float_t* fCharVy;
  Float_t* fCharVz;

  Float_t* fPionEk;              // Pion variables
  Float_t* fPionPx;
  Float_t* fPionPy;
  Float_t* fPionPz;
  Float_t* fPionMa;
  Float_t* fPionTh;
  Float_t* fPionPh;
  Float_t* fPionTm;
  Float_t* fPionOA;

  Float_t* fEtaEk;              // Eta variables
  Float_t* fEtaPx;
  Float_t* fEtaPy;
  Float_t* fEtaPz;
  Float_t* fEtaMa;
  Float_t* fEtaTh;
  Float_t* fEtaPh;
  Float_t* fEtaTm;
  Float_t* fEtaOA;

  Int_t* fTaggCh;                // Tagged variables
  Float_t* fTaggEk;
  Float_t* fTaggTm;
  Int_t* fTaggMH;

  //Variables required for energy loss correction

  TF1* fECorrCB;
  TF1* fECoTAPS;

  TF1** fELossCB;
  TF1** fELoTAPS;

  Bool_t fTableCB;
  Bool_t fTblTAPS;
  
 public:
  
  TA2PhysicsPMartel(const char*, TA2Analysis*);
  virtual ~TA2PhysicsPMartel();
  virtual void LoadVariable();   // variables for display/cuts
  virtual void PostInit( );      // init after parameter input
  virtual void SetConfig(Char_t*, Int_t);
  virtual void ParseMisc(char* line);     //Parses additional information from configuration file

  virtual void Reconstruct();    // reconstruct detector info
  virtual TA2DataManager* CreateChild(const char*, Int_t){return NULL;}
  virtual void Finish();

  virtual Bool_t MakeTable( TString );
  virtual void SortCharged( );
  
  virtual void Sort2Photon( );
  virtual void SortNPhoton( );
  virtual void MarkUndefined(Int_t);
  virtual void MarkEndBuffer( );
  
  Double_t Sqr(Double_t x){return (x*x);}
  
  ClassDef(TA2PhysicsPMartel,1)
    };

//-----------------------------------------------------------------------------

inline Bool_t TA2PhysicsPMartel::MakeTable(TString det)
{
    
  TString filename = "ELookup"+det+".table";
  ifstream file (filename);
  
  if ( !file ) {
    printf ("Lookup table not found\n");
    return false;
  }
  
  Int_t nX, nE;
  Double_t valX, valE, valL;
  Double_t* fitX;
  Double_t* fitY;    
  char buffer[256] ;

  Double_t max;
  
  TFile *myfile = new TFile((det+"Fits.root"),"RECREATE");
  
  file >> nX >> nE;
  
  if ( det=="CB" ) {
    // Jamie's version
    //fECorrCB = new TF1("fECorrCB","[0]+([1]*x)+([2]*x*x)+([3]*x*x*x)+([4]*x*x*x*x)+([5]*x*x*x*x*x)",0,500);
    //fECorrCB->SetParameters(1.11,-0.0384,0.000458,-0.00000263,0.00000000724,-0.00000000000772);
    //
    // My first version
    //fECorrCB = new TF1("fECorrCB","expo+[2]",0,500);
    //fECorrCB->SetParameters(0.221,-0.0362,-0.130);
    //
    // My final version
    fECorrCB = new TF1("fECorrCB","[0]+[1]/(x-[2])",0,500);
    fECorrCB->SetParameters(-45.5323,4795.06,-63.3645);
    //
    printf ("Creating CB lookup table\n");
    fELossCB = new TF1*[nX];
  }
  else if ( det=="TAPS" ) {
    // Jamie's version
    //fECoTAPS = new TF1("fECoTAPS","[0]+([1]*x)+([2]*x*x)+([3]*x*x*x)+([4]*x*x*x*x)+([5]*x*x*x*x*x)",0,500);
    //fECoTAPS->SetParameters(1.11,-0.0384,0.000458,-0.00000263,0.00000000724,-0.00000000000772);
    //
    // My first version
    //fECoTAPS = new TF1("fECoTAPS","expo+[2]",0,500);
    //fECoTAPS->SetParameters(0.221,-0.0362,-0.130);
    //
    // My final version
    fECoTAPS = new TF1("fECoTAPS","[0]+[1]/(x-[2])",0,500);
    fECoTAPS->SetParameters(-55.6124,15000,-169.646);
    //
    printf ("Creating TAPS lookup table\n");
    fELoTAPS = new TF1*[nX];
  }
  else {
    printf ("Invalid detector for lookup table\n");
    return false;
  }
  
  fitX = new Double_t[nE];
  fitY = new Double_t[nE];
  
  //Ignore second line
  file.getline(buffer,256);
  file.getline(buffer,256);
  
  //Read in the data
  for ( Int_t iX=0; iX<nX; iX++ ) {

    max = 0;
    
    for ( Int_t iE=0; iE<nE; iE++ ) {   //Read in the data   
      
      file >> valX >> valE >> valL;
      
      fitX[iE] = valE;
      fitY[iE] = valL;

      if(valL > max){
    max = valL;
      }
      
      //printf("%d\t%f\t%d\t%f\t%f\n", iX, valX, iE, valE, valL);
      
    }
    
    TGraph *loss = new TGraph(nE, fitX, fitY);
    
    //TF1 *fit1 = new TF1("fit1","expo+[2]",0,500);
    TF1 *fit1 = new TF1("fit1","[0]+[1]/(x-[2])",0,500);

    if(max > 0){
      fit1->SetParameters(4.5,3200,-46.5);
      /*
      fit1->SetParLimits(0,3.5,7);                                          
      fit1->SetParLimits(1,2000,6500);                                      
      fit1->SetParLimits(2,-80,-30);                                        
      */
      fit1->SetParLimits(0,0,30);
      fit1->SetParLimits(1,2000,15000);
      fit1->SetParLimits(2,-150,-30);
    }
    
    loss->Fit("fit1","Q","",20,260);
    
    if ( det=="CB" ) {
      fELossCB[iX] = fit1;
    }
    else if ( det=="TAPS" ) {
      fELoTAPS[iX] = fit1;
    }
    
    myfile->cd();
    loss->Write();
    fit1->Write();
    
  }
  
  myfile->Close();
  delete myfile;
  
  file.close();
  
  return true;
      
}

//-----------------------------------------------------------------------------

inline void TA2PhysicsPMartel::SortCharged()
{
  TA2Particle fChar = *fPARTchar[fNchar];

  fCheckCharged[fNchar] = 2;

  fDetNaI[fNchar] = fChar.HasDetectorA(EDetNaI);
  fDetPID[fNchar] = fChar.HasDetectorA(EDetPID);
  fDetMWPC[fNchar] = fChar.HasDetectorA(EDetMWPC);

  if ( fDetNaI[fNchar] ) {
    fENaI[fNchar] = fChar.GetT();
    fDetNaITot++;
  }
  //else fENaI[fNchar] = ENullFloat;
  else fENaI[fNchar] = -1;

  if ( fDetPID[fNchar] ) {
    fEPID[fNchar] = fChar.GetVetoEnergy();
    fDetPIDTot++;
  }
  //else fEPID[fNchar] = ENullFloat;
  else fEPID[fNchar] = -1;

  if ( fDetMWPC[fNchar] ) {
    fEMWPC[fNchar] = 0;
    //fEMWPC[fNchar] = fChar.GetEtrackMwpc();
    fDetMWPCTot++;
  }
  //else fEMWPC[fNchar] = ENullFloat;
  else fEMWPC[fNchar] = -1;

  fCanCutNaIPID = fIsCutNaIPID && fDetNaI[fNchar] && fDetPID[fNchar];
  fCanCutNaIMWPC = fIsCutNaIMWPC && fDetNaI[fNchar] && fDetMWPC[fNchar];
  fCanCutPIDMWPC = fIsCutPIDMWPC && fDetPID[fNchar] && fDetMWPC[fNchar];

  if ( fCanCutNaIPID ) {
    fCheckNaIPID[fNchar] = fCutNaIPID->IsInside(fENaI[fNchar], fEPID[fNchar]);
    //fCheckCharged[fNchar] = fCheckNaIPID[fNchar];
  }
  else fCheckNaIPID[fNchar] = 2;

  if ( fCanCutNaIMWPC ) {
    fCheckNaIMWPC[fNchar] = fCutNaIMWPC->IsInside(fENaI[fNchar], fEMWPC[fNchar]);
    //fCheckCharged[fNchar] = fCheckNaIMWPC[fNchar];
  }
  else fCheckNaIMWPC[fNchar] = 2;

  if ( fCanCutPIDMWPC ) {
    fCheckPIDMWPC[fNchar] = fCutPIDMWPC->IsInside(fEPID[fNchar], fEMWPC[fNchar]);
    //fCheckCharged[fNchar] = fCheckPIDMWPC[fNchar];
  }
  else fCheckPIDMWPC[fNchar] = 2;
  /*  
  if ( ( fCanCutNaIPID && !fCheckNaIPID[fNchar] ) ||
       ( fCanCutNaIMWPC && !fCheckNaIMWPC[fNchar] ) ||
       ( fCanCutPIDMWPC && !fCheckPIDMWPC[fNchar] ) )
    fCheckCharged[fNchar] = false;
  */
  if ( ( fCanCutNaIPID && fCheckNaIPID[fNchar] ) ||
       ( fCanCutNaIMWPC && fCheckNaIMWPC[fNchar] ) ||
       ( fCanCutPIDMWPC && fCheckPIDMWPC[fNchar] ) )
    fCheckCharged[fNchar] = true;
  
  if ( fCheckNaIPID[fNchar] == 1 ) fCheckNaIPIDTot++;
  if ( fCheckNaIMWPC[fNchar] == 1 ) fCheckNaIMWPCTot++;
  if ( fCheckPIDMWPC[fNchar] == 1 ) fCheckPIDMWPCTot++;
  if ( fCheckCharged[fNchar] == 1 ) {
    fCheckChargedTot++;
    fCharPC[fNchar] = true;
  }
  else fCharPC[fNchar] = false;

  /*
  FILE *outfile = fopen("Charged.txt","a");
  fprintf(outfile,"%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",fNchar,fDetNaI[fNchar],fDetPID[fNchar],fDetMWPC[fNchar],fCanCutNaIPID,fCanCutNaIMWPC,fCanCutPIDMWPC,fCheckNaIPID[fNchar],fCheckNaIMWPC[fNchar],fCheckPIDMWPC[fNchar],fCheckCharged[fNchar]);
  fclose(outfile);
  */
}

//-----------------------------------------------------------------------------

inline void TA2PhysicsPMartel::Sort2Photon()
{
  // Test if 2 gamma 4-momenta combine to give a  possible pi0 or eta
  // by finding the invariant mass. This is a fast version of SortNPhoton
  // for the frequent pi0 or eta -> 2 gamma situation
  
  TA2Particle phot1 = *fPARTneut[0];
  TA2Particle phot2 = *fPARTneut[1];
  
  Double_t time = (phot1.GetTime() + phot2.GetTime())/2;
  
  TLorentzVector p4 = phot1.GetP4() + phot2.GetP4();   // sum 4 momenta
  
  fM2g = p4.M();				       // inv. mass
  
  fMassDpi0[0] = TMath::Abs( p4.M() - fParticleID->GetMassMeV( kPi0));
  fMassDeta[0] = TMath::Abs( p4.M() - fParticleID->GetMassMeV( kEta));
  
  // is it a pi0
  if ( fMassDpi0[0] < fMaxMDpi0 ) {
  //printf("%6.1f  %6.1f  %6.1f  %6.1f\n", fMaxMDpi0, fMassDpi0[0], fM2g, time);
    (*fPARTpi0[0]).SetP4( p4);
    (*fPARTpi0[0]).SetTime( time);
    fNpi0 = 1;
    fIsMesonIndex[0] = ETrue;
    fIsMesonIndex[1] = ETrue;
    fNeutMi[0] = fNeutMi[1] = 1;
  }
  // wasn't a pi0 so test for eta
  else if ( fMassDeta[0] < fMaxMDeta ) {
    (*fPARTeta[0]).SetP4( p4);
    (*fPARTeta[0]).SetTime( time);
    fNeta = 1;
    fIsMesonIndex[0] = ETrue;
    fIsMesonIndex[1] = ETrue;
    fNeutMi[0] = fNeutMi[1] = 1;
  }
  // not an eta so assume both photons are gamma-prime
  else {
    fPARTgprime[0] = fPARTneut[0];
    fPARTgprime[1] = fPARTneut[1];
    fNgprime = 2;
    fNeutMi[0] = fNeutMi[1] = 0;
  }
  return;
}

//-----------------------------------------------------------------------------

inline void TA2PhysicsPMartel::SortNPhoton()
{
  // Take sample of n gamma 4-momenta and sort into possible pi0 or eta
  // by finding the invariant mass of the possible 2-photon combinations.
  // The differences of the 2-photon invariant masses and pi0/eta mass are
  // sorted in ascending order.
  
  TLorentzVector p4temp, q4;
  Double_t mPi0 = fParticleID->GetMassMeV( kPi0);
  Double_t mEta = fParticleID->GetMassMeV( kEta);
  Double_t* pi0diff = fMassDpi0;
  Double_t* etadiff = fMassDeta;
  Int_t* ij = fMassIJ;
  Int_t i,j,k,jk;
  Int_t n = fNneut;
  fP4photonTot.SetXYZT(0.0,0.0,0.0,0.0);         // zero total photon 4-mom
  
  Double_t time;
  
  // Loop over possible 2-photon combinations i != j, ij = ji
  for ( i = 0, k = 0; i < n; i++ ) {
    
    fP4photonTot += (*fPARTneut[i]).GetP4();
    fIsMesonIndex[i] = EFalse;                   // initialise not meson
    fNeutMi[i] = 0;
    
    for ( j = i + 1; j < n; j++ ) {
      
      // add the 4 momenta
      p4temp =  (*fPARTneut[i]).GetP4() + (*fPARTneut[j]).GetP4();
      *pi0diff++ = TMath::Abs( p4temp.M() - mPi0 ); // check inv mass pi0
      *etadiff++ = TMath::Abs( p4temp.M() - mEta ); // check inv. mass eta
      *ij++ = i | (j<<16);                       // store i,j indices
      k++;                                       // permutation counter
      
    }
  }
  
  if ( n == 6 ) fM6g = fP4photonTot.M();
  TMath::Sort(k, fMassDeta, fMassIeta, EFalse);  // sort mass diffs ascending
  TMath::Sort(k, fMassDpi0, fMassIpi0, EFalse);  // sort mass diffs ascending
  Int_t nMeson = k;                              // max # possible mesons
  
  // Check for eta combinations
  for ( i = 0; i < nMeson; i++) {
    
    // exit loop when mass-diff exceeds maximum bound
    if ( fMassDeta[fMassIeta[i]] > fMaxMDeta ) break;
    
    // get indices
    jk = fMassIJ[fMassIeta[i]];
    j = jk & 0xffff;
    k = (jk>>16) & 0xffff;
    
    // photon already used to construct meson ?
    if ( fIsMesonIndex[j] || fIsMesonIndex[k] ) continue;
    
    // set 4-momentum
    q4 = (*fPARTneut[j]).GetP4() + (*fPARTneut[k]).GetP4();
    (*fPARTeta[fNeta]).SetP4( q4);
    
    // set time
    time = ((*fPARTneut[j]).GetTime() + (*fPARTneut[k]).GetTime())/2;
    (*fPARTeta[fNeta++]).SetTime( time);
    
    // mark photons as used
    fIsMesonIndex[j] = fIsMesonIndex[k] = ETrue;
    fNeutMi[j] = fNeutMi[k] = fNeta;
  }
  
  // Check for pi0 combinations
  for ( i = 0; i < nMeson; i++) {
    
    // exit loop when mass-diff exceeds maximum bound
    if ( fMassDpi0[fMassIpi0[i]] > fMaxMDpi0 ) break;
    
    // get indices
    jk = fMassIJ[fMassIpi0[i]];
    j = jk & 0xffff;
    k = (jk>>16) & 0xffff;
    
    // photon already used to construct meson?
    if ( fIsMesonIndex[j] || fIsMesonIndex[k] ) continue;
    
    // set 4-momentum
    q4 = (*fPARTneut[j]).GetP4() + (*fPARTneut[k]).GetP4();
    (*fPARTpi0[fNpi0]).SetP4( q4);
    
    // set time
    time = ((*fPARTneut[j]).GetTime() + (*fPARTneut[k]).GetTime())/2;
    (*fPARTpi0[fNpi0++]).SetTime( time);
    
    // mark photons as used
    fIsMesonIndex[j] = fIsMesonIndex[k] = ETrue;
    fNeutMi[j] = fNeutMi[k] = fNpi0+fNeta;
  }
  
  // Put any photons not combined into a meson into the gamma-primed list
  for ( j = 0; j < n; j++ ) {
    if ( !fIsMesonIndex[j] ) fPARTgprime[fNgprime++] = fPARTneut[j];
  }
  
}

//-----------------------------------------------------------------------------

inline void TA2PhysicsPMartel::MarkUndefined( Int_t jtagg )
{
  // Initialise undefined
  //fTaggCh[jtagg] = EPhotoUndefined;
  //fTaggEk[jtagg] = EPhotoUndefined;
  //fTaggTm[jtagg] = EPhotoUndefined;
  //fTaggMH[jtagg] = EPhotoUndefined;
}

//-----------------------------------------------------------------------------

inline void TA2PhysicsPMartel::MarkEndBuffer()
{
  // Ensure the multi-data buffers are marked as ended
  fNeutCB[fNneut] = EBufferEnd;
  fNeutCh[fNneut] = EBufferEnd;
  fNeutCl[fNneut] = EBufferEnd;
  fNeutEk[fNneut] = EBufferEnd;
  fNeutPx[fNneut] = EBufferEnd;
  fNeutPy[fNneut] = EBufferEnd;
  fNeutPz[fNneut] = EBufferEnd;
  fNeutTh[fNneut] = EBufferEnd;
  fNeutPh[fNneut] = EBufferEnd;
  fNeutTm[fNneut] = EBufferEnd;
  fNeutMi[fNneut] = EBufferEnd;
  
  fCharCB[fNchar] = EBufferEnd;
  fCharPC[fNchar] = EBufferEnd;
  fCharCh[fNchar] = EBufferEnd;
  fCharCl[fNchar] = EBufferEnd;
  fCharVI[fNchar] = EBufferEnd;
  fCharVE[fNchar] = EBufferEnd;
  fCharWI[fNchar] = EBufferEnd;
  fCharWO[fNchar] = EBufferEnd;
  fCharWE[fNchar] = EBufferEnd;
  fCharUn[fNchar] = EBufferEnd;
  fCharEk[fNchar] = EBufferEnd;
  fCharPx[fNchar] = EBufferEnd;
  fCharPy[fNchar] = EBufferEnd;
  fCharPz[fNchar] = EBufferEnd;
  fCharTh[fNchar] = EBufferEnd;
  fCharPh[fNchar] = EBufferEnd;
  fCharTm[fNchar] = EBufferEnd;

  fCharWC[fNchar] = EBufferEnd;
  fCharVx[fNchar] = EBufferEnd;
  fCharVy[fNchar] = EBufferEnd;
  fCharVz[fNchar] = EBufferEnd;

  fPionEk[fNpi0] = EBufferEnd;
  fPionPx[fNpi0] = EBufferEnd;
  fPionPy[fNpi0] = EBufferEnd;
  fPionPz[fNpi0] = EBufferEnd;
  fPionMa[fNpi0] = EBufferEnd;
  fPionTh[fNpi0] = EBufferEnd;
  fPionPh[fNpi0] = EBufferEnd;
  fPionTm[fNpi0] = EBufferEnd;
  fPionOA[fNpi0] = EBufferEnd;

  fEtaEk[fNeta] = EBufferEnd;
  fEtaPx[fNeta] = EBufferEnd;
  fEtaPy[fNeta] = EBufferEnd;
  fEtaPz[fNeta] = EBufferEnd;
  fEtaMa[fNeta] = EBufferEnd;
  fEtaTh[fNeta] = EBufferEnd;
  fEtaPh[fNeta] = EBufferEnd;
  fEtaTm[fNeta] = EBufferEnd;
  fEtaOA[fNeta] = EBufferEnd;

  fTaggCh[fNtagg] = EBufferEnd;
  fTaggEk[fNtagg] = EBufferEnd;
  fTaggTm[fNtagg] = EBufferEnd;
  fTaggMH[fNmult] = EBufferEnd;
  
  fDetNaI[fNchar] = EBufferEnd;
  fDetPID[fNchar] = EBufferEnd;
  fDetMWPC[fNchar] = EBufferEnd;

  fENaI[fNchar] = EBufferEnd;
  fEPID[fNchar] = EBufferEnd;
  fEMWPC[fNchar] = EBufferEnd;

  fCheckNaIPID[fNchar] = EBufferEnd;
  fCheckNaIMWPC[fNchar] = EBufferEnd;
  fCheckPIDMWPC[fNchar] = EBufferEnd;
  fCheckCharged[fNchar] = EBufferEnd;
  
}

#endif
