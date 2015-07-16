//--Author	JRM Annand   13th Jan 2007
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Check plots of TAPS PbWO4 spectra
//

void TAPSPbWO4Clear(){
	TA2Detector* t = (TA2Detector*)(gAN->GetGrandChild("PbWO4"));
	t->ZeroAll();
}

void CheckTAPSPbWO4(TCanvas* canv){
	if(canv==NULL) {
		TAPSPbWO4Clear();
		return;
	}
	
  Char_t* hname[] = {
    "PbWO4_NADChits",
    "PbWO4_NTDChits",
    "PbWO4_Nhits",
    "PbWO4_ADCHits",
    "PbWO4_TDCHits",
    "PbWO4_Hits",
    "PbWO4_EnergyOR",
    "PbWO4_TimeOR",
    "PbWO4_EnergyOR_v_TimeOR",
    "PbWO4_Hits_v_EnergyOR",
    "PbWO4_Hits_v_TimeOR",
    //"PbWO4_ClPhi_v_ClTheta"
    "PbWO4_Etot"
  };
  Int_t log[] = { 0,0,0,1,1,1,1,0,0,0,0,0 };
  Int_t col[] = { 2,2,2,4,4,4,3,3,1,1,1,1 };
  Char_t* xname[] = {
    "Number of TAPS-PbWO4 RAW ADC Hits per Event",
    "Number of TAPS-PbWO4 TDC Hits per Event",
    "Number of TAPS-PbWO4 Hits (ADC and TDC) per Event",
    "TAPS-PbWO4 RAW ADC Hits distribution",
    "TAPS-PbWO4 TDC Hits distribution",
    "TAPS-PbWO4 Hits (ADC and TDC) distribution",
    "TAPS-PbWO4 Energy OR (MeV)",
    "TAPS-PbWO4 Time OR (ns)",
    "TAPS-PbWO4 Energy (MeV) vs Time (ns)",
    "TAPS-PbWO4 Pulse-Height vs Hits",
    "TAPS-PbWO4 Time vs Hits",
    //"TAPS-PbWO4 #theta vs #phi distribution of Cluster Hits"
    "TAPS-PbWO4 Total Energy"
  };

  TH1F* h1;

  canv->SetFillStyle(4000);
  canv->Divide(3,4,0.01,0.01);
  for( Int_t i=0; i<8; i++ ){
      h1 = (TH1F*)(gROOT->FindObject(hname[i]));
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
  TH2F* h2;
  for( Int_t i=8; i<12; i++ ){
    h2 = (TH2F*)(gROOT->FindObject(hname[i]));
    if( !h2 ){
      printf("No root histogram %s\n",hname[i]);
      continue;
    }
    canv->cd(i+1);
    if( log[i] ) canv->GetPad(i+1)->SetLogz();
    h2->GetXaxis()->SetTitle(xname[i]);
    h2->Draw("COLZ");
  }
  return;
}


