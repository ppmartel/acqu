// This code gets executed by the online analysis
// every Nth events (see online config Period: statement)
//int x=5;

void PeriodMacroTaggEff() {
  Double_t dError = 0;
  //printf("Period Macro - %d\n",gAN->GetNDAQEvent());

  if(gROOT->FindObject("FPD_ScalerCurr")){
    if((FPD_ScalerCurr->Integral())>0) {
      stringstream cmd;
      cmd << "caput -a TAGGER:RAW_SCALER 352";
      for(int n=352; n>=1; n--) {
	cmd << " " << FPD_ScalerCurr->GetBinContent(n);
      } 
      cmd << " > /dev/null";
      system(cmd.str().c_str());

      if((FPD_ScalerCurr->Integral(1,32))<=32) {
	printf("Tagger Section A appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(33,80))<=48) {
	printf("Tagger Section B appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(81,128))<=48) {
	printf("Tagger Section C appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(129,176))<=48) {
	printf("Tagger Section D appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(177,224))<=48) {
	printf("Tagger Section E appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(225,272))<=48) {
	printf("Tagger Section F appears to be off!\n");
	dError = 500;
      }
      /*
      if((FPD_ScalerCurr->Integral(273,320))<=48) {
	printf("Tagger Section G appears to be off!\n");
	dError = 500;
      }
      if((FPD_ScalerCurr->Integral(321,352))<=32) {
      	printf("Tagger Section H appears to be off!\n");
      	dError = 500;
      }
      */
      if(dError==500) printf("\n");

      /*
      if(gROOT->FindObject("ScInt4")){
	if((ScInt4->Integral())>0) {
	  Int_t maxBin = FPD_ScalerCurr->GetMaximumBin();
	  Double_t maxSca = FPD_ScalerCurr->GetBinContent(maxBin);
	  maxBin = ScInt4->GetEntries();
	  Double_t intrpt = ScInt4->GetBinContent(maxBin-1);
	  Double_t tgRate = (intrpt*maxSca/10000.0);
	  stringstream cmd;
	  cmd << "caput GEN:MON:MaxTaggRate " << tgRate << " > /dev/null";
	  system(cmd.str().c_str());
	}
      }
      */
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
	cmd << " " << taggeff;
      }
      cmd << " > /dev/null";
      system(cmd.str().c_str());
    }
  }

  // look for shift in FPD
  if(gROOT->FindObject("FPD_TimeOR")){
    Int_t iNBins = FPD_TimeOR->GetNbinsX();
    if((FPD_TimeOR->Integral()) > (100*iNBins)){
      TH1D *Temp_FPD = (TH1D*)FPD_TimeOR->Clone("Temp_FPD");
      if(gROOT->FindObject("Prev_FPD")){
	Temp_FPD->Add(Prev_FPD,-1);
	if((Temp_FPD->Integral()) > (100*iNBins)){
	  delete Prev_FPD;
	  TH1D *Prev_FPD = (TH1D*)FPD_TimeOR->Clone("Prev_FPD");
	}
      }
      else TH1D *Prev_FPD = (TH1D*)FPD_TimeOR->Clone("Prev_FPD");
      
      Int_t iPeak = Temp_FPD->GetMaximumBin();
      Double_t dTime = Temp_FPD->GetBinCenter(iPeak);
      //Double_t dTime = Temp_FPD->GetMean();
      if(dTime < -20 || dTime > 0){
	printf("Possible problem in FPD_TimeOR - Event %d\n\t\t\tPeak at %f ns\n\n",gAN->GetNDAQEvent(),dTime);
	dError += 2000;
        Temp_FPD->SaveAs(TString::Format("TaggerProblem_%d.root", gAN->GetNDAQEvent()));
      }	  
      delete Temp_FPD;
    }
  }

  // Check Target DAQ status
  TString sDAQ = gSystem->GetFromPipe("caget TARGET:HE:DAQ_Status");
  if(sDAQ.EndsWith("0")){
    printf("Possible problem with Target DAQ - Event %d\n\n",gAN->GetNDAQEvent());
    dError += 8000;
  }

  // determine number of events with a hardware error
  if((gROOT->FindObject("NHardwareError")) && (dError == 0)){
    if((gAN->GetNDAQEvent()) < 3000) NHardwareError->Reset();
    dError = (NHardwareError->Integral(2,-1));
  }

  // now write the resulting errors to epics
  stringstream cmd;
  cmd << "caput GEN:MON:EventsWithError.A " << dError << " > /dev/null";
  system(cmd.str().c_str());
}
