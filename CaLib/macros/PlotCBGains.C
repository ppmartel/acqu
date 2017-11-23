
//______________________________________________________________________________
void PlotCBGains()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    gSystem->Load("../../a2display/build/liba2display.so");
    
    // configuration
    const Int_t nPar = 720;
    const Char_t data[] = "Data.CB.E1";
    const Char_t calibrationOld[] = "2014-07-EPT_Prod_Neiser_OldCluster";
    const Char_t calibrationNew[] = "2014-07_EPT_Prod_Neiser";

    // data array
    Double_t parOld[nPar];
    Double_t parNew[nPar];

    // read data from set0
    TCMySQLManager::GetManager()->ReadParameters(data, calibrationOld, 0, parOld, nPar);
    TCMySQLManager::GetManager()->ReadParameters(data, calibrationNew, 0, parNew, nPar);

    gStyle->SetOptStat(0);
    
    hOld = new TH1D("CBGainsOld","CB Gains Old",nPar,0,nPar);
    hNew = new TH1D("CBGainsNew","",nPar,0,nPar);
    hNew->SetLineWidth(2.0);
    hOld->SetContent(parOld);
    hNew->SetContent(parNew);
    hNew->Divide(hOld);
    hNew->GetXaxis()->SetTitle("Crystal index");
    hNew->GetYaxis()->SetRangeUser(0.96,1.01);
    
    //hNew->GetYaxis()->SetTitle("New / Old gain");
    new TCanvas();
    hNew->Draw();
    

    new TCanvas();
    cb = new TH2CB("CB"," ");
    //gPad->SetLogz();
    gPad->SetRightMargin(0.13);
    cb->FillElements(*hNew);
    cb->GetZaxis()->SetRangeUser(0.96,1.01);
    cb->Draw("colz");
    //hNew->Draw("same");
}

