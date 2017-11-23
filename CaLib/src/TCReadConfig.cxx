// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadConfig                                                         //
//                                                                      //
// Read CaLib configuration files.                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCReadConfig.h"



// init static class members
TCReadConfig* TCReadConfig::fgReadConfig = 0;


//______________________________________________________________________________
TCReadConfig::TCReadConfig()
{
    // Constructor. 
    
    // init members
    fConfigTable = new THashTable();
    fConfigTable->SetOwner(kTRUE);
    
    // try to get the CaLib source path from the shell variable CALIB
    // otherwise use the current directory
    if (gSystem->Getenv("CALIB")) fCaLibPath = gSystem->Getenv("CALIB");
    else fCaLibPath = gSystem->pwd();
    
    // read the main configuration file
    ReadConfigFile("config/config.cfg");
}

//______________________________________________________________________________
TCReadConfig::~TCReadConfig()
{
    // Destructor.

    if (fConfigTable) delete fConfigTable;
}

//______________________________________________________________________________
void TCReadConfig::ReadConfigFile(const Char_t* cfgFile)
{
    // Read the configuration file 'cfgFile'.

    // build file name
    Char_t filename[128];
    sprintf(filename, "%s/%s", fCaLibPath.Data(), cfgFile); 

    // open the file
    std::ifstream infile;
    infile.open(filename);
        
    // check if file is open
    if (!infile.is_open())
    {
        Error("ReadConfigFile", "Could not open configuration file '%s'", filename);
    }
    else
    {
        Info("ReadConfigFile", "Reading configuration file '%s'", filename);
        
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
                // extract and save configuration element
                TCConfigElement* elem = CreateConfigElement(line);
                
                // check element
                if (!elem) continue;

                // check for FILE key
                if (*elem->GetKey() == "FILE") ReadConfigFile(*elem->GetValue());
            
                // add element to hash table
                fConfigTable->Add(elem);
            }
        }
    }
    
    // close the file
    infile.close();
}

//______________________________________________________________________________
TCConfigElement* TCReadConfig::CreateConfigElement(TString line)
{       
    // Create and return a configuration element (key: value) using the string
    // 'line'.
    // Return 0 if there was something wrong.

    // get bounds
    Ssiz_t aa = line.First(":")+1;
    Ssiz_t bb = line.Length()-aa;
    
    // extract the key
    TString key = line(0, aa-1);
    key.Remove(TString::kBoth, ' ');

    // extract the value
    TString value = line(aa, bb);
    value.Remove(TString::kBoth, ' ');

    // check for empty key
    if (key == "") return 0;

    // create the configuration element
    return new TCConfigElement(key.Data(), value.Data());
}

//______________________________________________________________________________
TString* TCReadConfig::GetConfig(TString configKey)
{   
    // Get the configuration value of the configuration key 'configKey'.
    // Return 0 if no such element exists.
    
    // search the configuration element
    TCConfigElement* elem = (TCConfigElement*) fConfigTable->FindObject(configKey);
    
    // return configuration value
    if (elem) return elem->GetValue();
    else return 0;
}

//______________________________________________________________________________
Int_t TCReadConfig::GetConfigInt(TString configKey)
{
    // Get the configuration value of the configuration key 'configKey' 
    // converted to Int_t.
    // Return 0 if no such element exists.

    // get value as string
    TString* v = GetConfig(configKey);

    // return value
    if (v) return atoi(v->Data());
    else return 0;
}

//______________________________________________________________________________
Double_t TCReadConfig::GetConfigDouble(TString configKey)
{
    // Get the configuration value of the configuration key 'configKey' 
    // converted to Double_t.
    // Return 0 if no such element exists.

    // get value as string
    TString* v = GetConfig(configKey);

    // return value
    if (v) return atof(v->Data());
    else return 0;
}

//______________________________________________________________________________
void TCReadConfig::GetConfigDoubleDouble(TString configKey, Double_t* out1, Double_t* out2)
{
    // Read two Double_t values of the configuration key 'configKey' and save them
    // to 'out1' and 'out2'.

    // get value as string
    TString* v = GetConfig(configKey);
    
    // check value
    if (!v) return;

    // read values
    Double_t d1, d2;
    Int_t n = sscanf(v->Data(), "%lf%lf", &d1, &d2);

    // check result
    if (n == 2)
    {
        *out1 = d1;
        *out2 = d2;
    }
    else
    {
        Error("GetConfigDoubleDouble", "Problems reading two double values from '%s'", 
              v->Data());
    }
}
ClassImp(TCReadConfig)
