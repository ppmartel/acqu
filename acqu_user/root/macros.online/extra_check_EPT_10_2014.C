TCanvas* 	watcher;
TH1D*		newHistTaggerOr = 0;
TH1D*		oldHistTaggerOr = 0;


void	timerEvent_()
{
	if(!oldHistTaggerOr)
	{
		oldHistTaggerOr = (TH1D*)FPD_TimeOR->Clone();
		watcher->cd(1);
		oldHistTaggerOr->Draw();
		return;
	}
	if(newHistTaggerOr)
		delete newHistTaggerOr;
	newHistTaggerOr = (TH1D*)FPD_TimeOR->Clone();
	newHistTaggerOr->Add(oldHistTaggerOr, -1);
	if(oldHistTaggerOr)
		delete oldHistTaggerOr;
	oldHistTaggerOr = (TH1D*)FPD_TimeOR->Clone();
	
	watcher->cd(1);
    newHistTaggerOr->Draw();

    //watcher->cd(2);
    //gPad->SetLogz();
    //NaI_Hits_v_TimeOR->Draw("colz");
}


void	extra_check_EPT_10_2014()
{
    watcher = new TCanvas("watcher","Watch Me");

    watcher->Divide(2,1);

    newHistTaggerOr->Draw();

    //watcher->cd(2);
    //gPad->SetLogz();
    //NaI_Hits_v_TimeOR->Draw("colz");
    
    TTimer* update_timer = new TTimer(2000, kFALSE);
    update_timer->Connect("Timeout()",0,0,"timerEvent_()");
    update_timer->Start();
}
