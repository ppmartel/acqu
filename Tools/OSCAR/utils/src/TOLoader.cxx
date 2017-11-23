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


#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "THnSparse.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCutG.h"
#include "TSystemDirectory.h"
#include "TChain.h"

#include "TOLoader.h"
#include "TOSUtils.h"
#include "TONumberReader.h"
#include "TOArray2D.h"

//______________________________________________________________________________
void TOLoader::LoadTaggerCalibration(const Char_t* fileName, Int_t nChannel,
                                     Double_t* taggEnergy, Double_t* taggEnergyErr,
                                     Bool_t quiet)
{
    // Load the photon tagger energy calibration file produced by ugcal (short format)
    // for 'nChannel' channels and save it into the arrays 'taggEnergy' and 'taggEnergyErr'.
   
    // read the tagger calibration
    TONumberReader r(fileName, 3);

    // get the number of read values
    Int_t nch = r.GetNrow();

    // check number of read-in channels
    if (nch >= nChannel) 
    {
        // loop over the channels and set the tagger calibration
        for (Int_t i = 0; i < nChannel; i++) 
        {
            if (taggEnergy)    taggEnergy[i]    = r.GetData()->Get(1, i);
            if (taggEnergyErr) taggEnergyErr[i] = r.GetData()->Get(2, i);
        }
        
        if (!quiet)
            Info("TOLoader::LoadTaggerCalibration", "Loaded tagger energy calib. for %d channels from '%s'", 
                 nChannel, fileName);
    }
    else
        Error("TOLoader::LoadTaggerCalibration", "Could not load all of the %d tagger channels!", nChannel);
}

//______________________________________________________________________________
void TOLoader::LoadTaggingEfficiency(const Char_t* fileName, Int_t nChannel,
                                     Double_t* taggEff)
{
    // Load the tagging efficiency calibration file formatted in two columns 
    // containing the tagging efficiency value for one channel for 'nChannel'
    // channels and save it into the array 'taggEff'.
   
    // read the tagging efficiency
    TONumberReader r(fileName, 1);

    // get the number of read values
    Int_t nch = r.GetNrow();

    // check number of read-in channels
    if (nch >= nChannel) 
    {
        // loop over the channels and set the efficiencies
        for (Int_t i = 0; i < nChannel; i++) taggEff[i] = r.GetData()->Get(0, i);

        Info("TOLoader::LoadTaggingEfficiency", "Loaded tagging efficiency for %d channels from '%s'", 
             nChannel, fileName);
    }
    else
        Error("TOLoader::LoadTaggingEfficiency", "Could not read all of the %d tagger channels!", nChannel);
}

//______________________________________________________________________________
Int_t TOLoader::LoadBadChannels(const Char_t* fileName, Int_t* badChannels, Bool_t quiet)
{
    // Load bad channels from an ASCII file (format: one channel
    // number per line) and save them into the array 'badChannels'.
    // Return the number of bad channels.
 
    // read the bad channels
    TONumberReader r(fileName, 1);

    // create the bad channel array and copy the channel numbers
    Int_t nBad = r.GetNrow();
    for (Int_t i = 0; i < nBad; i++) badChannels[i] = r.GetData()->Get(0, i);

    // check number of read-in channels
    if (!quiet)
        Info("TOLoader::LoadBadChannels", "Loaded %d bad channels from '%s'", nBad, fileName);
    
    return nBad;
}

//______________________________________________________________________________
void TOLoader::ListCanvasObjects(const Char_t* fName, const Char_t* cName)
{
    // List the objects of the canvas 'cName' saved in the ROOT file 'fName'.
    // load the canvas
    
    TCanvas* c;
    if (!LoadObject(fName, cName, &c, "", "q"))
    {
        Error("TOLoader::ListCanvasObjects", "No canvas named '%s' found in ROOT file '%s'!",
              cName, fName);
        return;
    }

    // get list of objects
    TList* l = c->GetListOfPrimitives();
    
    // print out objects
    TIter next(l);
    TObject* o; 
    printf("Canvas content:\n");
    printf("TYPE            NAME\n");
    while ((o = (TObject*)next())) printf("%-15s %s\n", o->ClassName(), o->GetName());
}

//______________________________________________________________________________
Bool_t TOLoader::LoadHistogramFromCanvas(const Char_t* fName, const Char_t* cName,
                                         const Char_t* hName, TH1** outH)
{
    // Load the histogram named 'hName' embedded in the canvas 'cName' from the 
    // ROOT file 'fName' and assign the pointer of 'outH' to it.
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    // load the canvas
    TCanvas* c;
    if (!LoadObject(fName, cName, &c, "", "q"))
    {
        Error("TOLoader::LoadHistogramFromCanvas", "No canvas named '%s' found in ROOT file '%s'!",
              cName, fName);
        return kFALSE;
    }

    // get list of objects
    TList* l = c->GetListOfPrimitives();
    
    // search histogram
    TIter next(l);
    TObject* o;
    while ((o = (TObject*)next()))
    {
        // check histogram object
        if (o->InheritsFrom("TH1"))
        {
            // check name
            if (!strcmp(o->GetName(), hName))
            {
                // clone object
                *outH = (TH1*) o->Clone();
                (*outH)->SetDirectory(0);
                
                // object was loaded
                return kTRUE;
            }
        }
    }

    // clean-up
    delete c;

    // object was not loaded
    Error("TOLoader::LoadHistogramFromCanvas", "No histogram '%s' of canvas '%s' found in ROOT file '%s'!",
          hName, cName, fName);
    return kFALSE;
}

//______________________________________________________________________________
void TOLoader::LoadFilesToChain(const Char_t* loc, TChain* chain)
{
    // Add all ROOT files in the location 'loc' to the TChain 'chain'.
    
    LoadFilesToChains(loc, &chain, 1);
}

//______________________________________________________________________________
void TOLoader::LoadFilesToChains(const Char_t* loc, TChain** chains, Int_t nCh)
{
    // Add all ROOT files in the location 'loc' to 'nCh' TChains 'chains' with
    // equal distribution of the number of file per chain.
    
    Char_t tmp[256];

    // try to get directory content
    TSystemDirectory dir("rawdir", loc);
    TList* list = dir.GetListOfFiles();
    if (!list)
    {
        Error("TOLoader::LoadFilesToChains", "'%s' is not a directory!", loc);
        return;
    }

    // sort files
    list->Sort();

    // loop over directory content
    Int_t n = 0;
    TIter next(list);
    TSystemFile* f;
    while ((f = (TSystemFile*)next()))
    {
        // look for ROOT files
        TString str(f->GetName());
        if (str.EndsWith(".root"))
        {
            // full path
            sprintf(tmp, "%s/%s", loc, f->GetName());

            // user information
            Info("TOLoader::LoadFilesToChains", "[%d] Adding '%s' to chain %d", n+1, tmp, (n % nCh));
            
            // add file to chains
            chains[n % nCh]->Add(tmp);

            // count file
            n++;
        }
    }
    
    // clean-up
    delete list;
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TH1** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TH1*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TH2** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TH2*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TH3** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TH3*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, THnSparse** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, THnSparse*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TF1** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TF1*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TGraph** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TGraph*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TGraphErrors** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TGraphErrors*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TCutG** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TCutG*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObject(const Char_t* fName, const Char_t* objName, TCanvas** outObj,
                            const Char_t* objDescr, const Char_t* opt)
{
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectGeneric(fName, objName, outObj, objDescr, opt);
}

//______________________________________________________________________________
Bool_t TOLoader::LoadObjects(const Char_t* fName, const Char_t* objName, TCanvas*** outObj,
                             Int_t startID, Int_t endID,
                             const Char_t* objDescr, const Char_t* opt)
{
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    return LoadObjectsGeneric(fName, objName, outObj, startID, endID, objDescr, opt);
}

//______________________________________________________________________________
template <class T>
Bool_t TOLoader::LoadObjectGeneric(const Char_t* fName, const Char_t* objName, T** outObj, 
                                   const Char_t* objDescr, const Char_t* opt)
{   
    // Template loader function for all serializable ROOT objects. This method is 
    // called by the overloaded explicit methods LoadObject() for the various object types.
    //
    // Load the object named 'objName' from the ROOT file 'fName' and assign
    // the pointer of 'outObj' to it. The string 'objDescr' is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    // check options
    Bool_t quiet = kFALSE;
    if (TOSUtils::Contains(opt, "q")) quiet = kTRUE;

    // set output object pointer to 0
    *outObj = 0;

    // try to open the file
    TFile f(fName);
    
    // check if file exists
    if (!f.IsZombie())
    {
        // try to load the object
        f.GetObject(objName, *outObj);
        if (*outObj) 
        {
            // clone the object
            *outObj = (T*) (*outObj)->Clone();
            
            // unset directory for histograms
            if ((*outObj)->InheritsFrom("TH1")) 
            {
                TH1* h = (TH1*) *outObj;
                h->SetDirectory(0);
            }

            // successful object loading
            if (!quiet)
            {   
                if (!strcmp(objDescr, ""))
                    Info("TOLoader::LoadObject", "Loaded the %s object '%s'", 
                         (*outObj)->ClassName(), objName);
                else
                    Info("TOLoader::LoadObject", "Loaded the %s object '%s' (%s)", 
                         (*outObj)->ClassName(), objName, objDescr);
            }
            return kTRUE;
        }
        else 
        {
            // object loading error
            if (!quiet)
            {
                if (!strcmp(objDescr, ""))
                    Error("TOLoader::LoadObject", "Could not load the object '%s'!", 
                          objName);
                else
                    Error("TOLoader::LoadObject", "Could not load the object '%s' (%s)!", 
                          objName, objDescr);
            }
            return kFALSE;
        }
    }
    else 
    {
        // file opening error
        Error("TOLoader::LoadObject", "Could not open the ROOT file '%s' containing the '%s' object!", 
              fName, objName);
        return kFALSE;
    }
}

//______________________________________________________________________________
template <class T>
Bool_t TOLoader::LoadObjectsGeneric(const Char_t* fName, const Char_t* objName, T*** outObj, 
                                    Int_t startID, Int_t endID,
                                    const Char_t* objDescr, const Char_t* opt)
{   
    // Template loader function for all serializable ROOT objects. This method is 
    // called by the overloaded explicit methods LoadObjects() for the various object types.
    //
    // Load the objects named 'objName' containing a '%d' that will be iterated from
    // 'startID' to 'endID' from the ROOT file 'fName' to the array of pointers
    // 'outObj'. The string 'objDescr' (also containing a '%d') is used for 
    // information and error messages.
    // Options can be specified in 'opt' (q=quiet).
    //
    // Return kTRUE if the loading was successful without any error, otherwise kFALSE.

    Char_t name[256];
    Char_t descr[256] = "";

    // loop over objects
    Int_t p = 0;
    for (Int_t i = startID; i <= endID; i++)
    {
        // create the object name
        sprintf(name, objName, i);
        
        // create the object description
        if (strcmp(objDescr, "")) sprintf(descr, objDescr, i);
        
        // get the output pointer
        T** obj = &(outObj[0][p++]);

        // load the object
        if (!LoadObjectGeneric(fName, name, obj, descr, opt))
        {
            Error("TOLoader::LoadObjects", "Could not load all objects!");
            return kFALSE;
        }
    }

    return kTRUE;
}

