// This code gets executed by the online analysis
// every Nth events (see online config Period: statement)
int x=5;
void PeriodMacro() {
  // Dump the current FP scalers to a nfs exported file system
  // if(gROOT->FindObject("FPD_ScalerCurr")){
  //   if((FPD_ScalerCurr->Integral()) > 0){
  //     FILE  *fp=fopen("scratch/tagscaler.txt","w");
  //     //FILE  *fp=fopen("/exports/a2raid5/tagscaler.txt","w");
  //     for(int n=1;n<=352;n++){
  // 	fprintf(fp,"%d\n", FPD_ScalerCurr->GetBinContent(n));
  //     }
  //     fclose(fp);
  //   }
  // }

  //replaced writing out the tagger scalers to file, with writing to EPICS.
   // write the tagger scalers to data
  if(gROOT->FindObject("FPD_ScalerCurr")){
    if((FPD_ScalerCurr->Integral())>0) {
      stringstream cmd;
      cmd << "caput -a TAGGER:RAW_SCALER 352";
      for(int n=352; n>=1; n--) {
	cmd << FPD_ScalerCurr->GetBinContent(n) << " ";
      } 
      cmd << " > /dev/null";
      system(cmd.str().c_str());
    }
  }
  
  // calculate the background-subtracted pairspec data
  if(gROOT->FindObject("PairSpec_Open")){
    if((PairSpec_Open->Integral())>0) {
      stringstream cmd;
      cmd << "caput -a BEAM:PAIRSPEC:TaggEff 352";
      for(int n=1; n<=352; n++) {
	Double_t open = PairSpec_Open->GetBinContent(n);
	Double_t gated = PairSpec_Gated->GetBinContent(n);
	Double_t gated_dly = PairSpec_GatedDly->GetBinContent(n);
	Double_t taggeff = (gated-gated_dly)/open;
	if(!TMath::Finite(taggeff))
          taggeff = 0;
	cmd << taggeff << " ";
      }
      cmd << " > /dev/null";
      system(cmd.str().c_str());
    }
  }

  // determine number of events with a hardware error
  if(gROOT->FindObject("NHardwareError")){
    if((gAN->GetNDAQEvent()) < 3000) NHardwareError->Reset();
    stringstream cmd;
    cmd << "caput GEN:MON:EventsWithError.A " << (NHardwareError->Integral(2,-1)) << " > /dev/null";
    system(cmd.str().c_str());
  }

  // look for hole in MWPC
  if(gROOT->FindObject("MWPC_Wires_Hits")){
    if((MWPC_Wires_Hits->Integral()) > (400*528)){
      Int_t iPrev, iThis;
      Double_t dDiff;
      Int_t iProb = 0;
      iThis = MWPC_Wires_Hits->GetBinContent(1);
      for(Int_t i=1; i<528; i++){
	iPrev = iThis;
	iThis = MWPC_Wires_Hits->GetBinContent(i+1);
	dDiff = (TMath::Abs((iThis-iPrev)/(1.*iPrev)));
	if(dDiff > 0.5) iProb++;
      }
      if(iProb > 8){
	//printf("%d problems found in MWPC Wires!!!\n",iProb);
	//system("ssh macrobusy ogg123 -q /usr/share/sounds/extra/achtung.ogg &");
      }
      MWPC_Wires_Hits->Reset();
    }
  }  
}
