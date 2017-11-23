// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCWriteARCalib                                                       //
//                                                                      //
// Write AcquRoot calibration files.                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCWriteARCalib.h"



//______________________________________________________________________________
TCWriteARCalib::TCWriteARCalib(CalibDetector_t det, const Char_t* templateFile)
{
    // Constructor.
    
    // init members
    fDetector = det;
    strcpy(fTemplate, templateFile);
}

//______________________________________________________________________________
void TCWriteARCalib::Write(const Char_t* calibFile, 
                           const Char_t* calibration, Int_t run)
{
    // Write the calibration file 'calibFile' for the run 'run' using the
    // calibration 'calibration'.
    
    // get MySQL manager
    TCMySQLManager* m = TCMySQLManager::GetManager();
    
    // read the template file
    Bool_t isTagger = kFALSE;
    if (fDetector == kDETECTOR_TAGG) isTagger = kTRUE;
    TCReadARCalib* r = new TCReadARCalib(fTemplate, isTagger);
     
    // read SG for TAPS
    TCReadARCalib* rSG = 0;
    if (fDetector == kDETECTOR_TAPS) 
        rSG = new TCReadARCalib(fTemplate, kFALSE, "TAPSSG:");

    // get the number of detectors
    Int_t nDet = r->GetNelements();
    Int_t nDetTW = r->GetNtimeWalks();
    Int_t nDetSG = 0;
    if (rSG) nDetSG = rSG->GetNelements();

    // create parameter array
    Double_t par[nDet];

    // check the detetector
    switch (fDetector)
    {
        case kDETECTOR_TAGG:
        {
            // read time offset
            if (m->ReadParametersRun("Data.Tagger.T0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetOffset(par[i]);

            break;
 
        }
        case kDETECTOR_CB:
        {
            // read time offset
            if (m->ReadParametersRun("Data.CB.T0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetOffset(par[i]);

            // read ADC gain
            if (m->ReadParametersRun("Data.CB.E1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetADCGain(par[i]);
            
            if (nDetTW)
            {
                // read time walk parameter 0
                if (m->ReadParametersRun("Data.CB.Walk.Par0", calibration, run, par, nDetTW))
                    for (Int_t i = 0; i < nDetTW; i++) r->GetTimeWalk(i)->SetPar0(par[i]);

                // read time walk parameter 1
                if (m->ReadParametersRun("Data.CB.Walk.Par1", calibration, run, par, nDetTW))
                    for (Int_t i = 0; i < nDetTW; i++) r->GetTimeWalk(i)->SetPar1(par[i]);

                // read time walk parameter 2
                if (m->ReadParametersRun("Data.CB.Walk.Par2", calibration, run, par, nDetTW))
                    for (Int_t i = 0; i < nDetTW; i++) r->GetTimeWalk(i)->SetPar2(par[i]);

                // read time walk parameter 3
                if (m->ReadParametersRun("Data.CB.Walk.Par3", calibration, run, par, nDetTW))
                    for (Int_t i = 0; i < nDetTW; i++) r->GetTimeWalk(i)->SetPar3(par[i]);
            }

            break;
        }
        case kDETECTOR_TAPS:
        {
            // read time offset
            if (m->ReadParametersRun("Data.TAPS.T0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetOffset(par[i]);

            // read TDC gain
            if (m->ReadParametersRun("Data.TAPS.T1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetTDCGain(par[i]);

            // read ADC pedestal
            if (m->ReadParametersRun("Data.TAPS.LG.E0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetPedestal(par[i]);

            // read ADC gain
            if (m->ReadParametersRun("Data.TAPS.LG.E1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetADCGain(par[i]);
            
            // read CFD threshold
            if (m->ReadParametersRun("Data.TAPS.CFD", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetEnergyLow(par[i]);
             
            if (nDetSG)
            {
                // read SG ADC pedestal
                if (m->ReadParametersRun("Data.TAPS.SG.E0", calibration, run, par, nDetSG))
                    for (Int_t i = 0; i < nDetSG; i++) rSG->GetElement(i)->SetPedestal(par[i]);

                // read SG ADC gain
                if (m->ReadParametersRun("Data.TAPS.SG.E1", calibration, run, par, nDetSG))
                    for (Int_t i = 0; i < nDetSG; i++) rSG->GetElement(i)->SetADCGain(par[i]);
            }

            break;
        }
        case kDETECTOR_PID:
        {
            // read phi angle
            if (m->ReadParametersRun("Data.PID.Phi", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetZ(par[i]);

            // read time offset
            if (m->ReadParametersRun("Data.PID.T0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetOffset(par[i]);

            // read ADC pedestal
            if (m->ReadParametersRun("Data.PID.E0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetPedestal(par[i]);

            // read ADC gain
            if (m->ReadParametersRun("Data.PID.E1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetADCGain(par[i]);

            break;
        }
        case kDETECTOR_VETO:
        {
            // read time offset
            if (m->ReadParametersRun("Data.Veto.T0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetOffset(par[i]);

            // read TDC gain
            if (m->ReadParametersRun("Data.Veto.T1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetTDCGain(par[i]);
            
            // read ADC pedestal
            if (m->ReadParametersRun("Data.Veto.E0", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetPedestal(par[i]);

            // read ADC gain
            if (m->ReadParametersRun("Data.Veto.E1", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetADCGain(par[i]);

            // read LED thresholds
            if (m->ReadParametersRun("Data.Veto.LED", calibration, run, par, nDet))
                for (Int_t i = 0; i < nDet; i++) r->GetElement(i)->SetEnergyLow(par[i]);

            break;
        }
        case kDETECTOR_NODET:
        {
            break;
        }
    }
    
    // open the template file
    std::ifstream ftemp;
    ftemp.open(fTemplate);
        
    // check if file is open
    if (!ftemp.is_open())
    {
        Error("Write", "Could not open template AcquRoot calibration file!");
        return;
    }
 
    // open the output file
    FILE* fout = fopen(calibFile, "w");
    if (!fout)
    {   Error("Write", "Could not open new AcquRoot calibration file!");
        return;
    }

    // read template file
    Char_t out[245];
    Int_t nElem = 0;
    Int_t nElemTW = 0;
    Int_t nElemSG = 0;
    
    // read the file
    while (ftemp.good())
    {
        TString line;
        line.ReadLine(ftemp, kFALSE);
            
        // check for element line
        if(line.BeginsWith("Element:"))
        {
            r->GetElement(nElem++)->Format(out);
            fprintf(fout, "Element: %s\n", out);
        }
        else if(line.BeginsWith("TimeWalk:"))
        {
            r->GetTimeWalk(nElemTW++)->Format(out);
            fprintf(fout, "TimeWalk: %s\n", out);
        }
        else if(line.BeginsWith("TAPSSG:"))
        {
            rSG->GetElement(nElemSG++)->Format(out);
            fprintf(fout, "TAPSSG: %s\n", out);
        }
        else
        {
            // write normal line
            fprintf(fout, "%s\n", line.Data());
        }
    }

    // close the files
    ftemp.close();
    fclose(fout);

    // clean-up
    delete r;
    if (rSG) delete rSG;
}
ClassImp(TCWriteARCalib)
