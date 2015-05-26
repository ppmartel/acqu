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


#include "TA2MyPhysics.h"



//______________________________________________________________________________
TA2MyPhysics::TA2MyPhysics(const char* name, TA2Analysis* analysis) 
    : TA2AccessSQL(name, analysis)
{
    // Constructor.
    
    Char_t tmp[256];
   
    // command-line recognition for SetConfig()
    AddCmdList(myPhysicsConfigKeys);
    
    fOutputFile = 0;

    fMCVertX = 0;
    fMCVertY = 0;
    fMCVertZ = 0;
    
    fMCBranchDirCos = 0;
    fMCBranchVert = 0;

    fLastEvent = 0;

    fEventCounter       = 0;
    fEventOffset        = 0;
    fSaveEvent          = 0;
    fScalerReadCounter  = 0;
    fUseBadScalerReads  = kFALSE;

    fTaggerPhotonNhits  = 0;                 
    fTaggerPhotonHits   = 0;                 
    fTaggerPhotonEnergy = 0;                 
    fTaggerPhotonTime   = 0;                 

    fNaINhits         = 0;
    fNaIHits          = 0;
    fNaIEnergy        = 0;
    fNaITime          = 0;
    fCBClTime         = 0;       
    fCBPattern        = 0; 
    fNaILEDPattern    = -1;
    fNaILEDNhits      = 0;         
    fNaILEDHits       = 0;           
    fCBClusterHit     = 0;         
    fCBNCluster       = 0;          
    fCBCluster        = 0;  
    fCBEnergyScale    = 1.;

    fPIDNhits         = 0;               
    fPIDHits          = 0;               
    fPIDHitPos        = 0;            
    fPIDEnergy        = 0;
    fPIDTime          = 0;
    
    fMWPCNtracks      = 0;
    fMWPCtracks       = 0;

    fTAPSType         = kTAPS_2007;
    fBaF2PWO          = 0;   
    fBaF2PWONhits     = 0;            
    fBaF2PWOHits      = 0;            
    fBaF2PWOEnergy    = 0;
    fBaF2PWOTime      = 0;         
    fTAPSPattern      = 0; 
    fBaF2CFDPattern   = -1;
    fBaF2LED1Pattern  = -1;
    fBaF2LED2Pattern  = -1;
    fVetoLEDPattern   = -1;
    fBaF2CFDNhits     = 0;         
    fBaF2CFDHits      = 0;           
    fBaF2LED1Nhits    = 0;           
    fBaF2LED1Hits     = 0;           
    fBaF2LED2Nhits    = 0;           
    fBaF2LED2Hits     = 0;            
    fVetoLEDNhits     = 0;           
    fVetoLEDHits      = 0;            
    fTAPSClusterHit   = 0;          
    fTAPSNCluster     = 0;         
    fTAPSCluster      = 0;  
    fTAPSEnergyScale  = 1.;

    fVetoNhits        = 0;          
    fVetoHits         = 0;          
    fVetoEnergy       = 0;
    fVetoTime         = 0;

    fPbWO4Nhits       = 0;      
    fPbWO4Hits        = 0;
    fPbWO4Energy      = 0;
    fPbWO4Time        = 0;

    fCP               =  0;     
    fEBeamBitPos      = 10;
    fEBeamBitNeg      =  4;
    fEBeamBitADC      =  6;

    fTOFBarNhits      = 0;          
    fTOFBarHits       = 0;          
    fTOFBarMeanEnergy = 0;      
    fTOFBarMeanTime   = 0;      
  
    fPbGlassNhits     = 0;        
    fPbGlassTime      = 0;    

    fNTotal   = 0;
    fNNeutral = 0;
    fNCharged = 0;
    fPart     = 0;
    fPartCB   = 0;
    fPartTAPS = 0;
    fPartNeutral = 0;
    fPartCharged = 0;

    fNMC = 0;
    for (Int_t i = 0; i < gMyPhysics_MaxMCPart; i++) fPartMC[i] = new TOMCParticle(kRootino, kFALSE);
    
    fTrigger = new TOA2Trigger();
    fL1Bit_CB          = 0x01;
    fL1Bit_TAPS_OR     = 0x08;
    fL1Bit_TAPS_M2     = 0x40;
    fL1Bit_Pulser      = 0x80;
    fL1Bit_TAPS_Pulser = 0x10;
    fCBSumInL2         = kFALSE;
    fL2Bit_M1          = 0x00;
    fL2Bit_M2          = 0x00;
    fL2Bit_M3          = 0x00;
    fL2Bit_M4          = 0x00;

    fTrigTotalMult       = 0;
    fTrigIsTAPSTotalMult = kTRUE;
    fTrigTAPSMult        = 0;
    fTrigCBSum           = 0;
    fTrigCBMeanGain      = 1;
    fTrigCBRelGains      = 0;
    fTrigCBSumCDF        = 0;
    fTrigCBSegThr        = 0;
    fTrigTAPSLED1Thr     = 0;  
    fTrigTAPSLED2Thr     = 0; 
    fNTrigTAPSExclRings  = 0;
    fTrigTAPSExclRings   = new Int_t[gMyPhysics_MaxTAPSRing];

    fNBadTaggerChannels = 0;
    fBadTaggerChannels = new Int_t[gMyPhysics_MaxTaggerCh];
    fNBadCBClusters = 0;
    fBadCBClusters = new Int_t[gMyPhysics_MaxNaI];
    fNBadTAPSClusters = 0;
    fBadTAPSClusters = new Int_t[gMyPhysics_MaxBaF2PWO];
    fNBadParticleCuts = 0;
    fBadParticleCuts = 0;

    fOldScalerSum = new Double_t[gAR->GetMaxScaler()];
    for (Int_t i = 0; i < gAR->GetMaxScaler(); i++) fOldScalerSum[i] = 0.;
    
    fNBadScalerReads = 0;
    fBadScalerReads = 0;
    fIsBadScalerSkip = kFALSE;
    fH_BadScR_SumScalers = 0;
    fH_BadScR_SumFPDScalers = 0;

    fH_CB_PID_Coinc_Hits = new TH1*[gMyPhysics_MaxPID];
    fH_CB_PID_dE_E = new TH2*[gMyPhysics_MaxPID];
    for (Int_t i = 0; i < gMyPhysics_MaxPID; i++) 
    {
        // CB-PID coincidence plots
        sprintf(tmp, "CB_PID_Coinc_Hits_%02d", i);
        //fH_CB_PID_Coinc_Hits[i] = new TH1F(tmp, tmp, 360, 0, 180);
        
        // CB-PID dE vs. E plots
        sprintf(tmp, "CB_PID_dE_E_%02d", i);
        //fH_CB_PID_dE_E[i] = new TH2F(tmp, tmp, 1000, 0, 1000, 400, 0, 20);
    }

    fCBPIDPhiLimit = 15.;
    fPIDDroopCorr = 0;
    fPIDdEECuts = 0;

    fPSA = 0;
    
    fPrompt_Min_CB = 0;
    fPrompt_Max_CB = 0;
    fPrompt_Min_TAPS = 0;
    fPrompt_Max_TAPS = 0;

    fProtonECorrCB = 0;
    fProtonECorrTAPS = 0;

    fPhotonResThetaCB = 0;
    fPhotonResPhiCB = 0;
    fPhotonResEnergyCB = 0;
    fPhotonResThetaTAPS = 0;
    fPhotonResPhiTAPS = 0;
    fPhotonResEnergyTAPS = 0;

    // CaLib
    fCaLibReader = 0;
}

//______________________________________________________________________________
TA2MyPhysics::~TA2MyPhysics()
{
    // Destructor.
    
    if (fBadTaggerChannels) delete [] fBadTaggerChannels;
    if (fBadCBClusters) delete [] fBadCBClusters;
    if (fBadTAPSClusters) delete [] fBadTAPSClusters;
}

//______________________________________________________________________________
void TA2MyPhysics::SetConfig(Char_t* line, Int_t key)
{
    // Read in analysis configuration parameters and configure the class. 

    switch (key)
    {
        case EMP_CALIB_BADSCR:
        {
            // declare sscanf helpers (max. 16 data names)
            Int_t list;
            Int_t first;
            Int_t last;
            Char_t* d[16];
            for (Int_t i = 0; i < 16; i++) d[i] = new Char_t[128];

            // read config line
            Int_t nfields = sscanf(line, "%i%i%i%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", &list, &first, &last,
                                   d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);

            // check whether CaLib reader exists
            if (fCaLibReader)
            {
                // set flags
                fCaLibReader->SetBadScRlist((Bool_t) list);
                fCaLibReader->SetBadScRfirst((Bool_t) first);
                fCaLibReader->SetBadScRlast((Bool_t) last);

                // read (from database) & configure bad scaler reads
                if (fCaLibReader->ReadScalerReads(nfields-3, d))
                {
                    // set members
                    fNBadScalerReads = fCaLibReader->GetNBadScR();
                    fBadScalerReads = fCaLibReader->GetBadScR();

                    // activate bad scaler reads
                    fUseBadScalerReads = kTRUE;
                }
                else
                {
                    Error("SetConfig", "Bad scaler read could not be configured!");
                }
            }
            else
            {
                Error("SetConfig", "Bad scaler read cannot be configured because CaLib was not configured!");
            }

            // clean up
            for (Int_t i = 0; i < 16; i++) delete d[i];
            break;
        }
        case EMP_RUN_NUMBER:
        {   
            // read run number
            if (sscanf(line, "%d", &fRunNumber) == 1)
                Info("SetConfig", "Overwriting run number with %d", fRunNumber);
            else
                Error("SetConfig", "Could not overwrite run number!");
            break;
        }
        case EMP_BAD_TAGG_CH:
        {   
            Char_t fileName[256];

            // read number and list of bad tagger channels
            if (sscanf(line, "%s", fileName))
                fNBadTaggerChannels = TOLoader::LoadBadChannels(gSystem->ExpandPathName(fileName), fBadTaggerChannels);
            break;
        }
        case EMP_BAD_CB_CLUSTERS:
        {   
            Char_t fileName[256];

            // read number and list of bad CB cluster centers
            if (sscanf(line, "%s", fileName))
                fNBadCBClusters = TOLoader::LoadBadChannels(gSystem->ExpandPathName(fileName), fBadCBClusters);
            break;
        }
        case EMP_BAD_TAPS_CLUSTERS:
        {   
            Char_t fileName[256];

            // read number and list of bad TAPS cluster centers
            if (sscanf(line, "%s", fileName))
                fNBadTAPSClusters = TOLoader::LoadBadChannels(gSystem->ExpandPathName(fileName), fBadTAPSClusters);
            break;
        }
        case EMP_BAD_PARTICLE_CUTS:
        {   
            // read number of bad particle cuts
            if (sscanf(line, "%d", &fNBadParticleCuts) == 1)
            {
                fBadParticleCuts = new TOKinCut*[fNBadParticleCuts];
                Info("SetConfig", "Init %d bad particle cut(s)", fNBadParticleCuts);
            }
            else
                Error("SetConfig", "Bad syntax in bad particle cuts definition!");
            break;
        }
        case EMP_BAD_PARTICLE_CUT_PHI:
        {
            Int_t id;
            Double_t min, max;
            // read id and min/max values of cut
            if (sscanf(line, "%d%lf%lf", &id, &min, &max) == 3)
            {
                fBadParticleCuts[id] = new TOKinCut(kPhiCut, min, max);
                Info("SetConfig", "Setting up bad particle cut %d: Exclude phi range from %.2f to %.2f degrees", id, min, max);
            }
            else
                Error("SetConfig", "Bad syntax in bad particle phi cut definition!");
            break;
        }
        case EMP_TRIGGER_TOTAL_MULT:
        {
            if (sscanf(line, "%d", &fTrigTotalMult) != 1)
                Error("SetConfig", "Bad syntax in trigger total multiplicity definition!");
            break;
        }
        case EMP_TRIGGER_TOTAL_MULT_NO_TAPS:
        {
            fTrigIsTAPSTotalMult = kFALSE;
            Info("SetConfig", "TAPS multiplicity will not contribute to total multiplicity in software trigger");
            break;
        }
        case EMP_TRIGGER_TAPS_MULT:
        {
            if (sscanf(line, "%d", &fTrigTAPSMult) != 1)
                Error("SetConfig", "Bad syntax in trigger TAPS multiplicity definition!");
            break;
        }
        case EMP_TRIGGER_CB_THRESHOLDS:
        {
            if (sscanf(line, "%lf%lf", &fTrigCBSum, &fTrigCBSegThr) != 2)
                Error("SetConfig", "Bad syntax in trigger CB thresholds definition!");
            break;
        }
        case EMP_TRIGGER_CB_REL_GAINS:
        {
            Char_t fileName[256];
            
            // load the CB NaI relative gains
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in trigger CB relative NaI gains definition!");
            else 
            {   
                // check if not MC
                if (!fIsMC)
                {
                    Error("SetConfig", "Relative CB NaI gains are only used in MC analysis - ignoring!");
                    break;
                }

                // create array
                fTrigCBRelGains = new Double_t[gMyPhysics_MaxNaI];

                // read gains
                TONumberReader r(gSystem->ExpandPathName(fileName), 1);
                for (Int_t i = 0; i < gMyPhysics_MaxNaI; i++) fTrigCBRelGains[i] = r.GetData()->Get(0, i);
            
                Info("SetConfig", "Relative CB NaI gains were read from '%s'", gSystem->ExpandPathName(fileName));
            }   
            break;
        }
        case EMP_TRIGGER_CB_SUM_CDF:
        {
            Char_t fileName[256];
            
            // load the CB CDF TF1 from the ROOT file
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in trigger CB sum CDF definition!");
            else TOLoader::LoadObject(gSystem->ExpandPathName(fileName), "CDF", &fTrigCBSumCDF);
            break;
        }
        case EMP_TRIGGER_TAPS_LED1_THRESHOLD:
        {
            if (sscanf(line, "%lf", &fTrigTAPSLED1Thr) != 1)
                Error("SetConfig", "Bad syntax in trigger TAPS LED1 thresholds definition!");
            break;
        }
        case EMP_TRIGGER_TAPS_LED2_THRESHOLD:
        {
            if (sscanf(line, "%lf", &fTrigTAPSLED2Thr) != 1)
                Error("SetConfig", "Bad syntax in trigger TAPS LED2 thresholds definition!");
            break;
        }
        case EMP_TRIGGER_TAPS_EXCLUDED_RINGS:
        {   
            Char_t fileName[256];

            // read number and list of trigger-excluded TAPS rings
            if (sscanf(line, "%s", fileName))
                fNTrigTAPSExclRings = TOLoader::LoadBadChannels(gSystem->ExpandPathName(fileName), fTrigTAPSExclRings);
            break;
        }
        case EMP_CB_PID_PHI_LIMIT:
        {
            if (sscanf(line, "%lf", &fCBPIDPhiLimit) != 1)
                Error("SetConfig", "Bad syntax in the CB-PID coincidence phi limit!");
            else Info("SetConfig", "Using a CB-PID coincidence phi limit of %.2f degrees", fCBPIDPhiLimit);
            break;
        }
        case EMP_BEAM_POL_BITS:
        {
            if (sscanf(line, "%d%d", &fEBeamBitPos, &fEBeamBitNeg) != 2)
                Error("SetConfig", "Bad syntax in beam helicity bit definition!");
            else Info("SetConfig", "Using beam helicity bit: %d (+) and %d (-)", fEBeamBitPos, fEBeamBitNeg);
            break;
        }
        case EMP_BEAM_POL_ADC:
        {
            if (sscanf(line, "%d", &fEBeamBitADC) != 1)
                Error("SetConfig", "Bad syntax in beam helicity bit ADC definition!");
            else Info("SetConfig", "Using beam helicity bit ADC %d", fEBeamBitADC);
            break;
        }
        case EMP_L1_BITS:
        {
            if (sscanf(line, "%x%x%x%x%x", &fL1Bit_CB, &fL1Bit_TAPS_OR, &fL1Bit_TAPS_M2, &fL1Bit_Pulser,
                                           &fL1Bit_TAPS_Pulser) != 5)
                Error("SetConfig", "Bad syntax in L1 trigger bits definition!");
            else Info("SetConfig", "Using L1 trigger bits: CB (0x%.4x), TAPS_OR (0x%.4x), TAPS_M2 (0x%.4x), "
                                   "Pulser (0x%.4x), TAPS Pulser (0x%.4x)",
                      fL1Bit_CB, fL1Bit_TAPS_OR, fL1Bit_TAPS_M2, fL1Bit_Pulser, fL1Bit_TAPS_Pulser);
            break;
        }
        case EMP_L1_CBSUM_L2:
        {
            fCBSumInL2 = kTRUE;
            Info("SetConfig", "Reading CB sum trigger from L2 trigger pattern");
            break;
        }
        case EMP_L2_MULTI_BITS:
        {
            if (sscanf(line, "%x%x%x%x", &fL2Bit_M1, &fL2Bit_M2, &fL2Bit_M3, &fL2Bit_M4) != 4)
                Error("SetConfig", "Bad syntax in L2 trigger multiplicity bits definition!");
            else Info("SetConfig", "Using L2 trigger multiplicity bits: M1+ (0x%.4x), M2+ (0x%.4x), M3+ (0x%.4x), M4+ (0x%.4x)",
                      fL2Bit_M1, fL2Bit_M2, fL2Bit_M3, fL2Bit_M4);
            break;
        }
        case EMP_PID_DROOP_CORR:
        {
            Char_t droop_file[256];
            if (sscanf(line, "%s", droop_file) != 1)
                Error("SetConfig", "Bad syntax in PID droop correction file location!");
            else 
            {
                // create PID droop correction TGraph array
                fPIDDroopCorr = new TGraph*[gMyPhysics_MaxPID];
                if (TOLoader::LoadObjects(gSystem->ExpandPathName(droop_file), "Droop_Corr_%02d", &fPIDDroopCorr, 0, 23, "", "q"))
                    Info("SetConfig", "Using PID droop correction from '%s'", droop_file);
                else
                {
                    delete [] fPIDDroopCorr;
                    fPIDDroopCorr = 0;
                    Error("SetConfig", "Could not load the PID droop correction from '%s'", droop_file);
                }
            }
            break;
        }
        case EMP_LAST_EVENT:
        {
            if (sscanf(line, "%lld", &fLastEvent) != 1)
                Error("SetConfig", "Bad syntax in last event definition!");
            else Info("SetConfig", "Aborting analysis after event %lld\n", fLastEvent);
            break;
        }
        case EMP_TAPS_PSA_DATA:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in TAPS PSA data definition!");
            else 
            {
                // create PSA object if necessary
                if (!fPSA) fPSA = new TOA2PSA(fileName);
                else fPSA->LoadPSAData(fileName);
                Info("SetConfig", "Loaded PSA data from '%s'", fileName);
            }
            break;
        }
        case EMP_TAPS_PSA_CONFIG:
        {
            Double_t sig, min, max;
            if (sscanf(line, "%lf%lf%lf", &sig, &min, &max) != 3)
                Error("SetConfig", "Bad syntax in TAPS PSA configuration!");
            else 
            {
                // create PSA object if necessary
                if (!fPSA) fPSA = new TOA2PSA(sig, min, max);
                else 
                {
                    fPSA->SetPhotonSigmaFactor(sig);
                    fPSA->SetNucleonExclZone(min, max);
                }
                Info("SetConfig", "Setting TAPS PSA photon factor to %.2lf", sig);
                Info("SetConfig", "Exclusion zone for PSA nucleons from %.2lf to %.2lf", min, max);
            }
            break;
        }
        case EMP_TAGGER_PROMPT:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in Tagger prompt definition!");
            else 
            {
                // load graphs
                if (TOLoader::LoadObject(fileName, "CB_Min", &fPrompt_Min_CB, "", "q") &&
                    TOLoader::LoadObject(fileName, "CB_Max", &fPrompt_Max_CB, "", "q") &&
                    TOLoader::LoadObject(fileName, "TAPS_Min", &fPrompt_Min_TAPS, "", "q") &&
                    TOLoader::LoadObject(fileName, "TAPS_Max", &fPrompt_Max_TAPS, "", "q"))
                    Info("SetConfig", "Loaded Tagger prompt window data");
                else Error("SetConfig", "Could not load all Tagger prompt window data!");
            }
            break;
        }
        case EMP_PROTON_ECORR_CB:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in CB proton energy correction definition!");
            else 
            {
                Char_t tmp[256];

                // create graph array
                fProtonECorrCB = new TGraph*[gMyPhysics_MaxNaI];

                // load graphs
                Int_t nLoad = 0;
                for (Int_t i = 0; i < gMyPhysics_MaxNaI; i++)
                {
                    sprintf(tmp, "ProtonECorr_%03d", i);
                    if (TOLoader::LoadObject(fileName, tmp, &fProtonECorrCB[i], "", "q")) nLoad++;
                }
                Info("SetConfig", "Loaded proton energy correction data for %d CB detectors", nLoad);
            }
            break;
        }
        case EMP_PROTON_ECORR_TAPS:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in TAPS proton energy correction definition!");
            else 
            {
                Char_t tmp[256];

                // create graph array
                fProtonECorrTAPS = new TGraph*[gMyPhysics_MaxBaF2PWO];

                // load graphs
                Int_t nLoad = 0;
                for (Int_t i = 0; i < gMyPhysics_MaxBaF2PWO; i++)
                {
                    sprintf(tmp, "ProtonECorr_%03d", i);
                    if (TOLoader::LoadObject(fileName, tmp, &fProtonECorrTAPS[i], "", "q")) nLoad++;
                }
                Info("SetConfig", "Loaded proton energy correction data for %d TAPS detectors", nLoad);
            }
            break;
        }
        case EMP_PHOTON_RES_CB:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in CB photon resolution file definition!");
            else 
            {
                // load graphs
                if (TOLoader::LoadObject(fileName, "ThetaRes", &fPhotonResThetaCB, "", "q") &&
                    TOLoader::LoadObject(fileName, "PhiRes", &fPhotonResPhiCB, "", "q") &&
                    TOLoader::LoadObject(fileName, "EnergyRes", &fPhotonResEnergyCB, "", "q"))
                    Info("SetConfig", "Loaded CB photon resolutions");
                else Error("SetConfig", "Could not load all CB photon resolutions!");
            }
            break;
        }
        case EMP_PHOTON_RES_TAPS:
        {
            Char_t fileName[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in TAPS photon resolution file definition!");
            else 
            {
                // load graphs
                if (TOLoader::LoadObject(fileName, "ThetaRes", &fPhotonResThetaTAPS, "", "q") &&
                    TOLoader::LoadObject(fileName, "PhiRes", &fPhotonResPhiTAPS, "", "q") &&
                    TOLoader::LoadObject(fileName, "EnergyRes", &fPhotonResEnergyTAPS, "", "q"))
                    Info("SetConfig", "Loaded TAPS photon resolutions");
                else Error("SetConfig", "Could not load all TAPS photon resolutions!");
            }
            break;
        }
//        case EMP_PARTICLE_RES:
//        {
//            Char_t fileName[256];
        
//            if (sscanf(line, "%s", fileName) != 1)
//                Error("SetConfig", "Bad syntax in particle resolution file definition!");
//            else 
//            {
//                // load resolutions
//                if (TOLoader::LoadObject(fileName, "DetRes", &fParticleRes, "", "q"))
//                    Info("SetConfig", "Loaded particle resolutions");
//                else 
//                    Error("SetConfig", "Could not load particle resolutions!");
//            }
//            break;
//        }
        case EMP_CB_TAPS_ENERGY_SCALE:
        {
            if (sscanf(line, "%lf%lf", &fCBEnergyScale, &fTAPSEnergyScale) != 2)
                Error("SetConfig", "Bad syntax in CB/TAPS energy scale definition!");
            else 
            {
                Info("SetConfig", "Setting CB energy scaling to %lf", fCBEnergyScale);
                Info("SetConfig", "Setting TAPS energy scaling to %lf", fTAPSEnergyScale);
            }
            break;
        }
        case EMP_PID_DE_E_CUTS:
        {
            Char_t fileName[256];
            Char_t tmp[256];
        
            if (sscanf(line, "%s", fileName) != 1)
                Error("SetConfig", "Bad syntax in PID dE-E file definition!");
            else 
            {
                // load function array
                fPIDdEECuts = new TF1*[gMyPhysics_MaxPID];

                // load cut functions
                Int_t nLoad = 0;
                for (Int_t i = 0; i < gMyPhysics_MaxPID; i++)
                {
                    sprintf(tmp, "cut_%d", i);
                    if (TOLoader::LoadObject(fileName, tmp, &fPIDdEECuts[i], "", "q")) nLoad++;
                }

                // check loading
                if (nLoad == gMyPhysics_MaxPID) Info("SetConfig", "Loaded dE-E cuts for %d PID elements", nLoad);
                else Error("SetConfig", "Could not load all PID dE-E cuts!");
            }
            break;
        }
        default:
        {
            // default SetConfig() of base class
            TA2AccessSQL::SetConfig(line, key);
            break;
        }
    }
}

//______________________________________________________________________________ 
Int_t TA2MyPhysics::GetPatternIndex(const Char_t* patternName, TA2BitPattern* pattern)
{
    // Returns the index of the pattern called 'patternName' in the bit pattern 'pattern'.
    // If no such pattern is found -1 is returned.

    // check if pattern exists
    if (!pattern) return -1;

    // loop over pattern
    for (Int_t i = 0; i < pattern->GetNPattern(); i++)
    {
        if (strcmp(patternName, pattern->GetPatternName(i)) == 0) return i;
    }

    return -1;
}


//______________________________________________________________________________ 
void TA2MyPhysics::PostInit()
{
    // Do some further initialisation after all setup parameters have been read in.
 
    // call PostInit() of parent class
    TA2AccessSQL::PostInit();

    // show information about the selected analysis class
    printf("\n\n\n  ****** PHYSICS ANALYSIS ******\n\n");
    printf("   Name       : %s\n", GetName());
    printf("   Class Name : %s\n", ClassName());
    printf("\n");
 
    // calculate the mean CB NaI gain
    if (fNaI) fTrigCBMeanGain = CalculateMeanCBGain();
    
    // show the hardware trigger settings
    if (!fIsMC)
    {
        printf("\n\n\n  ****** HARDWARE TRIGGER ******\n\n");
        printf("   L1 Bits\n");
        printf("   CB sum      : 0x%.4x", fL1Bit_CB);
        if (fCBSumInL2) printf(" (read from L2 pattern)\n");
        else printf("\n");
        printf("   TAPS OR     : 0x%.4x\n", fL1Bit_TAPS_OR);
        printf("   TAPS M2     : 0x%.4x\n", fL1Bit_TAPS_M2);
        printf("   Pulser      : 0x%.4x\n", fL1Bit_Pulser);
        printf("   TAPS Pulser : 0x%.4x\n", fL1Bit_TAPS_Pulser);
        printf("\n");
        printf("   L2 Bits\n");
        printf("   M1(+)       : 0x%.4x\n", fL2Bit_M1);
        printf("   M2(+)       : 0x%.4x\n", fL2Bit_M2);
        printf("   M3(+)       : 0x%.4x\n", fL2Bit_M3);
        printf("   M4+         : 0x%.4x\n", fL2Bit_M4);
    }

    // show the software trigger settings
    printf("\n\n\n  ****** SOFTWARE TRIGGER ******\n\n");
    printf("   Total multiplicity        : %d or more ", fTrigTotalMult);
    if (fTrigIsTAPSTotalMult) printf("(TAPS included)\n");
    else printf("(TAPS excluded)\n");
    printf("   TAPS alone multiplicity   : %d or more\n", fTrigTAPSMult);
    printf("   CB sum                    : %6.2f MeV", fTrigCBSum);
    if (fTrigCBSumCDF) printf(" (CDF for MC)\n");
    else printf(" (No CDF for MC)\n");
    printf("   CB NaI mean gain          : %8.6f MeV/Ch\n", fTrigCBMeanGain);
    if (fTrigCBRelGains) printf("   Relative CB NaI gains     : yes\n");
    if (fCaLibReader)
    {
        // CB LED
        if (fCaLibReader->GetCBled())
            printf("   CB segment LED threshold  : database value * %6.2f MeV\n", fTrigCBSegThr);
        else
            printf("   CB segment LED threshold  : %6.2f MeV\n", fTrigCBSegThr);
        
        // TAPS LED1
        if (fCaLibReader->GetTAPSled())
        {
            printf("   TAPS LED1 threshold       : database value * %6.2f MeV\n", fTrigTAPSLED1Thr);
            printf("   TAPS LED2 threshold       : database value * %6.2f MeV\n", fTrigTAPSLED2Thr);
        }
        else
        {
            printf("   TAPS LED1 threshold       : %6.2f MeV\n", fTrigTAPSLED1Thr);
            printf("   TAPS LED2 threshold       : %6.2f MeV\n", fTrigTAPSLED2Thr);
        }
    }
    else
    {
        printf("   CB segment LED threshold  : %6.2f MeV\n", fTrigCBSegThr);
        printf("   TAPS LED1 threshold       : %6.2f MeV\n", fTrigTAPSLED1Thr);
        printf("   TAPS LED2 threshold       : %6.2f MeV\n", fTrigTAPSLED2Thr);
    }
    if (fNTrigTAPSExclRings)
    {
        printf("   %2d TAPS ring(s) excluded  : %s", fNTrigTAPSExclRings, 
                                                     TOSUtils::FormatArrayList(fNTrigTAPSExclRings, fTrigTAPSExclRings));
    }

    printf("\n\n\n");

    // print scaler read list
    if (fUseBadScalerReads && fBadScalerReads)
    {
        printf("  %d Bad scaler read(s): %s", fNBadScalerReads,
                                              TOSUtils::FormatArrayList(fNBadScalerReads, fBadScalerReads));
        printf("\n\n");
    }
     
    // print bad tagger list
    if (fBadTaggerChannels)
    {
        printf("  %d Bad tagger channel(s): %s", fNBadTaggerChannels, 
                                                 TOSUtils::FormatArrayList(fNBadTaggerChannels, fBadTaggerChannels));
        printf("\n\n");
    }

    // print bad CB cluster list
    if (fBadCBClusters)
    {
        printf("  %d Bad CB cluster center(s): %s", fNBadCBClusters, 
                                                    TOSUtils::FormatArrayList(fNBadCBClusters, fBadCBClusters));
        printf("\n\n");
    }

    // print bad TAPS cluster list
    if (fBadTAPSClusters)
    {
        printf("  %d Bad TAPS cluster center(s): %s", fNBadTAPSClusters, 
                                                      TOSUtils::FormatArrayList(fNBadTAPSClusters, fBadTAPSClusters));
        printf("\n\n");
    }

    // assign the TAPS pointers
    if (fTAPS && fBaF2PWO)
    {
        if      (fBaF2PWO->GetType() == EBaF2)        fTAPSType = kTAPS_2007;
        else if (fBaF2PWO->GetType() == EBaF2_PWO_08) fTAPSType = kTAPS_2008;
        else if (fBaF2PWO->GetType() == EBaF2_PWO_09) fTAPSType = kTAPS_2009;

        fBaF2PWOHits     = fBaF2PWO->GetHits();
        fBaF2PWOEnergy   = fBaF2PWO->GetEnergy();
        fBaF2PWOTime     = fBaF2PWO->GetTime();

        fTAPSPattern     = fBaF2PWO->GetBitPattern();
        fBaF2CFDPattern  = GetPatternIndex("TAPSCFDPattern", fTAPSPattern);
        fBaF2LED1Pattern = GetPatternIndex("TAPSLED1Pattern", fTAPSPattern);
        fBaF2LED2Pattern = GetPatternIndex("TAPSLED2Pattern", fTAPSPattern);
        fVetoLEDPattern  = GetPatternIndex("TAPSVetoPattern", fTAPSPattern);
        
        if (fBaF2CFDPattern >= 0)
        {
            fBaF2CFDHits = fTAPSPattern->GetHits(fBaF2CFDPattern);
            printf("  BaF2 CFD Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2CFDPattern), fBaF2CFDPattern);
        }

        if (fBaF2LED1Pattern >= 0)
        {
            fBaF2LED1Hits = fTAPSPattern->GetHits(fBaF2LED1Pattern);
            printf("  BaF2 LED1 Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2LED1Pattern), fBaF2LED1Pattern);
        }

        if (fBaF2LED2Pattern >= 0)
        {
            fBaF2LED2Hits = fTAPSPattern->GetHits(fBaF2LED2Pattern);
            printf("  BaF2 LED2 Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2LED2Pattern), fBaF2LED2Pattern);
        }

        if (fVetoLEDPattern >= 0)
        {
            fVetoLEDHits = fTAPSPattern->GetHits(fVetoLEDPattern);
            printf("  Veto LED Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fVetoLEDPattern), fVetoLEDPattern);
        }

        fTAPSClusterHit  = fBaF2PWO->GetClustHit();
        fTAPSCluster     = fBaF2PWO->GetCluster();
    }
    
    // assign the CB pointers
    if (fCB && fNaI)
    {
        fCBClTime       = fNaI->GetClTimeOR();
        fNaIHits        = fNaI->GetHits();
        fNaIEnergy      = fNaI->GetEnergy();
        fNaITime        = fNaI->GetTime();

//        fCBPID_Index    = fCB->GetPIDElementIndex();
//        fCBPID_dE       = fCB->GetPIDEnergyDepo();
        fCBClusterHit   = fNaI->GetClustHit();
        fCBCluster      = fNaI->GetCluster();
        
        fCBPattern     = fNaI->GetBitPattern();
        fNaILEDPattern = GetPatternIndex("CBLEDPattern", fCBPattern);
        
        if (fNaILEDPattern >= 0)
        {
            fNaILEDHits = fCBPattern->GetHits(fNaILEDPattern);
            printf("  NaI LED Pattern '%s' found at index %d\n", fCBPattern->GetPatternName(fNaILEDPattern), fNaILEDPattern);
        }
    }
    
    // assign the PID pointers
    if (fPID)
    {
        fPIDHits    = fPID->GetHits();
        fPIDHitPos  = fPID->GetPosition();
        fPIDEnergy  = fPID->GetEnergy();
        fPIDTime    = fPID->GetTime();
    }

    // assign the MWPC pointers
    if (fMWPC)
    {
        fMWPCtracks = fMWPC->GetTracks();
    }

    // assign the Veto pointers
    if (fVeto)
    {
        fVetoHits    = fVeto->GetHits();
        fVetoEnergy  = fVeto->GetEnergy();
        fVetoTime    = fVeto->GetTime();
    }
    
    // assign the PbWO4 pointers
    if (fPbWO4)
    {
        fPbWO4Hits   = fPbWO4->GetHits();
        fPbWO4Energy = fPbWO4->GetEnergy();
        fPbWO4Time   = fPbWO4->GetTime();
    }

    // assign the TOF pointers
    if (fTOF)
    {
        fTOFBarHits       = fTOF->GetBarHits();
        fTOFBarMeanEnergy = fTOF->GetMeanEnergy();
        fTOFBarMeanTime   = fTOF->GetMeanTime();
    }

    // assign the Pb glass pointers
    if (fPbGlassApp && fPbGlass)
    {
        fPbGlassTime = fPbGlass->GetTime();
    }

    // electron beam helicity state pointer
    fCP = &fADC[fEBeamBitADC]; 

    // apply CB/TAPS energy scaling
    if (fNaI) fNaI->SetEnergyScale(fCBEnergyScale);
    if (fBaF2PWO) fBaF2PWO->SetEnergyScale(fTAPSEnergyScale);

    // create histograms
    fH_EventInfo = new TH1F("EventInfo", "EventInfo", 20, 0, 20);
    fH_EventInfo->GetXaxis()->SetBinLabel(1, "Events");
    fH_EventInfo->GetXaxis()->SetBinLabel(2, "Hel+");
    fH_EventInfo->GetXaxis()->SetBinLabel(3, "Hel-");
    fH_EventInfo->GetXaxis()->SetBinLabel(4, "Hel undef");
    fH_EventInfo->GetXaxis()->SetBinLabel(5, "CB");
    fH_EventInfo->GetXaxis()->SetBinLabel(6, "TAPS OR");
    fH_EventInfo->GetXaxis()->SetBinLabel(7, "TAPS Pulser");
    fH_EventInfo->GetXaxis()->SetBinLabel(8, "TAPS M2");
    fH_EventInfo->GetXaxis()->SetBinLabel(9, "Pulser");
    fH_EventInfo->GetXaxis()->SetBinLabel(10, "M1(+)");
    fH_EventInfo->GetXaxis()->SetBinLabel(11, "M2(+)");
    fH_EventInfo->GetXaxis()->SetBinLabel(12, "M3(+)");
    fH_EventInfo->GetXaxis()->SetBinLabel(13, "M4(+)");
    fH_EventInfo->GetXaxis()->SetBinLabel(14, "Scaler reads");
    fH_EventInfo->GetXaxis()->SetBinLabel(15, "HW Errors");
    if (fIsMC)
    {
        fH_MCVertX = new TH1F("MCVertexX", "MCVertexX", 400, -10, 10);
        fH_MCVertY = new TH1F("MCVertexY", "MCVertexY", 400, -10, 10);
        fH_MCVertZ = new TH1F("MCVertexZ", "MCVertexZ", 400, -10, 10);
    }
    else
    {
        fH_ErrorInfo = new TH2F("ErrorInfo", "ErrorInfo;Module Index;Error Code", 50000, 0, 50000, 20, 0, 20);
        fH_Corrected_Scalers = new TH1F("CorrectedScalers", "CorrectedScalers", gAR->GetMaxScaler(), 0, gAR->GetMaxScaler());
        fH_Corrected_SumScalers = new TH1F("CorrectedSumScalers", "CorrectedSumScalers", gAR->GetMaxScaler(), 0, gAR->GetMaxScaler());

        if (fUseBadScalerReads)
        {
            fH_BadScR_SumScalers = new TH1D("BadScR_SumScalers", "BadScR_SumScalers", gAR->GetMaxScaler(), 0, gAR->GetMaxScaler());
            if (fLadder) fH_BadScR_SumFPDScalers = new TH1D("BadScR_SumFPDScalers", "BadScR_SumFPDScalers", fLadder->GetNelement(), 0, fLadder->GetNelement());
        }
    }
}

//______________________________________________________________________________ 
void TA2MyPhysics::LoadVariable()
{
    // Register class variables within AcquRoot.

    // variable for event saving into the reduced AcquRoot ROOT file 
    TA2DataManager::LoadVariable("SaveEvent", &fSaveEvent, EISingleX);
}

//______________________________________________________________________________ 
void TA2MyPhysics::Reconstruct()
{
    // AcquRoot Reconstruct() method.
 
    ////////////////////////////////////////////////////////////////////////////
    // Process (bad) scaler read                                              //
    ////////////////////////////////////////////////////////////////////////////

    // check for scaler read event
    if (!fIsMC && gAR->IsScalerRead())
    {
        // reset event skipping flag
        fIsBadScalerSkip = kFALSE;

        // increment scaler reads counter
        fScalerReadCounter++;

        // check whether previous scaler read (interval) was good in order to update scaler histos
        if (fUseBadScalerReads && !IsBadScalerRead(fScalerReadCounter - 1))
        {
            // loop over scalers
            for (Int_t i = 0; i < gAR->GetMaxScaler(); i++)
            {
                // fill sum scalers
                fH_BadScR_SumScalers->AddBinContent(i+1, (Double_t) fScaler[i]);
            }

            // check for ladder
            if (fLadder)
            {
                // loop over ladder scalers
                for (UInt_t i = 0; i < fLadder->GetNelement(); i++)
                {
                    // fill ladder sum scalers
                    fH_BadScR_SumFPDScalers->AddBinContent(i+1, (Double_t) fLadder->GetScalerCurr()[i]);
                }
            }
        }
    }

    // apply bad scaler read event skip
    if (!fIsMC && fUseBadScalerReads)
    {
        // set the skip flag once for events before first scaler read
        if (fIsBadScalerSkip == kFALSE && IsBadScalerRead(fScalerReadCounter)) fIsBadScalerSkip = kTRUE;
        
        // check for bad scaler read in order to skip event
        if (fIsBadScalerSkip)
        {
            // display user info
            if (fEventCounter == 0 || gAR->IsScalerRead()) Info("Reconstruct", "Skipping scaler read %d", fScalerReadCounter);

            // increment event counter
            fEventCounter++;

            // check if last event was processed and finish the analysis
            if (fLastEvent && fEventCounter > fLastEvent) TerminateAnalysis();

            // skip this event
            return;
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    // Set detector event-dependent variables                                 //
    ////////////////////////////////////////////////////////////////////////////
 
    // do not save current event in reduced AcquRoot ROOT file by default
    fSaveEvent = 0;
    

    ////////////////////////////////////////////////////////////////////////////
    // Set detector event-dependent variables                                 //
    ////////////////////////////////////////////////////////////////////////////
    
    if (fNaI)
    {
        fCBNCluster = fNaI->GetNCluster();
        fNaINhits   = fNaI->GetNhits();
    }

    if (fBaF2PWO)
    {
        fBaF2PWONhits = fBaF2PWO->GetNhits();
        fTAPSNCluster = fBaF2PWO->GetNCluster();
    }

    if (fNaILEDHits) fNaILEDNhits = fCBPattern->GetNHits()[fNaILEDPattern];
    if (fBaF2CFDHits) fBaF2CFDNhits = fTAPSPattern->GetNHits()[fBaF2CFDPattern];
    if (fBaF2LED1Hits) fBaF2LED1Nhits = fTAPSPattern->GetNHits()[fBaF2LED1Pattern];
    if (fBaF2LED2Hits) fBaF2LED2Nhits = fTAPSPattern->GetNHits()[fBaF2LED2Pattern];
    if (fVetoLEDHits) fVetoLEDNhits = fTAPSPattern->GetNHits()[fVetoLEDPattern];

    if (fPID)
    {
        fPIDNhits = fPID->GetNhits();
    }

    if (fVeto)
    {
        fVetoNhits = fVeto->GetNhits();
    }

    if (fPbWO4)
    {
        fPbWO4Nhits = fPbWO4->GetNhits();
    }

    if (fTOF)
    {
        fTOFBarNhits = fTOF->GetNBarHits();
    }

    if (fPbGlass)
    {
        fPbGlassNhits = fPbGlass->GetNhits();
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Read-in beam helicity                                                  //
    ////////////////////////////////////////////////////////////////////////////
    
    if (!fIsMC)
    {
        if (fCP)
        {
            UShort_t eHS = *fCP;
            if (eHS == fEBeamBitNeg) fTrigger->SetBeamHel(kNeg);
            else if (eHS == fEBeamBitPos) fTrigger->SetBeamHel(kPos);
            else fTrigger->SetBeamHel(kUndef);
        }
        else fTrigger->SetBeamHel(kUndef);
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // Read-in hardware trigger                                               //
    ////////////////////////////////////////////////////////////////////////////
  
    if (!fIsMC)
    {
        // Level 1 trigger
        if (fCBSumInL2) 
        {
            if (fADC[1] & fL1Bit_CB)      fTrigger->SetTriggerL1(kCB); 
        }
        else 
        {
            if (fADC[0] & fL1Bit_CB)      fTrigger->SetTriggerL1(kCB); 
        }
        if (fADC[0] & fL1Bit_TAPS_OR)     fTrigger->SetTriggerL1(kTAPS_OR);
        if (fADC[0] & fL1Bit_TAPS_M2)     fTrigger->SetTriggerL1(kTAPS_M2);
        if (fADC[0] & fL1Bit_Pulser)      fTrigger->SetTriggerL1(kPulser);
        if (fADC[0] & fL1Bit_TAPS_Pulser) fTrigger->SetTriggerL1(kTAPS_Pulser);
        
        // Level 2 trigger
        if (fADC[1] & fL2Bit_M1) fTrigger->SetTriggerL2(kM1); 
        if (fADC[1] & fL2Bit_M2) fTrigger->SetTriggerL2(kM2); 
        if (fADC[1] & fL2Bit_M3) fTrigger->SetTriggerL2(kM3); 
        if (fADC[1] & fL2Bit_M4) fTrigger->SetTriggerL2(kM4); 
    }
 

    ////////////////////////////////////////////////////////////////////////////
    // Read-in tagger hits                                                    //
    ////////////////////////////////////////////////////////////////////////////
     
    if (fTagger && fLadder)
    {
        // get the electron beam energy
        Double_t eBeamEnergy = fTagger->GetBeamEnergy();
        
        // get the channel electron energies
        const Double_t* fpdEnergy = fLadder->GetECalibration();

        // data or MC handling
        if (fIsMC)
        {
            // get tagger hits and the time array
            UInt_t nhits = fLadder->GetNhits();
            Int_t* hits = fLadder->GetHits();
            Double_t* time = fLadder->GetTimeOR();
            
            // create hit, energy and time arrays
            fTaggerPhotonNhits = 0;
            fTaggerPhotonHits = new Int_t[nhits];
            fTaggerPhotonEnergy = new Double_t[nhits];
            fTaggerPhotonTime = new Double_t[nhits];
            
            // loop over tagger hits
            for (UInt_t i = 0; i < nhits; i++)
            {
                // skip bad tagger channels
                if (IsBadTaggerChannel(hits[i])) continue;
                
                // set hit element, time and energy
                fTaggerPhotonHits[fTaggerPhotonNhits] = hits[i];
                fTaggerPhotonEnergy[fTaggerPhotonNhits] = eBeamEnergy - fpdEnergy[hits[i]];
                fTaggerPhotonTime[fTaggerPhotonNhits] = time[i];
                fTaggerPhotonNhits++;
            }
        }
        else
        {
            // get the tagger hit multiplicity
            Int_t m = fLadder->GetNMultihit();

            // count total number of hits including all hit multiplicities
            Int_t nhits = 0;
            for (Int_t i = 0; i < m; i++) nhits += fLadder->GetNhitsM(i);
            
            // create hit, energy and time arrays
            fTaggerPhotonNhits = 0;
            fTaggerPhotonHits = new Int_t[nhits];
            fTaggerPhotonEnergy = new Double_t[nhits];
            fTaggerPhotonTime = new Double_t[nhits];
            
            // read-in all the hits
            // loop over hit multiplicity
            for (Int_t i = 0; i < m; i++)
            {   
                // number of hits of current multiplicity
                Int_t nhits = fLadder->GetNhitsM(i);

                // hit array of current multiplicity
                Int_t* hits = fLadder->GetHitsM(i);

                // time array of current multiplicity
                Double_t* time = fLadder->GetTimeORM(i);

                // loop over hits of current multiplicity
                for (Int_t j = 0; j < nhits; j++)
                {
                    // skip bad tagger channels
                    if (IsBadTaggerChannel(hits[j])) continue;

                    // set hit element, time and energy
                    fTaggerPhotonHits[fTaggerPhotonNhits] = hits[j];
                    fTaggerPhotonEnergy[fTaggerPhotonNhits] = eBeamEnergy - fpdEnergy[hits[j]];
                    fTaggerPhotonTime[fTaggerPhotonNhits] = time[j];
                    fTaggerPhotonNhits++;
                }
            } // for: hit multiplicity loop
        } // if: data handling
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Read-in MC information                                                 //
    ////////////////////////////////////////////////////////////////////////////
    
    if (fIsMC)
    {
        // set MC branches in first event (does not work in PostInit())
        if (fEventCounter == 0)
        {
            Int_t nBr = gAR->GetNbranch();
            for (Int_t i = 0; i < nBr; i++)
            {
                Char_t* brN = gAR->GetBranchName(i);
                if (!strcmp(brN, "dircos")) fMCBranchDirCos = gAR->GetBranch(i);
                else if (!strcmp(brN, "vertices")) fMCBranchVert = gAR->GetBranch(i);
            }
        }

        // get vertex
        Float_t* vert = (Float_t*) (fEvent[EI_vertex]);
    
        // set vertex
        fMCVertX = vert[0];
        fMCVertY = vert[1];
        fMCVertZ = vert[2];
    
        // fill histograms
        fH_MCVertX->Fill(fMCVertX);
        fH_MCVertY->Fill(fMCVertY);
        fH_MCVertZ->Fill(fMCVertZ);
    }

    
    ////////////////////////////////////////////////////////////////////////////
    // Collect MC particles                                                   //
    ////////////////////////////////////////////////////////////////////////////
    
    if (fIsMC) 
    {
        // get number of particles
        fNMC = *(Int_t*) (fEvent[EI_npart]);
        
        // read particle IDs and 4-momenta
        Int_t* g3_ID = (Int_t*) (fEvent[EI_idpart]);
        Float_t* etot = (Float_t*) (fEvent[EI_elab]);
        Float_t* ptot = (Float_t*) (fEvent[EI_plab]);
        Float_t dircos[fNMC][3];
        fMCBranchDirCos->SetAddress(dircos);
        fMCBranchDirCos->GetEntry(fEventCounter - fEventOffset);

        // read particle vertices
        Float_t vertices[fNMC][3];
        if (fMCBranchVert)
        {
            fMCBranchVert->SetAddress(vertices);
            fMCBranchVert->GetEntry(fEventCounter - fEventOffset);
        }

        // create particles and set their properties
        for (Int_t i = 0; i < fNMC; i++)
        {
            // set particle ID
            fPartMC[i]->SetGeant3_ID(g3_ID[i]);
        
            // set 4-momentum (convert to MeV)
            fPartMC[i]->SetP4(1000.*ptot[i]*dircos[i][0],
                              1000.*ptot[i]*dircos[i][1],
                              1000.*ptot[i]*dircos[i][2],
                              1000.*etot[i]);
        
            // set vertex
            if (fMCBranchVert) fPartMC[i]->SetVertex(vertices[i][0], vertices[i][1], vertices[i][2]);
        }

        // information output at first event
        if (fEventCounter == 0)
        {
            printf("\n\n\n  ****** MC PARTICLE COLLECTION ******\n\n");
            if (fMCBranchVert) printf("   Found %d MC particles with vertices:\n\n", fNMC);
            else printf("   Found %d MC particles:\n\n", fNMC);
            for (Int_t i = 0; i < fNMC; i++) 
                printf("   %2d. %-22s (PDG: %5d   G3: %2d)\n", i+1, fPartMC[i]->GetName(), 
                       fPartMC[i]->GetPDG_ID(), fPartMC[i]->GetGeant3_ID());
            printf("\n");
            printf("\n");
        }
        
        // update event offset when at last event of current tree
        Long64_t tevents = fMCBranchDirCos->GetEntries();
        if (fEventCounter - fEventOffset == tevents - 1) fEventOffset += tevents;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Prepare particle collection                                            //
    ////////////////////////////////////////////////////////////////////////////
    
    // detector particle arrays
    fPart = new TOA2DetParticle*[fCBNCluster + fTAPSNCluster];
    fPartCB = new TOA2DetParticle*[fCBNCluster];
    fPartTAPS = new TOA2DetParticle*[fTAPSNCluster];
    
    // neutral/charged particle arrays
    fPartNeutral = new TOA2DetParticle*[fCBNCluster + fTAPSNCluster];
    fPartCharged = new TOA2DetParticle*[fCBNCluster + fTAPSNCluster];
    
    // total number of particles
    fNTotal = 0;

    // number of neutral/charged particles
    fNNeutral = 0;
    fNCharged = 0;


    ////////////////////////////////////////////////////////////////////////////
    // Collect particles in CB                                                //
    ////////////////////////////////////////////////////////////////////////////
    
    // loop over all CB clusters
    Int_t nCB = 0;
    for (UInt_t i = 0; i < fCBNCluster; i++)
    {
        // get cluster
        HitCluster_t* cl = fCBCluster[fCBClusterHit[i]];
        Int_t clNhits    = cl->GetNhits();
        UInt_t* clHits   = cl->GetHits();
        Int_t center     = cl->GetIndex();
        
        // skip bad cluster centers
        if (IsBadCBCluster(center)) continue;
        
        // create particle
        fPartCB[nCB] = new TOA2DetParticle();

        // set particle properties
        fPartCB[nCB]->SetDetector(kCBDetector);
        fPartCB[nCB]->SetPosition(cl->GetMeanPosition());
        fPartCB[nCB]->SetEnergy(cl->GetEnergy());
        
        // check all bad particle cuts
        Bool_t bad = kFALSE;
        for (Int_t j = 0; j < fNBadParticleCuts; j++)
        {
            // check if particle is within this bad particle cut
            if (fBadParticleCuts[j]->IsInside(fPartCB[nCB]))
            {
                bad = kTRUE;
                break;
            }
        }
        // skip bad particle
        if (bad)
        {
            delete fPartCB[nCB];
            continue;
        }
        
        // set other particle properties
        fPartCB[nCB]->SetTime(cl->GetTime());
        fPartCB[nCB]->SetCentralElement(center);
        fPartCB[nCB]->SetCentralEnergy(cl->GetEnergy()*cl->GetCentralFrac());
        fPartCB[nCB]->SetClusterSize(clNhits);
        fPartCB[nCB]->SetClusterHits(clNhits, clHits);
        fPartCB[nCB]->SetClusterHitEnergies(clNhits, cl->GetEnergies());
        fPartCB[nCB]->SetClusterHitTimes(clNhits, cl->GetTimes());
        fPartCB[nCB]->SetClusterHitTimeMultiplicities(clNhits, cl->GetTimeMultiplicities());
        
        
        // calculate photon resolutions
        CalculatePhotonResolutions(fPartCB[nCB]);

        // link neutral/charged lists and fill charged particle properties
        Int_t pid_index;
        Double_t pid_energy = CheckClusterPID(fPartCB[nCB], &pid_index);
        if (pid_energy > 0)
        {
            // proton energy loss correction
            if (fProtonECorrCB)
            {
                Double_t e = fPartCB[nCB]->GetEnergy();
                Double_t corr = fProtonECorrCB[fPartCB[nCB]->GetCentralElement()]->Eval(e);
                fPartCB[nCB]->SetEnergy(e + corr*e);
            }

            // light attenuation correction for protons
            //if (!fIsMC)
            //{
            //    Double_t a0 =  1.156669e+00;
            //    Double_t a1 = -3.878119e-02;
            //    Double_t a2 =  4.566693e-04;
            //    Double_t a3 = -2.595171e-06;
            //    Double_t a4 =  7.111759e-09;
            //    Double_t a5 = -7.534346e-12;
            //    Double_t e = fPartCB[nCB]->GetEnergy();
            //    Double_t corr = a0 + a1*e + a2*e*e + a3*e*e*e + a4*e*e*e*e + a5*e*e*e*e*e;
            //    Double_t e_corr;
            //    if (e < 250) e_corr = e + corr*e;
            //    else e_corr = e + -0.123795297*e;
            //    fPartCB[nCB]->SetEnergy(e_corr);
            //}

            // perform droop correction
            if (fPIDDroopCorr)
            {
                // get theta of cluster
                Double_t pid_theta = fPartCB[nCB]->GetTheta() * TMath::RadToDeg();

                // normalize to energy at 45 degrees
                pid_energy /= fPIDDroopCorr[pid_index]->Eval(pid_theta);
            }
            
            // set PID index
            fPartCB[nCB]->SetPIDIndex(pid_index);

            // set PID energy
            fPartCB[nCB]->SetPIDEnergy(pid_energy);
            
            // set PID time
            if (fPIDTime) fPartCB[nCB]->SetPIDTime(fPIDTime[pid_index]);
            
            // link particle to charged list
            fPartCharged[fNCharged++] = fPartCB[nCB];
        }
        else 
        {
            // perform quadratic energy correction
            if (fCaLibReader)
            {
                if (fCaLibReader->GetCBquadEnergy())
                {
                    Double_t energy = fPartCB[nCB]->GetEnergy();
                    fPartCB[nCB]->SetEnergy(fCaLibReader->GetCBQuadEnergyPar0(center)*energy + 
                                            fCaLibReader->GetCBQuadEnergyPar1(center)*energy*energy);
                }
            }

            // link particle to neutral list
            fPartNeutral[fNNeutral++] = fPartCB[nCB];
        }
        
        // link particle to total list
        fPart[fNTotal++] = fPartCB[nCB];

        // increment number of accepted particles
        nCB++;
    } 

    // set number of accepted particles
    fCBNCluster = nCB;
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Collect tracks in the MWPC                                             //
    ////////////////////////////////////////////////////////////////////////////
    
    if (fMWPC)
    {
        fMWPC->ResetWiresAndStripsUsage();
        fMWPCNtracks = fMWPC->GetNtracks();
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // Collect particles in TAPS                                              //
    ////////////////////////////////////////////////////////////////////////////
    
    // loop over all TAPS clusters
    Int_t nTAPS = 0;
    for (UInt_t i = 0; i < fTAPSNCluster; i++)
    {
        // get cluster
        HitCluster_t* cl = fTAPSCluster[fTAPSClusterHit[i]];
        Int_t clNhits    = cl->GetNhits();
        UInt_t* clHits   = cl->GetHits();
        Int_t center     = cl->GetIndex();
        
        // skip bad cluster centers
        if (IsBadTAPSCluster(center)) continue;
        
        // create particle
        fPartTAPS[nTAPS] = new TOA2DetParticle();

        // set particle properties
        fPartTAPS[nTAPS]->SetDetector(kTAPSDetector);
        fPartTAPS[nTAPS]->SetPosition(cl->GetMeanPosition());
        fPartTAPS[nTAPS]->SetEnergy(cl->GetEnergy());
        
        // check all bad particle cuts
        Bool_t bad = kFALSE;
        for (Int_t j = 0; j < fNBadParticleCuts; j++)
        {
            // check if particle is within this bad particle cut
            if (fBadParticleCuts[j]->IsInside(fPartTAPS[nTAPS]))
            {
                bad = kTRUE;
                break;
            }
        }
        // skip bad particle
        if (bad)
        {
            delete fPartTAPS[nTAPS];
            continue;
        }
  
        // set other particle properties
        fPartTAPS[nTAPS]->SetTime(cl->GetTime());
        fPartTAPS[nTAPS]->SetCentralElement(center);
        fPartTAPS[nTAPS]->SetCentralEnergy(cl->GetEnergy()*cl->GetCentralFrac());
        fPartTAPS[nTAPS]->SetClusterSize(clNhits);
        fPartTAPS[nTAPS]->SetClusterHits(clNhits, clHits);
        fPartTAPS[nTAPS]->SetClusterHitEnergies(clNhits, cl->GetEnergies());
        fPartTAPS[nTAPS]->SetClusterHitTimes(clNhits, cl->GetTimes());
        fPartTAPS[nTAPS]->SetClusterHitTimeMultiplicities(clNhits, cl->GetTimeMultiplicities());
        fPartTAPS[nTAPS]->SetCentralSGEnergy(fBaF2PWO->GetSGEnergy(cl->GetIndex()));
        
        // calculate photon resolutions
        CalculatePhotonResolutions(fPartTAPS[nTAPS]);
        
        // link neutral/charged lists and fill charged particle properties
        Int_t veto_index;
        Double_t veto_energy = CheckClusterVeto(fPartTAPS[nTAPS], &veto_index);
        if (veto_energy > 0)
        {
            // proton energy loss correction
            if (fProtonECorrTAPS)
            {
                Double_t e = fPartTAPS[nTAPS]->GetEnergy();
                Double_t corr = fProtonECorrTAPS[fPartTAPS[nTAPS]->GetCentralElement()]->Eval(e);
                fPartTAPS[nTAPS]->SetEnergy(e + corr*e);
            }

            // set Veto index
            fPartTAPS[nTAPS]->SetVetoIndex(veto_index);

            // set Veto energy
            fPartTAPS[nTAPS]->SetVetoEnergy(veto_energy);
            
            // set Veto time
            if (fVetoTime) fPartTAPS[nTAPS]->SetVetoTime(fVetoTime[veto_index]);

            // link particle to charged list
            fPartCharged[fNCharged++] = fPartTAPS[nTAPS];
        }
        else 
        {
            // perform quadratic energy correction
            if (fCaLibReader)
            {
                if (fCaLibReader->GetTAPSquadEnergy())
                {
                    Double_t energy = fPartTAPS[nTAPS]->GetEnergy();
                    fPartTAPS[nTAPS]->SetEnergy(fCaLibReader->GetTAPSQuadEnergyPar0(center)*energy + 
                                                fCaLibReader->GetTAPSQuadEnergyPar1(center)*energy*energy);
                }
            }

            // link particle to neutral list
            fPartNeutral[fNNeutral++] = fPartTAPS[nTAPS];
        }

        // link particle to total list
        fPart[fNTotal++] = fPartTAPS[nTAPS];

        // increment number of accepted particles
        nTAPS++;
    } 
    
    // set number of accepted particles
    fTAPSNCluster = nTAPS;
    

    ////////////////////////////////////////////////////////////////////////////
    // Update corrected sum scaler histogram                                  //
    ////////////////////////////////////////////////////////////////////////////
    
    if (!fIsMC && gAR->IsScalerRead())
    {
        // user information
        //printf("Scaler read @ event %lld\n", fEventCounter);

        // update corrected clock scalers
        UpdateCorrectedScaler(0);
        UpdateCorrectedScaler(1);
        UpdateCorrectedScaler(144);
        UpdateCorrectedScaler(145);
    }
 

    ////////////////////////////////////////////////////////////////////////////
    // Fill event information                                                 //
    ////////////////////////////////////////////////////////////////////////////
    
    // count event
    fH_EventInfo->Fill(0);

    if (!fIsMC)
    {
        // fill beam helicity
        if (fTrigger->IsBeamHel(kPos)) fH_EventInfo->Fill(1);
        else if (fTrigger->IsBeamHel(kNeg)) fH_EventInfo->Fill(2);
        else if (fTrigger->IsBeamHel(kUndef)) fH_EventInfo->Fill(3);
        
        // fill Level 1 trigger
        if (fTrigger->IsTriggerL1(kCB))          fH_EventInfo->Fill(4);
        if (fTrigger->IsTriggerL1(kTAPS_OR))     fH_EventInfo->Fill(5);
        if (fTrigger->IsTriggerL1(kTAPS_Pulser)) fH_EventInfo->Fill(6);
        if (fTrigger->IsTriggerL1(kTAPS_M2))     fH_EventInfo->Fill(7);
        if (fTrigger->IsTriggerL1(kPulser))      fH_EventInfo->Fill(8);
        
        // fill Level 2 trigger
        if (fTrigger->IsTriggerL2(kM1)) fH_EventInfo->Fill(9);
        if (fTrigger->IsTriggerL2(kM2)) fH_EventInfo->Fill(10);
        if (fTrigger->IsTriggerL2(kM3)) fH_EventInfo->Fill(11);
        if (fTrigger->IsTriggerL2(kM4)) fH_EventInfo->Fill(12);

        // fill number of scaler reads
        if (!fIsMC && gAR->IsScalerRead()) fH_EventInfo->Fill(13);

        // fill number of events with hardware errors
        if (!fIsMC && gAR->GetHardError()) fH_EventInfo->Fill(14);
        
        // fill types of hardware errors
        for (Int_t i = 0; i < gAR->GetHardError(); i++)
        {
            fH_ErrorInfo->Fill(gAR->GetHardwareError()[i].fModIndex, 
                               gAR->GetHardwareError()[i].fErrCode);
        }
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Call virtual reconstruction method of child class                      //
    ////////////////////////////////////////////////////////////////////////////
    
    ReconstructPhysics();


    ////////////////////////////////////////////////////////////////////////////
    // Clean-up memory                                                        //
    ////////////////////////////////////////////////////////////////////////////
    
    // tagger hit arrays
    delete [] fTaggerPhotonHits;
    delete [] fTaggerPhotonEnergy;
    delete [] fTaggerPhotonTime;

    // detector particle arrays
    for (UInt_t i = 0; i < fCBNCluster; i++) delete fPartCB[i];
    for (UInt_t i = 0; i < fTAPSNCluster; i++) delete fPartTAPS[i];
    delete [] fPartCB;
    delete [] fPartTAPS;

    // neutral/charged particle arrays
    delete [] fPartNeutral;
    delete [] fPartCharged;

    
    ////////////////////////////////////////////////////////////////////////////
    // Trigger reset                                                          //
    ////////////////////////////////////////////////////////////////////////////
    
    fTrigger->Reset();

    
    ////////////////////////////////////////////////////////////////////////////
    // Increment event counter and check last event                           //
    ////////////////////////////////////////////////////////////////////////////
    
    fEventCounter++;

    // check if last event was processed and finish the analysis
    if (fLastEvent && fEventCounter > fLastEvent) TerminateAnalysis();
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsBadScalerRead(Int_t scr)
{
    // Check if the scaler read 'scr' is in the list of bad scaler reads.
    // Return kTRUE if yes, otherwise or if no bad scaler reads list was
    // specified kFALSE;

    // loop over bad scaler reads list (should be sorted)
    for (Int_t i = 0; i < fNBadScalerReads; i++)
    {
        // check for bad scaler read
        if (fBadScalerReads[i] < scr) continue;
        if (fBadScalerReads[i] == scr)
            return kTRUE;
        else
            return kFALSE;
    }

    return kFALSE;
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsBadTaggerChannel(Int_t ch)
{
    // Check if the tagger channel 'ch' is in the list of bad tagger channels.
    // Return kTRUE if yes, otherwise or if no bad tagger channel list was
    // specified kFALSE;

    // loop over bad tagger channel list
    for (Int_t i = 0; i < fNBadTaggerChannels; i++)
    {
        if (ch == fBadTaggerChannels[i]) return kTRUE;
    }
    return kFALSE;
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsBadCBCluster(Int_t center)
{
    // Check if the cluster center 'center' is in the list of bad CB clusters.
    // Return kTRUE if yes, otherwise or if no bad CB cluster list was
    // specified kFALSE;

    // loop over bad CB cluster list
    for (Int_t i = 0; i < fNBadCBClusters; i++)
    {
        if (center == fBadCBClusters[i]) return kTRUE;
    }
    return kFALSE;
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsBadTAPSCluster(Int_t center)
{
    // Check if the cluster center 'center' is in the list of bad TAPS clusters.
    // Return kTRUE if yes, otherwise or if no bad TAPS cluster list was
    // specified kFALSE;

    // loop over bad TAPS cluster list
    for (Int_t i = 0; i < fNBadTAPSClusters; i++)
    {
        if (center == fBadTAPSClusters[i]) return kTRUE;
    }
    return kFALSE;
}

//______________________________________________________________________________
Double_t TA2MyPhysics::GetTaggEff(Int_t ch)
{
    // Return the tagging efficiency of the channel 'ch' read from the CaLib 
    // database.
    // If no values were read from the database 1 is returned.
    // In the analysis of MC data always 1 is returned.

    // MC data
    if (fIsMC) return 1;

    // check for CaLib
    if (fCaLibReader)
    {
        if (fCaLibReader->GetTAGGeff()) return fCaLibReader->GetTAGGEffVal(ch);
        else return 1;
    }
    else return 1;
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsTrigExclTAPSRing(Int_t ring)
{
    // Check if the TAPS ring 'ring' is in the list of trigger-excluded TAPS rings.
    // Return kTRUE if yes, otherwise or if no trigger-excluded TAPS ring list was
    // specified kFALSE;

    // loop over trigger-excluded TAPS ring list
    for (Int_t i = 0; i < fNTrigTAPSExclRings; i++)
    {
        if (ring == fTrigTAPSExclRings[i]) return kTRUE;
    }
    return kFALSE;
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::IsTrigger(TOA2RecParticle& meson)
{
    // Software trigger method.
    // Check the trigger for the reconstructed meson 'meson'.

    // init variables
    Double_t cbSum = 0.;
    Int_t multCB = 0;
    Int_t multTAPS_LED1 = 0;
    Int_t multTAPS_LED2 = 0;

    // create and init the CB segment pattern
    Bool_t segPat[45];
    for (Int_t i = 0; i < 45; i++) segPat[i] = kFALSE;
    
    // create and init the TAPS block trigger pattern
    Bool_t tapsLED1Pat[6] = { kFALSE, kFALSE, kFALSE, kFALSE, kFALSE, kFALSE };
    Bool_t tapsLED2Pat[6] = { kFALSE, kFALSE, kFALSE, kFALSE, kFALSE, kFALSE };
    
    // check if LED values were read using CaLib
    Bool_t useCaLibLEDCB = kFALSE;
    Bool_t useCaLibLEDTAPS = kFALSE;
    if (fCaLibReader)
    {
        if (fCaLibReader->GetCBled()) useCaLibLEDCB = kTRUE;
        if (fCaLibReader->GetTAPSled()) useCaLibLEDTAPS = kTRUE;
    }
    
    // get number of decay particles
    Int_t nHits = meson.GetNDetectedProducts();
    
    // get particles
    TOA2DetParticle** part = meson.GetDetectedProducts();

    // loop over all particles
    for (Int_t i = 0; i < nHits; i++)
    {
        // get cluster size and hits
        Int_t clNhits    = part[i]->GetClusterSize();
        UInt_t* clHits   = part[i]->GetClusterHits();
        Double_t* clHitE = part[i]->GetClusterHitEnergies();
        
        // particles in CB
        if (part[i]->GetDetector() == kCBDetector)
        {
            // loop over all cluster hits
            for (Int_t j = 0; j < clNhits; j++)
            {
                // calculate the segment index
                Int_t seg = (Int_t)(clHits[j] / 16);
                
                // set threshold either via CaLib or via config file
                Double_t thr;
                if (useCaLibLEDCB) thr = fCaLibReader->GetCBLEDThr(clHits[j]) * fTrigCBSegThr;
                else thr = fTrigCBSegThr;

                // check NaI segment high discriminator threshold
                if (clHitE[j] > thr) segPat[seg] = kTRUE;

                // add de-calibrated energy to total sum
                Double_t eDecal;
                if (fIsMC) 
                {
                    // check if relative gains were read in
                    if (fTrigCBRelGains) eDecal = clHitE[j] / (fNaI->GetElement(clHits[j])->GetA1() / fTrigCBMeanGain) / fTrigCBRelGains[clHits[j]];
                    else eDecal = clHitE[j] / (fNaI->GetElement(clHits[j])->GetA1() / fTrigCBMeanGain);
                }
                else eDecal = clHitE[j] / (fNaI->GetElement(clHits[j])->GetA1() / fTrigCBMeanGain);
                cbSum += eDecal;
            }
        }
        
        // particles in TAPS
        if (part[i]->GetDetector() == kTAPSDetector)
        {
            // loop over all cluster hits
            for (Int_t j = 0; j < clNhits; j++)
            {
                // skip elements in trigger-excluded TAPS rings
                if (IsTrigExclTAPSRing(TOA2Detector::GetTAPSRing(clHits[j], fTAPSType))) continue;

                // get the block index (0..5)
                Int_t block = TOA2Detector::GetTAPSBlock(clHits[j], fTAPSType) - 1;
                
                // set thresholds either via CaLib or via config file
                Double_t thr_led1, thr_led2;
                if (useCaLibLEDTAPS) 
                {
                    thr_led1 = fCaLibReader->GetTAPSLED1Thr(clHits[j]) * fTrigTAPSLED1Thr;
                    thr_led2 = fCaLibReader->GetTAPSLED2Thr(clHits[j]) * fTrigTAPSLED2Thr;
                }
                else 
                {
                    thr_led1 = fTrigTAPSLED1Thr;
                    thr_led2 = fTrigTAPSLED2Thr;
                }

                // check LED1/LED2 trigger condition
                if (clHitE[j] > thr_led1) tapsLED1Pat[block] = kTRUE;
                if (clHitE[j] > thr_led2) tapsLED2Pat[block] = kTRUE;
            }
        }
    }

    //
    // count multiplicities
    //

    // loop over CB segments
    for (Int_t i = 0; i < 45; i++)
    {
        // count segments that fulfilled the segment high discriminator threshold
        if (segPat[i]) multCB++;
    }

    // loop over TAPS blocks
    for (Int_t i = 0; i < 6; i++)
    {
        // add blocks that fired the LED1 to the total multiplicity
        if (tapsLED1Pat[i]) multTAPS_LED1++;
        
        // add blocks that fired the LED2 to the TAPS alone multiplicity
        if (tapsLED2Pat[i]) multTAPS_LED2++;
    }
    
    //
    // CB sum for MC analysis when CB sum CDF is provided
    //

    Bool_t cbCDF;
    if (fIsMC && fTrigCBSumCDF)
    {
        // get random number in ]0,max] where max is the maximum value of the CDF
        Double_t rnd = gRandom->Uniform(0, fTrigCBSumCDF->GetParameter(0));

        // compare random number to function value
        // at the energy sum value
        if (rnd < fTrigCBSumCDF->Eval(cbSum)) cbCDF = kTRUE;
        else cbCDF = kFALSE;
    }
    else cbCDF = kTRUE;

    //
    // store software trigger
    //
    
    fTrigger->SetCBEnergySum(cbSum);
    fTrigger->SetMultCB(multCB);
    fTrigger->SetMultTAPSLED1(multTAPS_LED1);
    fTrigger->SetMultTAPSLED2(multTAPS_LED2);

    //
    // make trigger decision
    //

    // CB sum + total multiplicity
    Int_t multTotal;
    if (fTrigIsTAPSTotalMult) multTotal = multCB + multTAPS_LED1;
    else multTotal = multCB;
    if (cbSum >= fTrigCBSum && cbCDF && multTotal >= fTrigTotalMult) return kTRUE;

    // TAPS alone
    if (multTAPS_LED2 >= fTrigTAPSMult) return kTRUE;

    // no trigger here
    return kFALSE;
}

//______________________________________________________________________________
void TA2MyPhysics::ClearCBTimeWalk()
{
    // Set the time walk parameters of all CB elements to zero.
    
    Double_t zeroWalk[4] = { 0, 0, 0, 0 };

    // check if NaI cluster detector is here
    if (fNaI)
    {
        Int_t nelem = fNaI->GetNelement();
        for (Int_t i = 0; i < nelem; i++) fNaI->GetElement(i)->GetTimeWalk()->SetWalk(zeroWalk);
    }
    else Error("ClearCBTimeWalk", "Could not (yet) find the NaI cluster detector"
               " to clear the time walk values!");
}

//______________________________________________________________________________
Double_t TA2MyPhysics::GetBGSubtractionWeight(TOA2RecParticle& meson, 
                                              Double_t taggerTime,
                                              Double_t limitPromptLowCB, Double_t limitPromptHighCB,
                                              Double_t limitBG1LowCB, Double_t limitBG1HighCB,
                                              Double_t limitBG2LowCB, Double_t limitBG2HighCB,
                                              Double_t limitPromptLowTAPS, Double_t limitPromptHighTAPS,
                                              Double_t limitBG1LowTAPS, Double_t limitBG1HighTAPS,
                                              Double_t limitBG2LowTAPS, Double_t limitBG2HighTAPS,
                                              Bool_t* outIsOutside)
{
    // Return the weight for the tagger background subtraction using
    // - the detected meson 'meson'
    // - the tagger time 'taggerTime'
    // - the limits for the prompt region 'limitPromptLowCB/TAPS' and 'limitPromptHighCB/TAPS'
    // - the limits for the background region 1 'limitBG1LowCB/TAPS' and 'limitBG1HighCB/TAPS'
    // - the limits for the background region 2 'limitBG2LowCB/TAPS' and 'limitBG2HighCB/TAPS'
    // If 'outIsOutside' is non-zero it will be used to return kTRUE if the hit is a useless
    // background hit.
    
    // MC events are always considered prompt
    if (fIsMC)
    {
        if (outIsOutside) *outIsOutside = kFALSE;
        return 1.;
    }

    // calculate the coincidence time taking into account the detector time
    // of the meson to remove the trigger time
    
    // meson time from TAPS
    if (meson.GetNDetectorHits(kTAPSDetector)) 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() + taggerTime;
    
        // check for the prompt region
        if (coincTime > limitPromptLowTAPS && coincTime < limitPromptHighTAPS)
        {
            if (outIsOutside) *outIsOutside = kFALSE;
            return 1.;
        }
        // check for background region
        else if ((coincTime > limitBG1LowTAPS && coincTime < limitBG1HighTAPS) ||
                 (coincTime > limitBG2LowTAPS && coincTime < limitBG2HighTAPS))
        {
            if (outIsOutside) *outIsOutside = kFALSE;
            return - (limitPromptHighTAPS - limitPromptLowTAPS) / 
                     (limitBG1HighTAPS - limitBG1LowTAPS + limitBG2HighTAPS - limitBG2LowTAPS);
        }
        // useless background here
        else
        {
            if (outIsOutside) *outIsOutside = kTRUE;
            return 0.;
        }
    }
    // meson time from CB
    else 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() - taggerTime;
        
        // check for the prompt region
        if (coincTime > limitPromptLowCB && coincTime < limitPromptHighCB)
        {
            if (outIsOutside) *outIsOutside = kFALSE;
            return 1.;
        }
        // check for background region
        else if ((coincTime > limitBG1LowCB && coincTime < limitBG1HighCB) ||
                 (coincTime > limitBG2LowCB && coincTime < limitBG2HighCB))
        {
            if (outIsOutside) *outIsOutside = kFALSE;
            return - (limitPromptHighCB - limitPromptLowCB) / 
                     (limitBG1HighCB - limitBG1LowCB + limitBG2HighCB - limitBG2LowCB);
        }
        // useless background here
        else
        {
            if (outIsOutside) *outIsOutside = kTRUE;
            return 0.;
        }
     }
}

//______________________________________________________________________________
Double_t TA2MyPhysics::GetBGSubtractionWeight(TOA2RecParticle& meson, 
                                              Int_t taggerChannel,
                                              Double_t taggerTime,
                                              Double_t limitBG1LowCB, Double_t limitBG1HighCB,
                                              Double_t limitBG2LowCB, Double_t limitBG2HighCB,
                                              Double_t limitBG1LowTAPS, Double_t limitBG1HighTAPS,
                                              Double_t limitBG2LowTAPS, Double_t limitBG2HighTAPS,
                                              Bool_t* outIsOutside)
{
    // Return the weight for the tagger background subtraction using
    // - the detected meson 'meson'
    // - the tagger channel 'taggerChannel'
    // - the tagger time 'taggerTime'
    // - the limits for the background region 1 'limitBG1LowCB/TAPS' and 'limitBG1HighCB/TAPS'
    // - the limits for the background region 2 'limitBG2LowCB/TAPS' and 'limitBG2HighCB/TAPS'
    // The limits for the prompt window will be taken from the tagger prompt graphs.
    // If 'outIsOutside' is non-zero it will be used to return kTRUE if the hit is a useless
    // background hit.
    
    // MC events are always considered prompt
    if (fIsMC)
    {
        if (outIsOutside) *outIsOutside = kFALSE;
        return 1.;
    }

    // get prompt limits
    Double_t limitPromptLowCB = fPrompt_Min_CB->Eval(taggerChannel);
    Double_t limitPromptHighCB = fPrompt_Max_CB->Eval(taggerChannel);
    Double_t limitPromptLowTAPS = fPrompt_Min_TAPS->Eval(taggerChannel);
    Double_t limitPromptHighTAPS = fPrompt_Max_TAPS->Eval(taggerChannel);
    
    // call normal method
    return GetBGSubtractionWeight(meson, taggerTime,
                                  limitPromptLowCB, limitPromptHighCB,
                                  limitBG1LowCB, limitBG1HighCB,
                                  limitBG2LowCB, limitBG2HighCB,
                                  limitPromptLowTAPS, limitPromptHighTAPS,
                                  limitBG1LowTAPS, limitBG1HighTAPS,
                                  limitBG2LowTAPS, limitBG2HighTAPS,
                                  outIsOutside);
}
 
//______________________________________________________________________________
Double_t TA2MyPhysics::GetRandomTaggerTime(TOA2RecParticle& meson, Int_t channel, 
                                           Double_t subtr_weight, Double_t taggerTime)
{
    // Return a randomized tagger time for the tagger channel 'channel' within
    // the prompt window of the meson-Tagger time if the weight 'subtr_weight' is
    // negative (random background).
    // If 'subtr_weight' is positive the original tagger time 'taggerTime' will
    // be returned.

    // check weight
    if (subtr_weight > 0) 
        return taggerTime;
    else
    {
        // meson time from TAPS
        if (meson.GetNDetectorHits(kTAPSDetector)) 
        {
            Double_t new_coinc = gRandom->Uniform(fPrompt_Min_TAPS->Eval(channel), fPrompt_Max_TAPS->Eval(channel));
            return new_coinc - meson.GetTime();
        }
        // meson time from CB
        else  
        {
            Double_t new_coinc = gRandom->Uniform(fPrompt_Min_CB->Eval(channel), fPrompt_Max_CB->Eval(channel));
            return meson.GetTime() - new_coinc;
        }
    }
}

//______________________________________________________________________________
void TA2MyPhysics::FillTaggerCoincidence(TOA2RecParticle& meson, Double_t taggerTime, TH1* hCB, TH1* hTAPS)
{
    // Fill the tagger coincidence time of the meson 'meson' using the tagger time 'taggerTime'
    // depending of the detector the meson time was calculated from into the histograms
    // 'hCB' or 'hTAPS', respectively.

    // meson time from TAPS
    if (meson.GetNDetectorHits(kTAPSDetector)) 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() + taggerTime;
        if (hTAPS) hTAPS->Fill(coincTime);
    }
    // meson time from CB
    else 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() - taggerTime;
        if (hCB) hCB->Fill(coincTime);
    }
}

//______________________________________________________________________________
void TA2MyPhysics::FillTaggerCoincidence(TOA2RecParticle& meson, Double_t taggerTime, 
                                         Int_t taggerElement, TH2* hCB, TH2* hTAPS)
{
    // Fill the tagger coincidence time of the meson 'meson' using the tagger time 'taggerTime'
    // against the tagged element 'taggerElement' depending of the detector the meson time was 
    // calculated from into the histogram 'hCB' or 'hTAPS', respectively.

    // meson time from TAPS
    if (meson.GetNDetectorHits(kTAPSDetector)) 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() + taggerTime;
        if (hTAPS) hTAPS->Fill(coincTime, taggerElement);
    }
    // meson time from CB
    else 
    {
        // calculate coincidence time
        Double_t coincTime = meson.GetTime() - taggerTime;
        if (hCB) hCB->Fill(coincTime, taggerElement);
    }
}
 
//______________________________________________________________________________
void TA2MyPhysics::UpdateCorrectedScaler(Int_t sc)
{
    // Update the entry of the scaler 'sc' in the corrected scaler histogram.
    // IMPORTANT: Call this function only during a scaler-read event, i.e.
    // after checking gAR->IsScalerRead().

    // check scaler number
    if (sc >= gAR->GetMaxScaler())
    {
        Error("UpdateCorrectedScaler", "Scaler %d is larger than max. scaler value!", sc);
        return;
    }

    // check previous sum scaler entry to avoid double counting
    // do not do this in offline ROOT file input mode
    if ((gAR->IsOnline() && fScalerSum[sc] != fOldScalerSum[sc]) ||
        !gAR->IsOnline())
    {
        // free running, overflow vulnerable 24-bit scaler (total)
        if (sc == 0)
        {
            UInt_t scaler_0;
            if (fScaler[0] < fScaler[1]) scaler_0 = fScaler[0] + 16777216;
            else scaler_0 = fScaler[0];
            fH_Corrected_Scalers->SetBinContent(1, scaler_0);
            fH_Corrected_SumScalers->AddBinContent(1, scaler_0);
        }
        // free running, overflow vulnerable 24-bit scaler (tagger)
        else if (sc == 144)
        {
            UInt_t scaler_144;
            if (fScaler[144] < fScaler[145]) scaler_144 = fScaler[144] + 16777216;
            else scaler_144 = fScaler[144];
            fH_Corrected_Scalers->SetBinContent(145, scaler_144);
            fH_Corrected_SumScalers->AddBinContent(145, scaler_144);
        }
        else 
        {
            fH_Corrected_Scalers->SetBinContent(sc+1, fScaler[sc]);
            fH_Corrected_SumScalers->AddBinContent(sc+1, fScaler[sc]);
        }

        // update old sum scaler value
        fOldScalerSum[sc] = fScalerSum[sc];    
    }
}

//______________________________________________________________________________
Double_t TA2MyPhysics::CheckClusterVeto(TOA2DetParticle* inCluster, Int_t* outVetoIndex) const
{
    // Return the maximum energy deposited in one of the veto detectors of the
    // TAPS cluster 'inCluster'.
    // Returns 0 if no cluster veto fired.
    //
    // If 'outVetoIndex' is provided write the index of the corresponding veto
    // element to that variable (-1 if no cluster veto fired).
    
    // quit if there is no veto hit
    if (!fVetoNhits)
    {
        // set veto index
        if (outVetoIndex) *outVetoIndex = -1;

        // exit
        return 0.;
    }
    
    Int_t center        = inCluster->GetCentralElement();
    UInt_t nNeighbours  = fTAPSCluster[center]->GetNNeighbour();
    UInt_t* neighbours  = fTAPSCluster[center]->GetNeighbour();
        
    Double_t maxVetoEnergy = 0;
    if (outVetoIndex) *outVetoIndex = -1;

    // loop over all veto hits
    for (UInt_t i = 0; i < fVetoNhits; i++)
    {
        // check central element
        if (fVetoHits[i] == TOA2Detector::GetVetoInFrontOfElement(center, fTAPSType))
        {
            if (fVetoEnergy[fVetoHits[i]] > maxVetoEnergy) 
            {
                maxVetoEnergy = fVetoEnergy[fVetoHits[i]]
                                * TMath::Abs(TMath::Cos(inCluster->GetTheta()));
                if (outVetoIndex) *outVetoIndex = fVetoHits[i];
            }
        }
        
        // loop over all neighbour elements
        for (UInt_t j = 0; j < nNeighbours; j++)
        {
            if (fVetoHits[i] == TOA2Detector::GetVetoInFrontOfElement((Int_t)neighbours[j], fTAPSType))
            {
                if (fVetoEnergy[fVetoHits[i]] > maxVetoEnergy) 
                {
                    maxVetoEnergy = fVetoEnergy[fVetoHits[i]]
                                    * TMath::Abs(TMath::Cos(inCluster->GetTheta()));
                    if (outVetoIndex) *outVetoIndex = fVetoHits[i];
                }
            }
        }
 
        // loop over all cluster members
        for (Int_t j = 0; j < inCluster->GetClusterSize(); j++)
        {
            if (fVetoHits[i] == TOA2Detector::GetVetoInFrontOfElement(inCluster->GetClusterHits()[j], fTAPSType))
            {
                if (fVetoEnergy[fVetoHits[i]] > maxVetoEnergy) 
                {
                    maxVetoEnergy = fVetoEnergy[fVetoHits[i]]
                                    * TMath::Abs(TMath::Cos(inCluster->GetTheta()));
                    if (outVetoIndex) *outVetoIndex = fVetoHits[i];
                }
            }
        }
    }

    return maxVetoEnergy;
}

//______________________________________________________________________________
Double_t TA2MyPhysics::CheckClusterPID(TOA2DetParticle* inCluster, Int_t* outPIDIndex) const
{
    // Return the theta-weighted energy deposited in the coincident PID element
    // of the CB cluster 'inCluster'.
    // Returns 0 if no coincident PID element fired.
    //
    // If 'outPIDIndex' is provided write the index of the PID element with the 
    // minimum difference in the phi angle to that variable (-1 if no cluster PID fired).
    
    // quit if there is no PID hit
    if (!fPIDNhits)
    {
        // set PID index
        if (outPIDIndex) *outPIDIndex = -1;

        // exit
        return 0.;
    }

    // init variables
    Double_t minPhiDiff = 180;
    Int_t pidIndex = -1;

    // loop over all PID hits and search best coincident hit
    for (UInt_t i = 0; i < fPIDNhits; i++)
    {
        // calculate CB-PID phi difference
        Double_t phi_diff = TVector2::Phi_mpi_pi(fPIDHitPos[fPIDHits[i]]->Z()*TMath::DegToRad() - inCluster->GetPhi());
        phi_diff = TMath::Abs(phi_diff*TMath::RadToDeg());
  
        // update minimal phi difference
        if (phi_diff < minPhiDiff)
        {
            pidIndex = fPIDHits[i];
            minPhiDiff = phi_diff;
        }
    }
    
    // fill CB-PID coincidence histograms
    //fH_CB_PID_Coinc_Hits[pidIndex]->Fill(minPhiDiff);
    
    // check phi difference limit
    Double_t pidEnergy = 0;
    if (minPhiDiff <= fCBPIDPhiLimit)
    {   
        // check if energy information is available
        if (fPIDEnergy)
        {
            // calculate theta-weighted energy
            pidEnergy = fPIDEnergy[pidIndex]
                        * TMath::Abs(TMath::Sin(inCluster->GetTheta()));
        
        }
        else pidEnergy = 1.;
    }
   
    // fill CB-PID dE vs. E histograms
    //if (pidEnergy > 0.) fH_CB_PID_dE_E[pidIndex]->Fill(inCluster->GetEnergy(), pidEnergy);

    // set PID index
    if (outPIDIndex) *outPIDIndex = pidIndex;
    
    // return PID energy
    return pidEnergy;
}

//______________________________________________________________________________
void TA2MyPhysics::FillPSA(TH2* h, TOA2DetParticle* p, Double_t w)
{
    // Fill the PSA information of the particle 'p' into the histogram 'h'
    // using the weight 'w'.
    
    Double_t psaR;
    Double_t psaA;
    
    // get PSA information
    if (p->CalculatePSA(&psaR, &psaA)) h->Fill(psaA, psaR, w);
}

//______________________________________________________________________________
void TA2MyPhysics::FilldE_E(TH2* hCB, TH2* hTAPS, TOA2DetParticle* p, Double_t w)
{
    // Fill the dE/E information of the particle 'p' into the histogram 'hCB'
    // or 'hTAPS' using the weight 'w'. 
    
    // check the detector
    if (p->GetDetector() == kTAPSDetector) hTAPS->Fill(p->GetEnergy(), p->GetVetoEnergy(), w);
    else if (p->GetDetector() == kCBDetector) hCB->Fill(p->GetEnergy(), p->GetPIDEnergy(), w);
}

//______________________________________________________________________________
void TA2MyPhysics::FillTOF(TH2* hCB, TH2* hTAPS, TOA2DetParticle* p, Double_t t, Bool_t isMC, Double_t w)
{
    // Fill the TOF information of the particle 'p' into the histogram 'hCB'
    // or 'hTAPS' using the weight 'w'. The tagger time 't' and 'isMC' is used 
    // to calculate the TOF.
    
    // check the detector
    if (p->GetDetector() == kTAPSDetector) hTAPS->Fill(p->CalculateTOFTagger(t, isMC), p->GetEnergy(), w);
    else if (p->GetDetector() == kCBDetector) hCB->Fill(p->CalculateTOFTagger(t, isMC), p->GetEnergy(), w);
}

//______________________________________________________________________________
Bool_t TA2MyPhysics::CheckPSA(TOA2DetParticle* p, TCutG* c)
{   
    // Check if the PSA information of the particle 'p' is inside the graphical
    // cut 'c'. Return kTRUE if so, otherwise return kFALSE.
    // If the particle 'p' was not detected in TAPS kFALSE is returned.
    
    Double_t psaR;
    Double_t psaA;
    
    // get PSA information
    if (p->CalculatePSA(&psaR, &psaA))
    {
        // check cut
        return c->IsInside(psaA, psaR);
    }
    else return kFALSE;
}

//______________________________________________________________________________
Int_t TA2MyPhysics::GetForeignVetoHits(TOA2DetParticle* p)
{
    // Return the number of Veto hits that are not in front of the central element
    // or its neighbours of the particle 'p'.

    // check if the particle was detected in TAPS
    if (p->GetDetector() != kTAPSDetector) 
    {
        Error("IsForeignVetoHit", "The particle was not detected in TAPS!");
        return fVetoNhits;
    }

    // get the central element
    Int_t center = p->GetCentralElement();

    // get the corresponding detector cluster element from the BaF2 class
    HitCluster_t* c = fBaF2PWO->GetCluster(center);
    
    // get the central element's neighbours
    UInt_t nNeighbours = c->GetNNeighbour();
    UInt_t* neighbours = c->GetNeighbour();
 
    // the number of foreign veto hits
    Int_t nForeign = 0;

    // loop over all veto hits
    for (UInt_t i = 0; i < fVetoNhits; i++)
    {   
        Bool_t isForeign = kTRUE;

        // check central element
        if (fVetoHits[i] == TOA2Detector::GetVetoInFrontOfElement(center, fTAPSType)) continue;

        // loop over all neighbour elements
        for (UInt_t j = 0; j < nNeighbours; j++)
        {
            if (fVetoHits[i] == TOA2Detector::GetVetoInFrontOfElement((Int_t)neighbours[j], fTAPSType))
            {
                isForeign = kFALSE;
                break;
            }
        }

        // count foreign hit
        if (isForeign) nForeign++;
    }

    return nForeign;
}

//______________________________________________________________________________ 
Bool_t TA2MyPhysics::IsPSAPhoton(TOA2DetParticle* p)
{
    // Return kTRUE if the particle 'p' is considered as photon according to PSA.
    // Return kFALSE otherwise.
    // Return always kTRUE in MC analysis.
    // Return kTRUE if PSA is not configured.

    // check MC
    if (fIsMC) return kTRUE;

    // PSA configured?
    if (!fPSA) return kTRUE;
    
    // perform PSA
    return fPSA->IsPSAPhoton(p);
}

//______________________________________________________________________________ 
Bool_t TA2MyPhysics::IsPSANucleon(TOA2DetParticle* p)
{
    // Return kTRUE if the particle 'p' is considered as nucleon according to PSA.
    // Return kFALSE otherwise.
    // Return always kTRUE in MC analysis.
    // Return kTRUE if PSA is not configured.

    // check MC
    if (fIsMC) return kTRUE;
    
    // PSA configured?
    if (!fPSA) return kTRUE;
    
    // perform PSA
    return fPSA->IsPSANucleon(p);
}

//______________________________________________________________________________ 
Bool_t TA2MyPhysics::IsPIDProton(TOA2DetParticle* p)
{
    // Return kTRUE if the particle 'p' is considered as proton according to the
    // PID dE-E cut.
    // Return kTRUE if the particle was detected in TAPS.
    // Return kTRUE if no PID dE-E cuts were loaded.
    
    // check for PID dE-E cuts
    if (!fPIDdEECuts) return kTRUE;

    // TAPS particles
    if (p->GetDetector() != kCBDetector) return kTRUE;

    // get PID element index
    Int_t elem = p->GetPIDIndex();

    // check if specific PID dE-E cut was loaded
    if (!fPIDdEECuts[elem])
    {
        Error("IsPIDProton", "No PID dE-E cuts for element %d found!", elem);
        return kTRUE;
    }

    // check for proton
    if (p->GetPIDEnergy() > fPIDdEECuts[elem]->Eval(p->GetEnergy())) return kTRUE;
    else return kFALSE;
}

//______________________________________________________________________________ 
Bool_t TA2MyPhysics::IsPIDPion(TOA2DetParticle* p)
{
    // Return kTRUE if the particle 'p' is considered as pion according to the
    // PID dE-E cut.
    // Return kFALSE if the particle was detected in TAPS.
    // Return kTRUE if no PID dE-E cuts were loaded.
    
    // check for PID dE-E cuts
    if (!fPIDdEECuts) return kTRUE;

    // TAPS particles
    if (p->GetDetector() != kCBDetector) return kFALSE;

    // get PID element index
    Int_t elem = p->GetPIDIndex();

    // check if specific PID dE-E cut was loaded
    if (!fPIDdEECuts[elem])
    {
        Error("IsPIDProton", "No PID dE-E cuts for element %d found!", elem);
        return kTRUE;
    }

    // check for pion
    if (p->GetPIDEnergy() > fPIDdEECuts[elem]->Eval(p->GetEnergy())) return kFALSE;
    else return kTRUE;
}

//______________________________________________________________________________ 
void TA2MyPhysics::UndoQuadraticEnergyCorr(TOA2DetParticle* p)
{
    // Undo the quadratic energy correction that was applied to the neutral
    // particle 'p'.

    // get parameters
    Double_t a, b;
    if (p->GetDetector() == kCBDetector)
    {
        if (!fCaLibReader->GetCBquadEnergy()) return;
        a = fCaLibReader->GetCBQuadEnergyPar0(p->GetCentralElement());
        b = fCaLibReader->GetCBQuadEnergyPar1(p->GetCentralElement());
    }
    else if (p->GetDetector() == kTAPSDetector)
    {
        if (!fCaLibReader->GetTAPSquadEnergy()) return;
        a = fCaLibReader->GetTAPSQuadEnergyPar0(p->GetCentralElement());
        b = fCaLibReader->GetTAPSQuadEnergyPar1(p->GetCentralElement());
    }
    else
    {
        Error("UndoQuadraticEnergyCorr", "Particle was detected in unknown detector!");
        return;
    }
    
    // get corrected energy
    Double_t e = p->GetEnergy();
    
    // restore original energy
    Double_t e_orig = (-a + TMath::Sqrt(a*a + 4.*b*e)) / 2. / b;
    p->SetEnergy(e_orig);
}

//______________________________________________________________________________ 
void TA2MyPhysics::CalculatePhotonResolutions(TOA2DetParticle* p)
{
    // Calculate the photon resolutions for the particle 'p' assuming it is a photon
    // and save the errors into the particle object.
    // The original energy of the particle should be stored into 'p' because this
    // method is performing the quadratic energy correction for photons.

    // get detector
    if (p->GetDetector() == kCBDetector)
    {
        // check if all resolution data were loaded
        if (!fPhotonResThetaCB || !fPhotonResPhiCB || !fPhotonResEnergyCB) return;

        // perform quadratic energy correction to get photon energy
        Double_t energy = p->GetEnergy();
        if (fCaLibReader)
        {
            if (fCaLibReader->GetCBquadEnergy())
            {
                energy = fCaLibReader->GetCBQuadEnergyPar0(p->GetCentralElement())*p->GetEnergy() + 
                         fCaLibReader->GetCBQuadEnergyPar1(p->GetCentralElement())*p->GetEnergy()*p->GetEnergy();
            }
        }
        
        // save theta error
        p->SetErrorTheta(fPhotonResThetaCB->Eval(p->GetTheta()*TMath::RadToDeg()) * TMath::DegToRad());
        
        // save phi error
        p->SetErrorPhi(fPhotonResPhiCB->Eval(p->GetTheta()*TMath::RadToDeg()) * TMath::DegToRad());

        // save energy error
        p->SetErrorEnergy(fPhotonResEnergyCB->Eval(energy));
    }
    else if (p->GetDetector() == kTAPSDetector)
    {
        // check if all resolution data were loaded
        if (!fPhotonResThetaTAPS || !fPhotonResPhiTAPS || !fPhotonResEnergyTAPS) return;

        // perform quadratic energy correction to get photon energy
        Double_t energy = p->GetEnergy();
        if (fCaLibReader)
        {
            if (fCaLibReader->GetTAPSquadEnergy())
            {
                energy = fCaLibReader->GetTAPSQuadEnergyPar0(p->GetCentralElement())*p->GetEnergy() + 
                         fCaLibReader->GetTAPSQuadEnergyPar1(p->GetCentralElement())*p->GetEnergy()*p->GetEnergy();
            }
        }
        
        // save theta error
        p->SetErrorTheta(fPhotonResThetaTAPS->Eval(p->GetTheta()*TMath::RadToDeg()) * TMath::DegToRad());
        
        // save phi error
        p->SetErrorPhi(fPhotonResPhiTAPS->Eval(p->GetTheta()*TMath::RadToDeg()) * TMath::DegToRad());

        // save energy error
        p->SetErrorEnergy(fPhotonResEnergyTAPS->Eval(energy));
    }
}

//______________________________________________________________________________ 
Double_t TA2MyPhysics::CalculateMeanCBGain()
{
    // Calculate the mean CB NaI gain.
    
    Double_t mean = 0;
    Int_t nElem = 0;

    // loop over CB elements
    for (UInt_t i = 0; i < fNaI->GetNelement(); i++) 
    {
        // skip CB holes
        if (TOA2Detector::IsCBHole(i)) continue;
        
        // skip bad elements
        if (fNaI->GetElement(i)->IsIgnored()) continue;

        // sum up gains
        mean += fNaI->GetElement(i)->GetA1();
        nElem++;
    }
    
    return mean / (Double_t)nElem;
}

//______________________________________________________________________________ 
void TA2MyPhysics::OpenOutputFile()
{
    // Open the output file for e.g. direct tree writing.

    Char_t tmp[256];
    Char_t tmp2[256];
    
    // check if file is already open
    if (fOutputFile) return;

    // check if data or MC analysis to construct the filename
    if (!fIsMC) 
    {
        sprintf(tmp, "%sARHistograms_%s.root", gAR->GetTreeDir(), TOSUtils::ExtractPureFileName(gAR->GetFileName()));
    }
    else 
    {
        // Search last slash
        Int_t pos = TOSUtils::LastIndexOf(gAR->GetTreeFileList(0), '/');

        // get string until filename
        strncpy(tmp2, gAR->GetTreeFileList(0), pos);
        tmp2[pos]='\0';

        // get parent directory
        strcpy(tmp, TOSUtils::ExtractFileName(tmp2));

        // create new string
        sprintf(tmp2, "%s_%s", tmp, TOSUtils::ExtractPureFileName(gAR->GetTreeFileList(0)));
        sprintf(tmp, "%sARHistograms_%s.root", gAR->GetTreeDir(), tmp2);
    }

    // open the file
    fOutputFile = new TFile(tmp, "recreate");
   
    // user information
    Info("OpenOutputFile", "Opening output ROOT file '%s'\n", tmp);
}

//______________________________________________________________________________ 
void TA2MyPhysics::TerminateAnalysis()
{
    // Terminate the analysis (taken from AcquSys).

    gAN->EndFile();
    gAN->Finish();
    fflush(NULL);
}

ClassImp(TA2MyPhysics)
