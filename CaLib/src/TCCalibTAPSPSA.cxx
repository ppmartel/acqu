// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSPSA                                                       //
//                                                                      //
// Calibration module for TAPS PSA.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSPSA.h"



//______________________________________________________________________________
TCCalibTAPSPSA::TCCalibTAPSPSA()
    : TCCalib("TAPS.PSA", "TAPS PSA", "Data.TAPS.SG.E1",
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fNpoints = 0;
    fRadiusMean = 0;
    fRadiusSigma = 0;
    fMean = 0;
    fSigma = 0;
    fLMean = 0;
    fLSigma = 0;
    fGMean = 0;
    fGSigma = 0;
    fFileManager = 0;
    fAngleProj = 0;
    fDelay = 0;
}

//______________________________________________________________________________
TCCalibTAPSPSA::~TCCalibTAPSPSA()
{
    // Destructor. 
    
    if (fRadiusMean) delete [] fRadiusMean;
    if (fRadiusSigma) delete [] fRadiusSigma;
    if (fMean) delete [] fMean;
    if (fSigma) delete [] fSigma;
    if (fLMean) delete fLMean;
    if (fLSigma) delete fLSigma;
    if (fFileManager) delete fFileManager;
    if (fAngleProj) delete fAngleProj;
    if (fGMean)
    {
        for (Int_t i = 0; i < fNelem; i++) delete fGMean[i];
        delete [] fGMean;
    }
    if (fGSigma)
    {
        for (Int_t i = 0; i < fNelem; i++) delete fGSigma[i];
        delete [] fGSigma;
    }
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Init()
{
    // Init the module.
    
    // init members
    fNpoints = 0;
    fRadiusMean = new Double_t[100];
    fRadiusSigma = new Double_t[100];
    fMean = new Double_t[100];
    fSigma = new Double_t[100];
    fLMean = 0;
    fLSigma = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fAngleProj = 0;
    fDelay = 0;
    fGMean = new TGraph*[fNelem];
    fGSigma = new TGraph*[fNelem];
    for (Int_t i = 0; i < fNelem; i++)
    {
        fGMean[i] = 0;
        fGSigma[i] = 0;
    }

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.PSA.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.PSA.Histo.Fit.Name");
    
    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("TAPS.PSA.Fit.Delay");

    // draw main histogram
    fCanvasFit->SetLogz();
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t lowLimitX, highLimitX, lowLimitY, highLimitY;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.PSA.Histo.Fit.Xaxis.Range", &lowLimitX, &highLimitX);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.PSA.Histo.Fit.Yaxis.Range", &lowLimitY, &highLimitY);
  
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
    fCanvasFit->cd();
    TCUtils::FormatHistogram(fMainHisto, "TAPS.PSA.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
    
    // reset points
    fNpoints = 0;

    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        // cast to 2-dim histo
        TH2* h2 = (TH2*) fMainHisto;

        // prepare stuff for adding
        Int_t added = 0;
        Double_t added_r[500];
        
        // get bins for fitting range
        Int_t startBin = h2->GetYaxis()->FindBin(lowLimitY);
        Int_t endBin = h2->GetYaxis()->FindBin(highLimitY);

        // loop over energy bins
        for (Int_t i = startBin; i <= endBin; i++)
        {   
            // create angle projection
            sprintf(tmp, "ProjAngle_%d_%d", elem, i);
            TH1* proj = (TH1D*) h2->ProjectionX(tmp, i, i, "e");
            
            // check if in adding mode
            if (added)
            {
                // add projection
                fAngleProj->Add(proj);
                delete proj;
                 
                // save bin contribution
                added_r[added++] = h2->GetYaxis()->GetBinCenter(i);
            }
            else 
            {
                if (fAngleProj) delete fAngleProj;
                fAngleProj = proj;
            }

            // check if projection has enough entries
            Double_t limit;
            if (h2->GetYaxis()->GetBinCenter(i) < 100) limit = 0.12*h2->GetEntries();
            else limit = 0.05*h2->GetEntries();
            if (fAngleProj->GetEntries() < limit && i < endBin)
            {
                // start adding mode
                if (!added)
                {
                    // enter adding mode
                    added_r[added++] = h2->GetYaxis()->GetBinCenter(i);
                }

                // go to next bin
                continue;
            }
            else
            {
                Double_t radius = 0;
                
                // finish adding mode
                if (added)
                {
                    // calculate energy
                    for (Int_t j = 0; j < added; j++) radius += added_r[j];
                    radius /= (Double_t)added;
                    
                    added = 0;
                }
                else
                {
                    radius = h2->GetYaxis()->GetBinCenter(i);
                }
                
                // skip point in punch-through region
                if (radius > 140 && radius < 310) continue;
                
                // rebin
                fAngleProj->Rebin(2);
                            
                // find peaks
                Double_t peakPhoton = 45;
                 
                // create fitting function
                if (fFitFunc) delete fFitFunc;
                sprintf(tmp, "fFunc_%i", elem);
                fFitFunc = new TF1(tmp, "gaus(0)+pol1(3)", peakPhoton-2, peakPhoton+2);
                fFitFunc->SetLineColor(2);

                // prepare fitting function
                fFitFunc->SetParameters(1, 45, 1, 1, 1, 1);
                fFitFunc->SetParLimits(0, 1, 1e5);
                fFitFunc->SetParLimits(1, 44, 47);
                fFitFunc->SetParLimits(2, 0.1, 2);

                // perform fit
                for (Int_t i = 0; i < 10; i++)
                    if (!fAngleProj->Fit(fFitFunc, "RB0Q")) break;

                // second iteration
                fFitFunc->SetRange(fFitFunc->GetParameter(1) - 4*fFitFunc->GetParameter(2),
                                   fFitFunc->GetParameter(1) + 4*fFitFunc->GetParameter(2));
                
                // perform fit
                for (Int_t i = 0; i < 10; i++)
                    if (!fAngleProj->Fit(fFitFunc, "RB0Q")) break;

                // get parameters
                if (fNpoints == 0) radius = 20;
                fRadiusMean[fNpoints] = radius;
                fRadiusSigma[fNpoints] = radius;
                fMean[fNpoints] = fFitFunc->GetParameter(1);
                fSigma[fNpoints] = fFitFunc->GetParameter(2);
                fNpoints++;

                // plot projection fit  
                if (fDelay > 0)
                {
                    fCanvasResult->cd();
                    fAngleProj->Draw("hist");
                    fFitFunc->Draw("same");
                    fCanvasResult->Update();
                    fCanvasFit->Update();
                    gSystem->Sleep(fDelay);
                }
            
            } // if: projection has sufficient statistics
        
        } // for: loop over energy bins
        
        // add last dummy point
        fRadiusMean[fNpoints] = 600;
        fRadiusSigma[fNpoints] = 600;
        fMean[fNpoints] = fMean[fNpoints-1];
        fSigma[fNpoints] = fSigma[fNpoints-1];
        fNpoints++;


        // 
        // create lines
        //

        if (fLMean) delete fLMean;
        fLMean = new TPolyLine(fNpoints);
        fLMean->SetLineWidth(2);

        if (fLSigma) delete fLSigma;
        fLSigma = new TPolyLine(fNpoints);
        fLSigma->SetLineWidth(2);
        fLSigma->SetLineStyle(2);

        for (Int_t i = 0; i < fNpoints; i++)
        {
            fLMean->SetPoint(i, fMean[i], fRadiusMean[i]);
            fLSigma->SetPoint(i, fMean[i] - 3*fSigma[i], fRadiusSigma[i]);
        }
        
        // draw lines
        fCanvasFit->cd();
        fLMean->Draw();
        fLSigma->Draw();

        // set log axis
        fCanvasFit->cd()->SetLogz();
    }
    else
    {
        fCanvasFit->cd()->SetLogz(kFALSE);
    }

    // update canvas
    fMainHisto->GetXaxis()->SetRangeUser(38, 50);
    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // reset mean and sigma from poly line
        for (Int_t i = 0; i < fNpoints; i++)
        {
            // mean
            fRadiusMean[i] = fLMean->GetY()[i];
            fMean[i] = fLMean->GetX()[i];

            // sigma
            fRadiusSigma[i] = fLSigma->GetY()[i];
            fSigma[i] = (fMean[i] - fLSigma->GetX()[i]) / 3;
        }
        
        // create graphs
        if (fGMean[elem]) delete fGMean[elem];
        fGMean[elem] = new TGraph(fNpoints, fRadiusMean, fMean);
        if (fGSigma[elem]) delete fGSigma[elem];
        fGSigma[elem] = new TGraph(fNpoints, fRadiusSigma, fSigma);
    }
    else
    {
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    processed ", elem);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibTAPSPSA::PrintValues()
{
    // Disable this method.
    
    Info("PrintValues", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Write()
{
    // Write the obtained calibration values to the database.
    
    Char_t tmp[256];
    Int_t nSave = 0;

    // open output file
    sprintf(tmp, "PSA_%s.root", fCalibration.Data());
    TFile f(tmp, "recreate");
    
    // save graphs
    f.cd();
    for (Int_t i = 0; i < fNelem; i++)
    {
        if (fGMean[i])
        {
            sprintf(tmp, "Mean_%03d", i);
            fGMean[i]->Write(tmp);
            sprintf(tmp, "Sigma_%03d", i);
            fGSigma[i]->Write(tmp);
            nSave++;
        }
    }
    
    Info("Write", "%d PSA graphs were written to '%s'", nSave, f.GetName());
}
ClassImp(TCCalibTAPSPSA)
