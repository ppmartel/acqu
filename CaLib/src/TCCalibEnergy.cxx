// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibEnergy                                                        //
//                                                                      //
// Base energy calibration module class.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibEnergy.h"



//______________________________________________________________________________
TCCalibEnergy::TCCalibEnergy(const Char_t* name, const Char_t* title, const Char_t* data,
                             Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.
    
    // init members
    fPi0Pos = 0;
    fLine = 0;
    fDetectorView = 0;
}

//______________________________________________________________________________
TCCalibEnergy::~TCCalibEnergy()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibEnergy::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members

    fPi0Pos = 0;
    fLine = new TIndicatorLine();
    
    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
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
    fOverviewHisto = new TH1F("Overview", ";Element;2#gamma inv. mass [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fMainHisto, tmp);
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fFitHisto, tmp);
    fFitHisto->GetYaxis()->SetRangeUser(0.0, fFitHisto->GetMaximum()*1.05);
    fFitHisto->Draw("hist");
     
    // check for sufficient statistics
    if (fFitHisto->GetEntries() > 500)
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fEnergy_%i", elem);
        fFitFunc = new TF1(tmp, "gaus(0)+pol3(3)");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        fPi0Pos = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (fPi0Pos < 100 || fPi0Pos > 160) fPi0Pos = 135;

        // configure fitting function
        initFitFunction();

        // fit
        for (Int_t i = 0; i < 10; i++)
            if (!fFitHisto->Fit(fFitFunc, "RBQ0")) break;

        // final results
        fPi0Pos = fFitFunc->GetParameter(1); 
        
        // check if mass is in normal range
        if (fPi0Pos < 80 || fPi0Pos > 200) fPi0Pos = 135;
 
        // set indicator line
        fLine->SetY1(0);
        fLine->SetupY(0, fFitHisto->GetMaximum()*1.05);
        fLine->SetX1(fPi0Pos);
        fLine->SetX2(fPi0Pos);
   
        // draw fitting function
        if (fFitFunc) {
            fFitFunc->Draw("same");

            delete fFitPeak;
            fFitPeak = new TF1("peak", "gaus(0)", fFitFunc->GetXmin(), fFitFunc->GetXmax());
            fFitPeak->SetLineColor(kGreen);

            // copy parameters from fit
            for(int i=0;i<fFitPeak->GetNpar(); ++i) {
                fFitPeak->SetParameter(i, fFitFunc->GetParameter(i));
            }

            // background = fit - peak
            delete fFitBackGround;
            fFitBackGround = new TF1("bg",Form("%s-peak",fFitFunc->GetName()), fFitFunc->GetXmin(), fFitFunc->GetXmax());
            fFitBackGround->SetLineColor(kBlue);

            fFitPeak->Draw("same");
            fFitBackGround->Draw("same");

            //
            //double peak_integral = fFitPeak->Integral(fFitPeak->GetXmin(), fFitPeak->GetXmax(), nullptr);
            //double bg_integral   = fFitBackGround->Integral(fFitBackGround->GetXmin(), fFitBackGround->GetXmax(), nullptr);
            //double peak_ratio    = peak_integral / bg_integral;
        }
    
        // draw indicator line
        fLine->Draw();
    }

    double xmin,xmax;
    fFitFunc->GetRange(xmin,xmax);
    double maximum = fFitFunc->GetMaximumX();
    fFitOk=false;
    if(maximum < xmax && maximum > xmin) {
        puts("Fit OK!\n");
        fFitOk=true;
    } else {
        puts("No Maximum found in fit!");
    }

    if(fDetectorView) {
        fDetectorView->SetElement(elem, vCurrPos);
        fExtraCanvas->cd();
        fDetectorView->Draw("col");
        fExtraCanvas->Update();
    }

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
void TCCalibEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries() > 500)
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPi0Pos) fPi0Pos = fLine->GetX1();
        
        // calculate the new gain
        // apply fConvergenceFactor only to the desired procentual change of fOldVal,
        // given by (TCConfig::kPi0Mass / fPi0Pos - 1)
        fNewVal[elem] = fOldVal[elem] + fOldVal[elem] * fConvergenceFactor * (TCConfig::kPi0Mass / fPi0Pos - 1);
        
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fPi0Pos);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    
        // update average calculation
        fAvr += fPi0Pos;
        fAvrDiff += TMath::Abs(fPi0Pos - TCConfig::kPi0Mass);
        fNcalc++;
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Pi0: %12.8f    "
           "old gain: %12.8f    new gain: %12.8f    diff: %6.2f %%",
           elem, fPi0Pos, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    if (this->InheritsFrom("TCCalibCBEnergy"))
    {
        if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    }
    printf("\n");

    // show average
    if (elem == fNelem-1)
    {
        fAvr /= (Double_t)fNcalc;
        fAvrDiff /= (Double_t)fNcalc;
        printf("Average pi0 position           : %.3f MeV\n", fAvr);
        printf("Average difference to pi0 mass : %.3f MeV\n", fAvrDiff);
    }

    if(fDetectorView) {
        fDetectorView->SetElement(elem, fFitOk ? vFitOK : vFitFailed);
        fExtraCanvas->cd();
        fDetectorView->Draw("col");
        fExtraCanvas->Update();
    }
}   


void TCCalibCBEnergy::initFitFunction()
{
    fFitFunc->SetRange(fPi0Pos - 50, fPi0Pos + 80);
    fFitFunc->SetParameters(fFitHisto->GetMaximum(), fPi0Pos, 8, 1, 1, 1, 0.1);
    fFitFunc->SetParLimits(1, 130, 140);
    fFitFunc->SetParLimits(2, 3, 15);
}

void TCCalibTAPSEnergyLG::initFitFunction()
{
    fFitFunc->SetRange(80, 250);
    fFitFunc->SetParameters(fFitHisto->GetMaximum(), fPi0Pos, 10, 1, 1, 1, 0.1);
    fFitFunc->SetParLimits(0, 1, fFitHisto->GetMaximum());
    fFitFunc->SetParLimits(1, 115, 140);
    fFitFunc->SetParLimits(2, 5, 50);
    fFitFunc->FixParameter(6, 0);
}

ClassImp(TCCalibEnergy)
