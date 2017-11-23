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


#ifndef TCREADCONFIG_H
#define TCREADCONFIG_H

#include <fstream>
#include <cstdlib>

#include "TSystem.h"
#include "TString.h"
#include "THashTable.h"
#include "TError.h"


class TCConfigElement : public TObject
{

private:
    TString key;                // config key
    TString value;              // config value

public:
    TCConfigElement(const Char_t* k, const Char_t* v) : key(k), value(v) {  } 
    virtual ~TCConfigElement() { }
    TString* GetKey() { return &key; }
    TString* GetValue() { return &value; }
    virtual const Char_t* GetName() const { return key.Data(); }
    virtual ULong_t Hash() const { return key.Hash(); }
    
    ClassDef(TCConfigElement, 0) // Key-value based configuration element
};


class TCReadConfig
{

private:
    THashTable* fConfigTable;           // hash table containing config elements
    TString fCaLibPath;                 // path of the calib source
    static TCReadConfig* fgReadConfig;  // pointer to the static instance of this class

    void ReadConfigFile(const Char_t* cfgFile);
    TCConfigElement* CreateConfigElement(TString line);
    
public:
    TCReadConfig();
    virtual ~TCReadConfig();

    TString* GetConfig(TString configKey);
    Int_t GetConfigInt(TString configKey);
    Double_t GetConfigDouble(TString configKey);
    void GetConfigDoubleDouble(TString configKey, Double_t* out1, Double_t* out2);
    
    static TCReadConfig* GetReader() 
    {
        // return a pointer to the static instance of this class
        if (!fgReadConfig) fgReadConfig = new TCReadConfig();
        return fgReadConfig; 
    }

    ClassDef(TCReadConfig, 0) // Configuration file reader
};

#endif

