void PizzaEnergy()
{
    TH1* h[24];
    TCanvas* c = new TCanvas();
    c->Divide(6, 4, 0.001, 0.001);

    for (Int_t i = 0; i < 24; i++)
    {
        h[i] = (TH1*) gDirectory->Get(TString::Format("Pizza_Energy%d", i));
        c->cd(i+1)->SetLogy();
        h[i]->GetXaxis()->SetRangeUser(0, 2000);
        h[i]->Draw();
        h[i]->SetTitle(TString::Format("Element %d", i));
        //TLine* aLine = new TLine(100, 0, 100, h[i]->GetMaximum());
        //aLine->SetLineWidth(2);
        //aLine->Draw();
    }
}

