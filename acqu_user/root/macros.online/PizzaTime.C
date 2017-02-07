void PizzaTime()
{
    TH1* h[24];
    TCanvas* c = new TCanvas();
    c->Divide(6, 4, 0.001, 0.001);

    for (Int_t i = 0; i < 24; i++)
    {
        h[i] = (TH1*) gDirectory->Get(TString::Format("Pizza_Time%d", i));
        c->cd(i+1)->SetLogy();
        h[i]->GetXaxis()->SetRangeUser(-1000, 1000);
        h[i]->Draw();
        h[i]->SetTitle(TString::Format("Element %d", i));
    }
}

