// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTime                                                          //
//                                                                      //
// Base time calibration module class.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTIME_H
#define TCCALIBTIME_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"

#include "TIndicatorLine.h"

class TCCalibTime : public TCCalib
{

protected:
    Double_t* fTimeGain;                // TDC gain array
    Double_t fMean;                     // mean time position
    TIndicatorLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTime() : TCCalib(), fTimeGain(0), fMean(0), fLine(0) { }
    TCCalibTime(const Char_t* name, const Char_t* title, const Char_t* data,
                Int_t nElem);
    virtual ~TCCalibTime();

    ClassDef(TCCalibTime, 0) // Base time calibration class
};


class TCCalibTaggerTime : public TCCalibTime
{

public:
    TCCalibTaggerTime() 
        : TCCalibTime("Tagger.Time", "Tagger time calibration",
                     "Data.Tagger.T0",
                     TCReadConfig::GetReader()->GetConfigInt("Tagger.Elements")) { }
    virtual ~TCCalibTaggerTime() { }
    
    ClassDef(TCCalibTaggerTime, 0) // Tagger time calibration class
};


class TCCalibCBTime : public TCCalibTime
{

public:
    TCCalibCBTime() 
        : TCCalibTime("CB.Time", "CB time calibration",
                     "Data.CB.T0",
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBTime() { }

    virtual void Fit(Int_t elem);
    
    ClassDef(TCCalibCBTime, 0) // CB time calibration class
};


class TCCalibCBRiseTime : public TCCalibTime
{

public:
    TCCalibCBRiseTime() 
        : TCCalibTime("CB.RiseTime", "CB rise time calibration",
                     "Data.CB.Walk.Par0",
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBRiseTime() { }
    
    ClassDef(TCCalibCBRiseTime, 0) // CB rise time calibration class
};


class TCCalibTAPSTime : public TCCalibTime
{

public:
    TCCalibTAPSTime() 
        : TCCalibTime("TAPS.Time", "TAPS time calibration",
                     "Data.TAPS.T0",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSTime() { }
    
    ClassDef(TCCalibTAPSTime, 0) // TAPS time calibration class
};


class TCCalibPIDTime : public TCCalibTime
{

public:
    TCCalibPIDTime() 
        : TCCalibTime("PID.Time", "PID time calibration",
                     "Data.PID.T0",
                     TCConfig::kMaxPID) { }
    virtual ~TCCalibPIDTime() { }
    
    ClassDef(TCCalibPIDTime, 0) // PID time calibration class
};


class TCCalibVetoTime : public TCCalibTime
{

public:
    TCCalibVetoTime() 
        : TCCalibTime("Veto.Time", "Veto time calibration",
                     "Data.Veto.T0",
                     TCReadConfig::GetReader()->GetConfigInt("Veto.Elements")) { }
    virtual ~TCCalibVetoTime() { }
    
    ClassDef(TCCalibVetoTime, 0) // Veto time calibration class
};

#endif

