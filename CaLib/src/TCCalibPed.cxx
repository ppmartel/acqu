// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPed                                                           //
//                                                                      //
// Base pedestal calibration module class.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPed.h"



//______________________________________________________________________________
TCCalibPed::TCCalibPed(const Char_t* name, const Char_t* title, const Char_t* data,
                       Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.
    
    // init members
    fADC = 0;
    fFileManager = 0;
    fMean = 0;
    fLine = 0;
    fDetectorView = 0;

}

//______________________________________________________________________________
TCCalibPed::~TCCalibPed()
{
    // Destructor. 
    
    if (fADC) delete [] fADC;
    if (fFileManager) delete fFileManager;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibPed::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fADC = new Int_t[fNelem];
    for (Int_t i = 0; i < fNelem; i++) fADC[i] = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fMean = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
  
    // read ADC numbers
    ReadADC();

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Pedestal position [Channel]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // prepare main canvas
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    
    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}    

//______________________________________________________________________________
void TCCalibPed::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // load the pedestal histogram
    if (fFitHisto) delete fFitHisto;
    sprintf(tmp, "ADC%d", fADC[elem]);
    fFitHisto = fFileManager->GetHistogram(tmp);
    
    // dummy position
    fMean = 100;

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fPed_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
	Double_t fTotMaxPos = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

	// find first big jump
	for(int i=10; i<200; i++)
	{
            if(fFitHisto->GetBinContent(i) > 100 &&
	       fFitHisto->GetBinContent(i) > 5*fFitHisto->GetBinContent(i-3) && 
	       fFitHisto->GetBinContent(i) > 5*fFitHisto->GetBinContent(i+3))
	    {
	        Double_t fTotMaxPos = i+2;
	        fMean = fFitHisto->GetBinCenter( fTotMaxPos );
	        break;
	    }
        }

        // configure fitting function
        fFitFunc->SetRange(fMean - 2, fMean + 2);
        fFitFunc->SetLineColor(2);
        fFitFunc->SetParameters(1, fMean, 0.1);
	fFitFunc->SetParLimits(2, 0.001, 1);
	if (fTotMaxPos > fMean)
	{
	    fFitFunc->SetRange(fMean - 7, fMean + 5);
	    fFitFunc->SetParLimits(2, 0.0000001, 5);
	}
        fFitHisto->Fit(fFitFunc, "RB0Q");
	
	// second iteration for elements where pedestal isn't the maximum
	if (fTotMaxPos > fMean)
	{
	  fFitFunc->SetRange(fFitFunc->GetParameter(1) - 15, fFitFunc->GetParameter(1) + 10);
	  fFitHisto->Fit(fFitFunc, "RB0Q");
	}

        // final results
        fMean = fFitFunc->GetParameter(1); 

        // check if reasonable
        if (fMean < 50 || fMean > 130) fMean = 100;

        // draw mean indicator line
        fLine->SetY1(0);
        fFitHisto->GetXaxis()->SetRange(0, fFitHisto->GetNbinsX());
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // set indicator line
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    //fFitHisto->GetXaxis()->SetRangeUser(fMean-10, fMean+10);
    fFitHisto->GetXaxis()->SetRangeUser(40, 140);
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


    if(fDetectorView) {
        fDetectorView->Reset("");
        fDetectorView->SetElement(elem, 1);
        fDetectorView->SetTitle(Form("TAPS Element %d",elem));
        fExtraCanvas->cd();
        fDetectorView->Draw("col");
        fExtraCanvas->Update();
    }

}

//______________________________________________________________________________
void TCCalibPed::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();
 
        // save pedestal position
        fNewVal[elem] = fMean;
    
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fNewVal[elem]);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    "
           "old pedestal: %12.8f    new pedestal: %12.8f    diff: %6.2f %%",
           elem, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibPed::ReadADC()
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
    
    // read the calibration file with the correct element identifier
    if (this->InheritsFrom("TCCalibTAPSPedSG")) strcpy(tmp, "TAPSSG:");
    else strcpy(tmp, "Element:");
    TCReadARCalib c(filename, kFALSE, tmp);

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
ClassImp(TCCalibPed)
