/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2009
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOArray2D                                                            //
//                                                                      //
// Serializable 2-dim. array for Double_t data types.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOArray2D
#define OSCAR_TOArray2D

#include "TNamed.h"
#include "TBuffer.h"

class TOArray2D : public TNamed
{

protected:
    Int_t fNcol;                                   // Number of columns (1st dimension)
    Int_t fNrow;                                   // Number of rows (2nd dimension)
    Double_t** fData;                              //[fNcol][fNrow] Data array

    Bool_t BoundsOk(const Char_t* inLoc, Int_t inColumn, Int_t inRow) const;

public:
    TOArray2D(): TNamed(), fNcol(0), fNrow(0), fData(0) { }
    TOArray2D(Int_t inColumns, Int_t inRows, const Char_t* inName = ""); 
    virtual ~TOArray2D();
    
    void Set(Int_t inColumn, Int_t inRow, Double_t inValue);
    virtual void ResetNaN(Double_t reset = 0);

    Int_t GetNcol() const { return fNcol; }
    Int_t GetNrow() const { return fNrow; }
    Double_t Get(Int_t inColumn, Int_t inRow) const;
    Double_t* GetColumn(Int_t inColumn) const;
    Double_t* GetSum(Int_t c1, Int_t c2, Double_t w1 = 1.0, Double_t w2 = 1.0) const;
    
    virtual void Scale(Double_t c);
    virtual void Abs();
    virtual Bool_t Add(TOArray2D* d, Double_t c = 1);
    virtual Bool_t Multiply(TOArray2D* d);
    virtual Bool_t Divide(TOArray2D* d);
    virtual Bool_t Max(TOArray2D* d);
    virtual Bool_t Min(TOArray2D* d);

    virtual void Print(const Char_t* format = "") const;
    virtual void PrintColumn(Int_t inColumn, const Char_t* format = "") const;
    virtual void PrintRow(Int_t inRow, const Char_t* format = "") const;

    ClassDef(TOArray2D, 1)  // Serializable 2-dim. array
};

#endif

