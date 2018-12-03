{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");

    Int_t isCB = gROOT->ProcessLine("TString s1 = gAR->GetFileName();s1.Contains(\"CB\")");
    Int_t isTagg = gROOT->ProcessLine("TString s1 = gAR->GetFileName();s1.Contains(\"Tagg\")");

    if(isCB && isTagg){
      watcher->Divide(2,2);
      watcher->SetWindowSize(1200,1000);
    }
    else {
      watcher->Divide(2,1);
      watcher->SetWindowSize(1000,500);
    }
    watcher->SetWindowPosition(1920,500);

    Int_t pad=1;

    if(isTagg){
      watcher->cd(pad++);
      gPad->SetLogy();
      if(FPD_ScalerCurr) FPD_ScalerCurr->Draw();

      watcher->cd(pad++);
      if(FPD_TimeAll) FPD_TimeAll->Draw();
    }

    if(isCB){
      watcher->cd(pad++);
      gPad->SetLogy(); //Edo May.06.2018
      if(MWPC_Wires_Hits){
	//MWPC_Wires_Hits->GetXaxis()->SetRangeUser(230,528); Paolo jan.11.2018
	MWPC_Wires_Hits->Draw();
      }
      
      watcher->cd(pad++);
      gPad->SetLogz();
      if(NaI_Hits_v_TimeOR) NaI_Hits_v_TimeOR->Draw("colz");
    }

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",4000);

    update_timer->Start();
}
