void Finish(TString sDir = "")
{

  // Finishing Macro for running with Worker
  printf("Events: %d      Events Accepted: %d\n", 
	gAN->GetNEvent(), gAN->GetNEvAnalysed() );

  printf("\nEnd-of-Run macro executing\n");

  // If user has not specified output directory, pull from AR
  if(sDir.Length() == 0) sDir = gAR->GetTreeDir();
  if(sDir.Length() == 0) sDir = ".";
  
  // Append "/" to Directory if unspecified
  if(!(sDir.EndsWith("/"))) sDir.Append("/");

  // Create output filename from input file name
  TString sFile;
  if (gAR->IsOnline()) sFile = gAR->GetFileName();
  else sFile = gAR->GetTreeFile()->GetName();
  while(sFile.Contains("/")) sFile.Remove(0,1+sFile.Index("/"));
  sFile.ReplaceAll(".dat",".root");
  sFile.Prepend("Hist_");
  sFile.Prepend(sDir);

  // Save histograms to file and close it
  TFile f(sFile, "recreate");
  f.SetCompressionLevel(4);
  gROOT->GetList()->Write();
  f.Close();
  cout << "All histograms saved to " << sFile << endl;

  gSystem->Exit(0);

}

