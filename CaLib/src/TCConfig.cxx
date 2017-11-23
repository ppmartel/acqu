// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCConfig                                                             //
//                                                                      //
// CaLib configuration namespace                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCConfig.h"


namespace TCConfig
{   
    // detector elements
    const Int_t kMaxCrystal   = 720;
    const Int_t kMaxCB        = 720;
    const Int_t kMaxTAPS      = 510;
    const Int_t kMaxPID       =  24;
    const Int_t kMaxVeto      = 510;
    const Int_t kMaxTAGGER    = 352;
    
    // name of the main table
    const Char_t* kCalibMainTableName = "run_main";

    // format of the main table
    const Char_t* kCalibMainTableFormat = 
                    "run INT NOT NULL,"
                    "path VARCHAR(256),"
                    "filename VARCHAR(256),"
                    "time DATETIME DEFAULT '1900-01-01 00:00:00',"
                    "description VARCHAR(256),"
                    "run_note VARCHAR(256),"
                    "size BIGINT DEFAULT 0,"
                    "target VARCHAR(20),"
                    "target_pol VARCHAR(128),"
                    "target_pol_deg DOUBLE DEFAULT 0,"
                    "beam_pol VARCHAR(128),"
                    "beam_pol_deg DOUBLE DEFAULT 0,"
                    "changed TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                  ON UPDATE CURRENT_TIMESTAMP,"
                    "PRIMARY KEY (run) ";

    // header of the data tables
    const Char_t* kCalibDataTableHeader =
                    "calibration VARCHAR(256),"
                    "description VARCHAR(1024),"
                    "first_run INT NOT NULL,"
                    "last_run INT NOT NULL,"
                    "changed TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                  ON UPDATE CURRENT_TIMESTAMP,";
    
    // additional settings for the data tables
    const Char_t* kCalibDataTableSettings = ",PRIMARY KEY (calibration, first_run) ";
    
    // version numbers
    const Char_t kCaLibVersion[] = "0.1.11";
    const Int_t kContainerFormatVersion = 2;
    extern const Char_t kCaLibDumpName[] = "CaLib_Dump";

    // constants
    const Double_t kPi0Mass = 134.9766;
    const Double_t kEtaMass = 547.853;
}

