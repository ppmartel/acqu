// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCWriteARCalib                                                       //
//                                                                      //
// Write AcquRoot calibration files.                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCWRITEARCALIB_H
#define TCWRITEARCALIB_H

#include "TError.h"

#include "TCConfig.h"
#include "TCMySQLManager.h"


class TCWriteARCalib
{

private:
    CalibDetector_t fDetector;              // detector type
    Char_t fTemplate[256];                  // template calibration file

public:
    TCWriteARCalib() 
    {
        fDetector = kDETECTOR_NODET;
        fTemplate[0] = '\0';
    }
    TCWriteARCalib(CalibDetector_t det, const Char_t* templateFile);
    virtual ~TCWriteARCalib() { }
    
    void Write(const Char_t* calibFile, 
               const Char_t* calibration, Int_t run);
    
    ClassDef(TCWriteARCalib, 0) // AcquRoot calibration file writer
};

#endif

