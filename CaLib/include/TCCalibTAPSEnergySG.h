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


#ifndef TCCALIBTAPSENERGYSG_H
#define TCCALIBTAPSENERGYSG_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSEnergySG : public TCCalib
{

private:
    Double_t* fPedOld;                  //[fNelem] old pedestal array
    Double_t* fGainOld;                 //[fNelem] old gain array
    Double_t* fPedNew;                  //[fNelem] new pedestal array
    Double_t* fGainNew;                 //[fNelem] new gain array
    Double_t fPhi1;                     // PSA phi angle for low interval
    Double_t fPhi2;                     // PSA phi angle for high interval
    Double_t fRadius1;                  // PSA radius of low interval
    Double_t fRadius2;                  // PSA radius of high interval
    TCFileManager* fFileManager;        // file manager
    TLine* fLine1;                      // mean indicator line for low interval
    TLine* fLine2;                      // mean indicator line for high interval
    TF1* fFitFunc2;                     // second fitting function
    TH1* fFitHisto2;                    // second fitting histogram
    TH1* fPhiHisto1;                    // histogram of phi angles for low interval
    TH1* fPhiHisto2;                    // histogram of phi angles for high interval

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSEnergySG();
    virtual ~TCCalibTAPSEnergySG();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibTAPSEnergySG, 0) // TAPS SG energy calibration
};

#endif

