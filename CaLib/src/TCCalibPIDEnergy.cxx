// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDEnergy                                                     //
//                                                                      //
// Calibration module for the PID energy.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDEnergy.h"



//______________________________________________________________________________
TCCalibPIDEnergy::TCCalibPIDEnergy()
    : TCCalib("PID.Energy", "PID energy calibration", "Data.PID.E0", TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPed = 0;
    fGain = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fPeak = 0;
    fPeak_Err = 0;
    fPeakMC = 0;
    fPeakMC_Err = 0;
    fLine = 0;
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;
}

//______________________________________________________________________________
TCCalibPIDEnergy::~TCCalibPIDEnergy()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fPed) delete [] fPed;
    if (fGain) delete [] fGain;
    if (fLinPlot) delete fLinPlot;
    if (fPeak) delete [] fPeak;
    if (fPeak_Err) delete [] fPeak_Err;
    if (fPeakMC) delete [] fPeakMC;
    if (fPeakMC_Err) delete [] fPeakMC_Err;
    if (fLine) delete fLine;
    if (fMCHisto) delete fMCHisto;
    if (fMCFile) delete fMCFile;
}

//______________________________________________________________________________
void TCCalibPIDEnergy::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fPed = new Double_t[fNelem];
    fGain = new Double_t[fNelem];
    fLinPlot = 0;
    fNpeak = 0;
    fPeak = 0;
    fPeak_Err = 0;
    fPeakMC = 0;
    fPeakMC_Err = 0;
    fLine =  new TLine();
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Histo.Fit.Name");
    
    // get MC histogram file
    TString fileMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.MC.File"))
    {
        Error("Init", "MC file name was not found in configuration!");
        return;
    }
    else fileMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.MC.File");
    
    // get MC histogram name
    TString histoMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Histo.MC.Name"))
    {
        Error("Init", "MC histogram name was not found in configuration!");
        return;
    }
    else histoMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Histo.MC.Name");

    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("PID.Energy.Fit.Delay");

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters("Data.PID.E0", fCalibration.Data(), fSet[0], fPed, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.PID.E1", fCalibration.Data(), fSet[0], fGain, fNelem);

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
    
    // user information
    Info("Init", "Fitting MC data");

    // perform fitting for the MC histogram
    FitSlices(fMCHisto);
    fCanvasFit->Update();
    printf("MC proton peak positions: ");
    for (Int_t i = 0; i < fNpeak; i++) printf("%.2f  ", fPeakMC[i]);
    printf("\n");

    // user information
    Info("Init", "Fitting of MC data finished");
}

//______________________________________________________________________________
void TCCalibPIDEnergy::FitSlices(TH2* h)
{
    // Fit the energy slices of the dE vs E histogram 'h'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Energy.Fit.Range", &lowLimit, &highLimit);
    Double_t interval = TCReadConfig::GetReader()->GetConfigDouble("PID.Energy.Fit.Interval");
    
    // count points
    fNpeak = (highLimit - lowLimit) / interval;
    
    // prepare arrays
    if (h == fMCHisto) 
    {
        fPeakMC = new Double_t[fNpeak];
        fPeakMC_Err = new Double_t[fNpeak];
    }
    else
    {
        if (!fPeak) 
        {
            fPeak = new Double_t[fNpeak];
            fPeak_Err = new Double_t[fNpeak];
        }
    }
    
    // loop over energy slices
    Double_t start = lowLimit;
    Int_t nfit = 0;
    while (start < highLimit)
    {
        // create projection
        sprintf(tmp, "Proj_%d", (Int_t)start);
        Int_t firstBin = h->GetXaxis()->FindBin(start);
        Int_t lastBin = h->GetXaxis()->FindBin(start + interval);
        if (fFitHisto) delete fFitHisto;
        fFitHisto = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");
        if (h != fMCHisto) TCUtils::FormatHistogram(fFitHisto, "PID.Energy.Histo.Fit");
        
        // create fitting function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fGauss_%d", (Int_t)start);
        fFitFunc = new TF1(tmp, "expo(0)+gaus(2)");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        TSpectrum s(1);
        s.Search(fFitHisto, 10, "goff nobackground", 0.1);
        Double_t peak = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
        
        // prepare fitting function
        Double_t range;
        Double_t peak_range;
        if (h == fMCHisto)
        {
            range = 20./start+0.3;
            peak_range = 0.2;
            fFitFunc->SetRange(peak - 1.5*range, peak + range*2);
            fFitFunc->SetParameter(2, fFitHisto->GetXaxis()->FindBin(peak));
            fFitFunc->SetParLimits(2, 0, 100000);
            fFitFunc->SetParameter(3, peak);
            fFitFunc->SetParLimits(3, peak - peak_range, peak + peak_range);
            fFitFunc->SetParameter(4, 1);
            fFitFunc->SetParLimits(4, 0.1, 10);
             
            // perform fit
            fFitHisto->Fit(fFitFunc, "RB0Q");
        }
        else
        {
            range = 6000/start+40;
            peak_range = 10;
            fFitFunc->SetRange(peak - range, peak + range);
            fFitFunc->SetParameter(2, fFitHisto->GetXaxis()->FindBin(peak));
            fFitFunc->SetParLimits(2, 0, 100000);
            fFitFunc->SetParameter(3, peak);
            fFitFunc->SetParLimits(3, peak - peak_range, peak + peak_range);
            fFitFunc->SetParameter(4, 20);
            fFitFunc->SetParLimits(4, 18, 100);
             
            // perform first fit
            fFitHisto->Fit(fFitFunc, "RB0Q");

            // adjust fitting range
            Double_t sigma = fFitFunc->GetParameter(4);
            fFitFunc->SetRange(peak - 3*sigma, peak + range+3.5*sigma);

            // perform second fit
            for (Int_t i = 0; i < 20; i++)
                if (!fFitHisto->Fit(fFitFunc, "RBQ0") && fFitFunc->GetParError(3) > 0) break;
        }
        
        // get peak
        peak = fFitFunc->GetParameter(3);
        Double_t peak_err = fFitFunc->GetParError(3);
        if (peak_err == 0) peak_err = 1;

        // format line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(peak);
        fLine->SetX2(peak);
        
        // save peak position
        if (h == fMCHisto) 
        {
            fPeakMC[nfit] = peak;
            fPeakMC_Err[nfit] = peak_err;
        }
        else 
        {
            fPeak[nfit] = peak;
            fPeak_Err[nfit] = peak_err;
        }

        // plot projection fit  
        if (fDelay > 0)
        {
            fCanvasFit->cd(2);
            fFitHisto->GetXaxis()->SetRangeUser(peak*0.4, peak*1.6);
            fFitHisto->Draw("hist");
            fFitFunc->Draw("same");
            fLine->Draw();
            fCanvasFit->Update();
            gSystem->Sleep(fDelay);
        }

        // increment loop variables
        start += interval;
        nfit++;
        
    } // while: loop over energy slices
}

//______________________________________________________________________________
void TCCalibPIDEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
   
    // get histogram
    TH3* h3 = (TH3*) fFileManager->GetHistogram(tmp);
    if (!h3)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create 2D projection
    if (fMainHisto) delete fMainHisto;
    sprintf(tmp, "%02d_yxe", elem);
    fMainHisto = (TH2D*) h3->Project3D(tmp);
    fMainHisto->SetTitle(tmp);
    delete h3;
 
    // draw main histogram
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "PID.Energy.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {   
        // fit the energy slices
        FitSlices((TH2*)fMainHisto);

        // create linear plot
        if (fLinPlot) delete fLinPlot;
        fLinPlot = new TGraphErrors(fNpeak, fPeakMC, fPeak, fPeakMC_Err, fPeak_Err);
        sprintf(tmp, "Element %d", elem);
        fLinPlot->SetName(tmp);
        fLinPlot->SetTitle(tmp);
        fLinPlot->GetXaxis()->SetTitle("MC peak position [MeV]");
        fLinPlot->GetYaxis()->SetTitle("Data peak position [Channel]");
        
        // create linear fitting function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "Func_%d", elem);
        fFitFunc = new TF1(tmp, "pol1");
        fFitFunc->SetLineColor(kRed);
        
        // fit linear plot
        fFitFunc->SetRange(0.9*TMath::MinElement(fNpeak, fPeakMC), 
                           1.1*TMath::MaxElement(fNpeak, fPeakMC));
        fLinPlot->Fit(fFitFunc, "RB0Q");

        // plot linear plot
        fCanvasResult->cd();
        fLinPlot->Draw("ap");
        fFitFunc->Draw("same");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibPIDEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t noval = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // calculate pedestal
        fPed[elem] = fFitFunc->GetParameter(0);

        // calculate gain
        fGain[elem] = 1. / fFitFunc->GetParameter(1);
    }
    else
    {
        fPed[elem] = 0.;
        fGain[elem] = 0.001;
        noval = kTRUE;
    }

    // user information
    printf("Element: %03d    Pedestal: %12.8f    Gain: %12.8f",
           elem, fPed[elem], fGain[elem]);
    if (noval) printf("    -> no fit");
    printf("\n");
 
    // save canvas
    Char_t tmp[256];
    sprintf(tmp, "Elem_%d", elem);
    SaveCanvas(fCanvasResult, tmp);
}   

//______________________________________________________________________________
void TCCalibPIDEnergy::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Pedestal: %12.8f    Gain: %12.8f\n",
               i, fPed[i], fGain[i]);
    }
}

//______________________________________________________________________________
void TCCalibPIDEnergy::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters("Data.PID.E0", fCalibration.Data(), fSet[i], fPed, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.PID.E1", fCalibration.Data(), fSet[i], fGain, fNelem);
    }
}
ClassImp(TCCalibPIDEnergy)
