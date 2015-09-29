// checks online analysis of MWPC

void MWPCClear(){
	TA2Detector* s = (TA2Detector*)(gAN->GetGrandChild("Strips"));
	TA2Detector* w = (TA2Detector*)(gAN->GetGrandChild("Wires"));
	TA2Apparatus* a = (TA2Apparatus*)(gAN->GetChild("MWPC"));

	if( s ) s->ZeroAll();
	else printf("MWPC Strip detector class not found\n");
	if( w ) w->ZeroAll();
	else printf("MWPC Wire detector class not found\n");
	if( a ) a->ZeroAll();
	else printf("CB apparatus class not found\n");
}


CheckMWPC(TCanvas* canv){
  Char_t* hname[] = {
    "MWPC_Strips_Hits",
    "MWPC_Wires_Hits",
    "MWPC_Strips_Nhits",
    "MWPC_Wires_Nhits",

  };
  Int_t log[] = { 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  Int_t col[] = { 6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  Char_t* xname[] = {
    "MWPC Strip Hit Pattern",
    "MWPC Wire Hit Pattern",
    "MWPC Strip No. of Hits",
    "MWPC Wire No. of Hits",
  };
  TH1F* h1;
  canv->SetFillStyle(4000);
  canv->Divide(2,2,0.01,0.01);
  for( Int_t i=0; i<4; i++ ){
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
  return;
   TH2F* h2;
   for( Int_t i=2; i<28; i++ ){
     h2 = (TH2F*)(gROOT->FindObjectAny(hname[i]));
     if( !h2 ){
       printf("No root histogram %s\n",hname[i]);
       continue;
     }
     canv->cd(i+1);
     if( log[i] ) canv->GetPad(i+1)->SetLogz();
     h2->GetXaxis()->SetTitle(xname[i]);
     h2->Draw("colz");
   }
   return;
}


