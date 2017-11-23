// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// PIDEnergy.C                                                          //
//                                                                      //
// Check calibration of runsets.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void PIDEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    const Char_t* data = "Data.PID.E0";
    const Char_t* hName = "CaLib_PID_Energy_Proton_PID_Energy";

    // configuration (December 2007)
    //const Char_t calibration[] = "LD2_Dec_07";
    //const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/Dec_07/AR/out/ARHistograms_CB_RUN.root";
    //const Char_t mcFile[] = "/usr/panther_scratch0/werthm/A2/Dec_07/MC/calibration/all.root";

    // configuration (February 2009)
    //const Char_t calibration[] = "LD2_Feb_09";
    //const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/Feb_09/AR/out/ARHistograms_CB_RUN.root";
    //const Char_t mcFile[] = "/usr/panther_scratch0/werthm/A2/Feb_09/MC/calibration/all.root";
    
    // configuration (May 2009)
    const Char_t calibration[] = "LD2_May_09";
    const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/May_09/AR/out/ARHistograms_CB_RUN.root";
    const Char_t mcFile[] = "/usr/panther_scratch0/werthm/A2/May_09/MC/calibration/all.root";
    
    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);

    // create canvas
    Int_t n = TMath::Sqrt(nSets);
    TCanvas* cOverview = new TCanvas("c", "c", 1200, 900);
    cOverview->Divide(n, nSets / n + 1);
    
    // create arrays
    Double_t* pos = new Double_t[nSets+1];
    Double_t* fwhm = new Double_t[nSets+1];
    
    // total sum histogram
    TH1* hTot = 0;

    // load MC histo
    TFile* fMC = new TFile(mcFile);
    TH2* hMC2 = (TH2*) fMC->Get(hName);
    TH1* hMC = hMC2->ProjectionX("mc");
    hMC->SetLineColor(kBlue);

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    { 
        // create file manager
        TCFileManager m(data, calibration, 1, &i, filePat);
        
        // get histo
        TH2* h2 = (TH2*) m.GetHistogram(hName);
        
        // skip empty histo
        if (!h2) continue;

        // project histo
        sprintf(tmp, "Proj_%d", i);
        TH1* h = (TH1*) h2->ProjectionX(tmp);
        //TH1* h = (TH1*) h2->ProjectionX(tmp, 9, 9);
        //h->Rebin(4);

        // add to total histogram
        if (!hTot) hTot = (TH1*) h->Clone();
        else hTot->Add(h);
 
        // fit histo
        cOverview->cd(i+1);
        h->SetLineColor(kBlack);
        h->Draw();
        hMC->Scale(h->GetMaximum()/hMC->GetMaximum());
        hMC->DrawCopy("same");
    }
}

