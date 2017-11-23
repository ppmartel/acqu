// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTargetPosition                                                //
//                                                                      //
// Calibration module for the target position.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTargetPosition.h"



//______________________________________________________________________________
TCCalibTargetPosition::TCCalibTargetPosition()
    : TCCalib("Target.Position", "Target position calibration", "Data.CB.E1",
              TCReadConfig::GetReader()->GetConfigInt("Target.Position.Bins"))
{
    // Empty constructor.
    
    // init members
    fSigmaPrev = 0;
    fLine = 0;

}

//______________________________________________________________________________
TCCalibTargetPosition::~TCCalibTargetPosition()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTargetPosition::Init()
{
    // Init the module.
    
    // init members
    fSigmaPrev = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("Target.Position.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("Target.Position.Histo.Fit.Name");
    
    // copy to new parameters
    fNewVal[0] = 0;

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }

    // get target position limits
    Double_t min, max;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("Target.Position.Range", &min, &max);
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Target position [cm];#pi^{0} peak sigma [MeV]", fNelem, min, max);
    fOverviewHisto->SetMarkerStyle(20);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    TCUtils::FormatHistogram(fMainHisto, "Target.Position.Histo.Fit");
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "Target.Position.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTargetPosition::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fEnergy_%i", elem);
        fFitFunc = new TF1(tmp, "pol2+gaus(3)");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        Double_t peak = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (peak < 100 || peak > 160) peak = 135;

        // estimate background
        Double_t bgPar0, bgPar1;
        TCUtils::FindBackground(fFitHisto, peak, 50, 50, &bgPar0, &bgPar1);
        
        // configure fitting function
        fFitFunc->SetRange(peak - 60, peak + 60);
        fFitFunc->SetLineColor(2);
        fFitFunc->SetParameters( 3.8e+2, -1.90, 0.1, 150, peak, 8.9);
        fFitFunc->SetParLimits(5, 3, 20);  
        fFitFunc->FixParameter(2, 0);
        fFitHisto->Fit(fFitFunc, "RB0Q");

        // final results
        peak = fFitFunc->GetParameter(4); 

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (peak < 80 || peak > 200) peak = 135;
        
        // set indicator line
        fLine->SetX1(peak);
        fLine->SetX2(peak);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "Target.Position.Histo.Fit");
    fFitHisto->Draw("hist");
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd();
        fOverviewHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibTargetPosition::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // get sigma
        Double_t sigma = fFitFunc->GetParameter(5);
        Double_t sigma_err = fFitFunc->GetParError(5);

        // skip bad values
        Double_t diff = TMath::Abs(fSigmaPrev - sigma);
        
        if (diff/sigma < 0.1 || elem == 0)
        {
            // save sigma
            fSigmaPrev = sigma;

            // update overview histogram
            fOverviewHisto->SetBinContent(elem+1, sigma);
            fOverviewHisto->SetBinError(elem+1, sigma_err);
        
            // user information
            printf("Position: %6.2f cm   FWHM: %.2f MeV\n", fOverviewHisto->GetXaxis()->GetBinCenter(elem+1), sigma*2.35);
        }
    }

    // finish calibration
    if (elem == fNelem-1)
    {
        // update overview plot
        fCanvasResult->cd();
        fOverviewHisto->Draw("E1");
        fCanvasResult->Update();
    
        // fit plot
        if (fFitFunc) delete fFitFunc;
        fFitFunc = new TF1("fResult", "pol2");
        fFitFunc->SetLineColor(2);

        // select range around minimum
        Double_t min = TCUtils::GetHistogramMinimumPosition(fOverviewHisto);
        fFitFunc->SetRange(min - 2, min + 2);

        // fit histogram 
        fOverviewHisto->Fit(fFitFunc, "RBQ0");

        // get minimum
        Double_t targetPos = -fFitFunc->GetParameter(1) / 2. / fFitFunc->GetParameter(2);
        
        // reset range and refit
        fFitFunc->SetRange(targetPos - 2, targetPos + 2);
        fOverviewHisto->Fit(fFitFunc, "RBQ0");
        
        // get new minimum
        targetPos = -fFitFunc->GetParameter(1) / 2. / fFitFunc->GetParameter(2);

        // draw indicator line
        fLine->SetY1(0);
        fLine->SetY2(fOverviewHisto->GetMaximum());
        fLine->SetX1(targetPos);
        fLine->SetX2(targetPos);
        fLine->Draw("same");
        
        // update canvas
        fFitFunc->Draw("same");
        fCanvasResult->Update();

        // save value
        fNewVal[0] = targetPos;

        // user information
        PrintValues();
     }

}   

//______________________________________________________________________________
void TCCalibTargetPosition::Write()
{
    // Save the overview plot.
    
    // save overview picture
    SaveCanvas(fCanvasResult, "Overview");
}

//______________________________________________________________________________
void TCCalibTargetPosition::PrintValues()
{
    // Print out the old and new values for all elements.

    printf("\n");
    printf("target position: %12.8f\n", fNewVal[0]);
    printf("\n");
}
ClassImp(TCCalibTargetPosition)
