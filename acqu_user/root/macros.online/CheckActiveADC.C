
CheckActiveADC(TCanvas* canv){
  if(canv==NULL) {
    return;
  }

  Char_t* hname[] = {
    "ADC216",
    "ADC215",
    "ADC218",
    "ADC219",
    "ADC220",
    "ADC221",
    "ADC222",
    "ADC223",
    "ADC224",
    "ADC225",
    "ADC226",
    "ADC227",
    "ADC228",
    "ADC229",
    "ADC214",
    "ADC231",
  };
  Int_t log[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0 };
  Int_t col[] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,0,0 };
  Char_t* xname[] = {
    "Active Target - ADC 1",
    "Active Target - ADC 2",
    "Active Target - ADC 3",
    "Active Target - ADC 4",
    "Active Target - ADC 5",
    "Active Target - ADC 6",
    "Active Target - ADC 7",
    "Active Target - ADC 8",
    "Active Target - ADC 9",
    "Active Target - ADC 10",
    "Active Target - ADC 11",
    "Active Target - ADC 12",
    "Active Target - ADC 13",
    "Active Target - ADC 14",
    "Active Target - ADC 15",
    "Active Target - ADC Sum",
  };
  TH1F* h1;
  canv->SetFillStyle(4000);
  canv->Divide(4,4);
  for( Int_t i=0; i<16; i++ ){
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
}
