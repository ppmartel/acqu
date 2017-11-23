// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddCalibMC.C                                                         //
//                                                                      //
// Add a MC calibration including a dummy run number and initial        //
// calibrations from AcquRoot configuration files to a CaLib database.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddCalibMC()
{
    // load CaLib
    gSystem->Load("../build/lib/libCaLib.so");
 
    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t target[]           = "LH2";
    const Int_t dummyRun            = 999001;
    const Char_t calibName[]        = "2015-MC-Init-Test";
    const Char_t calibDesc[]        = "Calibration for MC data, test to set proper starting values";
    const Char_t calibFileTagger[]  = "../acqu_user/data/AR-Analysis-Tagger-EPT.dat";
    const Char_t calibFileCB[]      = "../acqu_user/data/AR-Analysis-CB-NaI.dat";
    const Char_t calibFileTAPS[]    = "../acqu_user/data/AR-Analysis-TAPS-BaF2.dat";
    const Char_t calibFilePID[]     = "../acqu_user/data/AR-Analysis-CB-PID.dat";
    const Char_t calibFileVeto[]    = "../acqu_user/data/AR-Analysis-TAPS-Veto.dat";

    // set MC mode to pass 0s and 1s to the database for gains, offset, pedestals, etc.
    TCMySQLManager::GetManager()->SetMC(true);

    // add raw files to the database
    TCMySQLManager::GetManager()->AddRun(dummyRun, target, calibDesc);
    
    // read AcquRoot calibration of tagger
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAGG, calibFileTagger,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // read AcquRoot calibration of CB
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_CB, calibFileCB,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // init CB quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.CB.Energy.Quad", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    
    // init CB LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.LED", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    
    // read AcquRoot calibration of TAPS
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // init TAPS quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.Energy.Quad", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
 
    // init TAPS LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED1", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED2", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);

    // read AcquRoot calibration of PID
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_PID, calibFilePID,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // read AcquRoot calibration of Veto 
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_VETO, calibFileVeto,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    gSystem->Exit(0);
}

