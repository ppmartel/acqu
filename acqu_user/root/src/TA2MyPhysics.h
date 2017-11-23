/*************************************************************************
 * Authors: Dominik Werthmueller, 2008-2014
 *          Thomas Strub, 2013
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TA2MyPhysics                                                         //
//                                                                      //
// This is an abstract base physics analysis class.                     //
// Classes inheriting from this class must implement the LoadVariable() //
// and the ReconstructPhysics() method.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef __TA2MyPhysics_h__
#define __TA2MyPhysics_h__


#include "TOLoader.h"
#include "TOGlobals.h"
#include "TOMCParticle.h"
#include "TOA2RecParticle.h"
#include "TOA2Trigger.h"
#include "TOKinCut.h"
#include "TOA2PSA.h"
#include "TONumberReader.h"
#include "TOA2DetParticle.h"
#include "TOArray2D.h"

#include "TNtupleD.h"
#include "TChain.h"
#include "TF1.h"
#include "THnSparse.h"

#include "TA2AccessSQL.h"

enum {
    EMP_CALIB_BADSCR = 20000,
    EMP_RUN_NUMBER,
    EMP_BAD_TAGG_CH,
    EMP_BAD_CB_CLUSTERS,
    EMP_BAD_TAPS_CLUSTERS,
    EMP_BAD_PARTICLE_CUTS,
    EMP_BAD_PARTICLE_CUT_PHI,
    EMP_TRIGGER_TOTAL_MULT,
    EMP_TRIGGER_TOTAL_MULT_NO_TAPS,
    EMP_TRIGGER_TAPS_MULT,
    EMP_TRIGGER_CB_THRESHOLDS,
    EMP_TRIGGER_CB_REL_GAINS,
    EMP_TRIGGER_CB_SUM_CDF,
    EMP_TRIGGER_TAPS_LED1_THRESHOLD,
    EMP_TRIGGER_TAPS_LED2_THRESHOLD,
    EMP_TRIGGER_TAPS_EXCLUDED_RINGS,
    EMP_CB_PID_PHI_LIMIT,
    EMP_BEAM_POL_BITS,
    EMP_BEAM_POL_ADC,
    EMP_L1_BITS,
    EMP_L1_CBSUM_L2,
    EMP_L2_MULTI_BITS,
    EMP_PID_DROOP_CORR,
    EMP_LAST_EVENT,
    EMP_TAPS_PSA_DATA,
    EMP_TAPS_PSA_CONFIG,
    EMP_TAGGER_PROMPT,
    EMP_PROTON_ECORR_CB,
    EMP_PROTON_ECORR_TAPS,
    EMP_PHOTON_RES_CB,
    EMP_PHOTON_RES_TAPS,
    EMP_PARTICLE_RES,
    EMP_CB_TAPS_ENERGY_SCALE,
    EMP_PID_DE_E_CUTS
};


static const Int_t gMyPhysics_MaxTaggerCh = 352;
static const Int_t gMyPhysics_MaxNaI      = 720;
static const Int_t gMyPhysics_MaxPID      =  24;
static const Int_t gMyPhysics_MaxBaF2PWO  = 438;
static const Int_t gMyPhysics_MaxTAPSRing =  11;
static const Int_t gMyPhysics_MaxMCPart   =  30;


static const Map_t myPhysicsConfigKeys[] = {
    // General keys
    {"Use-CaLib-BadScR:"             , EMP_CALIB_BADSCR},                   // key for CaLib bad scaler read configuration
    {"Run-Number:"                   , EMP_RUN_NUMBER},                     // key for run number overwriting
    {"Bad-Tagger-Channels:"          , EMP_BAD_TAGG_CH},                    // key for bad tagger channel read-in
    {"Bad-CB-Clusters:"              , EMP_BAD_CB_CLUSTERS},                // key for the bad CB cluster centers
    {"Bad-TAPS-Clusters:"            , EMP_BAD_TAPS_CLUSTERS},              // key for the bad TAPS cluster centers
    {"Bad-Particle-Cuts:"            , EMP_BAD_PARTICLE_CUTS},              // key for bad particle cuts
    {"Bad-Particle-Cut-Phi:"         , EMP_BAD_PARTICLE_CUT_PHI},           // key for bad particle phi cut
    {"Trigger-Total-Mult:"           , EMP_TRIGGER_TOTAL_MULT},             // key for the trigger total multiplicity
    {"Trigger-Total-Mult-No-TAPS:"   , EMP_TRIGGER_TOTAL_MULT_NO_TAPS},     // key for the exclusion of TAPS to the trigger total multiplicity
    {"Trigger-TAPS-Mult:"            , EMP_TRIGGER_TAPS_MULT},              // key for the trigger TAPS multiplicity
    {"Trigger-CB-Thresholds:"        , EMP_TRIGGER_CB_THRESHOLDS},          // key for the trigger CB sum and segment thresholds
    {"Trigger-CB-Rel-Gains:"         , EMP_TRIGGER_CB_REL_GAINS},           // key for the relative gains used in the CB sum (MC)
    {"Trigger-CB-Sum-CDF:"           , EMP_TRIGGER_CB_SUM_CDF},             // key for the trigger CB sum CDF
    {"Trigger-TAPS-LED1-Threshold:"  , EMP_TRIGGER_TAPS_LED1_THRESHOLD},    // key for the trigger TAPS LED1 threshold
    {"Trigger-TAPS-LED2-Threshold:"  , EMP_TRIGGER_TAPS_LED2_THRESHOLD},    // key for the trigger TAPS LED2 threshold
    {"Trigger-TAPS-Excluded-Rings:"  , EMP_TRIGGER_TAPS_EXCLUDED_RINGS},    // key for the trigger-excluded TAPS rings
    {"CB-PID-Phi-Limit:"             , EMP_CB_PID_PHI_LIMIT},               // key for the CB-PID coincidence phi limit
    {"Beam-Pol-Bits:"                , EMP_BEAM_POL_BITS},                  // key for the beam helicity bit definition
    {"Beam-Pol-ADC:"                 , EMP_BEAM_POL_ADC},                   // key for the beam helicity ADC definition
    {"L1-Bits:"                      , EMP_L1_BITS},                        // key for the L1 trigger bits
    {"L1-CBSum-in-L2:"               , EMP_L1_CBSUM_L2},                    // key for the CB sum L1->L2 trigger toggle
    {"L2-Multi-Bits:"                , EMP_L2_MULTI_BITS},                  // key for the L2 trigger multiplicity bits
    {"PID-Droop-Corr:"               , EMP_PID_DROOP_CORR},                 // key for PID droop correction
    {"Last-Event:"                   , EMP_LAST_EVENT},                     // key for the last event
    {"TAPS-PSA-Data:"                , EMP_TAPS_PSA_DATA},                  // key for TAPS PSA data file
    {"TAPS-PSA-Config:"              , EMP_TAPS_PSA_CONFIG},                // key for TAPS PSA configuration
    {"Tagger-Prompt:"                , EMP_TAGGER_PROMPT},                  // key for Tagger prompt window configuration
    {"Proton-Energy-Corr-CB:"        , EMP_PROTON_ECORR_CB},                // key for CB proton energy correction
    {"Proton-Energy-Corr-TAPS:"      , EMP_PROTON_ECORR_TAPS},              // key for TAPS proton energy correction
    {"Photon-Resolution-CB:"         , EMP_PHOTON_RES_CB},                  // key for CB photon resolution
    {"Photon-Resolution-TAPS:"       , EMP_PHOTON_RES_TAPS},                // key for TAPS photon resolution
    {"Particle-Resolutions:"         , EMP_PARTICLE_RES},                   // key for particle resolutions
    {"CB-TAPS-Energy-Scale:"         , EMP_CB_TAPS_ENERGY_SCALE},           // key for CB and TAPS energy scaling
    {"PID-dE-E-Cuts:"                , EMP_PID_DE_E_CUTS},                  // key for PID dE-E cuts
    // Termination
    {NULL        , -1           }
};


class TA2MyPhysics : public TA2AccessSQL
{

private:
    // ----------------------------------- General ----------------------------------- 
    Long64_t fLastEvent;                                    // last event to analyze
    TH1* fH_EventInfo;                                      // general event information
    TH2* fH_ErrorInfo;                                      // hardware error information (error code vs module index)
    TH1* fH_Corrected_Scalers;                              // overflow corrected current scalers
    TH1* fH_Corrected_SumScalers;                           // overflow corrected accumulated scalers
    TH1* fH_MCVertX;                                        // MC vertex x-coordinate
    TH1* fH_MCVertY;                                        // MC vertex y-coordinate
    TH1* fH_MCVertZ;                                        // MC vertex z-coordinate
    
    // -------------------------------- Bad detectors -------------------------------- 
    Int_t fNBadTaggerChannels;                              // number of bad tagger channels
    Int_t* fBadTaggerChannels;                              // list of bad tagger channels
    Int_t fNBadCBClusters;                                  // number of bad CB cluster centers
    Int_t* fBadCBClusters;                                  // list of bad CB cluster centers
    Int_t fNBadTAPSClusters;                                // number of bad TAPS cluster centers
    Int_t* fBadTAPSClusters;                                // list of bad TAPS cluster centers
    Int_t fNBadParticleCuts;                                // number of bad particle cuts
    TOKinCut** fBadParticleCuts;                            // list of bad particle cuts

    // ----------------------------------- Scalers ----------------------------------- 
    Double_t* fOldScalerSum;                                // used to avoid double sum scaler addition
    
    // ------------------------------- Bad scaler reads ------------------------------ 
    Int_t fNBadScalerReads;                                 // number of bad scaler reads
    Int_t* fBadScalerReads;                                 // list of bad scaler reads
    Bool_t fIsBadScalerSkip;                                // flag for skipping events
    TH1* fH_BadScR_SumScalers;                              // sum scalers w/o bad scaler reads
    TH1* fH_BadScR_SumFPDScalers;                           // FPD sum scalers w/o bad scaler reads

    // ------------------------------------- PID ------------------------------------- 
    TH1** fH_CB_PID_Coinc_Hits;                             // CB-PID coincidence hits
    TH2** fH_CB_PID_dE_E;                                   // CB-PID dE vs. E plots
    Double_t fCBPIDPhiLimit;                                // CB-PID coincidence phi limit
    TGraph** fPIDDroopCorr;                                 // array of PID droop correction graphs
    TF1** fPIDdEECuts;                                      // array of PID dE-E cuts

    // ------------------------------------- PSA ------------------------------------- 
    TOA2PSA* fPSA;                                          // TAPS pulse-shape analysis

    // ----------------------------- Random subtraction ------------------------------ 
    TGraph* fPrompt_Min_CB;                                 // graph of minima of CB-Tagger prompt windows
    TGraph* fPrompt_Max_CB;                                 // graph of maxima of CB-Tagger prompt windows
    TGraph* fPrompt_Min_TAPS;                               // graph of minima of TAPS-Tagger prompt windows
    TGraph* fPrompt_Max_TAPS;                               // graph of maxima of TAPS-Tagger prompt windows
    
    // -------------------------- Proton energy correction --------------------------- 
    TGraph** fProtonECorrCB;                                // proton energy correction factors for CB
    TGraph** fProtonECorrTAPS;                              // proton energy correction factors for TAPS
 
    // ----------------------------- Photon resolutions ------------------------------ 
    TGraph* fPhotonResThetaCB;                              // photon theta resolution for CB
    TGraph* fPhotonResPhiCB;                                // photon phi resolution for CB
    TGraph* fPhotonResEnergyCB;                             // photon energy resolution for CB
    TGraph* fPhotonResThetaTAPS;                            // photon theta resolution for TAPS 
    TGraph* fPhotonResPhiTAPS;                              // photon phi resolution for TAPS 
    TGraph* fPhotonResEnergyTAPS;                           // photon energy resolution for TAPS
    
    // ------------------------------- Software trigger  ----------------------------- 
    Int_t fTrigTotalMult;                                   // total multiplicity
    Bool_t fTrigIsTAPSTotalMult;                            // toggle for the inclusion of TAPS to the total multiplicity
    Int_t fTrigTAPSMult;                                    // TAPS alone multiplicity
    Double_t fTrigCBSum;                                    // CB energy sum
    Double_t fTrigCBMeanGain;                               // CB NaI mean gain for relative de-calibration
    Double_t* fTrigCBRelGains;                              // CB NaI relative gains for MC de-calibration
    TF1* fTrigCBSumCDF;                                     // CB energy sum CDF
    Double_t fTrigCBSegThr;                                 // CB NaI segment high discriminator threshold
    Double_t fTrigTAPSLED1Thr;                              // TAPS LED1 thresholds
    Double_t fTrigTAPSLED2Thr;                              // TAPS LED2 thresholds
    Int_t fNTrigTAPSExclRings;                              // number of trigger-excluded TAPS rings
    Int_t* fTrigTAPSExclRings;                              // list of trigger-excluded TAPS rings

    // ------------------------ Utility methods for this class ----------------------- 
    void LoadDetectors(TA2DataManager* parent, Int_t depth); 
    void UpdateCorrectedScaler(Int_t sc);
    Int_t GetPatternIndex(const Char_t* patternName, TA2BitPattern* pattern);
    void ApplyCaLib();
    Double_t CalculateMeanCBGain();
    void CalculatePhotonResolutions(TOA2DetParticle* p);
    void TerminateAnalysis();

protected:
    // ----------------------------------- General ----------------------------------- 
    TFile* fOutputFile;                                     // output file for direct tree writing
    Double_t fMCVertX;                                      // MC vertex x-coordinate
    Double_t fMCVertY;                                      // MC vertex y-coordinate
    Double_t fMCVertZ;                                      // MC vertex z-coordinate
    TBranch* fMCBranchDirCos;                               // MC branch of direction cosines
    TBranch* fMCBranchVert;                                 // MC branch of vertices
    Long64_t fEventCounter;                                 // event counter
    Long64_t fEventOffset;                                  // event offset when analyzing multiple files
    Int_t fSaveEvent;                                       // if 1 : save current event in reduced AcquRoot ROOT file
    Int_t fScalerReadCounter;                               // number of processed scaler reads
    Bool_t fUseBadScalerReads;                              // key to activate bad scaler reads

    // ----------------------------------- Tagger ----------------------------------- 
    UInt_t fTaggerPhotonNhits;                              // number of photons in the tagger
    Int_t* fTaggerPhotonHits;                               // pointer to the Tagger photon hits
    Double_t* fTaggerPhotonEnergy;                          // pointer to the Tagger photon energy array
    Double_t* fTaggerPhotonTime;                            // pointer to the Tagger photon time array

    // ------------------------------------- CB ------------------------------------- 
    UInt_t fNaINhits;                                       // number of NaI hits
    Int_t* fNaIHits;                                        // pointer to the NaI hits
    Double_t* fNaIEnergy;                                   // pointer to the NaI energy array
    Double_t* fNaITime;                                     // pointer to the NaI time array
    Double_t* fCBClTime;                                    // pointer to the CB cluster time
    TA2BitPattern* fCBPattern;                              // pointer to the CB bit pattern
    Int_t fNaILEDPattern;                                   // index of the NaI LED pattern in the CB bit pattern
    UInt_t fNaILEDNhits;                                    // number of LED hits in the NaI
    Int_t* fNaILEDHits;                                     // pointer to the NaI LED hits
//    Int_t* fCBPID_Index;                                    // pointer to the PID index of the CB cluster
//    Double_t* fCBPID_dE;                                    // pointer to the PID energy deposition of the CB cluster
    UInt_t* fCBClusterHit;                                  // pointer to the CB cluster hit indices
    UInt_t fCBNCluster;                                     // number of clusters in CB
    HitCluster_t** fCBCluster;                              // pointer to the list of CB clusters
    Double_t fCBEnergyScale;                                // NaI energy scaling

    // ------------------------------------- PID ------------------------------------- 
    UInt_t fPIDNhits;                                       // number of hits in the PID
    Int_t* fPIDHits;                                        // pointer to the PID hits
    Double_t* fPIDEnergy;                                   // pointer to the PID energy array
    Double_t* fPIDTime;                                     // pointer to the PID time array
    TVector3** fPIDHitPos;                                  // pointer to the PID hit position list

    // ------------------------------------ MWPC ------------------------------------ 
    Int_t fMWPCNtracks;                                     // number of tracks in the MWPC
    const TA2MwpcTrack* fMWPCtracks;                        // list of tracks in the MWPC    

    // ------------------------------------ TAPS ------------------------------------ 
    A2TAPSType_t fTAPSType;                                 // TAPS type
    UInt_t fBaF2PWONhits;                                   // number of BaF2/PWO hits
    Int_t* fBaF2PWOHits;                                    // pointer to the BaF2/PWO hits
    Double_t* fBaF2PWOEnergy;                               // pointer the the BaF2/PWO energy array
    Double_t* fBaF2PWOTime;                                 // pointer to the BaF2/PWO time
    TA2BitPattern* fTAPSPattern;                            // pointer to the TAPS bit pattern
    Int_t fBaF2CFDPattern;                                  // index of the BaF2 CFD pattern in the TAPS bit pattern
    Int_t fBaF2LED1Pattern;                                 // index of the BaF2 LED1 pattern in the TAPS bit pattern
    Int_t fBaF2LED2Pattern;                                 // index of the BaF2 LED2 pattern in the TAPS bit pattern
    Int_t fVetoLEDPattern;                                  // index of the Veto LED pattern in the TAPS bit pattern
    UInt_t fBaF2CFDNhits;                                   // number of CFD hits in the BaF2
    Int_t* fBaF2CFDHits;                                    // pointer to the BaF2 CFD hits
    UInt_t fBaF2LED1Nhits;                                  // number of LED1 hits in the BaF2
    Int_t* fBaF2LED1Hits;                                   // pointer to the BaF2 LED1 hits
    UInt_t fBaF2LED2Nhits;                                  // number of LED2 hits in the BaF2
    Int_t* fBaF2LED2Hits;                                   // pointer to the BaF2 LED2 hits
    UInt_t fVetoLEDNhits;                                   // number of LED hits in the Veto
    Int_t* fVetoLEDHits;                                    // pointer to the Veto LED hits
    UInt_t* fTAPSClusterHit;                                // pointer to the TAPS cluster hit indices
    UInt_t fTAPSNCluster;                                   // number of clusters in TAPS
    HitCluster_t** fTAPSCluster;                            // pointer to the list of TAPS clusters
    Double_t fTAPSEnergyScale;                              // BaF2PWO energy scaling
    
    // ------------------------------------ Veto ------------------------------------ 
    UInt_t fVetoNhits;                                      // number of Veto hits
    Int_t* fVetoHits;                                       // pointer to the Veto hits
    Double_t* fVetoEnergy;                                  // pointer to the Veto energy array
    Double_t* fVetoTime;                                    // pointer to the Veto time array
    
    // ------------------------------------ PbWO4 ----------------------------------- 
    UInt_t fPbWO4Nhits;                                     // number of PbWO4 hits
    Int_t* fPbWO4Hits;                                      // pointer to the PbWO4 hits array
    Double_t* fPbWO4Energy;                                 // pointer to the PbWO4 energy array
    Double_t* fPbWO4Time;                                   // pointer to the PbWO4 time array

    // -------------------------------- Polarisation -------------------------------- 
    UShort_t* fCP;                                          // Circular polarisation: pointer to the electron beam helicity state
    Int_t fEBeamBitPos;                                     // positive helicity bit value
    Int_t fEBeamBitNeg;                                     // negative helicity bit value
    Int_t fEBeamBitADC;                                     // helicity bit ADC

    // ------------------------------------ TOF ------------------------------------- 
    UInt_t fTOFBarNhits;                                    // number of TOF bar hits
    Int_t* fTOFBarHits;                                     // pointer to the TOF bar hits
    Double_t* fTOFBarMeanEnergy;                            // pointer to the TOF bar mean energy
    Double_t* fTOFBarMeanTime;                              // pointer to the TOF bar mean time
  
    // ----------------------------- Pb glass detector ------------------------------ 
    UInt_t fPbGlassNhits;                                   // number of hits in the Pb glass detector
    Double_t* fPbGlassTime;                                 // pointer to the Pb glass detector time
    
    // ----------------------------- Particle collection ---------------------------- 
    Int_t fNTotal;                                          // total number of detected particles
    Int_t fNNeutral;                                        // number of detected neutral particles
    Int_t fNCharged;                                        // number of detected charged particles
    TOA2DetParticle** fPart;                                // array of detected particles
    TOA2DetParticle** fPartCB;                              // array of particles detected in CB
    TOA2DetParticle** fPartTAPS;                            // array of particles detected in TAPS
    TOA2DetParticle** fPartNeutral;                         // array of detected neutral particles
    TOA2DetParticle** fPartCharged;                         // array of detected charged particles

    // ---------------------------- MC Particle collection --------------------------- 
    Int_t fNMC;                                             // number of MC particles
    TOMCParticle* fPartMC[gMyPhysics_MaxMCPart];            // array of MC particles
    
    // ------------------------------- Hardware trigger  ----------------------------- 
    TOA2Trigger* fTrigger;                                  // A2 software and hardware trigger
    Int_t fL1Bit_CB;                                        // Level 1 CB sum trigger bit
    Int_t fL1Bit_TAPS_OR;                                   // Level 1 TAPS LED1 OR trigger bit
    Int_t fL1Bit_TAPS_M2;                                   // Level 1 TAPS LED2 M2+ trigger bit
    Int_t fL1Bit_Pulser;                                    // Level 1 CB pulser trigger bit
    Int_t fL1Bit_TAPS_Pulser;                               // Level 1 TAPS pulser trigger bit
    Bool_t fCBSumInL2;                                      // toggle for the CB sum L1->L2 migration
    Int_t fL2Bit_M1;                                        // Level 2 M1+ trigger bit
    Int_t fL2Bit_M2;                                        // Level 2 M2+ trigger bit
    Int_t fL2Bit_M3;                                        // Level 2 M3+ trigger bit
    Int_t fL2Bit_M4;                                        // Level 2 M4+ trigger bit
    
    // -------------------- Utility methods for analysis classes -------------------- 

    Bool_t IsBadScalerRead(Int_t scr);
    Bool_t IsBadTaggerChannel(Int_t ch);
    Bool_t IsBadCBCluster(Int_t center);
    Bool_t IsBadTAPSCluster(Int_t center);
    Bool_t IsTrigExclTAPSRing(Int_t ring);
    Bool_t IsTrigger(TOA2RecParticle& mesons);
    Double_t GetTaggEff(Int_t ch);
    void ClearCBTimeWalk();
    Double_t GetBGSubtractionWeight(TOA2RecParticle& meson,
                                    Double_t taggerTime,
                                    Double_t limitPromptLowCB, Double_t limitPromptHighCB,
                                    Double_t limitBG1LowCB, Double_t limitBG1HighCB,
                                    Double_t limitBG2LowCB, Double_t limitBG2HighCB,
                                    Double_t limitPromptLowTAPS, Double_t limitPromptHighTAPS,
                                    Double_t limitBG1LowTAPS, Double_t limitBG1HighTAPS,
                                    Double_t limitBG2LowTAPS, Double_t limitBG2HighTAPS,
                                    Bool_t* outIsOutside = 0);
    Double_t GetBGSubtractionWeight(TOA2RecParticle& meson,
                                    Int_t taggerChannel,
                                    Double_t taggerTime,
                                    Double_t limitBG1LowCB, Double_t limitBG1HighCB,
                                    Double_t limitBG2LowCB, Double_t limitBG2HighCB,
                                    Double_t limitBG1LowTAPS, Double_t limitBG1HighTAPS,
                                    Double_t limitBG2LowTAPS, Double_t limitBG2HighTAPS,
                                    Bool_t* outIsOutside = 0);
    Double_t GetRandomTaggerTime(TOA2RecParticle& meson, Int_t channel, 
                                 Double_t subtr_weight, Double_t taggerTime);
    Double_t CheckClusterVeto(TOA2DetParticle* inCluster, Int_t* outVetoIndex = 0) const;
    Double_t CheckClusterPID(TOA2DetParticle* inCluster, Int_t* outPIDIndex = 0) const;
    Bool_t CheckPSA(TOA2DetParticle* p, TCutG* c);
    Int_t GetForeignVetoHits(TOA2DetParticle* p);

    Bool_t IsPSAPhoton(TOA2DetParticle* p);
    Bool_t IsPSANucleon(TOA2DetParticle* p);
    Bool_t IsPIDProton(TOA2DetParticle* p);
    Bool_t IsPIDPion(TOA2DetParticle* p);
    
    void UndoQuadraticEnergyCorr(TOA2DetParticle* p);

    void FillPSA(TH2* h, TOA2DetParticle* p, Double_t w = 1.);
    void FilldE_E(TH2* hCB, TH2* hTAPS, TOA2DetParticle* p, Double_t w = 1.);
    void FillTOF(TH2* hCB, TH2* hTAPS, TOA2DetParticle* p, Double_t t, Bool_t isMC, Double_t w = 1.);
    void FillTaggerCoincidence(TOA2RecParticle& meson, Double_t taggerTime, TH1* hCB, TH1* hTAPS);
    void FillTaggerCoincidence(TOA2RecParticle& meson, Double_t taggerTime, Int_t taggerElement, TH2* hCB, TH2* hTAPS);
    
    void OpenOutputFile();

public:
    TA2MyPhysics(const char* name, TA2Analysis* analysis);
    virtual ~TA2MyPhysics();

    virtual void SetConfig(Char_t* line, Int_t key);
    virtual void PostInit();
    virtual void LoadVariable();
    virtual void ReconstructPhysics() = 0;
    void Reconstruct();
    virtual TA2DataManager* CreateChild( const char*, Int_t ) { return NULL; } 
 
    Long64_t GetEventCounter() const { return fEventCounter; }
    Int_t GetRunNumber() const { return fRunNumber; }
    TFile* GetOutputFile() const { return fOutputFile; }

    ClassDef(TA2MyPhysics, 1)
};

#endif

