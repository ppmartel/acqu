// This code gets executed by the online analysis
// every Nth events (see online config Period: statement)
//int x=5;

void PeriodMacro() {
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

  // look for hole in MWPC
  
  // if(gROOT->FindObject("MWPC_Wires_Hits")){
  //   Int_t iNBins = MWPC_Wires_Hits->GetNbinsX();
  //   if((MWPC_Wires_Hits->Integral()) > (100*iNBins)){
  //     Int_t iPrev, iThis;
  //     Double_t dDiff;
  //     Int_t iProb = 0;
  //     iThis = MWPC_Wires_Hits->GetBinContent(1);
  //     for(Int_t i=1; i<iNBins; i++){
  // 	iPrev = iThis;
  //       //TODO: exclude broken/noisy channels (as of 20.01.2017)
  //       if (i == 140 || i == 142 || i == 159 ||
  // 	    i == 192 || i == 232 || i == 233 ||
  // 	    i == 327 || i == 423 || i == 526 || ((i>=168)&&(i<=174))||
  // 	    i == 527)
  //         continue;
  // 	iThis = MWPC_Wires_Hits->GetBinContent(i+1);
  // 	dDiff = (TMath::Abs((iThis-iPrev)/(1.*iPrev)));
  // 	if(dDiff > 0.5) iProb++;
  //     }
  //     if(iProb > 8){
  // 	printf("Possible problem in MWPC Wires - Event %d\n\t\t\t%d jumps found\n\n",gAN->GetNDAQEvent(),iProb);
  // 	dError += 1000;
  //     }
  //   }
  //   if((MWPC_Wires_Hits->Integral()) > (400*iNBins)) MWPC_Wires_Hits->Reset();
  // }
  


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
      if(dTime < -10 || dTime > 10){
	printf("Possible problem in FPD_TimeOR - Event %d\n\t\t\tPeak at %f ns\n\n",gAN->GetNDAQEvent(),dTime);
	dError += 2000;
      }	  
      delete Temp_FPD;
    }
  }

  // look for shift in NaI
  Bool_t bShift = false;
  if(gROOT->FindObject("NaI_Hits_v_TimeOR")){
    Int_t iNBins = ((NaI_Hits_v_TimeOR->GetNbinsX())*(NaI_Hits_v_TimeOR->GetNbinsX()));
    if((NaI_Hits_v_TimeOR->Integral()) > (10000*720/8)){	
      TH1D *Proj_NaI;
      for(Int_t i=0; i<720; i+=8){
	if((i==32) || (i==352) || (i==360) || (i==680)) continue;
	Proj_NaI = (TH1D*)NaI_Hits_v_TimeOR->ProjectionX("Proj_NaI",i+1,i+8);
	
	Double_t dTime = Proj_NaI->GetMean();
	if((dTime < 20) || (dTime > 52)){ //TODO: changed upper limit to from 50 to 52 on 06.11.2016 (mean around 50.x for several channels)
	  if(!bShift) printf("Possible problem in NaI_Hits_v_TimeOR - Event %d\n",gAN->GetNDAQEvent());
	  printf("\t\t\tPeak at %f ns (Channels %3d-%3d)\n",dTime,i,i+7);
	  if(!bShift) dError += 4000;
	  bShift = true;
	}
      }
      if(bShift) printf("\n");
      else NaI_Hits_v_TimeOR->Reset();
    }
  }

  // Check Target DAQ status
  TString sDAQ = gSystem->GetFromPipe("caget TARGET:HE:DAQ_Status");
  if(sDAQ.EndsWith("0")){
    printf("Possible problem with Target DAQ - Event %d\n\n",gAN->GetNDAQEvent());
    dError += 8000;
  }

  // Check Archiver
  TString sArchive = gSystem->GetFromPipe("echo 'http://slowcontrol:4812/main' | wget -O- -i- -q | grep -A 1 '>State<' | grep -v State | sed 's/ /_/g' | sed 's/>/ /g' | sed 's/</ /g' | awk '{print $3}'");
  if(!sArchive.Contains("RUNNING")){
    printf("Possible problem with EPICS Archiver - Event %d\n\n",gAN->GetNDAQEvent());
    dError += 16000;
  }

  // determine number of events with a hardware error
  if((gROOT->FindObject("NHardwareError")) && (dError == 0)){
    if(gROOT->FindObject("Prev_Error")){
      Double_t dRatio = ((NHardwareError->Integral(2,-1))-(Prev_Error->Integral(2,-1)))/((NHardwareError->GetEntries())-(Prev_Error->GetEntries()));
      //printf("Total Errors = %d\tPrev Errors = %d\tTotal Events = %d\tPrev Events = %d\tRatio = %.3f\n",(NHardwareError->Integral(2,-1)),(Prev_Error->Integral(2,-1)),(NHardwareError->GetEntries()),(Prev_Error->GetEntries()),dRatio);
      if(dRatio > 0.00){ // Report when any of the events have an error
      //if(dRatio > 0.1){ // Report when more than 10% of the events have an error
	dError = (NHardwareError->Integral(2,-1));
      }

      delete Prev_Error;
      TH1D *Prev_Error = (TH1D*)NHardwareError->Clone("Prev_Error");
    }
    else{
      NHardwareError->Reset();
      TH1D *Prev_Error = (TH1D*)NHardwareError->Clone("Prev_Error");
    }
  }

  // now write the resulting errors to epics
  stringstream cmd;
  cmd << "caput GEN:MON:EventsWithError.A " << dError << " > /dev/null";
  system(cmd.str().c_str());

  // Perform some online asymmetry
  if(gROOT->FindObject("PHYS_Pi0P_Hel0") && gROOT->FindObject("PHYS_Pi0P_Hel1")){
    TH1D *hel0P = (TH1D*)PHYS_Pi0P_Hel0->ProjectionX("hel0P",96,105,50,89);
    TH1D *hel0L = (TH1D*)PHYS_Pi0P_Hel0->ProjectionX("hel0L",1,95,50,89);
    TH1D *hel0R = (TH1D*)PHYS_Pi0P_Hel0->ProjectionX("hel0R",106,200,50,89);

    TH1D *hel1P = (TH1D*)PHYS_Pi0P_Hel1->ProjectionX("hel1P",96,105,50,89);
    TH1D *hel1L = (TH1D*)PHYS_Pi0P_Hel1->ProjectionX("hel1L",1,95,50,89);
    TH1D *hel1R = (TH1D*)PHYS_Pi0P_Hel1->ProjectionX("hel1R",106,200,50,89);

    TH1D *hel0S = (TH1D*)hel0P->Clone("hel0S");
    hel0S->Sumw2();
    hel0S->Add(hel0L,(-5/95.));
    hel0S->Add(hel0R,(-5/95.));

    TH1D *hel1S = (TH1D*)hel1P->Clone("hel1S");
    hel1S->Sumw2();
    hel1S->Add(hel1L,(-5/95.));
    hel1S->Add(hel1R,(-5/95.));

    TH1D *helNum = (TH1D*)hel0S->Clone("helNum");
    helNum->Add(hel1S,-1);
    helNum->Scale(TMath::Sqrt(2));
    TH1D *helDen = (TH1D*)hel0S->Clone("helDen");
    helDen->Add(hel1S,1);

    helNum->Rebin(4);
    helDen->Rebin(4);

    TH1D *helAsy = (TH1D*)helNum->Clone("helAsy");
    helAsy->Divide(helDen);
    //printf("g+p->pi0+p - F = %0.3f +/- %0.3f\n",helAsy->GetBinContent(7),helAsy->GetBinError(7));

    stringstream cmd;
    cmd << "caput GEN:MON:Pi0Asymmetry.A " << helAsy->GetBinContent(7) << " > /dev/null";
    system(cmd.str().c_str());
  }

  // read out pi0 counts from 
  if(gROOT->FindObject("PHYS_IM_2g")){
    Double_t dNPi0 = 0;
    if(gROOT->FindObject("Prev_IM")){
      TH1D *Temp_IM = (TH1D*)PHYS_IM_2g->Clone("Temp_IM");
      Temp_IM->Add(Prev_IM,-1);
      if((Temp_IM->Integral()) > 100){
	dNPi0 = Temp_IM->Integral();
	  
	delete Temp_IM;
	delete Prev_IM;
	  
	TH1D *Prev_IM = (TH1D*)PHYS_IM_2g->Clone("Prev_IM");
      }
    }
    else{
      dNPi0 =PHYS_IM_2g->Integral();
      TH1D *Prev_IM = (TH1D*)PHYS_IM_2g->Clone("Prev_IM");
    }

    stringstream cmd;
    cmd << "caput GEN:MON:Pi0PerScRead.A " << dNPi0 << " > /dev/null";
    system(cmd.str().c_str());
  }
  /*/
  // fill array for CB display
  if(gROOT->FindObject("PHYS_CB_Display_R")){
    Int_t minB = 0;
    Int_t maxB = 0;
    Double_t maxE = 0;
    maxB = (PHYS_CB_Display_R->GetMaximumBin());
    maxE = TMath::Max(maxE,PHYS_CB_Display_R->GetBinContent(maxB));
    maxB = (PHYS_CB_Display_G->GetMaximumBin());
    maxE = TMath::Max(maxE,PHYS_CB_Display_G->GetBinContent(maxB));
    maxB = (PHYS_CB_Display_B->GetMaximumBin());
    maxE = TMath::Max(maxE,PHYS_CB_Display_B->GetBinContent(maxB));

    minB = PHYS_CB_Display_T->GetMinimumBin();
    maxB = PHYS_CB_Display_T->GetMaximumBin();

    Double_t minT = PHYS_CB_Display_T->GetBinContent(minB);
    Double_t maxT = PHYS_CB_Display_T->GetBinContent(maxB);

    stringstream cmdR, cmdG, cmdB, cmdT;
    cmdR << "caput -a CB:CB:NaI_Hits:R 720";
    cmdG << "caput -a CB:CB:NaI_Hits:G 720";
    cmdB << "caput -a CB:CB:NaI_Hits:B 720";
    cmdT << "caput -a CB:CB:NaI_Hits:T 720";

    for(int n=1; n<=720; n++) {
      if((PHYS_CB_Display_R->GetBinContent(n)) == 0) cmdR << " 0";
      else cmdR << " " << TMath::Nint(255*TMath::Log10(PHYS_CB_Display_R->GetBinContent(n))/TMath::Log10(maxE));
      if((PHYS_CB_Display_G->GetBinContent(n)) == 0) cmdG << " 0";
      else cmdG << " " << TMath::Nint(255*TMath::Log10(PHYS_CB_Display_G->GetBinContent(n))/TMath::Log10(maxE));
      if((PHYS_CB_Display_B->GetBinContent(n)) == 0) cmdB << " 0";
      else cmdB << " " << TMath::Nint(255*TMath::Log10(PHYS_CB_Display_B->GetBinContent(n))/TMath::Log10(maxE));
      if((PHYS_CB_Display_T->GetBinContent(n)) == 0) cmdT << " 0";
      else cmdT << " " << TMath::Nint(50*((PHYS_CB_Display_T->GetBinContent(n))-minT)/(maxT-minT));
    }
    
    cmdR << " > /dev/null";
    cmdG << " > /dev/null";
    cmdB << " > /dev/null";
    cmdT << " > /dev/null";
    
    system(cmdR.str().c_str());
    system(cmdG.str().c_str());
    system(cmdB.str().c_str());
    system(cmdT.str().c_str());
  }
  /*/
  // fill array for CB display
  if(gROOT->FindObject("PHYS_CB_Display_R")){
    stringstream cmdR, cmdG, cmdB, cmdT;
      cmdR << "caput -a CB:CB:NaI_Hits:R 720";
      cmdG << "caput -a CB:CB:NaI_Hits:G 720";
      cmdB << "caput -a CB:CB:NaI_Hits:B 720";
      cmdT << "caput -a CB:CB:NaI_Hits:T 720";
      for(int n=1; n<=720; n++) {
	cmdR << " " << PHYS_CB_Display_R->GetBinContent(n);
	cmdG << " " << PHYS_CB_Display_G->GetBinContent(n);
	cmdB << " " << PHYS_CB_Display_B->GetBinContent(n);
	cmdT << " " << PHYS_CB_Display_T->GetBinContent(n);
      }
      cmdR << " > /dev/null";
      cmdG << " > /dev/null";
      cmdB << " > /dev/null";
      cmdT << " > /dev/null";
      system(cmdR.str().c_str());
      system(cmdG.str().c_str());
      system(cmdB.str().c_str());
      system(cmdT.str().c_str());
  }
}
