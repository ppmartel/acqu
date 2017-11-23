/*************************************************************************
 * Author: Dominik Werthmueller, 2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOGlobals                                                            //
//                                                                      //
// This namespace grants access to global objects and methods.          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TDatabasePDG.h"

#include "TOGlobals.h"

namespace TOGlobals
{
    TDatabasePDG* fPDG_DB = 0;
    
    const Double_t kDeuteronMass  = 1875.612762;
    const Double_t kProtonMass    = 938.27203;
    const Double_t kNeutronMass   = 939.56536;
    const Double_t kLambdaMass    = 1115.683;
    const Double_t kSigmaPMass    = 1189.37;
    const Double_t kSigmaMMass    = 1197.449;
    const Double_t kSigma0Mass    = 1192.642;
    const Double_t kEtaMass       = 547.853;
    const Double_t kPi0Mass       = 134.9766;
    const Double_t kPiChargedMass = 139.57018;
    const Double_t kK0Mass        = 497.614;
    const Double_t kKChargedMass  = 493.677;
    const Double_t kEtaPrimeMass  = 957.78;
    const Double_t kOmegaMass     = 782.65;
    const Double_t kHe3Mass       = 2808.391;
    const Double_t kDiprotonMass  = 1871.4;
    
    extern const Double_t kKChargedLifetime = 1.238e-8;
    extern const Double_t kKS0Lifetime      = 8.954e-11;
    extern const Double_t kLambdaLifetime   = 2.632e-10;
    extern const Double_t kSigmaPLifetime   = 8.018e-11;
    extern const Double_t kSigmaMLifetime   = 1.479e-10;
    extern const Double_t kSigma0Lifetime   = 0.074e-18;
    
    const Double_t kHbarGeVfm     = 0.197463569747999998;     // unit: GeV*fm/c
    
    const Int_t kDiProtonID       = 99998;
    const Int_t kDeuteronID       = 99999;

    TDatabasePDG* GetPDG_DB()
    {
        // Returns a pointer to the global PDG database object.

        // Create database if necessary
        if (!fPDG_DB) 
        {
            fPDG_DB = new TDatabasePDG;
            fPDG_DB->ReadPDGTable();
        
            // add the deuteron to the database
            fPDG_DB->AddParticle("diproton", "diproton", kDiprotonMass/1000., kTRUE,
                                 0., 6., "diproton", kDiProtonID, 0, 99);
            
            // add the deuteron to the database
            fPDG_DB->AddParticle("deuteron", "deuteron", kDeuteronMass/1000., kTRUE,
                                 0., 3., "deuteron", kDeuteronID, 0, 45);
        }

        return fPDG_DB;
    }
}

