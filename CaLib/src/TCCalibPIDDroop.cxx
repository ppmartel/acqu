// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDDroop                                                      //
//                                                                      //
// Calibration module for the PID droop correction.                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDDroop.h"



//______________________________________________________________________________
TCCalibPIDDroop::TCCalibPIDDroop()
    : TCCalib("PID.Droop", "PID droop correction", "Data.PID.E0", TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fOutFile = 0;
    fFileManager = 0;
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fNpoint = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = 0;
    fDelay = 0;
}

//______________________________________________________________________________
TCCalibPIDDroop::~TCCalibPIDDroop()
{
    // Destructor. 
    
    // close the output file
    if (fOutFile)
    {
        Info("Calculate", "Closing output file '%s'", fOutFile->GetName());
        delete fOutFile;
    }

    if (fFileManager) delete fFileManager;
    if (fProj2D) delete fProj2D;
    if (fLinPlot) delete fLinPlot;
    if (fPeak) delete [] fPeak;
    if (fTheta) delete [] fTheta;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibPIDDroop::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fNpoint = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = new TLine();
    fDelay = 0;
    
    // try to open the output file
    sprintf(tmp, "PID_Droop_Corr_%s.root", fCalibration.Data());
    fOutFile = new TFile(tmp, "create");
    if (fOutFile->IsZombie())
    {
        Error("Init", "Could not create output file '%s'!", tmp);
        return;
    }
    else
    {
        Info("Init", "Opening output file '%s'", tmp);
    }

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
    
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Droop.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Droop.Histo.Fit.Name");
    
    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("PID.Droop.Fit.Delay");

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
}

//______________________________________________________________________________
Bool_t TCCalibPIDDroop::FitHisto(Double_t* outPeak)
{
    // Fit the 'fFitHisto' and write the position of the proton peak to 'outPeak'.
    // Return kTRUE on success, otherwise kFALSE.

    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.01);
    Double_t peakPion = TMath::MinElement(s.GetNPeaks(), s.GetPositionX());
    Double_t peak = 0;
    Double_t diff = 1e3;
    for (Int_t i = 0; i < s.GetNPeaks(); i++)
    {
        if (TMath::Abs(s.GetPositionX()[i] - 3) < diff)
        {
            diff = TMath::Abs(s.GetPositionX()[i] - 3);
            peak = s.GetPositionX()[i];
        }
    }
    
    // create fitting function
    if (fFitFunc) delete fFitFunc;
    fFitFunc = new TF1("Fitfunc", "expo(0)+landau(2)+gaus(5)", 0.1*peakPion, peak+4);
    fFitFunc->SetLineColor(2);
    
    // prepare fitting function
    fFitFunc->SetParameters(9.25568, -3.76050e-01, 
                            5e+03, peakPion, 2.62472e-01, 
                            6e+03, peak, 0.4);
    fFitFunc->SetParLimits(2, 0, 1e6);
    fFitFunc->SetParLimits(3, 0.9*peakPion, 1.1*peakPion);
    fFitFunc->SetParLimits(6, 2, 5);
    fFitFunc->SetParLimits(5, 0, 1e5);
    fFitFunc->SetParLimits(4, 0.1, 1);
    fFitFunc->SetParLimits(7, 0.3, 5);
  
    // perform first fit
    Int_t fitRes;
    for (Int_t i = 0; i < 20; i++)
        if (!(fitRes = fFitHisto->Fit(fFitFunc, "RB0Q"))) break;

    // save peak
    if (outPeak) *outPeak = fFitFunc->GetParameter(6);

    // reject bad fits
    if (fFitFunc->GetParameter(7) / fFitFunc->GetParameter(5) > 1 || fitRes) 
        return kFALSE;

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibPIDDroop::FitSlices(TH3* h, Int_t elem)
{
    // Fit the theta slices of the dE vs E histogram 'h'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    Double_t lowEnergy, highEnergy;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Droop.Fit.Range", &lowLimit, &highLimit);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Droop.Fit.Energy.Range", &lowEnergy, &highEnergy);
    Double_t interval = TCReadConfig::GetReader()->GetConfigDouble("PID.Droop.Fit.Interval");
    
    // count points
    fNpeak = (highLimit - lowLimit) / interval;
    fNpoint = 0;

    // prepare arrays
    if (!fPeak) fPeak = new Double_t[fNpeak];
    if (!fTheta) fTheta = new Double_t[fNpeak];
    
    //
    // get global proton position
    //
    
    // create 2D projection
    if (fProj2D) delete fProj2D;
    fProj2D = (TH2D*) h->Project3D("Proj2DTot_yxe");
    sprintf(tmp, "%02d total", elem);
    fProj2D->SetTitle(tmp);

    // create 1D projection
    if (fFitHisto) delete fFitHisto;
    Int_t firstBin = fProj2D->GetXaxis()->FindBin(lowEnergy);
    Int_t lastBin = fProj2D->GetXaxis()->FindBin(highEnergy);
    fFitHisto = (TH1D*) fProj2D->ProjectionY("ProjTot", firstBin, lastBin, "e");
    sprintf(tmp, "%02d total %.f < E < %.f", elem, lowEnergy, highEnergy);
    fFitHisto->SetTitle(tmp);
        
    // fit histo
    Double_t peakTotal;
    FitHisto(&peakTotal);
    
    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(peakTotal);
    fLine->SetX2(peakTotal);
  
    // plot projection fit  
    if (fDelay > 0)
    {
        TCUtils::FormatHistogram(fProj2D, "PID.Droop.Histo.Fit");
        fCanvasFit->cd(1);
        fProj2D->Draw("colz");
        fFitHisto->GetXaxis()->SetRangeUser(0, peakTotal+3);
        fCanvasFit->cd(2);
        fFitHisto->Draw("hist");
        fFitFunc->Draw("same");
        fLine->Draw();
        fCanvasFit->Update();
        gSystem->Sleep(2000);
    }


    //
    // loop over theta slices
    //
    Double_t start = lowLimit;
    while (start < highLimit)
    {
        // set theta axis range
        h->GetZaxis()->SetRangeUser(start, start + interval);
        
        // create 2D projection
        if (fProj2D) delete fProj2D;
        sprintf(tmp, "Proj2D_%d_yxe", (Int_t)start);
        fProj2D = (TH2D*) h->Project3D(tmp);
        sprintf(tmp, "%02d dE vs E : %.f < #theta < %.f", elem, start, start + interval);
        fProj2D->SetTitle(tmp);

        // create 1D projection
        if (fFitHisto) delete fFitHisto;
        sprintf(tmp, "Proj_%d", (Int_t)start);
        Int_t firstBin = fProj2D->GetXaxis()->FindBin(lowEnergy);
        Int_t lastBin = fProj2D->GetXaxis()->FindBin(highEnergy);
        fFitHisto = (TH1D*) fProj2D->ProjectionY(tmp, firstBin, lastBin, "e");
        sprintf(tmp, "%02d dE : %.f < #theta < %.f, %.f < E < %.f", elem, start, start + interval,
                                                             lowEnergy, highEnergy);
        fFitHisto->SetTitle(tmp);
            
        // fit histo
        Double_t peak;
        Bool_t fitRes = FitHisto(&peak);
        
        // on success
        if (fitRes)
        {
            // format line
            fLine->SetY1(0);
            fLine->SetY2(fFitHisto->GetMaximum() + 20);
            fLine->SetX1(peak);
            fLine->SetX2(peak);
            
            // save peak and theta position
            fPeak[fNpoint] = peak / peakTotal;
            fTheta[fNpoint] = start + interval / 2.;
            
            // count point
            fNpoint++;
        }
        
        // plot projection fit  
        if (fDelay > 0)
        {
            TCUtils::FormatHistogram(fProj2D, "PID.Droop.Histo.Fit");
            fCanvasFit->cd(1);
            fProj2D->Draw("colz");
            fFitHisto->GetXaxis()->SetRangeUser(0, peak+4);
            fCanvasFit->cd(2);
            fFitHisto->Draw("hist");
            if (fitRes) 
            {
                fFitFunc->Draw("same");
                fLine->Draw();
            }
            fCanvasFit->Update();
            gSystem->Sleep(fDelay);
        }
       
        // increment loop variables
        start += interval;
     
     } // while: loop over energy slices
}

//______________________________________________________________________________
void TCCalibPIDDroop::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
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
    
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {   
        // fit the theta slices
        FitSlices((TH3*)fMainHisto, elem);

        // create linear plot
        if (fLinPlot) delete fLinPlot;
        fLinPlot = new TGraph(fNpoint, fTheta, fPeak);
        sprintf(tmp, "Droop_Corr_%02d", elem);
        fLinPlot->SetName(tmp);
        fLinPlot->SetTitle(tmp);
        fLinPlot->GetXaxis()->SetTitle("CB Cluster theta angle [deg]");
        fLinPlot->GetYaxis()->SetTitle("#theta bin proton peak / total proton peak pos.");
        fLinPlot->SetMarkerStyle(20);
        fLinPlot->SetMarkerSize(1);
        fLinPlot->SetMarkerColor(kBlue);
        
        // plot linear plot
        fCanvasResult->cd();
        fLinPlot->Draw("ap");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibPIDDroop::Calculate(Int_t elem)
{
    // Nothing to do here.
    
}

//______________________________________________________________________________
void TCCalibPIDDroop::PrintValues()
{
    // Disable this method.

    Info("PrintValues", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibPIDDroop::Write()
{
    // Disable this method.
    
    // check if output file exits
    if (!fOutFile)
    {
        Error("Write", "Cannot save droop correction to output file!");
        return;
    }
    
    // save TGraph to output file
    fOutFile->cd();
    fLinPlot->Write();
    Info("Write", "Droop correction '%s' was written to '%s'", fLinPlot->GetName(), fOutFile->GetName());
}
ClassImp(TCCalibPIDDroop)
