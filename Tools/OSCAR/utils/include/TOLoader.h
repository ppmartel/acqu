/*************************************************************************
 * Author: Dominik Werthmueller, 2009-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOLoader                                                             //
//                                                                      //
// This namespace contains some methods to load objects from files.     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOLoader
#define OSCAR_TOLoader

#include "Rtypes.h"

class TChain;
class TH1;
class TH2;
class TH3;
class TF1;
class TGraph;
class TGraphErrors;
class THnSparse;
class TCanvas;
class TCutG;

namespace TOLoader
{
    void LoadTaggerCalibration(const Char_t* fileName, Int_t nChannel,
                               Double_t* taggEnergy, Double_t* taggEnergyErr, 
                               Bool_t quiet = kFALSE);
    void LoadTaggingEfficiency(const Char_t* fileName, Int_t nChannel,
                               Double_t* taggEff);
    Int_t LoadBadChannels(const Char_t* fileName, Int_t* badChannels, Bool_t quiet = kFALSE);
    void ListCanvasObjects(const Char_t* fName, const Char_t* cName);
    Bool_t LoadHistogramFromCanvas(const Char_t* fName, const Char_t* cName,
                                   const Char_t* hName, TH1** outH);
    void LoadFilesToChain(const Char_t* loc, TChain* chain);
    void LoadFilesToChains(const Char_t* loc, TChain** chains, Int_t nCh);

    // TH1
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TH1** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = "");
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TH1*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
    
    // TH2
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TH2** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = ""); 
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TH2*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
    // TH3
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TH3** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = ""); 
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TH3*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
    
    // THnSparse
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, THnSparse** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = ""); 
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, THnSparse*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
    
    // TF1
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TF1** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = "");
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TF1*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
     
    // TGraph
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TGraph** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = ""); 
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TGraph*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
     
    // TGraphErrors
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TGraphErrors** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = "");
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TGraphErrors*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
     
    // TCutG
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TCutG** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = "");
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TCutG*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");
     
    // TCanvas
    Bool_t LoadObject(const Char_t* fName, const Char_t* objName, TCanvas** outObj,
                      const Char_t* objDescr = "", const Char_t* opt = ""); 
    Bool_t LoadObjects(const Char_t* fName, const Char_t* objName, TCanvas*** outObj,
                       Int_t startID, Int_t endID,
                       const Char_t* objDescr = "", const Char_t* opt = "");

    // template methods
    template <class T>
    Bool_t LoadObjectGeneric(const Char_t* fName, const Char_t* objName, T** outObj,
                             const Char_t* objDescr = "", const Char_t* opt = "");
    template <class T>
    Bool_t LoadObjectsGeneric(const Char_t* fName, const Char_t* objName, T*** outObj,
                              Int_t startID, Int_t endID,
                              const Char_t* objDescr = "", const Char_t* opt = "");
}

#endif

