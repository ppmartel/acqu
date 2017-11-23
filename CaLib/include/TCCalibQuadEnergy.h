// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibQuadEnergy                                                    //
//                                                                      //
// Base quadratic energy correction module class.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBQUADENERGY_H
#define TCCALIBQUADENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibQuadEnergy : public TCCalib
{

private:
    Double_t* fPar0;                        // correction parameter 0
    Double_t* fPar1;                        // correction parameter 1
    TH2* fMainHisto2;                       // histogram with mean photon energy of pi0
    TH2* fMainHisto3;                       // histogram with mean photon energy of eta
    TH1* fFitHisto1b;                       // fitting histogram
    TH1* fFitHisto2;                        // fitting histogram
    TH1* fFitHisto3;                        // fitting histogram
    TF1* fFitFunc1b;                        // additional fitting function
    Double_t fPi0Pos;                       // pi0 position
    Double_t fEtaPos;                       // eta position
    Double_t fPi0MeanE;                     // pi0 mean energy
    Double_t fEtaMeanE;                     // eta mean energy
    TLine* fLinePi0;                        // pi0 indicator line
    TLine* fLineEta;                        // eta indicator line
    TLine* fLineMeanEPi0;                   // pi0 mean photon energy indicator line
    TLine* fLineMeanEEta;                   // eta mean photon energy indicator line
    TH1* fPi0PosHisto;                      // histogram of pi0 positions
    TH1* fEtaPosHisto;                      // histogram of eta positions

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibQuadEnergy() : TCCalib(), fPar0(0), fPar1(0),
                          fMainHisto2(0), fMainHisto3(0), 
                          fFitHisto1b(0), fFitHisto2(0), fFitHisto3(0),
                          fFitFunc1b(0),
                          fPi0Pos(0), fEtaPos(0), fPi0MeanE(0), fEtaMeanE(0),
                          fLinePi0(0), fLineEta(0), fLineMeanEPi0(0), fLineMeanEEta(0),
                          fPi0PosHisto(0), fEtaPosHisto(0) { }
    TCCalibQuadEnergy(const Char_t* name, const Char_t* title, const Char_t* data,
                      Int_t nElem);
    virtual ~TCCalibQuadEnergy();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibQuadEnergy, 0) // Base quadratic energy correction class
};


class TCCalibCBQuadEnergy : public TCCalibQuadEnergy
{
    
public:
    TCCalibCBQuadEnergy()
        : TCCalibQuadEnergy("CB.QuadEnergy", "CB quadratic energy correction", 
                            "Data.CB.Energy.Quad.Par0", 
                            TCConfig::kMaxCB) { }
    virtual ~TCCalibCBQuadEnergy() { }

    ClassDef(TCCalibCBQuadEnergy, 0) // CB quadratic energy correction
};


class TCCalibTAPSQuadEnergy : public TCCalibQuadEnergy
{
    
public:
    TCCalibTAPSQuadEnergy()
        : TCCalibQuadEnergy("TAPS.QuadEnergy", "TAPS quadratic energy correction", 
                            "Data.TAPS.Energy.Quad.Par0", 
                            TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSQuadEnergy() { }

    ClassDef(TCCalibTAPSQuadEnergy, 0) // TAPS quadratic energy correction
};

#endif

