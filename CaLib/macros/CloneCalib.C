// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CloneCalib.C                                                         //
//                                                                      //
// Clone a calibration.                                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void CloneCalib()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // macro configuration
    const Char_t calibOrig[]    = "LD2_MC_May_09";
    const Char_t calibNew[]     = "LH2_MC_Apr_09";
    const Char_t calibNewDesc[] = "MC calibration for April 2009 beamtime";
    const Int_t firstRunNew     = 999004;
    const Int_t lastRunNew      = 999004;
    
    // clone the calibration
    TCMySQLManager::GetManager()->CloneCalibration(calibOrig, calibNew, calibNewDesc, 
                                                   firstRunNew, lastRunNew);

    gSystem->Exit(0);
}

