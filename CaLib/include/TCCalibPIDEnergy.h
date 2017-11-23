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


#ifndef TCCALIBPIDENERGY_H
#define TCCALIBPIDENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TH3.h"
#include "TLine.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TSpectrum.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibPIDEnergy : public TCCalib
{

private:
    TCFileManager* fFileManager;        // file manager
    Double_t* fPed;                     // pedestal values
    Double_t* fGain;                    // gain values
    TGraphErrors* fLinPlot;             // linear fitting histogram
    Int_t fNpeak;                       // number of proton peaks
    Double_t* fPeak;                    //[fNpeak] proton peak positions
    Double_t* fPeak_Err;                //[fNpeak] proton peak position errors
    Double_t* fPeakMC;                  //[fNpeak] proton MC peak positions
    Double_t* fPeakMC_Err;              //[fNpeak] proton MC peak position errors
    TLine* fLine;                       // mean indicator line
    Int_t fDelay;                       // projection fit display delay
    TH2* fMCHisto;                      // MC histogram
    TFile* fMCFile;                     // MC ROOT file
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
    void FitSlices(TH2* h);

public:
    TCCalibPIDEnergy();
    virtual ~TCCalibPIDEnergy();

    virtual void Write();
    virtual void PrintValues();


    ClassDef(TCCalibPIDEnergy, 0) // PID energy calibration
};

#endif

