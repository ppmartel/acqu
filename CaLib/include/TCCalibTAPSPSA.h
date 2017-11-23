// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSPSA                                                       //
//                                                                      //
// Calibration module for TAPS PSA.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSPSA_H
#define TCCALIBTAPSPSA_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"
#include "TPolyLine.h"
#include "TGraph.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSPSA : public TCCalib
{

private:
    Int_t fNpoints;                     // number of points
    Double_t* fRadiusMean;              // radius
    Double_t* fRadiusSigma;             // radius
    Double_t* fMean;                    // means
    Double_t* fSigma;                   // sigmas
    TPolyLine* fLMean;                  // mean line
    TPolyLine* fLSigma;                 // sigma
    TGraph** fGMean;                    // mean graph
    TGraph** fGSigma;                   // sigma graph
    TCFileManager* fFileManager;        // file manager
    TH1* fAngleProj;                    // angle projection
    Double_t fDelay;                    // display delay
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSPSA();
    virtual ~TCCalibTAPSPSA();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibTAPSPSA, 0) // TAPS PSA
};

#endif

