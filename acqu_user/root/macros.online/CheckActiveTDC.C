
CheckActiveTDC(TCanvas* canv){
  if(canv==NULL) {
    return;
  }

  Char_t* hname[] = {
    "ADC232M0",
    "ADC233M0",
    "ADC234M0",
    "ADC235M0",
    "ADC236M0",
    "ADC237M0",
    "ADC238M0",
    "ADC239M0",
    "ADC240M0",
    "ADC241M0",
    "ADC242M0",
    "ADC243M0",
    "ADC244M0",
    "ADC245M0",
    "ADC246M0",
    "ADC247M0",
  };
  Int_t log[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  Int_t col[] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0 };
  Char_t* xname[] = {
    "Active Target - TDC 1",
    "Active Target - TDC 2",
    "Active Target - TDC 3",
    "Active Target - TDC 4",
    "Active Target - TDC 5",
    "Active Target - TDC 6",
    "Active Target - TDC 7",
    "Active Target - TDC 8",
    "Active Target - TDC 9",
    "Active Target - TDC 10",
    "Active Target - TDC 11",
    "Active Target - TDC 12",
    "Active Target - TDC 13",
    "Active Target - TDC 14",
    "Active Target - TDC 15",
    "Active Target - TDC Sum",
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
