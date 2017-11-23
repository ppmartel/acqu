/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2011                               *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// MyFinishMacro.C                                                      //
//                                                                      //
// Save all histograms to a ROOT file.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


Char_t* fTmpCh = 0;


//______________________________________________________________________________
Int_t LastIndexOf(const Char_t* s, Char_t c)
{
    // Returns the position of the last occurence of the character c
    // in the string s. Returns -1 if c was not found (taken from OSCAR's TOSUtils).
    
    Char_t* pos = strrchr(s, (Int_t)c);
    if (pos) return pos-s;
    else return -1;
}

//______________________________________________________________________________
Char_t* ExtractFileName(const Char_t* s)
{
    // Extracts the file name of a file given by its full Unix paths in
    // the string s (taken from OSCAR's TOSUtils).
    
    // Create output string if empty
    if (!fTmpCh) fTmpCh = new Char_t[256];
    
    // Search last slash
    Int_t pos = LastIndexOf(s, '/');

    // Return the same string or copy substring
    if (pos == -1) sprintf(fTmpCh, "%s", s);
    else strncpy(fTmpCh, s+pos+1, strlen(s)-pos);
    
    return fTmpCh;
}

//______________________________________________________________________________
Char_t* ExtractPureFileName(const Char_t* s)
{
    // Extracts the file name and truncates the file ending of a file given 
    // by its full Unix path in the string s (taken from OSCAR's TOSUtils).
    
    // Create output string if empty
    if (!fTmpCh) fTmpCh = new Char_t[256];
   
    // Get the file name
    Char_t fn[256];
    sprintf(fn, "%s", ExtractFileName(s));
    
    // Search last dot
    Int_t pos = LastIndexOf(fn, '.');
    
    // Strip file ending
    if (pos == -1) sprintf(fTmpCh, "%s", fn);
    else
    {
        strncpy(fTmpCh, fn, pos);
        fTmpCh[pos] = '\0';
    }

    return fTmpCh;
}

//______________________________________________________________________________
Char_t* ExtractPureFileNameParDirPrefix(const Char_t* s)
{
    // Extracts the file name and truncates the file ending of a file given 
    // by its full Unix path in the string s (taken from OSCAR's TOSUtils).
    // Use the parent directory as prefix.
    
    Char_t tmp[256];
    Char_t tmp2[256];

    // Create output string if empty
    if (!fTmpCh) fTmpCh = new Char_t[256];
    
    // Search last slash
    Int_t pos = LastIndexOf(s, '/');

    // get string until filename
    strncpy(tmp, s, pos);
    tmp[pos]='\0';

    // get parent directory
    strcpy(tmp2, ExtractFileName(tmp));

    // create new string
    sprintf(tmp, "%s_%s", tmp2, ExtractPureFileName(s));
    strcpy(fTmpCh, tmp);

    return fTmpCh;
}

//______________________________________________________________________________
Bool_t IsOSCARTree()
{
    // Check if an A2 OSCAR Tree was created in the analysis.
    
    if (!gAR->GetAnalysis()->GetPhysics()) return kFALSE;

    TTree* t = gAR->GetAnalysis()->GetPhysics()->GetTree();

    if (t)
    {
        if (!strcmp(t->GetName(), "OA2")) return kTRUE;
        else return kFALSE;
    }
    else return kFALSE;
}

//______________________________________________________________________________
void CorrectTaggerScalers(Bool_t useNewScalers)
{
    // Correct the tagger scaler using the tagger livetime and the total
    // livetime.
    //
    // Arguments:
    //    useNewScalers:
    //       kTRUE   : use new scaler pairs (after 24th September 2008 / Run 20000)
    //       kFALSE  : use old scaler pairs
  
    Char_t name[256];
    
    // create total scaler histogram name
    sprintf(name, "SumScalers0to%d", gAR->GetMaxScaler());

    // load histograms
    TH1* hSc  = gDirectory->Get(name);
    TH1* hScC = gDirectory->Get("CorrectedSumScalers");
    TH1* hScT = (TH1*) gDirectory->Get("FPD_ScalerAcc");
    if (!hScT) hScT = (TH1*) gROOT->Get("FPD_ScalerAcc");

    // check if all histograms are here
    if (!hSc) 
    {
        printf("ERROR: Could not load the '%s' histograms which is required for the tagger scaler correction!\n", name);
        return;
    }
    if (!hScC) 
    {
        printf("ERROR: Could not load the 'CorrectedSumScalers' histograms which is required for the tagger scaler correction!\n");
        return;
    }
    if (!hScT) 
    {
        printf("ERROR: Could not load the 'FPD_ScalerAcc' histograms which is required for the tagger scaler correction!\n");
        return;
    }
    
    // load scaler values
    Double_t sc_0_ar   = hSc->GetBinContent(1);
    Double_t sc_1_ar   = hSc->GetBinContent(2);
    Double_t sc_144_ar = hSc->GetBinContent(145);
    Double_t sc_145_ar = hSc->GetBinContent(146);
    Double_t sc_534_ar = hSc->GetBinContent(535);
    Double_t sc_535_ar = hSc->GetBinContent(536);
    Double_t sc_528_ar = hSc->GetBinContent(529);
    Double_t sc_529_ar = hSc->GetBinContent(530);

    Double_t sc_0   = hScC->GetBinContent(1);
    Double_t sc_1   = hScC->GetBinContent(2);
    Double_t sc_144 = hScC->GetBinContent(145);
    Double_t sc_145 = hScC->GetBinContent(146);

    // calculate livetimes and the correction factor
    Double_t live_tot, live_tag, corr_fac;
    if (useNewScalers)
    {
        live_tot = sc_528_ar / sc_529_ar;
        live_tag = sc_535_ar / sc_534_ar;
        corr_fac = live_tot / live_tag;
    }
    else
    {
        live_tot = sc_1 / sc_0;
        live_tag = sc_145 / sc_144;
        corr_fac = live_tot / live_tag;
    }
   
    // divide correction factor by 2 to correct double scaler entries
    // in offline ROOT file processing
    if (!gAR->IsOnline()) corr_fac /= 2.;
    
    // calculate error
    Double_t fc2 = corr_fac * corr_fac;
    Double_t corr_fac_err;
    if (useNewScalers) corr_fac_err = TMath::Sqrt(fc2/sc_529_ar + fc2/sc_528_ar + fc2/sc_534_ar + fc2/sc_535_ar);
    else corr_fac_err = TMath::Sqrt(fc2/sc_0 + fc2/sc_1 + fc2/sc_144 + fc2/sc_145);

    // correct all tagger channels
    Int_t n_corr = 0;
    if (useNewScalers) strcpy(name, "FPD_ScalerAccCorrNew");
    else strcpy(name, "FPD_ScalerAccCorr");
    TH1F* hScTC = new TH1F(name, name, hScT->GetNbinsX(), 
                            hScT->GetXaxis()->GetXmin(), hScT->GetXaxis()->GetXmax());
    for (Int_t i = 0; i < hScT->GetNbinsX(); i++)
    {
        // read value and error
        Double_t tsc = hScT->GetBinContent(i+1);
        Double_t tsc_err = TMath::Sqrt(tsc);

        // calculate new value and error
        Double_t tscC, tscC_err;

        if (tsc == 0)
        {
            tscC = 0;
            tscC_err = 0;
        }
        else
        {
            tscC = tsc * corr_fac;
            tscC_err = TMath::Sqrt(tscC*tscC*tsc_err*tsc_err/tsc/tsc + 
                                   tscC*tscC*corr_fac_err*corr_fac_err/corr_fac/corr_fac);
        }

        // fill new value and error
        hScTC->SetBinContent(i+1, tscC);
        hScTC->SetBinError(i+1, tscC_err);
        n_corr++;
    }

    // correct p2/tagger ratios
    Double_t ratio, ratio_err;
    Double_t ratio_corr, ratio_corr_err;
    for (Int_t i = 0; i < 352; i++)
    {
        sprintf(name, "P2TaggerRatio_%d", i);
        ntup = (TNtupleD*) gDirectory->Get(name);
        if (ntup)
        {
            ntup->SetBranchAddress("ratio", &ratio);
            ntup->SetBranchAddress("ratio_err", &ratio_err);
            sprintf(name, "P2TaggerRatioCorr_%d", i);
            TNtupleD* ntup_corr = new TNtupleD(name, name, "ratio:ratio_err");
            for (Int_t j = 0; j < ntup->GetEntries(); j++)
            {
                ntup->GetEntry(j);
                ratio_corr = ratio / corr_fac;
                ratio_corr_err = TMath::Sqrt(ratio_corr*ratio_corr*ratio_err*ratio_err/ratio/ratio + 
                                             ratio_corr*ratio_corr*corr_fac_err*corr_fac_err/corr_fac/corr_fac);
                ntup_corr->Fill(ratio_corr, ratio_corr_err);
            }
        }
    }

    // print information
    printf("\n");
    printf("------- Tagger scaler correction -------\n");
    printf("\n");
    if (useNewScalers)
    {
        printf("Scaler 528 (CB live) : %e \n", sc_528_ar);
        printf("Scaler 529 (CB free) : %e \n", sc_529_ar);
        printf("Scaler 534 (FP free) : %e \n", sc_534_ar);
        printf("Scaler 535 (FP live) : %e \n", sc_535_ar);
    }
    else
    {
        printf("Scaler   0 (CB free) : %e (AR: %e, diff: %e)\n", sc_0,   sc_0_ar,    sc_0   - sc_0_ar);
        printf("Scaler   1 (CB live) : %e (AR: %e, diff: %e)\n", sc_1,   sc_1_ar,    sc_1   - sc_1_ar);
        printf("Scaler 144 (FP free) : %e (AR: %e, diff: %e)\n", sc_144, sc_144_ar,  sc_144 - sc_144_ar);
        printf("Scaler 145 (FP live) : %e (AR: %e, diff: %e)\n", sc_145, sc_145_ar,  sc_145 - sc_145_ar);
    }
    printf("\n");
    printf("livetime total    : %lf\n", live_tot);
    printf("livetime tagger   : %lf\n", live_tag);
    printf("correction factor : %lf\n", corr_fac);
    printf("\n");
    printf("-> %d tagger channels corrected\n", n_corr);
    printf("-> Ratio P2/tagger corrected\n", n_corr);
    printf("\n");
    printf("----------------------------------------\n");
    printf("\n");
}

//______________________________________________________________________________
void MyFinishMacro()
{
    // Main method.
    
    Char_t outFileName[256];
    
    printf("\nEnd of file: Executing MyFinishMacro ...\n");
    
    // check for physics analysis
    if (!gAR->GetAnalysis()->GetPhysics())
    {
        // Build the output file name depending on the type of analysis
        if (gAR->IsOnline()) sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                                     ExtractPureFileName(gAR->GetFileName()));
        else 
            sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                    ExtractPureFileNameParDirPrefix(gAR->GetTreeFile()->GetName()));

        // create output file and save objects
        TFile* f = new TFile(outFileName, "recreate");
        f->SetCompressionLevel(4);
        gROOT->GetList()->Write();
        printf("Saved all histograms to %s\n", f->GetName());
        delete f;
    }
    else
    {
        // get the run number
        TA2MyPhysics* ana = (TA2MyPhysics*)gAR->GetAnalysis()->GetPhysics();
        Int_t run = 0;
        if (ana) run = ana->GetRunNumber();
        
        // Correct tagger scalers for different dead-times in the tagger DAQ and
        // the CB DAQ, respectively.
        // On September 24 2008 a new pair of scalers was installed for this purpose
        // (after run 19000 (September 25 2008) this should work).
        CorrectTaggerScalers(kFALSE);
        if (run > 19000) CorrectTaggerScalers(kTRUE);
 
        // Check if output file is already open
        TFile* f;
        if (f = ana->GetOutputFile())
        {
            // Save histograms to file and close it
            f->Write();
            f->cd();
            gROOT->GetList()->Write();
            printf("Saved all histograms to %s\n", f->GetName());
            delete f;
        }
        else
        {
            // Build the output file name depending on the type of analysis
            if (gAR->IsOnline()) sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                                         ExtractPureFileName(gAR->GetFileName()));
            else 
                sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                        ExtractPureFileNameParDirPrefix(gAR->GetTreeFile()->GetName()));

            // Save histograms to file and close it
            f = new TFile(outFileName, "recreate");
            f->SetCompressionLevel(4);
            gROOT->GetList()->Write();
            printf("Saved all histograms to %s\n", f->GetName());
            delete f;
        }
    }

    gSystem->Exit(0);
}

