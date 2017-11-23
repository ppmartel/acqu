// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibEnergy                                                        //
//                                                                      //
// Base energy calibration module class.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBENERGY_H
#define TCCALIBENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TIndicatorLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"

#include "a2display.h"
#include "TH2CB.h"
#include "TH2TAPS.h"


class TCCalibEnergy : public TCCalib
{

protected:
    Double_t fPi0Pos;                   // pi0 position
    TIndicatorLine* fLine;              // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

protected:
        TH2Crystals* fDetectorView;
        virtual void initFitFunction() =0;
        bool fFitOk;
        enum {
            vFitOK =1,
            vCurrPos =2,
            vFitFailed=3
        };

public:
    TCCalibEnergy() : TCCalib(), fPi0Pos(0), fLine(0) { }
    TCCalibEnergy(const Char_t* name, const Char_t* title, const Char_t* data,
                  Int_t nElem);
    virtual ~TCCalibEnergy();

    ClassDef(TCCalibEnergy, 0) // Base energy calibration class
};


class TCCalibCBEnergy : public TCCalibEnergy
{
protected:
    virtual void initFitFunction();
    
public:
    TCCalibCBEnergy()
        : TCCalibEnergy("CB.Energy", "CB energy calibration", 
                        "Data.CB.E1", 
                        TCConfig::kMaxCB) {
        fDetectorView = new TH2CB("calib_cb", "CB");
    }
    virtual ~TCCalibCBEnergy() { }

    ClassDef(TCCalibCBEnergy, 0) // CB energy calibration
};


class TCCalibTAPSEnergyLG : public TCCalibEnergy
{
protected:
    virtual void initFitFunction();
    
public:
    TCCalibTAPSEnergyLG()
        : TCCalibEnergy("TAPS.Energy.LG", "TAPS LG energy calibration", 
                        "Data.TAPS.LG.E1",
                        TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) {
        fDetectorView = new TH2TAPS("calib_taps", "TAPS");
    }

    virtual ~TCCalibTAPSEnergyLG() { }

    ClassDef(TCCalibTAPSEnergyLG, 0) // TAPS LG energy calibration
};

#endif

