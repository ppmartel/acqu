// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ChangeRunNumbersDump.C                                               //
//                                                                      //
// Change first/last run numbers of calibration sets in a CaLib dump.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void ChangeRunNumbersDump(const Char_t* filename)
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // macro configuration
    const Int_t newFirstRun = 50000;
    const Int_t newLastRun  = 50381;

    // load CaLib container
    TFile f(filename);
    TCContainer* c = (TCContainer*) f.Get("CaLib_Dump");
    
    // user info
    printf("Updating first/last runs of calibration sets to %d/%d\n", newFirstRun, newLastRun);

    // loop over all calibrations
    for (Int_t i = 0; i < c->GetNCalibrations(); i++)
    {
        // get calibration
        TCCalibration* cal = c->GetCalibration(i);
        
        // set new first and last runs
        cal->SetFirstRun(newFirstRun);
        cal->SetLastRun(newLastRun);
    
        // user info
        printf("Updated calibration '%s'\n", cal->GetCalibData());
    }
    
    // construct new file name
    TString s(filename);
    s.ReplaceAll(".root", "");
    s.Append("_updated.root");

    // save update container
    TFile fout(s.Data(), "recreate");
    c->Write();

    // user info
    printf("Saved updated calibrations to '%s'\n", s.Data());

    gSystem->Exit(0);
}

