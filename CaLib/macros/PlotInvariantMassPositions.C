void PlotInvariantMassPositions() {
	gSystem->Load("../../a2display/build/liba2display.so");
	gROOT->LoadMacro("../acqu_user/root/macros/ExtractAllHistograms.C");

	TFile* f = new TFile("CBQuadEnergy2.root");
	
	TList* histos = ExtractAllHistograms(f);

	TH1F* pi0 = histos->FindObject("Pi0 position overview");
	pi0->Scale(1./135.0); //Divide(new TF1("pi0_mass","135",0,1000));
	TH1F* eta = histos->FindObject("Eta position overview");
	eta->Scale(1./548.0);
	
	//pi0->Draw();
	//cout << pi0->GetEntries() << endl;
	TH2CB* pi0_cb = new TH2CB("Pi0pos","Pi0 peak positions");
	pi0_cb->FillElements(*pi0);
	pi0_cb->GetZaxis()->SetRangeUser(0.95,1.05);

	TH2CB* eta_cb = new TH2CB("Etapos","Eta peak positions");
	eta_cb->FillElements(*eta);
	eta_cb->GetZaxis()->SetRangeUser(0.9,1.1);
	
	
	TCanvas* c = new TCanvas();
	c->Divide(1,2);

	c->cd(1);
	pi0_cb->Draw("colz");

	c->cd(2);
	eta_cb->Draw("colz");

	
	
}
