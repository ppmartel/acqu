// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAPSPed.C                                                            //
//                                                                      //
// Make run sets depending on the stability in time of a calibration.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TFile.h"
#include "TList.h"
#include "TCReadARCalib.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1.h"
#include "TSystem.h"
#include "TCMySQLManager.h"
#include "TLine.h"
#include "TGraph.h"


class TCReadARCalib;
TCReadARCalib* gReadAR;
TList* gFiles;


//______________________________________________________________________________
void CheckPedestals(const Char_t* loc)
{
    // Some old pedestal checking method.

    Char_t t[256];

    // number of runs
    Int_t nRuns = gFiles->GetSize();

    // number of channels
    Int_t nCh = gReadAR->GetNelements();

    // create arrays
    Double_t** pedPos = new Double_t*[nCh];
    Double_t* runNumbersD = new Double_t[nRuns];
    for (Int_t i = 0; i < nCh; i++) pedPos[i] = new Double_t[nRuns];

    // open the output files
    TFile* fROOTout = new TFile("/tmp/ped.root", "RECREATE");

    // create directories
    for (Int_t i = 0; i < nCh; i++)
    {
        sprintf(t, "%03d_%s", i, gReadAR->GetElement(i)->GetADC());
        fROOTout->mkdir(t);
    }

    TF1* func = new TF1("gausfunc", "gaus", 0 , 200);
    
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

        // loop over ADCs
        for (Int_t j = 0; j < nCh; j++)
        {
            // load histogram
            sprintf(t, "%03d_%s", j, gReadAR->GetElement(j)->GetADC());
            fROOTout->cd(t);
            sprintf(t, "ADC%s", gReadAR->GetElement(j)->GetADC());
            TH1D* h = (TH1D*) f->Get(t);
            
            // fit gaussian to pedestal
            //if (j == 20201 || j == 21201) h->GetXaxis()->SetRange(90, 115);
            //else h->GetXaxis()->SetRange(90, 115);
            //if (j == 27112) h->GetXaxis()->SetRange(60, 110);
            //else h->GetXaxis()->SetRange(60, 99);
            h->GetXaxis()->SetRangeUser(60, 120);
            Double_t maxPos = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
            
            h->GetXaxis()->SetRangeUser(60, 150);
            func->SetParameters(1, maxPos, 0.1);
            func->SetRange(maxPos - 2, maxPos + 2);
            h->Fit(func, "RBQ");
            maxPos = func->GetParameter(1);
            
            // save position in file and memory
            pedPos[j][i] = maxPos;

            sprintf(t, "Run_%d", runNumber);
            TCanvas* c = new TCanvas(t, t);
            h->Draw();
            
            TLine* tline = new TLine(maxPos, 0, maxPos, 10000);
            tline->SetLineColor(kRed);
            tline->SetLineWidth(2);
            tline->Draw();
            
            c->Write(c->GetName(), TObject::kOverwrite);
    
            delete h;
            delete c;
            delete tline;
        }
    }
    
    // create pedestal evolution graphs
    fROOTout->cd();
    
    // loop over channels
    for (Int_t j = 0; j < nCh; j++)
    {
        printf("Creating pedestal graph for channel %d\n", j);
        
        TGraph* g = new TGraph(nRuns, runNumbersD, pedPos[j]);
        sprintf(t, "Overview_%03d_%s", j, gReadAR->GetElement(j)->GetADC());
        g->SetName(t);
        g->SetTitle(t);
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
void TAPSPed()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    Bool_t watch = kFALSE;
    const Char_t* data = "Data.TAPS.LG.E0";
    const Char_t* elemDesc = "Element:";
    //const Char_t* elemDesc = "TAPSSG:";
    Double_t yMin = 110;
    Double_t yMax = 160;

    // configuration (December 2007)
    //const Char_t calibration[] = "LD2_Dec_07";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Dec_07/AR/out/ADC";
    //const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/TAPS/BaF2.dat";
    //const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/TAPS/Veto.dat";

    // configuration (February 2009)
    //const Char_t calibration[] = "LD2_Feb_09";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Feb_09/AR/out/ADC";
    //const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Feb_09/TAPS/BaF2_PWO.dat";
    //const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Feb_09/TAPS/Veto.dat";
    
    // configuration (May 2009)
    const Char_t calibration[] = "LD2_May_09";
    const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/May_09/AR/out/ADC";
    const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/May_09/TAPS/BaF2_PWO.dat";
    //const Char_t* fAR = "/usr/users/werthm/AcquRoot/acqu/acqu/data/May_09/TAPS/Veto.dat";

    // read the calibration file with the correct element identifier
    gReadAR = new TCReadARCalib(fAR, kFALSE, elemDesc);

    // user info
    printf("Found %d elements in '%s'\n", gReadAR->GetNelements(), fAR);

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
 
    // check pedestals
    CheckPedestals(fLoc);

    printf("%d runs analyzed.\n", gFiles->GetSize());
    
    // clean-up
    delete gFiles;

    gSystem->Exit(0);
}

