// SVN Info: $Id: TA2MyRateEstimation.cc 558 2010-03-31 09:00:52Z werthm $

/*************************************************************************
 * Author: Dominik Werthmueller, 2013
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TA2TAPSAnalysis                                                      //
//                                                                      //
// Class used for the analysis of TAPS stand-alone data.                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TA2TAPSAnalysis.h"



//______________________________________________________________________________
TA2TAPSAnalysis::TA2TAPSAnalysis(const char* name, TA2Analysis* analysis) 
    : TA2Physics(name, analysis)
{
    // Constructor.
    
    // init members
    fTAPS = 0;
    fBaF2_LG = 0;
    fBaF2_LGS = 0;
    fBaF2_SG = 0;
    fBaF2_SGS = 0;
    fVeto = 0;
    fPWO = 0;
    fPWO_S = 0;
    fPWO_Veto = 0;
    fPWO_Veto_S = 0;
    
    fTAPSPattern = 0;
    fBaF2CFDPattern = 0;
    fBaF2LED1Pattern = 0;
    fBaF2LED2Pattern = 0;
    fBaF2CFDNhits = 0;
    fBaF2CFDHits = 0;
    fBaF2LED1Nhits = 0;
    fBaF2LED1Hits = 0;
    fBaF2LED2Nhits = 0;
    fBaF2LED2Hits = 0;
    fVetoLEDNhits = 0;
    fVetoLEDHits = 0;

    fH_BaF2_LG = 0;
    fH_BaF2_LG_CFD = 0;
    fH_BaF2_LG_LED1 = 0;
    fH_BaF2_LG_LED2 = 0;
    fH_BaF2_Time = 0;

    fH_BaF2_LGS = 0;
    fH_BaF2_LGS_CFD = 0;
    fH_BaF2_LGS_LED1 = 0;
    fH_BaF2_LGS_LED2 = 0;

    fH_BaF2_SG = 0;
    fH_BaF2_LG = 0;
    
    fH_Veto = 0;
    fH_Veto_Time = 0;

    fH_PWO = 0;
    fH_PWO_CFD = 0;
    fH_PWO_Time = 0;
    
    fH_PWO_S = 0;
    fH_PWO_S_CFD = 0;
    fH_PWO_S_Time = 0;
    
    fH_PWO_Veto = 0;
    fH_PWO_Veto_CFD = 0;
    fH_PWO_Veto_Time = 0;
    
    fH_PWO_Veto_S = 0;
    fH_PWO_Veto_S_CFD = 0;
    fH_PWO_Veto_S_Time = 0;
}

//______________________________________________________________________________
TA2TAPSAnalysis::~TA2TAPSAnalysis()
{
    // Destructor.

}

//______________________________________________________________________________
void TA2TAPSAnalysis::SetConfig(Char_t* line, Int_t key)
{
    // Read in analysis configuration parameters and configure the class. 
    
    // default parent class SetConfig()
    TA2Physics::SetConfig(line, key);
}

//______________________________________________________________________________ 
void TA2TAPSAnalysis::PostInit()
{
    // Do some further initialisation after all setup parameters have been read in.
    
    Char_t tmp[256];

    printf("\n");

    // get the TAPS apparatus
    fTAPS = (TA2GenericApp*) fParent->GetChild("TAPS", "TA2GenericApp");
    if (fTAPS) printf("Found TAPS apparatus\n");

    // get the TAPS detectors
    if (fTAPS)
    {
        // get the BaF2 LG detector
        fBaF2_LG = (TA2GenericDetector*) fTAPS->GetChild("BaF2_LG", "TA2GenericDetector");
        if (fBaF2_LG) printf("Found BaF2_LG detector\n");
        
        // get the BaF2 LGS detector
        fBaF2_LGS = (TA2GenericDetector*) fTAPS->GetChild("BaF2_LGS", "TA2GenericDetector");
        if (fBaF2_LGS) printf("Found BaF2_LGS detector\n");
        
        // get the BaF2 SG detector
        fBaF2_SG = (TA2GenericDetector*) fTAPS->GetChild("BaF2_SG", "TA2GenericDetector");
        if (fBaF2_SG) printf("Found BaF2_SG detector\n");
        
        // get the BaF2 SGS detector
        fBaF2_SGS = (TA2GenericDetector*) fTAPS->GetChild("BaF2_SGS", "TA2GenericDetector");
        if (fBaF2_SGS) printf("Found BaF2_SGS detector\n");
        
        // get the Veto detector
        fVeto = (TA2GenericDetector*) fTAPS->GetChild("Veto", "TA2GenericDetector");
        if (fVeto) printf("Found Veto detector\n");
         
        // get the PWO detector
        fPWO = (TA2GenericDetector*) fTAPS->GetChild("PWO", "TA2GenericDetector");
        if (fPWO) printf("Found PWO detector\n");
        
        // get the PWO sensitive detector
        fPWO_S = (TA2GenericDetector*) fTAPS->GetChild("PWO_S", "TA2GenericDetector");
        if (fPWO_S) printf("Found PWO_S detector\n");
        
        // get the PWO Veto detector
        fPWO_Veto = (TA2GenericDetector*) fTAPS->GetChild("PWO_Veto", "TA2GenericDetector");
        if (fPWO_Veto) printf("Found PWO_Veto detector\n");
        
        // get the PWO Veto sensitive detector
        fPWO_Veto_S = (TA2GenericDetector*) fTAPS->GetChild("PWO_Veto_S", "TA2GenericDetector");
        if (fPWO_Veto_S) printf("Found PWO_Veto_S detector\n");
    }
    
    // get the TAPS bit pattern
    fTAPSPattern     = fParent->GetBitPattern();
    fBaF2CFDPattern  = GetPatternIndex("BaF2_CFD_Pat", fTAPSPattern);
    fBaF2LED1Pattern = GetPatternIndex("BaF2_LED1_Pat", fTAPSPattern);
    fBaF2LED2Pattern = GetPatternIndex("BaF2_LED2_Pat", fTAPSPattern);
    fVetoLEDPattern  = GetPatternIndex("Veto_LED_Pat", fTAPSPattern);
    
    if (fBaF2CFDPattern >= 0)
    {
        fBaF2CFDHits = fTAPSPattern->GetHits(fBaF2CFDPattern);
        printf("BaF2 CFD Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2CFDPattern), fBaF2CFDPattern);
    }
    if (fBaF2LED1Pattern >= 0)
    {
        fBaF2LED1Hits = fTAPSPattern->GetHits(fBaF2LED1Pattern);
        printf("BaF2 LED1 Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2LED1Pattern), fBaF2LED1Pattern);
    }
    if (fBaF2LED2Pattern >= 0)
    {
        fBaF2LED2Hits = fTAPSPattern->GetHits(fBaF2LED2Pattern);
        printf("BaF2 LED2 Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fBaF2LED2Pattern), fBaF2LED2Pattern);
    }
    if (fVetoLEDPattern >= 0)
    {
        fVetoLEDHits = fTAPSPattern->GetHits(fVetoLEDPattern);
        printf("Veto LED Pattern '%s' found at index %d\n", fTAPSPattern->GetPatternName(fVetoLEDPattern), fVetoLEDPattern);
    }
    
    // create BaF2_LG histograms
    if (fBaF2_LG)
    {
        Int_t nElem = fBaF2_LG->GetNelement();
        fH_BaF2_LG = new TH1*[nElem];
        fH_BaF2_LG_CFD = new TH1*[nElem];
        fH_BaF2_LG_LED1 = new TH1*[nElem];
        fH_BaF2_LG_LED2 = new TH1*[nElem];
        fH_BaF2_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "BaF2_LG_%03d", i+1);
            fH_BaF2_LG[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LG_CFD_%03d", i+1);
            fH_BaF2_LG_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LG_LED1_%03d", i+1);
            fH_BaF2_LG_LED1[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LG_LED2_%03d", i+1);
            fH_BaF2_LG_LED2[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_Time_%03d", i+1);
            fH_BaF2_Time[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
        }
    }

    // create BaF2_LGS histograms
    if (fBaF2_LGS)
    {
        Int_t nElem = fBaF2_LGS->GetNelement();
        fH_BaF2_LGS = new TH1*[nElem];
        fH_BaF2_LGS_CFD = new TH1*[nElem];
        fH_BaF2_LGS_LED1 = new TH1*[nElem];
        fH_BaF2_LGS_LED2 = new TH1*[nElem];
        
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "BaF2_LGS_%03d", i+1);
            fH_BaF2_LGS[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LGS_CFD_%03d", i+1);
            fH_BaF2_LGS_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LGS_LED1_%03d", i+1);
            fH_BaF2_LGS_LED1[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "BaF2_LGS_LED2_%03d", i+1);
            fH_BaF2_LGS_LED2[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
        }
    }

    // create BaF2_SG histograms
    if (fBaF2_SG)
    {
        Int_t nElem = fBaF2_SG->GetNelement();
        fH_BaF2_SG = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "BaF2_SG_%03d", i+1);
            fH_BaF2_SG[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
        }
    }

    // create BaF2_SGS histograms
    if (fBaF2_SGS)
    {
        Int_t nElem = fBaF2_SGS->GetNelement();
        fH_BaF2_SGS = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "BaF2_SGS_%03d", i+1);
            fH_BaF2_SGS[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
        }
    }

    // create Veto histograms
    if (fVeto)
    {
        Int_t nElem = fVeto->GetNelement();
        fH_Veto = new TH1*[nElem];
        fH_Veto_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "Veto_%03d", i+1);
            fH_Veto[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "Veto_Time_%03d", i+1);
            fH_Veto_Time[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
        }
    }

    // create PWO histograms
    if (fPWO)
    {
        Int_t nElem = fPWO->GetNelement();
        fH_PWO = new TH1*[nElem];
        fH_PWO_CFD = new TH1*[nElem];
        fH_PWO_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "PWO_%03d", i+1);
            fH_PWO[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_CFD_%03d", i+1);
            fH_PWO_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_Time_%03d", i+1);
            fH_PWO_Time[i] = new TH1F(tmp, tmp, 12000, -10000, 2000);
        }
    }

    // create PWO_S histograms
    if (fPWO_S)
    {
        Int_t nElem = fPWO_S->GetNelement();
        fH_PWO_S_CFD = new TH1*[nElem];
        fH_PWO_S = new TH1*[nElem];
        fH_PWO_S_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "PWO_S_%03d", i+1);
            fH_PWO_S[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_S_CFD_%03d", i+1);
            fH_PWO_S_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_S_Time_%03d", i+1);
            fH_PWO_S_Time[i] = new TH1F(tmp, tmp, 12000, -10000, 2000);
         }
    }
    
    // create PWO Veto histograms
    if (fPWO_Veto)
    {
        Int_t nElem = fPWO_Veto->GetNelement();
        fH_PWO_Veto = new TH1*[nElem];
        fH_PWO_Veto_CFD = new TH1*[nElem];
        fH_PWO_Veto_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "PWO_Veto_%03d", i+1);
            fH_PWO_Veto[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_Veto_CFD_%03d", i+1);
            fH_PWO_Veto_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_Veto_Time_%03d", i+1);
            fH_PWO_Veto_Time[i] = new TH1F(tmp, tmp, 12000, -10000, 2000);
        }
    }
    
    // create PWO_Veto_S histograms
    if (fPWO_Veto_S)
    {
        Int_t nElem = fPWO_Veto_S->GetNelement();
        fH_PWO_Veto_S = new TH1*[nElem];
        fH_PWO_Veto_S_CFD = new TH1*[nElem];
        fH_PWO_Veto_S_Time = new TH1*[nElem];
        for (Int_t i = 0; i < nElem; i++)
        {
            sprintf(tmp, "PWO_Veto_S_%03d", i+1);
            fH_PWO_Veto_S[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_Veto_S_CFD_%03d", i+1);
            fH_PWO_Veto_S_CFD[i] = new TH1F(tmp, tmp, 4096, 0, 4096);
            sprintf(tmp, "PWO_Veto_S_Time_%03d", i+1);
            fH_PWO_Veto_S_Time[i] = new TH1F(tmp, tmp, 12000, -10000, 2000);
         }
    }
 
    // call PostInit of parent class
    TA2Physics::PostInit();
}

//______________________________________________________________________________ 
Int_t TA2TAPSAnalysis::GetPatternIndex(const Char_t* patternName, TA2BitPattern* pattern)
{
    // Returns the index of the pattern called 'patternName' in the bit pattern 'pattern'.
    // If no such pattern is found -1 is returned.

    // check if pattern exists
    if (!pattern) return -1;

    // loop over pattern
    for (Int_t i = 0; i < pattern->GetNPattern(); i++)
    {
        if (strcmp(patternName, pattern->GetPatternName(i)) == 0) return i;
    }

    return -1;
}

//______________________________________________________________________________
void TA2TAPSAnalysis::FillADCHits(TA2Detector* det, TH1** h)
{
    // Fill the raw ADC hits in the detector 'det' into the histogram array 'h'.

    // check for raw hits
    if (det->IsRawHits())
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < det->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = det->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = det->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            h[elem]->Fill(value_adc);
        }
    }
}

//______________________________________________________________________________
void TA2TAPSAnalysis::FillTDCHits(TA2Detector* det, TH1** h)
{
    // Fill the raw TDC hits in the detector 'det' into the histogram array 'h'.

    // check for raw hits
    if (det->IsRawHits())
    {
        // loop over TDC hits
        for (UInt_t i = 0; i < det->GetNTDChits(); i++)
        {
            // get element number
            Int_t elem = det->GetRawTimeHits()[i];
            
            // get TDC value
            UShort_t value_tdc = det->GetElement(elem)->GetRawTDCValue();
            
            // fill uncut TDC hits
            h[elem]->Fill(value_tdc);
        }
    }
}

//______________________________________________________________________________
void TA2TAPSAnalysis::FillTDCHitsM(TA2Detector* det, TH1** h)
{
    // Fill the TDC multi-hits in the detector 'det' into the histogram array 'h'.

    // loop over all hits
    for (UInt_t i = 0; i < det->GetNhits(); i++)
    {
        // get element number
        Int_t elem = det->GetHits()[i];
            
        // fill time
        h[elem]->Fill(det->GetTime()[elem]);
    }
}

//______________________________________________________________________________
void TA2TAPSAnalysis::Reconstruct()
{
    // Main analysis method that is called for every event.
    
    // fill BaF2 LG ADC spectra
    if (fBaF2_LG && fBaF2_LG->IsRawHits())
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fBaF2_LG->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fBaF2_LG->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fBaF2_LG->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_BaF2_LG[elem]->Fill(value_adc);
            
            // get pattern hits
            if (fBaF2CFDHits) fBaF2CFDNhits = fTAPSPattern->GetNHits()[fBaF2CFDPattern];
            if (fBaF2LED1Hits) fBaF2LED1Nhits = fTAPSPattern->GetNHits()[fBaF2LED1Pattern];
            if (fBaF2LED2Hits) fBaF2LED2Nhits = fTAPSPattern->GetNHits()[fBaF2LED2Pattern];

            // fill hits where CFD fired
            for (UInt_t j = 0; j < fBaF2CFDNhits; j++)
            {
                if (fBaF2CFDHits[j] == elem) 
                {
                    fH_BaF2_LG_CFD[elem]->Fill(value_adc);
                }
            }

            // fill hits where LED1 fired
            for (UInt_t j = 0; j < fBaF2LED1Nhits; j++)
            {
                if (fBaF2LED1Hits[j] == elem) fH_BaF2_LG_LED1[elem]->Fill(value_adc);
            }

            // fill hits where LED2 fired
            for (UInt_t j = 0; j < fBaF2LED2Nhits; j++)
            {
                if (fBaF2LED2Hits[j] == elem) fH_BaF2_LG_LED2[elem]->Fill(value_adc);
            }
        }
        
        // fill raw TDC hits
        FillTDCHits(fBaF2_LG, fH_BaF2_Time);
    }

    // fill BaF2 LGS ADC spectra
    if (fBaF2_LGS && fBaF2_LGS->IsRawHits())
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fBaF2_LGS->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fBaF2_LGS->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fBaF2_LGS->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_BaF2_LGS[elem]->Fill(value_adc);
            
            // get pattern hits
            if (fBaF2CFDHits) fBaF2CFDNhits = fTAPSPattern->GetNHits()[fBaF2CFDPattern];
            if (fBaF2LED1Hits) fBaF2LED1Nhits = fTAPSPattern->GetNHits()[fBaF2LED1Pattern];
            if (fBaF2LED2Hits) fBaF2LED2Nhits = fTAPSPattern->GetNHits()[fBaF2LED2Pattern];

            // fill hits where CFD fired
            for (UInt_t j = 0; j < fBaF2CFDNhits; j++)
            {
                if (fBaF2CFDHits[j] == elem) 
                {
                    fH_BaF2_LGS_CFD[elem]->Fill(value_adc);
                }
            }

            // fill hits where LED1 fired
            for (UInt_t j = 0; j < fBaF2LED1Nhits; j++)
            {
                if (fBaF2LED1Hits[j] == elem) fH_BaF2_LGS_LED1[elem]->Fill(value_adc);
            }

            // fill hits where LED2 fired
            for (UInt_t j = 0; j < fBaF2LED2Nhits; j++)
            {
                if (fBaF2LED2Hits[j] == elem) fH_BaF2_LGS_LED2[elem]->Fill(value_adc);
            }
        }
    }

    // fill BaF2 SG spectra
    if (fBaF2_SG)
    {
        // fill raw ADC hits
        FillADCHits(fBaF2_SG, fH_BaF2_SG);
    }

    // fill BaF2 SGS spectra
    if (fBaF2_SGS)
    {
        // fill raw ADC hits
        FillADCHits(fBaF2_SGS, fH_BaF2_SGS);
    }
    
    // fill Veto spectra
    if (fVeto)
    {
        // fill raw ADC hits
        FillADCHits(fVeto, fH_Veto);

        // fill raw TDC hits
        FillTDCHits(fVeto, fH_Veto_Time);
    }

    // fill PWO spectra
    if (fPWO && fPWO->IsRawHits())
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fPWO->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fPWO->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fPWO->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_PWO[elem]->Fill(value_adc);
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fPWO->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fPWO->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fH_PWO_CFD[elem]->Fill(value_adc);
            }
        }

        // fill raw TDC hits
        FillTDCHitsM(fPWO, fH_PWO_Time);
    }

    // fill PWO_S spectra
    if (fPWO_S)
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fPWO_S->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fPWO_S->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fPWO_S->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_PWO_S[elem]->Fill(value_adc);
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fPWO_S->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fPWO_S->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fH_PWO_S_CFD[elem]->Fill(value_adc);
            }
        }

        // fill raw TDC hits
        FillTDCHitsM(fPWO_S, fH_PWO_S_Time);
    }
    
    // fill PWO_Veto spectra
    if (fPWO_Veto)
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fPWO_Veto->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fPWO_Veto->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fPWO_Veto->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_PWO_Veto[elem]->Fill(value_adc);
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fPWO_Veto->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fPWO_Veto->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fH_PWO_Veto_CFD[elem]->Fill(value_adc);
            }
        }
         
        // fill raw TDC hits
        FillTDCHitsM(fPWO_Veto, fH_PWO_Veto_Time);
    }
    
    // fill PWO_Veto_S spectra
    if (fPWO_Veto_S)
    {
        // loop over ADC hits
        for (UInt_t i = 0; i < fPWO_Veto_S->GetNADChits(); i++)
        {
            // get element number
            Int_t elem = fPWO_Veto_S->GetRawEnergyHits()[i];
            
            // get ADC value
            UShort_t value_adc = fPWO_Veto_S->GetElement(elem)->GetRawADCValue();
            
            // fill uncut ADC hits
            fH_PWO_Veto_S[elem]->Fill(value_adc);
            
            // fill hits where CFD (TDC) fired
            for (UInt_t j = 0; j < fPWO_Veto_S->GetNTDChits(); j++)
            {
                // get element number
                Int_t tdc = fPWO_Veto_S->GetRawTimeHits()[j];
                    
                // fill hits where CFD fired
                if (tdc == elem) fH_PWO_Veto_S_CFD[elem]->Fill(value_adc);
            }
        }

        // fill raw TDC hits
        FillTDCHitsM(fPWO_Veto_S, fH_PWO_Veto_S_Time);
    }

}
ClassImp(TA2TAPSAnalysis)
