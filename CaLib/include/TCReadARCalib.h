// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadARCalib                                                        //
//                                                                      //
// Read AcquRoot calibration files.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCREADARCALIB_H
#define TCREADARCALIB_H

#include <fstream>
#include <sstream>

#include "TSystem.h"
#include "TString.h"
#include "TList.h"
#include "TError.h"


class TCARElement : public TObject
{

private:
    Bool_t fIsTagger;                       // tagger toggle
    Char_t fADC[16];                        // ADC identifier
    Double_t fEnergyLow;                    // energy low threshold
    Double_t fEnergyHigh;                   // energy high threshold
    Double_t fPed;                          // ADC pedestal
    Double_t fADCGain;                      // ADC gain
    Char_t fTDC[16];                        // TDC identifier
    Double_t fTimeLow;                      // time low threshold
    Double_t fTimeHigh;                     // time high threshold
    Double_t fOffset;                       // TDC offset
    Double_t fTDCGain;                      // TDC gain
    Double_t fX;                            // x coordinate
    Double_t fY;                            // y coordinate
    Double_t fZ;                            // z coordinate
    Double_t fTaggCalib;                    // tagger calibration
    Double_t fTaggOverlap;                  // tagger overlap
    Int_t fTaggScaler;                      // tagger scaler

public:
    TCARElement() : TObject()
    {
        fIsTagger = kFALSE;
        fADC[0] = '\0';
        fEnergyLow = 0;
        fEnergyHigh = 0;
        fPed = 0;
        fADCGain = 0;
        fTDC[0] = '\0';
        fTimeLow = 0;
        fTimeHigh = 0;
        fOffset = 0;
        fTDCGain = 0;
        fX = 0;
        fY = 0;
        fZ = 0;
        fTaggCalib = 0;
        fTaggOverlap = 0;
        fTaggScaler = 0;
    }
    virtual ~TCARElement() { }

    Bool_t Parse(const Char_t* line, Bool_t isTagger)
    {   
        // set tagger toggle
        fIsTagger = isTagger;

        // tagger calibration file or not
        if (fIsTagger)
        {
            // read the calibration line
            Int_t n = sscanf(line, "%*s%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%d", 
                             fADC, &fEnergyLow, &fEnergyHigh, &fPed, &fADCGain, 
                             fTDC, &fTimeLow, &fTimeHigh, &fOffset, &fTDCGain, 
                             &fX, &fY, &fZ, &fTaggCalib, &fTaggOverlap, &fTaggScaler);

            // check read-in
            return n == 16 ? kTRUE : kFALSE;
        }
        else
        {
            // read the calibration line
            Int_t n = sscanf(line, "%*s%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf", 
                             fADC, &fEnergyLow, &fEnergyHigh, &fPed, &fADCGain, 
                             fTDC, &fTimeLow, &fTimeHigh, &fOffset, &fTDCGain, 
                             &fX, &fY, &fZ);

            // check read-in
            return n == 13 ? kTRUE : kFALSE;
        }
    }

    void Format(Char_t* out)
    {   
        // tagger calibration file or not
        if (fIsTagger)
        {
            // write the calibration line
            sprintf(out, "%7s %6.3lf %6.1lf %7.2lf %8.6lf "
                         "%7s %7.1lf %7.1lf %8.2lf %8.6lf "
                         "%8.3lf %8.3lf %8.3lf %8.3lf %5.3lf %3d", 
                         fADC, fEnergyLow, fEnergyHigh, fPed, fADCGain, 
                         fTDC, fTimeLow, fTimeHigh, fOffset, fTDCGain, 
                         fX, fY, fZ, fTaggCalib, fTaggOverlap, fTaggScaler);
        }
        else
        {
            // write the calibration line
            sprintf(out, "%7s %6.3lf %6.1lf %7.2lf %8.6lf "
                         "%7s %7.1lf %7.1lf %8.2lf %8.6lf "
                         "%8.3lf %8.3lf %8.3lf", 
                         fADC, fEnergyLow, fEnergyHigh, fPed, fADCGain, 
                         fTDC, fTimeLow, fTimeHigh, fOffset, fTDCGain, 
                         fX, fY, fZ);
        }
    }

    Bool_t IsTagger() const { return fIsTagger; }
    const Char_t* GetADC() const { return fADC; }
    Double_t GetEnergyLow() const { return fEnergyLow; }
    Double_t GetEnergyHigh() const { return fEnergyHigh; }
    Double_t GetPedestal() const { return fPed; }
    Double_t GetADCGain() const { return fADCGain; }
    const Char_t* GetTDC() const { return fTDC; }
    Double_t GetTimeLow() const { return fTimeLow; }
    Double_t GetTimeHigh() const { return fTimeHigh; }
    Double_t GetOffset() const { return fOffset; }
    Double_t GetTDCGain() const { return fTDCGain; }
    Double_t GetX() const { return fX; }
    Double_t GetY() const { return fY; }
    Double_t GetZ() const { return fZ; }
    Double_t GetTaggCalib() const { return fTaggCalib; }
    Double_t GetTaggOverlap() const { return fTaggOverlap; }
    Int_t GetTaggScaler() const { return fTaggScaler; }
 
    void SetIsTagger(Bool_t b) { fIsTagger = b; }
    void SetADC(const Char_t* adc) { strcpy(fADC, adc); }
    void SetEnergyLow(Double_t low) { fEnergyLow = low; }
    void SetEnergyHigh(Double_t high) { fEnergyHigh = high; }
    void SetPedestal(Double_t ped) { fPed = ped; }
    void SetADCGain(Double_t gain) { fADCGain = gain; }
    void SetTDC(const Char_t* tdc) { strcpy(fTDC, tdc); }
    void SetTimeLow(Double_t low) { fEnergyLow = low; }
    void SetTimeHigh(Double_t high) { fEnergyHigh = high; }
    void SetOffset(Double_t off) { fOffset = off; }
    void SetTDCGain(Double_t gain) { fTDCGain = gain; }
    void SetX(Double_t x) { fX = x; }
    void SetY(Double_t y) { fY = y; }
    void SetZ(Double_t z) { fZ = z; }
    void SetTaggCalib(Double_t c) { fTaggCalib = c; }
    void SetTaggOverlap(Double_t o) { fTaggOverlap = o; }
    void SetTaggScaler(Int_t s) { fTaggScaler = s; }

    ClassDef(TCARElement, 0) // Class for element statements in AcquRoot config files
};


class TCARTimeWalk : public TObject
{

private:
    Int_t fIndex;                       // element index
    Double_t fPar0;                     // parameter 0
    Double_t fPar1;                     // parameter 1
    Double_t fPar2;                     // parameter 2
    Double_t fPar3;                     // parameter 3

public:
    TCARTimeWalk() : TObject(), fIndex(0), fPar0(0), fPar1(0), 
                                fPar2(0), fPar3(0) { }
    virtual ~TCARTimeWalk() { }

    Bool_t Parse(const Char_t* line)
    {   
        // read the calibration line
        Int_t n = sscanf(line, "%*s%d%lf%lf%lf%lf", 
                               &fIndex, &fPar0, &fPar1, &fPar2, &fPar3);
        
        // check read-in
        return n == 5 ? kTRUE : kFALSE;
    }

    void Format(Char_t* out)
    {   
        // write the calibration line
        sprintf(out, "%3d %11.6lf %11.6lf %11.6lf %11.6lf",
                     fIndex, fPar0, fPar1, fPar2, fPar3);
    }

    Int_t GetIndex() const { return fIndex; }
    Double_t GetPar0() const { return fPar0; }
    Double_t GetPar1() const { return fPar1; }
    Double_t GetPar2() const { return fPar2; }
    Double_t GetPar3() const { return fPar3; }
    
    void SetIndex(Int_t i ) { fIndex = i; }
    void SetPar0(Double_t p) { fPar0 = p; }
    void SetPar1(Double_t p) { fPar1 = p; }
    void SetPar2(Double_t p) { fPar2 = p; }
    void SetPar3(Double_t p) { fPar3 = p; }

    ClassDef(TCARTimeWalk, 0) // Class for time walk statements in AcquRoot config files
};


class TCARNeighbours : public TObject
{

private:
    Int_t fNneighbours;             // number of neighbours
    Int_t* fNeighbours;             // list of neighbours

public:
    TCARNeighbours() : TObject(), fNneighbours(0), fNeighbours(0) { }
    virtual ~TCARNeighbours()
    {
        if (fNeighbours) delete [] fNeighbours;
    }

    Bool_t Parse(const Char_t* line)
    {
        // create stringstream
        std::istringstream iss(line);
        
        // skip tag
        std::string tag;
        iss >> tag;

        // read number of neighbours
        iss >> fNneighbours;

        // create array
        if (fNeighbours) delete [] fNeighbours;
        fNeighbours = new Int_t[fNneighbours];
        
        // read neighbours
        for (Int_t i = 0; i < fNneighbours; i++) 
            iss >> fNeighbours[i];
        
        return kTRUE;
    }

    Int_t GetNneighbours() const { return fNneighbours; }
    Int_t* GetNeighbours() const { return fNeighbours; }
    Int_t GetNeighbour(Int_t n) const { return fNeighbours ? fNeighbours[n] : 0; }

    ClassDef(TCARNeighbours, 0) // Class for neighbour statements in AcquRoot config files
};


class TCReadARCalib
{

private:
    TList* fElements;               // list of detector elements
    TList* fTimeWalks;              // list of time walk elements
    TList* fNeighbours;             // list of neighbour statements

    void ReadCalibFile(const Char_t* filename, Bool_t isTagger,
                       const Char_t* elemIdent, const Char_t* nebrIdent);

public:
    TCReadARCalib() : fElements(0), fTimeWalks(0), fNeighbours(0) { }
    TCReadARCalib(const Char_t* calibFile, Bool_t isTagger,
                  const Char_t* elemIdent = "Element:", const Char_t* nebrIdent = "Next-Neighbour:");
    virtual ~TCReadARCalib();
    
    TList* GetElements() const { return fElements; }
    Int_t GetNelements() const { return fElements ? fElements->GetSize() : 0; }
    TCARElement* GetElement(Int_t n) const { return fElements ? (TCARElement*)fElements->At(n) : 0; }
    TList* GetTimeWalks() const { return fTimeWalks; }
    Int_t GetNtimeWalks() const { return fTimeWalks ? fTimeWalks->GetSize() : 0; }
    TCARTimeWalk* GetTimeWalk(Int_t n) const { return fTimeWalks ? (TCARTimeWalk*)fTimeWalks->At(n) : 0; }
    TList* GetNeighbours() const { return fNeighbours; }
    Int_t GetNneighbours() const { return fNeighbours ? fNeighbours->GetSize() : 0; }
    TCARNeighbours* GetNeighbour(Int_t n) const { return fNeighbours ? (TCARNeighbours*)fNeighbours->At(n) : 0; }

    ClassDef(TCReadARCalib, 0) // AcquRoot calibration file reader
};

#endif

