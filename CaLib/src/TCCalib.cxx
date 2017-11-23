// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalib                                                              //
//                                                                      //
// Abstract calibration module class.                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalib.h"



//______________________________________________________________________________
TCCalib::~TCCalib()
{
    // Destructor.
     
    if (fSet) delete [] fSet;
    if (fOldVal) delete [] fOldVal;
    if (fNewVal) delete [] fNewVal;
    if (fMainHisto) delete fMainHisto;
    if (fFitHisto) delete fFitHisto;
    if (fFitFunc) delete fFitFunc;
    if (fOverviewHisto) delete fOverviewHisto;
    //if (fCanvasFit) delete fCanvasFit;            // comment this to prevent crash
    //if (fCanvasResult) delete fCanvasResult;      // comment this to prevent crash
    if (fTimer) delete fTimer;
}

//______________________________________________________________________________
void TCCalib::Start(const Char_t* calibration, Int_t nSet, Int_t* set)
{
    // Start the calibration module for the 'nSet' sets in 'set' using the calibration 
    // identifier 'calibration'.
    
    // init members
    fCalibration = calibration;
    fNset = nSet;
    fSet = new Int_t[fNset];
    for (Int_t i = 0; i < fNset; i++) fSet[i] = set[i];
    fHistoName = "";
    fCurrentElem = 0;

    fMainHisto = 0;
    fFitHisto = 0;
    fFitFunc = 0;

    fOverviewHisto = 0;

    fCanvasFit = 0;
    fCanvasResult = 0;
    fExtraCanvas = 0;
    
    fAvr = 0;
    fAvrDiff = 0;
    fNcalc = 0;

    // create timer
    fTimer = new TTimer(100);
    fTimer->Connect("Timeout()", "TCCalib", this, "Next()");
    fTimerRunning = kFALSE;

    // create arrays
    fOldVal = new Double_t[fNelem];
    fNewVal = new Double_t[fNelem];

    // init arrays
    for (Int_t i = 0; i < fNelem; i++)
    {
        fOldVal[i] = 0;
        fNewVal[i] = 0;
    }
    
    // user information
    Info("Start", "Starting calibration module %s", GetName());
    Info("Start", "Module description: %s", GetTitle());
    for (Int_t i = 0; i < fNset; i++)
    {
        Int_t first_run = TCMySQLManager::GetManager()->GetFirstRunOfSet(fData.Data(), fCalibration.Data(), fSet[i]);
        Int_t last_run = TCMySQLManager::GetManager()->GetLastRunOfSet(fData.Data(), fCalibration.Data(), fSet[i]);
        Info("Start", "Calibrating set %d (Run %d to %d)", fSet[i], first_run, last_run);
    }

    
    // Read the convergence factor
    Char_t tmp[256];
    sprintf(tmp, "%s.ConvergenceFactor", GetName());
    if (!TCReadConfig::GetReader()->GetConfigDouble(tmp)) {
	    fConvergenceFactor = 1;
    }
    else fConvergenceFactor = TCReadConfig::GetReader()->GetConfigDouble(tmp);
    Info("Init", "Using a Convergence Factor of %f", fConvergenceFactor);
    
    // style options
    gStyle->SetPalette(1);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameFillColor(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(10);
    gStyle->SetCanvasColor(10);
    gStyle->SetStatColor(10);
    gStyle->SetFillColor(10);

    // draw the fitting canvas
    fCanvasFit = new TCanvas("Fitting", "Fitting", 0, 0, 400, 800);
    
    // connect event handler
    fCanvasFit->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TCCalib", this, 
                        "EventHandler(Int_t, Int_t, Int_t, TObject*)");

    // draw the result canvas
    fCanvasResult = new TCanvas("Result", "Result", gClient->GetDisplayWidth() - 900, 0, 900, 400);

    fExtraCanvas = new TCanvas("extra_canvas","Extra");
    
    // init sub-class
    Init();

    // start with the first element
    ProcessElement(0);
}

//______________________________________________________________________________
void TCCalib::EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected)
{
    // Event handler method.
    
    // catch key events
    if (event == kKeyPress)
    {
        // left key
        if (oy == kKey_Left) Previous();

        // right key
        if (oy == kKey_Right) Next();

        // space
        if (oy == kKey_Space) 
        {
            if (fTimerRunning) 
            {
                fTimer->Stop();
                fTimerRunning = kFALSE;
            }
            else 
            {
                fTimer->Start();
                fTimerRunning = kTRUE;
            }
        }
    }
}

//______________________________________________________________________________
void TCCalib::ProcessElement(Int_t elem, Bool_t ignorePrev)
{
    // Process the element 'elem'. If 'ignorePrev' is kTRUE the calculation
    // of the previous element will not be performed.

    // check if element is in range
    if (elem < 0 || elem >= fNelem)
    {
        // stop timer when it was active
        fTimer->Stop();
        fTimerRunning = kFALSE;

        // calculate last element and update result canvas
        if (elem == fNelem) 
        {
            if (!ignorePrev) Calculate(fCurrentElem);
            else printf("Ignoring element %d\n", fCurrentElem);
            fCanvasResult->Update();
        }

        // exit
        return;
    }
    
    // calculate previous element
    if (elem != fCurrentElem)
    {
        if (!ignorePrev) Calculate(fCurrentElem);
        else printf("Ignoring element %d\n", fCurrentElem);
    }

    // set current element
    fCurrentElem = elem;

    // process element
    Fit(elem);
}

//______________________________________________________________________________
void TCCalib::ProcessAll(Int_t msecDelay)
{
    // Process all elements using 'msecDelay' milliseconds delay.
    
    // check for delay
    if (msecDelay > 0)
    {
        // start automatic iteration
        fTimer->Start(msecDelay);
        fTimerRunning = kTRUE;
    }
    else
    {
        // loop over elements
        for (Int_t i = 0; i < fNelem; i++) Next();
    }
}

//______________________________________________________________________________
void TCCalib::Previous()
{
    // Process the previous element.

    ProcessElement(fCurrentElem - 1);
}

//______________________________________________________________________________
void TCCalib::Next()
{
    // Process the next element.
    
    ProcessElement(fCurrentElem + 1);
}

//______________________________________________________________________________
void TCCalib::Ignore()
{
    // Process the next element while skipping the calculation of the previous
    // element.
    
    ProcessElement(fCurrentElem + 1, kTRUE);
}

//______________________________________________________________________________
void TCCalib::StopProcessing()
{
    // Stop processing when in automatic mode.
    
    // stop timer when it was active
    fTimer->Stop();
    fTimerRunning = kFALSE;
}

//______________________________________________________________________________
void TCCalib::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    printf("\n");
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    old value: %12.8f    new value: %12.8f    "
               "diff: %6.2f %%\n",
               i, fOldVal[i], fNewVal[i],
               TCUtils::GetDiffPercent(fOldVal[i], fNewVal[i]));
    }
    printf("\n");
}

//______________________________________________________________________________
void TCCalib::PrintValuesChanged()
{
    // Print out the old and new values only for elements for which they changed.

    // loop over elements
    printf("\n");
    for (Int_t i = 0; i < fNelem; i++)
    {
        if (fOldVal[i] != fNewVal[i])
            printf("Element: %03d    old value: %12.8f    new value: %12.8f    "
                   "diff: %6.2f %%\n",
                   i, fOldVal[i], fNewVal[i],
                   TCUtils::GetDiffPercent(fOldVal[i], fNewVal[i]));
    }
    printf("\n");
}

//______________________________________________________________________________
void TCCalib::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
        TCMySQLManager::GetManager()->WriteParameters(fData.Data(), fCalibration.Data(), fSet[i], fNewVal, fNelem);
        
    // save overview picture
    SaveCanvas(fCanvasResult, "Overview");
}

//______________________________________________________________________________
void TCCalib::SaveCanvas(TCanvas* c, const Char_t* name)
{
    // Save the canvas 'c' to disk using the name 'name'.
    
    // get log directory
    if (TString* path = TCReadConfig::GetReader()->GetConfig("Log.Images"))
    {
        Char_t tmp[256];
        Char_t date[256];

        // create directory
        sprintf(tmp, "%s/%s", path->Data(), GetName());
        gSystem->mkdir(tmp, kTRUE);
        
        // format time stamp
        UInt_t day, month, year;
        UInt_t hour, min;
        TTimeStamp t;
        t.GetDate(kFALSE, 0, &year, &month, &day);
        t.GetTime(kFALSE, 0, &hour, &min);
        sprintf(date, "%d-%02d-%02d_%02d.%02d", year, month, day, hour, min);

        // save canvas (only for first set)
        sprintf(tmp, "%s/%s/%s_Set_%d_%s_%s.png", 
                path->Data(), GetName(), name, fSet[0], fCalibration.Data(), date);
        c->SaveAs(tmp);
    }
}
ClassImp(TCCalib)
