// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergySG                                                  //
//                                                                      //
// Calibration module for the TAPS SG energy.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSEnergySG.h"



//______________________________________________________________________________
TCCalibTAPSEnergySG::TCCalibTAPSEnergySG()
    : TCCalib("TAPS.Energy.SG", "TAPS SG energy calibration", "Data.TAPS.SG.E1",
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fPedOld = 0;
    fGainOld = 0;
    fPedNew = 0;
    fGainNew = 0;
    fPhi1 = 0;
    fPhi2 = 0;
    fRadius1 = 0;
    fRadius2 = 0;
    fFileManager = 0;
    fLine1 = 0;
    fLine2 = 0;
    fFitFunc2 = 0;
    fFitHisto2 = 0;
    fPhiHisto1 = 0;
    fPhiHisto2 = 0;
}

//______________________________________________________________________________
TCCalibTAPSEnergySG::~TCCalibTAPSEnergySG()
{
    // Destructor. 
    
    if (fPedOld) delete [] fPedOld;
    if (fGainOld) delete [] fGainOld;
    if (fPedNew) delete [] fPedNew;
    if (fGainNew) delete [] fGainNew;
    if (fFileManager) delete fFileManager;
    if (fLine1) delete fLine1;
    if (fLine2) delete fLine2;
    if (fFitFunc2) delete fFitFunc2;
    if (fFitHisto2) delete fFitHisto2;
    if (fPhiHisto1) delete fPhiHisto1;
    if (fPhiHisto2) delete fPhiHisto2;
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Init()
{
    // Init the module.
    
    // init members
    fPedOld = new Double_t[fNelem];
    fGainOld = new Double_t[fNelem];
    fPedNew = new Double_t[fNelem];
    fGainNew = new Double_t[fNelem];
    fPhi1 = 0;
    fPhi2 = 0;
    fRadius1 = 0;
    fRadius2 = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fLine1 = new TLine();
    fLine2 = new TLine();
    fFitFunc2 = 0;
    fFitHisto2 = 0;

    // configure lines
    fLine1->SetLineColor(4);
    fLine1->SetLineWidth(3);
    fLine2->SetLineColor(4);
    fLine2->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.Energy.SG.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.Energy.SG.Histo.Fit.Name");
    
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters("Data.TAPS.SG.E0", fCalibration.Data(), fSet[0], fPedOld, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.TAPS.SG.E1", fCalibration.Data(), fSet[0], fGainOld, fNelem);
    
    // copy old parameters to new ones
    for (Int_t i = 0; i < fNelem; i++)
    {
        fPedNew[i] = fPedOld[i];
        fGainNew[i] = fGainOld[i];
    }

    // create the overview histogram
    fPhiHisto1 = new TH1F("Overview 1", ";Element;photon PSA angle [deg]", fNelem, 0, fNelem);
    fPhiHisto1->SetMarkerStyle(2);
    fPhiHisto1->SetMarkerColor(4);
    fPhiHisto2 = new TH1F("Overview 2", ";Element;photon PSA angle [deg]", fNelem, 0, fNelem);
    fPhiHisto2->SetMarkerStyle(2);
    fPhiHisto2->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 3, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->Divide(1, 2, 0.001, 0.001);
    fCanvasResult->cd(1);
    TCUtils::FormatHistogram(fPhiHisto1, "TAPS.Energy.SG.Histo.Overview");
    fPhiHisto1->Draw("P");
    fCanvasResult->cd(2);
    TCUtils::FormatHistogram(fPhiHisto2, "TAPS.Energy.SG.Histo.Overview");
    fPhiHisto2->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t lowLimit1, highLimit1;
    Double_t lowLimit2, highLimit2;
    Double_t xMin, xMax;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.Energy.SG.Photon.Energy.Range.1", &lowLimit1, &highLimit1);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.Energy.SG.Photon.Energy.Range.2", &lowLimit2, &highLimit2);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.Energy.SG.Histo.Fit.Xaxis.Range", &xMin, &xMax);
     
    // calculate PSA radii
    fRadius1 = 0.5 * (highLimit1 + lowLimit1);
    fRadius2 = 0.5 * (highLimit2 + lowLimit2);

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
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "TAPS.Energy.SG.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
    
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        //
        // low interval
        //

        // create energy projection
        sprintf(tmp, "PSAProj_%d_%.f_%.f", elem, lowLimit1, highLimit1);
        TH2* h2 = (TH2*) fMainHisto;
        if (fFitHisto) delete fFitHisto;
        Int_t binMin = h2->GetYaxis()->FindBin(lowLimit1);
        Int_t binMax = h2->GetYaxis()->FindBin(highLimit1);
        fFitHisto = (TH1D*) h2->ProjectionX(tmp, binMin, binMax, "e");
        TCUtils::FormatHistogram(fFitHisto, "TAPS.Energy.SG.Histo.Fit");
       
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fPSA_1_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);

	// get important parameter positions
	Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(max, maxPos, 4);
	fFitFunc->SetParLimits(0, max - 20, max + 20);
	fFitFunc->SetParLimits(1, maxPos - 2, maxPos + 2);
	fFitFunc->SetParLimits(2, 0.1, 5);

	// estimate peak position
        fPhi1 = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // first iteration
        fFitFunc->SetRange(fPhi1 - 0.8, fPhi1 + 0.8);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), fPhi1, 0.5);
        fFitHisto->Fit(fFitFunc, "RBQ0");
        
        // second iteration
        fPhi1 = fFitFunc->GetParameter(1);
        Double_t sigma = fFitFunc->GetParameter(2);
        fFitFunc->SetRange(fPhi1 - 2*sigma, fPhi1 + 2*sigma);
        fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fPhi1 = fFitFunc->GetParameter(1); 
        
        // correct bad fits
        if (fPhi1 < xMin) fPhi1 = xMin + 1;
        if (fPhi1 > xMax) fPhi1 = xMax - 1;

        // draw mean indicator line
        fLine1->SetY1(0);
        fLine1->SetY2(fFitHisto->GetMaximum() + 20);
        fLine1->SetX1(fPhi1);
        fLine1->SetX2(fPhi1);
        
        //
        // high interval
        //

        // create energy projection
        sprintf(tmp, "PSAProj_%d_%.f_%.f", elem, lowLimit2, highLimit2);
        if (fFitHisto2) delete fFitHisto2;
        binMin = h2->GetYaxis()->FindBin(lowLimit2);
        binMax = h2->GetYaxis()->FindBin(highLimit2);
        fFitHisto2 = (TH1D*) h2->ProjectionX(tmp, binMin, binMax, "e");
        TCUtils::FormatHistogram(fFitHisto2, "TAPS.Energy.SG.Histo.Fit");
        
        // delete old function
        if (fFitFunc2) delete fFitFunc2;
        sprintf(tmp, "fPSA_2_%i", elem);
        fFitFunc2 = new TF1(tmp, "gaus");
        fFitFunc2->SetLineColor(2);

	// get important parameter positions
	maxPos = fFitHisto2->GetXaxis()->GetBinCenter(fFitHisto2->GetMaximumBin());
	max = fFitHisto2->GetBinContent(fFitHisto2->GetMaximumBin());

	// configure fitting function
	fFitFunc2->SetParameters(max, maxPos, 4);
	fFitFunc2->SetParLimits(0, max - 20, max + 20);
	fFitFunc2->SetParLimits(1, maxPos - 2, maxPos + 2);
	fFitFunc2->SetParLimits(2, 0.1, 5);

	// estimate peak position
        fPhi2 = fFitHisto2->GetBinCenter(fFitHisto2->GetMaximumBin());

        // first iteration
        fFitFunc2->SetRange(fPhi2 - 0.8, fPhi2 + 0.8);
        fFitFunc2->SetParameters(fFitHisto2->GetMaximum(), fPhi2, 0.5);
        fFitHisto2->Fit(fFitFunc2, "RBQ0");
        
        // second iteration
        fPhi2 = fFitFunc2->GetParameter(1);
        sigma = fFitFunc2->GetParameter(2);
        fFitFunc2->SetRange(fPhi2 - 2*sigma, fPhi2 + 2*sigma);
        fFitHisto2->Fit(fFitFunc2, "RBQ0");

        // final results
        fPhi2 = fFitFunc2->GetParameter(1); 
        
        // correct bad fits
        if (fPhi2 < xMin) fPhi2 = xMin + 1;
        if (fPhi2 > xMax) fPhi2 = xMax - 1;

        // draw mean indicator line
        fLine2->SetY1(0);
        fLine2->SetY2(fFitHisto2->GetMaximum() + 20);
        fLine2->SetX1(fPhi2);
        fLine2->SetX2(fPhi2);
        
        // set log axis
        fCanvasFit->cd(1)->SetLogz();

        // draw histogram
        fFitHisto->SetFillColor(35);
        fCanvasFit->cd(2);
        fFitHisto->Draw("hist");
        
        // draw fitting function
        if (fFitFunc) fFitFunc->Draw("same");
        
        // draw indicator line
        fLine1->Draw();
        
        // draw histogram
        fFitHisto2->SetFillColor(35);
        fCanvasFit->cd(3);
        fFitHisto2->Draw("hist");
        
        // draw fitting function
        if (fFitFunc2) fFitFunc2->Draw("same");
        
        // draw indicator line
        fLine2->Draw();
    }
    else
    {
        fCanvasFit->cd(1)->SetLogz(kFALSE);
    }

    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd(1);
        fPhiHisto1->Draw("E1");
        fCanvasResult->cd(2);
        fPhiHisto2->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine1->GetX1() != fPhi1) fPhi1 = fLine1->GetX1();
        if (fLine2->GetX1() != fPhi2) fPhi2 = fLine2->GetX1();

        // calculate short gate energies
        Double_t tanPhi1 = TMath::Tan(fPhi1*TMath::DegToRad());
        Double_t energySG1 = fRadius1 * tanPhi1 / TMath::Sqrt(1. + tanPhi1*tanPhi1);
        Double_t tanPhi2 = TMath::Tan(fPhi2*TMath::DegToRad());
        Double_t energySG2 = fRadius2 * tanPhi2 / TMath::Sqrt(1. + tanPhi2*tanPhi2);
        
        // calculate corresponding ADC values
        Double_t adc1 = energySG1 / fGainOld[elem] + fPedOld[elem];
        Double_t adc2 = energySG2 / fGainOld[elem] + fPedOld[elem];

        // scale factor
        Double_t a = energySG2 * tanPhi1 / energySG1 / tanPhi2;
        
        // new pedestal
        fPedNew[elem] = (adc2 - a * adc1) / (1. - a);
        
        // new gain
        fGainNew[elem] = energySG1 / (tanPhi1 * (adc1 - fPedNew[elem]) );
     
        // update overview histogram
        fPhiHisto1->SetBinContent(elem + 1, fPhi1);
        fPhiHisto1->SetBinError(elem + 1, 0.000001);
        fPhiHisto2->SetBinContent(elem + 1, fPhi2);
        fPhiHisto2->SetBinError(elem + 1, 0.000001);
    }
    else
    {
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Peak 1: %5.2f    Peak 2: %5.2f    "
           "old ped: %12.8f    new ped: %12.8f    "
           "old gain: %12.8f    new gain: %12.8f",
           elem, fPhi1, fPhi2,
           fPedOld[elem], fPedNew[elem],
           fGainOld[elem], fGainNew[elem]);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibTAPSEnergySG::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    "
               "old ped: %12.8f    new ped: %12.8f    "
               "old gain: %12.8f    new gain: %12.8f\n",
               i, fPedOld[i], fPedNew[i], fGainOld[i], fGainNew[i]);
    }
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters("Data.TAPS.SG.E0", fCalibration.Data(), fSet[i], fPedNew, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.TAPS.SG.E1", fCalibration.Data(), fSet[i], fGainNew, fNelem);
    }
   
    // save overview canvas
    SaveCanvas(fCanvasResult, "Overview");
}
ClassImp(TCCalibTAPSEnergySG)
