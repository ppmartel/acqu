// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDEnergyTrad                                                 //
//                                                                      //
// Calibration module for the PID energy (traditional method).          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDEnergyTrad.h"



//______________________________________________________________________________
TCCalibPIDEnergyTrad::TCCalibPIDEnergyTrad()
    : TCCalib("PID.Energy.Trad", "PID energy calibration (traditional)", "Data.PID.E1", TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPed = 0;
    fGain = 0;
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fPionPos = 0;
    fProtonPos = 0;
    fLine = 0;
    fLine2 = 0;
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;
}

//______________________________________________________________________________
TCCalibPIDEnergyTrad::~TCCalibPIDEnergyTrad()
{
    // Destructor. 
 
    if (fFileManager) delete fFileManager;
    if (fPed) delete [] fPed;
    if (fGain) delete [] fGain;
    if (fPionPos) delete fPionPos;
    if (fProtonPos) delete fProtonPos;
    if (fLine) delete fLine;
    if (fLine2) delete fLine2;
    if (fMCHisto) delete fMCHisto;
    if (fMCFile) delete fMCFile;
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fPed = new Double_t[fNelem];
    fGain = new Double_t[fNelem];
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fLine =  new TLine();
    fLine2 =  new TLine();
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;

    // configure line
    fLine->SetLineColor(kBlue);
    fLine->SetLineWidth(3);
    fLine2->SetLineColor(kGreen);
    fLine2->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.Fit.Name");
    
    // get MC histogram file
    TString fileMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.MC.File"))
    {
        Error("Init", "MC file name was not found in configuration!");
        return;
    }
    else fileMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.MC.File");
    
    // get MC histogram name
    TString histoMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.MC.Name"))
    {
        Error("Init", "MC histogram name was not found in configuration!");
        return;
    }
    else histoMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.MC.Name");

    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("PID.Energy.Trad.Fit.Delay");

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
    
    // create the pion position overview histogram
    fPionPos = new TH1F("Pion position overview", ";Element;pion peak position [MeV]", fNelem, 0, fNelem);
    fPionPos->SetMarkerStyle(2);
    fPionPos->SetMarkerColor(4);
    
    // create the proton position overview histogram
    fProtonPos = new TH1F("Proton position overview", ";Element;proton peak position [MeV]", fNelem, 0, fNelem);
    fProtonPos->SetMarkerStyle(2);
    fProtonPos->SetMarkerColor(4);
    
    // draw the overview histograms
    fCanvasResult->Divide(1, 2, 0.001, 0.001);
    fCanvasResult->cd(1);
    fPionPos->Draw("P");
    fCanvasResult->cd(2);
    fProtonPos->Draw("P");
 
    // user information
    Info("Init", "Fitting MC data");

    // perform fitting for the MC histogram
    TCUtils::FormatHistogram(fMCHisto, "PID.Energy.Trad.Histo.Fit");
    FitSlice(fMCHisto);
    fCanvasFit->Update();
    gSystem->Sleep(5000);

    // user information
    Info("Init", "Fitting of MC data finished");
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::FitSlice(TH2* h)
{
    // Fit the energy slice of the dE vs E histogram 'h'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Energy.Trad.Fit.Range", &lowLimit, &highLimit);
    Int_t firstBin = h->GetXaxis()->FindBin(lowLimit);
    Int_t lastBin = h->GetXaxis()->FindBin(highLimit);
     
    // create projection
    sprintf(tmp, "%s_Proj", h->GetName());
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");
    
    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.05);
    fPionData = TMath::MinElement(s.GetNPeaks(), s.GetPositionX());
    fProtonData = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
    
    // create fitting function
    if (fFitFunc) delete fFitFunc;
    sprintf(tmp, "fFunc_%s", h->GetName());
    fFitFunc = new TF1(tmp, "expo(0)+landau(2)+gaus(5)", 0.2*fPionData, fProtonData+5);
    fFitFunc->SetLineColor(2);
    
    // prepare fitting function
    fFitFunc->SetParameters(9.25568, -3.76050e-01, 
                            5e+03, fPionData, 2.62472e-01, 
                            6e+03, fProtonData, 5.82477);
    fFitFunc->SetParLimits(2, 0, 1e6);
    fFitFunc->SetParLimits(3, 0.85*fPionData, 1.15*fPionData);
    fFitFunc->SetParLimits(6, 0.85*fProtonData, 1.15*fProtonData);
    fFitFunc->SetParLimits(5, 0, 1e5);
    fFitFunc->SetParLimits(4, 0.1, 5);
    fFitFunc->SetParLimits(7, 0.1, 10);
    
    // fit
    for (Int_t i = 0; i < 10; i++)
        if (!fFitHisto->Fit(fFitFunc, "RB0Q")) break;
    
    // reset range for second fit
    Double_t start;
    if (h == fMCHisto) start = 0.05;
    else start = fFitFunc->GetParameter(3) - 2.5*fFitFunc->GetParameter(4);
    fFitFunc->SetRange(start, fFitFunc->GetParameter(6) + 4*fFitFunc->GetParameter(7));
    
    // second fit
    for (Int_t i = 0; i < 10; i++)
        if (!fFitHisto->Fit(fFitFunc, "RB0Q")) break;
 
    // get positions
    fPionData = fFitFunc->GetParameter(3);
    fProtonData = fFitFunc->GetParameter(6);
    
    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(fPionData);
    fLine->SetX2(fPionData);
    
    // format line
    fLine2->SetY1(0);
    fLine2->SetY2(fFitHisto->GetMaximum() + 20);
    fLine2->SetX1(fProtonData);
    fLine2->SetX2(fProtonData);
           
    // plot histogram and line
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(0, fFitFunc->GetParameter(6) + 4*fFitFunc->GetParameter(7));
    fFitHisto->Draw("hist");
    fFitFunc->Draw("same");
    fLine->Draw();
    fLine2->Draw();
    
    // save MC data
    if (h == fMCHisto)
    {
        fPionMC = fPionData;
        fProtonMC = fProtonData;
    }

    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Fit(Int_t elem)
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
    TCUtils::FormatHistogram(fMainHisto, "PID.Energy.Trad.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {   
        // fit the energy slices
        FitSlice((TH2*)fMainHisto);
    } 
    
    // update overview
    fCanvasResult->cd(1);
    fPionPos->Draw("E1");
    fCanvasResult->cd(2);
    fProtonPos->Draw("E1");
    fCanvasResult->Update();
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t noval = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPionData) fPionData = fLine->GetX1();
        if (fLine2->GetX1() != fProtonData) fProtonData = fLine2->GetX1();
        
        // calculate peak differences
        Double_t diffMC = fProtonMC - fPionMC;
        Double_t diffData = fProtonData - fPionData;

        // calculate pedestal
        fPed[elem] = 100 * (fPionMC*fProtonData - fProtonMC*fPionData) / -diffMC;

        // calculate gain
        fGain[elem] = 0.01 * diffMC / diffData;
    
        // update overview histograms
        fPionPos->SetBinContent(elem+1, fPionData);
        fPionPos->SetBinError(elem+1, 0.0000001);
        fProtonPos->SetBinContent(elem+1, fProtonData);
        fProtonPos->SetBinError(elem+1, 0.0000001);
    }
    else
    {
        fPed[elem] = 0.;
        fGain[elem] = 0.001;
        noval = kTRUE;
    }

    // user information
    printf("Element: %03d    Pion: %12.8f    Proton: %12.8f    Pedestal: %12.8f    Gain: %12.8f",
           elem, fPionData, fProtonData, fPed[elem], fGain[elem]);
    if (noval) printf("    -> no fit");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::PrintValues()
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
void TCCalibPIDEnergyTrad::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters("Data.PID.E0", fCalibration.Data(), fSet[i], fPed, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.PID.E1", fCalibration.Data(), fSet[i], fGain, fNelem);
    }
    
    // save overview canvas
    SaveCanvas(fCanvasResult, "Overview");
}
ClassImp(TCCalibPIDEnergyTrad)
