//--Author	JRM Annand   13th Jan 2007
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Check plots of Pizza detector spectra
//

void PizzaClear(){
	TA2Detector* t = (TA2Detector*)(gAN->GetGrandChild("Pizza"));
	TA2Apparatus* a = (TA2Apparatus*)(gAN->GetChild("PizzaApp"));
	if( t ) t->ZeroAll();
	else printf("Pizza detector class not found\n");
	if( a ) a->ZeroAll();
	else printf("Pizza apparatus class not found\n");
}

CheckPizza(TCanvas* canv){
	if(canv==NULL) {
		PizzaClear();
		return;
	}
 
  Char_t* hname[] = {
    "Pizza_NADChits",
    "Pizza_NTDChits",
    "Pizza_Nhits",
    "Pizza_ADCHits",
    "Pizza_TDCHits",
    "Pizza_Hits"
  };
  Int_t log[] = { 1,0,0,1,1,1 };
  Int_t col[] = { 2,2,2,4,4,4 };
  Char_t* xname[] = {
    "Number of Pizza RAW ADC Hits per Event",
    "Number of Pizza TDC Hits per Event",
    "Number of Pizza Hits (ADC and TDC) per Event",
    "Pizza RAW ADC Hits distribution",
    "Pizza TDC Hits distribution",
    "Pizza Hits (ADC and TDC) distribution",
  };
  TH1F* h1;
  canv->SetFillStyle(4000);
  canv->Divide(3,3,0.01,0.01);
  for( Int_t i=0; i<6; i++ ){
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
  //  return;
}

