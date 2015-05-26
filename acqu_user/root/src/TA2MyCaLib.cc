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


#include "TA2MyCaLib.h"

#include <limits>

//______________________________________________________________________________
TA2MyCaLib::TA2MyCaLib(const char* name, TA2Analysis* analysis) 
    : TA2MyPhysics(name, analysis)
{
    // Constructor.

    // command-line recognition for SetConfig()
    AddCmdList(myCaLibConfigKeys);

    // init variables
    fNCalib    = 0;
    fNelemTAGG = 0;
    fNelemCB   = 0;
    fNelemTAPS = 0;
    fNelemPID  = 0;
    fNelemVeto = 0;
    
    // target position
    fCalib_Target_Position = 0;

    // CB energy calibration
    fCalib_CB_Energy = 0;    
    
    // CB quadratic energy correction
    fCalib_CB_Quad = 0;    
    fCalib_CB_Quad_Pi0_Min = 0;    
    fCalib_CB_Quad_Pi0_Max = 0;    
    fCalib_CB_Quad_Eta_Min = 0;    
    fCalib_CB_Quad_Eta_Max = 0;    
    
    // CB time calibration
    fCalib_CB_Time = 0;    
    
    // CB rise time calibration
    fCalib_CB_RiseTime = 0;
    fCalib_CB_RiseTime_Pi0_Min = 0;
    fCalib_CB_RiseTime_Pi0_Max = 0;

    // CB time walk calibration
    fCalib_CB_Walk            = 0;   
    fCalib_CB_Walk_Pi0_Min    = 0;
    fCalib_CB_Walk_Pi0_Max    = 0;
    fCalib_CB_Walk_Prompt_Min = 0;   
    fCalib_CB_Walk_Prompt_Max = 0;
    fCalib_CB_Walk_BG1_Min    = 0;
    fCalib_CB_Walk_BG1_Max    = 0;
    fCalib_CB_Walk_BG2_Min    = 0;
    fCalib_CB_Walk_BG2_Max    = 0;
    fCalib_CB_Walk_MM_Min     = 0;
    fCalib_CB_Walk_MM_Max     = 0;

    // CB time walk calibration for EPT
    fCalib_CB_Walk_EPT            = 0;
    fCalib_CB_Walk_EPT_Pi0_Min    = 0;
    fCalib_CB_Walk_EPT_Pi0_Max    = 0;
    fCalib_CB_Walk_EPT_Phi_Min    = 0;
    fCalib_CB_Walk_EPT_Phi_Max    = 0;

    // CB proton energy correction
    fCalib_CB_Proton_ECorr = 0;

    // CB-TAPS LED calibration
    fCalib_CBTAPS_LED = 0;

    // TAPS energy calibration
    fCalib_TAPS_Energy = 0;
    
    // TAPS energy calibration (BG subtr.)
    fCalib_TAPS_Energy_BG_Subtr = 0;
    
    // TAPS time calibration
    fCalib_TAPS_Time = 0;    
    
    // TAPS quadratic energy correction
    fCalib_TAPS_Quad = 0;    
    fCalib_TAPS_Quad_Pi0_Min = 0;    
    fCalib_TAPS_Quad_Pi0_Max = 0;    
    fCalib_TAPS_Quad_Eta_Min = 0;    
    fCalib_TAPS_Quad_Eta_Max = 0;    
    
    // TAPS PSA calibration
    fCalib_TAPS_PSA = 0;

    // TAPS LED calibration
    fCalib_TAPS_LED = 0;
    
    // TAPS proton energy correction
    fCalib_TAPS_Proton_ECorr = 0;

    // PID Phi calibration
    fCalib_PID_Phi = 0;
    
    // PID energy calibration
    fCalib_PID_Energy = 0;
    fCalib_PID_Energy_Pi0_Min = 0;
    fCalib_PID_Energy_Pi0_Max = 0;
    fCalib_PID_Energy_Prompt_Min = 0;
    fCalib_PID_Energy_Prompt_Max = 0;
    fCalib_PID_Energy_BG1_Min = 0;
    fCalib_PID_Energy_BG1_Max = 0;
    fCalib_PID_Energy_BG2_Min = 0;
    fCalib_PID_Energy_BG2_Max = 0;
    fCalib_PID_Energy_MM_Min = 0;
    fCalib_PID_Energy_MM_Max = 0;
    
    // PID energy calibration (traditional)
    fCalib_PID_Energy_Trad = 0;
    
    // PID droop calibration
    fCalib_PID_Droop = 0;
    
    // PID time calibration
    fCalib_PID_Time = 0;

    // PID efficiency calibration
    fCalib_PID_Eff = 0;
    fCalib_PID_Eff_Pi0_Min = 0;
    fCalib_PID_Eff_Pi0_Max = 0;
    fCalib_PID_Eff_Prompt_Min = 0;
    fCalib_PID_Eff_Prompt_Max = 0;
    fCalib_PID_Eff_BG1_Min = 0;
    fCalib_PID_Eff_BG1_Max = 0;
    fCalib_PID_Eff_BG2_Min = 0;
    fCalib_PID_Eff_BG2_Max = 0;
    fCalib_PID_Eff_MM_Min = 0;
    fCalib_PID_Eff_MM_Max = 0;

    // veto-crystal correlation
    fCalib_Veto_Corr = 0;
    
    // Veto energy calibration
    fCalib_Veto_Energy = 0;
    
    // Veto time calibration
    fCalib_Veto_Time = 0;
    
    // Veto efficiency calibration
    fCalib_Veto_Eff = 0;
    fCalib_Veto_Eff_Pi0_Min = 0;
    fCalib_Veto_Eff_Pi0_Max = 0;
    fCalib_Veto_Eff_Prompt_Min = 0;
    fCalib_Veto_Eff_Prompt_Max = 0;
    fCalib_Veto_Eff_BG1_Min = 0;
    fCalib_Veto_Eff_BG1_Max = 0;
    fCalib_Veto_Eff_BG2_Min = 0;
    fCalib_Veto_Eff_BG2_Max = 0;
    fCalib_Veto_Eff_MM_Min = 0;
    fCalib_Veto_Eff_MM_Max = 0;

    // tagger time
    fCalib_Tagger_Time = 0;

    // proton light attenuation correction
    fCalib_Proton_Lt_Att = 0;
    fCalib_Proton_Lt_Att_Pi0_Min = 0;
    fCalib_Proton_Lt_Att_Pi0_Max = 0;
    fCalib_Proton_Lt_Att_Prompt_Min = 0;
    fCalib_Proton_Lt_Att_Prompt_Max = 0;
    fCalib_Proton_Lt_Att_BG1_Min = 0;
    fCalib_Proton_Lt_Att_BG1_Max = 0;
    fCalib_Proton_Lt_Att_BG2_Min = 0;
    fCalib_Proton_Lt_Att_BG2_Max = 0;
    fCalib_Proton_Lt_Att_MM_Min = 0;
    fCalib_Proton_Lt_Att_MM_Max = 0;
    fCalib_Proton_Lt_Att_CB_FGenCorr = 0;

    // photon resolutions
    fCalib_Photon_Res = 0;

    // particle resolutions
    fCalib_Particle_Res = 0;

    // bad scaler reads
    fCalib_BadScR = 0;

    // PWO check
    fCalib_PWO_Check = 0;

    // PWO pedestal
    fCalib_PWO_Ped = 0;
    fCalib_PWO_MaxRing = 0;

    taps_time_cut_min = -std::numeric_limits<double>::infinity();
    taps_time_cut_max =  std::numeric_limits<double>::infinity();
    taps_time_cut_max_ring = 1000;
}

//______________________________________________________________________________
TA2MyCaLib::~TA2MyCaLib()
{
    // Destructor.
    
}

bool TA2MyCaLib::TAPSTimeCutOK(const TOA2DetParticle* particle) const {
    return (particle->GetTime() >= taps_time_cut_min && particle->GetTime() <= taps_time_cut_max);
}

bool TA2MyCaLib::isInnerTAPSArea(const TOA2DetParticle* particle) const
{
    return TOA2Detector::GetTAPSRing(particle->GetCentralElement(), fTAPSType) <= taps_time_cut_max_ring;
}

UInt_t TA2MyCaLib::GetTAPSSectorElement(const UInt_t elem) {
    return elem % 72;
}


//______________________________________________________________________________
void TA2MyCaLib::SetConfig(Char_t* line, Int_t key)
{
    // Read in analysis configuration parameters and configure the class. 
 
    Bool_t error = kFALSE;
    
    switch (key)
    {
        case ECALIB_TARGET_POS:
            // Enable target position calibration
            if (sscanf(line, "%d", &fCalib_Target_Position) != 1) error = kTRUE;
            if (fCalib_Target_Position) fNCalib++;
            break;
        case ECALIB_CB_ENERGY:
            // Enable CB energy calibration
            if (sscanf(line, "%d", &fCalib_CB_Energy) != 1) error = kTRUE;
            if (fCalib_CB_Energy) fNCalib++;
            break;
        case ECALIB_CB_QUAD:
            // Enable CB quadratic energy correction
            if (sscanf(line, "%d%lf%lf%lf%lf", &fCalib_CB_Quad, 
                       &fCalib_CB_Quad_Pi0_Min, &fCalib_CB_Quad_Pi0_Max,
                       &fCalib_CB_Quad_Eta_Min, &fCalib_CB_Quad_Eta_Max) != 5) error = kTRUE;
            if (fCalib_CB_Quad) fNCalib++;
            break;
        case ECALIB_CB_TIME:
            // Enable CB time calibration
            if (sscanf(line, "%d", &fCalib_CB_Time) != 1) error = kTRUE;
            if (fCalib_CB_Time) fNCalib++;
            break;
        case ECALIB_CB_RISETIME:
            // Enable CB rise time calibration
            if (sscanf(line, "%d%lf%lf", &fCalib_CB_RiseTime, 
                       &fCalib_CB_RiseTime_Pi0_Min, &fCalib_CB_RiseTime_Pi0_Max) != 3) error = kTRUE;
            if (fCalib_CB_RiseTime) fNCalib++;
            break;
        case ECALIB_CB_WALK:
            // Enable CB time walk calibration
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_CB_Walk,
                       &fCalib_CB_Walk_Pi0_Min,    &fCalib_CB_Walk_Pi0_Max,
                       &fCalib_CB_Walk_Prompt_Min, &fCalib_CB_Walk_Prompt_Max,
                       &fCalib_CB_Walk_BG1_Min,    &fCalib_CB_Walk_BG1_Max,
                       &fCalib_CB_Walk_BG2_Min,    &fCalib_CB_Walk_BG2_Max,
                       &fCalib_CB_Walk_MM_Min,     &fCalib_CB_Walk_MM_Max) != 11) error = kTRUE;
            if (fCalib_CB_Walk) fNCalib++;
            break;
        case ECALIB_CB_WALK_EPT:
            // Enable CB time walk calibration for EPT
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf",
                       &fCalib_CB_Walk_EPT,
                       &fCalib_CB_Walk_EPT_Pi0_Min, &fCalib_CB_Walk_EPT_Pi0_Max,
                       &fCalib_CB_Walk_EPT_Phi_Min, &fCalib_CB_Walk_EPT_Phi_Max,
                       &fCalib_CB_Walk_EPT_ProtonE_Min, &fCalib_CB_Walk_EPT_ProtonE_Max) != 7) error = kTRUE;
            if (fCalib_CB_Walk_EPT) fNCalib++;
            break;

        case ECALIB_CB_PROTON_ECORR:
            // Enable CB proton energy correction
            if (sscanf(line, "%d", &fCalib_CB_Proton_ECorr) != 1) error = kTRUE;
            if (fCalib_CB_Proton_ECorr) fNCalib++;
            break;
        case ECALIB_CBTAPS_LED:
            // Enable CB-TAPS LED calibration
            if (sscanf(line, "%d", &fCalib_CBTAPS_LED) != 1) error = kTRUE;
            if (fCalib_CBTAPS_LED) fNCalib++;
            break;
        case ECALIB_TAPS_ENERGY:
            // Enable TAPS energy calibration
            if (sscanf(line, "%d", &fCalib_TAPS_Energy) != 1) error = kTRUE;
            if (fCalib_TAPS_Energy) fNCalib++;
            break;
        case ECALIB_TAPS_ENERGY_BG_SUBTR:
            // Enable TAPS energy calibration (BG subtr.)
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_TAPS_Energy_BG_Subtr,
                       &fCalib_TAPS_Energy_Prompt_Min, &fCalib_TAPS_Energy_Prompt_Max,
                       &fCalib_TAPS_Energy_BG1_Min,    &fCalib_TAPS_Energy_BG1_Max,
                       &fCalib_TAPS_Energy_BG2_Min,    &fCalib_TAPS_Energy_BG2_Max,
                       &fCalib_TAPS_Energy_MM_Min,     &fCalib_TAPS_Energy_MM_Max) != 9) error = kTRUE;
            if (fCalib_TAPS_Energy_BG_Subtr) fNCalib++;
            break;
        case ECALIB_TAPS_TIME:
            // Enable TAPS time calibration
            if (sscanf(line, "%d", &fCalib_TAPS_Time) != 1) error = kTRUE;
            if (fCalib_TAPS_Time) fNCalib++;
            break;
        case ECALIB_TAPS_QUAD:
            // Enable TAPS quadratic energy correction
            if (sscanf(line, "%d%lf%lf%lf%lf", &fCalib_TAPS_Quad, 
                       &fCalib_TAPS_Quad_Pi0_Min, &fCalib_TAPS_Quad_Pi0_Max,
                       &fCalib_TAPS_Quad_Eta_Min, &fCalib_TAPS_Quad_Eta_Max) != 6) error = kTRUE;
            if (fCalib_TAPS_Quad) fNCalib++;
            break;
        case ECALIB_TAPS_PSA:
            // Enable TAPS PSA calibration
            if (sscanf(line, "%d", &fCalib_TAPS_PSA) != 1) error = kTRUE;
            if (fCalib_TAPS_PSA) fNCalib++;
            break;
        case ECALIB_TAPS_LED:
            // Enable TAPS LED calibration
            if (sscanf(line, "%d", &fCalib_TAPS_LED) != 1) error = kTRUE;
            if (fCalib_TAPS_LED) fNCalib++;
            break;
        case ECALIB_TAPS_PROTON_ECORR:
            // Enable TAPS proton energy correction
            if (sscanf(line, "%d", &fCalib_TAPS_Proton_ECorr) != 1) error = kTRUE;
            if (fCalib_TAPS_Proton_ECorr) fNCalib++;
            break;
        case ECALIB_PID_PHI:
            // Enable PID Phi calibration
            if (sscanf(line, "%d", &fCalib_PID_Phi) != 1) error = kTRUE;
            if (fCalib_PID_Phi) fNCalib++;
            break;
        case ECALIB_PID_ENERGY:
            // Enable PID energy calibration
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_PID_Energy,
                       &fCalib_PID_Energy_Pi0_Min,    &fCalib_PID_Energy_Pi0_Max,
                       &fCalib_PID_Energy_Prompt_Min, &fCalib_PID_Energy_Prompt_Max,
                       &fCalib_PID_Energy_BG1_Min,    &fCalib_PID_Energy_BG1_Max,
                       &fCalib_PID_Energy_BG2_Min,    &fCalib_PID_Energy_BG2_Max,
                       &fCalib_PID_Energy_MM_Min,     &fCalib_PID_Energy_MM_Max) != 11) error = kTRUE;
             if (fCalib_PID_Energy) fNCalib++;
            break;
        case ECALIB_PID_ENERGY_TRAD:
            // Enable PID energy calibration (traditional)
            if (sscanf(line, "%d", &fCalib_PID_Energy_Trad) != 1) error = kTRUE;
            if (fCalib_PID_Energy_Trad) fNCalib++;
            break;
        case ECALIB_PID_DROOP:
            // Enable PID droop calibration
            if (sscanf(line, "%d", &fCalib_PID_Droop) != 1) error = kTRUE;
            if (fCalib_PID_Droop) fNCalib++;
            break;
        case ECALIB_PID_TIME:
            // Enable PID time calibration
            if (sscanf(line, "%d", &fCalib_PID_Time) != 1) error = kTRUE;
            if (fCalib_PID_Time) fNCalib++;
            break;
        case ECALIB_PID_EFF:
            // Enable PID efficiency calibration
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_PID_Eff,
                       &fCalib_PID_Eff_Pi0_Min,    &fCalib_PID_Eff_Pi0_Max,
                       &fCalib_PID_Eff_Prompt_Min, &fCalib_PID_Eff_Prompt_Max,
                       &fCalib_PID_Eff_BG1_Min,    &fCalib_PID_Eff_BG1_Max,
                       &fCalib_PID_Eff_BG2_Min,    &fCalib_PID_Eff_BG2_Max,
                       &fCalib_PID_Eff_MM_Min,     &fCalib_PID_Eff_MM_Max) != 11) error = kTRUE;
            if (fCalib_PID_Eff) fNCalib++;
            break;
        case ECALIB_VETO_CORR:
            // Enable veto-crystal correlation
            if (sscanf(line, "%d", &fCalib_Veto_Corr) != 1) error = kTRUE;
            if (fCalib_Veto_Corr) fNCalib++;
            break;
        case ECALIB_VETO_ENERGY:
            // Enable Veto energy calibration
            if (sscanf(line, "%d", &fCalib_Veto_Energy) != 1) error = kTRUE;
            if (fCalib_Veto_Energy) fNCalib++;
            break;
	case ECALIB_VETO_TIME:
            // Enable Veto time calibration
            if (sscanf(line, "%d", &fCalib_Veto_Time) != 1) error = kTRUE;
            if (fCalib_Veto_Time) fNCalib++;
            break;
        case ECALIB_VETO_EFF:
            // Enable Veto efficiency calibration
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_Veto_Eff,
                       &fCalib_Veto_Eff_Pi0_Min,    &fCalib_Veto_Eff_Pi0_Max,
                       &fCalib_Veto_Eff_Prompt_Min, &fCalib_Veto_Eff_Prompt_Max,
                       &fCalib_Veto_Eff_BG1_Min,    &fCalib_Veto_Eff_BG1_Max,
                       &fCalib_Veto_Eff_BG2_Min,    &fCalib_Veto_Eff_BG2_Max,
                       &fCalib_Veto_Eff_MM_Min,     &fCalib_Veto_Eff_MM_Max) != 11) error = kTRUE;
            if (fCalib_Veto_Eff) fNCalib++;
            break;
        case ECALIB_TAGGER_TIME:
            // Enable tagger time calibration
            if (sscanf(line, "%d", &fCalib_Tagger_Time) != 1) error = kTRUE;
            if (fCalib_Tagger_Time) fNCalib++;
            break;
        case ECALIB_PROTON_LT_ATT:
            Char_t tmp[256];
            Int_t nLoad;

            // Enable proton light attenuation correction
            if (sscanf(line, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
                       &fCalib_Proton_Lt_Att,
                       &fCalib_Proton_Lt_Att_Pi0_Min,    &fCalib_Proton_Lt_Att_Pi0_Max,
                       &fCalib_Proton_Lt_Att_Prompt_Min, &fCalib_Proton_Lt_Att_Prompt_Max,
                       &fCalib_Proton_Lt_Att_BG1_Min,    &fCalib_Proton_Lt_Att_BG1_Max,
                       &fCalib_Proton_Lt_Att_BG2_Min,    &fCalib_Proton_Lt_Att_BG2_Max,
                       &fCalib_Proton_Lt_Att_MM_Min,     &fCalib_Proton_Lt_Att_MM_Max) != 11) error = kTRUE;
            fCalib_Proton_Lt_Att_CB_FGenCorr = new TGraph*[gMyPhysics_MaxNaI];
            nLoad = 0;
            for (Int_t i = 0; i < gMyPhysics_MaxNaI; i++)
            {
                sprintf(tmp, "ProtonECorr_%03d", i);
                if (TOLoader::LoadObject("/usr/users/werthm/src/ROOT/macros/calibration/CB/Proton_Energy_Corr_CB.root", 
                                         tmp, &fCalib_Proton_Lt_Att_CB_FGenCorr[i], "", "q")) nLoad++;
            }
            Info("SetConfig", "Loaded inverse proton energy loss for %d CB detectors", nLoad);
            if (fCalib_Proton_Lt_Att) fNCalib++;
            break;
        case ECALIB_PHOTON_RES:
            // Enable photon resolutions
            if (sscanf(line, "%d", &fCalib_Photon_Res) != 1) error = kTRUE;
            if (fCalib_Photon_Res) fNCalib++;
            break;
        case ECALIB_PARTICLE_RES:
            // Enable particle resolutions
            if (sscanf(line, "%d", &fCalib_Particle_Res) != 1) error = kTRUE;
            if (fCalib_Particle_Res) 
            {
                // open the output file
                OpenOutputFile();
                fOutputFile->cd();

                // create event tree for CB
                fCalib_Particle_Res_Tree_CB = new TTree("CaLib_Particle_Res_CB", "Particle resolution event tree for CB");
                fCalib_Particle_Res_Tree_CB->Branch("theta_gen", &fCalib_Particle_Res_Theta_Gen, "theta_gen/F");
                fCalib_Particle_Res_Tree_CB->Branch("theta_det", &fCalib_Particle_Res_Theta_Det, "theta_det/F");
                fCalib_Particle_Res_Tree_CB->Branch("phi_gen", &fCalib_Particle_Res_Phi_Gen, "phi_gen/F");
                fCalib_Particle_Res_Tree_CB->Branch("phi_det", &fCalib_Particle_Res_Phi_Det, "phi_det/F");
                fCalib_Particle_Res_Tree_CB->Branch("energy_gen", &fCalib_Particle_Res_Energy_Gen, "energy_gen/F");
                fCalib_Particle_Res_Tree_CB->Branch("energy_det", &fCalib_Particle_Res_Energy_Det, "energy_det/F");
                fCalib_Particle_Res_Tree_CB->Branch("index_det", &fCalib_Particle_Res_Module_Det, "index_det/I");
                fCalib_Particle_Res_Tree_CB->SetAutoFlush(-5000000);
                
                // create event tree for TAPS
                fCalib_Particle_Res_Tree_TAPS = new TTree("CaLib_Particle_Res_TAPS", "Particle resolution event tree for TAPS");
                fCalib_Particle_Res_Tree_TAPS->Branch("theta_gen", &fCalib_Particle_Res_Theta_Gen, "theta_gen/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("theta_det", &fCalib_Particle_Res_Theta_Det, "theta_det/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("phi_gen", &fCalib_Particle_Res_Phi_Gen, "phi_gen/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("phi_det", &fCalib_Particle_Res_Phi_Det, "phi_det/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("energy_gen", &fCalib_Particle_Res_Energy_Gen, "energy_gen/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("energy_det", &fCalib_Particle_Res_Energy_Det, "energy_det/F");
                fCalib_Particle_Res_Tree_TAPS->Branch("index_det", &fCalib_Particle_Res_Module_Det, "index_det/I");
                fCalib_Particle_Res_Tree_TAPS->SetAutoFlush(-5000000);
 
                fNCalib++;
            }
            break;
        case ECALIB_BADSCR:
            // disable bad scaler read skipping for the calibration
            if (fUseBadScalerReads)
            {
                fUseBadScalerReads = kFALSE;
                Info("SetConfig", "Bad scaler reads were automatically disabled.");
            }
            // Enable bad scaler read
            if (sscanf(line, "%d", &fCalib_BadScR) != 1) error = kTRUE;
            if (fCalib_BadScR) fNCalib++;
            break;
        case ECALIB_PWO_CHECK:
            // Enable PWO check
            if (sscanf(line, "%d", &fCalib_PWO_Check) != 1) error = kTRUE;
            if (fCalib_PWO_Check) fNCalib++;
            break;
        case ECALIB_PWO_PED:
            // Enable PWO pedestal calibration
            if (sscanf(line, "%d", &fCalib_PWO_Ped) != 1) error = kTRUE;
            if (fCalib_PWO_Ped) fNCalib++;
            break;

        case ECALIB_TAPS_TIME_CUT:
            if( sscanf(line, "%lf %lf", &taps_time_cut_min, &taps_time_cut_max) != 2) error = kTRUE;
            break;

        case ECALIB_TAPS_TIME_CUT_MAXRING:
            if( sscanf(line, "%d", &taps_time_cut_max_ring) != 1) error = kTRUE;
            break;

        default:
            // default parent class SetConfig()
            TA2MyPhysics::SetConfig(line, key);
            break;
    }

    if (error) PrintError(line);
}

//______________________________________________________________________________ 
void TA2MyCaLib::PostInit()
{
    // Do some further initialisation after all setup parameters have been read in.
    
    Char_t name[256];
    Char_t title[256];
    
    // call PostInit of parent class
    TA2MyPhysics::PostInit();
    
    // get number of detector elements
    fLadder ? fNelemTAGG = fLadder->GetNelement() : fNelemTAGG = 0;
    fNaI ? fNelemCB = fNaI->GetNelement() : fNelemCB = 0;
    fBaF2PWO ? fNelemTAPS = fBaF2PWO->GetNelement() : fNelemTAPS = 0;
    fPID ? fNelemPID = fPID->GetNelement() : fNelemPID = 0;
    fVeto ? fNelemVeto = fVeto->GetNelement() : fNelemVeto = 0;
 
    // show information about the enabled calibration histogram creation
    printf("\n\n  ****** ENABLED CALIBRATION HISTOGRAMS ******\n\n");
    printf("   %d calibration(s) enabled.\n\n", fNCalib);
    if (fCalib_Target_Position) printf("   - target position\n");
    if (fCalib_CB_Energy)       printf("   - CB energy\n");
    if (fCalib_CB_Quad)         
    {
        printf("   - CB quadratic energy\n");
        printf("     -> pi0 invariant mass cut  : %8.2f to %8.2f MeV\n", fCalib_CB_Quad_Pi0_Min, fCalib_CB_Quad_Pi0_Max);
        printf("     -> eta invariant mass cut  : %8.2f to %8.2f MeV\n", fCalib_CB_Quad_Eta_Min, fCalib_CB_Quad_Eta_Max);
    }
    if (fCalib_CB_Time)     printf("   - CB time\n");
    if (fCalib_CB_RiseTime) 
    {
        printf("   - CB rise time\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_CB_RiseTime_Pi0_Min, fCalib_CB_RiseTime_Pi0_Max);
    }
    if (fCalib_CB_Walk)         
    {
        printf("   - CB time walk\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_CB_Walk_Pi0_Min, fCalib_CB_Walk_Pi0_Max);
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_CB_Walk_MM_Min, fCalib_CB_Walk_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n", fCalib_CB_Walk_Prompt_Min, fCalib_CB_Walk_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n", fCalib_CB_Walk_BG1_Min, fCalib_CB_Walk_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n", fCalib_CB_Walk_BG2_Min, fCalib_CB_Walk_BG2_Max);
    }
    if (fCalib_CB_Walk_EPT)
    {
        printf("   - CB time walk for EPT\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_CB_Walk_EPT_Pi0_Min, fCalib_CB_Walk_EPT_Pi0_Max);
        printf("     -> pi0/proton phi cut         : %8.2f to %8.2f Degree\n", fCalib_CB_Walk_EPT_Phi_Min, fCalib_CB_Walk_EPT_Phi_Max);
        printf("     -> proton energy cut          : %8.2f to %8.2f MeV\n", fCalib_CB_Walk_EPT_ProtonE_Min, fCalib_CB_Walk_EPT_ProtonE_Max);
    }
    if (fCalib_CB_Proton_ECorr) printf("   - CB proton energy correction\n");
    if (fCalib_CBTAPS_LED)  printf("   - CB-TAPS LED\n");
    if (fCalib_TAPS_Energy) {
        printf("   - TAPS energy\n");
        printf("   - Time Cut: %lf - %lf ns\n", taps_time_cut_min, taps_time_cut_max);
        printf("     - for Ring Numbers smaller than %d\n", taps_time_cut_max_ring);
    }

    if (fCalib_TAPS_Energy_BG_Subtr)         
    {
        printf("   - TAPS energy (BG. subtr.)\n");
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_TAPS_Energy_MM_Min, fCalib_TAPS_Energy_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n", fCalib_TAPS_Energy_Prompt_Min, fCalib_TAPS_Energy_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n", fCalib_TAPS_Energy_BG1_Min, fCalib_TAPS_Energy_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n", fCalib_TAPS_Energy_BG2_Min, fCalib_TAPS_Energy_BG2_Max);
    }
    if (fCalib_TAPS_Time)   printf("   - TAPS time\n");
    if (fCalib_TAPS_Quad)         
    {
        printf("   - TAPS quadratic energy\n");
        printf("     -> pi0 invariant mass cut  : %8.2f to %8.2f MeV\n", fCalib_TAPS_Quad_Pi0_Min, fCalib_TAPS_Quad_Pi0_Max);
        printf("     -> eta invariant mass cut  : %8.2f to %8.2f MeV\n", fCalib_TAPS_Quad_Eta_Min, fCalib_TAPS_Quad_Eta_Max);
    }
    if (fCalib_TAPS_PSA)    printf("   - TAPS PSA\n");
    if (fCalib_TAPS_LED)    printf("   - TAPS LED\n");
    if (fCalib_TAPS_Proton_ECorr) printf("   - TAPS proton energy correction\n");
    if (fCalib_PID_Phi)     printf("   - PID Phi\n");
    if (fCalib_PID_Energy)
    {
        printf("   - PID energy\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_PID_Energy_Pi0_Min, fCalib_PID_Energy_Pi0_Max);
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_PID_Energy_MM_Min, fCalib_PID_Energy_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n", fCalib_PID_Energy_Prompt_Min, fCalib_PID_Energy_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n", fCalib_PID_Energy_BG1_Min, fCalib_PID_Energy_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n", fCalib_PID_Energy_BG2_Min, fCalib_PID_Energy_BG2_Max);
    }
    if (fCalib_PID_Energy_Trad)
    {
        printf("   - PID energy (traditional)\n");
    }
    if (fCalib_PID_Droop)   printf("   - PID droop\n");
    if (fCalib_PID_Time)    printf("   - PID time\n");
    if (fCalib_PID_Eff)
    {
        printf("   - PID efficiency\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_PID_Eff_Pi0_Min, fCalib_PID_Eff_Pi0_Max);
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_PID_Eff_MM_Min, fCalib_PID_Eff_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n", fCalib_PID_Eff_Prompt_Min, fCalib_PID_Eff_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n", fCalib_PID_Eff_BG1_Min, fCalib_PID_Eff_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n", fCalib_PID_Eff_BG2_Min, fCalib_PID_Eff_BG2_Max);
    }
    if (fCalib_Veto_Corr)   printf("   - veto-crystal correlation\n");
    if (fCalib_Veto_Energy) printf("   - Veto energy\n");
    if (fCalib_Veto_Time)   printf("   - Veto time\n");
    if (fCalib_Veto_Eff)
    {
        printf("   - Veto efficiency\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_Veto_Eff_Pi0_Min, fCalib_Veto_Eff_Pi0_Max);
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_Veto_Eff_MM_Min, fCalib_Veto_Eff_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n", fCalib_Veto_Eff_Prompt_Min, fCalib_Veto_Eff_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n", fCalib_Veto_Eff_BG1_Min, fCalib_Veto_Eff_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n", fCalib_Veto_Eff_BG2_Min, fCalib_Veto_Eff_BG2_Max);
    }
    if (fCalib_Tagger_Time) printf("   - Tagger time\n");
    if (fCalib_Proton_Lt_Att)
    {
        printf("   - Proton light attenuation correction\n");
        printf("     -> pi0 invariant mass cut     : %8.2f to %8.2f MeV\n", fCalib_Proton_Lt_Att_Pi0_Min, fCalib_Proton_Lt_Att_Pi0_Max);
        printf("     -> missing mass cut           : %8.2f to %8.2f MeV\n", fCalib_Proton_Lt_Att_MM_Min, fCalib_Proton_Lt_Att_MM_Max);
        printf("     -> Random subtraction prompt  : %8.2f to %8.2f ns\n",  fCalib_Proton_Lt_Att_Prompt_Min, fCalib_Proton_Lt_Att_Prompt_Max);
        printf("     -> Random subtraction BG 1    : %8.2f to %8.2f ns\n",  fCalib_Proton_Lt_Att_BG1_Min, fCalib_Proton_Lt_Att_BG1_Max);
        printf("     -> Random subtraction BG 2    : %8.2f to %8.2f ns\n",  fCalib_Proton_Lt_Att_BG2_Min, fCalib_Proton_Lt_Att_BG2_Max);
    }
    if (fCalib_Photon_Res)   printf("   - Photon resolutions\n");
    if (fCalib_Particle_Res) printf("   - Particle resolutions\n");
    if (fCalib_BadScR)       printf("   - Bad scaler reads\n");
    if (fCalib_PWO_Check)    printf("   - PWO check\n");
    if (fCalib_PWO_Ped)      printf("   - PWO pedestal\n");
     
    printf("\n");
    
    // prepare for target position calibration
    if (fCalib_Target_Position)
    {
        fHCalib_CB_IM_TargPos = new TH2F("CaLib_CB_IM_TargPos", "CaLib_CB_IM_TargPos;2#gamma invariant mass [MeV];target position [cm]",      
                                         1000, 0, 1000, 200, -10, 10);
    }
    
    // prepare for CB energy calibration
    if (fCalib_CB_Energy)
    {
        fHCalib_CB_IM          = new TH2F("CaLib_CB_IM", "CaLib_CB_IM;2#gamma invariant mass [MeV];CB element",      
                                          500, 0, 1000, fNelemCB, 0, fNelemCB);
        fHCalib_CB_IM_Neut     = new TH2F("CaLib_CB_IM_Neut", "CaLib_CB_IM_Neut;2#gamma invariant mass [MeV];CB element", 
                                          500, 0, 1000, fNelemCB, 0, fNelemCB);
    }
    
    // prepare for CB quadratic energy correction
    if (fCalib_CB_Quad)
    {   
        fHCalib_CB_Quad_IM         = new TH2F("CaLib_CB_Quad_IM", 
				              "CaLib_CB_Quad_IM;2#gamma invariant mass [MeV];CB element",
					      1000, 0, 1000, fNelemCB, 0, fNelemCB);
        fHCalib_CB_Quad_Pi0_Mean_E = new TH2F("CaLib_CB_Quad_Pi0_Mean_E", 
				              "CaLib_CB_Quad_Pi0_Mean_E;Mean photon energy of #pi^{0} [MeV];CB element", 
					      1000, 0, 1000, fNelemCB, 0, fNelemCB);
        fHCalib_CB_Quad_Eta_Mean_E = new TH2F("CaLib_CB_Quad_Eta_Mean_E", 
					      "CaLib_CB_Quad_Eta_Mean_E;Mean photon energy of #eta [MeV];CB element", 
                                              1000, 0, 1000, fNelemCB, 0, fNelemCB);
    }
    
    // prepare for CB time calibration
    if (fCalib_CB_Time)
    {
        fHCalib_CB_Time      = new TH2F("CaLib_CB_Time", "CaLib_CB_Time;CB cluster time [ns];CB element",
                                        2000, -100, 100, fNelemCB, 0, fNelemCB);
        fHCalib_CB_Time_Neut = new TH2F("CaLib_CB_Time_Neut", "CaLib_CB_Time_Neut;CB neutral cluster time [ns];CB element", 
                                        2000, -100, 100, fNelemCB, 0, fNelemCB);
        // dt=0.11771 ns
        fHCalib_CB_Time_Ind = new TH2F("CaLib_CB_Time_Ind", "CaLib_CB_Time_Ind;CB neutral cluster time [ns];CB element", 
                                        16990, -999.94645, 999.94645, fNelemCB, 0, fNelemCB);
    }
    
    // prepare for CB rise time calibration
    if (fCalib_CB_RiseTime)
    {
        fHCalib_CB_RiseTime = new TH2F("CaLib_CB_RiseTime", "CaLib_CB_RiseTime;CB cluster time - tagger time [ns];CB element",
                                       1000, -100, 100, fNelemCB, 0, fNelemCB);
    }
    
    // prepare for CB time walk calibration
    if (fCalib_CB_Walk)
    {
        fHCalib_CB_Walk_IM           = new TH1F("CaLib_CB_Walk_IM", "CaLib_CB_Walk_IM;2#gamma invariant mass [MeV];Counts",
                                                1000, 0, 1000);
        fHCalib_CB_Walk_MM           = new TH2F("CaLib_CB_Walk_MM", "CaLib_CB_Walk_MM;Missing mass [MeV];Tagger element",
                                                2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_CB_Walk_Rand_Time_CB = new TH1F("CaLib_CB_Walk_Rand_Time_CB", "CaLib_CB_Walk_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                8000, -1000, 1000);
        
        fHCalib_CB_Walk_E_T = new TH2*[fNelemCB];

        for (Int_t i = 0; i < fNelemCB; i++)
        {
            sprintf(name, "CaLib_CB_Walk_E_T_%03d", i);
            sprintf(title, "CaLib_CB_Walk_E_T_%03d;CB energy [MeV];CB time [ns]", i);
            fHCalib_CB_Walk_E_T[i] = new TH2F(name, title, 400, 0, 400, 300, -100, 200);
        }
    }

    // prepare for CB time walk for EPT calibration
    if (fCalib_CB_Walk_EPT)
    {
        fHCalib_CB_Walk_EPT_IM           = new TH1F("CaLib_CB_Walk_EPT_IM", "CaLib_CB_Walk_EPT_IM;2#gamma invariant mass [MeV];Counts",
                                                1000, 0, 1000);
        fHCalib_CB_Walk_EPT_Phi          = new TH1F("CaLib_CB_Walk_EPT_Phi", "CaLib_CB_Walk_EPT_Phi;pi0/proton phi diff;Counts",
                                                1080, 0, 360);

        fHCalib_CB_Walk_EPT_E_T = new TH2*[fNelemCB];

        for (Int_t i = 0; i < fNelemCB; i++)
        {
            sprintf(name, "CaLib_CB_Walk_EPT_E_T_%03d", i);
            sprintf(title, "CaLib_CB_Walk_EPT_E_T_%03d;CB energy [MeV];CB time [ns]", i);
            fHCalib_CB_Walk_EPT_E_T[i] = new TH2F(name, title, 400, 0, 400, 300, -100, 200);
        }
    }

    // prepare for CB proton energy correction
    if (fCalib_CB_Proton_ECorr)
    {
        fHCalib_CB_Proton_ECorr = new TH2*[fNelemCB];
        fHCalib_CB_Proton_ECorr_Inv = new TH2*[fNelemCB];
        fHCalib_CB_Proton_ECorr_Check = new TH2F("Calib_CB_Proton_ECorr_Check", "Calib_CB_Proton_ECorr_Check;E_{diff} [MeV];CB element",
                                                 400, -200, 200, fNelemCB, 0, fNelemCB);

        for (Int_t i = 0; i < fNelemCB; i++)
        {
            sprintf(name, "Calib_CB_Proton_ECorr_%03d", i);
            sprintf(title, "Calib_CB_Proton_ECorr_%03d;T^{c}_{p} [MeV];f^{corr}_{E}", i);
            fHCalib_CB_Proton_ECorr[i] = new TH2F(name, title, 250, 0, 500, 600, -1, 5);
            sprintf(name, "Calib_CB_Proton_ECorr_Inv_%03d", i);
            sprintf(title, "Calib_CB_Proton_ECorr_Inv_%03d;T^{v}_{p} [MeV];f^{corr}_{E}", i);
            fHCalib_CB_Proton_ECorr_Inv[i] = new TH2F(name, title, 250, 0, 500, 600, -1, 5);
        }
    }
     
    // prepare for CB-TAPS LED calibration
    if (fCalib_CBTAPS_LED)
    {
        fHCalib_CB_LED_M2    = new TH2F("CaLib_CB_LED_M2",    "CaLib_CB_LED_M2;CB energy [MeV];CB element", 1000, 0, 1000, fNelemCB, 0, fNelemCB);
        fHCalib_TAPS_LED_M2  = new TH2F("CaLib_TAPS_LED_M2",  "CaLib_TAPS_LED_M2;TAPS energy [MeV];TAPS element", 1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_CB_LED_M3    = new TH2F("CaLib_CB_LED_M3",    "CaLib_CB_LED_M3;CB energy [MeV];CB element", 1000, 0, 1000, fNelemCB, 0, fNelemCB);
        fHCalib_TAPS_LED_M3  = new TH2F("CaLib_TAPS_LED_M3",  "CaLib_TAPS_LED_M3;TAPS energy [MeV];TAPS element", 1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_LED2_M2 = new TH2F("CaLib_TAPS_LED2_M2", "CaLib_TAPS_LED2_M2;TAPS energy [MeV];TAPS element", 1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
    }
     
    // prepare for TAPS energy calibration
    if (fCalib_TAPS_Energy)
    {
        fHCalib_TAPS_IM_Neut     = new TH2F("CaLib_TAPS_IM_Neut", "CaLib_TAPS_IM_Neut;2#gamma invariant mass [MeV];TAPS element", 
                                            500, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TOF_TAPS         = new TH3F("CaLib_TOF_TAPS", "CaLib_TOF_TAPS;TAPS element;Tagger Element",
                                            2000, -10, 10, fNelemTAPS, 0, fNelemTAPS, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_TOF_TAPS_Neut         = new TH3F("CaLib_TOF_TAPS_Neut", "CaLib_TOF_TAPS_Neut; TAPS Element;Tagger element",
                                            2000, -10, 10, fNelemTAPS, 0, fNelemTAPS, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_TAPS_IM_Neut_1CB_1TAPS = new TH2F("CaLib_TAPS_IM_Neut_1CB_1TAPS", "CaLib_TAPS_IM_Neut_1CB_1TAPS;2#gamma invariant mass [MeV];TAPS element", 
                                                  500, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_IM_TAPS      = new TH2F("CaLib_TAPS_IM_TAPS", "CaLib_TAPS_IM_TAPS;2#gamma invariant mass [MeV];TAPS element", 
                                             500, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_IM_Neut_TAPS = new TH2F("CaLib_TAPS_IM_Neut_TAPS", "CaLib_TAPS_IM_Neut_TAPS;2#gamma invariant mass [MeV];TAPS element", 
                                             500, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_IM_Neut_TAPS_mult = new TH3F("CaLib_TAPS_IM_Neut_TAPS_mult", "IMgg vs TAPS det el vs nr CB + TAPS (in time)", 500, 0, 1000, fNelemTAPS, 0, fNelemTAPS, 10, 0, 10);
    }
    
    // prepare for TAPS energy calibration (BG subtr.)
    if (fCalib_TAPS_Energy_BG_Subtr)
    {
        fHCalib_TAPS_IM_Neut_BG_Subtr = new TH2F("CaLib_TAPS_IM_Neut_BG_Subtr", "CaLib_TAPS_IM_Neut_BG_Subtr;2#gamma invariant mass [MeV];TAPS element", 
                                                 500, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Energy_MM = new TH2F("CaLib_TAPS_Energy_MM", "CaLib_TAPS_Energy_MM;Missing mass [MeV];Tagger element",
                                          2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_TAPS_Energy_Rand_Time_CB = new TH1F("CaLib_TAPS_Energy_Rand_Time_CB", "CaLib_TAPS_Energy_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                    8000, -1000, 1000);
    }
    
    // prepare for TAPS time calibration
    if (fCalib_TAPS_Time)
    {
        fHCalib_TAPS_Time      = new TH2F("CaLib_TAPS_Time", "CaLib_TAPS_Time;TAPS cluster time [ns];TAPS element", 
                                          2000, -100, 100, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Time_Neut = new TH2F("CaLib_TAPS_Time_Neut", "CaLib_TAPS_Time_Neut;TAPS neutral cluster time [ns];TAPS element", 
                                          2000, -100, 100, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Time_Neut_IM_Cut = new TH2F("CaLib_TAPS_Time_Neut_IM_Cut", "CaLib_TAPS_Time_Neut_IM_Cut;TAPS neutral cluster time [ns];TAPS element", 
                                                 2000, -100, 100, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Time_Ind = new TH2F("CaLib_TAPS_Time_Ind", "CaLib_TAPS_Time_Ind;TAPS time [ns];TAPS element", 
                                         10000, -1000, 1000, fNelemTAPS, 0, fNelemTAPS);
    }
    
    // prepare for TAPS quadratic energy correction
    if (fCalib_TAPS_Quad)
    {   
        fHCalib_TAPS_Quad_IM         = new TH2F("CaLib_TAPS_Quad_IM", "CaLib_TAPS_Quad_IM;2#gamma invariant mass [MeV];TAPS element",
                                                1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Quad_Pi0_Mean_E = new TH2F("CaLib_TAPS_Quad_Pi0_Mean_E", "CaLib_TAPS_Quad_Pi0_Mean_E;Mean photon energy of #pi^{0} [MeV];TAPS element", 
                                                1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_Quad_Eta_Mean_E = new TH2F("CaLib_TAPS_Quad_Eta_Mean_E", "CaLib_TAPS_Quad_Eta_Mean_E;Mean photon energy of #eta [MeV];TAPS element",
                                                1000, 0, 1000, fNelemTAPS, 0, fNelemTAPS);
    }
    
    // prepare for TAPS PSA calibration
    if (fCalib_TAPS_PSA)
    {
        fHCalib_TAPS_PSAR_PSAA = new TH2*[fNelemTAPS];
        fHCalib_TAPS_PSAR_PSAA_SM = new TH2*[fNelemTAPS];

        for (Int_t i = 0; i < fNelemTAPS; i++)
        {
            sprintf(name, "CaLib_TAPS_PSAR_PSAA_%03d", i);
            sprintf(title, "CaLib_TAPS_PSAR_PSAA_%03d;PSA angle [deg];PSA radius [MeV]", i);
            fHCalib_TAPS_PSAR_PSAA[i] = new TH2F(name, title, 1800, 0, 180, 100, 0, 600);
            
            sprintf(name, "CaLib_TAPS_PSAR_PSAA_SM_%03d", i);
            sprintf(title, "CaLib_TAPS_PSAR_PSAA_SM_%03d;PSA angle [deg];PSA radius [MeV]", i);
            fHCalib_TAPS_PSAR_PSAA_SM[i] = new TH2F(name, title, 1800, 0, 180, 100, 0, 600);
        }
    }
    
    // prepare for TAPS LED calibration
    if (fCalib_TAPS_LED)
    {
        fHCalib_TAPS_LED_Norm = new TH2F("CaLib_TAPS_LED_Norm", "CaLib_TAPS_LED_Norm;Energy [MeV];TAPS element", 
                                         2000, 0, 2000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_LED_LED1 = new TH2F("CaLib_TAPS_LED_LED1", "CaLib_TAPS_LED_LED1;Energy [MeV];TAPS element", 
                                         2000, 0, 2000, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_TAPS_LED_LED2 = new TH2F("CaLib_TAPS_LED_LED2", "CaLib_TAPS_LED_LED2;Energy [MeV];TAPS element", 
                                         2000, 0, 2000, fNelemTAPS, 0, fNelemTAPS);
    }
    
    // prepare for TAPS proton energy correction
    if (fCalib_TAPS_Proton_ECorr)
    {
        fHCalib_TAPS_Proton_ECorr = new TH2*[fNelemTAPS];
        fHCalib_TAPS_Proton_ECorr_Inv = new TH2*[fNelemTAPS];
        fHCalib_TAPS_Proton_ECorr_Check = new TH2F("Calib_TAPS_Proton_ECorr_Check", "Calib_TAPS_Proton_ECorr_Check;E_{diff} [MeV];TAPS element",
                                                 400, -200, 200, fNelemTAPS, 0, fNelemTAPS);

        for (Int_t i = 0; i < fNelemTAPS; i++)
        {
            sprintf(name, "Calib_TAPS_Proton_ECorr_%03d", i);
            sprintf(title, "Calib_TAPS_Proton_ECorr_%03d;T^{c}_{p} [MeV];f^{corr}_{E}", i);
            fHCalib_TAPS_Proton_ECorr[i] = new TH2F(name, title, 250, 0, 500, 600, -1, 5);
            sprintf(name, "Calib_TAPS_Proton_ECorr_Inv_%03d", i);
            sprintf(title, "Calib_TAPS_Proton_ECorr_Inv_%03d;T^{v}_{p} [MeV];f^{corr}_{E}", i);
            fHCalib_TAPS_Proton_ECorr_Inv[i] = new TH2F(name, title, 250, 0, 500, 600, -1, 5);
        }
    }
  
    // prepare for PID Phi calibration
    if (fCalib_PID_Phi)
    {
        fHCalib_PID_CBPhi_ID        = new TH2F("CaLib_PID_CBPhi_ID", "CaLib_PID_CBPhi_ID;CB cluster #Phi angle [deg];PID element", 
                                               720, -360, 360, fNelemPID, 0, fNelemPID);
        fHCalib_PID_CBPhi_ID_1Cryst = new TH2F("CaLib_PID_CBPhi_ID_1Cryst", "CaLib_PID_CBPhi_ID_1Cryst;CB cluster #Phi angle [deg];PID element", 
                                               720, -360, 360, fNelemPID, 0, fNelemPID);
    }
    
    // prepare for PID energy/droop calibration
    if (fCalib_PID_Energy || fCalib_PID_Droop)
    {
        fHCalib_PID_Energy_IM             = new TH1F("CaLib_PID_Energy_IM", "CaLib_PID_Energy_IM;2#gamma invariant mass [MeV];Counts",
                                                     1000, 0, 1000);
        fHCalib_PID_Energy_MM             = new TH2F("CaLib_PID_Energy_MM", "CaLib_PID_Energy_MM;Missing mass [MeV];Tagger element",
                                                     2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_PID_Energy_Rand_Time_CB   = new TH1F("CaLib_PID_Energy_Rand_Time_CB", "CaLib_PID_Energy_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                     8000, -1000, 1000);
        fHCalib_PID_Energy_Rand_Time_TAPS = new TH1F("CaLib_PID_Energy_Rand_Time_TAPS", "CaLib_PID_Energy_Rand_Time_TAPS;TAPS-tagger time [ns];Counts", 
                                                     8000, -1000, 1000);
        fHCalib_PID_Energy_Pi0_Energy     = new TH1F("CaLib_PID_Energy_Pi0_Energy", "CaLib_PID_Energy_Pi0_Energy;#pi^{0} energy [MeV];Counts", 
                                                     1000, 0, 2000);
        fHCalib_PID_Energy_Pi0_Theta      = new TH1F("CaLib_PID_Energy_Pi0_Theta", "CaLib_PID_Energy_Pi0_Theta;#pi^{0} #theta [deg];Counts", 
                                                     180, 0, 180);
        fHCalib_PID_Energy_Proton_Energy  = new TH1F("CaLib_PID_Energy_Proton_Energy", "CaLib_PID_Energy_Proton_Energy;proton energy [MeV];Counts", 
                                                     500, 0, 1000);
        fHCalib_PID_Energy_Raw_PID_Energy     = new TH2F("CaLib_PID_Energy_Raw_PID_Energy", "CaLib_PID_Energy_Raw_PID_Energy;Raw PID energy [MeV];PID element", 
                                                         400, 0, 10, fNelemPID, 0, fNelemPID);
        fHCalib_PID_Energy_Proton_PID_Energy  = new TH2F("CaLib_PID_Energy_Proton_PID_Energy", "CaLib_PID_Energy_Proton_PID_Energy;Proton PID energy [MeV];PID element", 
                                                         200, 0, 10, fNelemPID, 0, fNelemPID);
        fHCalib_PID_dE_E = new TH3*[fNelemPID];
        for (Int_t i = 0; i < fNelemPID; i++)
        {
            if (fIsMC || fCalib_PID_Droop) 
            {
                sprintf(name, "CaLib_PID_dE_E_%03d", i);
                sprintf(title, "CaLib_PID_dE_E_%03d;CB cluster energy [MeV];PID energy [MeV];CB cluster theta [deg]", i);
                fHCalib_PID_dE_E[i] = new TH3F(name, title, 500, 0, 1000, 400, 0, 20, 72, 16, 160);
            }
            else 
            {
                sprintf(name, "CaLib_PID_dE_E_%03d", i);
                sprintf(title, "CaLib_PID_dE_E_%03d;CB cluster energy [MeV];PID energy [Channel];CB cluster theta [deg]", i);
                fHCalib_PID_dE_E[i] = new TH3F(name, title,  500, 0, 1000, 500, 0, 2000, 14, 20, 160);
            }
        }
    }

    // prepare for PID energy (traditional)
    if (fCalib_PID_Energy_Trad)
    {
        fHCalib_PID_dE_E_Trad = new TH3*[fNelemPID];
        for (Int_t i = 0; i < fNelemPID; i++)
        {
            sprintf(name, "CaLib_PID_dE_E_%03d", i);
            sprintf(title, "CaLib_PID_dE_E_%03d;CB cluster energy [MeV];PID energy [MeV];CB cluster theta [deg]", i);
            fHCalib_PID_dE_E_Trad[i] = new TH3F(name, title, 500, 0, 1000, 400, 0, 20, 1, 16, 160);
        }
    }

    // prepare for PID time calibration
    if (fCalib_PID_Time)
    {
        fHCalib_PID_Time = new TH2F("CaLib_PID_Time", "CaLib_PID_Time;PID time [ns];PID element", 
				    1000, -100, 100, fNelemPID, 0, fNelemPID);
        fHCalib_PID_Time_Ind = new TH2F("CaLib_PID_Time_Ind", "CaLib_PID_Time_Ind;PID time [ns];PID element", 
                                        1700, -200, 200, fNelemPID, 0, fNelemPID);
    }
    
    // prepare for PID efficiency calibration
    if (fCalib_PID_Eff)
    {
        fHCalib_PID_Eff_IM           = new TH1F("CaLib_PID_Eff_IM", "CaLib_PID_Eff_IM;2#gamma invariant mass [MeV];Counts",
                                                1000, 0, 1000);
        fHCalib_PID_Eff_MM           = new TH2F("CaLib_PID_Eff_MM", "CaLib_PID_Eff_MM;Missing mass [MeV];Tagger element",
                                                2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_PID_Eff_Rand_Time_CB = new TH1F("CaLib_PID_Eff_Rand_Time_CB", "CaLib_PID_Eff_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                 8000, -1000, 1000);
        fHCalib_PID_Eff_ThetaDiff    = new TH1F("CaLib_PID_Eff_ThetaDiff", "CaLib_PID_Eff_ThetaDiff", 180, 0, 180);
        fHCalib_PID_Eff_Hit          = new TH1F("CaLib_PID_Eff_Hit", "CaLib_PID_Eff_Hit", 36, -180, 180);
        fHCalib_PID_Eff_NoHit        = new TH1F("CaLib_PID_Eff_NoHit", "CaLib_PID_Eff_NoHit", 36, -180, 180);
    }

    // prepare for veto-crystal correlation
    if (fCalib_Veto_Corr)
    {
        fHCalib_Veto_Corr = new TH2F("CaLib_Veto_Corr", "CaLib_Veto_Corr;Veto element;TAPS element", 
                                     fNelemVeto, 0, fNelemVeto, fNelemTAPS, 0, fNelemTAPS);
    }
    
    // prepare for Veto energy calibration
    if (fCalib_Veto_Energy)
    {
        fHCalib_Veto_dE_E = new TH2*[fNelemVeto];
        
        for (Int_t i = 0; i < fNelemVeto; i++)
        {
            sprintf(name, "CaLib_Veto_dE_E_%03d", i);
            sprintf(title, "CaLib_Veto_dE_E_%03d;TAPS cluster energy [MeV];Veto energy [MeV]", i);
            fHCalib_Veto_dE_E[i] = new TH2F(name, title, 1000, 0, 1000, 400, 0, 20);
        }
    }
     
    // prepare for Veto time calibration
    if (fCalib_Veto_Time)
    {
        fHCalib_Veto_Time = new TH2F("CaLib_Veto_Time", "CaLib_Veto_Time;Veto time [ns];Veto element", 
				     1000, -100, 100, fNelemVeto, 0, fNelemVeto);
        fHCalib_Veto_Time_Ind = new TH2F("CaLib_Veto_Time_Ind", "CaLib_Veto_Time_Ind;Veto time [ns];Veto element", 
                                         4000, -1000, 1000, fNelemVeto, 0, fNelemVeto);
    }
    
    // prepare for Veto efficiency calibration
    if (fCalib_Veto_Eff)
    {
        fHCalib_Veto_Eff_IM           = new TH1F("CaLib_Veto_Eff_IM", "CaLib_Veto_Eff_IM;2#gamma invariant mass [MeV];Counts",
                                                 1000, 0, 1000);
        fHCalib_Veto_Eff_MM           = new TH2F("CaLib_Veto_Eff_MM", "CaLib_Veto_Eff_MM;Missing mass [MeV];Tagger element",
                                                 2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_Veto_Eff_Rand_Time_CB = new TH1F("CaLib_Veto_Eff_Rand_Time_CB", "CaLib_Veto_Eff_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                  8000, -1000, 1000);
        fHCalib_Veto_Eff_ThetaDiff    = new TH1F("CaLib_Veto_Eff_ThetaDiff", "CaLib_Veto_Eff_ThetaDiff", 180, 0, 180);
        fHCalib_Veto_Eff_Hit          = new TH1F("CaLib_Veto_Eff_Hit", "CaLib_Veto_Eff_Hit", fNelemTAPS, 0, fNelemTAPS);
        fHCalib_Veto_Eff_NoHit        = new TH1F("CaLib_Veto_Eff_NoHit", "CaLib_Veto_Eff_NoHit", fNelemTAPS, 0, fNelemTAPS);
    }

    // prepare for tagger time calibration
    if (fCalib_Tagger_Time)
    {
        fHCalib_Tagger_Time      = new TH2F("CaLib_Tagger_Time", "CaLib_Tagger_Time;Tagger-TAPS time [ns];Tagger element",
                                            8000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_Tagger_Time_Neut = new TH2F("CaLib_Tagger_Time_Neut", "CaLib_Tagger_Time_Neut;Tagger-TAPS time [ns];Tagger element", 
                                            8000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_Tagger_Time_Pi0 = new TH2F("CaLib_Tagger_Time_Pi0", "CaLib_Tagger_Time_Pi0;Tagger-TAPS time [ns];Tagger element", 
                                           8000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
//        fHCalib_Tagger_Time_Ind  = new TH2F("CaLib_Tagger_Time_Ind", "CaLib_Tagger_Time_Ind;Tagger time [ns];Tagger element",
//                                            2200, -500, 500, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_Tagger_Time_Ind  = new TH2F("CaLib_Tagger_Time_Ind", "CaLib_Tagger_Time_Ind;Tagger time [ns];Tagger element",
                                            20000, -500, 500, fNelemTAGG, 0, fNelemTAGG);
    }

    // prepare for proton light attenuation correction
    if (fCalib_Proton_Lt_Att)
    {
        fHCalib_Proton_Lt_Att_IM           = new TH1F("CaLib_Proton_Lt_Att_IM", "CaLib_Proton_Lt_Att_IM;2#gamma invariant mass [MeV];Counts",
                                                 1000, 0, 1000);
        fHCalib_Proton_Lt_Att_MM           = new TH2F("CaLib_Proton_Lt_Att_MM", "CaLib_Proton_Lt_Att_MM;Missing mass [MeV];Tagger element",
                                                 2000, -1000, 1000, fNelemTAGG, 0, fNelemTAGG);
        fHCalib_Proton_Lt_Att_Rand_Time_CB = new TH1F("CaLib_Proton_Lt_Att_Rand_Time_CB", "CaLib_Proton_Lt_Att_Rand_Time_CB;CB-tagger time [ns];Counts", 
                                                  8000, -1000, 1000);
        
        fHCalib_Proton_Lt_Att_CB = new TH2F("Calib_Proton_Lt_Att_CB", "Calib_Proton_Lt_Att_CB;T^{m}_{p} [MeV];f^{corr}_{atten}", 
                                            250, 0, 500, 600, -1, 5);
        fHCalib_Proton_Lt_Att_TAPS = new TH2*[fNelemTAPS];

        for (Int_t i = 0; i < fNelemTAPS; i++)
        {
            sprintf(name, "Calib_Proton_Lt_Att_TAPS_%03d", i);
            sprintf(title, "Calib_Proton_Lt_Att_TAPS_%03d;T^{m}_{p} [MeV];f^{corr}_{atten}", i);
            fHCalib_Proton_Lt_Att_TAPS[i] = new TH2F(name, title, 250, 0, 500, 600, -1, 5);
        }
    }
    
    // prepare for photon resolutions
    if (fCalib_Photon_Res)
    {
        fHCalib_Photon_Res_Theta_CB    = new TH2F("CaLib_Photon_Res_Theta_CB",    "CaLib_Photon_Res_Theta_CB",    180, 0, 180, 400, -100, 100);
        fHCalib_Photon_Res_Theta_TAPS  = new TH2F("CaLib_Photon_Res_Theta_TAPS",  "CaLib_Photon_Res_Theta_TAPS",  360, 0, 180, 400, -100, 100);
        fHCalib_Photon_Res_Phi_CB      = new TH2F("CaLib_Photon_Res_Phi_CB",      "CaLib_Photon_Res_Phi_CB",      180, 0, 180, 400, -100, 100);
        fHCalib_Photon_Res_Phi_TAPS    = new TH2F("CaLib_Photon_Res_Phi_TAPS",    "CaLib_Photon_Res_Phi_TAPS",    360, 0, 180, 400, -100, 100);
        fHCalib_Photon_Res_Energy_CB   = new TH2F("CaLib_Photon_Res_Energy_CB",   "CaLib_Photon_Res_Energy_CB",   500, 0, 1500, 1500, 0, 1500);
        fHCalib_Photon_Res_Energy_TAPS = new TH2F("CaLib_Photon_Res_Energy_TAPS", "CaLib_Photon_Res_Energy_TAPS", 500, 0, 1500, 1500, 0, 1500);
    }

    // prepare for particle resolutions
    if (fCalib_Particle_Res)
    {
        fHCalib_Particle_Res_ClusterMult = new TH1F("CaLib_Particle_Res_ClusterMult", "CaLib_Particle_Res_ClusterMult", 20, 0, 20);
    }

    // prepare for bad scaler read
    if (fCalib_BadScR)
    {
        Int_t maxReads = 5000;
        fHCalib_BadScR_NaIHits     = new TH2F("CaLib_BadScR_NaIHits",     "CaLib_BadScR_NaIHits;Scaler reads;NaI hits",     
                                              maxReads, 0, maxReads, fNaI->GetNelement(),     0, fNaI->GetNelement());
        fHCalib_BadScR_BaF2PWOHits = new TH2F("CaLib_BadScR_BaF2PWOHits", "CaLib_BadScR_BaF2PWOHits;Scaler reads;BaF2PWO hits", 
                                              maxReads, 0, maxReads, fBaF2PWO->GetNelement(), 0, fBaF2PWO->GetNelement());
        fHCalib_BadScR_BaF2Hits    = new TH2F("CaLib_BadScR_BaF2Hits", "CaLib_BadScR_BaF2Hits;Scaler reads;BaF2 hits", 
                                              maxReads, 0, maxReads, fBaF2PWO->GetNelement(), 0, fBaF2PWO->GetNelement());
        fHCalib_BadScR_PWOHits     = new TH2F("CaLib_BadScR_PWOHits", "CaLib_BadScR_PWOHits;Scaler reads;PWO hits", 
                                              maxReads, 0, maxReads, fBaF2PWO->GetNelement(), 0, fBaF2PWO->GetNelement());
        fHCalib_BadScR_PIDHits     = new TH2F("CaLib_BadScR_PIDHits",     "CaLib_BadScR_PIDHits;Scaler reads;PID hits",
                                              maxReads, 0, maxReads, fPID->GetNelement(),     0, fPID->GetNelement());
        fHCalib_BadScR_VetoHits    = new TH2F("CaLib_BadScR_VetoHits",    "CaLib_BadScR_VetoHits;Scaler reads;Veto hits",
                                              maxReads, 0, maxReads, fVeto->GetNelement(),    0, fVeto->GetNelement());
        fHCalib_BadScR_LadderHits  = new TH2F("CaLib_BadScR_LadderHits",  "CaLib_BadScR_LadderHits;Scaler reads;Ladder hits",
                                              maxReads, 0, maxReads, fLadder->GetNelement(),  0, fLadder->GetNelement());    
        fHCalib_BadScR_Scalers     = new TH2F("CaLib_BadScR_Scalers",     "CaLib_BadScR_Scalers;Scaler reads;Scalers", 
                                              maxReads, 0, maxReads, gAR->GetMaxScaler(), 0, gAR->GetMaxScaler());
    }

    // prepare for PWO check
    if (fCalib_PWO_Check)
    {
        fHCalib_PWO_Check_ClusterPos       = new TH2F("CaLib_PWO_ClusterPos",       "CaLib_PWO_Check_ClusterPos",       600, -60, 60, 600, -60, 60);
        fHCalib_PWO_Check_ClusterPos_Neut  = new TH2F("CaLib_PWO_ClusterPos_Neut",  "CaLib_PWO_Check_ClusterPos_Neut",  600, -60, 60, 600, -60, 60);
        fHCalib_PWO_Check_ClusterPos_Charg = new TH2F("CaLib_PWO_ClusterPos_Charg", "CaLib_PWO_Check_ClusterPos_Charg", 600, -60, 60, 600, -60, 60);
        fHCalib_PWO_Check_Rand_Time_CB = new TH1F("Calib_PWO_Check_Rand_Time_CB", "Calib_PWO_Check_Rand_Time_CB", 8000, -1000, 1000);
        fHCalib_PWO_Check_Rand_Time_BaF2 = new TH1F("Calib_PWO_Check_Rand_Time_BaF2", "Calib_PWO_Check_Rand_Time_BaF2", 8000, -1000, 1000);
        fHCalib_PWO_Check_Rand_Time_PWO = new TH1F("Calib_PWO_Check_Rand_Time_PWO", "Calib_PWO_Check_Rand_Time_PWO", 8000, -1000, 1000);
        fHCalib_PWO_Check_TOF_BaF2_Photon  = new TH2F("Calib_PWO_Check_TOF_BaF2_Photon",  "Calib_PWO_Check_TOF_BaF2_Photon",  1000, -50, 50, 1500, 0, 1500);
        fHCalib_PWO_Check_TOF_BaF2_Proton = new TH2F("Calib_PWO_Check_TOF_BaF2_Proton", "Calib_PWO_Check_TOF_BaF2_Proton", 1000, -50, 50, 1500, 0, 1500);
        fHCalib_PWO_Check_DeltaE_BaF2_Proton = new TH2F("Calib_PWO_Check_DeltaE_BaF2_Proton", "Calib_PWO_Check_DeltaE_BaF2_Proton", 1000, 0, 1000, 400, 0, 10);
        fHCalib_PWO_Check_TOF_PWO_Photon  = new TH2F("Calib_PWO_Check_TOF_PWO_Photon",  "Calib_PWO_Check_TOF_PWO_Photon",  1000, -50, 50, 1500, 0, 1500);
        fHCalib_PWO_Check_TOF_PWO_Proton = new TH2F("Calib_PWO_Check_TOF_PWO_Proton", "Calib_PWO_Check_TOF_PWO_Proton", 1000, -50, 50, 1500, 0, 1500);
        fHCalib_PWO_Check_DeltaE_PWO_Proton = new TH2F("Calib_PWO_Check_DeltaE_PWO_Proton", "Calib_PWO_Check_DeltaE_PWO_Proton", 1000, 0, 1000, 400, 0, 10);
        fHCalib_PWO_Check_Edet_BaF2_Proton = new TH2F("Calib_PWO_Check_Edet_BaF2_Proton", "Calib_PWO_Check_Edet_BaF2_Proton", 500, 0, 1000, 500, 0, 1000);
        fHCalib_PWO_Check_Edet_PWO_Proton = new TH2F("Calib_PWO_Check_Edet_PWO_Proton", "Calib_PWO_Check_Edet_PWO_Proton", 500, 0, 1000, 500, 0, 1000);
        fHCalib_PWO_Check_ClusterSize_Photon = new TH2F("Calib_PWO_Check_ClusterSize_Photon", "Calib_PWO_Check_ClusterSize_Photon", fNelemTAPS, 0, fNelemTAPS, 50, 0, 50);
        fHCalib_PWO_Check_ClusterSize_Proton = new TH2F("Calib_PWO_Check_ClusterSize_Proton", "Calib_PWO_Check_ClusterSize_Proton", fNelemTAPS, 0, fNelemTAPS, 50, 0, 50);
        fHCalib_PWO_Check_ClusterInfo_Photon = new TH2F("Calib_PWO_Check_ClusterInfo_Photon", "Calib_PWO_Check_ClusterInfo_Photon", fNelemTAPS, 0, fNelemTAPS, fNelemTAPS, 0, fNelemTAPS);
        fHCalib_PWO_Check_ClusterInfo_Proton = new TH2F("Calib_PWO_Check_ClusterInfo_Proton", "Calib_PWO_Check_ClusterInfo_Proton", fNelemTAPS, 0, fNelemTAPS, fNelemTAPS, 0, fNelemTAPS);
    }

    // prepare for PWO pedestal
    if (fCalib_PWO_Ped)
    {
        // check for raw hits
        if (!fBaF2PWO->IsRawHits()) printf("Error: PWO pedestal calibration needs the activation of RawHits in BaF2PWO!\n");
        if (!fVeto->IsRawHits()) printf("Error: PWO pedestal calibration needs the activation of RawHits in Veto!\n");

        // init histograms
        if (fTAPSType == kTAPS_2008) fCalib_PWO_MaxRing = 1;
        else if (fTAPSType == kTAPS_2009) fCalib_PWO_MaxRing = 2;
        else printf("PWO Pedestal analysis activated but TAPS has no PWO elements!\n");
        fHCalib_PWO_Ped = new TH1*[fNelemTAPS];
        fHCalib_PWO_Veto_Ped = new TH1*[fNelemVeto];
        for (Int_t i = 0; i < fNelemVeto; i++) fHCalib_PWO_Veto_Ped[i] = 0;
        for (Int_t i = 0; i < fNelemTAPS; i++)
        {
            if (TOA2Detector::GetTAPSRing(i, fTAPSType) <= fCalib_PWO_MaxRing) 
            {
                sprintf(name, "ADC%d", fBaF2PWO->GetElement(i)->GetIadc());
                fHCalib_PWO_Ped[i] = new TH1F(name, name, 4096, 0, 4096);
                Int_t veto = TOA2Detector::GetVetoInFrontOfElement(i, fTAPSType);
                if (!fHCalib_PWO_Veto_Ped[veto]) 
                {
                    sprintf(name, "ADC%d", fVeto->GetElement(veto)->GetIadc());
                    fHCalib_PWO_Veto_Ped[veto] = new TH1F(name, name, 4096, 0, 4096);
                }
            }
            else fHCalib_PWO_Ped[i] = 0;
        }
    }
}

//______________________________________________________________________________
void TA2MyCaLib::ReconstructPhysics()
{
    // Main analysis method that is called for every event.

    // ---------------------------------- target position ---------------------------------- 
    
    if (fCalib_Target_Position)
    {
        // look for two neutral hits in CB
        if (fNNeutral == 2 && fCBNCluster == 2 && fNCharged == 0)
        {
            // get theta of particles
            Double_t theta1 = fPartCB[0]->GetTheta()*TMath::RadToDeg();

            // apply cut theta cut
            if (theta1 > 70 && theta1 < 110)
            {
                // loop over target positions
                Int_t nBins = fHCalib_CB_IM_TargPos->GetNbinsY();
                for (Int_t i = 1; i <= nBins; i++)
                {
                    // get target position
                    Double_t targetPos = fHCalib_CB_IM_TargPos->GetYaxis()->GetBinCenter(i);

                    // clone detected particles
                    TOA2DetParticle part1(*fPartCB[0]);
                    TOA2DetParticle part2(*fPartCB[1]);
                    
                    // shift coordinates
                    part1.SetZ(part1.GetZ() - targetPos);
                    part2.SetZ(part2.GetZ() - targetPos);

                    // calculate the 4-vectors assuming photons
                    TLorentzVector p4Gamma_1;
                    TLorentzVector p4Gamma_2;
                    part1.Calculate4Vector(&p4Gamma_1, 0);
                    part2.Calculate4Vector(&p4Gamma_2, 0);
                
                    // calculate the invariant mass
                    Double_t im = (p4Gamma_1 + p4Gamma_2).M();
                    
                    // fill invariant mass vs target position
                    fHCalib_CB_IM_TargPos->Fill(im, targetPos);
                }
            }
        }
    }

    
    // ------------------------------------- CB energy ------------------------------------- 
    
    if (fCalib_CB_Energy)
    {
        // loop over CB hits
        for (UInt_t i = 0; i < fCBNCluster; i++)
        {
            // calculate the 4-vector assuming a photon
            TLorentzVector p4Gamma_1;
            fPartCB[i]->Calculate4Vector(&p4Gamma_1, 0);

            // loop over CB hits
            for (UInt_t j = i+1; j < fCBNCluster; j++)
            {
                // calculate 4-vector assuming a photon
                TLorentzVector p4Gamma_2;
                fPartCB[j]->Calculate4Vector(&p4Gamma_2, 0);

                // calculate invariant mass of hit combination
                Double_t im = (p4Gamma_1 + p4Gamma_2).M();

                // fill invariant mass
                if (fPartCB[i]->GetTheta()*TMath::RadToDeg() > 27 &&
                    fPartCB[j]->GetTheta()*TMath::RadToDeg() > 27)
                {
                    fHCalib_CB_IM->Fill(im, fPartCB[i]->GetCentralElement());
                    fHCalib_CB_IM->Fill(im, fPartCB[j]->GetCentralElement());

                    // fill invariant mass for neutral hits
                    if (fPartCB[i]->GetPIDEnergy() == 0 && fPartCB[j]->GetPIDEnergy() == 0)
                    {
                        fHCalib_CB_IM_Neut->Fill(im, fPartCB[i]->GetCentralElement());
                        fHCalib_CB_IM_Neut->Fill(im, fPartCB[j]->GetCentralElement());
                    }
                }
            }
        }
    }


    // -------------------------- CB quadratic energy correction --------------------------- 
    
    if (fCalib_CB_Quad)
    {
        // look for two neutral hits in CB + 1 charged hit (only 2 neutral show cosmic (?))
        // bump before eta peak
        if (fNNeutral == 2 && fNCharged == 1)
        {
            // check for 2 neutral hits in CB
            Int_t nGamma = 0;
            TLorentzVector p4Gamma[2];
            Int_t center[2];
            for (UInt_t i = 0; i < fCBNCluster; i++)
            {
                // skip charged hits
                if (fPartCB[i]->GetPIDEnergy() > 0) continue;
                
                // save central element
                center[nGamma] = fPartCB[i]->GetCentralElement();

                // calculate the 4-vector assuming a photon
                fPartCB[i]->Calculate4Vector(&p4Gamma[nGamma++], 0);
            }
        
            // only when 2 photons found
            if (nGamma == 2)
            {
                // calculate the invariant mass
                Double_t im = (p4Gamma[0] + p4Gamma[1]).M();
                
                // calculate the mean photon energy
                Double_t meanE = 0.5 * (p4Gamma[0].E() + p4Gamma[1].E());

                // fill the invariant mass
                fHCalib_CB_Quad_IM->Fill(im, center[0]);
                fHCalib_CB_Quad_IM->Fill(im, center[1]);

                // fill the pi0 mean energy
                if (im > fCalib_CB_Quad_Pi0_Min && im < fCalib_CB_Quad_Pi0_Max)
                {
                    fHCalib_CB_Quad_Pi0_Mean_E->Fill(meanE, center[0]);
                    fHCalib_CB_Quad_Pi0_Mean_E->Fill(meanE, center[1]);
                }

                // fill the eta mean energy
                if (im > fCalib_CB_Quad_Eta_Min && im < fCalib_CB_Quad_Eta_Max)
                {
                    fHCalib_CB_Quad_Eta_Mean_E->Fill(meanE, center[0]);
                    fHCalib_CB_Quad_Eta_Mean_E->Fill(meanE, center[1]);
                }
            }
        }
    }


    // -------------------------------------- CB time ------------------------------------- 
    
    if (fCalib_CB_Time)
    {
        // loop over CB hits
        for (UInt_t i = 0; i < fCBNCluster; i++)
        {
            // get the time
            Double_t time_1 = fPartCB[i]->GetTime();

            // loop over CB hits
            for (UInt_t j = i+1; j < fCBNCluster; j++)
            {
                // get the time 
                Double_t time_2 = fPartCB[j]->GetTime();

                // fill time difference
                fHCalib_CB_Time->Fill(time_1 - time_2, fPartCB[i]->GetCentralElement());
                fHCalib_CB_Time->Fill(time_2 - time_1, fPartCB[j]->GetCentralElement());

                // fill time difference for neutral hits
                if (fPartCB[i]->GetPIDEnergy() == 0 && fPartCB[j]->GetPIDEnergy() == 0)
                {
                    fHCalib_CB_Time_Neut->Fill(time_1 - time_2, fPartCB[i]->GetCentralElement());
                    fHCalib_CB_Time_Neut->Fill(time_2 - time_1, fPartCB[j]->GetCentralElement());
                }
            }
            
            if (fPartCB[i]->GetPIDEnergy() == 0) fHCalib_CB_Time_Ind->Fill(time_1, fPartCB[i]->GetCentralElement());
        }
    }
    
    
    // ------------------------------------ CB rise time ---------------------------------- 
 
    if (fCalib_CB_RiseTime)
    {
        // look for two neutral hits in CB
        if (fNNeutral == 2 && fCBNCluster == 2 && fNCharged <= 1)
        {
            // exclude events with 1 neutral in TAPS
            if (fTAPSNCluster) 
            {
                if (fPartTAPS[0]->GetVetoEnergy() == 0) 
                    goto label_end_cb_risetime;
            }

            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_cb_risetime;
            TOA2DetParticle** decay_photons = pi0.GetDetectedProducts();
        
            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            //fHCalib_CB_Walk_IM->Fill(im);

            // invariant mass cut
            if (im < fCalib_CB_RiseTime_Pi0_Min || im > fCalib_CB_RiseTime_Pi0_Max) goto label_end_cb_risetime;

            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // fill time differences 
                fHCalib_CB_RiseTime->Fill(decay_photons[0]->GetTime() - fTaggerPhotonTime[i], decay_photons[0]->GetCentralElement());
                fHCalib_CB_RiseTime->Fill(decay_photons[1]->GetTime() - fTaggerPhotonTime[i], decay_photons[1]->GetCentralElement());

            }
        } 
    } 
    label_end_cb_risetime:
 

    // ----------------------------------- CB time walk ----------------------------------- 
    
    if (fCalib_CB_Walk)
    {
        // Clear the time walk parameters before analyzing
        if (fEventCounter == 0) ClearCBTimeWalk();

        // look for two neutral hits in CB
        if (fNNeutral == 2 && fCBNCluster == 2 && fNCharged == 0)
        {
            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_cb_timewalk;
            TOA2DetParticle** decay_photons = pi0.GetDetectedProducts();
        
            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            fHCalib_CB_Walk_IM->Fill(im);

            // invariant mass cut
            if (im < fCalib_CB_Walk_Pi0_Min || im > fCalib_CB_Walk_Pi0_Max) goto label_end_cb_timewalk;

            // photon 1 cluster properties
            Int_t g1_nhits      = decay_photons[0]->GetClusterSize();
            UInt_t* g1_hits     = decay_photons[0]->GetClusterHits();
            Double_t* g1_energy = decay_photons[0]->GetClusterHitEnergies();
            Double_t* g1_time   = decay_photons[0]->GetClusterHitTimes();

            // photon 2 cluster properties
            Int_t g2_nhits      = decay_photons[1]->GetClusterSize();
            UInt_t* g2_hits     = decay_photons[1]->GetClusterHits();
            Double_t* g2_energy = decay_photons[1]->GetClusterHitEnergies();
            Double_t* g2_time   = decay_photons[1]->GetClusterHitTimes();
            
            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(pi0, fTaggerPhotonTime[i], 
                                                               fCalib_CB_Walk_Prompt_Min, fCalib_CB_Walk_Prompt_Max, 
                                                               fCalib_CB_Walk_BG1_Min, fCalib_CB_Walk_BG1_Max,
                                                               fCalib_CB_Walk_BG2_Min, fCalib_CB_Walk_BG2_Max,
                                                               0, 0, 0, 0, 0, 0);
                
                // fill tagger coincidence time
                FillTaggerCoincidence(pi0, fTaggerPhotonTime[i], fHCalib_CB_Walk_Rand_Time_CB, 0);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;

                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - *p4Pi0).M() - TOGlobals::kProtonMass;
                fHCalib_CB_Walk_MM->Fill(mm, tagg_element, subtr_weight);
 
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_CB_Walk_MM_Min && mm < fCalib_CB_Walk_MM_Max) mmOk = kTRUE;

                if (mmOk)
                {
                    // fill energies and times of all elements of the photon cluster 1
                    for (Int_t j = 0; j < g1_nhits; j++) 
                        fHCalib_CB_Walk_E_T[g1_hits[j]]->Fill(g1_energy[j], g1_time[j] - fTaggerPhotonTime[i], subtr_weight);
                
                    // fill energies and times of all elements of the photon cluster 2
                    for (Int_t j = 0; j < g2_nhits; j++) 
                        fHCalib_CB_Walk_E_T[g2_hits[j]]->Fill(g2_energy[j], g2_time[j] - fTaggerPhotonTime[i], subtr_weight);
                }

            } // tagger loop
        
        } // if: two neutral hits in CB

    } // end CB time walk
    label_end_cb_timewalk:

    // ----------------------------------- CB time walk for EPT -----------------------------------

    if (fCalib_CB_Walk_EPT)
    {
        // Clear the time walk parameters before analyzing
        if (fEventCounter == 0) ClearCBTimeWalk();

        // look for two neutral hits and one charged in CB
        if (!(fNNeutral == 2 && fCBNCluster == 3 && fNCharged == 1))
            goto label_end_cb_timewalk_ept;

        // reconstruct pi0
        TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
        if (!pi0.Reconstruct(fNNeutral, fPartNeutral))
            goto label_end_cb_timewalk_ept;

        // get the invariant mass
        TLorentzVector* p4Pi0 = pi0.Get4Vector();
        Double_t im = p4Pi0->M();
        fHCalib_CB_Walk_EPT_IM->Fill(im);

        // invariant mass cut
        if (im < fCalib_CB_Walk_EPT_Pi0_Min || im > fCalib_CB_Walk_EPT_Pi0_Max)
            goto label_end_cb_timewalk_ept;

        // assume the charged particle to be a proton
        TOA2DetParticle* proton = fPartCharged[0];
        TLorentzVector* p4proton = new TLorentzVector();
        proton->Calculate4Vector(p4proton, TOGlobals::kProtonMass);

        // phi difference cut
        Double_t phi = TMath::Abs(p4proton->Phi() - p4Pi0->Phi())*TMath::RadToDeg();
        fHCalib_CB_Walk_EPT_Phi->Fill(phi);
        if(phi<fCalib_CB_Walk_EPT_Phi_Min || phi>fCalib_CB_Walk_EPT_Phi_Max)
            goto label_end_cb_timewalk_ept;
        
        // use only high energetic protons to have minimal timewalk
        // timewalk calibration could also be iterated to minimize bias
        Double_t proton_energy = proton->GetEnergy();
        if(proton_energy<fCalib_CB_Walk_EPT_ProtonE_Min || proton_energy>fCalib_CB_Walk_EPT_ProtonE_Max)
            goto label_end_cb_timewalk_ept;
        Double_t proton_time = proton->GetTime();
        
        // fill the timings of the two photons
        TOA2DetParticle** decay_photons = pi0.GetDetectedProducts();        
        
        // photon 1 cluster properties
        Int_t g1_nhits      = decay_photons[0]->GetClusterSize();
        UInt_t* g1_hits     = decay_photons[0]->GetClusterHits();
        Double_t* g1_energy = decay_photons[0]->GetClusterHitEnergies();
        Double_t* g1_time   = decay_photons[0]->GetClusterHitTimes();
        Int_t* g1_mults     = decay_photons[0]->GetClusterHitTimeMultiplicities(); 
        

        // photon 2 cluster properties
        Int_t g2_nhits      = decay_photons[1]->GetClusterSize();
        UInt_t* g2_hits     = decay_photons[1]->GetClusterHits();
        Double_t* g2_energy = decay_photons[1]->GetClusterHitEnergies();
        Double_t* g2_time   = decay_photons[1]->GetClusterHitTimes();    
        Int_t* g2_mults     = decay_photons[1]->GetClusterHitTimeMultiplicities(); 
        
        // use only first crystal (with highest energy) of both photons
//        if(g1_nhits>1) g1_nhits=1;
//        if(g2_nhits>1) g2_nhits=1;
        
        // fill energies and times of all elements of the photon cluster 1
        for (Int_t j = 0; j < g1_nhits; j++)  {
          if(g1_mults[j] != 1)
            continue;
          fHCalib_CB_Walk_EPT_E_T[g1_hits[j]]->Fill(g1_energy[j], g1_time[j] - proton_time);
        }
    
        // fill energies and times of all elements of the photon cluster 2
        for (Int_t j = 0; j < g2_nhits; j++) {
          if(g2_mults[j] != 1)
            continue;  
          fHCalib_CB_Walk_EPT_E_T[g2_hits[j]]->Fill(g2_energy[j], g2_time[j] - proton_time);
        }

    } // end CB time walk
    label_end_cb_timewalk_ept:


    // --------------------------- CB proton energy correction ----------------------------

    if (fCalib_CB_Proton_ECorr)
    {
        if (fCBNCluster == 1)
        {   
            // charged clusters
            if (fPartCB[0]->GetPIDEnergy() > 0)
            {
                // calculate the correction factor
                Double_t e_vertex = fPartMC[0]->Get4Vector()->E() - fPartMC[0]->Get4Vector()->M();
                Double_t e_crystal = fPartCB[0]->GetEnergy();
                Double_t f_corr = (e_vertex - e_crystal) / e_crystal;
                Double_t f_corr_inv = (e_vertex - e_crystal) / e_vertex;
                
                // fill histograms
                fHCalib_CB_Proton_ECorr[fPartCB[0]->GetCentralElement()]->Fill(e_crystal, f_corr);
                fHCalib_CB_Proton_ECorr_Inv[fPartCB[0]->GetCentralElement()]->Fill(e_vertex, f_corr_inv);
                fHCalib_CB_Proton_ECorr_Check->Fill(e_vertex - e_crystal, fPartCB[0]->GetCentralElement());
            }
        }
    }
    
    
    // ----------------------------- CB-TAPS LED calibration ------------------------------ 
    
    if (fCalib_CBTAPS_LED)
    {
        // global M2+ or M3+ trigger
        if (fTrigger->IsTriggerL1(kCB) && 
            !fTrigger->IsTriggerL1(kTAPS_M2) && 
            (fTrigger->IsTriggerL2(kM2) || fTrigger->IsTriggerL2(kM3)))
        {
            // create and init arrays
            Int_t elem[45+6];
            Double_t energy[45+6];
            for (Int_t i = 0; i < 45+6; i++)
            {
                elem[i] = -1;
                energy[i] = 0;
            }

            // loop over NaI hits
            for (UInt_t i = 0; i < fNaINhits; i++)
            {
                // calculate segment
                Int_t seg = (Int_t)(fNaIHits[i] / 16);
                
                // get energy
                Double_t e = fNaIEnergy[fNaIHits[i]];

                // store maximum energy
                if (e > energy[seg])
                {
                    elem[seg] = fNaIHits[i];
                    energy[seg] = e;
                }
            }

            // loop over BaF2 hits
            for (UInt_t i = 0; i < fBaF2PWONhits; i++)
            {
                // calculate sector
                Int_t sec = TOA2Detector::GetTAPSBlock(fBaF2PWOHits[i], fTAPSType) - 1;
                
                // get energy
                Double_t e = fBaF2PWOEnergy[fBaF2PWOHits[i]];

                // store maximum energy
                if (e > energy[45+sec])
                {
                    elem[45+sec] = fBaF2PWOHits[i];
                    energy[45+sec] = e;
                }
            }

            // sort energies
            Int_t sort_id[45+6];
            TMath::Sort(45+6, energy, sort_id);
            
            // select M2+ trigger
            if (fTrigger->IsTriggerL2(kM2))
            {
                // fill 2 element with the highest energies
                for (Int_t i = 0; i < 2; i++)
                {
                    if (sort_id[i] < 45)
                        fHCalib_CB_LED_M2->Fill(energy[sort_id[i]], elem[sort_id[i]]);
                    else
                        fHCalib_TAPS_LED_M2->Fill(energy[sort_id[i]], elem[sort_id[i]]);
                }
            }
             
            // select M3+ trigger
            if (fTrigger->IsTriggerL2(kM3))
            {
                // fill 3 elements with the highest energies
                for (Int_t i = 0; i < 3; i++)
                {
                    if (sort_id[i] < 45)
                        fHCalib_CB_LED_M3->Fill(energy[sort_id[i]], elem[sort_id[i]]);
                    else
                        fHCalib_TAPS_LED_M3->Fill(energy[sort_id[i]], elem[sort_id[i]]);
                }
            }
            
        } // if: global M2+ or M3+ trigger

        // TAPS only M2+ trigger
        if (fTrigger->IsTriggerL1(kTAPS_M2) && !fTrigger->IsTriggerL1(kCB))
        {
            // create and init arrays
            Int_t elem[6];
            Double_t energy[6];
            for (Int_t i = 0; i < 6; i++)
            {
                elem[i] = -1;
                energy[i] = 0;
            }

            // loop over BaF2 hits
            for (UInt_t i = 0; i < fBaF2PWONhits; i++)
            {
                // calculate sector
                Int_t sec = TOA2Detector::GetTAPSBlock(fBaF2PWOHits[i], fTAPSType) - 1;
                
                // get energy
                Double_t e = fBaF2PWOEnergy[fBaF2PWOHits[i]];

                // store maximum energy
                if (e > energy[sec])
                {
                    elem[sec] = fBaF2PWOHits[i];
                    energy[sec] = e;
                }
            }

            // sort energies
            Int_t sort_id[6];
            TMath::Sort(6, energy, sort_id);
            
            // fill 2 elements with the highest energies
            fHCalib_TAPS_LED2_M2->Fill(energy[sort_id[0]], elem[sort_id[0]]);
            fHCalib_TAPS_LED2_M2->Fill(energy[sort_id[1]], elem[sort_id[1]]);
 
        } // if: TAPS only M2+ trigger
    }
    

    // ----------------------------------- TAPS energy ------------------------------------ 
    
    if (fCalib_TAPS_Energy)
    {
        // loop over TAPS clusters
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            for (UInt_t t = 0; t < fTaggerPhotonNhits; t++)
            {
                fHCalib_TOF_TAPS->Fill(fPartTAPS[i]->GetTime() - fTaggerPhotonTime[t], fPartTAPS[i]->GetCentralElement(), fTaggerPhotonHits[t]);
                if (fPartTAPS[i]->GetVetoEnergy() == 0)
                    fHCalib_TOF_TAPS_Neut->Fill(fPartTAPS[i]->GetTime() + fTaggerPhotonTime[t], fPartTAPS[i]->GetCentralElement(), fTaggerPhotonHits[t]);
            }

            if( !TAPSTimeCutOK(fPartTAPS[i]) && isInnerTAPSArea(fPartTAPS[i]))
                continue;

            // calculate 4-vector assuming a photon
            TLorentzVector p4Gamma_1;
            fPartTAPS[i]->Calculate4Vector(&p4Gamma_1, 0);
            
            // loop over CB clusters
            for (UInt_t j = 0; j < fCBNCluster; j++)
            {
                // calculate the 4-vector assuming a photon
                TLorentzVector p4Gamma_2;
                fPartCB[j]->Calculate4Vector(&p4Gamma_2, 0);

                // calculate invariant mass of hit combination
                Double_t im = (p4Gamma_1 + p4Gamma_2).M();

                // fill invariant mass
                if (fPartTAPS[i]->GetVetoEnergy() == 0 && fPartCB[j]->GetPIDEnergy() == 0)
                {
                    fHCalib_TAPS_IM_Neut->Fill(im, fPartTAPS[i]->GetCentralElement());

                    // if inner crystal OR 2 neutral hits
                    if( isInnerTAPSArea(fPartTAPS[i]) || fNNeutral == 2 )
                        // fill invariant mass for all combinations of 1 cluster in CB and 1 cluster in TAPS
                        fHCalib_TAPS_IM_Neut_1CB_1TAPS->Fill(im, fPartTAPS[i]->GetCentralElement());
                }
            }
            
            // loop over TAPS clusters
            for (UInt_t j = i+1; j < fTAPSNCluster; j++)
            {

                if( !TAPSTimeCutOK(fPartTAPS[j]) && isInnerTAPSArea(fPartTAPS[i]))
                    continue;

                // calculate 4-vector assuming a photon
                TLorentzVector p4Gamma_2;
                fPartTAPS[j]->Calculate4Vector(&p4Gamma_2, 0);

                // calculate invariant mass of hit combination
                Double_t im = (p4Gamma_1 + p4Gamma_2).M();

                // fill invariant mass
                fHCalib_TAPS_IM_TAPS->Fill(im, fPartTAPS[i]->GetCentralElement());
                fHCalib_TAPS_IM_TAPS->Fill(im, fPartTAPS[j]->GetCentralElement());

                // fill invariant mass for neutral hits
                if (fPartTAPS[i]->GetVetoEnergy() == 0 && fPartTAPS[j]->GetVetoEnergy() == 0)
                {
                    fHCalib_TAPS_IM_Neut_TAPS->Fill(im, fPartTAPS[i]->GetCentralElement());
                    fHCalib_TAPS_IM_Neut_TAPS->Fill(im, fPartTAPS[j]->GetCentralElement());
                }
            }
        }
    }

    UInt_t nneutrTAPS =  0;
    for (UInt_t iTAPS = 0; iTAPS < fTAPSNCluster; iTAPS++)
    {
        if( TAPSTimeCutOK(fPartTAPS[iTAPS]) )
            nneutrTAPS++;
    }


    UInt_t nNeutr = nneutrTAPS + fCBNCluster;
    if(nneutrTAPS > 0)
    {
        for(UInt_t iTAPS = 0; iTAPS < fTAPSNCluster; iTAPS++ )
        {
            if( !TAPSTimeCutOK(fPartTAPS[iTAPS]) )
                continue;
            if(fPartTAPS[iTAPS]->GetVetoEnergy() < 1.0)
            {
                TLorentzVector p4Gamma_1;
                fPartTAPS[iTAPS]->Calculate4Vector(&p4Gamma_1, 0);
                for(UInt_t jCB = 0; jCB < fCBNCluster; jCB++)
                {
                    TLorentzVector p4Gamma_2;
                    fPartCB[jCB]->Calculate4Vector(&p4Gamma_2, 0);
                    // calculate invariant mass of hit combination
                    Double_t im = (p4Gamma_1 + p4Gamma_2).M();

                    fHCalib_TAPS_IM_Neut_TAPS_mult->Fill( im , fPartTAPS[iTAPS]->GetCentralElement(), nNeutr );
                }
            }
        }
    }

    
    
    // ----------------------------- TAPS energy (BG subtr.) ------------------------------ 
    
    if (fCalib_TAPS_Energy_BG_Subtr)
    {
        // look for two neutral clusters
        if (fNNeutral == 2)
        {
            // select 1 photon in CB and 1 photon in TAPS
            TOA2DetParticle* partCB;
            TOA2DetParticle* partTAPS;
            if (fPartNeutral[0]->GetDetector() == kCBDetector &&
                fPartNeutral[1]->GetDetector() == kTAPSDetector)
            {
                partCB = fPartNeutral[0];
                partTAPS = fPartNeutral[1];
            }
            else if (fPartNeutral[0]->GetDetector() == kTAPSDetector &&
                     fPartNeutral[1]->GetDetector() == kCBDetector)
            {
                partCB = fPartNeutral[1];
                partTAPS = fPartNeutral[0];
            }
            else goto label_end_taps_energy_bg_subtr;

            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_taps_energy_bg_subtr;
            
            // reconstruct random background subtraction particle in CB
            TOA2RecParticle recPart(1);
            recPart.Reconstruct(1, &partCB);

            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(recPart, fTaggerPhotonTime[i], 
                                                               fCalib_TAPS_Energy_Prompt_Min, fCalib_TAPS_Energy_Prompt_Max, 
                                                               fCalib_TAPS_Energy_BG1_Min, fCalib_TAPS_Energy_BG1_Max,
                                                               fCalib_TAPS_Energy_BG2_Min, fCalib_TAPS_Energy_BG2_Max,
                                                               fCalib_TAPS_Energy_Prompt_Min, fCalib_TAPS_Energy_Prompt_Max, 
                                                               fCalib_TAPS_Energy_BG1_Min, fCalib_TAPS_Energy_BG1_Max,
                                                               fCalib_TAPS_Energy_BG2_Min, fCalib_TAPS_Energy_BG2_Max);
                
                // fill tagger coincidence time
                FillTaggerCoincidence(recPart, fTaggerPhotonTime[i], fHCalib_TAPS_Energy_Rand_Time_CB, 0);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - p4Pi0Constr).M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_TAPS_Energy_MM_Min && mm < fCalib_TAPS_Energy_MM_Max) mmOk = kTRUE;
                
                // fill invariant mass
                if (mmOk) fHCalib_TAPS_IM_Neut_BG_Subtr->Fill(im, partTAPS->GetCentralElement(), subtr_weight);
                
                // fill missing mass
                fHCalib_TAPS_Energy_MM->Fill(mm, tagg_element, subtr_weight);
                
            } // tagger loop
        
        } // if: two neutral + 1 charged hit
 
    }
    label_end_taps_energy_bg_subtr:


    // ------------------------------------- TAPS time ------------------------------------ 
    
    if (fCalib_TAPS_Time)
    {
        // loop over TAPS hits
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            // get the time
            Double_t time_1 = fPartTAPS[i]->GetTime();
            
            // calculate 4-vector assuming a photon
            TLorentzVector p4Gamma_1;
            fPartTAPS[i]->Calculate4Vector(&p4Gamma_1, 0);
 
            // loop over TAPS hits
            for (UInt_t j = i+1; j < fTAPSNCluster; j++)
            {
                // get the time 
                Double_t time_2 = fPartTAPS[j]->GetTime();
                
                // calculate 4-vector assuming a photon
                TLorentzVector p4Gamma_2;
                fPartTAPS[j]->Calculate4Vector(&p4Gamma_2, 0);
                
                // calculate invariant mass of hit combination
                Double_t im = (p4Gamma_1 + p4Gamma_2).M();
 
                // fill time difference
                fHCalib_TAPS_Time->Fill(time_1 - time_2, fPartTAPS[i]->GetCentralElement());
                fHCalib_TAPS_Time->Fill(time_2 - time_1, fPartTAPS[j]->GetCentralElement());

                // fill time difference for neutral hits
                if (fPartTAPS[i]->GetVetoEnergy() == 0 && fPartTAPS[j]->GetVetoEnergy() == 0)
                {
                    fHCalib_TAPS_Time_Neut->Fill(time_1 - time_2, fPartTAPS[i]->GetCentralElement());
                    fHCalib_TAPS_Time_Neut->Fill(time_2 - time_1, fPartTAPS[j]->GetCentralElement());
                
                    // invariant mass cut
                    if (im > 50)
                    {
                        fHCalib_TAPS_Time_Neut_IM_Cut->Fill(time_1 - time_2, fPartTAPS[i]->GetCentralElement());
                        fHCalib_TAPS_Time_Neut_IM_Cut->Fill(time_2 - time_1, fPartTAPS[j]->GetCentralElement());
                    }
                }
            }
            
            if (fPartTAPS[i]->GetVetoEnergy() == 0) fHCalib_TAPS_Time_Ind->Fill(time_1, fPartTAPS[i]->GetCentralElement());
        }
    }
    
    
    // ------------------------- TAPS quadratic energy correction ------------------------- 
    
    if (fCalib_TAPS_Quad)
    {
        // request 2 neutral clusters
        if (fNNeutral == 2)
        {
            Double_t neutInCB = kFALSE;
            Double_t neutInTAPS = kFALSE;
            TLorentzVector p4Gamma_taps;
            TLorentzVector p4Gamma_cb;

            // search neutral hit in CB and calculate the photon 4-vector
            for (UInt_t i = 0; i < fCBNCluster; i++)
            {
                // check if cluster is neutral
                if (fPartCB[i]->GetPIDEnergy() == 0) 
                {
                    fPartCB[i]->Calculate4Vector(&p4Gamma_cb, 0);
                    neutInCB = kTRUE;
                }
            }

            // search neutral hit in TAPS and calculate the photon 4-vector
            for (UInt_t i = 0; i < fTAPSNCluster; i++)
            {
                if (fPartTAPS[i]->GetVetoEnergy() == 0) 
                {
                    fPartTAPS[i]->Calculate4Vector(&p4Gamma_taps, 0);
                    neutInTAPS = kTRUE;
                }
            }

            // check if 1 cluster in CB and 1 cluster in TAPS were found
            if (neutInCB && neutInTAPS)
            {
                // calculate the invariant mass
                Double_t im = (p4Gamma_cb + p4Gamma_taps).M();
                
                // calculate the mean photon energy
                Double_t meanE = 0.5 * (p4Gamma_cb.E() + p4Gamma_taps.E());

                // get the theta of the cluster
                Int_t elem = fPartTAPS[0]->GetCentralElement();

                // fill the invariant mass
                fHCalib_TAPS_Quad_IM->Fill(im, elem);

                // fill the pi0 mean energy
                if (im > fCalib_TAPS_Quad_Pi0_Min && im < fCalib_TAPS_Quad_Pi0_Max)
                {
                    fHCalib_TAPS_Quad_Pi0_Mean_E->Fill(meanE, elem);
                }
                
                // fill the eta mean energy
                if (im > fCalib_TAPS_Quad_Eta_Min && im < fCalib_TAPS_Quad_Eta_Max)
                {
                    fHCalib_TAPS_Quad_Eta_Mean_E->Fill(meanE, elem);
                }
            }
        }
    }
    
    
    // ------------------------------- TAPS PSA calibration ------------------------------- 

    if (fCalib_TAPS_PSA)
    {
        // loop over TAPS clusters
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            // get particle
            TOA2DetParticle* p = fPartTAPS[i];
            Int_t elem = p->GetCentralElement();

            // calculate the PSA information
            Double_t lgE = p->GetCentralEnergy();
            Double_t sgE = p->GetCentralSGEnergy();
            Double_t r = TMath::Sqrt(sgE*sgE + lgE*lgE); 
            Double_t a = TMath::ATan(sgE/lgE)*TMath::RadToDeg();
            
            // fill histogram
            fHCalib_TAPS_PSAR_PSAA[elem]->Fill(a, r);
            
            // smeared SG
            sgE = p->GetCentralSGEnergy() + gRandom->Gaus(0, 0.5);
            r = TMath::Sqrt(sgE*sgE + lgE*lgE); 
            a = TMath::ATan(sgE/lgE)*TMath::RadToDeg();
            fHCalib_TAPS_PSAR_PSAA_SM[elem]->Fill(a, r);
         }
    }
    
    
    // ------------------------------- TAPS LED calibration ------------------------------- 

    if (fCalib_TAPS_LED)
    {
        // check if there was a charged hit in TAPS
        Bool_t charged_hit = kFALSE;
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            if (fPartTAPS[i]->GetVetoEnergy() > 0) 
            {
                charged_hit = kTRUE;
                break;
            }
        }

        // accept only neutral hits in TAPS
        if (!charged_hit)
        {
            // loop over all BaF2 hits
            for (UInt_t i = 0; i < fBaF2PWONhits; i++)
            {
                // fill energy
                fHCalib_TAPS_LED_Norm->Fill(fBaF2PWOEnergy[fBaF2PWOHits[i]], fBaF2PWOHits[i]);
            
                // fill energy under LED1 condition
                for (UInt_t j = 0; j < fBaF2LED1Nhits; j++)
                {
                    if (fBaF2LED1Hits[j] == TOA2Detector::GetVetoInFrontOfElement(fBaF2PWOHits[i], fTAPSType)) 
                        fHCalib_TAPS_LED_LED1->Fill(fBaF2PWOEnergy[fBaF2PWOHits[i]], fBaF2PWOHits[i]);
                }
                
                // fill energy under LED2 condition
                for (UInt_t j = 0; j < fBaF2LED2Nhits; j++)
                {
                    if (fBaF2LED2Hits[j] == TOA2Detector::GetVetoInFrontOfElement(fBaF2PWOHits[i], fTAPSType)) 
                        fHCalib_TAPS_LED_LED2->Fill(fBaF2PWOEnergy[fBaF2PWOHits[i]], fBaF2PWOHits[i]);
                }
            }
        }
    }
    
    
    // -------------------------- TAPS proton energy correction --------------------------- 
    
    if (fCalib_TAPS_Proton_ECorr)
    {
        if (fTAPSNCluster == 1)
        {   
            // charged clusters
            if (fPartTAPS[0]->GetVetoEnergy() > 0)
            {
                // calculate the correction factor
                Double_t e_vertex = fPartMC[0]->Get4Vector()->E() - fPartMC[0]->Get4Vector()->M();
                Double_t e_crystal = fPartTAPS[0]->GetEnergy();
                Double_t f_corr = (e_vertex - e_crystal) / e_crystal;
                Double_t f_corr_inv = (e_vertex - e_crystal) / e_vertex;
                
                // fill histograms
                fHCalib_TAPS_Proton_ECorr[fPartTAPS[0]->GetCentralElement()]->Fill(e_crystal, f_corr);
                fHCalib_TAPS_Proton_ECorr_Inv[fPartTAPS[0]->GetCentralElement()]->Fill(e_vertex, f_corr_inv);
                fHCalib_TAPS_Proton_ECorr_Check->Fill(e_vertex - e_crystal, fPartTAPS[0]->GetCentralElement());
            }
        }
    }
    

    // -------------------------------------- PID Phi ------------------------------------- 
    
    if (fCalib_PID_Phi)
    {
        // select 1 cluster in CB and 1 hit in the PID
        if (fCBNCluster == 1 && fPIDNhits == 1)
        {
            fHCalib_PID_CBPhi_ID->Fill(fPartCB[0]->GetPhi()*TMath::RadToDeg(), fPIDHits[0]);
            
            // 1 crystal only
            if (fPartCB[0]->GetClusterSize() == 1)
                fHCalib_PID_CBPhi_ID_1Cryst->Fill(fPartCB[0]->GetPhi()*TMath::RadToDeg(), fPIDHits[0]);
        }
    }
 

    // ------------------------------------ PID energy ------------------------------------ 

    // new method (Tom)
    if (fCalib_PID_Energy)
    {
        
        // fill raw PID energy
        for (UInt_t i = 0; i < fPIDNhits; i++) 
            fHCalib_PID_Energy_Raw_PID_Energy->Fill(fPIDEnergy[fPIDHits[i]], fPIDHits[i]);
        
        // Set pedestal and gain to get raw ADC values
        if (!fIsMC && fEventCounter == 0)
        {
            // loop over PID elements
            for (Int_t i = 0; i < fNelemPID; i++)
            {
                // set pedestal to 0
                fPID->GetElement(i)->SetA0(0);

                // set gain to 1
                fPID->GetElement(i)->SetA1(1);

                // set large energy cuts
                fPID->GetElement(i)->SetEnergyLowThr(0);
                fPID->GetElement(i)->SetEnergyHighThr(5000);
            }
        }

        // look for two neutral + 1 charged hit
        if (fNNeutral == 2 && fNCharged == 1)
        {
            // check if charged hit is in CB
            if (fPartCharged[0]->GetPIDEnergy() == 0) goto label_end_pid_energy;
            
            // bad PID region
            if (fPartCharged[0]->GetDetector() == kCBDetector && 
                fPartCharged[0]->GetTheta()*TMath::RadToDeg() < 27) goto label_end_pid_energy;
        
            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_pid_energy;
            
            // check software trigger
            if (!IsTrigger(pi0)) goto label_end_pid_energy;
  
            // recoil proton
            TOA2DetParticle* recoil_proton = fPartCharged[0];
            TLorentzVector p4RecoilProton;
            recoil_proton->Calculate4Vector(&p4RecoilProton, TOGlobals::kProtonMass);
 
            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // invariant mass cut
            Bool_t imOk = kTRUE;
            if (im < fCalib_PID_Energy_Pi0_Min || im > fCalib_PID_Energy_Pi0_Max) imOk = kFALSE;

            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(pi0, fTaggerPhotonTime[i], 
                                                               fCalib_PID_Energy_Prompt_Min, fCalib_PID_Energy_Prompt_Max, 
                                                               fCalib_PID_Energy_BG1_Min, fCalib_PID_Energy_BG1_Max,
                                                               fCalib_PID_Energy_BG2_Min, fCalib_PID_Energy_BG2_Max,
                                                               fCalib_PID_Energy_Prompt_Min, fCalib_PID_Energy_Prompt_Max, 
                                                               fCalib_PID_Energy_BG1_Min, fCalib_PID_Energy_BG1_Max,
                                                               fCalib_PID_Energy_BG2_Min, fCalib_PID_Energy_BG2_Max);
                
                // fill tagger coincidence time
                if (imOk) FillTaggerCoincidence(pi0, fTaggerPhotonTime[i], fHCalib_PID_Energy_Rand_Time_CB, fHCalib_PID_Energy_Rand_Time_TAPS);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
 
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - p4Pi0Constr).M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_PID_Energy_MM_Min && mm < fCalib_PID_Energy_MM_Max) mmOk = kTRUE;
                
                // check coplanarity
                Double_t phi_diff = TMath::Abs(p4Pi0Constr.Phi() - p4RecoilProton.Phi())*TMath::RadToDeg();
                Bool_t copOk = kTRUE;
                if (phi_diff < 170 || phi_diff > 190) copOk = kFALSE;
                
                // fill invariant mass
                if (mmOk && copOk) fHCalib_PID_Energy_IM->Fill(im, subtr_weight);
                
                // fill missing mass
                if (imOk && copOk) fHCalib_PID_Energy_MM->Fill(mm, tagg_element, subtr_weight);
                
                // check charged hit - PID coincidences
                if (imOk && mmOk && copOk)
                {
                    // check pi0
                    fHCalib_PID_Energy_Pi0_Energy->Fill(p4Pi0->E(), subtr_weight);
                    fHCalib_PID_Energy_Pi0_Theta->Fill(p4Pi0->Theta()*TMath::RadToDeg(), subtr_weight);
                    
                    // check proton
                    fHCalib_PID_Energy_Proton_Energy->Fill(recoil_proton->GetEnergy(), subtr_weight);

                    // get CB theta
                    Double_t th = recoil_proton->GetTheta()*TMath::RadToDeg();
                    
                    // get CB energy
                    Double_t e = recoil_proton->GetEnergy();
                    
                    // get PID energy
                    Double_t de = recoil_proton->GetPIDEnergy();
 
                    // get PID element
                    Int_t elem = recoil_proton->GetPIDIndex();
                    
                    // fill dE vs E
                    fHCalib_PID_dE_E[elem]->Fill(e, de, th, subtr_weight);

                    // fill dE
                    fHCalib_PID_Energy_Proton_PID_Energy->Fill(de, elem, subtr_weight);
                
                } // missing mass ok

            } // tagger loop
        
        } // if: two neutral + 1 charged hit
    }
    label_end_pid_energy:
    
    
    // ---------------------------- PID energy (traditional) ------------------------------ 
    
    // traditional method (Evie)
    if (fCalib_PID_Energy_Trad)
    {
        // loop over CB hits
        for (UInt_t i = 0; i < fCBNCluster; i++)
        {
            // get PID energy
            Double_t de = fPartCB[i]->GetPIDEnergy();
            
            // skip neutral hits
            if (de == 0) continue;
            
            // get CB theta
            Double_t th = fPartCB[i]->GetTheta()*TMath::RadToDeg();
            
            // get CB energy
            Double_t e = fPartCB[i]->GetEnergy();

            // get PID element
            Int_t elem = fPartCB[i]->GetPIDIndex();
            
            // fill dE vs E
            fHCalib_PID_dE_E_Trad[elem]->Fill(e, de, th);
        }
    }
 

    // -------------------------------------- PID time ------------------------------------- 
    
    if (fCalib_PID_Time)
    {
        // loop over PID hits
        for (UInt_t i = 0; i < fPIDNhits; i++)
        {
            // get the time
            Double_t time_1 = fPIDTime[fPIDHits[i]];

            // loop over PID hits
            for (UInt_t j = i+1; j < fPIDNhits; j++)
            {
                // get the time 
                Double_t time_2 = fPIDTime[fPIDHits[j]];

                // fill time difference
                fHCalib_PID_Time->Fill(time_1 - time_2, fPIDHits[i]);
                fHCalib_PID_Time->Fill(time_2 - time_1, fPIDHits[j]);
            }
            
            fHCalib_PID_Time_Ind->Fill(time_1, fPIDHits[i]);
        }
    }
    
    
    // ----------------------------------- PID efficiency ---------------------------------- 

    if (fCalib_PID_Eff)
    {

        // look for two neutral + 1 charged hit
        if (fNNeutral >= 2 && fCBNCluster >= 3)
        {
            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_pid_eff;
            
            // only photons in the ball
            if (pi0.GetDetectedProduct(0)->GetDetector() == kTAPSDetector) goto label_end_pid_eff;
            if (pi0.GetDetectedProduct(1)->GetDetector() == kTAPSDetector) goto label_end_pid_eff;
            
            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // invariant mass cut
            Bool_t imOk = kTRUE;
            if (im < fCalib_PID_Eff_Pi0_Min || im > fCalib_PID_Eff_Pi0_Max) imOk = kFALSE;

            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(pi0, fTaggerPhotonTime[i], 
                                                               fCalib_PID_Eff_Prompt_Min, fCalib_PID_Eff_Prompt_Max, 
                                                               fCalib_PID_Eff_BG1_Min, fCalib_PID_Eff_BG1_Max,
                                                               fCalib_PID_Eff_BG2_Min, fCalib_PID_Eff_BG2_Max,
                                                               fCalib_PID_Eff_Prompt_Min, fCalib_PID_Eff_Prompt_Max, 
                                                               fCalib_PID_Eff_BG1_Min, fCalib_PID_Eff_BG1_Max,
                                                               fCalib_PID_Eff_BG2_Min, fCalib_PID_Eff_BG2_Max);
                
                // fill tagger coincidence time
                if (imOk) FillTaggerCoincidence(pi0, fTaggerPhotonTime[i], fHCalib_PID_Eff_Rand_Time_CB, 0);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - p4Pi0Constr).M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_PID_Eff_MM_Min && mm < fCalib_PID_Eff_MM_Max) mmOk = kTRUE;
                
                // fill invariant mass
                if (mmOk) fHCalib_PID_Eff_IM->Fill(im, subtr_weight);
                
                // fill missing mass
                if (imOk) fHCalib_PID_Eff_MM->Fill(mm, tagg_element, subtr_weight);
                
          
                // check PID coincidences
                if (imOk && mmOk)
                {
                    // calculate proton 
                    TLorentzVector p4Proton = p4Beam + p4Target - p4Pi0Constr;

                    // loop over all CB clusters and calculate theta difference
                    Double_t thetaDiff[fCBNCluster];
                    for (UInt_t j = 0; j < fCBNCluster; j++)
                    {
                        // calculate 4-vector
                        TLorentzVector p4Cand;
                        fPartCB[j]->Calculate4Vector(&p4Cand, TOGlobals::kProtonMass);

                        // calculate theta difference
                        thetaDiff[j] = TMath::Abs(p4Proton.Theta() - p4Cand.Theta())*TMath::RadToDeg();
                    }
                    
                    // get the smallest theta difference
                    Int_t min_elem = TMath::LocMin(fCBNCluster, thetaDiff);
                    
                    // fill smallest theta difference
                    fHCalib_PID_Eff_ThetaDiff->Fill(thetaDiff[min_elem], subtr_weight);
                    
                    // cut on theta difference and good PID angle
                    if (thetaDiff[min_elem] < 20 && 
                        fPartCB[min_elem]->GetTheta()*TMath::RadToDeg() > 30)
                    {
                        // check if PID fired
                        if (fPartCB[min_elem]->GetPIDEnergy() > 0) 
                            fHCalib_PID_Eff_Hit->Fill(fPartCB[min_elem]->GetPhi()*TMath::RadToDeg(), subtr_weight);
                        else 
                            fHCalib_PID_Eff_NoHit->Fill(fPartCB[min_elem]->GetPhi()*TMath::RadToDeg(), subtr_weight);
                    }

                } // missing mass ok

            } // tagger loop
        
        } // if: at least two neutral hits
    }
    label_end_pid_eff:


    // ----------------------------- veto-crystal correlation ----------------------------- 
    
    if (fCalib_Veto_Corr)
    {
        // loop over BaF2/PWO hits
        for (UInt_t i = 0; i < fBaF2PWONhits; i++)
        {
            // loop over veto hits
            for (UInt_t j = 0; j < fVetoNhits; j++) 
            {   
                fHCalib_Veto_Corr->Fill(fVetoHits[j], fBaF2PWOHits[i]);
            }
        }
    }
 

    // ----------------------------------- Veto energy ------------------------------------ 
    
    if (fCalib_Veto_Energy)
    {
        // loop over TAPS hits
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            // get Veto energy
            Double_t de = fPartTAPS[i]->GetVetoEnergy();
            
            // skip neutral hits
            if (de == 0) continue;
            
            // get TAPS energy
            Double_t e = fPartTAPS[i]->GetEnergy();

            // get Veto element
            Int_t elem = fPartTAPS[i]->GetVetoIndex();

            // fill dE vs E
            fHCalib_Veto_dE_E[elem]->Fill(e, de);
        }
    }


    // -------------------------------------- Veto time ------------------------------------- 
    
    if (fCalib_Veto_Time)
    {
        // loop over Veto hits
        for (UInt_t i = 0; i < fVetoNhits; i++)
        {
            // get the time
            Double_t time_1 = fVetoTime[fVetoHits[i]];

            // loop over Veto hits
            for (UInt_t j = i+1; j < fVetoNhits; j++)
            {
                // get the time 
                Double_t time_2 = fVetoTime[fVetoHits[j]];

                // fill time difference
                fHCalib_Veto_Time->Fill(time_1 - time_2, fVetoHits[i]);
                fHCalib_Veto_Time->Fill(time_2 - time_1, fVetoHits[j]);
            }
            
            fHCalib_Veto_Time_Ind->Fill(time_1, fVetoHits[i]);
        }
    }
    
    
    // ---------------------------------- Veto efficiency ---------------------------------- 

    if (fCalib_Veto_Eff)
    {
        // look for two neutral + 1 charged hit
        if (fNNeutral >= 2 && fTAPSNCluster >= 1)
        {
            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_veto_eff;
            
            // only photons in the ball
            if (pi0.GetDetectedProduct(0)->GetDetector() == kTAPSDetector) goto label_end_veto_eff;
            if (pi0.GetDetectedProduct(1)->GetDetector() == kTAPSDetector) goto label_end_veto_eff;

            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // invariant mass cut
            Bool_t imOk = kTRUE;
            if (im < fCalib_Veto_Eff_Pi0_Min || im > fCalib_Veto_Eff_Pi0_Max) imOk = kFALSE;

            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(pi0, fTaggerPhotonTime[i], 
                                                               fCalib_Veto_Eff_Prompt_Min, fCalib_Veto_Eff_Prompt_Max, 
                                                               fCalib_Veto_Eff_BG1_Min, fCalib_Veto_Eff_BG1_Max,
                                                               fCalib_Veto_Eff_BG2_Min, fCalib_Veto_Eff_BG2_Max,
                                                               fCalib_Veto_Eff_Prompt_Min, fCalib_Veto_Eff_Prompt_Max, 
                                                               fCalib_Veto_Eff_BG1_Min, fCalib_Veto_Eff_BG1_Max,
                                                               fCalib_Veto_Eff_BG2_Min, fCalib_Veto_Eff_BG2_Max);
                
                // fill tagger coincidence time
                if (imOk) FillTaggerCoincidence(pi0, fTaggerPhotonTime[i], fHCalib_Veto_Eff_Rand_Time_CB, 0);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - p4Pi0Constr).M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_Veto_Eff_MM_Min && mm < fCalib_Veto_Eff_MM_Max) mmOk = kTRUE;
                
                // fill invariant mass
                if (mmOk) fHCalib_Veto_Eff_IM->Fill(im, subtr_weight);
                
                // fill missing mass
                if (imOk) fHCalib_Veto_Eff_MM->Fill(mm, tagg_element, subtr_weight);
                
                // check Veto coincidences
                if (imOk && mmOk)
                {
                    // calculate proton 
                    TLorentzVector p4Proton = p4Beam + p4Target - p4Pi0Constr;

                    // loop over all TAPS clusters and calculate theta difference
                    Double_t thetaDiff[fTAPSNCluster];
                    for (UInt_t j = 0; j < fTAPSNCluster; j++)
                    {
                        // calculate 4-vector (energy is not important so take a photon)
                        TLorentzVector p4Cand;
                        fPartTAPS[j]->Calculate4Vector(&p4Cand, 0);

                        // calculate theta difference
                        thetaDiff[j] = TMath::Abs(p4Proton.Theta() - p4Cand.Theta())*TMath::RadToDeg();
                    }
                    
                    // get the smallest theta difference
                    Int_t min_elem = TMath::LocMin(fTAPSNCluster, thetaDiff);
                    
                    // fill smallest theta difference
                    fHCalib_Veto_Eff_ThetaDiff->Fill(thetaDiff[min_elem], subtr_weight);
                    
                    // cut on theta difference
                    if (thetaDiff[min_elem] < 5)
                    {
                        // check if Veto fired
                        if (fPartTAPS[min_elem]->GetVetoEnergy() > 0) 
                            fHCalib_Veto_Eff_Hit->Fill(fPartTAPS[min_elem]->GetCentralElement(), subtr_weight);
                        else 
                            fHCalib_Veto_Eff_NoHit->Fill(fPartTAPS[min_elem]->GetCentralElement(), subtr_weight);
                    }

                } // missing mass ok

            } // tagger loop
        
        } // if: at least two neutral hits
    }
    label_end_veto_eff:

 

    // ------------------------------------ tagger time ----------------------------------- 
    
    if (fCalib_Tagger_Time)
    {
        // look for pi0 in combination with a proton
        // at least one photon has to be in TAPS
        Bool_t isPi0 = kFALSE;
        Double_t taps_time_pi0 = 0;
        Int_t nGamma = 0;
        Int_t nGammaTAPS = 0;
        TLorentzVector p4Gamma[2];
        if (fNNeutral == 2 && fNCharged == 1)
        {
            // loop over CB clusters
            for (UInt_t i = 0; i < fCBNCluster; i++)
            {
                if (fPartCB[i]->GetPIDEnergy() == 0) 
                {
                    // calculate 4-vector
                    fPartCB[i]->Calculate4Vector(&p4Gamma[nGamma++], 0);
                }
            }
            
            // loop over TAPS clusters
            for (UInt_t i = 0; i < fTAPSNCluster; i++)
            {
                if (fPartTAPS[i]->GetVetoEnergy() == 0 &&
                    TOA2Detector::GetTAPSRing(fPartTAPS[i]->GetCentralElement(), fTAPSType) > 2) 
                {
                    // calculate 4-vector
                    fPartTAPS[i]->Calculate4Vector(&p4Gamma[nGamma++], 0);
                    
                    // save time
                    taps_time_pi0 += fPartTAPS[i]->GetTime();
                    
                    // count photons in TAPS
                    nGammaTAPS++;
                }
            }
         
            // check if at least one photon was in TAPS
            if (nGammaTAPS)
            {
                // calculate average time
                taps_time_pi0 /= (Double_t)nGammaTAPS;

                // check for pi0
                Double_t im = (p4Gamma[0] + p4Gamma[1]).M();
                if (im > 120 && im < 150) isPi0 = kTRUE;
            }
        }

        // tagger loop
        for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
        {
            // get tagger element and time
            Int_t tagg_element = fTaggerPhotonHits[i];
            Double_t tagg_time = fTaggerPhotonTime[i];
            
            // check if there is no TAPS -> tagging efficiency run
            if (!fBaF2PWO)
            {
                // second loop over tagger hits
                for (UInt_t j = i+1; j < fTaggerPhotonNhits; j++)
                {
                    // get tagger element and time
                    Int_t tagg_element_2 = fTaggerPhotonHits[j];
                    Double_t tagg_time_2 = fTaggerPhotonTime[j];
                    
                    // fill time difference
                    fHCalib_Tagger_Time->Fill(tagg_time - tagg_time_2, tagg_element);
                    fHCalib_Tagger_Time->Fill(tagg_time_2 - tagg_time, tagg_element_2);
                }
            }

            // loop over TAPS hits
            for (UInt_t j = 0; j < fTAPSNCluster; j++)
            {
                // get the time 
                Double_t taps_time = fPartTAPS[j]->GetTime();
                
                // skip PWOs
                if (TOA2Detector::IsTAPSPWO(fPartTAPS[j]->GetCentralElement(), fTAPSType)) continue; 

                // fill time difference
                fHCalib_Tagger_Time->Fill(tagg_time + taps_time, tagg_element);
                
                // neutral hits
                if (fPartTAPS[j]->GetVetoEnergy() == 0)
                {
                    fHCalib_Tagger_Time_Neut->Fill(tagg_time + taps_time, tagg_element);
                }
            }

            // hits from pi0
            if (isPi0) 
            {
                fHCalib_Tagger_Time_Pi0->Fill(tagg_time + taps_time_pi0, tagg_element);
            }
            
            fHCalib_Tagger_Time_Ind->Fill(tagg_time, tagg_element);
        }
    }

    
    // ----------------------------- Proton light attenuation ------------------------------ 

    if (fCalib_Proton_Lt_Att)
    {
        // look for two neutral + 1 charged hit
        if (fNNeutral >= 2 && fNCharged == 1)
        {
            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_proton_lt_att;
            
            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // invariant mass cut
            Bool_t imOk = kTRUE;
            if (im < fCalib_Proton_Lt_Att_Pi0_Min || im > fCalib_Proton_Lt_Att_Pi0_Max) imOk = kFALSE;

            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
                
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = GetBGSubtractionWeight(pi0, fTaggerPhotonTime[i], 
                                                               fCalib_Proton_Lt_Att_Prompt_Min, fCalib_Proton_Lt_Att_Prompt_Max, 
                                                               fCalib_Proton_Lt_Att_BG1_Min, fCalib_Proton_Lt_Att_BG1_Max,
                                                               fCalib_Proton_Lt_Att_BG2_Min, fCalib_Proton_Lt_Att_BG2_Max,
                                                               fCalib_Proton_Lt_Att_Prompt_Min, fCalib_Proton_Lt_Att_Prompt_Max, 
                                                               fCalib_Proton_Lt_Att_BG1_Min, fCalib_Proton_Lt_Att_BG1_Max,
                                                               fCalib_Proton_Lt_Att_BG2_Min, fCalib_Proton_Lt_Att_BG2_Max);
                
                // fill tagger coincidence time
                if (imOk) FillTaggerCoincidence(pi0, fTaggerPhotonTime[i], fHCalib_Proton_Lt_Att_Rand_Time_CB, 0);

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing mass
                Double_t mm = (p4Beam + p4Target - p4Pi0Constr).M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > fCalib_Proton_Lt_Att_MM_Min && mm < fCalib_Proton_Lt_Att_MM_Max) mmOk = kTRUE;
                
                // fill invariant mass
                if (mmOk) fHCalib_Proton_Lt_Att_IM->Fill(im, subtr_weight);
                
                // fill missing mass
                if (imOk) fHCalib_Proton_Lt_Att_MM->Fill(mm, tagg_element, subtr_weight);
                
                // check Veto coincidences
                if (imOk && mmOk)
                {
                    // calculate proton 
                    TLorentzVector p4Proton = p4Beam + p4Target - p4Pi0Constr;
                    
                    // get measured energy and calculate correction factor
                    Double_t f_att = 1;
                    Double_t e_vertex = p4Proton.E() - p4Proton.M();
                    Double_t e_meas = fPartCharged[0]->GetEnergy();
                    Int_t elem = fPartCharged[0]->GetCentralElement();

                    // proton in CB or TAPS
                    if (fPartCharged[0]->GetDetector() == kCBDetector)
                    { 
                        Double_t e_crystal = e_vertex - fCalib_Proton_Lt_Att_CB_FGenCorr[elem]->Eval(e_vertex)*e_vertex;
                        f_att = (e_crystal - e_meas) / e_meas;
                        fHCalib_Proton_Lt_Att_CB->Fill(e_meas, f_att);
                    }
                    else if (fPartCharged[0]->GetDetector() == kTAPSDetector)
                    {
                        fHCalib_Proton_Lt_Att_TAPS[elem]->Fill(e_meas, f_att);
                    }
                    
                } // missing mass ok

            } // tagger loop
        
        } // if: at least two neutral hits
    }
    label_end_proton_lt_att:
    
    
    // -------------------------------- Photon resolutions --------------------------------- 

    if (fCalib_Photon_Res)
    {
        // request 1 neutral cluster
        if (fNNeutral == 1 && fNCharged == 0)
        {
            // generated photon
            Double_t th_gen = fPartMC[0]->Get4Vector()->Theta()*TMath::RadToDeg();
            Double_t phi_gen = fPartMC[0]->Get4Vector()->Phi()*TMath::RadToDeg();
            Double_t e_gen = fPartMC[0]->Get4Vector()->E();
            
            // detected photon
            Double_t th_det = fPartNeutral[0]->GetTheta()*TMath::RadToDeg();
            Double_t phi_det = fPartNeutral[0]->GetPhi()*TMath::RadToDeg();
            Double_t e_det = fPartNeutral[0]->GetEnergy();

            // fill histos
            if (fPartNeutral[0]->GetDetector() == kCBDetector)
            {
                fHCalib_Photon_Res_Theta_CB->Fill(th_gen, th_gen-th_det);
                fHCalib_Photon_Res_Phi_CB->Fill(th_gen, phi_gen-phi_det);
                fHCalib_Photon_Res_Energy_CB->Fill(e_gen, e_det);
            }
            else
            {
                fHCalib_Photon_Res_Theta_TAPS->Fill(th_gen, th_gen-th_det);
                fHCalib_Photon_Res_Phi_TAPS->Fill(th_gen, phi_gen-phi_det);
                fHCalib_Photon_Res_Energy_TAPS->Fill(e_gen, e_det);
            }
        }
    }


    // ------------------------------- Particle resolutions --------------------------------- 

    if (fCalib_Particle_Res)
    {
        // fill cluster multiplicity
        fHCalib_Particle_Res_ClusterMult->Fill(fNTotal);

        // request 1 cluster
        if (fNTotal == 1)
        {
            // generated particle
            fCalib_Particle_Res_Theta_Gen  = fPartMC[0]->Get4Vector()->Theta()*TMath::RadToDeg();
            fCalib_Particle_Res_Phi_Gen    = fPartMC[0]->Get4Vector()->Phi()*TMath::RadToDeg();
            fCalib_Particle_Res_Energy_Gen = fPartMC[0]->Get4Vector()->E() - fPartMC[0]->Get4Vector()->M();
            
            // detected particle
            TOA2DetParticle* p = fCBNCluster ? fPartCB[0] : fPartTAPS[0];
            fCalib_Particle_Res_Theta_Det  = p->GetTheta()*TMath::RadToDeg();
            fCalib_Particle_Res_Phi_Det    = p->GetPhi()*TMath::RadToDeg();
            fCalib_Particle_Res_Energy_Det = p->GetEnergy();
            fCalib_Particle_Res_Module_Det = p->GetCentralElement();

            if (fCBNCluster) fCalib_Particle_Res_Tree_CB->Fill();
            else fCalib_Particle_Res_Tree_TAPS->Fill();
        }
    }


    // --------------------------------- Bad scaler reads ---------------------------------- 

    if (fCalib_BadScR)
    {
        // fill scalers
        if (gAR->IsScalerRead())
        {
            for (Int_t i = 0; i < gAR->GetMaxScaler(); i++)
                fHCalib_BadScR_Scalers->SetBinContent(fScalerReadCounter, i+1, (Double_t) fScaler[i]);
        }

        // fill NaI hits
        if (fNaI)
        {
            // get number of hits
            UInt_t nhits = fNaI->GetNhits();
            Int_t* hits = fNaI->GetHits();

            // loop over hits
            for (UInt_t i = 0; i < nhits; i++)
                fHCalib_BadScR_NaIHits->Fill(fScalerReadCounter, hits[i]);
        }

        // fill BaF2PWO/BaF2/PWO hits
        if (fBaF2PWO)
        {
            // get number of hits
            UInt_t nhits = fBaF2PWO->GetNhits();
            Int_t* hits = fBaF2PWO->GetHits();

            // loop over hits
            for (UInt_t i = 0; i < nhits; i++)
            {
                fHCalib_BadScR_BaF2PWOHits->Fill(fScalerReadCounter, hits[i]);
                if (TOA2Detector::IsTAPSPWO(hits[i], fTAPSType)) fHCalib_BadScR_PWOHits->Fill(fScalerReadCounter, hits[i]);
                fHCalib_BadScR_BaF2Hits->Fill(fScalerReadCounter, hits[i]);
            }
        }

        // fill PID hits
        if (fPID)
        {
            // get number of hits
            UInt_t nhits = fPID->GetNhits();
            Int_t* hits = fPID->GetHits();

            // loop over CB ADC hits
            for (UInt_t i = 0; i < nhits; i++)
                fHCalib_BadScR_PIDHits->Fill(fScalerReadCounter, hits[i]);
        }

        // fill Veto hits
        if (fVeto)
        {
            // get number of hits
            UInt_t nhits = fVeto->GetNhits();
            Int_t* hits = fVeto->GetHits();

            // loop over CB ADC hits
            for (UInt_t i = 0; i < nhits; i++)
                fHCalib_BadScR_VetoHits->Fill(fScalerReadCounter, hits[i]);
        }

        // fill Ladder hits
        if (fLadder)
        {
            // get number of hits
            UInt_t nhits = fLadder->GetNhits();
            Int_t* hits = fLadder->GetHits();

            // loop over CB ADC hits
            for (UInt_t i = 0; i < nhits; i++)
                fHCalib_BadScR_LadderHits->Fill(fScalerReadCounter, hits[i]);
        }
        
    }
    
    
    // ------------------------------------ PWO check -------------------------------------- 

    if (fCalib_PWO_Check)
    {
        /*
        // fill cluster centers
        for (UInt_t i = 0; i < fTAPSNCluster; i++)
        {
            // fill charged and neutral clusters
            if (fPartTAPS[i]->GetVetoEnergy() > 0) 
                fHCalib_PWO_Check_ClusterPos_Charg->Fill(-fPartTAPS[i]->GetX(), fPartTAPS[i]->GetY());
            else 
                fHCalib_PWO_Check_ClusterPos_Neut->Fill(-fPartTAPS[i]->GetX(), fPartTAPS[i]->GetY());
            
            // fill all clusters
            fHCalib_PWO_Check_ClusterPos->Fill(-fPartTAPS[i]->GetX(), fPartTAPS[i]->GetY());
        }
        */

        // look for two neutral clusters
        if (fNNeutral == 2 && (fNCharged == 0 || fNCharged  == 1))
        {
            /*
            // select 1 photon in CB and 1 photon in TAPS
            TOA2DetParticle* partCB;
            TOA2DetParticle* partTAPS;
            if (fPartNeutral[0]->GetDetector() == kCBDetector &&
                fPartNeutral[1]->GetDetector() == kTAPSDetector)
            {
                partCB = fPartNeutral[0];
                partTAPS = fPartNeutral[1];
            }
            else if (fPartNeutral[0]->GetDetector() == kTAPSDetector &&
                     fPartNeutral[1]->GetDetector() == kCBDetector)
            {
                partCB = fPartNeutral[1];
                partTAPS = fPartNeutral[0];
            }
            else goto label_end_pwo_check;
            */

            // photon in CB
            TOA2DetParticle* partCB = 0;
            if (fPartNeutral[0]->GetDetector() == kCBDetector) partCB = fPartNeutral[0];
            else if (fPartNeutral[1]->GetDetector() == kCBDetector) partCB = fPartNeutral[1];
            
            // photon in TAPS BaF2
            TOA2DetParticle* partTAPS = 0;
            if (fPartNeutral[0]->GetDetector() == kTAPSDetector && 
                !TOA2Detector::IsTAPSPWO(fPartNeutral[0]->GetCentralElement(), fTAPSType)) partTAPS = fPartNeutral[0];
            else if (fPartNeutral[1]->GetDetector() == kTAPSDetector && 
                !TOA2Detector::IsTAPSPWO(fPartNeutral[1]->GetCentralElement(), fTAPSType)) partTAPS = fPartNeutral[1];
            
            // correct PWO-tagger time offset
            if (!fIsMC)
            {
                if (fPartNeutral[0]->GetDetector() == kTAPSDetector && 
                    TOA2Detector::IsTAPSPWO(fPartNeutral[0]->GetCentralElement(), fTAPSType)) 
                    fPartNeutral[0]->SetTime(fPartNeutral[0]->GetTime() + 6.06009e+01);
                if (fPartNeutral[1]->GetDetector() == kTAPSDetector && 
                    TOA2Detector::IsTAPSPWO(fPartNeutral[1]->GetCentralElement(), fTAPSType)) 
                    fPartNeutral[1]->SetTime(fPartNeutral[1]->GetTime() + 6.06009e+01);
                if (fNCharged && fPartCharged[0]->GetDetector() == kTAPSDetector && 
                    TOA2Detector::IsTAPSPWO(fPartCharged[0]->GetCentralElement(), fTAPSType)) 
                    fPartCharged[0]->SetTime(fPartCharged[0]->GetTime() + 6.06009e+01);
            }

            // reconstruct pi0
            TOA2RecMeson2g pi0(fNNeutral, TOGlobals::kPi0Mass);
            if (!pi0.Reconstruct(fNNeutral, fPartNeutral)) goto label_end_pwo_check;
            
            // reconstruct random background subtraction particle in CB
            TOA2RecParticle recPartCB(1);
            if (partCB) recPartCB.Reconstruct(1, &partCB);

            // reconstruct random background subtraction particle in TAPS BaF2
            TOA2RecParticle recPartTAPS(1);
            if (partTAPS) recPartTAPS.Reconstruct(1, &partTAPS);

            // get the invariant mass
            TLorentzVector* p4Pi0 = pi0.Get4Vector();
            Double_t im = p4Pi0->M();
            TLorentzVector p4Pi0Constr;
            pi0.CalculateConstrained4Vector(TOGlobals::kPi0Mass, &p4Pi0Constr);
            
            // invariant mass cut
            Bool_t imOk = kFALSE;
            if (im > 110 && im < 160) imOk = kTRUE;

            // target 4-vector
            TLorentzVector p4Target(0., 0., 0., TOGlobals::kProtonMass);
        
            // tagger loop
            for (UInt_t i = 0; i < fTaggerPhotonNhits; i++)
            {
                Int_t tagg_element = fTaggerPhotonHits[i];
                
                // skip bad tagger channels
                if (IsBadTaggerChannel(tagg_element)) continue;
                
                // beam 4-vector
                TLorentzVector p4Beam(0., 0., fTaggerPhotonEnergy[i], fTaggerPhotonEnergy[i]);
                
                // missing proton
                TLorentzVector p4ProtonCalc = p4Beam + p4Target - p4Pi0Constr;
                
                // missing mass
                Double_t mm = p4ProtonCalc.M() - TOGlobals::kProtonMass;
                
                // missing mass cut
                Bool_t mmOk = kFALSE;
                if (mm > -30 && mm < 30) mmOk = kTRUE;
                 
                // calculate background subtraction factor
                Double_t subtr_weight = 0;
                if (partTAPS) subtr_weight = GetBGSubtractionWeight(recPartTAPS, fTaggerPhotonTime[i], 
                                                                    -4, 4, -400, -50, 50, 400,
                                                                    -4, 4, -400, -50, 50, 400);
                else if (partCB) subtr_weight = GetBGSubtractionWeight(recPartCB, fTaggerPhotonTime[i], 
                                                                       -4, 4, -400, -50, 50, 400,
                                                                       -4, 4, -400, -50, 50, 400);
                 
                // fill tagger coincidence time
                if (imOk && mmOk) 
                {
                    for (Int_t j = 0; j < 2; j++)
                    {
                        if (fPartNeutral[j]->GetDetector() == kCBDetector)
                        {
                            fHCalib_PWO_Check_Rand_Time_CB->Fill(fPartNeutral[j]->GetTime()-fTaggerPhotonTime[i]);
                        }
                        else
                        {
                            if (TOA2Detector::IsTAPSPWO(fPartNeutral[j]->GetCentralElement(), fTAPSType))
                                fHCalib_PWO_Check_Rand_Time_PWO->Fill(fPartNeutral[j]->GetTime()-fTaggerPhotonTime[i]);
                            else
                                fHCalib_PWO_Check_Rand_Time_BaF2->Fill(fPartNeutral[j]->GetTime()+fTaggerPhotonTime[i]);
                        }
                    }
                }

                // skip useless background events (not belonging to the background windows)
                if (subtr_weight == 0) continue;
                
 
                // fill invariant mass
                //if (mmOk) fHCalib_TAPS_IM_Neut_BG_Subtr->Fill(im, partTAPS->GetCentralElement(), subtr_weight);
                
                // fill missing mass
                //fHCalib_TAPS_Energy_MM->Fill(mm, tagg_element, subtr_weight);
                
                // apply cuts
                if (imOk && mmOk)
                {
                    // fill diagnostic histograms for photons
                    for (Int_t j = 0; j < 2; j++)
                    {
                        // check for TAPS particles
                        if (fPartNeutral[j]->GetDetector() == kTAPSDetector) 
                        {
                            // get TAPS ring
                            Int_t ring = TOA2Detector::GetTAPSRing(fPartNeutral[j]->GetCentralElement(), fTAPSType);
                            
                            // cluster position
                            fHCalib_PWO_Check_ClusterPos->Fill(-fPartNeutral[j]->GetX(), fPartNeutral[j]->GetY(), subtr_weight);
                            fHCalib_PWO_Check_ClusterPos_Neut->Fill(-fPartNeutral[j]->GetX(), fPartNeutral[j]->GetY(), subtr_weight);
                            
                            // TOF-E
                            if (ring <= 2)
                                fHCalib_PWO_Check_TOF_PWO_Photon->Fill(fPartNeutral[j]->CalculateTOFTagger(fTaggerPhotonTime[i], fIsMC, fTAPSType), 
                                                                       fPartNeutral[j]->GetEnergy(), subtr_weight);
                            else
                                fHCalib_PWO_Check_TOF_BaF2_Photon->Fill(fPartNeutral[j]->CalculateTOFTagger(fTaggerPhotonTime[i], fIsMC), 
                                                                        fPartNeutral[j]->GetEnergy(), subtr_weight);
                        
                            // cluster info
                            fHCalib_PWO_Check_ClusterSize_Photon->Fill(fPartNeutral[j]->GetCentralElement(), fPartNeutral[j]->GetClusterSize(), subtr_weight);
                            for (Int_t k = 0; k < fPartNeutral[j]->GetClusterSize(); k++)
                                fHCalib_PWO_Check_ClusterInfo_Photon->Fill(fPartNeutral[j]->GetCentralElement(), fPartNeutral[j]->GetClusterHits()[k], subtr_weight);
                        }
                    }

                    // fill diagnostic histograms for protons
                    if (fNCharged && fPartCharged[0]->GetDetector() == kTAPSDetector)
                    {
                        // get TAPS ring
                        Int_t ring = TOA2Detector::GetTAPSRing(fPartCharged[0]->GetCentralElement(), fTAPSType);
 
                        // cluster position
                        fHCalib_PWO_Check_ClusterPos->Fill(-fPartCharged[0]->GetX(), fPartCharged[0]->GetY(), subtr_weight);
                        fHCalib_PWO_Check_ClusterPos_Charg->Fill(-fPartCharged[0]->GetX(), fPartCharged[0]->GetY(), subtr_weight);
                    
                        // TOF-E
                        if (ring <= 2)
                            fHCalib_PWO_Check_TOF_PWO_Proton->Fill(fPartCharged[0]->CalculateTOFTagger(fTaggerPhotonTime[i], fIsMC, fTAPSType), 
                                                                   fPartCharged[0]->GetEnergy(), subtr_weight);
                        else
                            fHCalib_PWO_Check_TOF_BaF2_Proton->Fill(fPartCharged[0]->CalculateTOFTagger(fTaggerPhotonTime[i], fIsMC), 
                                                                    fPartCharged[0]->GetEnergy(), subtr_weight);
                        
                        // dE-E
                        if (ring <= 2)
                            fHCalib_PWO_Check_DeltaE_PWO_Proton->Fill(fPartCharged[0]->GetEnergy(), fPartCharged[0]->GetVetoEnergy(), subtr_weight);
                        else
                            fHCalib_PWO_Check_DeltaE_BaF2_Proton->Fill(fPartCharged[0]->GetEnergy(), fPartCharged[0]->GetVetoEnergy(), subtr_weight);
                        
                        // E_det vs. E_calc
                        if (ring <= 2)
                            fHCalib_PWO_Check_Edet_PWO_Proton->Fill(p4ProtonCalc.E()-TOGlobals::kProtonMass, fPartCharged[0]->GetEnergy(), subtr_weight);
                        else
                            fHCalib_PWO_Check_Edet_BaF2_Proton->Fill(p4ProtonCalc.E()-TOGlobals::kProtonMass, fPartCharged[0]->GetEnergy(), subtr_weight);
                        
                        // cluster info
                        fHCalib_PWO_Check_ClusterSize_Proton->Fill(fPartCharged[0]->GetCentralElement(), fPartCharged[0]->GetClusterSize(), subtr_weight);
                        for (Int_t k = 0; k < fPartCharged[0]->GetClusterSize(); k++)
                            fHCalib_PWO_Check_ClusterInfo_Proton->Fill(fPartCharged[0]->GetCentralElement(), fPartCharged[0]->GetClusterHits()[k], subtr_weight);
      
                    }
                }

            } // tagger loop
        
        } // if: two neutral + 1 charged hit

    }
    label_end_pwo_check:


    // ------------------------------------ PWO pedestal ------------------------------------- 

    if (fCalib_PWO_Ped)
    {
        // 
        // PWO
        //

        // loop over ADC hits
        for (UInt_t i = 0; i < fBaF2PWO->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fBaF2PWO->GetRawEnergyHits()[i];
            
            // check if it's a PWO
            if (TOA2Detector::GetTAPSRing(elem, fTAPSType) > fCalib_PWO_MaxRing) continue;
            
            // get ADC value
            UShort_t value_adc = fBaF2PWO->GetElement(elem)->GetRawADCValue();
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fBaF2PWO->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fBaF2PWO->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fHCalib_PWO_Ped[elem]->Fill(value_adc);
            }
        }

        //
        // PWO-Veto
        //
        
        // select pedestal events by requesting no hits in TAPS
        if (fBaF2PWONhits) goto label_end_pwo_ped;

        // loop over ADC hits
        for (UInt_t i = 0; i < fVeto->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fVeto->GetRawEnergyHits()[i];
            
            // check if it's a PWO
            if (TOA2Detector::GetTAPSRing(elem, kTAPS_2007) > fCalib_PWO_MaxRing) continue;
            
            // get ADC value
            UShort_t value_adc = fVeto->GetElement(elem)->GetRawADCValue();
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fVeto->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fVeto->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fHCalib_PWO_Veto_Ped[elem]->Fill(value_adc);
            }
        }
    }
    label_end_pwo_ped:

    return;
}

//______________________________________________________________________________
Int_t TA2MyCaLib::GetPIDElementForPhi(Double_t phi)
{
    // Return the index of the PID element that covers the phi angle 'phi'.
    
    Int_t id = -1;
    Double_t minDiff = 360;

    // loop over PID elements
    for (Int_t i = 0; i < fNelemPID; i++)
    {   
        // calculate phi difference
        Double_t diff = TMath::Abs(fPID->GetPosition(i)->Z() - phi);

        // get minimal phi difference
        if (diff < minDiff)
        {
            id = i;
            minDiff = diff;
        }
    }
    
    return id;
}

ClassImp(TA2MyCaLib)
