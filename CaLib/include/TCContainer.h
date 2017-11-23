// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCContainer                                                          //
//                                                                      //
// Run information and calibration storage class.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCONTAINER_H 
#define TCCONTAINER_H

#include "TNamed.h"
#include "TFile.h"
#include "TList.h"

#include "TCConfig.h"


class TCRun : public TObject
{

private:
    Int_t fRun;                     // run number
    Char_t fPath[256];              // file path
    Char_t fFileName[256];          // file name
    Char_t fTime[256];              // time
    Char_t fDescription[256];       // description
    Char_t fRunNote[256];           // run note
    Long64_t fSize;                 // file size
    Char_t fTarget[20];             // target
    Char_t fTargetPol[128];         // target polarization
    Double_t fTargetPolDeg;         // target polarization degree
    Char_t fBeamPol[128];           // beam polarization
    Double_t fBeamPolDeg;           // beam polarization degree

public:
    TCRun() : TObject()
    {
        fRun = 0;
        fPath[0] = '\0';
        fFileName[0] = '\0';
        fTime[0] = '\0';
        fDescription[0] = '\0';
        fRunNote[0] = '\0';
        fSize = 0;
        fTarget[0] = '\0';
        fTargetPol[0] = '\0';
        fTargetPolDeg = 0;
        fBeamPol[0] = '\0';
        fBeamPolDeg = 0;
    }
    virtual ~TCRun() { }

    void SetRun(Int_t run) { fRun = run; }
    void SetPath(const Char_t* path) { strcpy(fPath, path); }
    void SetFileName(const Char_t* fname) { strcpy(fFileName, fname); }
    void SetTime(const Char_t* time) { strcpy(fTime, time); }
    void SetDescription(const Char_t* desc) { strcpy(fDescription, desc); }
    void SetRunNote(const Char_t* rnote) { strcpy(fRunNote, rnote); }
    void SetSize(Long64_t size) { fSize = size; }
    void SetTarget(const Char_t* target) { strcpy(fTarget, target); }
    void SetTargetPol(const Char_t* targetPol) { strcpy(fTargetPol, targetPol); }
    void SetTargetPolDeg(Double_t deg) { fTargetPolDeg = deg; }
    void SetBeamPol(const Char_t* beamPol) { strcpy(fBeamPol, beamPol); }
    void SetBeamPolDeg(Double_t deg) { fBeamPolDeg = deg; }

    Int_t GetRun() const { return fRun; }
    const Char_t* GetPath() const { return fPath; }
    const Char_t* GetFileName() const { return fFileName; }
    const Char_t* GetTime() const { return fTime; }
    const Char_t* GetDescription() const { return fDescription; }
    const Char_t* GetRunNote() const { return fRunNote; }
    Long64_t GetSize() const { return fSize; }
    const Char_t* GetTarget() const { return fTarget; }
    const Char_t* GetTargetPol() const { return fTargetPol; }
    Double_t GetTargetPolDeg() const { return fTargetPolDeg; }
    const Char_t* GetBeamPol() const { return fBeamPol; }
    Double_t GetBeamPolDeg() const { return fBeamPolDeg; }
    
    void Print()
    {
        printf("CaLib Run Information\n");
        printf("Run              : %d\n", fRun);
        printf("Path             : %s\n", fPath);
        printf("File name        : %s\n", fFileName);
        printf("Time             : %s\n", fTime);
        printf("Description      : %s\n", fDescription);
        printf("Run note         : %s\n", fRunNote);
        printf("Size in bytes    : %lld\n", fSize);
        printf("Target           : %s\n", fTarget);
        printf("Target pol.      : %s\n", fTargetPol);
        printf("Target pol. deg. : %lf\n", fTargetPolDeg);
        printf("Beam pol.        : %s\n", fBeamPol);
        printf("Beam pol. deg.   : %lf\n", fBeamPolDeg);
        printf("\n");
    }

    ClassDef(TCRun, 1) // Run storage class
};


class TCCalibration : public TObject
{

private:
    Char_t fData[256];                  // calibration data type
    Char_t fCalibration[256];           // name
    Char_t fDescription[256];           // description
    Int_t fFirstRun;                    // first run
    Int_t fLastRun;                     // last run
    Char_t fChangeTime[64];             // fill time
    Int_t fNpar;                        // number of parameters
    Double_t* fPar;                     //[fNpar] parameters

public:
    TCCalibration() : TObject() 
    {
        fData[0] = '\0';
        fCalibration[0] = '\0';
        fDescription[0] = '\0';
        fFirstRun = 0;
        fLastRun = 0;
        fChangeTime[0] = '\0';
        fNpar = 0;
        fPar = 0;
    }
    virtual ~TCCalibration() { if (fPar) delete [] fPar; }

    void SetCalibData(const Char_t* data)  { strcpy(fData, data); }
    void SetCalibration(const Char_t* calib) { strcpy(fCalibration, calib); }
    void SetDescription(const Char_t* desc) { strcpy(fDescription, desc); }
    void SetFirstRun(Int_t run) { fFirstRun = run; }
    void SetLastRun(Int_t run) { fLastRun = run; }
    void SetChangeTime(const Char_t* ctime) { strcpy(fChangeTime, ctime); }
    void SetParameters(Int_t npar, Double_t* par)
    {
        fNpar = npar;
        if (fPar) delete [] fPar;
        fPar = new Double_t[fNpar];
        for (Int_t i = 0; i < fNpar; i++) fPar[i] = par[i];
    }

    const Char_t* GetCalibData() const { return fData; }
    const Char_t* GetCalibration() const { return fCalibration; }
    const Char_t* GetDescription() const { return fDescription; }
    Int_t GetFirstRun() const { return fFirstRun; }
    Int_t GetLastRun() const { return fLastRun; }
    const Char_t* GetChangeTime() const { return fChangeTime; }
    Int_t GetNParameters() const { return fNpar; }
    Double_t* GetParameters() const { return fPar; }
    
    void Print()
    {
        printf("CaLib Calibration Information\n");
        printf("Calibration data : %s\n", fData);
        printf("Calibration      : %s\n", fCalibration);
        printf("Description      : %s\n", fDescription);
        printf("First run        : %d\n", fFirstRun);
        printf("Last run         : %d\n", fLastRun);
        printf("Change time      : %s\n", fChangeTime);
        printf("Number of par.   : %d\n", fNpar);
        for (Int_t i = 0; i < fNpar; i++) printf("Par_%03d          : %.17g\n", i, fPar[i]);
        printf("\n");
        printf("\n");
    }
    
    ClassDef(TCCalibration, 1) // Calibration storage class
};


class TCContainer : public TNamed
{

private:
    Int_t fVersion;                 // container format version
    TList* fRuns;                   //-> run list
    TList* fCalibrations;           //-> calibration list

public:
    TCContainer() : TNamed(), fVersion(0), fRuns(0), fCalibrations(0) { }
    TCContainer(const Char_t* name);
    virtual ~TCContainer();
    
    Int_t GetVersion() const { return fVersion; }
    TList* GetRuns() const { return fRuns; }
    Int_t GetNRuns() const { return fRuns ? fRuns->GetSize() : 0; }
    TCRun* GetRun(Int_t n) const { return fRuns ? (TCRun*)fRuns->At(n) : 0; }
    TList* GetCalibrations() const { return fCalibrations; }
    Int_t GetNCalibrations() const { return fCalibrations ? fCalibrations->GetSize() : 0; }
    TCCalibration* GetCalibration(Int_t n) const { return fCalibrations ? (TCCalibration*)fCalibrations->At(n) : 0; }

    TCRun* AddRun(Int_t run);
    TCCalibration* AddCalibration(const Char_t* calibration);
    Bool_t SaveAs(const Char_t* filename, Bool_t silence = kFALSE);
 
    void Print();
    void ShowRuns();
    void ShowCalibrations();

    ClassDef(TCContainer, 1) // CaLib storage class
};

#endif

