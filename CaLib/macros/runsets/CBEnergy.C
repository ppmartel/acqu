// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CBEnergy.C                                                           //
//                                                                      //
// Make run sets depending on the stability in time of a calibration.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "Config.h"

TCanvas* gCFit;
TH1* gHOverview;
TH1* gH;
TH2* gH2;
TF1* gFitFunc;
TLine* gLine;


//______________________________________________________________________________
void Fit(Int_t run, Bool_t fitEta)
{
    // Perform fit.
    
    Char_t tmp[256];
    
    // delete old function
    if (gFitFunc) delete gFitFunc;
    sprintf(tmp, "fEnergy_%i", run);
    gFitFunc = new TF1(tmp, "gaus(0)+pol3(3)");
    gFitFunc->SetLineColor(2);
    
    // configure fitting function
    if (fitEta)
    {
        gFitFunc->SetRange(470, 630);
        gFitFunc->SetParameters(gH->GetMaximum(), 547, 15, 1, 1, 1, 0.1);
        gFitFunc->SetParLimits(0, 0, gH->GetMaximum());  
        gFitFunc->SetParLimits(1, 530, 580);  
        gFitFunc->SetParLimits(2, 8, 20);
        gFitFunc->FixParameter(6, 0);
    }
    else
    {
        gFitFunc->SetRange(100, 180);
        gFitFunc->SetParameters(gH->GetMaximum(), 135, 10, 1, 1, 1, 0.1);
        gFitFunc->SetParLimits(0, 0, gH->GetMaximum()*10);  
        gFitFunc->SetParLimits(1, 115, 145);  
        gFitFunc->SetParLimits(2, 5, 12);
    }
    
    Int_t fitres;
    for (Int_t i = 0; i < 10; i++)
        if (!(fitres = gH->Fit(gFitFunc, "RB0Q"))) break;
  
    // get position
    Double_t pos = gFitFunc->GetParameter(1);

    // check failed fits
    if (fitres) 
    {
        printf("Run %d: fit failed\n", run);
        return;
    }
    else
    {
        if (fitEta) printf("Run %d: Eta : %.2f   FWHM : %.2f\n", run, pos, 2.35*gFitFunc->GetParameter(2));
        else printf("Run %d: Pi0 : %.2f   FWHM : %.2f\n", run, pos, 2.35*gFitFunc->GetParameter(2));
    }

    if (pos < 130) printf("\n");

    // indicator line
    gLine->SetX1(pos);
    gLine->SetX2(pos);
    gLine->SetY1(0);
    gLine->SetY2(gH->GetMaximum());

    // draw 
    gCFit->cd();
    if (fitEta) gH->GetXaxis()->SetRangeUser(400, 700);
    else gH->GetXaxis()->SetRangeUser(70, 200);
    gH->Draw();
    gFitFunc->Draw("same");
    gLine->Draw("same");

    // fill overview histogram
    gHOverview->SetBinContent(run+1, pos);
    gHOverview->SetBinError(run+1, 0.0001);
}

//______________________________________________________________________________
void CBEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    Bool_t watch = kTRUE;
    const Char_t* data = "Data.CB.E1";
    const Char_t* hName = "CaLib_CB_IM_Neut";
    Double_t yMin = 110;
    Double_t yMax = 160;   
    
    
    // create histogram
    gHOverview = new TH1F("CBEnergy", "CBEnergy", 40000, 0, 40000);
    gHOverview->SetXTitle("Run Number");
    gHOverview->SetYTitle("#pi^{0} peak [MeV]");
    TCanvas* cOverview = new TCanvas("CBEnergy", "CBEnergy");
    gHOverview->GetYaxis()->SetRangeUser(yMin, yMax);
    gHOverview->Draw("E1");
    
    // create line
    gLine = new TLine();
    gLine->SetLineColor(kBlue);
    gLine->SetLineWidth(2);

    // init fitting function
    gFitFunc = 0;
    
    // create fitting canvas
    gCFit = new TCanvas();
    
    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);
    
    // total number of runs
    Int_t nTotRuns = 0;

    // first and last runs
    Int_t first_run, last_run;

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    {
        // get runs of set
        Int_t nRuns;
        Int_t* runs = TCMySQLManager::GetManager()->GetRunsOfSet(data, calibration, i, &nRuns);
    
        // loop over runs
        for (Int_t j = 0; j < nRuns; j++)
        {
            // save first and last runs
            if (i == 0 && j == 0) first_run = runs[j];
            if (i == nSets-1 && j == nRuns-1) last_run = runs[j];

            
            // load ROOT file
            sprintf(tmp, "%s/Hist_CBTaggTAPS_%d.root", fLoc, runs[j]);
            TFile* gFile = new TFile(tmp);

            // check file
            if (!gFile) continue;
            if (gFile->IsZombie()) continue;

            // load histogram
            gH2 = (TH2*) gFile->Get(hName);
            if (!gH2) continue;
            if (!gH2->GetEntries()) continue;

            // project histogram
            sprintf(tmp, "Proj_%d", runs[j]);
            gH = gH2->ProjectionX(tmp);

            // fit the histogram
            Fit(runs[j], kFALSE);
            //Fit(runs[j], kTRUE);

            gFile->Close();
            
            // update canvases and sleep
            if (watch)
            {
                cOverview->Update();
                gCFit->Update();
                gSystem->Sleep(100);
                //if (runs[j] > 13595) Char_t gg = getchar();
            }
            
            // count run
            nTotRuns++;
        }

        // clean-up
        delete runs;

        // draw runset markers
        cOverview->cd();
        
        // get first run of set
        Int_t frun = TCMySQLManager::GetManager()->GetFirstRunOfSet(data, calibration, i);

        // draw line
        TLine* aLine = new TLine(frun, yMin, frun, yMax);
        aLine->SetLineColor(kBlue);
        aLine->SetLineWidth(2);
        aLine->Draw("same");
    }
    
    // adjust axis
    gHOverview->GetXaxis()->SetRangeUser(first_run-10, last_run+10);

    gHOverview->SaveAs("Calib_Overview_CBEnergy.root");

    printf("%d runs analyzed.\n", nTotRuns);

    gSystem->Exit(0);
}

