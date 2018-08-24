{
    TCanvas* watcher = new TCanvas("watcher","Watch Me");
    watcher->SetWindowSize(600,500);
    watcher->SetWindowPosition(1920,500);
    if(FPD_ScalerCurr) FPD_ScalerCurr->Draw();

    TTimer* update_timer = new TTimer("watcher->Update();watcher->Draw();",4000);

    update_timer->Start();
}
