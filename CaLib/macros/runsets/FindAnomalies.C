// This macro finds double peak structures,
// per channel, per run, but histogramming
// at time shifts of some other problems


// specify the directory with CaLib histogram root files
const char fFileDir[] = "analysis/CaLib_Full_Step134/root/";
const char fHistName[] = "CaLib_CB_Time";
TH2D* result;

Int_t ExtractRunNumber(TString& fname) {
	TRegexp re("[0-9]+");
	TString num = fname(re);
	return num.Atoi();
}

void WorkOnFile(const char filename[]) {
	Int_t run = ExtractRunNumber(filename);
	TString f(fFileDir);
	f.Append("/");
	f.Append(filename);

	TFile* file = new TFile(f);

	// check file
	if (!file) return;
	if (file->IsZombie()) return;


	TH2* hist = (TH2*) gFile->Get(fHistName);
	if (!hist) continue;
	if (!hist->GetEntries()) continue;

	cout << "Opened " << filename << endl;

	//hist->Draw("colz");

	hist->RebinX(10);
	
	for(Int_t i=1;i<=hist->GetNbinsY();i++) {
		//cout << i << endl;
		TH1D* proj = hist->ProjectionX("_px", i, i);
		//proj->Draw();
		TSpectrum *s = new TSpectrum(100);
		Int_t nfound = s->Search(proj, 6, "nodraw", 0.01);
		//printf("Found %d candidate peaks\n",nfound);
		TList *functions = proj->GetListOfFunctions();
		TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
		Float_t *xpeaks = s->GetPositionX();
		for (Int_t j = 0; j < nfound; j++) {
			//cout << xpeaks[j] << endl;
			result->Fill(xpeaks[j], run);
		}
	}
	
	file->Close();
}

void FindAnomalies() {
	
	TSystemDirectory dir(fFileDir,fFileDir);
	TList *files = dir.GetListOfFiles();

	
	if (!files)
		return;
	
	Int_t nRootFiles=0;
	TSystemFile *file;
	TString fname;
	TIter next(files);
	Int_t run_min = -1;
	Int_t run_max = 0;
	while ((file=(TSystemFile*)next())) {
		fname = file->GetName();
		if (!file->IsDirectory() && fname.EndsWith(".root")) {
			nRootFiles++;
			Int_t run = ExtractRunNumber(fname);
			run_min = (run_min<0 || run_min>run) ? run : run_min;
			run_max = run_max<run ? run : run_max;
		}
	}

	next.Reset();
	

	TString name(fHistName);
	name.Append("_Result");
	result = new TH2D(name, name, 1000, -500, 500,
	                  run_max-run_min+1, run_min, run_max);

	Int_t n = 0;
	while ((file=(TSystemFile*)next())) {
		fname = file->GetName();
		if (!file->IsDirectory() && fname.EndsWith(".root")) {
			n++;
			WorkOnFile(fname);
			cout << 100.0*n/nRootFiles << endl;
			//if(n>10)
			//	break;
		}
	}

	

	result->Draw("colz");
}

