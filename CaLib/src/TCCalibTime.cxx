// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTime                                                          //
//                                                                      //
// Base time calibration module class.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTime.h"


//______________________________________________________________________________
TCCalibTime::TCCalibTime(const Char_t* name, const Char_t* title, const Char_t* data,
                         Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.

    // init members
    fTimeGain = new Double_t[fNelem];
    for (Int_t i = 0; i < fNelem; i++) fTimeGain[i] = 0;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibTime::~TCCalibTime()
{
    // Destructor. 
    
    if (fTimeGain) delete [] fTimeGain;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTime::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fMean = 0;
    fLine = new TIndicatorLine();
    
    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
    // get time gain for TDCs
    if (this->InheritsFrom("TCCalibTAPSTime"))
    {
        // get individual time gain for TAPS TDCs (only from first set)
        TCMySQLManager::GetManager()->ReadParameters("Data.TAPS.T1", fCalibration.Data(), fSet[0], fTimeGain, fNelem);
    }
    else if (this->InheritsFrom("TCCalibVetoTime"))
    {
        // get individual time gain for Veto TDCs (only from first set)
        TCMySQLManager::GetManager()->ReadParameters("Data.Veto.T1", fCalibration.Data(), fSet[0], fTimeGain, fNelem);
    }
    else
    {
        // no time gain for CB rise time
        if (!this->InheritsFrom("TCCalibCBRiseTime"))
        {
            sprintf(tmp, "%s.TDCGain", GetName());
            Double_t tdc_gain;
            if (!TCReadConfig::GetReader()->GetConfig(tmp)) tdc_gain = 0.11771;
            else tdc_gain = TCReadConfig::GetReader()->GetConfigDouble(tmp);
            for (Int_t i = 0; i < fNelem; i++) fTimeGain[i] = tdc_gain;
            Info("Init", "Using a TDC gain of %f ns/channel", tdc_gain);
        }
    }

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
    fOverviewHisto = new TH1F("Overview", ";Element;Time peak position [ns]", fNelem, 0, fNelem);
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
void TCCalibTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // init variables
    Double_t factor = 2.5;
    Double_t range = 3.8;
    
    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fFitHisto, tmp);
    fFitHisto->Draw("hist");
 
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);

	// the fit function
	fFitFunc = new TF1("fFitFunc", "pol1(0)+gaus(2)");
	fFitFunc->SetLineColor(2);
	
	// get important parameter positions
	Double_t fMean = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(1, 0.1, max, fMean, 8);
	fFitFunc->SetParLimits(2, 0.1, max*10);
	fFitFunc->SetParLimits(3, fMean - 2, fMean + 2);
	fFitFunc->SetParLimits(4, 0, 20);                  
    
        // special configuration for certain classes
         if (!this->InheritsFrom("TCCalibTaggerTime") && 
             !this->InheritsFrom("TCCalibTAPSTime")   && 
             !this->InheritsFrom("TCCalibVetoTime")   &&
             !this->InheritsFrom("TCCalibCBRiseTime"))
        {   
            // only gaussian
            fFitFunc->FixParameter(0, 0);
            fFitFunc->FixParameter(1, 0);
        }
        if (this->InheritsFrom("TCCalibTAPSTime"))
        {
	    fFitFunc->SetParLimits(4, 0.001, 10);                  
            range = 10;
            factor = 5;
        }
        if (this->InheritsFrom("TCCalibPIDTime"))
        {
            factor = 1.5;
        }
        if (this->InheritsFrom("TCCalibCBRiseTime"))
        {
            factor = 10;
        }
        if (this->InheritsFrom("TCCalibTaggerTime"))
        {
            range = 5;
            factor = 10;
	    fFitFunc->SetParLimits(4, 0.01, 2);                  
        }

        // first iteration
	fFitFunc->SetRange(fMean - range, fMean + range);
	fFitHisto->Fit(fFitFunc, "RBQ0");
	fMean = fFitFunc->GetParameter(3);

        // second iteration
        Double_t sigma = fFitFunc->GetParameter(4);
        fFitFunc->SetRange(fMean -factor*sigma, fMean +factor*sigma);
        for (Int_t i = 0; i < 10; i++)
            if(!fFitHisto->Fit(fFitFunc, "RBQ0")) break;

        // final results
        fMean = fFitFunc->GetParameter(3); 

        // draw mean indicator line
        fLine->SetupY(0,fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
   
        // draw fitting function
        if (fFitFunc) fFitFunc->Draw("same");
    
        // draw indicator line
        fLine->Draw();
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
void TCCalibTime::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Char_t tmp[256];
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // calculate the new offset
        if (this->InheritsFrom("TCCalibCBRiseTime")) fNewVal[elem] = fOldVal[elem] + fMean;
        else fNewVal[elem] = fOldVal[elem] + fConvergenceFactor * fMean / fTimeGain[elem];
        
        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fMean);
        fOverviewHisto->SetBinError(elem + 1, 0.0000001);
        
        // update average calculation
        fAvr += fMean;
        fAvrDiff += TMath::Abs(fMean);
        fNcalc++;
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    if (this->InheritsFrom("TCCalibCBRiseTime")) strcpy(tmp, "rise time");
    else strcpy(tmp, "offset");

    printf("Element: %03d    Peak: %12.8f    "
           "old %s: %12.8f    new %s: %12.8f    diff: %6.2f %%",
           elem, fMean, tmp, fOldVal[elem], tmp, fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    
    if (this->InheritsFrom("TCCalibCBTime") ||
        this->InheritsFrom("TCCalibCBRiseTime")) 
    {
        if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    }
    printf("\n");

    // show average
    if (elem == fNelem-1)
    {
        fAvr /= (Double_t)fNcalc;
        fAvrDiff /= (Double_t)fNcalc;
        printf("Average center          : %.3f ns\n", fAvr);
        printf("Average difference to 0 : %.3f ns\n", fAvrDiff);
    }
}

double sqr(const double& x) { return x*x; }

double asymGaus(double* xarr, double* p) {
    double& x  = xarr[0];

    double& a  = p[0];
    double& x0 = p[1];
    double& s = (x<x0 ? p[2] : p[3]);

    return a * exp(- sqr(x-x0)/sqr(s) );
}

TF1* makeAsymGaus(const char* name="ag") {
    TF1* f = new TF1(name, asymGaus, -10, 10, 4);
    f->SetParameter(0,1);
    f->SetParameter(1,1);
    f->SetParameter(2,1);
    f->SetParameter(3,4);

    return f;
}

void TCCalibCBTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.

    Char_t tmp[256];

    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");

    // init variables
    //Double_t factor = 2.5;
    Double_t range = 3.8;

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fFitHisto, tmp);
    fFitHisto->Draw("hist");

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // get important parameter positions
        Double_t fMean = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
        Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);

        // the fit function
        TF1* tmpfit = new TF1("tmpfit", asymGaus, fMean - range, fMean + range, 4);
        tmpfit->SetLineColor(2);

        tmpfit->SetParLimits(0, 0, max*10.0);
        tmpfit->SetParameter(0, max);

        tmpfit->SetParameter(1, fMean);
        tmpfit->SetParameter(2, 5);
        tmpfit->SetParameter(3, 5);
        tmpfit->SetParLimits(2, 0, 50);
        tmpfit->SetParLimits(3, 0, 50);

        fFitHisto->Fit(tmpfit, "RBQ0");
        fMean = tmpfit->GetParameter(1);
        double sl = tmpfit->GetParameter(2);
        double sr = tmpfit->GetParameter(3);
        delete tmpfit;

        fFitFunc = new TF1("fFitFunc","pol4");


        // first iteration
        fFitFunc->SetRange(fMean - sl, fMean + sr);
        fFitHisto->Fit(fFitFunc, "RBQ0");
        fMean = fFitFunc->GetMaximumX();

        // draw mean indicator line
        fLine->SetupY(0,fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);

        // draw fitting function
        if (fFitFunc) fFitFunc->Draw("same");

        // draw indicator line
        fLine->Draw();
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

ClassImp(TCCalibTime)
