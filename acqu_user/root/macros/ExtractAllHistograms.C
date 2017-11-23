TList* result;

void WorkOnPad(TPad* pad) {
	TIter nextobj(pad->GetListOfPrimitives());
	TObject *obj;
	while (obj = nextobj()) {
		//cout << "Pad Object: " << obj->GetName() << endl;
		if(obj->InheritsFrom("TPad")) {
			WorkOnPad((TPad*)obj);
		}
		else if(obj->InheritsFrom("TH1")) {
			WorkOnHist((TH1*)obj);
		}
	}
}

void WorkOnHist(TH1* h) {
	cout << "Found histogram: " << h->GetName() << endl;
	result->Add(h);
}


TList* ExtractAllHistograms(TFile* f) {
	TIter nextkey(f->GetListOfKeys());
	TKey *key;
	result = new TList();
	while (key = (TKey*)nextkey()) {
		cout << "Key: " << key->GetName() << endl;
		TObject* obj = key->ReadObj();
		if(obj->InheritsFrom("TPad")) {
			WorkOnPad((TPad*)obj);
		}
		else if(obj->InheritsFrom("TH1")) {
			WorkOnHist((TH1*)obj);
		}
	}
	return result;
}

   
