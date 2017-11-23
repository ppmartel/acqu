// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibVetoEnergy                                                    //
//                                                                      //
// Calibration module for the Veto energy.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibVetoEnergy.h"



//______________________________________________________________________________
TCCalibVetoEnergy::TCCalibVetoEnergy()
    : TCCalib("Veto.Energy", "Veto energy calibration", "Data.Veto.E1",
              TCReadConfig::GetReader()->GetConfigInt("Veto.Elements"))
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPeak = 0;
    fPeakMC = 0;
    fLine = 0;
    fMCHisto = 0;
    fMCFile = 0;
}

//______________________________________________________________________________
TCCalibVetoEnergy::~TCCalibVetoEnergy()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fLine) delete fLine;
    if (fMCHisto) delete fMCHisto;
    if (fMCFile) delete fMCFile;
}

//______________________________________________________________________________
void TCCalibVetoEnergy::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fPeak = 0;
    fPeakMC = 0;
    fLine =  new TLine();
    fMCHisto = 0;
    fMCFile = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("Veto.Energy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("Veto.Energy.Histo.Fit.Name");
    
    // get MC histogram file
    TString fileMC;
    if (!TCReadConfig::GetReader()->GetConfig("Veto.Energy.MC.File"))
    {
        Error("Init", "MC file name was not found in configuration!");
        return;
    }
    else fileMC = *TCReadConfig::GetReader()->GetConfig("Veto.Energy.MC.File");
    
    // get MC histogram name
    TString histoMC;
    if (!TCReadConfig::GetReader()->GetConfig("Veto.Energy.Histo.MC.Name"))
    {
        Error("Init", "MC histogram name was not found in configuration!");
        return;
    }
    else histoMC = *TCReadConfig::GetReader()->GetConfig("Veto.Energy.Histo.MC.Name");

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;proton peak position [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // open the MC file
    fMCFile = new TFile(fileMC.Data());
    if (!fMCFile)
    {
        Error("Init", "Could not open MC file!");
        return;
    }
    if (fMCFile->IsZombie())
    {
        Error("Init", "Could not open MC file!");
        return;
    }

    // load the MC histogram
    fMCHisto = (TH2*) fMCFile->Get(histoMC.Data());
    if (!fMCHisto)
    {
        Error("Init", "Could not open MC histogram!");
        return;
    }
    
    // draw main histogram
    fCanvasFit->cd(1);
    fMCHisto->Draw("colz");
    
    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "Veto.Energy.Histo.Overview");
    fOverviewHisto->Draw("P");
    
    // user information
    Info("Init", "Fitting MC data");

    // perform fitting for the MC histogram
    FitSlice(fMCHisto, -1);
    fCanvasFit->Update();
    gSystem->Sleep(1000);

    // user information
    Info("Init", "Fitting of MC data finished");
}

//______________________________________________________________________________
void TCCalibVetoEnergy::FitSlice(TH2* h, Int_t elem)
{
    // Fit the energy slice of the dE vs E histogram 'h' of the element 'elem'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("Veto.Energy.Fit.Range", &lowLimit, &highLimit);
    
    // create projection
    sprintf(tmp, "Proj_%d", elem);
    Int_t firstBin = h->GetXaxis()->FindBin(lowLimit);
    Int_t lastBin = h->GetXaxis()->FindBin(highLimit);
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");
    if (h != fMCHisto) TCUtils::FormatHistogram(fFitHisto, "Veto.Energy.Histo.Fit");
        
    // create fitting function
    if (fFitFunc) delete fFitFunc;
    sprintf(tmp, "fFunc_%d", elem);
    fFitFunc = new TF1(tmp, "expo(0)+gaus(2)");
    fFitFunc->SetLineColor(2);
        
    // estimate peak position
    TSpectrum s;
    if (h == fMCHisto) s.Search(fFitHisto, 5, "goff nobackground", 0.03);
    else s.Search(fFitHisto, 5, "goff nobackground", 0.05);
    fPeak = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
        
    // prepare fitting function
    Double_t range = 30./lowLimit+0.3;
    Double_t peak_range = 0.2;
    fFitFunc->SetRange(fPeak - range, fPeak + range*2);
    fFitFunc->SetParameter(2, fFitHisto->GetXaxis()->FindBin(fPeak));
    fFitFunc->SetParLimits(2, 0, 100000);
    fFitFunc->SetParameter(3, fPeak);
    fFitFunc->SetParLimits(3, fPeak - peak_range, fPeak + peak_range);
    fFitFunc->SetParameter(4, 1);
    fFitFunc->SetParLimits(4, 0.1, 10);
     
    // perform first fit
    fFitHisto->Fit(fFitFunc, "RB0Q");

    // adjust fitting range
    Double_t sigma = fFitFunc->GetParameter(4);
    fFitFunc->SetRange(fPeak - 3*sigma, fPeak + range+3*sigma);

    // perform second fit
    fFitHisto->Fit(fFitFunc, "RB0Q");
    
    // get peak
    fPeak = fFitFunc->GetParameter(3);

    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(fPeak);
    fLine->SetX2(fPeak);
    
    // save peak position
    if (h == fMCHisto) fPeakMC = fPeak;

    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(fPeak*0.4, fPeak*1.6);
    fFitHisto->Draw("hist");
    fFitFunc->Draw("same");
    fLine->Draw();
    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibVetoEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
   
    // delete old histogram
    if (fMainHisto) delete fMainHisto;

    // get histogram
    fMainHisto = (TH2*) fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // draw main histogram
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "Veto.Energy.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {   
        // fit the energy slice
        FitSlice((TH2*)fMainHisto, elem);
        
        // update overview
        if (elem % 20 == 0)
        {
            fCanvasResult->cd();
            fOverviewHisto->Draw("E1");
            fCanvasResult->Update();
        }   

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibVetoEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPeak) fPeak = fLine->GetX1();
        
        // calculate the new gain
        fNewVal[elem] = fOldVal[elem] * (fPeakMC / fPeak);
    
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fPeak);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    peak: %12.8f    "
           "old gain: %12.8f    new gain: %12.8f    diff: %6.2f %%",
           elem, fPeak, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   
ClassImp(TCCalibVetoEnergy)
