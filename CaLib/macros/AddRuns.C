
//______________________________________________________________________________
void AddRuns()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // Adding the runfiles should be done by maintainers only!
    // add raw files to the database
    const Char_t rawfilePath[]      = "/local/raid0/A2/rundata/2014-10_EPT_Prod";
    const Char_t target[]           = "LH2";    
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);
    
    gSystem->Exit(0);
}

