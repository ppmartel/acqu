// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ConvertCaLib.C                                                       //
//                                                                      //
// Convert CaLib tables to the new format.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TString.h"
#include "TError.h"


// MySQL configuration (old database)
const Char_t* gDBHostOld = "phys-jaguar";
const Char_t* gDBNameOld = "acqu";
const Char_t* gDBUserOld = "acqu_rw";
const Char_t* gDBPassOld = "basel_write";

// MySQL configuration (new database)
const Char_t* gDBHostNew = "phys-jaguar";
const Char_t* gDBNameNew = "calib";
const Char_t* gDBUserNew = "acqu_rw";
const Char_t* gDBPassNew = "basel_write";

// global variables
TSQLServer* gDBOld;
TSQLServer* gDBNew;


//______________________________________________________________________________
Bool_t IsConnected(TSQLServer* db)
{
    // Check if the connection to the database 'db' is there.

    return db ? kTRUE : kFALSE;
}

//______________________________________________________________________________
TSQLResult* SendQuery(TSQLServer* db, const Char_t* query, Bool_t delRes = kTRUE)
{
    // Send a query to the database 'db' and return the result.
    // Delete the result (and return 0) if 'delRes' is kTRUE.
    
    // check server connection
    if (!IsConnected(db))
    {
        Error("SendQuery", "No connection to database!");
        return 0;
    }

    // execute query
    TSQLResult* res = db->Query(query);
    
    // return the query
    if (delRes)
    {
        delete res;
        return 0;
    }
    else return res;
}

//______________________________________________________________________________
void ConvertMainTable(const Char_t* targPre)
{
    // Convert the main table of a beamtime with the target prefix 'targPre'.

    // get runs
    TSQLResult* res = SendQuery(gDBOld, TString::Format("SELECT * FROM %s_run_main", targPre).Data(), kFALSE);

    // get number of runs
    Int_t nRuns = res->GetRowCount();

    // user information
    printf("Converting %d runs of the beamtime %s\n", nRuns, targPre);

    // loop over runs
    for (Int_t i = 0; i < nRuns; i++)
    {
        // get row
        TSQLRow* row = res->Next();
        
        // run number
        Int_t run = atoi(row->GetField(0));
        
        // path and filename
        Char_t filename[256];
        sprintf(filename, "%s%s", row->GetField(1), row->GetField(2));

        // status
        Char_t status[256];
        strcpy(status, row->GetField(3));

        // target
        Char_t target[256];
        strcpy(target, row->GetField(4));

        // size
        Int_t size = atoi(row->GetField(5));

        // event
        Int_t event = atoi(row->GetField(6));

        // date
        Char_t date[256];
        strcpy(date, row->GetField(7));

        // comment
        Char_t comment[256];
        strcpy(comment, row->GetField(9));

        // fill run into new database
        SendQuery(gDBNew, TString::Format("INSERT INTO run_main "
                                          "SET run = %d, "
                                          "filename = '%s',"
                                          "target = '%s',"
                                          "size = %d,"
                                          "time = '%s',"
                                          "run_note = '%s'",
                                          run, filename, target,
                                          size, date, comment).Data());

        // clean-up
        delete row;
    }

    // clean-up
    delete res;
}

//______________________________________________________________________________
void ConvertDataTable(const Char_t* parTable, const Char_t* setTable,
                      const Char_t* parTableNew,
                      Int_t nPar)
{
    // Convert 'nPar' parameters of the data table 'parTable' using the set 
    // definitions from the table 'setTable' into the new data table 'parTableNew'.

    // user information
    printf("Converting %d parameters of table %s\n", nPar, parTable);

    // read sets
    TSQLResult* set_res = SendQuery(gDBOld, TString::Format("SELECT DISTINCT * FROM %s ORDER BY start_run", setTable).Data(), kFALSE);
    
    // get number of sets
    Int_t nSets = set_res->GetRowCount();

    // user information
    printf("-> %d sets found\n", nSets);
    
    // loop over runsets
    for (Int_t i = 0; i < nSets; i++)
    {
        // get row
        TSQLRow* set_row = set_res->Next();
 
        // get runset number
        Int_t runset = atoi(set_row->GetField(0));

        // get run limits
        Int_t start_run = atoi(set_row->GetField(1));
        Int_t stop_run = atoi(set_row->GetField(2));

        // read data for this runset
        TSQLResult* data_res = SendQuery(gDBOld, 
                                         TString::Format("SELECT filled FROM %s WHERE runset = %d "
                                         "ORDER BY filled DESC LIMIT 1", parTable, runset).Data(),
                                         kFALSE);
        
        // get filled date
        TSQLRow* data_row = data_res->Next();
        Char_t filled[256];
        strcpy(filled, data_row->GetField(0));
        delete data_row;
        delete data_res;

        // user information
        printf("   -> Converting set %d (latest iteration: %s)\n", runset, filled);
        
        // prepare query for reading all parameters
        TString query("SELECT ");
        for (Int_t j = 0; j < nPar; j++) 
        {
            query.Append(TString::Format("par_%03d", j+1));
            if (j != nPar - 1) query.Append(",");
        }
        query.Append(TString::Format(" FROM %s WHERE runset = %d AND filled = '%s'", parTable, runset, filled));
        
        // read all parameters and write them to new query
        data_res = SendQuery(gDBOld, query.Data(), kFALSE);
        data_row = data_res->Next();
        query = TString::Format("INSERT INTO %s SET calibration = 'LD2_Domi', first_run = %d, last_run = %d, changed = '%s',", 
                                parTableNew, start_run, stop_run, filled);
        for (Int_t j = 0; j < nPar; j++) 
        {   
            // try to convert the parameter value
            Double_t val = 0;
            const Char_t* val_s = data_row->GetField(j);
            if (val_s != 0) val = atof(val_s);

            // append parameter to query
            query.Append(TString::Format("par_%03d = %f", j, val));
            if (j != nPar - 1) query.Append(",");
        }
    
        // send insert query to the new database
        SendQuery(gDBNew, query.Data());

        // clean-up
        delete data_row;
        delete data_res;

        // clean-up
        delete set_row;
    }

    // clean-up
    delete set_res;
}

//______________________________________________________________________________
void ConvertCaLib()
{
    // Main function.
    
    Char_t tmp[256];

    // connect to the old SQL server
    sprintf(tmp, "mysql://%s/%s", gDBHostOld, gDBNameOld);
    gDBOld = TSQLServer::Connect(tmp, gDBUserOld, gDBPassOld);

    // check server connection
    if (!IsConnected(gDBOld))
    {
        Error("ConvertCaLib", "Could not connect to the old database!");
        return;
    }

    // connect to the new SQL server
    sprintf(tmp, "mysql://%s/%s", gDBHostNew, gDBNameNew);
    gDBNew = TSQLServer::Connect(tmp, gDBUserNew, gDBPassNew);

    // check server connection
    if (!IsConnected(gDBNew))
    {
        Error("ConvertCaLib", "Could not connect to the new database!");
        return;
    }

    // convert the main table 
    ConvertMainTable("LD2");

    //
    // convert TAGG tables
    //

    // time offset
    ConvertDataTable("LD2_tagger_Toffset", "LD2_tagger_Toffset_sets", "tagg_t0", 352);
    

    //
    // convert CB tables
    //

    // energy gain
    ConvertDataTable("LD2_cb_Ecalib", "LD2_sets_main", "cb_e1", 720);
    
    // time offset
    ConvertDataTable("LD2_cb_Toffset", "LD2_cb_Toffset_sets", "cb_t0", 720);

    // time walk parameter 1
    ConvertDataTable("LD2_cb_TWalk0", "LD2_cb_TWalk_sets", "cb_walk0", 720);

    // time walk parameter 2
    ConvertDataTable("LD2_cb_TWalk1", "LD2_cb_TWalk_sets", "cb_walk1", 720);

    // time walk parameter 3
    ConvertDataTable("LD2_cb_TWalk2", "LD2_cb_TWalk_sets", "cb_walk2", 720);

    // time walk parameter 4
    ConvertDataTable("LD2_cb_TWalk3", "LD2_cb_TWalk_sets", "cb_walk3", 720);

    //
    // convert TAPS tables
    //

    // LG energy pedestal
    ConvertDataTable("LD2_taps_Eoffset_lg", "LD2_taps_Eoffset_sets", "taps_lg_e0", 438);

    // SG energy pedestal
    ConvertDataTable("LD2_taps_Eoffset_sg", "LD2_taps_Eoffset_sets", "taps_sg_e0", 438);
    
    // LG energy gain
    ConvertDataTable("LD2_taps_Ecalib_lg", "LD2_taps_Ecalib_sets", "taps_lg_e1", 438);

    // SG energy gain
    ConvertDataTable("LD2_taps_Ecalib_sg", "LD2_taps_Ecalib_sets", "taps_sg_e1", 438);
    
    // time offset
    ConvertDataTable("LD2_taps_Toffset", "LD2_taps_Toffset_sets", "taps_t0", 438);
    
    // time gain
    ConvertDataTable("LD2_taps_Tcalib", "LD2_taps_Tcalib_sets", "taps_t1", 438);

    //
    // convert PID tables
    //

    // energy pedestal
    ConvertDataTable("LD2_pid_Eoffset", "LD2_pid_Eoffset_sets", "pid_e0", 24);
    
    // energy gain
    ConvertDataTable("LD2_pid_Ecalib", "LD2_pid_Ecalib_sets", "pid_e1", 24);

    // time gain
    ConvertDataTable("LD2_pid_Toffset", "LD2_pid_Toffset_sets", "pid_t0", 24);
    
    // disconnect from server
    delete gDBOld;
    delete gDBNew;
}

