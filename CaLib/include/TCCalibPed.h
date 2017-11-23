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


#ifndef TCCALIBPED_H
#define TCCALIBPED_H

#include <cstdlib>

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCReadARCalib.h"
#include "TCFileManager.h"

#include "a2display.h"
#include "TH2CB.h"
#include "TH2TAPS.h"


class TCCalibPed : public TCCalib
{

private:
    Int_t* fADC;                        // array of element ADC numbers
    TCFileManager* fFileManager;        // file manager
    Double_t fMean;                     // mean position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    void ReadADC();
protected:

        TH2Crystals* fDetectorView;

public:
    TCCalibPed() : TCCalib(), fADC(0), fFileManager(0), fMean(0), fLine(0) { }
    TCCalibPed(const Char_t* name, const Char_t* title, const Char_t* data,
               Int_t nElem);
    virtual ~TCCalibPed();

    ClassDef(TCCalibPed, 0) // Base pedestal calibration class
};


class TCCalibTAPSPedLG : public TCCalibPed
{

public:
    TCCalibTAPSPedLG() 
        : TCCalibPed("TAPS.Ped.LG", "TAPS LG pedestal calibration",
                     "Data.TAPS.LG.E0",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) {

        fDetectorView = new TH2TAPS("calib_taps", "TAPS");

    }
    virtual ~TCCalibTAPSPedLG() { }
    
    ClassDef(TCCalibTAPSPedLG, 0) // TAPS LG pedestal calibration class
};


class TCCalibTAPSPedSG : public TCCalibPed
{

public:
    TCCalibTAPSPedSG() 
        : TCCalibPed("TAPS.Ped.SG", "TAPS SG pedestal calibration",
                     "Data.TAPS.SG.E0",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) {

        fDetectorView = new TH2TAPS("calib_taps", "TAPS");

    }
    virtual ~TCCalibTAPSPedSG() { }
    
    ClassDef(TCCalibTAPSPedSG, 0) // TAPS SG pedestal calibration class
};


class TCCalibTAPSPedVeto : public TCCalibPed
{

public:
    TCCalibTAPSPedVeto() 
        : TCCalibPed("TAPS.Ped.Veto", "Veto pedestal calibration",
                     "Data.Veto.E0",
                     TCReadConfig::GetReader()->GetConfigInt("Veto.Elements")) {

        fDetectorView = new TH2TAPS("calib_taps", "TAPS");

    }
    virtual ~TCCalibTAPSPedVeto() { }
    
    ClassDef(TCCalibTAPSPedVeto, 0) // Veto pedestal calibration class
};

#endif

