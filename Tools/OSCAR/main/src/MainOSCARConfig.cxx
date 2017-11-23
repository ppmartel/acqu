/*************************************************************************
 * Author: Dominik Werthmueller, 2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// MainOSCARConfig                                                      //
//                                                                      //
// Contains the main method for the oscar-config tool.                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include <cstdio>

#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TSystem.h"

#include "TOVersion.h"

Char_t gOSCAR_Env[256];

//______________________________________________________________________________
void PrintHelp()
{
    // Print the help to this application.

    printf("\n");
    printf("oscar-config - OSCAR configuration tool\n");
    printf("(C) 2010 by Dominik Werthmueller\n");
    printf("\n");
    printf("Usage: oscar-config [--incdir] [--help]\n");
    printf("\n");
}

//______________________________________________________________________________
TObjArray* GetModules()
{
    // Return an array of TObjString of the names of all OSCAR modules.
    // WARNING: The returned list has to be destroyed manually!

    TString m(OSCAR_MODULES);
    return m.Tokenize(",");
}

//______________________________________________________________________________
void PrintIncDir()
{
    // Print the include directories.
    
    TObjArray* modules = GetModules();
    TIterator* iter = modules->MakeIterator();
    while (TObjString* s = (TObjString*) iter->Next())
        printf("-I%s/%s/include ", gOSCAR_Env, s->GetString().Data());
    printf("\n");
    delete modules;
}

//______________________________________________________________________________
Int_t main(Int_t argc, Char_t* argv[])
{
    // Main method.
    
    // read and check OSCAR environment variable
    const Char_t* envOSCAR = gSystem->Getenv("OSCAR");
    if (envOSCAR == 0)
    {
        printf("Error: OSCAR environment variable is not defined!\n");
        return -1;
    }
    else strcpy(gOSCAR_Env, envOSCAR);

    // not enough arguments
    if (argc < 2) 
    {
        PrintHelp();
        return -1;
    }

    // loop over arguments
    for (Int_t i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--incdir")) PrintIncDir();
        else if (!strcmp(argv[i], "--help")) PrintHelp();
        else printf("Unknown argument '%s'!\n", argv[i]);
    }
}

