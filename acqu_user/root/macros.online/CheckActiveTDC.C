
CheckActiveTDC(TCanvas* canv){
  if(canv==NULL) {
    return;
  }

  Char_t* hname[] = {
    "ADC232M0",
    "ADC296M0",
    "ADC240M0",
    "ADC244M0",
    "ADC248M0",
    "ADC252M0",
    "ADC256M0",
    "ADC260M0",
    "ADC264M0",
    "ADC268M0",
    "ADC272M0",
    "ADC276M0",
    "ADC280M0",
    "ADC284M0",
    "ADC300M0",
    "ADC304M0",
  };
  Int_t log[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0 };
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
