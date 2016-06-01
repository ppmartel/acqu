{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");

    Int_t isCB = gROOT->ProcessLine("TString s1 = gAR->GetFileName();s1.Contains(\"CB\")");

    if(isCB){
      watcher->Divide(2,2);
      watcher->SetWindowSize(1000,1000);
    }
    else {
      watcher->Divide(2,1);
      watcher->SetWindowSize(1000,500);
    }
    watcher->SetWindowPosition(1920,500);

    watcher->cd(1);
    gPad->SetLogy();
    if(FPD_ScalerCurr) FPD_ScalerCurr->Draw();

    watcher->cd(2);
    if(FPD_TimeOR) FPD_TimeOR->Draw();

    if(isCB){
      watcher->cd(3);
      if(MWPC_Wires_Hits) MWPC_Wires_Hits->Draw();
      
      watcher->cd(4);
      gPad->SetLogz();
      if(NaI_Hits_v_TimeOR) NaI_Hits_v_TimeOR->Draw("colz");
    }

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",4000);

    update_timer->Start();
}
