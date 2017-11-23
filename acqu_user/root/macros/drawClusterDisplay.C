void drawAndSave(TH2Crystals* h, const char* filename)
{
	c = new TCanvas();
	NaI_ClustersAll->SetMarkerColor(kWhite);
	NaI_ClustersAll->Draw("text");
	h->Draw("samecolz");
	h->GetZaxis()->SetRangeUser(0.1,300);
	NaI_ClustersAll->Draw("textsame");
	gPad->SetLogz();
	gPad->Print(filename);
	c->Close();
}

void drawClusterDisplay()
{
	gStyle->SetLineScalePS(1.0);

	drawAndSave(NaI_ClustersEnergy, "ClustersOverview.eps");
	drawAndSave(NaI_ClustersSingle_0, "ClustersSingle0.eps");
	drawAndSave(NaI_ClustersSingle_1, "ClustersSingle1.eps");
	drawAndSave(NaI_ClustersSingle_2, "ClustersSingle2.eps");
}
