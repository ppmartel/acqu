// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAPSEnergy.C                                                         //
//                                                                      //
// Check calibration of runsets.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Fit(TH1* h, Double_t* outPos, Double_t* outFWHM)
{
    // Perform fit.
    
    Char_t tmp[256];

    Double_t x1 = 100;
    Double_t x2 = 200;

    TF1* func = new TF1("func", "gaus(0)+pol3(3)", x1, x2);
    func->SetParameters(h->GetMaximum(), 141, 8, 1, 1, 0.1, 0.1);
    func->SetLineColor(kBlue);
    func->SetParLimits(1, 130, 145);
    func->SetParLimits(2, 1, 15);
    h->Fit(func, "RBQO");
    
    // get position and FWHM
    fPi0Pos = func->GetParameter(1);
    *outPos = fPi0Pos;
    *outFWHM = 2.35*func->GetParameter(2);

    // indicator line
    TLine* line = new TLine();
    line->SetLineWidth(2);
    line->SetX1(fPi0Pos);
    line->SetX2(fPi0Pos);
    line->SetY1(0);
    line->SetY2(h->GetMaximum());

    TF1* fBG = new TF1("funcBG", "pol5", x1, x2);
    for (Int_t i = 0; i < 6; i++) fBG->SetParameter(i, func->GetParameter(3+i));
    fBG->SetLineColor(kRed);

    gPad->SetLogy();

    // draw 
    h->GetXaxis()->SetRangeUser(0, 300);
    h->Draw();
    func->Draw("same");
    fBG->Draw("same");
    line->Draw("same");
}

//______________________________________________________________________________
void TAPSEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    const Char_t* data = "Data.TAPS.LG.E1";
    //const Char_t* hName = "CaLib_TAPS_IM_Neut";
    //const Char_t* hName = "CaLib_TAPS_IM_Neut_1CB_1TAPS";
    const Char_t* hName = "CaLib_TAPS_IM_Neut_2TAPS";

    // configuration (December 2007)
    //const Char_t calibration[] = "LD2_Dec_07";
    //const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/Dec_07/AR/out/ARHistograms_CB_RUN.root";
    //const Char_t filePat[] = "/Users/fulgur/Desktop/calib/Dec_07/ARHistograms_CB_RUN.root";

    // configuration (February 2009)
    const Char_t calibration[] = "LD2_Feb_09";
    //const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/Feb_09/AR/out/ARHistograms_CB_RUN.root";
    const Char_t filePat[] = "/Users/fulgur/Desktop/calib/Feb_09/ARHistograms_CB_RUN.root";

    // configuration (May 2009)
    //const Char_t calibration[] = "LD2_May_09";
    //const Char_t filePat[] = "/usr/puma_scratch0/werthm/A2/May_09/AR/out/ARHistograms_CB_RUN.root";
    //const Char_t filePat[] = "/Users/fulgur/Desktop/calib/May_09/ARHistograms_CB_RUN.root";
    //const Char_t filePat[] = "/Users/fulgur/Desktop/calib/May_09/ARHistograms_CB_RUN.root";
    
    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);

    // create canvas
    Int_t n = TMath::Sqrt(nSets);
    TCanvas* cOverview = new TCanvas();
    cOverview->Divide(n, nSets / n + 1);
    
    // create arrays
    Double_t* pos = new Double_t[nSets+1];
    Double_t* fwhm = new Double_t[nSets+1];
    
    // total sum histogram
    TH1* hTot = 0;

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
        
        // add to total histogram
        if (!hTot) hTot = (TH1*) h->Clone();
        else hTot->Add(h);

        // fit histo
        cOverview->cd(i+1);
        Fit(h, &pos[i], &fwhm[i]);
    }

    // show total histogram
    TCanvas* cTot = new TCanvas();
    Fit(hTot, &pos[nSets], &fwhm[nSets]);

    // show results
    for (Int_t i = 0; i < nSets; i++)
        printf("Set %02d:   Pos: %.2f MeV   FWHM: %.2f MeV\n", i, pos[i], fwhm[i]);
    printf("Total :   Pos: %.2f MeV   FWHM: %.2f MeV\n", pos[nSets], fwhm[nSets]);
}

