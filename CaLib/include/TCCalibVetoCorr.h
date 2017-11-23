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


#ifndef TCCALIBVETOCORR_H
#define TCCALIBVETOCORR_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"
#include "TCReadARCalib.h"


class TCCalibVetoCorr : public TCCalib
{

private:
    Int_t fMax;                         // maximum element
    TCReadARCalib* fARCalib;            // AcquRoot calibration file reader

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    void ReadNeighbours();

public:
    TCCalibVetoCorr();
    virtual ~TCCalibVetoCorr();

    virtual void Write();
    virtual void PrintValues();
    virtual void PrintValuesChanged();

    ClassDef(TCCalibVetoCorr, 0) // Veto correlation
};

#endif

