// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibType                                                          //
//                                                                      //
// Calibration type class.                                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTYPE_H
#define TCCALIBTYPE_H

#include "TList.h"

#include "TCCalibData.h"


class TCCalibType : public TNamed
{

private:
    TList* fData;                   // list of associated data (enties not owned)
    
public:
    TCCalibType() : TNamed() { }
    TCCalibType(const Char_t* name, const Char_t* title);
    virtual ~TCCalibType();
 
    TList* GetData() const { return fData; }
    TCCalibData* GetData(Int_t n) { return fData ? (TCCalibData*)fData->At(n) : 0; }

    void AddData(TCCalibData* data) { if (fData) fData->Add(data); }
    void Print();

    ClassDef(TCCalibType, 1) // Calibration type class
};

#endif

