// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibVetoCorr                                                      //
//                                                                      //
// Calibration module for the Veto correlation.                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibVetoCorr.h"



//______________________________________________________________________________
TCCalibVetoCorr::TCCalibVetoCorr()
    : TCCalib("Veto.Correlation", "Veto correlation", "Data.Veto.E0", 
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fMax = 0;
    fARCalib = 0;
}

//______________________________________________________________________________
TCCalibVetoCorr::~TCCalibVetoCorr()
{
    // Destructor. 

    if (fARCalib) delete fARCalib;
}

//______________________________________________________________________________
void TCCalibVetoCorr::Init()
{
    // Init the module.
    
    // init members
    fMax = 0;
    fARCalib = 0;

    // read neighbours
    ReadNeighbours();

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("Veto.Correlation.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("Veto.Correlation.Histo.Fit.Name");
    
    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    TCUtils::FormatHistogram(fMainHisto, "Veto.Correlation.Histo.Fit");
    fMainHisto->Draw("colz");
}

//______________________________________________________________________________
void TCCalibVetoCorr::Fit(Int_t elem)
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
        // get the veto element with maximum number of hits
        fMax = fFitHisto->GetMaximumBin() - 1;
    
        // set range
        fFitHisto->GetXaxis()->SetRangeUser(fMax - 10, fMax + 10);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "Veto.Correlation.Histo.Fit");
    fFitHisto->Draw("hist");
    
    // update canvas
    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibVetoCorr::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // get number of TAPS elements
        Int_t maxTAPS = TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements");

        // 
        // check if veto in front of element has maximum hits
        //
        
        Bool_t inFront = kFALSE;
        if (fMax == TCUtils::GetVetoInFrontOfElement(elem, maxTAPS)) inFront = kTRUE;
    
        // user info
        if (inFront) 
        {
            printf("Element: %03d : Veto in front fired\n", elem);
            return;
        }

        //
        // check if veto of neighbouring element has maximum hits
        //
        
        Bool_t neighbr = kFALSE;
        TCARNeighbours* n = fARCalib->GetNeighbour(elem);
        
        // loop over neighbours
        for (Int_t i = 0; i < n->GetNneighbours(); i++)
        {
            // get veto of neighbour
            Int_t veto = TCUtils::GetVetoInFrontOfElement(n->GetNeighbour(i), maxTAPS);

            // check this veto
            if (fMax == veto) 
            {
                neighbr = kTRUE;
                break;
            }
        }
        
        // user info
        if (neighbr) 
        {
            printf("Element: %03d : Veto of neighbour fired\n", elem);
            return;
        }

        // not veto in front and no neighbour here
        printf("Element: %03d : No veto correlation found!! "
               "Maximum hits in veto %d  <---\n", elem, fMax);
    }
    else
    {
        printf("Element: %03d : no hits found!\n", elem);
    }
}   

//______________________________________________________________________________
void TCCalibVetoCorr::ReadNeighbours()
{
    // Read the detector neighbours of each element from the data file registered in the
    // configuration.
    
    Char_t tmp[256];
    const Char_t* filename;

    // get file name
    sprintf(tmp, "%s.Neighbours", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("ReadADC", "Neighbours file was not found!");
        return;
    }
    else filename = TCReadConfig::GetReader()->GetConfig(tmp)->Data();
    
    // read the calibration file with the correct element identifier
    fARCalib = new TCReadARCalib(filename, kFALSE, "Element:", "Next-TAPS:");

    // check number of detectors
    if (fARCalib->GetNneighbours() != fNelem)
    {
        Error("ReadNeighbours", "Number of neighbours statements in calibration file differs "
                                "from number requested by this module! (%d != %d)",
                                fARCalib->GetNelements(), fNelem);
        return;
    }
} 

//______________________________________________________________________________
void TCCalibVetoCorr::Write()
{
    // Disable this method.

    Info("Write", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibVetoCorr::PrintValues()
{
    // Disable this method.

    Info("PrintValues", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibVetoCorr::PrintValuesChanged()
{
    // Disable this method.

    Info("PrintValuesChanged", "Not implemented in this module");
}
ClassImp(TCCalibVetoCorr)
