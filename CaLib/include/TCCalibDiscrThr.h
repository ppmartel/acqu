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


#ifndef TCCALIBDISCRTHR_H
#define TCCALIBDISCRTHR_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibDiscrThr : public TCCalib
{

private:
    Int_t* fADC;                        // array of element ADC numbers
    TCFileManager* fFileManager;        // file manager
    Double_t* fPed;                     // pedestal array
    Double_t* fGain;                    // gain array
    TH2* fMainHisto2;                   // normalization histogram
    TH1* fDeriv;                        // derived histogram
    Double_t fThr;                      // threshold value
    TLine* fLine;                       // mean indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
    void ReadADC();
    
public:
    TCCalibDiscrThr() : TCCalib(), fADC(0), fFileManager(0),
                        fPed(0), fGain(0),
                        fMainHisto2(0), fDeriv(0), fThr(0), fLine(0) { }
    TCCalibDiscrThr(const Char_t* name, const Char_t* title, const Char_t* data,
                    Int_t nElem);
    virtual ~TCCalibDiscrThr();

    ClassDef(TCCalibDiscrThr, 0) // Discriminator threshold calibration base class
};


class TCCalibCBLED : public TCCalibDiscrThr
{

public:
    TCCalibCBLED()
        : TCCalibDiscrThr("CB.LED", "CB LED calibration", 
                          "Data.CB.LED",
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBLED() { }

    ClassDef(TCCalibCBLED, 0) // CB LED calibration
};


class TCCalibTAPSLED1 : public TCCalibDiscrThr
{

public:
    TCCalibTAPSLED1()
        : TCCalibDiscrThr("TAPS.LED1", "TAPS LED1 calibration", 
                          "Data.TAPS.LED1",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED1() { }

    ClassDef(TCCalibTAPSLED1, 0) // TAPS LED1 calibration
};


class TCCalibTAPSLED2 : public TCCalibDiscrThr
{

public:
    TCCalibTAPSLED2()
        : TCCalibDiscrThr("TAPS.LED2", "TAPS LED2 calibration", 
                          "Data.TAPS.LED2",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED2() { }

    ClassDef(TCCalibTAPSLED2, 0) // TAPS LED2 calibration
};

class TCCalibTAPSCFD : public TCCalibDiscrThr
{

public:
    TCCalibTAPSCFD()
        : TCCalibDiscrThr("TAPS.CFD", "TAPS CFD calibration", 
                          "Data.TAPS.CFD",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSCFD() { }

    ClassDef(TCCalibTAPSCFD, 0) // TAPS CFD calibration
};

class TCCalibVetoLED : public TCCalibDiscrThr
{

public:
    TCCalibVetoLED()
        : TCCalibDiscrThr("Veto.LED", "Veto LED calibration", 
                          "Data.Veto.LED",
                     TCReadConfig::GetReader()->GetConfigInt("Veto.Elements")) { }
    virtual ~TCCalibVetoLED() { }

    ClassDef(TCCalibVetoLED, 0) // Veto LED calibration
};

#endif

