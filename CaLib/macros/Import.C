// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Import.C                                                             //
//                                                                      //
// Import CaLib run data and calibrations from ROOT files.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Import()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // import CaLib data
    TCMySQLManager::GetManager()->Import("backup.root", kFALSE, kTRUE, "Target_Month_Year");
  
    gSystem->Exit(0);
}

