
CheckPhysics(TCanvas* canv){
  if(canv==NULL) {
    return;
  }

  Char_t* hname[] = {
    "PHYS_IM_2g",
    "PHYS_IM_2g_CB",
    "PHYS_IM_2g_TAPS",
    "PHYS_IM_2g_mix",
    "PHYS_IM_3g",
    "PHYS_IM_3g_CB", 
    "PHYS_IM_3g_E300", 
    "PHYS_IM_2g_TAGG",
    "PHYS_IM_2g_TAGGpr",
    "PHYS_IM_2g_TAGGr",
  };
  Int_t log[] = { 0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  Int_t col[] = { 2,2,2,2,3,3,3,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  Char_t* xname[] = {
    "Invariant Mass - 2 gamma",
    "Invariant Mass - 2 gamma (CB only)",
    "Invariant Mass - 2 gamma (TAPS only)",
    "Invariant Mass - 2 gamma (1CB-1TAPS)",
    "Invariant Mass - 3 gamma",
    "Invariant Mass - 3 gamma (CB only)",
    "Invariant Mass - 3 gamma (Eg > 300 MeV)",
    "Invariant Mass - 2 gamma v Tagger",
    "Invariant Mass - 2 gamma v Tagger (|tagg t| < 10)",
  };
  TH1F* h1;
  canv->SetFillStyle(4000);
  canv->Divide(3,3);
  for( Int_t i=0; i<8; i++ ){
      h1 = (TH1F*)(gROOT->FindObjectAny(hname[i]));
      if( !h1 ){
	printf("No root histogram %s\n",hname[i]);
	continue;
      }
      h1->SetLineColor( 1 );
      h1->SetFillColor( col[i] );
      canv->cd(i+1);
      if( log[i] ) canv->GetPad(i+1)->SetLogy();
      h1->GetXaxis()->SetTitle(xname[i]);
      h1->Draw();
  }

  // some hacky bit of code to do a pr sub right now
  TH1F* hpr = (TH1F*)(gROOT->FindObjectAny(hname[8]));
  TH1F* hr =  (TH1F*)(gROOT->FindObjectAny(hname[9])); 
 if (hpr && hr)
 {
    hpr->Add(hr,-0.1);
    canv->cd(9);
    hpr->Draw();
 }
}
