// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDPhi                                                        //
//                                                                      //
// Calibration module for the PID phi angle.                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDPhi.h"



//______________________________________________________________________________
TCCalibPIDPhi::TCCalibPIDPhi()
    : TCCalib("PID.Phi", "PID phi angle calibration", "Data.PID.Phi", TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fMean = 0;
    fLine = 0;
    fCanvasResult2 = 0;
    fOverviewHisto2 = 0;
    fFitFunc2 = 0;
}

//______________________________________________________________________________
TCCalibPIDPhi::~TCCalibPIDPhi()
{
    // Destructor. 
    
    if (fLine) delete fLine;
    if (fCanvasResult2) delete fCanvasResult2;
    if (fOverviewHisto2) delete fOverviewHisto2;
    if (fFitFunc2) delete fFitFunc2;
}

//______________________________________________________________________________
void TCCalibPIDPhi::Init()
{
    // Init the module.
    
    // init members
    fMean = 0;
    fLine = new TLine();
    fCanvasResult2 = 0;
    fOverviewHisto2 = 0;
    fFitFunc2 = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Phi.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Phi.Histo.Fit.Name");
    
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Phi angle [deg]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    TCUtils::FormatHistogram(fMainHisto, "PID.Phi.Histo.Fit");
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "PID.Phi.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibPIDPhi::Fit(Int_t elem)
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
        sprintf(tmp, "fPhi_%i", elem);
        
	// the fit function
	fFitFunc = new TF1("fFitFunc", "gaus(0)");
	fFitFunc->SetLineColor(2);
	
	// get maximum
        Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());

        // perform angle interval mapping if peak is split
        if (maxPos + 20 > 180 || maxPos - 20 < -180) 
        {
            printf("Mapping histogram to interval [0,360] because peak is split\n");
            
            // replace fitting histogram with mapped version
            TH1* hMapped = GetMappedHistogram(fFitHisto);
            delete fFitHisto;
            fFitHisto = hMapped;
            
            // get again maximum position
            maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
        }

	// configure fitting function
        fFitFunc->SetParameters(1, maxPos, 5);
        fFitFunc->SetRange(maxPos - 50, maxPos + 50);
  
        // fit
	fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fMean = fFitFunc->GetParameter(1); 
         
        // correct bad fits
        if (TMath::Abs(fMean) > 290) fMean = 0;

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "PID.Phi.Histo.Fit");
    fFitHisto->Draw("hist");
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
    // update canvas
    fCanvasFit->Update();
    
    // update overview
    fCanvasResult->cd();
    fOverviewHisto->Draw("E1");
    fCanvasResult->Update();
}

//______________________________________________________________________________
void TCCalibPIDPhi::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // set the new phi angle
        fNewVal[elem] = fMean;
    
        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fMean);
        fOverviewHisto->SetBinError(elem + 1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Peak: %12.8f", elem, fMean);
    if (unchanged) printf("    -> unchanged");
    printf("\n");

    // calculate final phi values using a fit of all elements
    if (elem == fNelem - 1)
    {
        // phi angles and sorted element indices
        Double_t phi[fNelem];
        Int_t elem_sorted[fNelem];

        // copy phi angle values
        for (Int_t i = 0; i < fNelem; i++)
            phi[i] = fOverviewHisto->GetBinContent(i + 1);
        
        // delete old stuff
        if (fOverviewHisto2) delete fOverviewHisto2;
        if (fCanvasResult2) delete fCanvasResult2;
        if (fFitFunc2) delete fFitFunc2;

        // (re-)create second overview histogram
        fOverviewHisto2 = new TH1F("Fit Overview", ";Element (sorted);Phi angle [deg]", fNelem, 0, fNelem);
        fOverviewHisto2->SetMarkerStyle(2);
        fOverviewHisto2->SetMarkerColor(4);
        
        // sort values
        TMath::Sort(fNelem, phi, elem_sorted, kFALSE);

        // fill sorted histogram
        for (Int_t i = 0; i < fNelem; i++)
        {
            fOverviewHisto2->SetBinContent(i + 1, phi[elem_sorted[i]]);
            fOverviewHisto2->SetBinError(i + 1, 0.0000001);
        }

        // create fitting function
        fFitFunc2 = new TF1("PhiFit", "pol1", -1, 25);
	fFitFunc2->SetLineColor(2);
        fFitFunc2->SetParameters(1, 1);
	
        // fit histogram
        fOverviewHisto2->Fit(fFitFunc2, "RBQ0");

        // calculate final phi angles
        for (Int_t i = 0; i < fNelem; i++)
        {
            // get true element number
            Int_t elem_true = elem_sorted[i];
            
            // get phi angle
            Double_t phi_fit = fFitFunc2->Eval(fOverviewHisto2->GetBinCenter(i+1));
        
            // map fitted values to interval [-180,180]
            if (phi_fit > 180) fNewVal[elem_true] = phi_fit - 360.;
            else if (phi_fit < -180) fNewVal[elem_true] = phi_fit + 360.;
            else fNewVal[elem_true] = phi_fit;
        }

        // print out overview
        printf("\nFinal result after global fit:\n");
        PrintValues();

        // (re-)create second result canvas
        fCanvasResult2 = new TCanvas("Fit Result", "Fit Result", 630, 0, 900, 400);
        
        // draw fitted histogram
        TCUtils::FormatHistogram(fOverviewHisto2, "PID.Phi.Histo.Overview");
        fOverviewHisto2->Draw("P");
        fFitFunc2->Draw("same");
    }
}   

//______________________________________________________________________________
TH1* TCCalibPIDPhi::GetMappedHistogram(TH1* histo)
{
    // Returns the angle mapped ([-180,180] -> [0,360]) version of the
    // histogram 'histo'. 
    
    Char_t name[256];
    
    // create new histogram
    sprintf(name, "%s_mapped", histo->GetName());
    TH1* hNew = new TH1F(name, name, histo->GetNbinsX(), 
                         histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
    
    // find bins
    Int_t bm180 = histo->GetXaxis()->FindBin(-180.);
    Int_t b0 = histo->GetXaxis()->FindBin(0.);
    Int_t b180 = histo->GetXaxis()->FindBin(180.);
    
    // map bin content
    Int_t nBins = histo->GetNbinsX();
    for (Int_t i = 0; i <= nBins; i++)
    { 
        if (i >= bm180 && i < b0) hNew->SetBinContent(i + b180 - bm180, histo->GetBinContent(i));
        if (i >= b0 && i < b180) hNew->SetBinContent(i, histo->GetBinContent(i));
    }
    
    return hNew;
}

//______________________________________________________________________________
void TCCalibPIDPhi::Write()
{
    // Overwrite this method of the parent class to save also the second
    // overview canvas.
    
    // call parent's method
    TCCalib::Write();

    // save overview picture
    SaveCanvas(fCanvasResult2, "Overview2");
}
ClassImp(TCCalibPIDPhi)
