// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibData                                                          //
//                                                                      //
// Calibration data class.                                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBDATA_H
#define TCCALIBDATA_H

#include "TNamed.h"


class TCCalibData : public TNamed
{

private:
    Int_t fSize;                // number of calibration parameters
    TString fTableName;         // name of the database table

public:
    TCCalibData() : TNamed() { }
    TCCalibData(const Char_t* name, const Char_t* title, Int_t nSize);
    virtual ~TCCalibData() { }
 
    Int_t GetSize() const { return fSize; }
    const Char_t* GetTableName() const { return fTableName.Data(); }

    void SetTableName(TString tName) { fTableName = tName; }
    
    void Print();
    virtual ULong_t Hash() const { return fName.Hash(); }

    ClassDef(TCCalibData, 1) // Calibration data class
};

#endif

