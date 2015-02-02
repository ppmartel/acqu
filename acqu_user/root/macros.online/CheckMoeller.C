void CheckMoeller() {

//   gROOT->SetStyle("Plain");

//  TFile *input = new TFile("nsummed_31-04Feb.root");

  TCanvas *c[9];
  Int_t ic = 0;

  TString start_name = "TDC_Vup_Left_Pair_Hel_";
  TString und = "_";

  for (Int_t nvup=0; nvup<4; nvup++) {
    for (Int_t hel=0; hel<2; hel++) {
      
      TString canvas_name = "Vuprom_"; canvas_name += nvup; 
      canvas_name += "_Helicity_"; canvas_name += hel;
      cout << "Drawing canvas " << canvas_name << endl;
      c[ic] = new TCanvas(canvas_name, canvas_name);
      c[ic]->Divide(10, 5);
      
      for (Int_t nright=0; nright<5; nright++) {
	for (Int_t nleft=0; nleft<10; nleft++) {
	  
	  TString hname = start_name;
	  hname += nvup; hname += und; hname += nleft; hname += und;
	  hname += nright; hname += und; hname += hel;
	  // 	    cout << "Left " << nleft << "\t right " << nright << "\t plot " << hname << endl;
	  TH1* histo = (TH1*) gROOT->FindObject(hname);
	  
	  Int_t npad = nleft+1 + 10*nright;
	  c[ic]->cd(npad);
 	  histo->Draw();
	  TString title = "L"; title += nleft; 
	  title += "R"; title += nright;
	  histo->SetTitle(title);
	  histo->SetName(title);
	}
      }
      ic++;
    }
  }

  TString outputname = "results_26Jan/Moeller_allplots_moeller.root";

  TFile *Output = new TFile(outputname, "RECREATE");
  for (Int_t ic=0; ic<9; ic++) {
    cout << "Writing canvas " << ic << endl;
    c[ic]->Write();
  }
  Output->Write();
  Output->Close();

  input->Close();
}
