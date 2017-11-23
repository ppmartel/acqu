// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddSet.C                                                             //
//                                                                      //
// Add manually a new set to the database.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddSet()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Int_t firstRun            = 13089;
    const Int_t lastRun             = 13841;
    const Char_t calibName[]        = "LD2_Dec_07";
    const Char_t calibDesc[]        = "Standard calibration for December 2007 beamtime";
    const Char_t calibType[]        = "Type.Tagger.Eff";

    // add set
    TCMySQLManager::GetManager()->AddSet(calibType, calibName, calibDesc, firstRun, lastRun, 0);
     
    gSystem->Exit(0);
}

