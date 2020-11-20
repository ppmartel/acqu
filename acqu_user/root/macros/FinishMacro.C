void FinishMacro(TString file = "ARHistograms.root")
{
  // Stuff to do at the end of an analysis run
  // Here all spectra are saved to disk
  printf("Events: %d      Events Accepted: %d\n",
	 gAN->GetNEvent(), gAN->GetNEvAnalysed() );
  printf("End-of-Run macro executing\n");
  TFile f(file,"recreate");
  if( !f.IsOpen() ){
    printf("Open file %s for histogram save FAILED!!\n",file.Data());
    return;
  }
  gROOT->GetList()->Write();
  f.Close();
  printf("All histograms saved to %s\n\n",file.Data());

  if (!(gAR->IsOnline())) gSystem->Exit(0);
}
