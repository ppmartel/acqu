/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TA2MyCaLib                                                           //
//                                                                      //
// Class used for the CaLib based calibration.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef __TA2MyCaLib_h__
#define __TA2MyCaLib_h__

#include "THnSparse.h"

#include "TOA2RecMeson2g.h"
#include "TOA2DetParticle.h"
#include "TOLoader.h"

#include "TA2MyPhysics.h"


enum {
    ECALIB_TARGET_POS = 30000,
    ECALIB_CB_ENERGY,
    ECALIB_CB_QUAD,
    ECALIB_CB_TIME,
    ECALIB_CB_RISETIME,
    ECALIB_CB_WALK,
    ECALIB_CB_WALK_EPT,
    ECALIB_CBTAPS_LED,
    ECALIB_CB_PROTON_ECORR,
    ECALIB_TAPS_ENERGY,
    ECALIB_TAPS_ENERGY_BG_SUBTR,
    ECALIB_TAPS_TIME,
    ECALIB_TAPS_QUAD,
    ECALIB_TAPS_PSA,
    ECALIB_TAPS_LED,
    ECALIB_TAPS_PROTON_ECORR,
    ECALIB_PID_PHI,
    ECALIB_PID_ENERGY,
    ECALIB_PID_ENERGY_TRAD,
    ECALIB_PID_DROOP,
    ECALIB_PID_TIME,
    ECALIB_PID_EFF,
    ECALIB_VETO_CORR,
    ECALIB_VETO_ENERGY,
    ECALIB_VETO_TIME,
    ECALIB_VETO_EFF,
    ECALIB_TAGGER_TIME,
    ECALIB_PROTON_LT_ATT,
    ECALIB_PHOTON_RES,
    ECALIB_PARTICLE_RES,
    ECALIB_BADSCR,
    ECALIB_PWO_CHECK,
    ECALIB_PWO_PED,
    ECALIB_TAPS_TIME_CUT,
    ECALIB_TAPS_TIME_CUT_MAXRING
};


static const Map_t myCaLibConfigKeys[] = {
    // Calibration configuration keys
    {"CaLib-Target-Position:"         ,   ECALIB_TARGET_POS           },        // target position
    {"CaLib-CB-Energy:"               ,   ECALIB_CB_ENERGY            },        // CB energy calibration
    {"CaLib-CB-Quad:"                 ,   ECALIB_CB_QUAD              },        // CB quadratic energy correction
    {"CaLib-CB-Time:"                 ,   ECALIB_CB_TIME              },        // CB time calibration
    {"CaLib-CB-RiseTime:"             ,   ECALIB_CB_RISETIME          },        // CB rise time calibration
    {"CaLib-CB-Walk:"                 ,   ECALIB_CB_WALK              },        // CB time walk calibration
    {"CaLib-CB-Walk-EPT:"             ,   ECALIB_CB_WALK_EPT          },        // CB time walk calibration for EPT
    {"CaLib-CB-Proton-Energy-Corr:"   ,   ECALIB_CB_PROTON_ECORR      },        // CB proton energy correction
    {"CaLib-CB-TAPS-LED:"             ,   ECALIB_CBTAPS_LED           },        // CB-TAPS LED thresholds
    {"CaLib-TAPS-Energy:"             ,   ECALIB_TAPS_ENERGY          },        // TAPS energy calibration
    {"CaLib-TAPS-Energy-BG-Subtr:"    ,   ECALIB_TAPS_ENERGY_BG_SUBTR },        // TAPS energy calibration (with random background subtr.)
    {"CaLib-TAPS-Time:"               ,   ECALIB_TAPS_TIME            },        // TAPS time calibration
    {"CaLib-TAPS-Quad:"               ,   ECALIB_TAPS_QUAD            },        // TAPS quadratic energy correction
    {"CaLib-TAPS-PSA:"                ,   ECALIB_TAPS_PSA             },        // TAPS pulse shape calibration
    {"CaLib-TAPS-LED:"                ,   ECALIB_TAPS_LED             },        // TAPS LED calibration
    {"CaLib-TAPS-Proton-Energy-Corr:" ,   ECALIB_TAPS_PROTON_ECORR    },        // CB proton energy correction
    {"CaLib-PID-Phi:"                 ,   ECALIB_PID_PHI              },        // PID Phi calibration
    {"CaLib-PID-Energy:"              ,   ECALIB_PID_ENERGY           },        // PID energy calibration
    {"CaLib-PID-Energy-Trad:"         ,   ECALIB_PID_ENERGY_TRAD      },        // PID energy calibration (traditional)
    {"CaLib-PID-Droop:"               ,   ECALIB_PID_DROOP            },        // PID droop calibration
    {"CaLib-PID-Time:"                ,   ECALIB_PID_TIME             },        // PID time calibration
    {"CaLib-PID-Eff:"                 ,   ECALIB_PID_EFF              },        // PID efficiency calibration
    {"CaLib-Veto-Corr:"               ,   ECALIB_VETO_CORR            },        // veto-crystal element correlation
    {"CaLib-Veto-Energy:"             ,   ECALIB_VETO_ENERGY          },        // Veto energy calibration
    {"CaLib-Veto-Time:"               ,   ECALIB_VETO_TIME            },        // Veto time calibration
    {"CaLib-Veto-Eff:"                ,   ECALIB_VETO_EFF             },        // Veto efficiency calibration
    {"CaLib-Tagger-Time:"             ,   ECALIB_TAGGER_TIME          },        // tagger time
    {"CaLib-Proton-Light-Att:"        ,   ECALIB_PROTON_LT_ATT        },        // Proton light attenuation correction
    {"CaLib-Photon-Res:"              ,   ECALIB_PHOTON_RES           },        // Photon energy and angle resolutions
    {"CaLib-Particle-Res:"            ,   ECALIB_PARTICLE_RES         },        // Particle energy and angle resolutions
    {"CaLib-BadScalerReads:"          ,   ECALIB_BADSCR               },        // bad scaler reads
    {"CaLib-PWO-Check:"               ,   ECALIB_PWO_CHECK            },        // PWO check
    {"CaLib-PWO-Ped:"                 ,   ECALIB_PWO_PED              },        // PWO pedestal calibration
    {"CaLib-TAPS-Time-Cut:"           ,   ECALIB_TAPS_TIME_CUT        },        // TAPS Time Cut
    {"CaLib-TAPS-Time-Cut-MaxRing:"   ,   ECALIB_TAPS_TIME_CUT_MAXRING    },        // Apply TAPS Time Cut for Sector Elements smaller than this
    // Termination
    {NULL        , -1           }
};


class TA2MyCaLib : public TA2MyPhysics
{

private:
    UInt_t fNCalib;                                         // number of enabled calibrations
    Int_t fNelemTAGG;                                       // number of Tagger elements
    Int_t fNelemCB;                                         // number of CB elements
    Int_t fNelemTAPS;                                       // number of TAPS elements
    Int_t fNelemPID;                                        // number of PID elements
    Int_t fNelemVeto;                                       // number of Veto elements

    // ---------------------------------- target position ---------------------------------- 
    Int_t fCalib_Target_Position;                           // target position calibration toggle
    TH2* fHCalib_CB_IM_TargPos;                             // CB invariant mass vs target position

    // ------------------------------------- CB energy ------------------------------------- 
    Int_t fCalib_CB_Energy;                                 // CB energy calibration toggle
    TH2* fHCalib_CB_IM;                                     // CB invariant mass vs element
    TH2* fHCalib_CB_IM_Neut;                                // CB invariant mass (from neutral hits) vs element

    // -------------------------- CB quadratic energy correction --------------------------- 
    Int_t fCalib_CB_Quad;                                   // CB quadratic energy correction toggle
    Double_t fCalib_CB_Quad_Pi0_Min;                        // lower bound for the pi0 invariant mass cut
    Double_t fCalib_CB_Quad_Pi0_Max;                        // upper bound for the pi0 invariant mass cut
    Double_t fCalib_CB_Quad_Eta_Min;                        // lower bound for the eta invariant mass cut
    Double_t fCalib_CB_Quad_Eta_Max;                        // upper bound for the eta invariant mass cut
    TH2* fHCalib_CB_Quad_IM;                                // CB invariant mass vs element
    TH2* fHCalib_CB_Quad_Pi0_Mean_E;                        // mean energy of pi0 in CB
    TH2* fHCalib_CB_Quad_Eta_Mean_E;                        // mean energy of eta in CB

    // -------------------------------------- CB time -------------------------------------- 
    Int_t fCalib_CB_Time;                                   // CB time calibration toggle
    TH2* fHCalib_CB_Time;                                   // CB time vs element
    TH2* fHCalib_CB_Time_Neut;                              // CB time (from neutral hits) vs element
    TH2* fHCalib_CB_Time_Ind;                               // CB time vs element, independent
    
    // ----------------------------------- CB rise time ------------------------------------ 
    Int_t fCalib_CB_RiseTime;                               // CB time calibration toggle
    Double_t fCalib_CB_RiseTime_Pi0_Min;                    // lower bound of the pi0 invariant mass cut
    Double_t fCalib_CB_RiseTime_Pi0_Max;                    // upper bound of the pi0 invariant mass cut
    TH2* fHCalib_CB_RiseTime;                               // CB time vs element
    
    // ----------------------------------- CB time walk ------------------------------------ 
    Int_t fCalib_CB_Walk;                                   // CB time walk calibration toggle
    Double_t fCalib_CB_Walk_Pi0_Min;                        // lower bound of the pi0 invariant mass cut 
    Double_t fCalib_CB_Walk_Pi0_Max;                        // upper bound of the pi0 invariant mass cut 
    Double_t fCalib_CB_Walk_Prompt_Min;                     // lower bound for prompt tagger hits 
    Double_t fCalib_CB_Walk_Prompt_Max;                     // upper bound for prompt tagger hits 
    Double_t fCalib_CB_Walk_BG1_Min;                        // lower bound for tagger hits in background window 1
    Double_t fCalib_CB_Walk_BG1_Max;                        // upper bound for tagger hits in background window 1
    Double_t fCalib_CB_Walk_BG2_Min;                        // lower bound for tagger hits in background window 2
    Double_t fCalib_CB_Walk_BG2_Max;                        // upper bound for tagger hits in background window 2
    Double_t fCalib_CB_Walk_MM_Min;                         // lower bound of the missing mass cut 
    Double_t fCalib_CB_Walk_MM_Max;                         // upper bound of the missing mass cut 
    TH1* fHCalib_CB_Walk_IM;                                // 2 photon invariant mass for pi0 identification
    TH2* fHCalib_CB_Walk_MM;                                // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_CB_Walk_Rand_Time_CB;                      // CB-tagger time difference for random subtraction
    TH2** fHCalib_CB_Walk_E_T;                              // time vs energy for all CB elements

    // ----------------------------------- CB time walk for EPT  ------------------------------------
    Int_t fCalib_CB_Walk_EPT;                               // CB time walk calibration toggle
    Double_t fCalib_CB_Walk_EPT_Pi0_Min;                    // lower bound of the pi0 invariant mass cut
    Double_t fCalib_CB_Walk_EPT_Pi0_Max;                    // upper bound of the pi0 invariant mass cut
    Double_t fCalib_CB_Walk_EPT_Phi_Min;                    // lower bound of the pi0/proton back2back phi cut
    Double_t fCalib_CB_Walk_EPT_Phi_Max;                    // upper bound of the pi0/proton back2back phi cut
    Double_t fCalib_CB_Walk_EPT_ProtonE_Min;                // lower bound of the proton energy
    Double_t fCalib_CB_Walk_EPT_ProtonE_Max;                // upper bound of the proton energy
    TH1* fHCalib_CB_Walk_EPT_IM;                            // 2 photon invariant mass for pi0 identification
    TH1* fHCalib_CB_Walk_EPT_Phi;                           // angle between pi0 and proton
    TH2** fHCalib_CB_Walk_EPT_E_T;                          // time vs energy for all CB elements


    // --------------------------- CB proton energy correction -----------------------------
    Int_t fCalib_CB_Proton_ECorr;                           // CB proton energy correction
    TH2** fHCalib_CB_Proton_ECorr;                          // f_corr vs measured energy for every detector
    TH2** fHCalib_CB_Proton_ECorr_Inv;                      // f_corr vs vertex energy for every detector
    TH2* fHCalib_CB_Proton_ECorr_Check;                     // checking histogram
    
    // ------------------------------- CB-TAPS LED threshold ------------------------------- 
    Int_t fCalib_CBTAPS_LED;                                // CB-TAPS LED threshold calibration toggle
    TH2* fHCalib_CB_LED_M2;                                 // highest energy per CB segment for every detector
    TH2* fHCalib_TAPS_LED_M2;                               // highest energy per TAPS sector for every detector
    TH2* fHCalib_CB_LED_M3;                                 // highest energy per CB segment for every detector
    TH2* fHCalib_TAPS_LED_M3;                               // highest energy per TAPS sector for every detector
    TH2* fHCalib_TAPS_LED2_M2;                              // highest energy per TAPS sector for every detector
     
    // ------------------------------------ TAPS energy ------------------------------------ 
    TH3* fHCalib_TOF_TAPS;
    TH3* fHCalib_TOF_TAPS_Neut;
    Int_t fCalib_TAPS_Energy;                               // TAPS energy calibration toggle
    TH2* fHCalib_TAPS_IM_Neut;                              // CB-TAPS invariant mass (from neutral hits) vs element
    TH2* fHCalib_TAPS_IM_Neut_1CB_1TAPS;                    // CB-TAPS invariant mass (from exactly 2 neutral hits) vs element
    TH2* fHCalib_TAPS_IM_TAPS;                              // TAPS-TAPS invariant mass vs element
    TH2* fHCalib_TAPS_IM_Neut_TAPS;                         // TAPS-TAPS invariant mass (from neutral hits) vs element
    TH3* fHCalib_TAPS_IM_Neut_TAPS_mult;                    // TAPS invariant mass vs element vs nr of multiplicities
    
    // ------------------------------ TAPS energy (BG subtr.) ------------------------------ 
    Int_t fCalib_TAPS_Energy_BG_Subtr;                      // TAPS energy calibration (BG subtr.) toggle
    Double_t fCalib_TAPS_Energy_Prompt_Min;                 // lower bound for prompt tagger hits 
    Double_t fCalib_TAPS_Energy_Prompt_Max;                 // upper bound for prompt tagger hits 
    Double_t fCalib_TAPS_Energy_BG1_Min;                    // lower bound for tagger hits in background window 1
    Double_t fCalib_TAPS_Energy_BG1_Max;                    // upper bound for tagger hits in background window 1
    Double_t fCalib_TAPS_Energy_BG2_Min;                    // lower bound for tagger hits in background window 2
    Double_t fCalib_TAPS_Energy_BG2_Max;                    // upper bound for tagger hits in background window 2
    Double_t fCalib_TAPS_Energy_MM_Min;                     // lower bound of the missing mass cut 
    Double_t fCalib_TAPS_Energy_MM_Max;                     // upper bound of the missing mass cut 
    TH2* fHCalib_TAPS_IM_Neut_BG_Subtr;                     // CB-TAPS invariant mass (from neutral hits) vs element
    TH2* fHCalib_TAPS_Energy_MM;                            // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_TAPS_Energy_Rand_Time_CB;                  // CB-tagger time difference for random subtraction
      
    // ------------------------------------- TAPS time ------------------------------------- 
    Int_t fCalib_TAPS_Time;                                 // TAPS time calibration toggle
    TH2* fHCalib_TAPS_Time;                                 // TAPS time vs element
    TH2* fHCalib_TAPS_Time_Neut;                            // TAPS time (from neutral hits) vs element
    TH2* fHCalib_TAPS_Time_Neut_IM_Cut;                     // TAPS time (from neutral hits) vs element with invariant mass cut
    TH2* fHCalib_TAPS_Time_Ind;                             // TAPS time vs element, independent
    
    // ------------------------- TAPS quadratic energy correction -------------------------- 
    Int_t fCalib_TAPS_Quad;                                 // TAPS quadratic energy correction toggle
    Double_t fCalib_TAPS_Quad_Pi0_Min;                      // lower bound for the pi0 invariant mass cut
    Double_t fCalib_TAPS_Quad_Pi0_Max;                      // upper bound for the pi0 invariant mass cut
    Double_t fCalib_TAPS_Quad_Eta_Min;                      // lower bound for the eta invariant mass cut
    Double_t fCalib_TAPS_Quad_Eta_Max;                      // upper bound for the eta invariant mass cut
    TH2* fHCalib_TAPS_Quad_IM;                              // TAPS invariant mass vs element
    TH2* fHCalib_TAPS_Quad_Pi0_Mean_E;                      // mean energy of pi0 in TAPS 
    TH2* fHCalib_TAPS_Quad_Eta_Mean_E;                      // mean energy of eta in TAPS 
    
    // ------------------------------- TAPS PSA calibration -------------------------------- 
    Int_t fCalib_TAPS_PSA;                                  // TAPS PSA calibration toggle
    TH2** fHCalib_TAPS_PSAR_PSAA;                           // LG, SG radius vs angle
    TH2** fHCalib_TAPS_PSAR_PSAA_SM;                        // LG, SG radius vs angle (smeared SG)

    // ------------------------------- TAPS LED calibration -------------------------------- 
    Int_t fCalib_TAPS_LED;                                  // TAPS LED calibration toggle
    TH2* fHCalib_TAPS_LED_Norm;                             // uncut energy spectrum
    TH2* fHCalib_TAPS_LED_LED1;                             // energy spectrum with LED1 hit
    TH2* fHCalib_TAPS_LED_LED2;                             // energy spectrum with LED2 hit
    
    // -------------------------- TAPS proton energy correction ---------------------------- 
    Int_t fCalib_TAPS_Proton_ECorr;                         // TAPS proton energy correction
    TH2** fHCalib_TAPS_Proton_ECorr;                        // f_corr vs measured energy for every detector
    TH2** fHCalib_TAPS_Proton_ECorr_Inv;                    // f_corr vs vertex energy for every detector
    TH2* fHCalib_TAPS_Proton_ECorr_Check;                   // checking histogram
 
    // -------------------------------------- PID Phi -------------------------------------- 
    Int_t fCalib_PID_Phi;                                   // PID Phi calibration toggle
    TH2* fHCalib_PID_CBPhi_ID;                              // CB cluster Phi vs PID element index
    TH2* fHCalib_PID_CBPhi_ID_1Cryst;                       // CB cluster (1 crystal) Phi vs PID element index
    TH2* fHCalib_PID_CBTheta_ID;                            // CB cluster Theta vs PID element index
    
    // ------------------------------------- PID energy ------------------------------------ 
    Int_t fCalib_PID_Energy;                                // PID energy calibration toggle
    Double_t fCalib_PID_Energy_Pi0_Min;                     // lower bound of the pi0 invariant mass cut 
    Double_t fCalib_PID_Energy_Pi0_Max;                     // upper bound of the pi0 invariant mass cut 
    Double_t fCalib_PID_Energy_Prompt_Min;                  // lower bound for prompt tagger hits 
    Double_t fCalib_PID_Energy_Prompt_Max;                  // upper bound for prompt tagger hits 
    Double_t fCalib_PID_Energy_BG1_Min;                     // lower bound for tagger hits in background window 1
    Double_t fCalib_PID_Energy_BG1_Max;                     // upper bound for tagger hits in background window 1
    Double_t fCalib_PID_Energy_BG2_Min;                     // lower bound for tagger hits in background window 2
    Double_t fCalib_PID_Energy_BG2_Max;                     // upper bound for tagger hits in background window 2
    Double_t fCalib_PID_Energy_MM_Min;                      // lower bound of the missing mass cut 
    Double_t fCalib_PID_Energy_MM_Max;                      // upper bound of the missing mass cut 
    TH1* fHCalib_PID_Energy_IM;                             // 2 photon invariant mass for pi0 identification
    TH2* fHCalib_PID_Energy_MM;                             // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_PID_Energy_Rand_Time_CB;                   // CB-tagger time difference for random subtraction
    TH1* fHCalib_PID_Energy_Rand_Time_TAPS;                 // TAPS-tagger time difference for random subtraction
    TH1* fHCalib_PID_Energy_Pi0_Energy;                     // energy of the pi0
    TH1* fHCalib_PID_Energy_Pi0_Theta;                      // theta of the pi0
    TH1* fHCalib_PID_Energy_Proton_Energy;                  // energy of the proton
    TH2* fHCalib_PID_Energy_Raw_PID_Energy;                 // raw PID energy
    TH2* fHCalib_PID_Energy_Proton_PID_Energy;              // proton PID energy
    TH3** fHCalib_PID_dE_E;                                 // Cluster theta vs CB-PID dE vs E plots

    // ------------------------------ PID energy (traditional) ----------------------------- 
    Int_t fCalib_PID_Energy_Trad;                           // PID energy calibration (traditional) toggle
    TH3** fHCalib_PID_dE_E_Trad;                            // Cluster theta vs CB-PID dE vs E plots

    // ------------------------------------- PID droop ------------------------------------ 
    Int_t fCalib_PID_Droop;                                 // PID droop calibration toggle

    // ------------------------------------- PID time -------------------------------------- 
    Int_t fCalib_PID_Time;                                  // PID time calibration toggle
    TH2* fHCalib_PID_Time;                                  // PID time vs element
    TH2* fHCalib_PID_Time_Ind;                              // PID time vs element, independent
    
    // ---------------------------------- PID efficiency ----------------------------------- 
    Int_t fCalib_PID_Eff;                                   // PID efficiency alibration toggle
    Double_t fCalib_PID_Eff_Pi0_Min;                        // lower bound of the pi0 invariant mass cut 
    Double_t fCalib_PID_Eff_Pi0_Max;                        // upper bound of the pi0 invariant mass cut 
    Double_t fCalib_PID_Eff_Prompt_Min;                     // lower bound for prompt tagger hits 
    Double_t fCalib_PID_Eff_Prompt_Max;                     // upper bound for prompt tagger hits 
    Double_t fCalib_PID_Eff_BG1_Min;                        // lower bound for tagger hits in background window 1
    Double_t fCalib_PID_Eff_BG1_Max;                        // upper bound for tagger hits in background window 1
    Double_t fCalib_PID_Eff_BG2_Min;                        // lower bound for tagger hits in background window 2
    Double_t fCalib_PID_Eff_BG2_Max;                        // upper bound for tagger hits in background window 2
    Double_t fCalib_PID_Eff_MM_Min;                         // lower bound of the missing mass cut 
    Double_t fCalib_PID_Eff_MM_Max;                         // upper bound of the missing mass cut 
    TH1* fHCalib_PID_Eff_IM;                                // 2 photon invariant mass for pi0 identification
    TH2* fHCalib_PID_Eff_MM;                                // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_PID_Eff_Rand_Time_CB;                      // CB-tagger time difference for random subtraction
    TH1* fHCalib_PID_Eff_ThetaDiff;                         // different in theta between calculated proton and measured cluster
    TH1* fHCalib_PID_Eff_Hit;                               // events with coincident PID hit
    TH1* fHCalib_PID_Eff_NoHit;                             // events with no coincident PID hit
    
    // ----------------------------- veto-crystal correlation  ----------------------------- 
    Int_t fCalib_Veto_Corr;                                 // veto-crystal correlation toggle
    TH2* fHCalib_Veto_Corr;                                 // veto hit vs TAPS crystal hit
    
    // ------------------------------------ Veto energy ------------------------------------ 
    Int_t fCalib_Veto_Energy;                               // Veto energy calibration toggle
    TH2** fHCalib_Veto_dE_E;                                // TAPS-Veto dE vs E plots

    // ------------------------------------ Veto time ------------------------------------ 
    Int_t fCalib_Veto_Time;                                 // Veto time calibration toggle
    TH2* fHCalib_Veto_Time;                                 // Veto time vs element
    TH2* fHCalib_Veto_Time_Ind;                             // Veto time vs element, independent
    
    // --------------------------------- Veto  efficiency ----------------------------------- 
    Int_t fCalib_Veto_Eff;                                  // Veto efficiency alibration toggle
    Double_t fCalib_Veto_Eff_Pi0_Min;                       // lower bound of the pi0 invariant mass cut 
    Double_t fCalib_Veto_Eff_Pi0_Max;                       // upper bound of the pi0 invariant mass cut 
    Double_t fCalib_Veto_Eff_Prompt_Min;                    // lower bound for prompt tagger hits 
    Double_t fCalib_Veto_Eff_Prompt_Max;                    // upper bound for prompt tagger hits 
    Double_t fCalib_Veto_Eff_BG1_Min;                       // lower bound for tagger hits in background window 1
    Double_t fCalib_Veto_Eff_BG1_Max;                       // upper bound for tagger hits in background window 1
    Double_t fCalib_Veto_Eff_BG2_Min;                       // lower bound for tagger hits in background window 2
    Double_t fCalib_Veto_Eff_BG2_Max;                       // upper bound for tagger hits in background window 2
    Double_t fCalib_Veto_Eff_MM_Min;                        // lower bound of the missing mass cut 
    Double_t fCalib_Veto_Eff_MM_Max;                        // upper bound of the missing mass cut 
    TH1* fHCalib_Veto_Eff_IM;                               // 2 photon invariant mass for pi0 identification
    TH2* fHCalib_Veto_Eff_MM;                               // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_Veto_Eff_Rand_Time_CB;                     // CB-tagger time difference for random subtraction
    TH1* fHCalib_Veto_Eff_ThetaDiff;                        // different in theta between calculated proton and measured cluster
    TH1* fHCalib_Veto_Eff_Hit;                              // events with coincident Veto hit
    TH1* fHCalib_Veto_Eff_NoHit;                            // events with no coincident Veto hit
      
    // ------------------------------------ tagger time ------------------------------------ 
    Int_t fCalib_Tagger_Time;                               // tagger time toggle
    TH2* fHCalib_Tagger_Time;                               // tagger + TAPS time vs tagger element for all TAPS element
    TH2* fHCalib_Tagger_Time_Neut;                          // tagger + TAPS time vs tagger element for all TAPS element (neutral hits)
    TH2* fHCalib_Tagger_Time_Pi0;                           // tagger + TAPS time vs tagger element for all TAPS element (phi0 gammas)
    TH2* fHCalib_Tagger_Time_Ind;                           // tagger time, independent
    
    // ------------------------ Proton light attenuation correction ------------------------ 
    Int_t fCalib_Proton_Lt_Att;                             // Proton light attenuation correction
    Double_t fCalib_Proton_Lt_Att_Pi0_Min;                  // lower bound of the pi0 invariant mass cut 
    Double_t fCalib_Proton_Lt_Att_Pi0_Max;                  // upper bound of the pi0 invariant mass cut 
    Double_t fCalib_Proton_Lt_Att_Prompt_Min;               // lower bound for prompt tagger hits 
    Double_t fCalib_Proton_Lt_Att_Prompt_Max;               // upper bound for prompt tagger hits 
    Double_t fCalib_Proton_Lt_Att_BG1_Min;                  // lower bound for tagger hits in background window 1
    Double_t fCalib_Proton_Lt_Att_BG1_Max;                  // upper bound for tagger hits in background window 1
    Double_t fCalib_Proton_Lt_Att_BG2_Min;                  // lower bound for tagger hits in background window 2
    Double_t fCalib_Proton_Lt_Att_BG2_Max;                  // upper bound for tagger hits in background window 2
    Double_t fCalib_Proton_Lt_Att_MM_Min;                   // lower bound of the missing mass cut 
    Double_t fCalib_Proton_Lt_Att_MM_Max;                   // upper bound of the missing mass cut 
    TGraph** fCalib_Proton_Lt_Att_CB_FGenCorr;              // correction functions for T_v -> T_c
    TH1* fHCalib_Proton_Lt_Att_IM;                          // 2 photon invariant mass for pi0 identification
    TH2* fHCalib_Proton_Lt_Att_MM;                          // pi0 missing mass for g + p -> p + pi0 identification
    TH1* fHCalib_Proton_Lt_Att_Rand_Time_CB;                // CB-tagger time difference for random subtraction
    TH2* fHCalib_Proton_Lt_Att_CB;                          // f_corr vs measured energy for CB
    TH2** fHCalib_Proton_Lt_Att_TAPS;                       // f_corr vs measured energy for every TAPS detector
    
    // --------------------------------- Photon resolutions --------------------------------- 
    Int_t fCalib_Photon_Res;                                // Photon resolutions
    TH2* fHCalib_Photon_Res_Theta_CB;                       // theta resolution in CB
    TH2* fHCalib_Photon_Res_Theta_TAPS;                     // theta resolution in TAPS
    TH2* fHCalib_Photon_Res_Phi_TAPS;                       // phi resolution in CB
    TH2* fHCalib_Photon_Res_Phi_CB;                         // phi resolution in TAPS
    TH2* fHCalib_Photon_Res_Energy_CB;                      // energy resolution in CB
    TH2* fHCalib_Photon_Res_Energy_TAPS;                    // energy resolution in TAPS
    
    // -------------------------------- Particle resolutions --------------------------------- 
    Int_t fCalib_Particle_Res;                              // Photon resolutions
    TH1* fHCalib_Particle_Res_ClusterMult;                  // cluster multiplicity
    TTree* fCalib_Particle_Res_Tree_CB;                     // output event tree
    TTree* fCalib_Particle_Res_Tree_TAPS;                   // output event tree
    Float_t fCalib_Particle_Res_Theta_Gen;                  // theta of generated event
    Float_t fCalib_Particle_Res_Theta_Det;                  // theta of detected event
    Float_t fCalib_Particle_Res_Phi_Gen;                    // theta of generated event
    Float_t fCalib_Particle_Res_Phi_Det;                    // theta of detected event
    Float_t fCalib_Particle_Res_Energy_Gen;                 // theta of generated event
    Float_t fCalib_Particle_Res_Energy_Det;                 // theta of detected event
    Int_t fCalib_Particle_Res_Module_Det;                   // detector module of detected event

    // ---------------------------------- bad scaler read -----------------------------------
    Int_t fCalib_BadScR;                                    // Bad scaler reads toggle
    TH2* fHCalib_BadScR_NaIHits;                            // NaI hits vs. scaler reads
    TH2* fHCalib_BadScR_BaF2PWOHits;                        // BaF2PWO hits vs. scaler reads
    TH2* fHCalib_BadScR_BaF2Hits;                           // BaF2 hits vs. scaler reads
    TH2* fHCalib_BadScR_PWOHits;                            // PWO hits vs. scaler reads
    TH2* fHCalib_BadScR_PIDHits;                            // PID hits vs. scaler reads
    TH2* fHCalib_BadScR_VetoHits;                           // Veto hits vs. scaler reads
    TH2* fHCalib_BadScR_LadderHits;                         // Ladder hits vs. scaler reads
    TH2* fHCalib_BadScR_Scalers;                            // Scalers vs. scaler reads
    
    // ------------------------------------- PWO check --------------------------------------
    Int_t fCalib_PWO_Check;                                 // PWO check toggle
    TH2* fHCalib_PWO_Check_ClusterPos;                      // cluster positions (all clusters)
    TH2* fHCalib_PWO_Check_ClusterPos_Neut;                 // cluster positions (neutral clusters)
    TH2* fHCalib_PWO_Check_ClusterPos_Charg;                // cluster positions (charged clusters)
    TH1* fHCalib_PWO_Check_Rand_Time_CB;                    // CB-tagger time difference for random subtraction
    TH1* fHCalib_PWO_Check_Rand_Time_BaF2;                  // BaF2-tagger time difference for random subtraction
    TH1* fHCalib_PWO_Check_Rand_Time_PWO;                   // PWO-tagger time difference for random subtraction
    TH2* fHCalib_PWO_Check_TOF_BaF2_Photon;                 // TOF-E BaF2 for photons
    TH2* fHCalib_PWO_Check_TOF_BaF2_Proton;                 // TOF-E BaF2 for protons
    TH2* fHCalib_PWO_Check_DeltaE_BaF2_Proton;              // dE-E BaF2 for protons
    TH2* fHCalib_PWO_Check_TOF_PWO_Photon;                  // TOF-E PWO for photons
    TH2* fHCalib_PWO_Check_TOF_PWO_Proton;                  // TOF-E PWO for protons
    TH2* fHCalib_PWO_Check_DeltaE_PWO_Proton;               // dE-E PWO for protons
    TH2* fHCalib_PWO_Check_Edet_BaF2_Proton;                // E_det vs. E_calc BaF2 for protons
    TH2* fHCalib_PWO_Check_Edet_PWO_Proton;                 // E_det vs. E_calc PWO for protons
    TH2* fHCalib_PWO_Check_ClusterSize_Photon;              // Photon cluster size vs. cluster center
    TH2* fHCalib_PWO_Check_ClusterSize_Proton;              // Proton cluster size vs. cluster center
    TH2* fHCalib_PWO_Check_ClusterInfo_Photon;              // Photon cluster members vs. cluster center
    TH2* fHCalib_PWO_Check_ClusterInfo_Proton;              // Proton cluster members vs. cluster center
   
    // ------------------------------------ PWO pedestal ------------------------------------
    Int_t fCalib_PWO_Ped;                                   // PWO pedestal toggle
    Int_t fCalib_PWO_MaxRing;                               // maximum ring with PWO crystals
    TH1** fHCalib_PWO_Ped;                                  // raw energy histograms for PWO
    TH1** fHCalib_PWO_Veto_Ped;                             // raw energy histograms for PWO-veto

    // ------------------------------------ TAPS Time Cut -------------------------------------------
    Double_t taps_time_cut_min;                             // TAPS time cut low edge
    Double_t taps_time_cut_max;                             // TAPS time cut high edge
    Int_t    taps_time_cut_max_ring;                  // Max Sector Element for time cut

    bool TAPSTimeCutOK(const TOA2DetParticle* particle) const;
    bool isInnerTAPSArea(const TOA2DetParticle *particle) const;
    static UInt_t GetTAPSSectorElement(const UInt_t elem);

    // ------------------------------------           -------------------------------------------
    Int_t GetPIDElementForPhi(Double_t phi);

public:
    TA2MyCaLib(const char* name, TA2Analysis* analysis);
    virtual ~TA2MyCaLib();

    void SetConfig(Char_t* line, Int_t key);
    void PostInit();
    void ReconstructPhysics();
    
    ClassDef(TA2MyCaLib, 1)
};

#endif

