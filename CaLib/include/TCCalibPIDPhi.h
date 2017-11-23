// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDPhi                                                        //
//                                                                      //
// Calibration module for the PID phi angle.                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBPIDPHI_H
#define TCCALIBPIDPHI_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibPIDPhi : public TCCalib
{

private:
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    TCanvas* fCanvasResult2;            // second result canvas
    TH1* fOverviewHisto2;               // second overview histogram
    TF1* fFitFunc2;                     // second fitting function

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
    TH1* GetMappedHistogram(TH1* histo);

public:
    TCCalibPIDPhi();
    virtual ~TCCalibPIDPhi();
    
    virtual void Write();

    ClassDef(TCCalibPIDPhi, 0) // PID phi calibration
};

#endif

