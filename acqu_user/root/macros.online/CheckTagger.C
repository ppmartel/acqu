//--Author	JRM Annand   13th Jan 2007
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Check plots of Tagger spectra
//


void TaggerClear(){
	TA2Detector* t = (TA2Detector*)(gAN->GetGrandChild("FPD"));
	t->ZeroAll();
}

CheckTagger(TCanvas* canv){

	if(canv==NULL) {
		TaggerClear();
		return;
	}

  Char_t* hname[] = {
    "FPD_Nhits", "FPD_NhitsPrompt", "FPD_NhitsRand",  
    "FPD_Hits", "FPD_ScalerCurr", "FPD_ScalerAcc",
    "FPD_Hits_v_TimeOR","FPD_TimeOR","TAGG_PhotonEnergy",
    "FPD_Hits_v_EnergyOR","FPD_EnergyOR",
    "TAGG_Micro_Hits_v_FPD_Hits",
  };
  Int_t log[] = { 0,0,0,0,1,1,0,0,0,0,0,0 };
  Int_t col[] = { 2,2,2,4,5,5,3,3,3,3,3,3 };
  Char_t* xname[] = {
    "Number of Tagger Hits per Event",
    "Number of Prompt Tagger Hits per Event",
    "Number of Random Tagger Hits per Event",
    "Tagger Hits distribution",
    "Tagger Current Scaler Read",
    "Tagger Accumulated Scalers",
    "Tagger Time vs Hits",
    "Tagger Time (ns)",
    "Tagger Photon Energy",
    "Tagger Pulse-Height vs Hits",
    "Tagger Pulse-Height (MeV)",
    "Microscope Hits vs Tagger Hits",
  };
  TH1* h1;
  Int_t npads = 0;
  canv->SetFillStyle(4000);
  if( (gROOT->FindObject(hname[10])) || (gROOT->FindObject(hname[11]))){
    canv->Divide(3,4,0.01,0.01);
    npads = 12;
  }
  else{
    canv->Divide(3,3,0.01,0.01);
    npads = 9;
  }

  TString cname;
  for( Int_t i=0; i<npads; i++ ){
    h1 = (TH1*)(gROOT->FindObject(hname[i]));
    if( !h1 ){
      printf("No root histogram %s\n",hname[i]);
      continue;
    }
    canv->cd(i+1);
    h1->GetXaxis()->SetTitle(xname[i]);

    cname = h1->ClassName();
    if( cname.Contains("TH1") ){
      h1->SetLineColor( 1 );
      h1->SetFillColor( col[i] );
      if( log[i] ) canv->GetPad(i+1)->SetLogy();
      h1->Draw();
    }
    else h1->Draw("COLZ");
  }
  return;
}


