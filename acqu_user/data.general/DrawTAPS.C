void DrawTAPS(TString sFile="BaF2-PbWO4.dat"){
  TTree *t1 = new TTree("t1","t1");
  t1->ReadFile(sFile,"Ch:X:Y:Z");
  TH2D *h1 = new TH2D("h1","Detector",130,-65,65,130,-65,65);
  Float_t fX, fY;
  t1->SetBranchAddress("X",&fX);
  t1->SetBranchAddress("Y",&fY);

  Int_t iNCh = t1->GetEntries();
  for(Int_t iChN=0; iChN<iNCh; iChN++){
    t1->GetEntry(iChN);
    cout << iChN << "\t" << fX << "\t" << fY << endl;
    h1->Fill(fX,fY,iChN+1);
  }
  h1->DrawClone("text");
}
