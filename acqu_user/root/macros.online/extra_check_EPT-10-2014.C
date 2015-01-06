{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");

    watcher->Divide(3,1);
    watcher->SetWindowPosition(1920,500);
    watcher->SetWindowSize(1700,700);

    watcher->cd(1);
    FPD_TimeOR->Draw();

    watcher->cd(2);
    gPad->SetLogz();
    NaI_Hits_v_TimeOR->Draw("colz");

    watcher->cd(3);
    gPad->SetLogz();
    TwoD2000v1400->Draw("colz");

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",2000);

    update_timer->Start();
}
