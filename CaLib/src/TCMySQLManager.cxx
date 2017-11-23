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


#include "TCMySQLManager.h"



// init static class members
TCMySQLManager* TCMySQLManager::fgMySQLManager = 0;


//______________________________________________________________________________
TCMySQLManager::TCMySQLManager()
{
    // Constructor.

    MC = false;
    fDB = 0;
    fSilence = kFALSE;
    fData = new THashList();
    fData->SetOwner(kTRUE);
    fTypes = new THashList();
    fTypes->SetOwner(kTRUE);

    // read CaLib data
    if (!ReadCaLibData())
    {
        if (!fSilence) Error("TCMySQLManager", "Could not read the CaLib data definitions!");
        return;
    }

    // read CaLib types
    if (!ReadCaLibTypes())
    {
        if (!fSilence) Error("TCMySQLManager", "Could not read the CaLib type definitions!");
        return;
    }

    // get database configuration
    TString* strDBHost;
    TString* strDBName;
    TString* strDBUser;
    TString* strDBPass;

    // get database hostname
    if (!(strDBHost = TCReadConfig::GetReader()->GetConfig("DB.Host")))
    {
        if (!fSilence) Error("TCMySQLManager", "Database host not included in configuration file!");
        return;
    }

    // get database name
    if (!(strDBName = TCReadConfig::GetReader()->GetConfig("DB.Name")))
    {
        if (!fSilence) Error("TCMySQLManager", "Database name not included in configuration file!");
        return;
    }

    // get database user
    if (!(strDBUser = TCReadConfig::GetReader()->GetConfig("DB.User")))
    {
        if (!fSilence) Error("TCMySQLManager", "Database user not included in configuration file!");
        return;
    }
    
    // get database password
    if (!(strDBPass = TCReadConfig::GetReader()->GetConfig("DB.Pass")))
    {
        if (!fSilence) Error("TCMySQLManager", "Database password not included in configuration file!");
        return;
    }

    // open connection to MySQL server on localhost
    Char_t szMySQL[200];
    sprintf(szMySQL, "mysql://%s/%s", strDBHost->Data(), strDBName->Data());
    fDB = TSQLServer::Connect(szMySQL, strDBUser->Data(), strDBPass->Data());
    if (!fDB)
    {
        if (!fSilence) Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else if (fDB->IsZombie())
    {
        if (!fSilence) Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else
    {
        if (!fSilence) Info("TCMySQLManager", "Connected to the database '%s' on '%s@%s' using CaLib %s",
                            strDBName->Data(), strDBUser->Data(), strDBHost->Data(), TCConfig::kCaLibVersion);
    }
}

//______________________________________________________________________________
TCMySQLManager::~TCMySQLManager()
{
    // Destructor.

    // close DB
    if (fDB) delete fDB;
    if (fData) delete fData;
    if (fTypes) delete fTypes;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadCaLibData()
{
    // Read the CaLib data definitions from the configuration file.
    // Return kTRUE on success, kFALSE if an error occured.
    
    // try to get the CaLib source path from the shell variable CALIB
    // otherwise use the current directory
    TString path;
    if (gSystem->Getenv("CALIB")) path = gSystem->Getenv("CALIB");
    else path = gSystem->pwd();
    path.Append("/data/data.def");

    // open the file
    std::ifstream infile;
    infile.open(path.Data());
        
    // check if file is open
    if (!infile.is_open())
    {
        if (!fSilence) Error("ReadCaLibData", "Could not open data definition file '%s'", path.Data());
        return kFALSE;
    }
    else
    {
        // read the file
        while (infile.good())
        {
            TString line;
            line.ReadLine(infile);
            
            // trim line
            line.Remove(TString::kBoth, ' ');
            
            // skip comments and empty lines
            if (line.BeginsWith("#") || line == "") continue;
            else
            {
                // tokenize the string
                TObjArray* token = line.Tokenize(",");
                
                // some variables for data extraction
                Int_t count = 0;
                TString name;
                TString title;
                TString table;
                Int_t size = 0;

                // iterate over tokens
                TIter next(token);
                TObjString* s;
                while ((s = (TObjString*)next()))
                {
                    // get the string and trim it
                    TString str(s->GetString());
                    str.Remove(TString::kBoth, ' ');
                    
                    // set data
                    switch (count)
                    {
                        case 0:
                            name = str;
                            break;
                        case 1:
                            title = str;
                            break;
                        case 2:
                            table = str;
                            break;
                        case 3:
                            size = atoi(str.Data());
                            break;
                    }

                    count++;
                }

                // clean-up
                delete token;
                
                // check number of tokens
                if (count == 4)
                {
                    // create a new CaLib data class
                    TCCalibData* data = new TCCalibData(name, title, size);
                    data->SetTableName(table);

                    // add it to list
                    fData->Add(data);
                }
                else
                {
                    if (!fSilence) Error("ReadCaLibData", "Error in CaLib data definition '%s'!", line.Data());
                    return kFALSE;
                }
            }
        }
    }
    
    // close the file
    infile.close();

    // user information
    if (!fSilence) Info("ReadCaLibData", "Registered %d data definitions", fData->GetSize());
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadCaLibTypes()
{
    // Read the CaLib type definitions from the configuration file.
    // Return kTRUE on success, kFALSE if an error occured.
    
    // try to get the CaLib source path from the shell variable CALIB
    // otherwise use the current directory
    TString path;
    if (gSystem->Getenv("CALIB")) path = gSystem->Getenv("CALIB");
    else path = gSystem->pwd();
    path.Append("/data/types.def");

    // open the file
    std::ifstream infile;
    infile.open(path.Data());
        
    // check if file is open
    if (!infile.is_open())
    {
        if (!fSilence) Error("ReadCaLibTypes", "Could not open types definition file '%s'", path.Data());
        return kFALSE;
    }
    else
    {
        // read the file
        while (infile.good())
        {
            TString line;
            line.ReadLine(infile);
            
            // trim line
            line.Remove(TString::kBoth, ' ');
            
            // skip comments and empty lines
            if (line.BeginsWith("#") || line == "") continue;
            else
            {
                // tokenize the string
                TObjArray* token = line.Tokenize(",");
                
                // some variables for data extraction
                Int_t count = 0;
                TString name;
                TString title;
                TCCalibType* type = 0;

                // iterate over tokens
                TIter next(token);
                TObjString* s;
                while ((s = (TObjString*)next()))
                {
                    // get the string and trim it
                    TString str(s->GetString());
                    str.Remove(TString::kBoth, ' ');
                    
                    // set name
                    if (count == 0) name = str;
                    else if (count == 1) title = str;
                    else
                    {   
                        // create type object if necessary
                        if (!type) type = new TCCalibType(name, title);
                        
                        // get calibration data
                        TCCalibData* d = (TCCalibData*) fData->FindObject(str.Data());

                        // add calibration data
                        if (d) type->AddData(d);
                        else
                        {
                            if (!fSilence) Error("ReadCaLibTypes", "CaLib data '%s' was not found!", str.Data());
                            return kFALSE;
                        }
                    }
                    
                    count++;
                }

                // clean-up
                delete token;

                // check number of tokens
                if (count > 2)
                {
                    // add type
                    fTypes->Add(type);
                }
                else
                {
                    if (!fSilence) Error("ReadCaLibTypes", "Error in CaLib type definition '%s'!", line.Data());
                    return kFALSE;
                }
            }
        }
    }
    
    // close the file
    infile.close();

    // user information
    if (!fSilence) Info("ReadCaLibTypes", "Registered %d type definitions", fTypes->GetSize());
    
    return kTRUE;
}

//______________________________________________________________________________
TSQLResult* TCMySQLManager::SendQuery(const Char_t* query)
{
    // Send a query to the database and return the result.

    // check server connection
    if (!IsConnected())
    {
        if (!fSilence) Error("SendQuery", "No connection to the database!");
        return 0;
    }

    // execute query
    return fDB->Query(query);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::IsConnected()
{
    // Check if the connection to the database is open.

    if (!fDB)
    {
        if (!fSilence) Error("IsConnected", "Cannot access database!");
        return kFALSE;
    }
    else
        return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchTable(const Char_t* data, Char_t* outTableName)
{
    // Search the table name of the calibration quantity 'data' and write it
    // to 'outTableName'.
    // Return kTRUE when a true table was found, otherwise kFALSE.
    
    // get CaLib data
    TCCalibData* d = (TCCalibData*) fData->FindObject(data);

    // check data
    if (d)
    {
        // copy table name
        strcpy(outTableName, d->GetTableName());
        return kTRUE;
    }
    else return kFALSE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchRunEntry(Int_t run, const Char_t* name, Char_t* outInfo)
{
    // Search the information 'name' for the run 'run' and write it to 'outInfo'.
    // Return kTRUE when the information was found, otherwise kFALSE.
    
    Char_t query[256];

    // create the query
    sprintf(query,
            "SELECT %s FROM %s "
            "WHERE run = %d",
            name, TCConfig::kCalibMainTableName, run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        if (!fSilence) Error("SearchRunEntry", "Could not find the information '%s' for run %d!",
                                               name, run);
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        if (!fSilence) Error("SearchRunEntry", "Could not find the information '%s' for run %d!",
                                               name, run);
        return kFALSE;
    }

    // get row
    TSQLRow* row = res->Next();
  
    // write the information
    const Char_t* field = row->GetField(0);
    if (!field) strcpy(outInfo, "");
    else strcpy(outInfo, field);

    // clean-up
    delete row;
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchSetEntry(const Char_t* data, const Char_t* calibration, Int_t set,
                                      const Char_t* name, Char_t* outInfo)
{
    // Search the information 'name' for the calibration identifier 'calibration' and 
    // the calibration data 'data' for the set number 'set' and write it to 'outInfo'.
    // Return kTRUE when the information was found, otherwise kFALSE.
  
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("SearchSetEntry", "No data table found!");
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "SELECT %s FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC LIMIT 1 OFFSET %d",
            name, table, calibration, set);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        if (!fSilence) Error("SearchSetEntry", "No runset %d found in table '%s'!", set, table);
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        if (!fSilence) Error("SearchSetEntry", "No runset %d found in table '%s'!", set, table);
        return kFALSE;
    }

    // get data
    TSQLRow* row = res->Next();
    const Char_t* d = row->GetField(0);
    if (!d) strcpy(outInfo, "");
    else strcpy(outInfo, d);

    // clean-up
    delete row;
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunEntries(Int_t first_run, Int_t last_run, 
                                        const Char_t* name, const Char_t* value)
{
    // Change the run entry 'name' for the runs 'first_run' to 'last_run' to 'value'.
    
    Char_t query[256];
    
    // check if first run is smaller than last run
    if (first_run > last_run)
    {
        if (!fSilence) Error("ChangeRunEntries", "First run of has to be smaller than last run!");
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "UPDATE %s SET %s = '%s' "
            "WHERE run >= %d AND"
            "      run <= %d",
            TCConfig::kCalibMainTableName, name, value, first_run, last_run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        if (!fSilence) Error("ChangeRunEntries", "Could not set the information '%s' for runs %d to %d to '%s'!",
                                                 name, first_run, last_run, value);
        return kFALSE;
    }

    // clean-up
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeSetEntry(const Char_t* data, const Char_t* calibration, Int_t set,
                                      const Char_t* name, const Char_t* value)
{
    // Change the information 'name' of the 'set'-th set of the calibration 'calibration'
    // for the calibration data 'data' to 'value'.
    
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("ChangeSetEntry", "No data table found!");
        return 0;
    }
    
    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // create the query
    sprintf(query,
            "UPDATE %s SET %s = '%s' "
            "WHERE calibration = '%s' AND "
            "first_run = %d",
            table, name, value, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        if (!fSilence) Error("ChangeSetEntry", "Could not set the information '%s' for set %d of '%s' to '%s'!",
                                               name, set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), value);
        return kFALSE;
    }

    // clean-up
    delete res;
 
    return kTRUE;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetNsets(const Char_t* data, const Char_t* calibration)
{
    // Get the number of runsets for the calibration identifier 'calibration'
    // and the calibration data 'data'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("GetNsets", "No data table found!");
        return 0;
    }

    // create the query
    sprintf(query,
            "SELECT first_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC",
            table, calibration);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        if (!fSilence) Error("GetNsets", "No runsets found in table '%s'!", table);
        return 0;
    }

    // count rows
    Int_t rows = res->GetRowCount();
    delete res;

    return rows;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetFirstRunOfSet(const Char_t* data, const Char_t* calibration, Int_t set)
{
    // Get the first run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "first_run", tmp)) return atoi(tmp);
    else 
    {
        if (!fSilence) Error("GetFirstRunOfSet", "Could not find first run of set!");
        return -1;
    }
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetLastRunOfSet(const Char_t* data, const Char_t* calibration, Int_t set)
{
    // Get the last run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "last_run", tmp)) return atoi(tmp);
    else 
    {
        if (!fSilence) Error("GetLastRunOfSet", "Could not find last run of set!");
        return -1;
    }
}

//______________________________________________________________________________
void TCMySQLManager::GetDescriptionOfSet(const Char_t* data, const Char_t* calibration, 
                                         Int_t set, Char_t* outDesc)
{
    // Get the description of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "description", tmp)) strcpy(outDesc, tmp);
    else 
    {
        if (!fSilence) Error("GetDescriptionOfSet", "Could not find description of set!");
    }
}

//______________________________________________________________________________
void TCMySQLManager::GetChangeTimeOfSet(const Char_t* data, const Char_t* calibration, 
                                        Int_t set, Char_t* outTime)
{
    // Get the change time of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "changed", tmp)) strcpy(outTime, tmp);
    else 
    {
        if (!fSilence) Error("GetChangeTimeOfSet", "Could not find change time of set!");
    }
}

//______________________________________________________________________________
Int_t* TCMySQLManager::GetRunsOfSet(const Char_t* data, const Char_t* calibration, 
                                    Int_t set, Int_t* outNruns = 0)
{
    // Return the list of runs that are in the set 'set' of the calibration data 'data'
    // for the calibration identifier 'calibration'.
    // If 'outNruns' is not zero the number of runs will be written to this variable.
    // NOTE: the run array must be destroyed by the caller.

    Char_t query[256];

    // get first and last run
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    Int_t last_run = GetLastRunOfSet(data, calibration, set);

    // check first run
    if (first_run < 0)
    {
        if (!fSilence) Error("GetRunsOfSet", "Could not find runs of set %d!", set);
        return 0;
    }

    //
    // get all the runs that lie between first and last run
    //

    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE time >= ( SELECT time FROM %s WHERE run = %d) "
            "AND time <= ( SELECT time FROM %s WHERE run = %d) "
            "ORDER by run,time",
            TCConfig::kCalibMainTableName, TCConfig::kCalibMainTableName, first_run, 
            TCConfig::kCalibMainTableName, last_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // get number of runs
    Int_t nruns = res->GetRowCount();

    // create run array
    Int_t* runs = new Int_t[nruns];

    // read all runs
    for (Int_t i = 0; i < nruns; i++)
    {
        // get next run
        TSQLRow* row = res->Next();

        // save run number
        runs[i] = atoi(row->GetField(0));

        // clean-up
        delete row;
    }

    // clean-up
    delete res;

    // write number of runs
    if (outNruns) *outNruns = nruns;

    return runs;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetSetForRun(const Char_t* data, const Char_t* calibration, Int_t run)
{
    // Return the number of the set of the calibration data 'data' for the calibration
    // identifier 'calibration' the run 'run' belongs to.
    // Return -1 if there is no such set.

    // get number of sets
    Int_t nSet = GetNsets(data, calibration);
    if (!nSet) return -1;
    
    // check if  run exists
    Char_t tmp[256];
    if (!SearchRunEntry(run, "run", tmp))
    {
        if (!fSilence) Error("GetSetForRun", "Run has no valid run number!");
        return -1;
    }
 
    // loop over sets
    for (Int_t i = 0; i < nSet; i++)
    {
        // get first and last run
        Int_t first_run = GetFirstRunOfSet(data, calibration, i);
        Int_t last_run = GetLastRunOfSet(data, calibration, i);

        // check if run is in this set
        if (run >= first_run && run <= last_run) return i;
    }

    // no set found here
    return -1;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadParametersRun(const Char_t* data, const Char_t* calibration, Int_t run, 
                                         Double_t* par, Int_t length)
{
    // Read 'length' parameters of the calibration data 'data' for the calibration identifier
    // 'calibration' valid for the run 'run' from the database to the value array 'par'.
    // Return kFALSE if an error occured, otherwise kTRUE.

    // get set
    Int_t set = GetSetForRun(data, calibration, run);

    // check set
    if (set == -1)
    {
        if (!fSilence) Error("ReadParametersRun", "No set of '%s' found for run %d",
                             ((TCCalibData*) fData->FindObject(data))->GetTitle(), run);
        return kFALSE;
    }

    // call main parameter reading method
    return ReadParameters(data, calibration, set, par, length);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadParameters(const Char_t* data, const Char_t* calibration, Int_t set, 
                                      Double_t* par, Int_t length)
{
    // Read 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the database to the value array 'par'.
    // Return kFALSE if an error occured, otherwise kTRUE.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("ReadParameters", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // check first run
    if (first_run < 0)
    {
        if (!fSilence) Error("ReadParameters", "No calibration found for set %d of '%s'!", 
                             set, ((TCCalibData*) fData->FindObject(data))->GetTitle());
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "SELECT * FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d",
            table, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        if (!fSilence) Error("ReadParameters", "No calibration found for set %d of '%s'!", 
                             set, ((TCCalibData*) fData->FindObject(data))->GetTitle());
        return kFALSE;
    }
    else if (!res->GetRowCount())
    {
        if (!fSilence) Error("ReadParameters", "No calibration found for set %d of '%s'!", 
                             set, ((TCCalibData*) fData->FindObject(data))->GetTitle());
        delete res;
        return kFALSE;
    }

    // get data (parameters start at field 5)
    TSQLRow* row = res->Next();
    for (Int_t i = 0; i < length; i++) par[i] = atof(row->GetField(i+5));

    // clean-up
    delete row;
    delete res;
    
    // user information
    if (!fSilence) Info("ReadParameters", "Read %d parameters of '%s' from the database", 
                        length, ((TCCalibData*) fData->FindObject(data))->GetTitle());
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::WriteParameters(const Char_t* data, const Char_t* calibration, Int_t set, 
                                       Double_t* par, Int_t length)
{
    // Write 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the value array 'par' to the database.
    // Return kFALSE if an error occured, otherwise kTRUE.

    Char_t table[256];
 
    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("WriteParameters", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // check first run
    if (first_run < 0)
    {
        if (!fSilence) Error("WriteParameters", "Could not write parameters of '%s'!",
                                                ((TCCalibData*) fData->FindObject(data))->GetTitle());
        return kFALSE;
    }

    // prepare the insert query
    TString query = TString::Format("UPDATE %s SET ", table);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        query.Append(TString::Format("par_%03d = %.17g", j, par[j]));
        if (j != length - 1) query.Append(",");
    }
    
    // finish query
    query.Append(TString::Format(" WHERE calibration = '%s' AND first_run = %d",
                                 calibration, first_run));
 
    // write data to database
    TSQLResult* res = SendQuery(query.Data());
    
    // check result
    if (!res)
    {
        if (!fSilence) Error("WriteParameters", "Could not write parameters of '%s'!", 
                             ((TCCalibData*) fData->FindObject(data))->GetTitle());
        return kFALSE;
    }
    else
    {
        delete res;
        if (!fSilence) Info("WriteParameters", "Wrote %d parameters of '%s' to the database", 
                                               length, ((TCCalibData*) fData->FindObject(data))->GetTitle());
        return kTRUE;
    }
}

//______________________________________________________________________________
void TCMySQLManager::InitDatabase()
{
    // Init a new CaLib database on a MySQL server.
    
    // ask for user confirmation
    Char_t answer[256];
    printf("\nWARNING: You are about to initialize a new CaLib database.\n"
           "         All existing tables in the database '%s' on '%s'\n"
           "         will be deleted!\n\n", fDB->GetDB(), fDB->GetHost());
    printf("Are you sure to continue? (yes/no) : ");
    if(scanf("%s", answer));
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        return;
    }
    
    // create the main table
    CreateMainTable();

    // create the data tables
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // create the data table
        CreateDataTable(d->GetName(), d->GetSize());
    }
}

//______________________________________________________________________________
void TCMySQLManager::AddRunFiles(const Char_t* path, const Char_t* target)
{
    // Look for raw ACQU files in 'path' and add all runs to the database
    // using the target specifier 'target'.

    // read the raw files
    TCReadACQU r(path);
    Int_t nRun = r.GetNFiles();
    
    // ask for user confirmation
    Char_t answer[256];
    printf("\n%d runs were found in '%s'\n"
           "They will be added to the database '%s' on '%s'\n", 
           nRun, path, fDB->GetDB(), fDB->GetHost());
    printf("Are you sure to continue? (yes/no) : ");
    if(scanf("%s", answer));
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        return;
    }

    // loop over runs
    Int_t nRunAdded = 0;
    for (Int_t i = 0; i < nRun; i++)
    {
        TCACQUFile* f = r.GetFile(i);
        
        // prepare the insert query
        TString ins_query = TString::Format("INSERT INTO %s SET "
                                            "run = %d, "
                                            "path = \"%s\", "
                                            "filename = \"%s\", "
                                            "time = STR_TO_DATE(\"%s\", \"%%a %%b %%d %%H:%%i:%%S %%Y\"), "
                                            "description = \"%s\", "
                                            "run_note = \"%s\", "
                                            "size = %lld,"
                                            "target = \"%s\"",
                                            TCConfig::kCalibMainTableName,
                                            f->GetRun(),
                                            path,
                                            f->GetFileName(),
                                            f->GetTime(),
                                            f->GetDescription(),
                                            f->GetRunNote(),
                                            f->GetSize(),
                                            target);

        // try to write data to database
        TSQLResult* res = SendQuery(ins_query.Data());
        if (res == 0)
        {
            Warning("AddRunFiles", "Run %d of file '%s/%s' could not be added to the database!", 
                    f->GetRun(), path, f->GetFileName());
        }
        else
        {
            nRunAdded++;
            delete res;
        }
    }

    // user information
    if (!fSilence) Info("AddRunFiles", "Added %d runs to the database", nRunAdded);
}

//______________________________________________________________________________
void TCMySQLManager::AddRun(Int_t run, const Char_t* target, const Char_t* desc)
{
    // Add a run to the run database using the run number 'run', the target
    // identifier 'target' and the description 'desc'.

    // prepare the insert query
    TString ins_query = TString::Format("INSERT INTO %s SET "
                                        "run = %d, "
                                        "description = '%s', "
                                        "target = '%s'",
                                        TCConfig::kCalibMainTableName, 
                                        run,
                                        desc,
                                        target);

    // try to write data to database
    TSQLResult* res = SendQuery(ins_query.Data());
    if (res == 0)
    {
        if (!fSilence) Warning("AddRun", "Run %d could not be added to the database!", run);
    }
    else
    {
        if (!fSilence) Info("AddRun", "Run %d ('%s') for target '%s' was added to the database", 
                            run, desc, target);
    }
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunPath(Int_t first_run, Int_t last_run, const Char_t* path)
{
    // Change the path of all runs between 'first_run' and 'last_run' to 'path'.

    // change path
    return ChangeRunEntries(first_run, last_run, "path", path);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunTarget(Int_t first_run, Int_t last_run, const Char_t* target)
{
    // Change the target of all runs between 'first_run' and 'last_run' to 'target'.

    // change target
    return ChangeRunEntries(first_run, last_run, "target", target);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunTargetPol(Int_t first_run, Int_t last_run, const Char_t* target_pol)
{
    // Change the target polarization of all runs between 'first_run' and 
    // 'last_run' to 'target_pol'.

    // change target polarization
    return ChangeRunEntries(first_run, last_run, "target_pol", target_pol);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunTargetPolDeg(Int_t first_run, Int_t last_run, Double_t target_pol_deg)
{
    // Change the degree of target polarization of all runs between 'first_run' and 
    // 'last_run' to 'target_pol_deg'.
    
    // create string
    Char_t tmp[256];
    sprintf(tmp, "%lf", target_pol_deg);

    // change degree of target polarization
    return ChangeRunEntries(first_run, last_run, "target_pol_deg", tmp);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunBeamPol(Int_t first_run, Int_t last_run, const Char_t* beam_pol)
{
    // Change the beam polarization of all runs between 'first_run' and 
    // 'last_run' to 'beam_pol'.

    // change beam polarization
    return ChangeRunEntries(first_run, last_run, "beam_pol", beam_pol);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeRunBeamPolDeg(Int_t first_run, Int_t last_run, Double_t beam_pol_deg)
{
    // Change the degree of beam polarization of all runs between 'first_run' and 
    // 'last_run' to 'beam_pol_deg'.
    
    // create string
    Char_t tmp[256];
    sprintf(tmp, "%lf", beam_pol_deg);

    // change degree of beam polarization
    return ChangeRunEntries(first_run, last_run, "beam_pol_deg", tmp);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ContainsCalibration(const Char_t* calibration)
{
    // Check if the calibration 'calibration' exists in the database.
    
    // loop over calibration data
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // get list of calibrations
        TList* list = GetAllCalibrations(d->GetName());
    
        // no calibrations
        if (!list) continue;
    
        // check if calibration exists
        TIter next(list);
        Bool_t found = kFALSE;
        TObjString* s;
        while ((s = (TObjString*)next()))
        {
            if (!strcmp(calibration, s->GetString().Data()))
            {
                found = kTRUE;
                break;
            }
        }
    
        // clean-up
        delete list;

        // check if calibration was found
        if (found) return kTRUE;
    }

    return kFALSE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeCalibrationName(const Char_t* calibration, const Char_t* newCalibration)
{
    // Change the calibration identifer 'calibration' in all calibration sets
    // to 'newCalibration'.
    
    Char_t query[256];
    
    // check if calibration was not found
    if (!ContainsCalibration(calibration))
    {
        if (!fSilence) Error("ChangeCalibrationName", "Calibration '%s' was not found in database!",
                             calibration);
        return kFALSE;
    }

    // loop over calibration data
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // create the query
        sprintf(query,
                "UPDATE %s SET calibration = '%s' "
                "WHERE calibration = '%s'",
                d->GetTableName(), newCalibration, calibration);

        // read from database
        TSQLResult* res = SendQuery(query);
        
        // check result
        if (!res)
        {
            if (!fSilence) Error("ChangeCalibrationName", "Could not rename calibration '%s' to '%s'!",
                                 calibration, newCalibration);
            return kFALSE;
        }

        // clean-up
        delete res;
    }
    
    if (!fSilence) Info("ChangeCalibrationName", "Renamed calibration '%s' to '%s'!",
                        calibration, newCalibration);
 
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::RemoveCalibration(const Char_t* calibration, const Char_t* data)
{
    // Remove the calibration data 'data' of the calibration 'calibration'.
    
    Char_t query[256];
    
    // check if calibration was not found
    if (!ContainsCalibration(calibration))
    {
        if (!fSilence) Error("RemoveCalibration", "Calibration '%s' of calibration '%s' was not found in database!",
                             ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "DELETE from %s "
            "WHERE calibration = '%s'",
            ((TCCalibData*) fData->FindObject(data))->GetTableName(), calibration);

    // read from database
    TSQLResult* res = SendQuery(query);
        
    // check result
    if (!res)
    {
        if (!fSilence) Error("RemoveCalibration", "Could not remove calibration '%s' of calibration '%s'!",
                             ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // clean-up
    delete res;
    
    if (!fSilence) Info("RemoveCalibration", "Removed calibration '%s' of calibration '%s'!",
                        ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
 
    return kTRUE;
}

//______________________________________________________________________________
Int_t TCMySQLManager::RemoveAllCalibrations(const Char_t* calibration)
{
    // Remove all calibrations with the calibration identifer 'calibration'.

    Int_t nCalib = 0;

    // loop over calibration data
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // remove calibration
        if (RemoveCalibration(calibration, d->GetName())) nCalib++;
    }

    return nCalib;
}

//______________________________________________________________________________
void TCMySQLManager::AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                                const Char_t* calib, const Char_t* desc,
                                Int_t first_run, Int_t last_run)
{
    // Read the calibration for the detector 'det' from the AcquRoot calibration
    // file 'calibFileAR' and create calibration sets for the runs 'first_run'
    // to 'last_run' using the calibration name 'calib' and the description
    // 'desc'.

    // read the calibration file
    TCReadARCalib r(calibFileAR, kFALSE);

    // get the number of elements
    Int_t nDet = r.GetNelements();

    // user information
    if (nDet) 
    {
        if (!fSilence) Info("AddCalibAR", "Found calibrations for %d detector elements", nDet);
    }
    else
    {
        if (!fSilence) Error("AddCalibAR", "No detector elements found in calibration file!");
        return;
    }

    // create generic parameter arrays
    Double_t eL[nDet];
    Double_t e0[nDet];
    Double_t e1[nDet];
    Double_t t0[nDet];
    Double_t t1[nDet];

    // read generic parameters
    for (Int_t i = 0; i < nDet; i++)
    {
        eL[i] = r.GetElement(i)->GetEnergyLow();
        if (!MC) {
            e0[i] = r.GetElement(i)->GetPedestal();
            e1[i] = r.GetElement(i)->GetADCGain();
            t0[i] = r.GetElement(i)->GetOffset();
            t1[i] = r.GetElement(i)->GetTDCGain();
        } else {
            e0[i] = 0.;
            e1[i] = 1.;
            t0[i] = 0.;
            t1[i] = 1.;
        }
    }

    // read detector specific calibration values
    // and write the data to the database (depends also
    // on the detector)
    switch (det)
    {
        // tagger
        case kDETECTOR_TAGG:
        {
            // write to database
            AddDataSet("Data.Tagger.T0", calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // CB
        case kDETECTOR_CB:
        {
            // write to database
            AddDataSet("Data.CB.E1", calib, desc, first_run, last_run, e1, nDet);
            AddDataSet("Data.CB.T0", calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // TAPS
        case kDETECTOR_TAPS:
        {
            // read the calibration file again for the SG stuff
            TCReadARCalib rSG(calibFileAR, kFALSE, "TAPSSG:");

            // get the number of elements
            Int_t nDetSG = rSG.GetNelements();

            // user information
            if (!nDetSG) 
            {
                if (!fSilence) Error("AddCalibAR", "No TAPS SG detector elements found in calibration file!");
                return;
            }

            // create SG parameter arrays
            Double_t e0SG[nDetSG];
            Double_t e1SG[nDetSG];
                
            // read SG parameters
            for (Int_t i = 0; i < nDetSG; i++)
            {
                if (!MC) {
                    e0SG[i] = rSG.GetElement(i)->GetPedestal();
                    e1SG[i] = rSG.GetElement(i)->GetADCGain();
                } else {
                    e0SG[i] = 0.;
                    e1SG[i] = 1.;
                }
            }

            // write to database
            AddDataSet("Data.TAPS.CFD", calib, desc, first_run, last_run, eL, nDet);
            AddDataSet("Data.TAPS.LG.E0", calib, desc, first_run, last_run, e0, nDet);
            AddDataSet("Data.TAPS.LG.E1", calib, desc, first_run, last_run, e1, nDet);
            AddDataSet("Data.TAPS.SG.E0", calib, desc, first_run, last_run, e0SG, nDetSG);
            AddDataSet("Data.TAPS.SG.E1", calib, desc, first_run, last_run, e1SG, nDetSG);
            AddDataSet("Data.TAPS.T0", calib, desc, first_run, last_run, t0, nDet);
            AddDataSet("Data.TAPS.T1", calib, desc, first_run, last_run, t1, nDet);
            
            break;
        }
        // PID
        case kDETECTOR_PID:
        {
            // create special parameter arrays
            Double_t phi[nDet];

            // read special parameters
            for (Int_t i = 0; i < nDet; i++)
            {
                phi[i] = r.GetElement(i)->GetZ();
            }

            // write to database
            AddDataSet("Data.PID.Phi", calib, desc, first_run, last_run, phi, nDet);
            AddDataSet("Data.PID.E0", calib, desc, first_run, last_run, e0, nDet);
            AddDataSet("Data.PID.E1", calib, desc, first_run, last_run, e1, nDet);
            AddDataSet("Data.PID.T0", calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // Veto
        case kDETECTOR_VETO:
        {
            // write to database
            AddDataSet("Data.Veto.LED", calib, desc, first_run, last_run, eL, nDet);
            AddDataSet("Data.Veto.E0", calib, desc, first_run, last_run, e0, nDet);
            AddDataSet("Data.Veto.E1", calib, desc, first_run, last_run, e1, nDet);
            AddDataSet("Data.Veto.T0", calib, desc, first_run, last_run, t0, nDet);
            AddDataSet("Data.Veto.T1", calib, desc, first_run, last_run, t1, nDet);
            
            break;
        }
        // no detector
        case kDETECTOR_NODET:
        {
            // do nothing
            break;
        }
    }
}

//______________________________________________________________________________
void TCMySQLManager::CreateMainTable()
{
    // Create the main table for CaLib.
    
    // user information
    if (!fSilence) Info("CreateMainTable", "Creating main CaLib table");

    // delete the old table if it exists
    TSQLResult* res = SendQuery(TString::Format("DROP TABLE IF EXISTS %s", TCConfig::kCalibMainTableName).Data());
    delete res;

    // create the table
    res = SendQuery(TString::Format("CREATE TABLE %s ( %s )", 
                                     TCConfig::kCalibMainTableName, TCConfig::kCalibMainTableFormat).Data());
    delete res;
}

//______________________________________________________________________________
void TCMySQLManager::CreateDataTable(const Char_t* data, Int_t nElem)
{
    // Create the table for the calibration data 'data' for 'nElem' elements.
    
    Char_t table[256];
    
    // get the table name
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("CreateDataTable", "No data table found!");
        return;
    }

    if (!fSilence) Info("CreateDataTable", "Adding data table '%s' for %d elements", table, nElem);
        
    // delete the old table if it exists
    TSQLResult* res = SendQuery(TString::Format("DROP TABLE IF EXISTS %s", table));
    delete res;

    // prepare CREATE TABLE query
    TString query;
    query.Append(TString::Format("CREATE TABLE %s ( %s ", table, TCConfig::kCalibDataTableHeader));

    // loop over elements
    for (Int_t j = 0; j < nElem; j++)
    {
        query.Append(TString::Format("par_%03d DOUBLE DEFAULT 0", j));
        if (j != nElem - 1) query.Append(", ");
    }

    // finish preparing the query
    query.Append(TCConfig::kCalibDataTableSettings);
    query.Append(" )");
    
    // submit the query
    res = SendQuery(query.Data());
    delete res;
}

//______________________________________________________________________________
TList* TCMySQLManager::SearchDistinctEntries(const Char_t* data, const Char_t* table)
{
    // Return a list of TStrings containing all distinct entries of the type
    // 'data' in the table 'table' of the CaLib database.
    // If nothing is found 0 is returned.
    // NOTE: The list must be destroyed by the caller.
    
    Char_t query[256];

    // get all entries
    sprintf(query, "SELECT DISTINCT %s from %s", data, table);
    TSQLResult* res = SendQuery(query);

    // count rows
    if (!res->GetRowCount())
    {
        delete res;
        return 0;
    }

    // get number of entries
    Int_t n = res->GetRowCount();

    // create list
    TList* list = new TList();
    list->SetOwner(kTRUE);

    // read all entries and add them to the list
    for (Int_t i = 0; i < n; i++)
    {
        // get next run
        TSQLRow* row = res->Next();

        // save target
        list->Add(new TObjString(row->GetField(0)));

        // clean-up
        delete row;
    }

    // clean-up
    delete res;

    // return the list
    return list;
}

//______________________________________________________________________________
TList* TCMySQLManager::GetAllTargets()
{
    // Return a list of TStrings containing all targets in the database.
    // If no targets were found 0 is returned.
    // NOTE: The list must be destroyed by the caller.
    
    return SearchDistinctEntries("target", TCConfig::kCalibMainTableName);
}

//______________________________________________________________________________
TList* TCMySQLManager::GetAllCalibrations(const Char_t* data)
{
    // Return a list of TStrings containing all calibration identifiers in the database
    // for the calibration data 'data'.
    // If no calibrations were found 0 is returned.
    // NOTE: The list must be destroyed by the caller.

    return SearchDistinctEntries("calibration", ((TCCalibData*) fData->FindObject(data))->GetTableName());
}

//______________________________________________________________________________
Bool_t TCMySQLManager::AddDataSet(const Char_t* data, const Char_t* calibration, const Char_t* desc,
                                  Int_t first_run, Int_t last_run, Double_t* par, Int_t length)
{
    // Create a new set of the calibration data 'data' with the calibration identifier
    // 'calibration' for the runs 'first_run' to 'last_run'. Use 'desc' as a 
    // description. Read the 'length' parameters from 'par'.
    // Return kFALSE when an error occured, otherwise kTRUE.
 
    Char_t table[256];
    
    //
    // check if first and last run exist
    //
    
    // check first run
    if (!SearchRunEntry(first_run, "run", table))
    {
        if (!fSilence) Error("AddDataSet", "First run has no valid run number!");
        return kFALSE;
    }
    
    // check last run
    if (!SearchRunEntry(last_run, "run", table))
    {
        if (!fSilence) Error("AddDataSet", "Last run has no valid run number!");
        return kFALSE;
    }

    //
    // check if the run range is ok
    //

    // check if first run is smaller than last run
    if (first_run > last_run)
    {
        if (!fSilence) Error("AddDataSet", "First run of set has to be smaller than last run!");
        return kFALSE;
    }

    // get number of existing sets
    Int_t nSet = GetNsets(data, calibration);

    // loop over sets
    for (Int_t i = 0; i < nSet; i++)
    {
        // get first and last runs of set
        Int_t setLow = GetFirstRunOfSet(data, calibration, i);
        Int_t setHigh = GetLastRunOfSet(data, calibration, i);

        // check if first run is member of this set
        if (first_run >= setLow && first_run <= setHigh)
        {
            if (!fSilence) Error("AddDataSet", "First run is already member of set %d", i);
            return kFALSE;
        }

        // check if last run is member of this set
        if (last_run >= setLow && last_run <= setHigh)
        {
            if (!fSilence) Error("AddDataSet", "Last run is already member of set %d", i);
            return kFALSE;
        }

        // check if sets are not overlapping
        if ((setLow >= first_run && setLow <= last_run) ||
            (setHigh >= first_run && setHigh <= last_run))
        {
            if (!fSilence) Error("AddDataSet", "Run overlap with set %d", i);
            return kFALSE;
        }
    }

    //
    // create the set
    //
    
    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("AddDataSet", "No data table found!");
        return kFALSE;
    }

    // prepare the insert query
    TString ins_query = TString::Format("INSERT INTO %s SET calibration = '%s', description = '%s', first_run = %d, last_run = %d,",
                                        table, calibration, desc, first_run, last_run);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        ins_query.Append(TString::Format("par_%03d = %.17g", j, par[j]));
        if (j != length - 1) ins_query.Append(",");
    }

    // write data to database
    TSQLResult* res = SendQuery(ins_query.Data());
    
    // check result
    if (!res)
    {
        if (!fSilence) Error("AddDataSet", "Could not add the set of '%s' for runs %d to %d!", 
                        ((TCCalibData*) fData->FindObject(data))->GetTitle(), first_run, last_run);
        return kFALSE;
    }
    else
    {
        delete res;
        if (!fSilence) Info("AddDataSet", "Added set of '%s' for runs %d to %d", 
                                      ((TCCalibData*) fData->FindObject(data))->GetTitle(), first_run, last_run);
        return kTRUE;
    }
}

//______________________________________________________________________________
Bool_t TCMySQLManager::AddDataSet(const Char_t* data, const Char_t* calibration, const Char_t* desc,
                                  Int_t first_run, Int_t last_run, Double_t par)
{
    // Create a new set of the calibration data 'data' with the calibration identifier
    // 'calibration' for the runs 'first_run' to 'last_run'. Use 'desc' as a 
    // description. Set all parameters to the value 'par'.
    // Return kFALSE when an error occured, otherwise kTRUE.
    
    // get maximum number of parameters
    Int_t length = ((TCCalibData*) fData->FindObject(data))->GetSize();
    
    // create and fill parameter array
    Double_t par_array[length];
    for (Int_t i = 0; i < length; i++) par_array[i] = par;

    // set parameters
    return AddDataSet(data, calibration, desc, first_run, last_run, par_array, length);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::AddSet(const Char_t* type, const Char_t* calibration, const Char_t* desc,
                              Int_t first_run, Int_t last_run, Double_t par)
{
    // Create new sets for the calibration type 'type' with the calibration identifier
    // 'calibration' for the runs 'first_run' to 'last_run'. Use 'desc' as a 
    // description. Set all parameters to the value 'par'.
    // Return kFALSE when an error occured, otherwise kTRUE.
    
    Bool_t ret = kTRUE;
    
    // create and fill parameter array
    Double_t par_array[TCConfig::kMaxCrystal];
    for (Int_t i = 0; i < TCConfig::kMaxCrystal; i++) par_array[i] = par;

    // get calibration type and data list
    TCCalibType* t = (TCCalibType*) fTypes->FindObject(type);
    TList* data = t->GetData();
    
    // loop over calibration data of this calibration type
    TIter next(data);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // add set
        if (AddDataSet(d->GetName(), calibration, desc, first_run, last_run, par_array, 
                       d->GetSize())) ret = kFALSE;
    }

    return ret;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::RemoveDataSet(const Char_t* data, const Char_t* calibration, Int_t set)
{
    // Remove the set 'set' from the calibration 'calibration' of the calibration data
    // 'data'.
    
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        if (!fSilence) Error("RemoveDataSet", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    
    // check first run
    if (first_run < 0)
    {
        if (!fSilence) Error("RemoveDataSet", "Could not delete set %d in '%s' of calibration '%s'!",
                             set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "DELETE FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d",
            table, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        if (!fSilence) Error("RemoveDataSet", "Could not delete set %d in '%s' of calibration '%s'!",
                           set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }
    else
    {
        if (!fSilence) Info("RemoveDataSet", "Deleted set %d in '%s' of calibration '%s'",
                                         set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kTRUE;
    }
}

//______________________________________________________________________________
Bool_t TCMySQLManager::RemoveSet(const Char_t* type, const Char_t* calibration, Int_t set)
{
    // Remove all sets 'set' from the calibration 'calibration' that are needed by the
    // calibration type 'type'.
    
    Bool_t ret = kTRUE;
    
    // get calibration type and data list
    TCCalibType* t = (TCCalibType*) fTypes->FindObject(type);
    TList* data = t->GetData();
 
    // loop over calibration data of this calibration type
    TIter next(data);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // remove set of calibration data
        if (!RemoveDataSet(d->GetName(), calibration, set))
            ret = kFALSE;
    }

    return ret;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SplitDataSet(const Char_t* data, const Char_t* calibration, Int_t set,
                                    Int_t lastRunFirstSet)
{
    // Split the set 'set' of the calibration data 'data' and the calibration identifier
    // 'calibration' into two sets. 'lastRunFirstSet' will be the last run of the first 
    // set. The first run of the second set will be the next found run in the database.
    
    Char_t query[256];
    Char_t tmp[256];
 
    // check if splitting run exists
    if (!SearchRunEntry(lastRunFirstSet, "run", tmp))
    {
        if (!fSilence) Error("SplitDataSet", "Splitting run has no valid run number!");
        return kFALSE;
    }
 
    // check if splitting run is in set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    Int_t last_run = GetLastRunOfSet(data, calibration, set);
    if (lastRunFirstSet < first_run || lastRunFirstSet > last_run)
    {
        if (!fSilence) Error("SplitDataSet", "Splitting run has to be in set!");
        return kFALSE;
    }

    //
    // get the first and last run of the second set
    //
    
    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE run > %d "
            "ORDER by run LIMIT 1",
            TCConfig::kCalibMainTableName, lastRunFirstSet);

    // read from database
    TSQLResult* res = SendQuery(query);
    if (!res)
    {
        if (!fSilence) Error("SplitDataSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        if (!fSilence) Error("SplitDataSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    
    // get row
    TSQLRow* row = res->Next();
  
    // get the first run of the second set
    Int_t firstRunSecondSet = 0;
    const Char_t* field = row->GetField(0);
    if (!field) 
    {
        if (!fSilence) Error("SplitDataSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    else firstRunSecondSet = atoi(field);
    
    // clean-up
    delete row;
    delete res;
    
    // get the last run of the second set
    Int_t lastRunSecondSet = last_run;

    // 
    // backup values
    //

    // backup the description
    Char_t desc[256];
    if (!SearchSetEntry(data, calibration, set, "description", desc))
    {
        if (!fSilence) Error("SplitDataSet", "Cannot read description of set %d in '%s' of calibration '%s'",
                          set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // get number of parameters
    Int_t nPar = ((TCCalibData*) fData->FindObject(data))->GetSize();

    // backup parameters
    Double_t par[nPar];
    if (!ReadParameters(data, calibration, set, par, nPar))
    {
        if (!fSilence) Error("SplitDataSet", "Cannot backup parameters of set %d in '%s' of calibration '%s'",
                          set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
    }

    // 
    // modify first set
    //

    // change the last run of the first set
    sprintf(tmp, "%d", lastRunFirstSet);
    if (!ChangeSetEntry(data, calibration, set, "last_run", tmp))
    {
        if (!fSilence) Error("SplitDataSet", "Cannot change last run of set %d in '%s' of calibration '%s'",
                          set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }
    else
    {
        if (!fSilence) Info("SplitDataSet", "Changed last run of set %d in '%s' of calibration '%s' to %d",
                                        set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration, lastRunFirstSet);
    }
    
    //
    // add second set
    //
    
    if (!AddDataSet(data, calibration, desc, firstRunSecondSet, lastRunSecondSet, par, nPar))
    {
        if (!fSilence) Error("SplitDataSet", "Cannot split set %d in '%s' of calibration '%s'",
                             set, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SplitSet(const Char_t* type, const Char_t* calibration, Int_t set,
                                Int_t lastRunFirstSet)
{
    // Split all sets 'set' of the calibration type 'type' and the calibration identifier
    // 'calibration' into two sets. 'lastRunFirstSet' will be the last run of the first 
    // set. The first run of the second set will be the next found run in the database.
    
    Bool_t ret = kTRUE;
    
    // get calibration type and data list
    TCCalibType* t = (TCCalibType*) fTypes->FindObject(type);
    TList* data = t->GetData();
    
    // loop over calibration data of this calibration type
    TIter next(data);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // split set of calibration data
        if (!SplitDataSet(d->GetName(), calibration, set, lastRunFirstSet))
            ret = kFALSE;
    }

    return ret;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::MergeDataSets(const Char_t* data, const Char_t* calibration, 
                                     Int_t set1, Int_t set2)
{
    // Merge the two adjacent sets 'set1' and 'set2' of the calibration data 'data' and the 
    // calibration identifier 'calibration' into one set. Description and parameters
    // of 'set1' will be used in the merged set.
    // get the first run of the set
    
    Char_t tmp[256];

    // check if the two sets are adjacent 
    if (TMath::Abs(set2 - set1) != 1)
    {
        if (!fSilence) Error("MergeDataSets", "Only adjacent sets can be merged!");
        return kFALSE;
    }

    // get information of set 1
    Int_t firstSet1 = GetFirstRunOfSet(data, calibration, set1);
    Int_t lastSet1 = GetLastRunOfSet(data, calibration, set1);
    
    // get information of set 2
    Int_t firstSet2 = GetFirstRunOfSet(data, calibration, set2);
    Int_t lastSet2 = GetLastRunOfSet(data, calibration, set2);

    // check set 1
    if (firstSet1 < 0 || lastSet1 < 0)
    {
        if (!fSilence) Error("MergeDataSets", "Could not find set %d in '%s' of calibration '%s'!",
                             set1, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // check set 2
    if (firstSet2 < 0 || lastSet2 < 0)
    {
        if (!fSilence) Error("MergeDataSets", "Could not find set %d in '%s' of calibration '%s'!",
                             set2, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return kFALSE;
    }

    // delete set 2
    if (!RemoveDataSet(data, calibration, set2))
    {
        if (!fSilence) Error("MergeDataSets", "Could not remove set %d!", set2);
        return kFALSE;
    }
    
    // adjust run interval
    // check if set 2 was in front of set 1
    if (firstSet2 < firstSet1)
    {
        // adjust first run of remaining set
        sprintf(tmp, "%d", firstSet2);
        if (!ChangeSetEntry(data, calibration, set1-1, "first_run", tmp))
        {
            if (!fSilence) Error("MergeDataSets", "Could not adjust run interval of set %d!", set1-1);
            return kFALSE;
        }
    }
    else
    {
        // adjust last run of remaining set
        sprintf(tmp, "%d", lastSet2);
        if (!ChangeSetEntry(data, calibration, set1, "last_run", tmp))
        {
            if (!fSilence) Error("MergeDataSets", "Could not adjust run interval of set %d!", set1);
            return kFALSE;
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::MergeSets(const Char_t* type, const Char_t* calibration, 
                                 Int_t set1, Int_t set2)
{
    // Merge all adjacent pairs of the sets 'set1' and 'set2' of the calibration type 'type' and the 
    // calibration identifier 'calibration' into one set. Description and parameters
    // of 'set1' will be used in the merged set.
    // get the first run of the set
    
    Bool_t ret = kTRUE;
    
    // get calibration type and data list
    TCCalibType* t = (TCCalibType*) fTypes->FindObject(type);
    TList* data = t->GetData();
    
    // loop over calibration data of this calibration type
    TIter next(data);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // merge sets of calibration data
        if (!MergeDataSets(d->GetName(), calibration, set1, set2))
            ret = kFALSE;
    }

    return ret;
}

//______________________________________________________________________________
Int_t TCMySQLManager::DumpRuns(TCContainer* container, Int_t first_run, Int_t last_run)
{
    // Dump the run information from run 'first_run' to run 'last_run' to 
    // the CaLib container 'container'.
    // If first_run and last_run is zero all available runs will be dumped.
    // Return the number of dumped runs.

    Char_t query[256];
    Char_t tmp[256];

    // create the query
    if (first_run == -1 && last_run == -1)
    {
        sprintf(query,
                "SELECT run FROM %s "
                "ORDER by run",
                TCConfig::kCalibMainTableName);
    }
    else
    {
        sprintf(query,
                "SELECT run FROM %s "
                "WHERE run >= %d "
                "AND run <= %d "
                "ORDER by run",
                TCConfig::kCalibMainTableName, first_run, last_run);
    }

    // read from database
    TSQLResult* res = SendQuery(query);

    // get number of runs
    Int_t nruns = res->GetRowCount();

    // loop over runs
    for (Int_t i = 0; i < nruns; i++)
    {
        // get next run
        TSQLRow* row = res->Next();
        
        // get run number
        Int_t run_number = atoi(row->GetField(0));

        // add new run
        TCRun* run = container->AddRun(run_number);
        
        // set path
        if (SearchRunEntry(run_number, "path", tmp)) run->SetPath(tmp);
        
        // set filename
        if (SearchRunEntry(run_number, "filename", tmp)) run->SetFileName(tmp);
        
        // set time
        if (SearchRunEntry(run_number, "time", tmp)) run->SetTime(tmp);
        
        // set description
        if (SearchRunEntry(run_number, "description", tmp)) run->SetDescription(tmp);
        
        // set run_note
        if (SearchRunEntry(run_number, "run_note", tmp)) run->SetRunNote(tmp);
        
        // set size
        if (SearchRunEntry(run_number, "size", tmp)) 
        {
            Long64_t size;
            sscanf(tmp, "%lld", &size);
            run->SetSize(size);
        }

        // set target
        if (SearchRunEntry(run_number, "target", tmp)) run->SetTarget(tmp);
        
        // set target polarization
        if (SearchRunEntry(run_number, "target_pol", tmp)) run->SetTargetPol(tmp);
        
        // set target polarization degree
        if (SearchRunEntry(run_number, "target_pol_deg", tmp)) run->SetTargetPolDeg(atof(tmp));
        
        // set beam polarization
        if (SearchRunEntry(run_number, "beam_pol", tmp)) run->SetBeamPol(tmp);

        // set beam polarization degree
        if (SearchRunEntry(run_number, "beam_pol_deg", tmp)) run->SetBeamPolDeg(atof(tmp));
        
        // user information
        if (!fSilence) Info("DumpRuns", "Dumped run %d", run_number);

        // clean-up
        delete row;
    }
    
    // clean-up
    delete res;

    return nruns;
}

//______________________________________________________________________________
Int_t TCMySQLManager::DumpCalibrations(TCContainer* container, const Char_t* calibration, 
                                       const Char_t* data)
{
    // Dump calibrations of the calibration data 'data' with the calibration 
    // identifier 'calibration' to the CaLib container 'container'.
    // Return the number of dumped calibrations.

    Char_t tmp[256];
    
    // get number of parameters
    Int_t nPar = ((TCCalibData*) fData->FindObject(data))->GetSize();

    // create the parameter array
    Double_t par[nPar];

    // get the number of sets
    Int_t nSet = GetNsets(data, calibration);
    
    // check calibration
    if (!nSet)
    {
        if (!fSilence) Error("DumpCalibrations", "No sets of '%s' of the calibration '%s' found!",    
                             ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);
        return 0;
    }

    // loop over sets
    for (Int_t i = 0; i < nSet; i++)
    {
        // read parameters
        ReadParameters(data, calibration, i, par, nPar);
        
        // add the calibration
        TCCalibration* c = container->AddCalibration(calibration);
        
        // set calibration data
        c->SetCalibData(data);

        // set description
        GetDescriptionOfSet(data, calibration, i, tmp);
        c->SetDescription(tmp);

        // set first and last run
        c->SetFirstRun(GetFirstRunOfSet(data, calibration, i));
        c->SetLastRun(GetLastRunOfSet(data, calibration, i));
        
        // set fill time
        GetChangeTimeOfSet(data, calibration, i, tmp);
        c->SetChangeTime(tmp);

        // set parameters
        c->SetParameters(nPar, par);
    }

    // user information
    if (!fSilence) Info("DumpCalibrations", "Dumped %d sets of '%s' of the calibration '%s'",
                        nSet, ((TCCalibData*) fData->FindObject(data))->GetTitle(), calibration);

    return nSet;
}

//______________________________________________________________________________
Int_t TCMySQLManager::DumpAllCalibrations(TCContainer* container, const Char_t* calibration)
{
    // Dump all calibrations with the calibration identifier 'calibration' to
    // the CaLib container 'container'.
    // Return the number of dumped calibrations.
    
    Int_t nDump = 0;

    // loop over calibration data
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // dump calibrations
        if (Int_t nd = DumpCalibrations(container, calibration, d->GetName())) nDump += nd;
    }

    return nDump;
}

//______________________________________________________________________________
Int_t TCMySQLManager::ImportRuns(TCContainer* container)
{
    // Import all runs from the CaLib container 'container' to the database.
    // Return the number of imported runs.

    // get number of runs
    Int_t nRun = container->GetNRuns();

    // loop over runs
    Int_t nRunAdded = 0;
    for (Int_t i = 0; i < nRun; i++)
    {
        // get the run
        TCRun* r = container->GetRun(i);
        
        // prepare the insert query
        TString ins_query = TString::Format("INSERT INTO %s SET "
                                            "run = %d, "
                                            "path = '%s', "
                                            "filename = '%s', "
                                            "time = '%s', "
                                            "description = '%s', "
                                            "run_note = '%s', "
                                            "size = %lld, "
                                            "target = '%s', "
                                            "target_pol = '%s', "
                                            "target_pol_deg = %lf, "
                                            "beam_pol = '%s', "
                                            "beam_pol_deg = %lf",
                                            TCConfig::kCalibMainTableName, 
                                            r->GetRun(),
                                            r->GetPath(),
                                            r->GetFileName(),
                                            r->GetTime(),
                                            r->GetDescription(),
                                            r->GetRunNote(),
                                            r->GetSize(),
                                            r->GetTarget(),
                                            r->GetTargetPol(),
                                            r->GetTargetPolDeg(),
                                            r->GetBeamPol(),
                                            r->GetBeamPolDeg());

        // try to write data to database
        TSQLResult* res = SendQuery(ins_query.Data());
        if (res == 0)
        {
            Warning("ImportRuns", "Run %d could not be added to the database!", 
                    r->GetRun());
        }
        else
        {
            if (!fSilence) Info("ImportRuns", "Added run %d to the database", r->GetRun());
            nRunAdded++;
            delete res;
        }
    }

    // user information
    if (!fSilence) Info("ImportRuns", "Added %d runs to the database", nRunAdded);
    
    return nRunAdded;
}

//______________________________________________________________________________
Int_t TCMySQLManager::ImportCalibrations(TCContainer* container, const Char_t* newCalibName, 
                                         const Char_t* data)
{
    // Import all calibrations from the CaLib container 'container' to the database.
    // If 'newCalibName' is non-zero rename the calibration to 'newCalibName'
    // If 'data' is not 0 import only calibrations of the data 'data'.
    // Return the number of imported calibrations.

    // get number of calibrations
    Int_t nCalib = container->GetNCalibrations();

    // loop over calibrations
    Int_t nCalibAdded = 0;
    for (Int_t i = 0; i < nCalib; i++)
    {
        // get the calibration
        TCCalibration* c = container->GetCalibration(i);
        
        // skip unwanted calibration data
        if (data != 0 && strcmp(c->GetCalibData(), data)) continue;

        // add the set with new calibration identifer or the same
        const Char_t* calibration;
        if (newCalibName) calibration = newCalibName;
        else calibration = c->GetCalibration();

        // add the set
        if (AddDataSet(c->GetCalibData(), calibration, c->GetDescription(), 
                       c->GetFirstRun(), c->GetLastRun(), c->GetParameters(), c->GetNParameters()))
        {
            if (!fSilence) Info("ImportCalibrations", "Added calibration '%s' of '%s' to the database",
                                calibration, ((TCCalibData*) fData->FindObject(c->GetCalibData()))->GetTitle());
            nCalibAdded++;
        }
        else
        {
            if (!fSilence) Error("ImportCalibrations", "Calibration '%s' of '%s' could not be added to the database!",
                                 calibration, ((TCCalibData*) fData->FindObject(c->GetCalibData()))->GetTitle());
        }
    }

    // user information
    if (!fSilence) Info("ImportCalibrations", "Added %d calibrations to the database", nCalibAdded);

    return nCalibAdded;
}

//______________________________________________________________________________
void TCMySQLManager::CloneCalibration(const Char_t* calibration, const Char_t* newCalibrationName,
                                      const Char_t* newDesc, Int_t new_first_run, Int_t new_last_run)
{
    // Create a clone of the calibration 'calibration' using 'newCalibrationName' as
    // the new calibration name and 'newDesc' as the calibration description.
    // For each calibration data one set from 'new_first_run' to 'new_last_run' 
    // is created with the values of the last set of the original calibration.
    
    // check if original calibration exists
    if (!ContainsCalibration(calibration))
    {
        if (!fSilence) Error("CloneCalibration", "Original calibration '%s' does not exist!", calibration);
        return;
    }

    // loop over calibration data
    TIter next(fData);
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        // get number of original sets
        Int_t nSets = GetNsets(d->GetName(), calibration);
        
        // read parameters
        Double_t par[d->GetSize()];
        if (ReadParameters(d->GetName(), calibration, nSets-1, par, d->GetSize()))
        {
            if (!AddDataSet(d->GetName(), newCalibrationName, newDesc, 
                            new_first_run, new_last_run, par, d->GetSize()))
                if (!fSilence) Error("CloneCalibration", "Could not clone calibration data '%s'!", d->GetName());
        }
        else
        {
            if (!fSilence) Error("CloneCalibration", "Could not read original data '%s'!", d->GetName());
        }
    }
}

//______________________________________________________________________________
void TCMySQLManager::Export(const Char_t* filename, Int_t first_run, Int_t last_run, 
                            const Char_t* calibration)
{
    // Export run and/or calibration data to the ROOT file 'filename'
    //
    // If 'first_run' is zero or greater and 'last_run' is zero or greater run
    // information from run 'first_run' to run 'last_run' is exported.
    // If 'first_run' is -1 and 'last_run' is -1 all run information is exported.
    // If 'first_run' is smaller than -1 or 'last_run' is smaller than -1 no run
    // information is exported.
    //
    // If 'calibration' is non-zero the calibration with the identifier 'calibration'
    // is exported.

    // create new container
    TCContainer* container = new TCContainer(TCConfig::kCaLibDumpName);
    
    // dump runs to container
    if (first_run >= -1 && last_run >= -1)
    {
        DumpRuns(container, first_run, last_run);
        if (!fSilence) 
        {
            if (container->GetNRuns())
                Info("Export", "Dumped %d runs to '%s'", container->GetNRuns(), filename);
            else
                Error("Export", "No runs were dumped to '%s'!", filename);
        }
    }

    // dump runs to container
    if (calibration)
    {
        DumpAllCalibrations(container, calibration);
        if (!fSilence) 
        {
            if (container->GetNCalibrations())
                Info("Export", "Dumped %d calibrations to '%s'", container->GetNCalibrations(), filename);
            else
                Error("Export", "No calibrations were dumped to '%s'!", filename);
        }
    }

    // save container to ROOT file
    container->SaveAs(filename, fSilence);
    
    // clean-up
    delete container;
}

//______________________________________________________________________________
TCContainer* TCMySQLManager::LoadContainer(const Char_t* filename)
{
    // Load the CaLib container from the ROOT file 'filename'.
    // Return the container if found, otherwise 0.
    // NOTE: The container must be destroyed by the caller.

    // try to open the ROOT file
    TFile* f = new TFile(filename);
    if (!f)
    {
        if (!fSilence) Error("LoadContainer", "Could not open the ROOT file '%s'!", filename);
        return 0;
    }
    if (f->IsZombie())
    {
        if (!fSilence) Error("LoadContainer", "Could not open the ROOT file '%s'!", filename);
        return 0;
    }

    // to load the CaLib container
    TCContainer* c_orig = (TCContainer*) f->Get(TCConfig::kCaLibDumpName);
    if (!c_orig)
    {
        if (!fSilence) Error("LoadContainer", "No CaLib container found in ROOT file '%s'!", filename);
        delete f;
        return 0;
    }
    
    // clone the container
    TCContainer* c = (TCContainer*) c_orig->Clone();

    // check container format
    if (c->GetVersion() != TCConfig::kContainerFormatVersion)
    {
        if (!fSilence) Error("LoadContainer", "Cannot load CaLib container format version %d", c->GetVersion());
        return 0;
    }
    
    // clean-up
    delete f;

    return c;
}

//______________________________________________________________________________
void TCMySQLManager::Import(const Char_t* filename, Bool_t runs, Bool_t calibrations,
                            const Char_t* newCalibName)
{
    // Import run and/or calibration data from the ROOT file 'filename'
    //
    // If 'runs' is kTRUE all run information is imported.
    // If 'calibrations' is kTRUE all calibration information is imported.
    // If 'newCalibName' is non-zero rename the calibration to 'newCalibName'
    
    // try to load the container
    TCContainer* c = LoadContainer(filename);
    if (!c)
    {
        if (!fSilence) Error("Import", "CaLib container could not be loaded from '%s'", filename);
        return;
    }

    // import runs
    if (runs) 
    {
        // get number of runs
        Int_t nRun = c->GetNRuns();

        // check if some runs were found
        if (nRun)
        {
            // ask for user confirmation
            Char_t answer[256];
            printf("\n%d runs were found in the ROOT file '%s'\n"
                   "They will be added to the database '%s' on '%s'\n", 
                   nRun, filename, fDB->GetDB(), fDB->GetHost());
            printf("Are you sure to continue? (yes/no) : ");
            if(scanf("%s", answer));
            if (strcmp(answer, "yes")) 
            {
                printf("Aborted.\n");
            }
            else
            {
                // import all runs
                ImportRuns(c);
            }
        }
        else
        {
            if (!fSilence) Error("Import", "No runs were found in ROOT file '%s'!", filename);
        }
    }

    // import calibrations
    if (calibrations) 
    {
        // get number of calibrations
        Int_t nCalib = c->GetNCalibrations();

        // check if some calibrations were found
        if (nCalib)
        {
            // get name of calibrations
            const Char_t* calibName = c->GetCalibration(0)->GetCalibration();

            // ask for user confirmation
            Char_t answer[256];
            printf("\n%d calibrations named '%s' were found in the ROOT file '%s'\n"
                   "They will be added to the database '%s' on '%s'\n", 
                   nCalib, calibName, filename, fDB->GetDB(), fDB->GetHost());
            if (newCalibName) printf("The calibrations will be renamed to '%s'\n", newCalibName);
            printf("Are you sure to continue? (yes/no) : ");
            if(scanf("%s", answer));
            if (strcmp(answer, "yes")) 
            {
                printf("Aborted.\n");
            }
            else
            {
                // import all runs
                ImportCalibrations(c, newCalibName);
            }
        }
        else
        {
            if (!fSilence) Error("Import", "No calibrations were found in ROOT file '%s'!", filename);
        }
    }

    // clean-up
    delete c;
}
ClassImp(TCMySQLManager)
