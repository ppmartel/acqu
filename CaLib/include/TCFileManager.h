// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCFileManager                                                        //
//                                                                      //
// Histogram building class.                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCFILEMANAGER_H
#define TCFILEMANAGER_H

#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <list>
#include <libgen.h>  // basename(), dirname()

#include "TFile.h"
#include "TH1.h"

#include "TCReadConfig.h"
#include "TCMySQLManager.h"


class TCFileManager
{

private:
    TString fInputFilePatt;                 // input file pattern
    TList* fFiles;                          // list of files
    TString fCalibData;                     // calibration data
    TString fCalibration;                   // calibration identifier
    Int_t fNset;                            // number of sets
    Int_t* fSet;                            //[fNset] array of set numbers

    void BuildFileList();
    // used to read in MC files
    const char* join_path(const char* path1, const char* path2, const char* path_sep);
    void list_files(const char* path, std::list<std::string>& file_list);
    void filter_list(std::list<std::string>& list, const char* pattern);
    const char* get_real_path(const char* path);

public:
    TCFileManager() : fInputFilePatt(0), fFiles(0),
                      fCalibData(), fCalibration(), fNset(0), fSet(0) { }
    TCFileManager(const Char_t* data, const Char_t* calibration,
                  Int_t nSet, Int_t* set, const Char_t* filePat = 0);
    virtual ~TCFileManager();

    TH1* GetHistogram(const Char_t* name);

    ClassDef(TCFileManager, 0) // Histogram building class
};

#endif

