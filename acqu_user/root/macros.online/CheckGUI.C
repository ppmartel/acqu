#include <TGFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TGClient.h>
#include <TImage.h>
#include <iostream>
#include <sstream>
#include <iomanip>

class MacroEntry : public TObject {
public:
	string name;
	string desc;
	static MacroEntry* Make(const string& _name, const string& _desc) {
		MacroEntry* h = new MacroEntry();
		h->name = _name;
		h->desc = _desc;
		return h;
	}
	ClassDef(MacroEntry,0);
};

TGMainFrame *fMainFrame;
TGComboBox *fCbxItems;
TCanvas *fCanvas;
TObjArray *items;

void CallMacro(Int_t i, const string& arg) {
	MacroEntry* item = (MacroEntry*)items->At(i);
	fCanvas->Clear();
	stringstream cmd;
	cmd << ".x " << item->name << "(" << arg << ")";
	gROOT->ProcessLine(cmd.str().c_str());
	fCanvas->Update();
}

void OnCbxItemsSelected(Int_t i) {
	if(i<0 || i>=items->GetEntriesFast())
		return;
	// execute the macro with fCanvas as argument
	CallMacro(i, "fCanvas");
}

void SelectItem(Int_t step) {
	Int_t i = fCbxItems->GetSelected();
	Int_t n = fCbxItems->GetNumberOfEntries();
	if(i==-1 && n>0) {
		i=0;
	}
	i += step;
	if(i<0)
		i=n-1;
	if(i>=n)
		i=0;
	fCbxItems->Select(i);
}

void OnBtnPrevClicked() {
	SelectItem(-1);
}

void OnBtnNextClicked() {
	SelectItem(+1);
}

void OnBtnClearClicked() {
	CallMacro(fCbxItems->GetSelected(), "NULL");
	CallMacro(fCbxItems->GetSelected(), "fCanvas");
}

void OnBtnClearAllClicked() {
	gROOT->ProcessLine("gAN->ZeroAll()");
	CallMacro(fCbxItems->GetSelected(), "fCanvas");
}

void OnBtnUpdateClicked() {
	CallMacro(fCbxItems->GetSelected(), "fCanvas");
}

void OnBtnSaveAllClicked() {
	// clean up directory...
	printf("\nDelete all PNG images on daq-master in /home/a2cb/OnlineSpectra/\n");
	stringstream CmdStr;
	CmdStr << "rm /home/a2cb/acqu/OnlineSpectra/*.png";
	system(CmdStr.str().c_str());

	// prepare elog command, note the blank space at every line!
	stringstream elog_cmd;
	Long_t runNum = gROOT->ProcessLine("gAR->GetRunNumber()");
	elog_cmd << "echo Run " << runNum << " Online Spectra | ";
	elog_cmd << "elog -h elog.office.a2.kph -u a2online a2messung ";
	elog_cmd << "-l 'Main Logbook 2016' -a Experiment='APPT Test 2016 II' ";
	elog_cmd << "-a Author='PLEASE FILL IN' -a Type=Routine ";
	elog_cmd << "-a Subject='Online Spectra Run " << runNum << "' ";

	
	Int_t i_save = fCbxItems->GetSelected();
        // update CB spectra before saving if previously selected
        if (i_save == 0)
            CallMacro(i_save, "fCanvas");
	for(Int_t i=0;i<items->GetEntriesFast(); i++) {
		MacroEntry* item = (MacroEntry*)items->At(i);
		fCbxItems->Select(i);
		stringstream filename;
		filename << "/home/a2cb/acqu/OnlineSpectra/"
		         << setw(2) << setfill('0') << i
		         << "-" << item->desc << "-Spectra.png";

		TImage *img = TImage::Create();
		img->FromPad((TVirtualPad*)fCanvas);
		img->WriteImage(filename.str().c_str());
		delete img;

		elog_cmd << "-f " << filename.str() << " ";
	}
	
	printf("\nPosting all PNG images to the Elog...Please Wait...\n");
	system(elog_cmd.str().c_str());
	printf("\nPLEASE EDIT THE ELOG ENTRY TO MAKE IT COMPLETE!\n\n");
	
	// restore previously selected item
	fCbxItems->Select(i_save);
}

void CheckGUI()
{
	static bool loaded = false;
	if(loaded) {
		cout << "CheckSystem already loaded..." << endl;
		return;
	}
	// init the list of macros to be called
	items = new TObjArray();
	items->Add(MacroEntry::Make("CheckCB.C","Crystal_Ball"));
	items->Add(MacroEntry::Make("CheckTAPSBaF2.C","TAPS_BaF2"));
	items->Add(MacroEntry::Make("CheckTAPSPbWO4.C","TAPS_PbWO4"));
	items->Add(MacroEntry::Make("CheckTAPSVeto.C","TAPS_Veto"));
	items->Add(MacroEntry::Make("CheckTagger.C","Tagger"));
	items->Add(MacroEntry::Make("CheckPID.C","PID"));
	items->Add(MacroEntry::Make("CheckMWPC.C","MWPC"));
	items->Add(MacroEntry::Make("CheckTrigger.C","Trigger"));
	items->Add(MacroEntry::Make("CheckLiveTimes.C","LiveTimes"));
	items->Add(MacroEntry::Make("CheckPhysics.C","Physics"));	
	items->Add(MacroEntry::Make("CheckActiveADC.C","Active_ADC"));	
	items->Add(MacroEntry::Make("CheckActiveTDC.C","Active_TDC"));	
	
	// main frame
	fMainFrame = new TGMainFrame(gClient->GetRoot(), 1240, 890, kMainFrame | kVerticalFrame);
	fMainFrame->SetName("fMainFrame");
	fMainFrame->SetWindowName("CheckSystem.C");
	fMainFrame->SetCleanup(kDeepCleanup);	
	//fMainFrame->Connect("CloseWindow()", 0, 0, "OnMainFrameClose()");
	fMainFrame->DontCallClose();
	
	// vertical frame
	TGVerticalFrame *fVerticalFrame1093 = new TGVerticalFrame(fMainFrame,874,496,kVerticalFrame);
	fVerticalFrame1093->SetName("fVerticalFrame1093");

	// horizontal frame
	TGHorizontalFrame *fHorizontalFrame1104 = new TGHorizontalFrame(fVerticalFrame1093,870,26,kHorizontalFrame);
	fHorizontalFrame1104->SetName("fHorizontalFrame1104");
	TGTextButton *fBtnPrev = new TGTextButton(fHorizontalFrame1104,"Prev");
	fBtnPrev->SetTextJustify(36);
	fBtnPrev->SetMargins(0,0,0,0);
	fBtnPrev->SetWrapLength(-1);
	fBtnPrev->Resize(34,22);
	fHorizontalFrame1104->AddFrame(fBtnPrev, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));

	ULong_t ucolor;        // will reflect user color changes
	gClient->GetColorByName("#ffffff",ucolor);

	// combo box
	fCbxItems = new TGComboBox(fHorizontalFrame1104,-1,kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
	fCbxItems->SetName("fCbxItems");

	fCbxItems->Resize(122,22);

	fHorizontalFrame1104->AddFrame(fCbxItems, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	TGTextButton *fBtnNext = new TGTextButton(fHorizontalFrame1104,"Next");
	fBtnNext->SetTextJustify(36);
	fBtnNext->SetMargins(0,0,0,0);
	fBtnNext->SetWrapLength(-1);
	fBtnNext->Resize(33,22);
	fHorizontalFrame1104->AddFrame(fBtnNext, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	TGLabel *fLblSpacer1 = new TGLabel(fHorizontalFrame1104,"      ");
	fLblSpacer1->SetTextJustify(36);
	fLblSpacer1->SetMargins(0,0,0,0);
	fLblSpacer1->SetWrapLength(-1);
	fHorizontalFrame1104->AddFrame(fLblSpacer1, new TGLayoutHints(kLHintsLeft | kLHintsExpandY,2,2,2,2));
	TGTextButton *fBtnClear = new TGTextButton(fHorizontalFrame1104,"Clear");
	fBtnClear->SetTextJustify(36);
	fBtnClear->SetMargins(0,0,0,0);
	fBtnClear->SetWrapLength(-1);
	fBtnClear->Resize(38,22);
	fHorizontalFrame1104->AddFrame(fBtnClear, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	TGTextButton *fBtnClearAll = new TGTextButton(fHorizontalFrame1104,"Clear ALL");
	fBtnClearAll->SetTextJustify(36);
	fBtnClearAll->SetMargins(0,0,0,0);
	fBtnClearAll->SetWrapLength(-1);
	fBtnClearAll->Resize(65,22);
	fHorizontalFrame1104->AddFrame(fBtnClearAll, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	TGLabel *fLblSpacer2 = new TGLabel(fHorizontalFrame1104,"      ");
	fLblSpacer2->SetTextJustify(36);
	fLblSpacer2->SetMargins(0,0,0,0);
	fLblSpacer2->SetWrapLength(-1);
	fHorizontalFrame1104->AddFrame(fLblSpacer2, new TGLayoutHints(kLHintsLeft | kLHintsExpandY,2,2,2,2));
	TGTextButton *fBtnUpdate = new TGTextButton(fHorizontalFrame1104,"Update");
	fBtnUpdate->SetTextJustify(36);
	fBtnUpdate->SetMargins(0,0,0,0);
	fBtnUpdate->SetWrapLength(-1);
	fBtnUpdate->Resize(52,22);
	fHorizontalFrame1104->AddFrame(fBtnUpdate, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	TGLabel *fLblSpacer3 = new TGLabel(fHorizontalFrame1104,"      ");
	fLblSpacer3->SetTextJustify(36);
	fLblSpacer3->SetMargins(0,0,0,0);
	fLblSpacer3->SetWrapLength(-1);
	fHorizontalFrame1104->AddFrame(fLblSpacer3, new TGLayoutHints(kLHintsLeft | kLHintsExpandY,2,2,2,2));
	TGTextButton *fBtnSaveAll = new TGTextButton(fHorizontalFrame1104,"Save to Elog");
	fBtnSaveAll->SetTextJustify(36);
	fBtnSaveAll->SetMargins(0,0,0,0);
	fBtnSaveAll->SetWrapLength(-1);
	fBtnSaveAll->Resize(97,22);
	fHorizontalFrame1104->AddFrame(fBtnSaveAll, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));

	fVerticalFrame1093->AddFrame(fHorizontalFrame1104, new TGLayoutHints(kLHintsTop | kLHintsExpandX,2,2,2,2));

	// embedded canvas
	TRootEmbeddedCanvas *fEmbeddedCanvas = new TRootEmbeddedCanvas(0,fVerticalFrame1093,870,462);
	fEmbeddedCanvas->SetName("fEmbeddedCanvas");
	fVerticalFrame1093->AddFrame(fEmbeddedCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,2,2,2,2));

	fMainFrame->AddFrame(fVerticalFrame1093, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,1,1,1,1));

	fMainFrame->SetMWMHints(kMWMDecorAll,
	                        kMWMFuncAll,
	                        kMWMInputModeless);
	fMainFrame->MapSubwindows();

	fMainFrame->Resize(fMainFrame->GetDefaultSize());
	fMainFrame->MapWindow();
	fMainFrame->Resize(1240,1000);

	// Connect signals
	fBtnPrev->Connect("Clicked()", 0, 0, "OnBtnPrevClicked()");
	fCbxItems->Connect("Selected(Int_t)", 0, 0, "OnCbxItemsSelected(Int_t)");
	fBtnNext->Connect("Clicked()", 0, 0, "OnBtnNextClicked()");
	fBtnClear->Connect("Clicked()", 0, 0, "OnBtnClearClicked()");
	fBtnClearAll->Connect("Clicked()", 0, 0, "OnBtnClearAllClicked()");
        fBtnUpdate->Connect("Clicked()", 0, 0, "OnBtnUpdateClicked()");
	fBtnSaveAll->Connect("Clicked()", 0, 0, "OnBtnSaveAllClicked()");
	
	// Set the canvas pointer
	fCanvas = fEmbeddedCanvas->GetCanvas();

	// init combobox
	for(Int_t i=0;i<items->GetEntriesFast(); i++) {
		MacroEntry* item = (MacroEntry*)items->At(i);
		fCbxItems->AddEntry(item->desc.c_str(), i);
	}
	fCbxItems->Select(0);
	
	loaded = true;
}
