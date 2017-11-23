// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalib                                                              //
//                                                                      //
// Abstract calibration module class.                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIB_H
#define TCCALIB_H

#include "KeySymbols.h"
#include "TString.h"
#include "TError.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTimer.h"
#include "TSystem.h"
#include "TTimeStamp.h"
#include "TStyle.h"
#include "TGClient.h"

#include "TCMySQLManager.h"
#include "TCReadConfig.h"
#include "TCUtils.h"


class TCCalib : public TNamed
{

protected:
    TString fData;              // used calibration data
    TString fCalibration;       // calibration identifier
    Int_t fNset;                // number of sets to be calibrated
    Int_t* fSet;                //[fNset] array of sets to be calibrated
    TString fHistoName;         // name of the calibration histogram
    Int_t fNelem;               // number of calibration values
    Int_t fCurrentElem;         // number of current element
    Double_t* fOldVal;          //[fNelem] old calibration value array
    Double_t* fNewVal;          //[fNelem] new calibration value array
    Double_t fAvr;              // average value
    Double_t fAvrDiff;          // average difference to aimed value
    Int_t fNcalc;               // number of calculated elements

    TH1* fMainHisto;            // main histogram 
    TH1* fFitHisto;             // fitting histogram
    TF1* fFitFunc;              // fitting function
    TF1* fFitPeak;
    TF1* fFitBackGround;


    TH1* fOverviewHisto;        // overview result histogram
    
    TCanvas* fCanvasFit;        // canvas containing the fits
    TCanvas* fCanvasResult;     // canvas containing the results
    TCanvas* fExtraCanvas;
    
    TTimer* fTimer;             // slow-motion timer
    Bool_t fTimerRunning;       // timer running state

    Double_t fConvergenceFactor; // used in gain/offset calculation

    virtual void Init() = 0;
    virtual void Fit(Int_t elem) = 0;
    virtual void Calculate(Int_t elem) = 0;
    void SaveCanvas(TCanvas* c, const Char_t* name);

public:
    TCCalib() : TNamed(),
                fData(), 
                fCalibration(),
                fSet(0), fHistoName(), 
                fNelem(0), fCurrentElem(0),
                fOldVal(0), fNewVal(0),
                fAvr(0), fAvrDiff(0), fNcalc(0),
                fMainHisto(0), fFitHisto(0), fFitFunc(0), fFitPeak(nullptr), fFitBackGround(nullptr),
                fOverviewHisto(0),
                fCanvasFit(0), fCanvasResult(0), 
                fTimer(0), fTimerRunning(kFALSE) { }
    TCCalib(const Char_t* name, const Char_t* title, 
            const Char_t* data, Int_t nElem) 
        : TNamed(name, title),
          fData(data), 
          fCalibration(),
          fSet(0), fHistoName(), 
          fNelem(nElem), fCurrentElem(0),
          fOldVal(0), fNewVal(0),
          fAvr(0), fAvrDiff(0), fNcalc(0),
          fMainHisto(0), fFitHisto(0), fFitFunc(0), fFitPeak(nullptr), fFitBackGround(nullptr),
          fOverviewHisto(0),
          fCanvasFit(0), fCanvasResult(0), 
          fTimer(0), fTimerRunning(kFALSE), fConvergenceFactor(1) { }
    virtual ~TCCalib();
    
    virtual void Write();
    virtual void PrintValues();
    virtual void PrintValuesChanged();

    void Start(const Char_t* calibration, Int_t nSet, Int_t* set);
    void ProcessAll(Int_t msecDelay = 0);
    void ProcessElement(Int_t elem, Bool_t ignorePrev = kFALSE);
    void Previous();
    void Next();
    void Ignore();
    void StopProcessing();
    
    TString GetCalibData() { return fData; }

    void EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected);

    ClassDef(TCCalib, 0) // Base calibration module class
};

#endif

