//--Author	JRM Annand   13th Jan 2007
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Check plots of Tagger spectra
//
// Ken Temp


void PizzaClear(){
}

CheckPizza(TCanvas* canv){
  
  if(canv==NULL) {
    PizzaClear();
    return;
  }
  

  TH1* h1;
  canv->SetFillStyle(4000);
  canv->Divide(4,4,0.01,0.01);
  
  Char_t* hname[] = {
    "BaF2_LG_001", "BaF2_LG_002", "BaF2_LG_129", "BaF2_LG_130", "BaF2_LG_193", "BaF2_LG_194", "BaF2_LG_257", "BaF2_LG_258", 
    "BaF2_Time_001", "BaF2_Time_002", "BaF2_Time_129", "BaF2_Time_130", "BaF2_Time_193", "BaF2_Time_194", "BaF2_Time_257", "BaF2_Time_258", 
  }
  printf("Doing root histogram %s\n",hname[i]);
  
  for( Int_t i=0; i<16; i++ ){
    printf("Doing root histogram %s\n",hname[i]);
    h1 = (TH1*)(gROOT->FindObject(hname[i]));
    if( !h1 ){
      printf("No root histogram %s\n",hname[i]);
      continue;
    }
    
    canv->cd(i+1);
    if(i<8)h1->SetAxisRange(50,350);
    else h1->SetAxisRange(1500,2500);
    h1->Draw();
    
  }
  return;
}


