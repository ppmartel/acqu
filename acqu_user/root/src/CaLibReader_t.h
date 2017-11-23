/*************************************************************************
 * Authors: Irakli Keshelashvili, 2010
 *          Dominik Werthmueller, 2010-2012
 *          Thomas Strub, 2013
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CaLibReader_t                                                        //
//                                                                      //
// CaLib database connection and reader class.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef __CaLibReader_t_h__
#define __CaLibReader_t_h__

#include "TError.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TF1.h"

#include "TA2TAPS_BaF2.h"


class CaLibReader_t 
{

private:
    TSQLServer* fDB;                    // SQL server
    Bool_t fWasError;                   // error flag
    Int_t fRun;                         // run number
    Char_t fCalibration[256];           // calibration identifier
    Bool_t fTAGGtime;                   // tagger time flag
    Bool_t fTAGGeff;                    // tagging efficiency
    Bool_t fCBenergy;                   // CB energy flag
    Bool_t fCBquadEnergy;               // CB quadratic energy flag
    Bool_t fCBtime;                     // CB time flag
    Bool_t fCBwalk;                     // CB walk flag
    Bool_t fCBled;                      // CB LED flag
    Bool_t fTAPSenergy;                 // TAPS energy flag
    Bool_t fTAPSquadEnergy;             // TAPS quadratic energy flag
    Bool_t fTAPStime;                   // TAPS time flag
    Bool_t fTAPScfd;                    // TAPS CFD flag
    Bool_t fTAPSled;                    // TAPS LED flag
    Bool_t fPIDphi;                     // PID phi flag
    Bool_t fPIDenergy;                  // PID energy flag
    Bool_t fPIDtime;                    // PID time flag
    Bool_t fVetoenergy;                 // Veto energy flag
    Bool_t fVetotime;                   // Veto time flag
    Bool_t fVetoled;                    // Veto led flag

    Bool_t fBadScRlist;                 // bad scaler read list flag
    Bool_t fBadScRfirst;                // bad scaler read first flag
    Bool_t fBadScRlast;                 // bad scaler read last flag
 
    Double_t* fTAGGEffVal;              // tagging efficiency values
    Double_t* fCBQuadEnergyPar0;        // CB quadratic energy correction parameter 0
    Double_t* fCBQuadEnergyPar1;        // CB quadratic energy correction parameter 1
    Double_t* fCBLEDThr;                // CB LED thresholds
    Double_t* fTAPSQuadEnergyPar0;      // TAPS quadratic energy correction parameter 0
    Double_t* fTAPSQuadEnergyPar1;      // TAPS quadratic energy correction parameter 1
    Double_t* fTAPSLED1Thr;             // TAPS LED1 thresholds
    Double_t* fTAPSLED2Thr;             // TAPS LED2 thresholds

    Int_t fTotNScR;                     // total number of scaler reads
    Int_t fNBadScR;                     // number of bad scaler reads
    Int_t* fBadScR;                     // list of bad scaler reads

    Bool_t SearchRunEntry(const Char_t* name, Char_t* out);
    Bool_t ReadParameters(const Char_t* table, Double_t* par, Int_t length);
    Bool_t ApplyPerRunCorr(const Char_t* table, Double_t* par, Int_t length);

    void DisableTAGGcalib();
    void DisableCBcalib();
    void DisableTAPScalib();
    void DisablePIDcalib();
    void DisableVetocalib();

public:
    CaLibReader_t(const Char_t* dbHost, const Char_t* dbName, 
                  const Char_t* dbUser, const Char_t* dbPass,
                  const Char_t* calibration, Int_t run);
    virtual ~CaLibReader_t();

    Bool_t ReadScalerReads(Int_t ndata_in, Char_t** data_in);

    Bool_t ApplyTAGGcalib(TA2Detector* det);
    Bool_t ApplyCBcalib(TA2Detector* det);
    Bool_t ApplyTAPScalib(TA2TAPS_BaF2* det);
    Bool_t ApplyPIDcalib(TA2Detector* det);
    Bool_t ApplyVetocalib(TA2Detector* det);

    Bool_t IsConnected() { return fDB ? kTRUE : kFALSE; }
    Bool_t WasError() { return fWasError; }
    void Deconnect()
    {
        delete fDB;
        fDB = 0;
    }

    void SetTAGGtime(Bool_t b = kTRUE) { fTAGGtime = b; }
    void SetTAGGeff(Bool_t b = kTRUE) { fTAGGeff = b; }
    void SetCBenergy(Bool_t b = kTRUE) { fCBenergy = b; }
    void SetCBquadEnergy(Bool_t b = kTRUE) { fCBquadEnergy = b; }
    void SetCBtime(Bool_t b = kTRUE) { fCBtime = b; } 
    void SetCBwalk(Bool_t b = kTRUE) { fCBwalk = b; }     
    void SetCBled(Bool_t b = kTRUE) { fCBled = b; }
    void SetTAPSenergy(Bool_t b = kTRUE) { fTAPSenergy = b; }
    void SetTAPSquadEnergy(Bool_t b = kTRUE) { fTAPSquadEnergy = b; }
    void SetTAPStime(Bool_t b = kTRUE) { fTAPStime = b; }    
    void SetTAPSled(Bool_t b = kTRUE) { fTAPSled = b; }
    void SetTAPScfd(Bool_t b = kTRUE) { fTAPScfd = b; }
    void SetPIDphi(Bool_t b = kTRUE) { fPIDphi = b; }
    void SetPIDenergy(Bool_t b = kTRUE) { fPIDenergy = b; }
    void SetPIDtime(Bool_t b = kTRUE) { fPIDtime = b; }
    void SetVetoenergy(Bool_t b = kTRUE) { fVetoenergy = b; }
    void SetVetotime(Bool_t b = kTRUE) { fVetotime = b; }
    void SetVetoled(Bool_t b = kTRUE) { fVetoled = b; }
    void SetBadScRlist(Bool_t b = kTRUE) { fBadScRlist = b; }
    void SetBadScRfirst(Bool_t b = kTRUE) { fBadScRfirst = b; }
    void SetBadScRlast(Bool_t b = kTRUE) { fBadScRlast = b; }
    
    Bool_t GetTAGGeff() const { return fTAGGeff; }
    Bool_t GetCBquadEnergy() const { return fCBquadEnergy; }
    Bool_t GetCBled() const { return fCBled; }
    Bool_t GetTAPSquadEnergy() const { return fTAPSquadEnergy; }
    Bool_t GetTAPSled() const { return fTAPSled; }
    Double_t GetTAGGEffVal(Int_t n) const { return fTAGGEffVal[n]; }
    Double_t GetCBQuadEnergyPar0(Int_t n) const { return fCBQuadEnergyPar0[n]; }
    Double_t GetCBQuadEnergyPar1(Int_t n) const { return fCBQuadEnergyPar1[n]; }
    Double_t GetCBLEDThr(Int_t n) const { return fCBLEDThr[n]; }
    Double_t GetTAPSQuadEnergyPar0(Int_t n) const { return fTAPSQuadEnergyPar0[n]; }
    Double_t GetTAPSQuadEnergyPar1(Int_t n) const { return fTAPSQuadEnergyPar1[n]; }
    Double_t GetTAPSLED1Thr(Int_t n) const { return fTAPSLED1Thr[n]; }
    Double_t GetTAPSLED2Thr(Int_t n) const { return fTAPSLED2Thr[n]; }
    Int_t GetTotNScR() const { return fTotNScR; } 
    Int_t GetNBadScR() const { return fNBadScR; } 
    Int_t* GetBadScR() const { return fBadScR; } 

};

#endif

