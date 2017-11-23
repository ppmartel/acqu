// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibDiscrThr                                                      //
//                                                                      //
// Calibration module for discriminator thresholds.                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibDiscrThr.h"



//______________________________________________________________________________
TCCalibDiscrThr::TCCalibDiscrThr(const Char_t* name, const Char_t* title, const Char_t* data,
                                 Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Empty constructor.

    // init members
    fADC = 0;
    fFileManager = 0;
    fPed = 0;
    fGain = 0;
    fMainHisto2 = 0;
    fDeriv = 0;
    fThr = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibDiscrThr::~TCCalibDiscrThr()
{
    // Destructor. 
    
    if (fADC) delete [] fADC;
    if (fFileManager) delete fFileManager;
    if (fPed) delete [] fPed;
    if (fGain) delete [] fGain;
    if (fMainHisto2) delete fMainHisto2;
    if (fDeriv) delete fDeriv;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibDiscrThr::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fADC = 0;
    fPed = 0;
    fGain = 0;
    fMainHisto2 = 0;
    fDeriv = 0;
    fThr = 0;
    fLine = new TLine();

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
    
    // get ADC list
    sprintf(tmp, "%s.ADCList", GetName());
    if (TCReadConfig::GetReader()->GetConfig(tmp))
    {
        // init ADC list
        fADC = new Int_t[fNelem];
        for (Int_t i = 0; i < fNelem; i++) fADC[i] = 0;
        
        // read ADC numbers
        ReadADC();
    }
    
    // get histogram name
    if (!fADC)
    {
        sprintf(tmp, "%s.Histo.Fit.Name", GetName());
        if (!TCReadConfig::GetReader()->GetConfig(tmp))
        {
            Error("Init", "Histogram name was not found in configuration!");
            return;
        }
        else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    }

    // get normalization histogram name
    TString normHistoName;
    sprintf(tmp, "%s.Histo.Norm.Name", GetName());
    if (TCReadConfig::GetReader()->GetConfig(tmp))
        normHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // read calibration
    if (fADC)
    {
        // read pedestal and gain sets
        sprintf(tmp, "%s.E0.Set", GetName());
        Int_t e0_set = TCReadConfig::GetReader()->GetConfigInt(tmp);
        sprintf(tmp, "%s.E1.Set", GetName());
        Int_t e1_set = TCReadConfig::GetReader()->GetConfigInt(tmp);
        
        // user info
        Info("Init", "Using set %d (pedestal) and set %d (gain) of energy calibration", e0_set, e1_set);
        
        // create arrays
        fPed = new Double_t[fNelem];
        fGain = new Double_t[fNelem];
        
        // read pedestal
        if (this->InheritsFrom("TCCalibTAPSCFD")) strcpy(tmp, "Data.TAPS.LG.E0");
        else if (this->InheritsFrom("TCCalibVetoLED")) strcpy(tmp, "Data.Veto.E0");
        TCMySQLManager::GetManager()->ReadParameters(tmp, fCalibration.Data(), e0_set, fPed, fNelem);
        
        // read gain
        if (this->InheritsFrom("TCCalibTAPSCFD")) strcpy(tmp, "Data.TAPS.LG.E1");
        else if (this->InheritsFrom("TCCalibVetoLED")) strcpy(tmp, "Data.Veto.E1");
        TCMySQLManager::GetManager()->ReadParameters(tmp, fCalibration.Data(), e1_set, fGain, fNelem);
    }

    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];
    
    // sum up all files contained in this runset
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
  
    // get the main calibration histogram
    if (!fADC)
    {
        fMainHisto = fFileManager->GetHistogram(fHistoName.Data());
        if (!fMainHisto)
        {
            Error("Init", "Main histogram does not exist!\n");
            return;
        }
    }

    // get the main normalization histogram
    if (normHistoName != "")
    {
        fMainHisto2 = (TH2*) fFileManager->GetHistogram(normHistoName.Data());
        if (!fMainHisto2)
        {
            Error("Init", "Normalization histogram does not exist!\n");
            return;
        }
    }

    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;threshold [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    if (!fADC)
    {
        sprintf(tmp, "%s.Histo.Fit", GetName());
        TCUtils::FormatHistogram(fMainHisto, tmp);
        if (fMainHisto2) TCUtils::FormatHistogram(fMainHisto2, tmp);
    }

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibDiscrThr::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    if (fADC)
    {
        if (fFitHisto) delete fFitHisto;
        sprintf(tmp, "ADC%d", fADC[elem]);
        fFitHisto = fFileManager->GetHistogram(tmp);
    }
    else
    {
        sprintf(tmp, "ProjHisto_%i", elem);
        TH2* h2 = (TH2*) fMainHisto;
        if (fFitHisto) delete fFitHisto;
        fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    }

    // create projection of the normalization histogram
    if (fMainHisto2)
    {
        TH1* hNorm = (TH1D*) fMainHisto2->ProjectionX(tmp, elem+1, elem+1, "e");
        fFitHisto->Divide(hNorm);
        delete hNorm;
    }
    
    // check for histo and sufficient statistics
    if (fFitHisto)
    {
        if (fFitHisto->GetEntries())
        {
           
            // derive histogram
            if (fDeriv) delete fDeriv;
            fDeriv = TCUtils::DeriveHistogram(fFitHisto);
            TCUtils::ZeroBins(fDeriv);
            
            // exclude pedestal
            if (fPed) 
            {
                Int_t lastBin = fDeriv->GetXaxis()->GetLast();
                fDeriv->GetXaxis()->SetRangeUser(fPed[elem]+7, fDeriv->GetXaxis()->GetBinCenter(lastBin));
                sprintf(tmp, "%s.Histo.Fit", GetName());
                TCUtils::FormatHistogram(fFitHisto, tmp);
            }
            
            // get maximum
            fThr = fDeriv->GetBinCenter(fDeriv->GetMaximumBin());

            // create fitting function
            if (fFitFunc) delete fFitFunc;
            sprintf(tmp, "Fitfunc_%d", elem);
            fFitFunc = new TF1(tmp, "gaus", fThr-8, fThr+8);
            fFitFunc->SetLineColor(kRed);
            fFitFunc->SetParameters(fDeriv->GetMaximum(), fThr, 1);

            // fit
            fDeriv->Fit(fFitFunc, "RBQ0");
            fThr = fFitFunc->GetParameter(1);
            
            // correct bad position
            if (fThr < fDeriv->GetXaxis()->GetXmin() || fThr > fDeriv->GetXaxis()->GetXmax()) 
                fThr = 0.5 * (fDeriv->GetXaxis()->GetXmin() + fDeriv->GetXaxis()->GetXmax());

            // draw histogram
            fCanvasFit->cd(2);
            if (fPed)
                fDeriv->GetXaxis()->SetRangeUser(fThr-7, fThr+7);
            else
                fDeriv->GetXaxis()->SetRangeUser(fThr-20, fThr+20);
            fDeriv->Draw("hist");

            // draw function
            fFitFunc->Draw("same");

            // draw indicator line
            fLine->Draw();

            // draw mean indicator line
            fLine->SetY1(0);
            fLine->SetY2(fFitHisto->GetMaximum() + 20);
            fLine->SetX1(fThr);
            fLine->SetX2(fThr);

            // draw histogram
            fFitHisto->SetFillColor(35);
            fCanvasFit->cd(1);
            fFitHisto->Draw("hist");
             
            // draw indicator line
            fLine->Draw();
        }
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
void TCCalibDiscrThr::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t empty = kFALSE;
    
    // check if fit was performed
    if (fFitHisto)
    {
        if (fFitHisto->GetEntries())
        {
            // check if line position was modified by hand
            if (fLine->GetX1() != fThr) fThr = fLine->GetX1();

            // calculate the new threshold
            if (fADC)
                fNewVal[elem] = fGain[elem] * (fThr - fPed[elem]);
            else 
                fNewVal[elem] = fThr;
        
            // update overview histogram
            fOverviewHisto->SetBinContent(elem + 1, fNewVal[elem]);
            fOverviewHisto->SetBinError(elem + 1, 0.000001);
        }
        else
        {   
            // set large threshold
            fNewVal[elem] = 9999;
            empty = kTRUE;
        }
    }
    else
    {   
        // set large threshold
        fNewVal[elem] = 9999;
        empty = kTRUE;
    }
 
    // user information
    printf("Element: %03d    "
           "fit: %14.8f    old threshold: %14.8f    new threshold: %14.8f    diff: %6.2f %%",
           elem, fThr, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (empty) printf("    -> empty");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibDiscrThr::ReadADC()
{
    // Read the ADC number of each element from the data file registered in the
    // configuration.
    
    Char_t tmp[256];
    const Char_t* filename;

    // get file name
    sprintf(tmp, "%s.ADCList", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("ReadADC", "ADC list file was not found!");
        return;
    }
    else filename = TCReadConfig::GetReader()->GetConfig(tmp)->Data();
    
    // read the calibration file
    TCReadARCalib c(filename, kFALSE);

    // check number of detectors
    if (c.GetNelements() != fNelem)
    {
        Error("ReadADC", "Number of elements in calibration file differs "
                         "from number requested by this module! (%d != %d)",
                         c.GetNelements(), fNelem);
        return;
    }

    // get element list and fill ADC numbers
    TList* list = c.GetElements();
    TIter next(list);
    TCARElement* e;
    Int_t n = 0;
    while ((e = (TCARElement*)next())) fADC[n++] = atoi(e->GetADC());
} 
ClassImp(TCCalibDiscrThr)
