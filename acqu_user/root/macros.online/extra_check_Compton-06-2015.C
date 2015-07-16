{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");

    watcher->Divide(4,2);
    watcher->SetWindowPosition(1920,500);
    watcher->SetWindowSize(1500,1000);

    watcher->cd(1);
    gPad->SetLogy();
    FPD_ScalerCurr->Draw();

    watcher->cd(2);
    FPD_TimeOR->Draw();

    watcher->cd(3);
    MWPC_Wires_Hits->Draw();

    watcher->cd(4);
    gPad->SetLogz();
    NaI_ClPhi_v_ClTheta->Draw("colz");

    watcher->cd(5);
    gPad->SetLogz();
    NaI_Hits_v_TimeOR->Draw("colz");

    watcher->cd(6);
    gPad->SetLogz();
     TwoD2000v1400->Draw("colz");

    watcher->cd(7);
    gPad->SetLogy();
    BaF2_Hits->Draw();

    watcher->cd(8);
        BaF2_ClPhi_v_ClTheta->Draw("colz");

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",4000);

    update_timer->Start();
}
