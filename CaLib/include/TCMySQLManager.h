// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCMySQLManager                                                       //
//                                                                      //
// This class handles all the communication with the MySQL server.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCMYSQLMANAGER_H
#define TCMYSQLMANAGER_H

#include <cstdlib>

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TList.h"
#include "TError.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "THashList.h"

#include "TCConfig.h"
#include "TCCalibType.h"
#include "TCReadConfig.h"
#include "TCReadACQU.h"
#include "TCReadARCalib.h"
#include "TCContainer.h"


class TCMySQLManager
{

private:
    TSQLServer* fDB;                            // MySQL database conneciton
    Bool_t fSilence;                            // silence mode toggle
    THashList* fData;                           // calibration data
    THashList* fTypes;                          // calibration types
    static TCMySQLManager* fgMySQLManager;      // pointer to static instance of this class
    
    Bool_t ReadCaLibData();
    Bool_t ReadCaLibTypes();
   
    TSQLResult* SendQuery(const Char_t* query);
    
    Bool_t SearchTable(const Char_t* data, Char_t* outTableName);
    Bool_t SearchRunEntry(Int_t run, const Char_t* name, Char_t* outInfo);
    Bool_t SearchSetEntry(const Char_t* data, const Char_t* calibration, Int_t set,
                          const Char_t* name, Char_t* outInfo);
    TList* SearchDistinctEntries(const Char_t* data, const Char_t* table);
    
    Bool_t ChangeRunEntries(Int_t first_run, Int_t last_run, 
                            const Char_t* name, const Char_t* value);
    Bool_t ChangeSetEntry(const Char_t* data, const Char_t* calibration, Int_t set,
                          const Char_t* name, const Char_t* value);
    
    Bool_t AddDataSet(const Char_t* data, const Char_t* calibration, const Char_t* desc,
                      Int_t first_run, Int_t last_run, Double_t* par, Int_t length);
    Bool_t AddDataSet(const Char_t* data, const Char_t* calibration, const Char_t* desc,
                      Int_t first_run, Int_t last_run, Double_t par);
    Bool_t SplitDataSet(const Char_t* data, const Char_t* calibration, Int_t set,
                        Int_t lastRunFirstSet);
    Bool_t MergeDataSets(const Char_t* data, const Char_t* calibration, 
                         Int_t set1, Int_t set2);

public:
    TCMySQLManager();
    virtual ~TCMySQLManager();

    void SetSilenceMode(Bool_t s) { fSilence = s; }
    Bool_t IsConnected();
    
    const Char_t* GetDBName() const { return fDB ? fDB->GetDB() : 0; }
    const Char_t* GetDBHost() const { return fDB ? fDB->GetHost() : 0; }
    THashList* GetDataTable() const { return fData; }
    THashList* GetTypeTable() const { return fTypes; }
    
    void CreateMainTable();
    void CreateDataTable(const Char_t* data, Int_t nElem);
 
    TList* GetAllCalibrations(const Char_t* data = "Data.Tagger.T0");
    TList* GetAllTargets();
    
    Bool_t ContainsCalibration(const Char_t* calibration);

    Int_t GetNsets(const Char_t* data, const Char_t* calibration);
    Int_t GetFirstRunOfSet(const Char_t* data, const Char_t* calibration, Int_t set);
    Int_t GetLastRunOfSet(const Char_t* data, const Char_t* calibration, Int_t set);
    void GetDescriptionOfSet(const Char_t* data, const Char_t* calibration, 
                             Int_t set, Char_t* outDesc);
    void GetChangeTimeOfSet(const Char_t* data, const Char_t* calibration, 
                            Int_t set, Char_t* outTime);
    Int_t* GetRunsOfSet(const Char_t* data, const Char_t* calibration,
                        Int_t set, Int_t* outNruns);
    Int_t GetSetForRun(const Char_t* data, const Char_t* calibration, Int_t run);

    Bool_t ReadParameters(const Char_t* data, const Char_t* calibration, Int_t set, 
                          Double_t* par, Int_t length);
    Bool_t ReadParametersRun(const Char_t* data, const Char_t* calibration, Int_t run, 
                             Double_t* par, Int_t length);
    Bool_t WriteParameters(const Char_t* data, const Char_t* calibration, Int_t set, 
                           Double_t* par, Int_t length);
    
    Bool_t ChangeRunPath(Int_t first_run, Int_t last_run, const Char_t* path);
    Bool_t ChangeRunTarget(Int_t first_run, Int_t last_run, const Char_t* target);
    Bool_t ChangeRunTargetPol(Int_t first_run, Int_t last_run, const Char_t* target_pol);
    Bool_t ChangeRunTargetPolDeg(Int_t first_run, Int_t last_run, Double_t target_pol_deg);
    Bool_t ChangeRunBeamPol(Int_t first_run, Int_t last_run, const Char_t* beam_pol);
    Bool_t ChangeRunBeamPolDeg(Int_t first_run, Int_t last_run, Double_t beam_pol_deg);
    
    Bool_t ChangeCalibrationName(const Char_t* calibration, const Char_t* newCalibration);
    Bool_t RemoveCalibration(const Char_t* calibration, const Char_t* data);
    Int_t RemoveAllCalibrations(const Char_t* calibration);

    Bool_t AddSet(const Char_t* type, const Char_t* calibration, const Char_t* desc,
                  Int_t first_run, Int_t last_run, Double_t par);
    Bool_t RemoveSet(const Char_t* type, const Char_t* calibration, Int_t set);
    Bool_t SplitSet(const Char_t* type, const Char_t* calibration, Int_t set,
                    Int_t lastRunFirstSet);
    Bool_t MergeSets(const Char_t* type, const Char_t* calibration, 
                     Int_t set1, Int_t set2);

    void AddRunFiles(const Char_t* path, const Char_t* target);
    void AddRun(Int_t run, const Char_t* target, const Char_t* desc);
    void AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                    const Char_t* calib, const Char_t* desc,
                    Int_t first_run, Int_t last_run);
    
    void InitDatabase();
    
    TCContainer* LoadContainer(const Char_t* filename);
    
    Int_t DumpRuns(TCContainer* container, Int_t first_run = -1, Int_t last_run = -1);
    Int_t DumpAllCalibrations(TCContainer* container, const Char_t* calibration);
    Int_t DumpCalibrations(TCContainer* container, const Char_t* calibration, 
                           const Char_t* data);
    
    Int_t ImportRuns(TCContainer* container);
    Int_t ImportCalibrations(TCContainer* container, const Char_t* newCalibName = 0,
                             const Char_t* data = 0);
    void CloneCalibration(const Char_t* calibration, const Char_t* newCalibrationName,
                          const Char_t* newDesc, Int_t new_first_run, Int_t new_last_run);
    void Export(const Char_t* filename, Int_t first_run, Int_t last_run, 
                const Char_t* calibration);
    void Import(const Char_t* filename, Bool_t runs, Bool_t calibrations,
                const Char_t* newCalibName = 0);
    Bool_t RemoveDataSet(const Char_t* data, const Char_t* calibration, Int_t set);

    static TCMySQLManager* GetManager()
    {
        // return a pointer to the static instance of this class
        if (!fgMySQLManager) fgMySQLManager = new TCMySQLManager();
        if (!fgMySQLManager->IsConnected())
        {
            Error("GetManager", "No connection to the database!");
            return 0;
        }
        else return fgMySQLManager;
    }
    
    ClassDef(TCMySQLManager, 0) // Communication with MySQL Server
};

#endif

