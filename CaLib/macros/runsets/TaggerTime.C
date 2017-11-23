// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TaggerTime.C                                                         //
//                                                                      //
// Make run sets depending on the stability in time of a calibration.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TFile.h"
#include "TList.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH2.h"
#include "TSystem.h"
#include "TCMySQLManager.h"
#include "TLine.h"
#include "TGraph.h"


TList* gFiles;


//______________________________________________________________________________
void CheckTime(const Char_t* loc)
{
    // Check time.

    Char_t t[256];

    // number of runs
    Int_t nRuns = gFiles->GetSize();

    // create arrays
    const Int_t nCh = 352;
    Double_t** pedPos = new Double_t*[nCh];
    Double_t* runNumbersD = new Double_t[nRuns];
    for (Int_t i = 0; i < nCh; i++) pedPos[i] = new Double_t[nRuns];

    // open the output files
    TFile* fROOTout = new TFile("/tmp/tagger_time.root", "RECREATE");

    // create directories
    for (Int_t i = 0; i < nCh; i++)
    {
        sprintf(t, "%03d", i);
        fROOTout->mkdir(t);
    }
    
    TF1* func = new TF1("func", "gaus(0)+pol1(3)", -1000 , 1000);

    // loop over runs
    for (Int_t i = 0; i < nRuns; i++)
    {   
        // get the file
        TFile* f = (TFile*) gFiles->At(i);

        // extract run number
        Int_t runNumber;
        sprintf(t, "%s/ARHistograms_CB_%%d.root", loc);
        sscanf(f->GetName(), t, &runNumber);
        runNumbersD[i] = (Double_t)runNumber;

        printf("Processing run %d (%d/%d)\n", runNumber, i+1, nRuns);

        fROOTout->cd();
        
        TH2* h2 = (TH2*) f->Get("CaLib_Tagger_Time");

        // loop over channels
        for (Int_t j = 0; j < nCh; j++)
        {
            // load histogram
            sprintf(t, "%03d", j);
            fROOTout->cd(t);
            sprintf(t, "%d_%d", i, j);
            TH1* h = h2->ProjectionX(t, j+1, j+1);
            h->Rebin(2);
            
            sprintf(t, "Run_%d", runNumber);
            TCanvas* c = new TCanvas(t, t);
            TLine* tline = 0;
            
            // check entries
            if (h->GetEntries())
            {
                // fit gaussian to peak
                Double_t maxPos = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
                func->SetParameters(1, maxPos, 0.5, 1, 0.1);
                func->SetRange(maxPos - 2, maxPos + 2);
                func->SetParLimits(0, 0, 1000);
                for (Int_t k = 0; k < 10; k++)
                    if (!h->Fit(func, "RBQ")) break;

                // save position in file and memory
                maxPos = func->GetParameter(1);
                pedPos[j][i] = maxPos;

                h->GetXaxis()->SetRangeUser(maxPos - 10, maxPos + 10);
                h->Draw();
                
                tline = new TLine(maxPos, 0, maxPos, 10000);
                tline->SetLineColor(kRed);
                tline->SetLineWidth(2);
                tline->Draw();
            }
            else
            {
                h->Draw();
            }

            c->Write(c->GetName(), TObject::kOverwrite);
    
            delete h;
            delete c;
            if (tline) delete tline;
        }
        
        delete h2;
    }
    
    // create pedestal evolution graphs
    fROOTout->cd();
    
    // loop over channels
    for (Int_t j = 0; j < nCh; j++)
    {
        printf("Creating time graph for channel %d\n", j);
        
        TGraph* g = new TGraph(nRuns, runNumbersD, pedPos[j]);
        sprintf(t, "Overview_%03d", j);
        g->SetName(t);
        g->SetTitle(t);
        //g->GetYaxis()->SetRangeUser(1200, 1300);
        g->Write(g->GetName(), TObject::kOverwrite);
        
        delete g;
    }

    printf("Saving output file\n");
    
    delete fROOTout;

    // cleanup
    for (Int_t i = 0; i < nCh; i++) delete [] pedPos[i];
    delete [] pedPos;
    delete [] runNumbersD;
}

//______________________________________________________________________________
void TaggerTime()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    Bool_t watch = kFALSE;
    const Char_t* data = "Data.Tagger.T0";

    // configuration (December 2007)
    const Char_t calibration[] = "LD2_Dec_07";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Dec_07/AR/out/tagger_time";
    const Char_t* fLoc = "/usr/cheetah_scratch0/kaeser/CaLib/Dec_07";

    // configuration (February 2009)
    //const Char_t calibration[] = "LD2_Feb_09";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Feb_09/AR/out/ADC";
    
    // configuration (May 2009)
    //const Char_t calibration[] = "LD2_May_09";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/May_09/AR/out/ADC";

    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);
    
    // file array
    gFiles = new TList();
    gFiles->SetOwner(kTRUE);

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    {
        // get runs of set
        Int_t nRuns;
        Int_t* runs = TCMySQLManager::GetManager()->GetRunsOfSet(data, calibration, i, &nRuns);
    
        // loop over runs
        for (Int_t j = 0; j < nRuns; j++)
        {
            // load ROOT file
            sprintf(tmp, "%s/ARHistograms_CB_%d.root", fLoc, runs[j]);
            TFile* f = new TFile(tmp);

            // check file
            if (!f) continue;
            if (f->IsZombie()) continue;

            // save file
            gFiles->Add(f);
        }

        // clean-up
        delete runs;
    }
 
    // check time
    CheckTime(fLoc);

    printf("%d runs analyzed.\n", gFiles->GetSize());
    
    // clean-up
    delete gFiles;

    gSystem->Exit(0);
}

