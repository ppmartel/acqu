// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CalibrateGUI.C                                                       //
//                                                                      //
// GUI calibrations using CaLib.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


class ButtonWindow;

// global variables
TList* gCalibrations;
TList* gCaLibModules;
void* gCurrentModule;
Bool_t gCalibSelected;
ButtonWindow* gMainWindow;


class ButtonWindow : public TGMainFrame 
{

private:
    TGTextButton* fTB_Init;
    TGTextButton* fTB_Prev;
    TGTextButton* fTB_Next;
    TGTextButton* fTB_Ignore;
    TGTextButton* fTB_Print;
    TGTextButton* fTB_PrintChanges;
    TGTextButton* fTB_Goto;
    TGTextButton* fTB_DoAll;
    TGTextButton* fTB_Quit;
    TGComboBox* fCBox_Calibration;
    TGComboBox* fCBox_Module;
    TGListBox* fLB_RunSet;
    TGNumberEntry* fNE_Delay;
    TGNumberEntry* fNE_Elem;
    TGRadioButton* fRadio_All;
    TGRadioButton* fRadio_Even;
    TGRadioButton* fRadio_Odd;

public:
    ButtonWindow();

    void ResizeFrame(TGFrame* f);
    void Goto();
    void DoNext();
    void DoIgnore();
    void DoPrev();
    void DoAll();
    void DoModulSelection(Int_t);
};


//______________________________________________________________________________
ButtonWindow::ButtonWindow() 
    : TGMainFrame(gClient->GetRoot(), 600, 500)
{
    // Main test window.
    SetWindowName("CaLib Control Panel");
    
    // ---------------------------------------------------------------------------------
    TGGroupFrame* config_frame = new TGGroupFrame(this, "Calibration and set configuration", kHorizontalFrame);
    config_frame->SetTitlePos(TGGroupFrame::kLeft);
    
    TGVerticalFrame* ver_frame_1 = new TGVerticalFrame(config_frame);
    
    // calibration selection
    fCBox_Calibration = new TGComboBox(ver_frame_1, "Choose calibration");
    fCBox_Calibration->Resize(260, 25);
    ver_frame_1->AddFrame(fCBox_Calibration, new TGLayoutHints(kLHintsLeft, 0, 5, 10, 0));
      
    // fill calibrations
    gCalibrations = TCMySQLManager::GetManager()->GetAllCalibrations();
    for (Int_t i = 0; i < gCalibrations->GetSize(); i++)
    {
        TObjString* s = (TObjString*) gCalibrations->At(i);
        fCBox_Calibration->AddEntry(s->GetString().Data(), i);
    }
    
    fCBox_Calibration->Connect("Selected(Int_t)", "ButtonWindow", this, "EnableModuleSelection(Int_t)");

    // calibration module selection
    fCBox_Module = new TGComboBox(ver_frame_1, "Choose calibration module");
    fCBox_Module->Resize(260, 25);
    ver_frame_1->AddFrame(fCBox_Module, new TGLayoutHints(kLHintsLeft, 0, 5, 10, 0));
  
    // fill modules
    for (Int_t i = 0; i < gCaLibModules->GetSize(); i++)
    {
        TCCalib* cmod = (TCCalib*) gCaLibModules->At(i);
        fCBox_Module->AddEntry(cmod->GetTitle(), i);
    }

    fCBox_Module->Connect("Selected(Int_t)", "ButtonWindow", this, "ReadRunsets(Int_t)");
    
    config_frame->AddFrame(ver_frame_1, new TGLayoutHints(kLHintsFillX));
    
    TGVerticalFrame* ver_frame_2 = new TGVerticalFrame(config_frame);
    
    // runset selection
    fLB_RunSet = new TGListBox(ver_frame_2);
    fLB_RunSet->SetMultipleSelections(kTRUE);
    fLB_RunSet->Resize(120, 60);
    ver_frame_2->AddFrame(fLB_RunSet, new TGLayoutHints(kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 5, 0, 10, 0));
    
    config_frame->AddFrame(ver_frame_2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddFrame(config_frame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

    // ---------------------------------------------------------------------------------
    
    // control buttons
    TGGroupFrame* control_frame = new TGGroupFrame(this, "Calibration control", kHorizontalFrame);
    control_frame->SetTitlePos(TGGroupFrame::kLeft);
    
    fTB_Init = new TGTextButton(control_frame, "Start module");
    ResizeFrame(fTB_Init);
    fTB_Init->Connect("Clicked()", "ButtonWindow", this, "StartModule()");
    control_frame->AddFrame(fTB_Init, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    fTB_Write = new TGTextButton(control_frame, "Write to DB");
    ResizeFrame(fTB_Write);
    fTB_Write->Connect("Clicked()", "ButtonWindow", this, "DoWrite()");
    control_frame->AddFrame(fTB_Write, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    fTB_Print = new TGTextButton(control_frame, "Print values");
    ResizeFrame(fTB_Print);
    fTB_Print->Connect("Clicked()", "ButtonWindow", this, "Print()");
    control_frame->AddFrame(fTB_Print, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    fTB_PrintChanges = new TGTextButton(control_frame, "Print changes");
    ResizeFrame(fTB_PrintChanges);
    fTB_PrintChanges->Connect("Clicked()", "ButtonWindow", this, "PrintChanges()");
    control_frame->AddFrame(fTB_PrintChanges, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    fTB_Quit = new TGTextButton(control_frame, "Quit");
    ResizeFrame(fTB_Quit);
    fTB_Quit->Connect("Clicked()", "ButtonWindow", this, "Quit()");
    control_frame->AddFrame(fTB_Quit, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
    
    AddFrame(control_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 0, 5));

    // ---------------------------------------------------------------------------------
    
    TGHorizontalFrame* nav_main_frame = new TGHorizontalFrame(this);
    
    // manual navigation
    TGGroupFrame* nav_man_frame = new TGGroupFrame(nav_main_frame, "Manual navigation", kHorizontalFrame);
    nav_man_frame->SetTitlePos(TGGroupFrame::kLeft);

    fTB_Prev = new TGTextButton(nav_man_frame, "Previous");
    ResizeFrame(fTB_Prev);
    fTB_Prev->SetToolTipText("Go to previous element", 200);
    fTB_Prev->Connect("Clicked()", "ButtonWindow", this, "DoPrev()");
    nav_man_frame->AddFrame(fTB_Prev, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));
    
    fTB_Next = new TGTextButton(nav_man_frame, "   Next   ");
    ResizeFrame(fTB_Next);
    fTB_Next->SetToolTipText("Go to next element", 200);
    fTB_Next->Connect("Clicked()", "ButtonWindow", this, "DoNext()");
    nav_man_frame->AddFrame(fTB_Next, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));
    
    fTB_Ignore = new TGTextButton(nav_man_frame, "   Ignore   ");
    ResizeFrame(fTB_Ignore);
    fTB_Ignore->SetToolTipText("Go to next element and ignore current one", 200);
    fTB_Ignore->Connect("Clicked()", "ButtonWindow", this, "DoIgnore()");
    nav_man_frame->AddFrame(fTB_Ignore, new TGLayoutHints(kLHintsLeft, 0, 20, 10, 0));

    fTB_Goto = new TGTextButton(nav_man_frame, "Go to");
    ResizeFrame(fTB_Goto);
    fTB_Goto->SetToolTipText("Go to specified element", 200);
    fTB_Goto->Connect("Released()", "ButtonWindow", this, "Goto()");
    nav_man_frame->AddFrame(fTB_Goto, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));
    
    fNE_Elem = new TGNumberEntry(nav_man_frame, 0, 3, -1, TGNumberFormat::kNESInteger,
                      TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 0, 719);
    ResizeFrame(fNE_Elem);
    nav_man_frame->AddFrame(fNE_Elem, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));
    
    nav_main_frame->AddFrame(nav_man_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));

    // automatic navigation
    TGGroupFrame* nav_auto_frame = new TGGroupFrame(nav_main_frame, "Automatic navigation", kHorizontalFrame);
    nav_auto_frame->SetTitlePos(TGGroupFrame::kLeft);

    fNE_Delay = new TGNumberEntry(nav_auto_frame, 0.1, 3, -1, TGNumberFormat::kNESRealTwo,
                                  TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 0.01, 5);
    ResizeFrame(fNE_Delay);
    nav_auto_frame->AddFrame(fNE_Delay, new TGLayoutHints(kLHintsLeft, 0, 5, 10, 0));

    fTB_DoAll = new TGTextButton(nav_auto_frame, "Start");
    ResizeFrame(fTB_DoAll);
    fTB_DoAll->SetToolTipText("Process automatically", 200);
    fTB_DoAll->Connect("Clicked()", "ButtonWindow", this, "DoAll()");
    nav_auto_frame->AddFrame(fTB_DoAll, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));

    fTB_Stop = new TGTextButton(nav_auto_frame, "Stop");
    ResizeFrame(fTB_Stop);
    fTB_Stop->SetToolTipText("Stop processing", 200);
    fTB_Stop->Connect("Clicked()", "ButtonWindow", this, "Stop()");
    nav_auto_frame->AddFrame(fTB_Stop, new TGLayoutHints(kLHintsLeft, 0, 0, 10, 0));

    nav_main_frame->AddFrame(nav_auto_frame, new TGLayoutHints(kLHintsLeft, 5, 0, 0, 0));

    AddFrame(nav_main_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 0, 5));
     
    // window configuration
    Connect("CloseWindow()", "ButtonWindow", this, "Quit()");
    DontCallClose();
    
    // Map all subwindows of main frame
    MapSubwindows();

    // Initialize the layout algorithm
    Resize(GetDefaultSize());
    
    // show window
    MapRaised();

    // move window
    Move(gClient->GetDisplayWidth() - GetDefaultWidth(), 
         gClient->GetDisplayHeight() - GetDefaultHeight() - 30);
}

//______________________________________________________________________________
void ButtonWindow::ResizeFrame(TGFrame* f)
{
    // Resize the frame 'f'.

    f->Resize(70, 40);
    f->ChangeOptions(f->GetOptions() | kFixedSize );
}

//______________________________________________________________________________
void ButtonWindow::Goto()
{
    // Go to a certain element in the current module.

    // get the element number
    Int_t n = fNE_Elem->GetNumber();
  
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->ProcessElement(n);
}

//______________________________________________________________________________
void ButtonWindow::DoPrev()
{
    // Go to the previous element in the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Previous();
}

//______________________________________________________________________________
void ButtonWindow::DoNext()
{
    // Go to the next element in the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Next();
}

//______________________________________________________________________________
void ButtonWindow::DoIgnore()
{
    // Go to the next element in the current module and ignore current one.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Ignore();
}

//______________________________________________________________________________
void ButtonWindow::DoWrite()
{
    // Write the values of the current module to the database.

    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Write();
}

//______________________________________________________________________________
void ButtonWindow::Print()
{
    // Print the values obtained by the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->PrintValues();
}

//______________________________________________________________________________
void ButtonWindow::PrintChanges()
{
    // Print the changed values obtained by the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->PrintValuesChanged();
}

//______________________________________________________________________________
void ButtonWindow::Quit()
{
    // Quit the application.
    
    // delete list of modules
    delete gCaLibModules;
    
    // quit ROOT
    gApplication->Terminate();
}

//______________________________________________________________________________
void ButtonWindow::DoAll()
{
    // Process all elements automatically.

    Float_t delay = fNE_Delay->GetNumber();
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->ProcessAll(1000*delay);
}

//______________________________________________________________________________
void ButtonWindow::Stop()
{
    // Stop automatic processing of the current module.

    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->StopProcessing();
}

//______________________________________________________________________________
void ButtonWindow::EnableModuleSelection(Int_t i)
{
    // Enable the module selection.
    
    gCalibSelected = kTRUE;
}

//______________________________________________________________________________
void ButtonWindow::ReadRunsets(Int_t i)
{
    // Read the runsets for the calibration data of the 'i'-th module
    // in the module selection combo box
    
    // check if calibration was selected
    if (!gCalibSelected)
    {
        TGMsgBox* msg = new TGMsgBox(gClient->GetRoot(), gMainWindow, "Error", "Please select first "
                                     "the calibration you want to work with!",
                                     kMBIconStop, kMBOk, 0, kFitWidth | kFitHeight, kTextLeft);
        return;
    }

    // get the selected calibration
    TObjString* calibration = (TObjString*) gCalibrations->At(fCBox_Calibration->GetSelected());

    // get the calibration data of the module
    TCCalib* c = (TCCalib*) gCaLibModules->At(i);
    TString data = c->GetCalibData();
    
    // get the number of runsets
    Int_t nsets = TCMySQLManager::GetManager()->GetNsets(data.Data(), calibration->GetString().Data());
    
    // fill the runsets into the list
    fLB_RunSet->RemoveAll();
    for (Int_t i = 0; i < nsets; i++)
    {
        // get the first and last runs
        Char_t ctime[256];    
        Int_t first_run = TCMySQLManager::GetManager()->GetFirstRunOfSet(data.Data(), calibration->GetString().Data(), i);
        Int_t last_run = TCMySQLManager::GetManager()->GetLastRunOfSet(data.Data(), calibration->GetString().Data(), i);
        TCMySQLManager::GetManager()->GetChangeTimeOfSet(data.Data(), calibration->GetString().Data(), i, ctime);
        ctime[strlen(ctime)-3] = '\0';

        // add list entry
        Char_t tmp[256];
        sprintf(tmp, "Set %d (Run %d to %d) of %s", i, first_run, last_run, ctime);
        fLB_RunSet->AddEntry(tmp, i);
    }
    
    // update list box
    fLB_RunSet->Layout();
}

//______________________________________________________________________________
void ButtonWindow::StartModule()
{
    // Start the selected module.

    // get the selected module
    Int_t module = fCBox_Module->GetSelected();

    // get the selected runsets
    TList set_list;
    fLB_RunSet->GetSelectedEntries(&set_list);

    // check if a run set is actually selected
    if(set_list.GetSize() ==0) {
        puts("No Run Set selected!\n");
        return;
    }

    // fill sets
    Int_t nSet = set_list.GetSize();
    Int_t set[999];     
    for (Int_t i = 0; i < nSet; i++)
    {
        TGLBEntry* e = (TGLBEntry*) set_list.At(i);
        set[i] = e->EntryId();
    }

    // get the calibration module
    gCurrentModule = (TCCalib*) gCaLibModules->At(module);
    
    // get the selected calibration
    TObjString* calibration = (TObjString*) gCalibrations->At(fCBox_Calibration->GetSelected());
    
    // start the module
    ((TCCalib*)gCurrentModule)->Start(calibration->GetString().Data(), nSet, set);
}

//______________________________________________________________________________
void CreateModuleList()
{
    // Find all calibration modules and create a list.
    
    // create the module list
    gCaLibModules = new TList();
    gCaLibModules->SetOwner(kTRUE);

    // init class list
    gClassTable->Init();

    // loop over all classes
    Int_t nClasses = gClassTable->Classes();
    for (Int_t i = 0; i < nClasses; i++)
    {
        // get class name
        TString c(gClassTable->At(i));

        // get TCCalib* classes
        if (c.BeginsWith("TCCalib"))
        {
            // skip non-module (base) classes
            if (c == "TCCalib") continue;
            if (c == "TCCalibPed") continue;
            if (c == "TCCalibTime") continue;
            if (c == "TCCalibEnergy") continue;
            if (c == "TCCalibQuadEnergy") continue;
            if (c == "TCCalibDiscrThr") continue;

            // add module to list if it is really a module
            TClass tc(c.Data());
            if (tc.InheritsFrom("TCCalib")) gCaLibModules->Add((TCCalib*) tc.New());
        }
    }
}

//______________________________________________________________________________
void CalibrateGUI()
{
    // Main method.

    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // find CaLib modules
    CreateModuleList();
    
    // no current module
    gCurrentModule = 0;
    
    // calibration not yet selected
    gCalibSelected = kFALSE;

    // Main method.
    gMainWindow = new ButtonWindow();
}

