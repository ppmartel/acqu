//--Author  D.Krambrich  28th May 2004
//--Update  JRM Annand   13th Jan 2007
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// Check plots of CB spectra
//

TH1F* CBHoles;

void InitHoles(){
	CBHoles = (TH1F*)gROOT->FindObject("CBHoles");
	if( CBHoles ) return;
	UInt_t nholes = 48;
	UInt_t cb_holes[] = {26, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40,
	                     311, 315, 316, 318, 319,
	                     353, 354, 355, 356, 357, 358, 359,
	                     360, 361, 362, 363, 364, 365, 366,
	                     400, 401, 402, 405, 408,
	                     679, 681, 682, 683, 684, 685, 686, 687, 688, 689,
	                     691, 692};
	CBHoles = new TH1F("CBHoles", "Missing Crystals of CB", 720, 0, 720);
	for(UInt_t i = 0; i<nholes; ++i){
		CBHoles->Fill(cb_holes[i], 1E12);
	}
}

void CBClear(){
	// Does not work properly. (Reset()!)
	TA2Detector* t = (TA2Detector*)(gAN->GetGrandChild("NaI"));
	t->ZeroAll();
	//  Scalers364to491->Reset();
}

void CheckCB(TCanvas* canv) {

	if(canv==NULL) {
		CBClear();
		return;
	}

	char *hname[] = {
		"1", "NaI_ADCHits",         "lin", "",
		"1", "CBHoles",             "lin", "Same",
		"2", "NaI_TDCHits",         "lin", "",
		"2 ", "CBHoles",             "lin", "Same",
		"3", "NaI_Hits",            "lin", "",
		"3", "CBHoles",             "lin", "Same",
		"4", "NaI_Nhits",           "lin", "",
		"5", "NaI_Etot",            "log", "",
		//"6", "SumScalers12to139",   "log", "",
		"6", "NaI_TimeOR",          "lin", "",
		"7", "NaI_ClNhits",         "lin", "",
		"8", "NaI_ClNhitsOR",       "lin", "",
		"9", "NaI_ClNhitsOR_v_ClEnergyOR", "log", "CONTZ",
		"10", "NaI_Hits_v_TimeOR",   "lin", "COLZ",
		"11", "NaI_Hits_v_EnergyOR", "log", "COLZ",
		"12", "NaI_ClPhi_v_ClTheta", "log", "COLZ",
	};
	Char_t* xname[] = {
		"CB Distribution of ADC Hits",
		"",
		"CB Distribution of TDC Hits",
		"",
		"CB Distribution of Hits",
		"",
		"CB Number Hits per Event",
		"CB Total Energy in Calorimeter",
		//"CB OR-16 Scalers",
		"CB Time",
		"CB Number of Clusters per Event",
		"CB Number of Hits per Cluster",
		"CB Hits per Cluster vs Cluster Energy",
		"CB Time vs Hits",
		"CB Energy vs Hits",
		"CB #theta - #phi distribution of Cluster Hits",
	};
	Int_t col[] = { 4,2,4,2,4,2,2,3,5,2,2,1,1,1,1 };

	UInt_t i, j;
	Int_t pad_no;
	TH1F* h1;
	TH2F* h2;

	InitHoles();

	
	
	canv->Divide(3,4);
	
	i = 0;
	for( j=0; j<44; j+=4){
		sscanf(hname[j], "%d", &pad_no);           // pad_no is 1st arg.
		canv->cd(pad_no);
		h1 = (TH1F*)gROOT->FindObjectAny(hname[j+1]); // name is 2nd argument
		if(strcmp(hname[j+2], "log") == 0)         // lin/log 3rd argument
			canv->GetPad(pad_no)->SetLogy();
		if(h1){
			h1->Draw(hname[j+3]);               // parameter 4th arg.
			h1->GetXaxis()->SetTitle(xname[i]);
			h1->SetLineColor( 1 );
			h1->SetFillColor( col[i] );
		}
		else printf("No object named '%s' was found.\n Check your macro!\n", hname[j+1]);
		gPad->RedrawAxis();
		i++;
	}
	for( j=44; j<60; j+=4){
		sscanf(hname[j], "%d", &pad_no);           // pad_no is 1st arg.
		canv->cd(pad_no);
		h2 = (TH2F*)gROOT->FindObjectAny(hname[j+1]); // name is 2nd argument
		if(strcmp(hname[j+2], "log") == 0)         // lin/log 3rd argument
			canv->GetPad(pad_no)->SetLogz();
 
		if(h2) {
			h2->Draw(hname[j+3]);               // parameter 4th arg.
			h2->GetXaxis()->SetTitle(xname[i]);
		}
		else printf("No object named '%s' was found.\n Check your macro!\n", hname[j+1]);
		gPad->RedrawAxis();
		i++;
	}
	//  delete CBHoles;

	return;
}
