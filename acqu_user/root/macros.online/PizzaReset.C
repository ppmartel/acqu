void PizzaReset()
{
    for (Int_t i = 0; i < 24; i++)
    {
        TH1* h = (TH1*) gDirectory->Get(TString::Format("Pizza_Energy%d", i));
        h->Reset();
        h = (TH1*) gDirectory->Get(TString::Format("Pizza_Time%d", i));
        h->Reset();
    }
}

