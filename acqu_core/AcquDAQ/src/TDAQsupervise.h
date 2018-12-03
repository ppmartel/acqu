//--Author	JRM Annand   11th Mar 2006
//--Rev 	JRM Annand...
//--Rev 	JRM Annand...26th Mar 2007 AcquDAQ
//--Rev 	JRM Annand...17th May 2007 Bug fix create header
//--Rev 	JRM Annand... 7th Dec 2007 Accomodate GUI control
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand... 1st Sep 2009 delete[]
//--Rev 	JRM Annand...19th Jul 2010 Slave mode
//--Rev         JRM Annand    4th Sep 2010 Check Mk1/Mk2 data
//--Rev         JRM Annand    6th Sep 2010 SetTrigMod()
//--Rev         JRM Annand   11th Sep 2010 Debug adc/scaler range Mk1 header
//--Rev         JRM Annand   12th Sep 2010 Add ConfigTCS() method & NetSlave
//--Rev         JRM Annand   15th Sep 2010 Move socket init to command loop
//--Rev         JRM Annand    4th Feb 2011 Add CAM_IO procedure
//--Rev         JRM Annand    5th Feb 2011 Mods to ExecRun()
//--Rev         JRM Annand   30th Apr 2011 Local GUI ctrl (Not local AcquRoot)
//--Rev         JRM Annand   12th May 2011 Command-loop mods for TDAQguiCtrl
//--Rev         JRM Annand   16th May 2011 Debug auto+GUI
//--Rev         JRM Annand    6th Jul 2011 gcc4.6-x86_64 warning fix
//--Rev         JRM Annand   20th May 2012 Add Go: and Stop: commands
//--Rev         JRM Annand    9th Jul 2012 Add ConfigVUPROM: command
//--Rev         JRM Annand    1st Sep 2012 ExecKill..explicit kill GUI
//--Rev         JRM Annand   29th Sep 2012 GetVUPROMparm()
//--Rev         JRM Annand    1st Oct 2012 Allow stop/go when storing
//--Rev         JRM Annand    2nd Mar 2013 VADC/VScalers Mk2 header counting
//--Rev         JRM Annand   10th Jul 2013 V874 config
//--Rev         JRM Annand    9th Sep 2013 mod to end-run...wait for expt
//--Rev         JRM Annand   24th Sep 2013 set fIRQMod from exp->PostInit()
//--Update      JRM Annand    8th Nov 2016 timeout in ExecEnd() procedure
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQsupervise
// Supervise and control operation of AcquDAQ
//

#ifndef __TDAQsupervise_h__
#define __TDAQsupervise_h__

#include "TA2Mk2Format.h"
#include "TA2Mk1Format.h"
#include "TControlBar.h"

R__EXTERN class TA2Control*  gCTRL;  // global ptr to AcquRoot application
class TDAQguiCtrl;


// Modes of entering control info to DAQ
enum { EExpCtrlUndef, EExpCtrlNet, EExpGUINet, EExpCtrlLocalAR, EExpCtrlLocal,
       EExpCtrlSlave, EExpCtrlNetSlave, EExpCtrlGUILocal};
// Command keys
enum { ESupStart, ESupGo, ESupRun, ESupPause, ESupStop, ESupEnd, ESupKill, 
       ESupDump,
       ESupExperiment, ESupComm, ESupFile, ESupRunNumber,
       ESupAuto, ESupNoAuto, ESupIsRunning, ESupConfigTCS, ESupSetTCSRunMode, ESupCAMAC, 
       ESupVUPROM, ESupV874 };
// timeout (in usleeps) waiting for TDAQ experiment to signal run ended
// with single-node DAQ the wait gets into an infinite loop
enum { ETimeOut = 2000000 };

class ARSocket_t;
class TDAQmodule;
class TDAQexperiment;

class TDAQsupervise : public TA2System {
 protected:
  TDAQexperiment* fEXP;              // DAQ container class
  TDAQguiCtrl* fGUICtrl;             // GUI specs.
  TControlBar* fCtrlBar;             // for GUI control
  Int_t fExpCtrlMode;                // mode of control I/O
  ARSocket_t* fSocket;               // network communication socket
  Char_t* fServerIP;                 // remote communications server
  Int_t fPort;                       // communications port
  TDAQmodule* fIRQMod;               // IRQ and trigger control module
  TDAQmodule* fTrigMod;              // Trigger hardware inhibit control module
  Char_t fTime[EMk2SizeTime];        // holds system time
  Char_t fComment[EMk2SizeComment];  // comment on DAQ session
  Char_t fFileName[EMk2SizeFName];   // output file name
  Char_t fCommandLine[ELineSize];    // for input commands
  Char_t* fGUIBuffer;                // diagnostic output to GUI text window
  Char_t* fOutBuff;                  // output buffer
  Int_t fRunNumber;                  // Experimental run number
  UInt_t fDataHeader;                // Either Mk1 or Mk2 format
  UInt_t fTimeOut;                   // Wait timeout for TDAQexperiment finish
  Bool_t fIsRunInit;                 // Experimental run initialised
  Bool_t fIsRunTerm;                 // Experimental run terminated
  Bool_t fIsBufferDump;              // Data buffer printout?
  Bool_t fIsAuto;                    // Auto file change?
  Bool_t fIsRunning;                 // DAQ currently running
 public:
  TDAQsupervise( Char_t*, TDAQexperiment*, Char_t* = NULL );
  virtual ~TDAQsupervise( );
  virtual void SetConfig( Char_t*, Int_t );
  virtual void SetInputMode( Char_t* );
  virtual void GetString();
  virtual void PutString( const Char_t* );
  virtual void CommandLoop( );
  virtual void ExecAutoStart( );
  virtual void ExecStart( );
  virtual void ExecGo( );
  virtual void ExecRun(Int_t = -1);
  virtual void ExecPause(  );
  virtual void ExecStop(  );
  virtual void ExecEnd(  );
  virtual void ExecKill(  );
  virtual void ExecBufferDump();
  virtual void DAQEnable();
  virtual void DAQDisable();
  virtual void GetSysTime();
  virtual void CreateHeader( void* );
  virtual void CreateMk1Header( void* );
  virtual void CreateHeader( void*, UInt_t );
  virtual Char_t* GetRunDesc();
  virtual void SaveRunLog();
  virtual Int_t GetRunLog();
  virtual void ConfigTCS();
  virtual void SetTCSRunMode(UInt_t runmode);
  virtual void CAMAC_IO(Int_t,Int_t,Int_t,Int_t,Int_t);
  virtual void ConfigVUPROM(Char_t*);
  virtual Int_t* GetVUPROMparm(Int_t);
  virtual void ConfigV874(Char_t*);
  //
  TDAQexperiment* GetEXP(){ return fEXP; };
  Int_t GetExpCtrlMode(){ return fExpCtrlMode; }
  ARSocket_t* GetSocket(){ return fSocket; }
  Char_t* GetServerIP(){ return fServerIP; }
  Int_t GetPort(){ return fPort; }
  TDAQmodule* GetIRQMod(){ return fIRQMod; }
  virtual Char_t* GetTime(){ return fTime; }
  virtual Char_t* GetComment(){ return fComment; }
  virtual Char_t* GetFileName(){ return fFileName; }
  Char_t* GetCommandLine(){ return fCommandLine; } 
  Char_t* GetGUIBuffer(){ return fGUIBuffer; }
  TDAQguiCtrl* GetGUICtrl(){ return fGUICtrl; }
  void SetGUIBuffer(Char_t* b){ fGUIBuffer = b; }
  Char_t* GetOutBuff(){ return fOutBuff; }
  virtual Int_t GetRunNumber(){ return fRunNumber; }
  virtual void SetRunNumber(Int_t r){
    fRunNumber = r;
    SaveRunLog();
  }
  virtual void SetTimeOut(UInt_t tmout){ fTimeOut = tmout; }
  virtual void SetAuto( Bool_t state ){ fIsAuto = state; }
  virtual void SetTrigMod( TDAQmodule* trigmod ){ fTrigMod = trigmod; }
  virtual void SetIRQMod( TDAQmodule* irqmod ){ fIRQMod = irqmod; }
  virtual Bool_t IsDataReady(){ return kFALSE; }
  virtual Bool_t IsRunInit(){ return fIsRunInit; }
  Bool_t IsRunTerm(){ return fIsRunTerm; }
  Bool_t IsBufferDump(){ return fIsBufferDump; }
  Bool_t IsAuto(){ return fIsAuto; }
  Bool_t IsRunning(){ return fIsRunning; }
  Bool_t IsSlave(){
    if( fExpCtrlMode == EExpCtrlSlave ) return kTRUE;
    if( fExpCtrlMode == EExpCtrlNetSlave ) return kTRUE;
    else return kFALSE;
  }
  Bool_t IsGUI(){
    if(fExpCtrlMode == EExpCtrlGUILocal) return kTRUE;
    else return kFALSE;
  }
  //
  ClassDef(TDAQsupervise,1)   
    };


#endif
