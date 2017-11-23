void FinishBatch(TString sInput="", TString sOutput="ARHist"){

  // Stuff to do at the end of an analysis run
  // Here all spectra are saved to disk
  printf("Events: %d      Events Accepted: %d\n",
	 gAN->GetNEvent(), gAN->GetNEvAnalysed() );
  printf("End-of-Run macro executing\n");

  TString sDir = gAR->GetTreeDir();
  if(!sDir.EndsWith("/")) sDir.Append("/");
  TString sFile;
  if (gAR->IsOnline()) sFile = gAR->GetFileName();
  else sFile = gAR->GetTreeFile()->GetName();
  while(sFile.Contains("/")) sFile.Remove(0,1+sFile.Index("/"));
  sFile.ReplaceAll(".dat",".root");
  if(sInput.Length() && sFile.BeginsWith(sInput)) sFile.Remove(0,sInput.Length());
  else sFile.Prepend("_");
  sFile.Prepend(sOutput);
  sFile.Prepend(sDir);
  
  // Save histograms to file and close it
  TFile f(sFile, "recreate");
  if( !f ){
    printf("Open file %s for histogram save FAILED!!\n",sFile.Data());
    return;
  }
  gROOT->GetList()->Write();
  f.Close();
  printf("All histograms saved to %s\n",sFile.Data());

  if (!(gAR->IsOnline())) gSystem->Exit(0);

}
