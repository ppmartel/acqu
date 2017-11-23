//--Author	JRM Annand    2nd Dec 2007
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand... 1st Sep 2009 delete[]
//--Rev 	JRM Annand...12th May 2011 general bug fix and upgrade
//--Rev 	JRM Annand...16th May 2011 debug auto running
//--Rev 	JRM Annand... 6th Jul 2011 gcc4.6-x86_64, Button ptrs.
//--Rev 	JRM Annand...10th Jul 2011 acknowledge change of comment
//--Rev 	JRM Annand...12th Jul 2012 extend for trigger control
//--Rev 	JRM Annand...28th Sep 2012 new trigger control buttons
//--Update	JRM Annand...18th Sep 2013 path for gui logos
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQguiCtrl
// GUI control of online DAQ and Status of Analyser
// Run from AcquRoot thread, controlling either local readout, or via
// network connection to remote DAQ.
// Note the GUI requires ASimage library
// Build root with: ./configure linuxx8664gcc  --gminimal --enable-asimage
//
#include "TDAQguiCtrl.h"
#include "TDAQexperiment.h"
#include "TDAQsupervise.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2DataServer.h"
#include "TColor.h"
#include "CMakeConfig.h"



ULong_t blue;         // pale blue
ULong_t lightBlue;
ULong_t midBlue;
ULong_t darkBlue;
ULong_t white;        // white
ULong_t offWhite;
ULong_t grey;
ULong_t black;

//-----------------------------------------------------------------------------
TGGC* SetGCVal( ULong_t colF, ULong_t colB, TGFont* font )
{
  // Get widget configuration for colours and font
  //
  GCValues_t valpFr;
  valpFr.fMask = kGCForeground | kGCBackground | kGCFillStyle | 
    kGCFont | kGCGraphicsExposures;
  valpFr.fForeground = colF;
  valpFr.fBackground = colB;
  valpFr.fFillStyle = kFillSolid;
  valpFr.fFont = font->GetFontHandle();
  valpFr.fGraphicsExposures = kFALSE;
  return gClient->GetGC(&valpFr, kTRUE);
}

//-----------------------------------------------------------------------------
TGGroupFrame* TDAQguiCtrl::SetGroup( TGCompositeFrame* f, Int_t x, Int_t y,
				     Int_t sx, Int_t sy,
				     ULong_t colF, ULong_t colB,
				     TGFont* font, const Char_t* name )
{ 
  // Confirgure container frame to hold a set of widgets
  // position, size, colours, font and label
  //
  TGGC   *uGC = SetGCVal( colF, colB, font );
  TGGroupFrame *gr = 
    new TGGroupFrame(f,name,kVerticalFrame,uGC->GetGC(),font->GetFontStruct(),
		     colB);
  gr->SetTitlePos(TGGroupFrame::kCenter);
  gr->SetLayoutBroken(kTRUE);
  f->AddFrame(gr,new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  gr->MoveResize(x,y,sx,sy);
  return gr;
}

//-----------------------------------------------------------------------------
TGTextButton* TDAQguiCtrl::SetButton( TGGroupFrame* gr, Int_t x, Int_t y,
				      Int_t sx, Int_t sy, ULong_t colF, ULong_t colB,
				      TGFont* font,
				      const Char_t* name, const Char_t* connect )
{
  // Enter a labeled button widget into a container...
  // Position, size, colours, font and label
  // Non-null connect string specifies button-click procedure
  //
  TGGC   *uGC = SetGCVal( colF, colB, font );
  TGTextButton *b = NULL;
  if( connect ){
    if( !strncmp( connect,"Check",5) )
      b =  new TGCheckButton(gr,name,-1,uGC->GetGC(),font->GetFontStruct());
  }
  if( !b )b = new TGTextButton(gr,name,-1,uGC->GetGC(),font->GetFontStruct());
  b->SetTextJustify(36);
  b->ChangeBackground(colB);
  if( connect )
    b->Connect("Clicked()","TDAQguiCtrl",this,connect);
  gr->AddFrame(b,new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  b->MoveResize(x,y,sx,sy);
  return b;
}

//-----------------------------------------------------------------------------
TGNumberEntry* TDAQguiCtrl::SetNumber( TGGroupFrame* gr, Int_t x, Int_t y,
				      Int_t sx, Int_t sy, ULong_t colF, ULong_t colB,
				      TGFont* font,
				       const Char_t* name, const Char_t* connect, Bool_t isHex )
{
  // Enter a labeled button widget into a container...
  // Position, size, colours, font and label
  // Non-null connect string specifies button-click procedure
  //
  //  TGGC   *uGC = SetGCVal( colF, colB, font );
  TGNumberEntry *b = NULL;
  if( isHex )
    b = new TGNumberEntry( gr, 0, 5, -1, TGNumberFormat::kNESHex );
  else
    b = new TGNumberEntry( gr, 0, 5, -1 );
  //  b->SetNumStyle(EStyle.kNESHex);
  //  b->SetTextJustify(36);
  b->ChangeBackground(colB);
  if( connect )
    b->Connect("ValueSet(Long_t)","TDAQguiCtrl",this,connect);
  gr->AddFrame(b,new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  SetLabel( gr, x-sx, y, sx, sy, colF, colB, font, name );
  b->MoveResize(x,y,sx,sy);
  return b;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetLabel( TGGroupFrame* gr, Int_t x, Int_t y,
			    Int_t sx, Int_t sy, ULong_t colF, ULong_t colB,
			    TGFont* font, const Char_t* name )
{
  // Enter a text label into a container frame
  // Position, size, colours, font and label
  //
  TGGC   *uGC = SetGCVal( colF, colB, font );
  TGLabel *l = 
    new TGLabel(gr,name,uGC->GetGC(),font->GetFontStruct(),kChildFrame, colB);
  l->SetTextJustify(36);
  gr->AddFrame(l,new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  l->MoveResize(x,y,sx,sy);
}

//-----------------------------------------------------------------------------
TGTextEntry* TDAQguiCtrl::SetText(TGGroupFrame* gr, Int_t x, Int_t y,
				  Int_t sx, Int_t sy, ULong_t colF, 
				  ULong_t colB, TGFont* font,
				  const Char_t* label, Int_t sl,
				  const Char_t* connect )
{
  // Enter a text-line display/entry widget into a group frame.
  // Position, size, colours, font
  // Non-null label string causes label to be written to the left of
  // the text-input widget
  // Non-null connect string gives optional change-text procedure
  //
  TGGC   *uGC = SetGCVal( black, offWhite, font );
  TGTextEntry *t =
    new TGTextEntry(gr, new TGTextBuffer(15),-1,uGC->GetGC(),
		    font->GetFontStruct(),
		    kSunkenFrame | kDoubleBorder | kOwnBackground, fTextBack);
  if( label ){
    SetLabel( gr,x,y,sl,sy, colF, colB, font, label );
    x += sl + 2;
    sx -= ( sl + 2);
  }
  t->SetMaxLength(255);
  t->SetAlignment(kTextLeft);
  t->SetText("");
  if( connect ){
    t->Connect("ReturnPressed()","TDAQguiCtrl",this,connect);
  }
  gr->AddFrame(t,new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  t->MoveResize(x,y,sx,sy);
  return t;
}

//-----------------------------------------------------------------------------
TDAQguiCtrl::TDAQguiCtrl( TDAQsupervise* s, const TGWindow* w )
  : TGMainFrame(w, 10,10,kMainFrame | kVerticalFrame)
{
  // AcquRoot GUI Control constructor
  // TGWindow w normally produced by gClient->GetRoot()
  fSupervise = s;                      // core control/supervise class
  fAR = s->GetEXP()->GetAR();          // local analysis if any
  SetLayoutBroken(kTRUE);
  fChan = 0;
  
  // Specify colours from the colour wheel
  blue = TColor::Number2Pixel(kBlue);
  midBlue = TColor::Number2Pixel(kBlue-8);
  lightBlue = TColor::Number2Pixel(kBlue-9);
  darkBlue = TColor::Number2Pixel(kBlue+2);
  white = TColor::Number2Pixel(kWhite);
  offWhite = TColor::Number2Pixel(kYellow-10);
  grey = TColor::Number2Pixel(kGray);
  black = TColor::Number2Pixel(kBlack);
  fTextBack = white;

  // Specify character fonts
  TGFont* Helv24B =
    gClient->GetFont("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-*-*");
  TGFont* Helv14B =
    gClient->GetFont("-*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*");
  //  TGFont* Helv12B =
  //    gClient->GetFont("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");
  TGFont* Helv12M = 
    gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

  // Main grouping "AcquRoot Online Control" group frame
  TGGC   *uGC = SetGCVal( white, grey, Helv24B );
  TGGroupFrame *fMGr = 
    new TGGroupFrame(this,"AcquDAQ Online Control",kVerticalFrame,
		     uGC->GetGC(),Helv24B->GetFontStruct(),darkBlue);
  fMGr->SetTitlePos(TGGroupFrame::kCenter);
  fMGr->SetLayoutBroken(kTRUE);
  Char_t* logoG = s->BuildName("data/GlaLogo1.jpg");
  Char_t* logoM = s->BuildName("data/MzLogo.jpg");
  TGIcon *gla = new TGIcon(fMGr, logoG);
  TGIcon *mz = new TGIcon(fMGr, logoM);
  delete[] logoG;
  delete[] logoM;
  fMGr->AddFrame(gla, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  gla->MoveResize(5,20,91,50);
  fMGr->AddFrame(mz, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  mz->DrawBorder();
  mz->MoveResize(560,20,74,50);
  
  // Number input for trigger system control
  SetLabel( fMGr, 110, 30, 100, 40, white, darkBlue, Helv14B, "\n Trigger\n" );
  fVuEnCPU = SetNumber(fMGr,440,34,62,24, white, midBlue, Helv12M,
		       "En-CPU 0x","EnCPU()", kTRUE);
  fVuIntDelay = SetNumber(fMGr,440,62,62,24, white, midBlue, Helv12M,
			  "Int-Del","IntDel()");
  fVuFCDelay = SetNumber(fMGr,440,90,62,24, white, midBlue, Helv12M,
			 "FC-Del","FCDel()");
  fVuRAMDownload = SetNumber(fMGr,440,118,62,24, white, midBlue, Helv12M,
			     "RAM-ID","RAM()");
  fVuInputMask = SetNumber(fMGr,310,34,62,24, white, midBlue, Helv12M,
			   "In-Mask 0x","InMask()", kTRUE);
  fVuInputPrescale = SetNumber(fMGr,310,62,62,24, white, midBlue, Helv12M,
			   "In-Pre","InPre()");
  fVuL1Prescale = SetNumber(fMGr,310,90,62,24, white, midBlue, Helv12M,
			   "L1-Pre","L1Pre()");
  fVuL2Prescale = SetNumber(fMGr,310,118,62,24, white, midBlue, Helv12M,
			   "L2-Pre","L2Pre()");
  fVuSetL1Width = SetNumber(fMGr,180,62,62,24, white, midBlue, Helv12M,
			   "L1-Width","SetL1Width()");
  fVuSetDelay = SetNumber(fMGr,180,90,62,24, white, midBlue, Helv12M,
			    "Strobe-D","SetDelay()");
  fVuSetMThresh = SetNumber(fMGr,180,118,62,24, white, midBlue, Helv12M,
			   "Set-MThr","SetMThresh()");
  fVuSetDebug = SetNumber(fMGr,570,90,62,24, white, midBlue, Helv12M,
			   "Set-Deb","SetDeb()");
  fVuSetChan = SetNumber(fMGr,570,118,62,24, white, midBlue, Helv12M,
			   "Set-Chan","SetChan()");
  
  // "DAQ Control" group frame
  TGGroupFrame *fCtrl = SetGroup( fMGr,8,150,478,74,white,midBlue,Helv14B,
				  "AcquDAQ Control");
  //  TGTextButton* b;
  fStartButton = SetButton(fCtrl,16,24,48,28, white, darkBlue, Helv12M,
			   "Start","Start()");
  fPauseButton = SetButton(fCtrl,72,24,48,28, white, darkBlue, Helv12M,
			   "Pause","Pause()");
  fRunButton = SetButton(fCtrl,128,24,48,28, white, darkBlue, Helv12M,
			 "Run","ExecRun()");
  fEndButton = SetButton(fCtrl,184,24,48,28, white, darkBlue, Helv12M,
			 "End","End()");
  fGoButton = SetButton(fCtrl,240,24,48,28, white, darkBlue, Helv12M,
			 "Go","Go()");
  fStopButton = SetButton(fCtrl,296,24,48,28, white, darkBlue, Helv12M,
			 "Stop","Stop()");
  fKillButton = SetButton(fCtrl,352,24,48,28, white, darkBlue, Helv12M,
		"Kill","Kill()");
  fAutoButton = (TGCheckButton*)SetButton(fCtrl,408,24,48,28, white,
					  darkBlue,Helv12M,"Auto",
					  "CheckAuto()");

  // "DAQ Diagnostics" group frame
  TGGroupFrame *fDiag = SetGroup( fMGr,494,150,142,74,white,midBlue,Helv14B,
				  "Diagnostics");
  fDumpButton = SetButton(fDiag, 16, 24, 48, 28, white, darkBlue, Helv12M,
			  "Dump","Dump()");
  fTestButton = SetButton(fDiag, 72, 24, 48, 28, white, darkBlue, Helv12M,
			  "Test",NULL);

  // "Run File Status" group frame
  TGGroupFrame *fStatus = SetGroup( fMGr,8,234,628,140,white,midBlue,Helv14B,
				    "Run-File Status");
  uGC =  SetGCVal( black, white, Helv12M );
  fExp =SetText( fStatus,16,24,600,26,white,darkBlue, Helv12M,
		 "Experiment",70,NULL);
  fFile = SetText( fStatus,16,88,500,26,white,darkBlue, Helv12M,
		   "File",70);
  fTextBack = offWhite;
  fCom = SetText( fStatus,16,56,600,26,white,darkBlue, Helv12M,
		  "Comment",70,"SaveComment()");
  fRun = new TGNumberEntry( fStatus, 0, 5, -1 );
  //			   (TGNumberFormat::EStyle)kNESInteger,
  //			   (TGNumberFormat::EAttribute)kNEANonNegative);
  fStatus->AddFrame(fRun, 
		    new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  SetLabel( fStatus, 520, 88, 32, 26, white, darkBlue, Helv12M, "Run" );
  fRun->MoveResize(554,88,62,26);
  fRun->Connect("ValueSet(Long_t)","TDAQguiCtrl",this,"SaveRun()");
    
  // "DAQ Dialogue from TDAQsupervise" group frame
  TGGroupFrame *fDial = SetGroup( fMGr,8,384,628,270,white,midBlue,Helv14B,
				  "AcquDAQ Dialogue");
  fCmd =SetText( fDial,16,24,600,26,white,darkBlue, Helv12M,
  		 "AcquDAQ>",70,"IssueCmd()");
   
  fMsg = new TGTextView(fDial,520,216);
  Char_t* initMsg = fSupervise->BuildName("AcquDAQ ",ENV_OR_CMAKE("acquversionID", CMAKE_ACQU_VERSION_ID),
			      " Messages:\n");
  fMsg->LoadBuffer(initMsg);
  fSupervise->SetGUIBuffer( (Char_t*)fMsg->GetText() );
  fDial->AddFrame(fMsg, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fMsg->MoveResize(16,56,596,192);

  fAnStat = SetGroup( fMGr,8,666,628,256,white,midBlue,Helv14B,
		      "Data Storage Status");
  // Embedded canvas
  fECanv = new TRootEmbeddedCanvas(0,fAnStat,256,192);
  Int_t wIDcanv = fECanv->GetCanvasWindowId();
  fCanv = new TCanvas("ARembGUI", 10, 10, wIDcanv);
  fECanv->AdoptCanvas(fCanv);
  fAnStat->AddFrame(fECanv,new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
  fECanv->MoveResize(300,30,312,202);
  fTextBack = white;
  fHardErr = SetText( fAnStat,16,32,114,26,white,darkBlue, Helv12M,
		      "Hard Err",64,NULL);  
  fADCErr = SetText( fAnStat,16,64,114,26,white,darkBlue, Helv12M,
		     "ADC Err",64,NULL);  
  fScalerErr = SetText( fAnStat,16,96,114,26,white,darkBlue, Helv12M,
			"Scaler Err",64,NULL);  
  fEvent = SetText( fAnStat,16,128,114,26,white,darkBlue, Helv12M,
		    "Event",48,NULL);  
  fBuffRec = SetText( fAnStat,144,32,114,26,white,darkBlue, Helv12M,
		      "Buff Rec.",64,NULL);  
  fBuffSort = SetText( fAnStat,144,64,114,26,white,darkBlue, Helv12M,
		       "Buff Sort",64,NULL);  
  fMByte = SetText( fAnStat,144,96,114,26,white,darkBlue, Helv12M,
		    "Data MByte",64,NULL);  
  fRunTime = SetText( fAnStat,144,128,114,26,white,darkBlue, Helv12M,
		    "Run Time",64,NULL);  
  fUpdateButton = SetButton(fAnStat,16, 160, 48, 28, white, darkBlue, Helv12M,
			    "Update","GetAnalysis()");
  fStoreButton = (TGCheckButton*)SetButton(fAnStat,144,160,64,28, white,
					   darkBlue,Helv12M,"Store",
					   "CheckStore()");
  
  fMGr->SetLayoutManager(new TGVerticalLayout(fMGr));
  AddFrame(fMGr, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fMGr->MoveResize(0,0,644,948);
  SetMWMHints(kMWMDecorAll, kMWMFuncAll, kMWMInputModeless);
  MapSubwindows();
  SetWindowName("AcquDAQ-GUI");
  Resize(GetDefaultSize());
  MapWindow();
  Resize(644,948);
  LoadMsgBuffer("");
  fAutoRun = fAutoEnd = kFALSE;
  // Set up event rate histo
  fMaxTick = 5000;
  fEventRate = new TH1F("Rate","AcquDAQ Event Rate",fMaxTick,0,fMaxTick);
  fEventRate->GetXaxis()->SetTitle("Elapsed Run Time (s)");
  fEventRate->SetFillColor(kBlue);
  // Show run information
  ShowRun();
  // Set up the timed stuff
  // default run every sec.
  fLastEvent = 0;
  fTick = 0;
  fTickLength = 1000;      // 1000 ms = 1 s
  fUpdate = new TTimer();
  fUpdate->Connect("Timeout()","TDAQguiCtrl",this,"Update()");
  fUpdate->Start(fTickLength,kFALSE);    // update status every second
  fEventRate->Draw();                    // show event-rate histo
}

//-----------------------------------------------------------------------------
TDAQguiCtrl::~TDAQguiCtrl()
{
  // Clean up all widgets, frames and layout hints that were used
  //  delete fMain;
  Cleanup();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Start()
{
  // Enable event interrupts, don't start new run file
  //
  fSupervise->ExecStart();
  //  fIsRunning = kTRUE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Go()
{
  // Enable event interrupts, don't start new run file
  //
  fSupervise->ExecGo();
  //  fIsRunning = kTRUE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::ShowRun()
{
  // Show run information
  fExp->SetText( fSupervise->GetRunDesc() );
  fCom->SetText( fSupervise->GetComment() );
  fFile->SetText( fSupervise->GetFileName() );
  fRun->SetIntNumber( fSupervise->GetRunNumber() );
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::ExecRun()
{
  // Start new run file and enable event interruptsw
  //  
  fLastEvent = 0;
  ResetEvent();
  ShowRun();
  fSupervise->ExecRun( );
  //  fIsRunning = kTRUE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Pause()
{
  // Disable event interrupts, don't close run file
  //
  fSupervise->ExecPause();
  //  fIsRunning = kFALSE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Stop()
{
  // Disable event interrupts, don't close run file
  //
  fSupervise->ExecStop();
  //  fIsRunning = kFALSE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::End()
{
  // Close run file and disable event interrupts
  //
  fSupervise->ExecEnd();
  //  fIsRunning = kFALSE;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Kill()
{
  // Shutdown the DAQ system
  //
  fSupervise->ExecKill();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Dump()
{
  // Dump a data buffer
  //
  fSupervise->ExecBufferDump();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::SaveExperiment()
{
  // Save changed experiment line to TDAQsupervise
  //
  if( fSupervise->IsRunInit() ){
    fSupervise->PutString("<Change not permitted while Run in progress>\n");
    return;
  }
  strcpy( fSupervise->GetRunDesc(), fExp->GetText() );
  fSupervise->PutString("<New experiment description entered>\n");
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SaveComment()
{
  // Save changed comment line to TDAQsupervise
  //
  if( fSupervise->IsRunInit() ){
    fSupervise->PutString("<Change not permitted while Run in progress>\n");
    return;
  }
  strcpy( fSupervise->GetComment(), fCom->GetText() );
  fSupervise->PutString("<New run comment entered>\n");
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SaveFile()
{
  // Save changed file-name line to TDAQsupervise
  //
  if( fSupervise->IsRunInit() ){
    fSupervise->PutString("<Change not permitted while Run in progress>\n");
    return;
  }
  strcpy( fSupervise->GetFileName(), fFile->GetText() );
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::SaveRun()
{
  // Write changed run number to TDAQsupervise and to file Run.log
  //
  if( fSupervise->IsRunInit() ){
    fSupervise->PutString("<Change not permitted while Run in progress>\n");
    return;
  }
  fSupervise->SetRunNumber( fRun->GetIntNumber()-1 );
  //  fSupervise->SaveRunLog();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::Update()
{
  // Update run-file status from TDAQsupervise
  //
  if( fAutoRun ){
    ExecRun();
    fAutoRun = kFALSE;
    return;
  }
  else if( fAutoEnd ){
    End();
    fAutoEnd = kFALSE;
    return;
  }
  if( fSupervise->IsDataReady() ){
    fSupervise->GetString();
    LoadMsgBuffer( fSupervise->GetCommandLine());
  }
  if(!IsRunning()) return;
  fFile->SetText( fSupervise->GetFileName() );
  fRun->SetIntNumber( fSupervise->GetRunNumber() );
  fMsg->ShowBottom();
  GetAnalysis();
  GetTrigger();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::LoadMsgBuffer( const Char_t* msg )
{
  // Running DAQ dialogue from TDAQsupervise to GUI frame
  //
  fMsg->AddLine( msg );
  fMsg->ShowBottom();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::SaveNumber( TGTextEntry* gtext, Int_t value )
{
  // Write integer number to text entry widget
  // 
  Char_t cvalue[16];
  sprintf( cvalue, "%d", value );
  gtext->SetText( cvalue );
  return;
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::GetAnalysis()
{
  // Get experiment parameters
  Double_t Mbyte;
  Int_t nEvent;
  if( fAR ){
    TA2DataServer* ds = fAR->GetDataServer();
    nEvent = fAR->GetCurrEvent();
    Mbyte = ((ds->GetNDataBuffer() - fStartDataBuff)
		      * ds->GetRecLen())/1048576;
    SaveNumber( fHardErr, fAR->GetHardError() );
    SaveNumber( fADCErr, fAR->GetADCError() );
    SaveNumber( fScalerErr, fAR->GetScalerError() );
    SaveNumber( fBuffRec, fAR->GetNDataBuffer() );
    SaveNumber( fBuffSort, ds->GetNDataBuffer() );
    EButtonState state = (EButtonState)ds->IsStore();
    fStoreButton->SetState( state, fAR->GetAnalysis()->IsPhysics() );
  }
  else {
    nEvent = fSupervise->GetEXP()->GetNEvent();
    Mbyte = fSupervise->GetEXP()->GetNRec() * fSupervise->GetEXP()->GetRecLen()
      /1048576;
    SaveNumber( fHardErr, -1 );
    SaveNumber( fADCErr, fSupervise->GetEXP()->GetNADCError() );
    SaveNumber( fScalerErr, fSupervise->GetEXP()->GetNScalerError() );
    SaveNumber( fBuffRec, fSupervise->GetEXP()->GetNRec() );
    SaveNumber( fBuffSort, -1 );
    EButtonState state = (EButtonState)fSupervise->GetEXP()->IsStore();
    fStoreButton->SetState( state, kFALSE );
  }
  if( IsRunning() ){
    SaveNumber( fEvent, nEvent );
    SaveNumber( fMByte, (Int_t)Mbyte );
    SaveNumber( fRunTime, fTick );
    Double_t rate = (nEvent - fLastEvent) * 1000/fTickLength;
    fLastEvent = nEvent;
    fEventRate->SetBinContent(fTick, rate);
    fEventRate->Draw();
    fCanv->cd();
    fCanv->Update();
    fTick++;
    if( fTick >= fMaxTick ) ResetEvent();
  }
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::GetTrigger()
{
  // Get trigger module settings
  // if there is any trigger module there
  Int_t* parm = fSupervise->GetVUPROMparm(fChan);
  if( !parm ) return;
  fVuEnCPU->SetIntNumber(*parm++);
  fVuIntDelay->SetIntNumber(*parm++);
  fVuFCDelay->SetIntNumber(*parm++);
  fVuRAMDownload->SetIntNumber(*parm++);
  fVuInputMask->SetIntNumber(*parm++);
  fVuInputPrescale->SetIntNumber(*parm++);
  fVuL1Prescale->SetIntNumber(*parm++);
  fVuL2Prescale->SetIntNumber(*parm++);
  fVuSetL1Width->SetIntNumber(*parm++);
  fVuSetDelay->SetIntNumber(*parm++);
  fVuSetMThresh->SetIntNumber(*parm++);
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::CheckStore()
{
  // Set data write/nowrite according to state of check button
  //
  if( fSupervise->IsRunInit() ){
    fSupervise->PutString("<Change not permitted while Run in progress>\n");
    return;
  }
  TA2DataServer* ds = fAR->GetDataServer();
  EButtonState state = fStoreButton->GetState();

  if( state ) LoadMsgBuffer( "Data storage enabled\n"); 
  else LoadMsgBuffer( "Data storage disabled\n"); 
  ds->SetIsStore( state );
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::CheckAuto()
{
  // Set data write/nowrite according to state of check button
  //
  EButtonState state = fAutoButton->GetState();

  if( state ) LoadMsgBuffer( "Auto file changing enabled\n"); 
  else LoadMsgBuffer( "Auto file changing disabled\n"); 
  fSupervise->SetAuto( state );
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::UpdateStartBuff()
{
  // Store the start-buffer number for a new data file
  //
  if( fAR ){
    TA2DataServer* ds = fAR->GetDataServer();
    fStartDataBuff = ds->GetNDataBuffer();
  }
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::IssueCmd()
{
  // Run loop checks and updates status every second
  //
  fSupervise->CommandConfig((Char_t*)fCmd->GetText());
  Update();
}

//-----------------------------------------------------------------------------
void TDAQguiCtrl::ResetEvent()
{
  // Reset the event-rate histogram
  //
  fEventRate->Reset();
  fTick = 0;
}

// DAQ trigger conditions setup
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetChan()
{
  // Set the channel to act upon....e.g. 0-15 for inputs
  fChan = fVuSetChan->GetIntNumber();
  GetTrigger();
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::EnCPU()
{
  // CPU enable mask
  Char_t line[128];
  Int_t datum = fVuEnCPU->GetHexNumber();
  sprintf(line, "Enable-CPU: %d\n", datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::IntDel()
{
  // Interrupt delay to CPU fChan
  Char_t line[128];
  Int_t datum = fVuIntDelay->GetIntNumber();
  sprintf(line, "Int-Delay: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::FCDel()
{
  // Fast clear delay
  Char_t line[128];
  Int_t datum = fVuFCDelay->GetIntNumber();
  sprintf(line, "FC-Delay: %d\n", datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::RAM()
{
  // Trigger conditions RAM load
  Char_t line[128];
  Int_t datum = fVuRAMDownload->GetIntNumber();
  sprintf(line, "RAM-Download: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::InMask()
{
  // Trigger inputs mask
  Char_t line[128];
  Int_t datum = fVuInputMask->GetIntNumber();
  sprintf(line, "Input-Mask: %d\n", datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::InPre()
{
  // Trigger prescale input fChan 
  Char_t line[128];
  Int_t datum = fVuInputPrescale->GetIntNumber();
  sprintf(line, "Input-Prescale: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::L1Pre()
{
  // L1 output fChan prescale
  Char_t line[128];
  Int_t datum = fVuL1Prescale->GetIntNumber();
  sprintf(line, "L1-Prescale: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::L2Pre()
{
  // L2 output fChan prescale
  Char_t line[128];
  Int_t datum = fVuL2Prescale->GetIntNumber();
  sprintf(line, "L2-Prescale: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetDeb()
{
  // Debugging
  Char_t line[128];
  Int_t datum = fVuSetDebug->GetIntNumber();
  sprintf(line, "Set-Debug-Out: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetL1Width()
{
  // L1 output gate width
  Char_t line[128];
  Int_t datum = fVuSetL1Width->GetIntNumber();
  sprintf(line, "L1-Width: %d\n", datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetDelay()
{
  // L1 or L2 strobe delay
  Char_t line[128];
  Int_t datum = fVuSetDelay->GetIntNumber();
  sprintf(line, "Strobe-Delay: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}
//-----------------------------------------------------------------------------
void TDAQguiCtrl::SetMThresh()
{
  // Multiplicity thresholds
  Char_t line[128];
  Int_t datum = fVuSetMThresh->GetIntNumber();
  sprintf(line, "Set-M-Threshold: %d %d\n", fChan, datum);
  fSupervise->ConfigVUPROM(line);
}

ClassImpQ(TDAQguiCtrl)
