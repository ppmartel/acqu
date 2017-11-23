// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBTimeWalk                                                    //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibCBTimeWalk.h"



//______________________________________________________________________________
TCCalibCBTimeWalk::TCCalibCBTimeWalk()
    : TCCalib("CB.TimeWalk", "CB time walk calibration", "Data.CB.Walk.Par0", TCConfig::kMaxCB)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPar0 = 0;
    fPar1 = 0;
    fPar2 = 0;
    fPar3 = 0;
    fEnergyProj = 0;
    fTimeProj = 0;
    fLine = 0;
    fDelay = 0;
}

//______________________________________________________________________________
TCCalibCBTimeWalk::~TCCalibCBTimeWalk()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fPar2) delete [] fPar2;
    if (fPar3) delete [] fPar3;
    if (fEnergyProj) delete fEnergyProj;
    if (fTimeProj) delete fTimeProj;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new TCFileManager(fData.Data(), fCalibration.Data(), fNset, fSet);
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fPar2 = new Double_t[fNelem];
    fPar3 = new Double_t[fNelem];
    fEnergyProj = 0;
    fTimeProj = 0;
    fLine =  new TLine();
    fDelay = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name");
    
    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("CB.TimeWalk.Fit.Delay");

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par0", fCalibration.Data(), fSet[0], fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par1", fCalibration.Data(), fSet[0], fPar1, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par2", fCalibration.Data(), fSet[0], fPar2, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par3", fCalibration.Data(), fSet[0], fPar3, fNelem);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->cd();
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.TimeWalk.Histo.Fit.Xaxis.Range", &lowLimit, &highLimit);
     
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
   
    // delete old histogram
    if (fMainHisto) delete fMainHisto;
  
    // get histogram
    fMainHisto = fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // draw main histogram
    if (fMainHisto->GetEntries() > 0) fCanvasFit->cd(1)->SetLogz(1);
    else fCanvasFit->cd(1)->SetLogz(0);
    TCUtils::FormatHistogram(fMainHisto, "CB.TimeWalk.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries() > 1000)
    {
        // create energy projection
        sprintf(tmp, "ProjEnergy_%d", elem);
        TH2* h2 = (TH2*) fMainHisto;
        if (fEnergyProj) delete fEnergyProj;
        fEnergyProj = new TH1F(tmp, tmp, 4000, 0, 1000);

        // prepare stuff for adding
        Int_t added = 0;
        Double_t added_e[300];
        
        // get bins for fitting range
        Int_t startBin = h2->GetXaxis()->FindBin(lowLimit);
        Int_t endBin = h2->GetXaxis()->FindBin(highLimit);

        // loop over energy bins
        for (Int_t i = startBin; i <= endBin; i++)
        {   
            // create time projection
            sprintf(tmp, "ProjTime_%d_%d", elem, i);
            TH1* proj = (TH1D*) h2->ProjectionY(tmp, i, i, "e");
            
            // check if in adding mode
            if (added)
            {
                // add projection
                fTimeProj->Add(proj);
                delete proj;
                 
                // save bin contribution
                added_e[added++] = h2->GetXaxis()->GetBinCenter(i);
            }
            else 
            {
                if (fTimeProj) delete fTimeProj;
                fTimeProj = proj;
            }

            // check if projection has enough entries
            if (fTimeProj->GetEntries() < 100 && i < endBin)
            {
                // start adding mode
                if (!added)
                {
                    // enter adding mode
                    added_e[added++] = h2->GetXaxis()->GetBinCenter(i);
                }

                // go to next bin
                continue;
            }
            else
            {
                Double_t energy = 0;

                // finish adding mode
                if (added)
                {
                    // calculate energy
                    for (Int_t j = 0; j < added; j++) energy += added_e[j];
                    energy /= (Double_t)added;
                    
                    added = 0;
                }
                else
                {
                    energy = h2->GetXaxis()->GetBinCenter(i);
                }

                //
                // fit time projection
                //
                
                // create fitting function
                if (fFitFunc) delete fFitFunc;
                sprintf(tmp, "fWalkProfile_%i", elem);
                fFitFunc = new TF1(tmp, "pol0(0)+gaus(1)");
                fFitFunc->SetLineColor(2);

                // prepare fitting function
                Int_t maxbin = fTimeProj->GetMaximumBin();
                Double_t peak = fTimeProj->GetBinCenter(maxbin);
                fFitFunc->SetRange(peak - 6, peak + 6);
                fFitFunc->SetParameters(1., fTimeProj->GetMaximum(), peak, 1.);
                fFitFunc->SetParLimits(0, 0, 10000); // offset
                fFitFunc->SetParLimits(1, 0, 10000); // peak
                fFitFunc->SetParLimits(2, -1000, 1000); // peak position
                fFitFunc->SetParLimits(3, 0.1, 20.0); // sigma
                
                // perform fit
                fTimeProj->Fit(fFitFunc, "RBQ0");

                // get parameters
                Double_t mean = fFitFunc->GetParameter(2);
                Double_t error = fFitFunc->GetParError(2);

                // format line
                fLine->SetY1(0);
                fLine->SetY2(fTimeProj->GetMaximum() + 20);
                fLine->SetX1(mean);
                fLine->SetX2(mean);

                // check fit error
                if (error < 1.)
                {
                    Int_t bin = fEnergyProj->GetXaxis()->FindBin(energy);
                    fEnergyProj->SetBinContent(bin, mean);
                    fEnergyProj->SetBinError(bin, error);
                }

                // plot projection fit  
                if (fDelay > 0)
                {
                    fCanvasFit->cd(2);
                    fTimeProj->GetXaxis()->SetRangeUser(mean - 30, mean + 30);
                    fTimeProj->Draw("hist");
                    fFitFunc->Draw("same");
                    fLine->Draw();
                    fCanvasFit->Update();
                    gSystem->Sleep(fDelay);
                }
            
            } // if: projection has sufficient statistics
        
        } // for: loop over energy bins

        //
        // fit profile
        //
        
        // create fitting function
        sprintf(tmp, "fTWalk_%d", elem);
        if (fFitFunc) delete fFitFunc;
        fFitFunc = new TF1(tmp, this, &TCCalibCBTimeWalk::TWFunc, 
                           lowLimit, highLimit, 4, 
                           "TCCalibCBTimeWalk", "TWFunc");

        // prepare fitting function
        fFitFunc->SetLineColor(2);
	fFitFunc->SetParameters(3.55e+01, 6.77e+01, 2.43e-01, 1.66e-01);
	//fFitFunc->SetParLimits(0, 30, 80);
	fFitFunc->SetParLimits(1, 30, 120);
	fFitFunc->SetParLimits(2, 1e-4, 10);
	fFitFunc->SetParLimits(3, 0, 1);
	
        // perform fit
        for (Int_t i = 0; i < 10; i++)
            if (!fEnergyProj->Fit(fFitFunc, "RB0Q")) break;
    
        // read parameters
        fPar0[elem] = fFitFunc->GetParameter(0);
        fPar1[elem] = fFitFunc->GetParameter(1);
        fPar2[elem] = fFitFunc->GetParameter(2);
        fPar3[elem] = fFitFunc->GetParameter(3);

        // draw energy projection and fit
        fCanvasResult->cd();
        fEnergyProj->SetMarkerStyle(20);
        fEnergyProj->SetMarkerColor(4);
        fEnergyProj->GetXaxis()->SetRangeUser(lowLimit, highLimit);
        fEnergyProj->GetYaxis()->SetRangeUser(TCUtils::GetHistogramMinimum(fEnergyProj) - 5, fEnergyProj->GetMaximum() + 5);
        fEnergyProj->Draw("E1");
        fFitFunc->Draw("same");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
Double_t TCCalibCBTimeWalk::TWFunc(Double_t* x, Double_t* par)
{   
    // Time walk fitting function.

    return par[0] + par[1] / TMath::Power(x[0] + par[2], par[3]);
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t noval = kFALSE;
    
    // check no fits
    if (fPar0[elem] == 0 && fPar1[elem] == 0 &&
        fPar2[elem] == 0 && fPar3[elem] == 0) noval = kTRUE;

    // user information
    printf("Element: %03d    Par0: %12.8f    "
           "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f",
           elem, fPar0[elem], fPar1[elem], fPar2[elem], fPar3[elem]);
    if (noval) printf("    -> no fit");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibCBTimeWalk::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f\n",
               i, fPar0[i], fPar1[i], fPar2[i], fPar3[i]);
    }
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par0", fCalibration.Data(), fSet[i], fPar0, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par1", fCalibration.Data(), fSet[i], fPar1, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par2", fCalibration.Data(), fSet[i], fPar2, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par3", fCalibration.Data(), fSet[i], fPar3, fNelem);
    }
}
ClassImp(TCCalibCBTimeWalk)
