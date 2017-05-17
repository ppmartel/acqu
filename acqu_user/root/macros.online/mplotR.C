// D.Watts 24th Jan 2004
// J.R.M. Annand 24th Jan 2004...generalise for single & multi-value

void mplotR(int cstart, int nx = 1, int ny = 1, int multi = -1, int log = 0,
	   Int_t rlow = -1, Int_t rhigh = -1 )
{
// plot multiple spectra (of the same type)
// cstart = start ADC index
// nx = # spectra in X direction, default = 1
// ny = # spectra in Y direction, default = 1
// multi for multi-hit ADC 0=1st hit; 1=2nd hit etc. default -1 = not multi hit
// log for log Y axis, default = linear
//
    char hist_name[16];
    char h[8];
    TH1F* hist;
    TAxis* xaxis;
    int w=1;   
//      
    TCanvas* rawcanv = new TCanvas("PID_QDCs","CB-Test", 20, 20, 1200, 960);
    rawcanv->SetFillStyle(4000);
    rawcanv->Divide(nx, ny, 0.0001, 0.0001);
    //    if( log ){ 
      //    }
    if( multi != -1){
	sprintf(h,"M");
	sprintf(h+1,"%d\0",multi);
    }
    else strcpy(h,"");
//            
    for (int t=cstart; t<(cstart+nx*ny); t++)
    {
	sprintf(hist_name,"ADC%d",t);
	strcat(hist_name,h);
	rawcanv->cd(w);
	if( log ) rawcanv->GetPad(w)->SetLogy();
	hist = (TH1F*)gROOT->FindObject(hist_name);
	if( hist ){
	  if( (rlow != -1) && (rhigh != -1 ) )
	    hist->GetXaxis()->SetRangeUser((Double_t)rlow,(Double_t)rhigh);
	  hist->SetLineColor(1);
	  hist->SetFillColor(2);
	  hist->Draw();
	}
	else printf("Histogram %s not found\n",hist_name);
	w++;
//              xaxis = (TAxis)adcspect->GetXaxis();
//              xaxis->SetRange(0, 4096);
    }
    rawcanv->Update();
    rawcanv->Draw();
    return;
}
//                                           
