void ApplyCalibFromAR() {
	// load CaLib
	gSystem->Load("libCaLib.so");
 
	// macro configuration: just change here for your beamtime and leave
	// the other parts of the code unchanged
	const Int_t firstRun            = 4921;
	const Int_t lastRun             = 6008;
	const Char_t calibName[]        = "2014-07_EPT_Prod_Neiser";
	const Char_t calibDesc[]        = "Calibration for July 2014 EPT Beamtime";
	const Char_t calibFileTAPS[]    = "../acqu_user/data/Detector-BaF2-PbWO4-Cosmics.dat";

	// remove the to be overwritten data sets first
	const Int_t runset = 0;
	TCMySQLManager::GetManager()->RemoveDataSet("Data.TAPS.LG.E0", calibName, runset);
	TCMySQLManager::GetManager()->RemoveDataSet("Data.TAPS.LG.E1", calibName, runset);
	TCMySQLManager::GetManager()->RemoveDataSet("Data.TAPS.T1", calibName, runset);

	
	// read AcquRoot calibration of TAPS
	TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
	                                         calibName, calibDesc,
	                                         firstRun, lastRun);
 
}
