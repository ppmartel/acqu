// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAPSEnergy.C                                                         //
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
void Fit(Int_t run)
{
    // Perform fit.
    
    Char_t tmp[256];

    // delete old function
    if (gFitFunc) delete gFitFunc;
    sprintf(tmp, "fEnergy_%i", run);
    gFitFunc = new TF1(tmp, "gaus(0)+pol3(3)");
    gFitFunc->SetLineColor(2);
    
    // configure fitting function
    gFitFunc->SetRange(80, 180);
    gFitFunc->SetLineColor(2);
    gFitFunc->SetParameters(gH->GetMaximum(), 135, 10, 1, 1, 1, 0.1);
    gFitFunc->SetParLimits(0, 0, 5000);  
    gFitFunc->SetParLimits(1, 110, 130);  
    gFitFunc->SetParLimits(2, 5, 10);
    Int_t fitres;
    
    for (Int_t i = 0; i < 10; i++)
        if (!(fitres = gH->Fit(gFitFunc, "RBQ0"))) break;
  
    // get position
    fPi0Pos = gFitFunc->GetParameter(1);

    // check failed fits
    if (fitres) 
    {
        printf("Run %d: fit failed\n", run);
        return;
    }

    // indicator line
    gLine->SetX1(fPi0Pos);
    gLine->SetX2(fPi0Pos);
    gLine->SetY1(0);
    gLine->SetY2(gH->GetMaximum());

    // draw 
    gCFit->cd();
    gH->GetXaxis()->SetRangeUser(70, 200);
    gH->Draw();
    gFitFunc->Draw("same");
    gLine->Draw("same");

    // fill overview histogram
    gHOverview->SetBinContent(run+1, fPi0Pos);
    gHOverview->SetBinError(run+1, 0.0001);
}

//______________________________________________________________________________
void TAPSEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    Bool_t watch = kFALSE;
    const Char_t* data = "Data.TAPS.LG.E1";
    const Char_t* hName = "CaLib_TAPS_IM_Neut_1CB_1TAPS";
    //const Char_t* hName = "CaLib_TAPS_IM_Neut_2TAPS";
    Double_t yMin = 110;
    Double_t yMax = 160;

    // create histogram
    gHOverview = new TH1F("TAPSEnergy", "TAPS #pi^{0} peak position", 40000, 0, 40000);
    gHOverview->SetXTitle("Run Number");
    gHOverview->SetYTitle("#pi^{0} peak position [MeV]");
    TCanvas* cOverview = new TCanvas("cTAPSEnergy","TAPS Energy");
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

    if(nSets <= 0) {
        printf("No run sets found for calibeation \"%s\"\n", calibration);
        gSystem->Exit(1);
    }
    
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

            // clean-up
            if (gH) delete gH;
            if (gH2) delete gH2;
            gH = 0;
            gH2 = 0;

            // load ROOT file
            sprintf(tmp, "%s/Hist_CBTaggTAPS_%d.root", fLoc, runs[j]);
            TFile* gFile = new TFile(tmp,"read");

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
            Fit(runs[j]);
            
            // update canvases and sleep
            if (watch)
            {
                cOverview->Update();
                gCFit->Update();
                gSystem->Sleep(50);
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

    gHOverview->SaveAs("Calib_Overview_TAPSEnergy.root");

    printf("%d runs analyzed.\n", nTotRuns);

    gSystem->Exit(0);
}

