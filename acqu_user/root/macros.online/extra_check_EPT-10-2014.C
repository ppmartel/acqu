{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");

    watcher->Divide(4,1);
    watcher->SetWindowPosition(1920,500);
    watcher->SetWindowSize(1700,700);

    watcher->cd(1);
    gPad->SetLogy();
    FPD_ScalerCurr->Draw();

    watcher->cd(2);
    gPad->SetLogy();
    MWPC_Wires_Hits->Draw();
    //watcher->cd(2);
    //FPD_TimeOR->Draw();

    watcher->cd(3);
    gPad->SetLogz();
    NaI_Hits_v_TimeOR->Draw("colz");

    watcher->cd(4);
    gPad->SetLogz();
    TwoD2000v1400->Draw("colz");

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",2000);

    update_timer->Start();
}
