// SVN Info: $Id: MyFinishMacro.C 781 2011-02-03 13:39:41Z werthm $

/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2008                               *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// MyFinishMacro.C                                                      //
//                                                                      //
// Save all histograms and the OSCAR A2 event tree to a ROOT file.      //
// This macro should be called in batch mode using ARBatchAnalysis.     //
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

    // change into the main ROOT memory directory
    gROOT->cd();

    // load histograms
    //TH1F* hSc  = (TH1F*) gDirectory->Get("SumScalers0to632");
    TH1F* hSc  = (TH1F*) gDirectory->Get("SumScalers0to139");
    TH1F* hScC = (TH1F*) gDirectory->Get("CorrectedSumScalers");
    TH1F* hScT = (TH1F*) gDirectory->Get("FPD_ScalerAcc");

    // check if all histograms are here
    if (!hSc) 
    {
      //printf("ERROR: Could not load the 'SumScalers0to632' histograms which is required for the tagger scaler correction!\n");
        printf("ERROR: Could not load the 'SumScalers0to139' histograms which is required for the tagger scaler correction!\n");
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
    Double_t sc_14_ar  = hSc->GetBinContent(15);
    /*
    Double_t sc_144_ar = hSc->GetBinContent(145);
    Double_t sc_145_ar = hSc->GetBinContent(146);
    Double_t sc_534_ar = hSc->GetBinContent(535);
    Double_t sc_535_ar = hSc->GetBinContent(536);
    Double_t sc_528_ar = hSc->GetBinContent(529);
    Double_t sc_529_ar = hSc->GetBinContent(530);
    */

    Double_t sc_0   = hScC->GetBinContent(1);
    Double_t sc_1   = hScC->GetBinContent(2);
    Double_t sc_144 = hScC->GetBinContent(145);
    Double_t sc_145 = hScC->GetBinContent(146);

    // calculate livetimes and the correction factor
    Double_t live_tot, live_tag, corr_fac;
    /*
    if (useNewScalers)
    {
        live_tot = sc_528_ar / sc_529_ar;
        live_tag = sc_535_ar / sc_534_ar;
        corr_fac = live_tot / live_tag;
    }
    */
    if (useNewScalers)
    {
        live_tot = sc_1_ar / sc_0_ar;
        live_tag = sc_14_ar / sc_0_ar;
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
    
    Double_t fc2 = corr_fac * corr_fac;
    Double_t corr_fac_err = TMath::Sqrt(fc2/sc_0 + fc2/sc_1 + fc2/sc_144 + fc2/sc_145);

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
    /*
    if (useNewScalers)
    {
        printf("Scaler 528 (CB live) : %e \n", sc_528_ar);
        printf("Scaler 529 (CB free) : %e \n", sc_529_ar);
        printf("Scaler 534 (FP free) : %e \n", sc_534_ar);
        printf("Scaler 535 (FP live) : %e \n", sc_535_ar);
    }
    */
    if (useNewScalers)
    {
        printf("Scaler 0  (CB free) : %e \n", sc_0_ar);
        printf("Scaler 1  (CB live) : %e \n", sc_1_ar);
        printf("Scaler 14 (FP live) : %e \n", sc_14_ar);
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
    
    cout << endl;
    cout << "End of file: Executing MyFinishMacro ..." << endl;
    
    // get the run number
    Int_t run = ((TA2MyPhysics*)gAR->GetAnalysis()->GetPhysics())->GetRunNumber();

    // Correct tagger scalers 
    CorrectTaggerScalers(kTRUE);
    //CorrectTaggerScalers(kFALSE);
    //if (run > 20000) CorrectTaggerScalers(kTRUE);
    
    // Check if an A2 OSCAR tree was created
    if (IsOSCARTree())
    {
        // get the ROOT file from the A2 OSCAR tree
        TTree* t = gAR->GetAnalysis()->GetPhysics()->GetTree();
        TFile* f = t->GetCurrentFile();
        
        // Save histograms to file and close it
        f->cd();
        f->Write();
        gROOT->GetList()->Write();
        f->Close();
        
        cout << "A2 OSCAR Tree was found." << endl;
        cout << "All histograms and tree saved to " << f->GetName() << endl << endl;
    }
    else
    {
        // Build the output file name depending on the type of analysis
        if (gAR->IsOnline()) sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                                     ExtractPureFileName(gAR->GetFileName()));
        else sprintf(outFileName, "%sARHistograms_%s.root", gAR->GetTreeDir(), 
                     ExtractPureFileName(gAR->GetTreeFile()->GetName()));
        
        // Save histograms to file and close it
        TFile f(outFileName, "recreate");
        f.SetCompressionLevel(4);
        gROOT->GetList()->Write();
        f.Close();

        cout << "All histograms saved to " << outFileName << endl;
    }
    
    // clear histograms
    //gAN->ZeroAll();
    //cout << "Resetting histograms" << endl << endl;
 
    gSystem->Exit(0);
}

