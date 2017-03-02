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
//--Update      JRM Annand   24th Sep 2013 set fIRQMod from exp->PostInit()
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQsupervise
// Supervise and control operation of AcquDAQ
//

#include "TDAQsupervise.h"
#include "TDAQexperiment.h"
#include "TDAQmodule.h"
#include "ARSocket_t.h"
#include "ARFile_t.h"
#include "TDAQguiCtrl.h"
#include "TVME_CATCH_TCS.h"
#include "TVME_CBD8210.h"
#include "TVME_VUPROM.h"
#include "TVME_V874.h"

#include <time.h>

#include <sstream>


// Modes of entering control info to DAQ
static Map_t kExpCtrl[] = {
  {"ENet",        EExpCtrlNet },       // commands over ethernet
  {"GUI-Net",     EExpGUINet },        // local GUI to remote node
  {"Local",       EExpCtrlLocal },     // commands local console
  {"Slave",       EExpCtrlSlave },     // slave system, no local control
  {"ENetSlave",   EExpCtrlNetSlave },  // slave system, but with command link
  {"GUI-Local",   EExpCtrlGUILocal },  // GUI control from local machine
  {NULL,          EExpCtrlUndef }
};

// Input command keys
static Map_t kSuperviseKeys[] = {
  {"Start:",         ESupStart},
  {"Go:",            ESupGo},
  {"Run:",           ESupRun},
  {"Pause:",         ESupPause},
  {"Stop:",          ESupStop},
  {"End:",           ESupEnd},
  {"Kill:",          ESupKill},
  {"Dump:",          ESupDump},
  {"Experiment:",    ESupExperiment},
  {"Comment:",       ESupComm},
  {"File:",          ESupFile},
  {"RunNumber:",     ESupRunNumber},
  {"Auto:",          ESupAuto},
  {"NoAuto:",        ESupNoAuto},
  {"IsRunning:",     ESupIsRunning},
  {"ConfigTCS:",     ESupConfigTCS},
  {"SetTCSRunMode:", ESupSetTCSRunMode},  
  {"Camio:",         ESupCAMAC},
  {"VUPROM:",        ESupVUPROM},
  {"V874:",          ESupV874},
  {NULL,             -1}
};

//-----------------------------------------------------------------------------
TDAQsupervise::TDAQsupervise( Char_t* name,  TDAQexperiment* exp,
			      Char_t* line )
  : TA2System( name, kSuperviseKeys, NULL, exp->GetLogStream() )
{
  // Import the pointer to the experiment class and
  // ensure local variables marked unitialised
  // file by default is NULL...output directed to the terminal
  fEXP = exp;
  fGUICtrl = NULL;
  fCtrlBar = NULL;
  fExpCtrlMode = EExpCtrlUndef;
  fSocket = NULL;
  fServerIP = NULL;
  fPort = 0;
  fIRQMod = exp->GetIRQMod();
  fTrigMod = exp->GetStartMod();
  fGUIBuffer = NULL;
  fOutBuff = NULL;
  fIsRunInit = kFALSE;
  fIsRunTerm = kTRUE;
  fIsBufferDump = kFALSE;
  fIsAuto = kFALSE;
  if( (exp->IsCtrl()) && (!fIRQMod) )
    PrintError(name,"<DAQ control enabled, but no control hardware found>",
	       EErrFatal);
  fDataHeader = exp->GetDataHeader();
  GetSysTime();
  strcpy( fComment, "Please enter a comment (and hit return)\n" );
  SetInputMode(line);
}

//-----------------------------------------------------------------------------
TDAQsupervise::~TDAQsupervise( )
{
  // Clean up
  if( fCtrlBar ) delete fCtrlBar;
}

//-----------------------------------------------------------------------------
void TDAQsupervise::SetConfig( Char_t* line, Int_t key )
{
  // Command decode
  Int_t temp;
  Int_t camC,camN,camA,camF,camD;
  //  Char_t string[ELineSize];

  switch( key ){
  case ESupStart:
    // Start DAQ
    ExecStart();
    break;
  case ESupGo:
    // Start run nostore
    ExecGo();
    break;
  case ESupRun:
    // Start a new run
    temp = sscanf(line,"%d",&camD);
    if( temp < 1 ) camD = -1;
    ExecRun( camD );
    break;
  case ESupPause:
    // Pause DAQ
    ExecPause();
    break;
  case ESupStop:
    // Stop DAQ
    ExecStop();
    break;
  case ESupEnd:
    // End run
    ExecEnd();
    break;
  case ESupKill:
    // Kill DAQ
    ExecKill();
    break;
  case ESupDump:
    // Print out data buffer
    ExecBufferDump();
    break;
  case ESupExperiment:
    // Get/Set Experiment description
    if( strlen( line ) )
      sscanf(line, "%s", GetRunDesc());
    PutString( GetRunDesc() );
    break;
  case ESupComm:
    // Get/Set Comment on run
    if( strlen( line ) )
      sscanf(line, "%s", fComment);
    PutString( fComment );
    break;
  case ESupFile:
    // Get/Set Data file name
    if( strlen( line ) )
      sscanf(line, "%s", fFileName);
    PutString( fFileName );
    break;
  case ESupRunNumber:
    // Get/set run number
    if( strlen( line ) ){
      sscanf(line, "%d", &temp);
      fRunNumber = temp - 1;      // -1 as SaveRunLog() adds +1
    }
    SaveRunLog();
    sprintf( fCommandLine, "Run Number = %d", temp );
    PutString( fCommandLine );
    break;
  case ESupAuto:
    fIsAuto = kTRUE;
    strcpy( fCommandLine, "Automatic run changing enabled\n" );
    PutString( fCommandLine );
    break;
  case ESupNoAuto:
    fIsAuto = kFALSE;
    strcpy( fCommandLine, "Automatic run changing disabled\n" );
    PutString( fCommandLine );
    break;
  case ESupIsRunning:
    // Is a run in progress
    sprintf( fCommandLine, "%d", fIsRunInit );
    PutString( fCommandLine );
    break;
  case ESupConfigTCS:
    // Re-configure TCS
    ConfigTCS();
    break;
  case ESupSetTCSRunMode:
    // Set the RunMode bit of the TCS controller
    // 1=enabled (default), 0=disabled
    // note that the TCS controller only reacts on this
    // bit when an End-of-Spill / Begin-of-Spill procedure is
    // executed, this can be done by ResetTrigCtrl() by the VUPROM
    if( strlen( line ) ){
      sscanf(line, "%d", &temp);
      temp &= 0x1; // only one bit is interesting
      SetTCSRunMode(temp);
    }
    break;
  case ESupCAMAC:
    // Issue CAMAC CNAF
    temp = sscanf(line,"%d %d %d %d %d",&camC,&camN,&camA,&camF,&camD);
    if( temp < 4 )
      PutString( "Need C,N,A,F + Datum (if write command)" );
    else if(temp < 5) camD = 0;
    CAMAC_IO(camC,camN,camA,camF,camD);
    break;
  case ESupVUPROM:
    ConfigVUPROM(line);
    break;
  case ESupV874:
    ConfigV874(line);
    break;
  default:
    PutString("<Superviser: unrecognised command keyword>\n");
    break;
  }
}

//-----------------------------------------------------------------------------
void TDAQsupervise::SetInputMode( Char_t* line )
{
  // Setup I/O mode of superviser from info in line
  // If ethernet initialise communications socket
  //
  Char_t mode[16];
  Char_t ipaddr[128];

  if( sscanf( line,"%s%s%d",mode,ipaddr,&fPort ) < 3 ){
    PrintError( line,"<Parse superviser I/O mode>",EErrFatal);
  }
  fExpCtrlMode = Map2Key( mode, kExpCtrl );
  if( (fExpCtrlMode == EExpCtrlLocal) && (fEXP->IsLocalAR()) )
    fExpCtrlMode = EExpCtrlLocalAR;
  fServerIP = BuildName(ipaddr);
  return;
}

//---------------------------------------------------------------------------
void TDAQsupervise::GetString()
{
  // Zero the command-line buffer
  // Then read from input stream
  //
  for( Int_t i=0; i<ELineSize; i++ ) fCommandLine[i] = 0;
  switch( fExpCtrlMode ){
  case EExpCtrlLocalAR:
  case EExpCtrlGUILocal:
    // should not happen...local GUI enabled
    break;
  case EExpCtrlLocal:
  default:
    // local text console
    printf("\nAcquDAQ> ");
    fgets(fCommandLine, ELineSize, stdin);
    break;
  case EExpCtrlNet:
  case EExpCtrlNetSlave:
    // network, commands from remote node
    fSocket->ReadChunked( fCommandLine, ELineSize, ELineSize );
    break;
  }
}

//---------------------------------------------------------------------------
void TDAQsupervise::PutString( const Char_t* line )
{
  // Put message on output stream
  //  Int_t length;
  
  switch( fExpCtrlMode ){
  case EExpCtrlLocalAR:
  case EExpCtrlGUILocal:
    // GUI control output
    if( fGUIBuffer ){
      fGUICtrl->LoadMsgBuffer(line);
      //      fGUICtrl->Update();
    }
    fprintf( fLogStream, "%s", line );
    break;
  case EExpCtrlLocal:
  default:
    printf("%s", line);
    break;
  case EExpCtrlNet:
  case EExpCtrlNetSlave:
    //    length = strlen(line);
    fSocket->WriteChunked( (void*)line,ELineSize,ELineSize );
    break;
  }
}

//-----------------------------------------------------------------------------
void TDAQsupervise::CommandLoop()
{
  // Continuous loop...await and execute DAQ control/supervise command
  // Save pointer to IRQ/Trigger control hardware
  // Create data buffer space if not already done
  // Communication socket moved here from SetInputMode()
  //
  usleep(10000);               // short pause 10000 micro-sec
  if( !fOutBuff ){
    if( fEXP->GetOutBuff() )
      fOutBuff = new Char_t[fEXP->GetOutBuff()->GetLenBuff()];
  }

  if( (fExpCtrlMode == EExpCtrlNet) || (fExpCtrlMode == EExpGUINet) ||
      (fExpCtrlMode == EExpCtrlNetSlave) ){
    Char_t name[256];
    strcpy( name,this->fName);
    Char_t* sname = BuildName( name, "_Sk");
    if( (fExpCtrlMode == EExpCtrlNet) || (fExpCtrlMode == EExpCtrlNetSlave) )
      fSocket = new ARSocket_t(sname, fServerIP, fPort, ESkLocal, 0, 0, this);
    else{
      sleep(1);
      fSocket = new ARSocket_t(sname, fServerIP, fPort, ESkRemote,0, 0, this);
    }
    fSocket->Initialise();
    delete[] sname;
  }
  // Start the control GUI
  // TApplication sets up the GUI interface to X11
  if( fEXP->IsLocalAR() || (fExpCtrlMode == EExpCtrlGUILocal) ){
    Int_t napp = 2;
    Char_t* apps[] = {(Char_t*)"-l",(Char_t*)"-n"};
    TApplication theApp("fGUICtrl",&napp,apps);
    fGUICtrl = new TDAQguiCtrl( this, gClient->GetRoot() );
    fGUICtrl->Update();
    theApp.Run(kTRUE);
  }
  else{
    // Check slave mode
    // Send a header if slave
    if( (fExpCtrlMode == EExpCtrlSlave) || (fExpCtrlMode == EExpCtrlNetSlave) ){
      ExecAutoStart();
    }
    for(;;){
      GetString();
      CommandConfig(fCommandLine);
    } 
  }
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecAutoStart( )
{
  // Slave-mode running. Send write a single header record to the data buffer.
  // and enable the local DAQ and IRQ
  // 15/09/10 Check 1st if the local buffer has been created
  //
  if( !fOutBuff ){
    if( fEXP->GetOutBuff() )
      fOutBuff = new Char_t[fEXP->GetOutBuff()->GetLenBuff()];
  }
  GetSysTime();                                      // Current time
  CreateHeader(fOutBuff);                            // Create header buffer
  fEXP->StoreBuffer(fOutBuff);                       // and store it
  fEXP->GetOutBuff()->Header(fDataHeader);           // mark next buffer data
  fEXP->ZeroNEvent();                                // set event # to 0
  sleep(1);                                          // short pause
  fIsRunTerm = kFALSE;                               // not run finished
  fIsRunInit = kTRUE;                                // run is initialised
  DAQEnable();                                       // Set DAQ running
  if( fTrigMod ) fTrigMod->RunTrigCtrl();            // enable & start triggers
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecStart( )
{
  // Start the DAQ, but do not start a new run
  // Check slave mode, data storage, run already initialised
  // enable IRQ
  //
  if( !fEXP->IsCtrl() ){
    PutString("ExecStart: <Control not enabled for this DAQ component>\n");
    return;
  }
  if( fEXP->IsStore() ){
    // run already initialised?
    if( !fIsRunInit ){
      PutString("ExecStart: <Data storage enabled & run not started>\n");
      return;
    }
  }
  GetSysTime();
  DAQEnable();
  if( fTrigMod ) fTrigMod->EnableTrigCtrl();            // enable triggers
  PutString("Run restarted\n");
  return;
}
//-----------------------------------------------------------------------------
void TDAQsupervise::ExecGo( )
{
  // Start the DAQ, but do not start a new run
  // Similar to ExecStart, but calls any additional functions e.g. TCS
  // if defined in the system
  //
  if( !fEXP->IsCtrl() ){
    PutString("ExecGo: <Control not enabled for this DAQ component>\n");
    return;
  }
  //  if( fEXP->IsStore() ){
  // run already initialised?
  //    PutString("ExecGo: <This command only available if running nostore>\n");
  //    return;
  //}
  GetSysTime();
  DAQEnable();
  if( fTrigMod ) fTrigMod->RunTrigCtrl();  // enable triggers and any other
  PutString("DAQ started\n");
  return;
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecRun( Int_t rnum )
{
  // Start data taking run. Check 1st on DAQ status...
  // Control, storage, already initialised?
  // 
  // Check if control is enabled (slave?)
  if( !fEXP->IsCtrl() ){
    PutString("ExecRun: <Control not enabled for this DAQ component>\n");
    return;
  }
  // storage enabled?
  if( !fEXP->IsStore() ){
    PutString("ExecRun: <Data storage not enabled for this DAQ session>\n");
    return;
  }
  // Run initialised
  if( fIsRunInit || (!fIsRunTerm) ){
    PutString("ExecRun: <Run already started...need to End first>\n");
    return;
  }
  // Check GUI control
  if( fGUIBuffer ) fGUICtrl->UpdateStartBuff();

  // Build file name from Run #
  // Run # supplied from command line, or Run.log, or AcquDAQ.dat
  if( rnum >= 0 ) fRunNumber = rnum;
  else{
    ARFile_t runFile( "Run.log","r+", this );
    if( (fRunNumber = GetRunLog()) != -1 ){}
    else fRunNumber = fEXP->GetRunStart();
  }
  sprintf( fFileName,"%s%s%d%s",fEXP->GetFileName(),"_",fRunNumber,".dat" );
  //
  GetSysTime();                                      // Current time
  CreateHeader(fOutBuff);                            // Create header buffer
  fEXP->StoreBuffer(fOutBuff);                       // and store it
  fEXP->GetOutBuff()->Header(fDataHeader);           // mark next buffer data
  fEXP->ZeroNEvent();                                // set event # to 0
  sleep(1);                                          // short pause
  fIsRunTerm = kFALSE;                               // not run finished
  fIsRunInit = kTRUE;                                // run is initialised
  DAQEnable();                                       // Set DAQ running
  if( fTrigMod ) fTrigMod->RunTrigCtrl();            // start & enable triggers
  sprintf(fCommandLine, "Opening file %s at %s\n",
	  fFileName, fTime);
  SaveRunLog();
  PutString(fCommandLine);                           // return message
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecPause(  )
{
  // Pause DAQ..just disable trigger input
  //
  if( !fEXP->IsCtrl() ){
    PutString("ExecPause: <Control not enabled for this DAQ component>\n");
    return;
  }
  if( fTrigMod ) fTrigMod->DisableTrigCtrl();      // disable triggers
  DAQDisable();
  PutString("Run Paused\n");
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecStop(  )
{
  // Stop DAQ, used only in nostore mode
  // 
  if( !fEXP->IsCtrl() ){
    PutString("ExecStop: <Control not enabled for this DAQ component>\n");
    return;
  }
  //  if( fEXP->IsStore() ){
  // data storage enabled?
  //PutString("ExecStop: <This command only available if running nostore>\n");
  //    return;
  //  }
  if( fTrigMod ) fTrigMod->EndTrigCtrl();      // stop the DAQ system
  DAQDisable();
  PutString("DAQ stopped\n");
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecEnd(  )
{
  // Stop data taking run
  if( !fEXP->IsCtrl() ){
    PutString("ExecRun: <Control not enabled for this DAQ component>\n");
    return;
  }
  // storage enabled?
  if( !fEXP->IsStore() ){
    PutString("ExecRun: <Data storage not enabled for this DAQ session>\n");
    return;
  }
  // Run initialised
  if( !fIsRunInit || (fIsRunTerm) ){
    PutString("ExecRun: <Run not started...need to Run XXX first>\n");
    return;
  }
  fIsRunTerm = kTRUE;              // flag run finished
  while( !fEXP->IsRunTerm() ) usleep(1);  // wait for experiment
  if( fTrigMod ) fTrigMod->EndTrigCtrl(); // stop & disable triggers
  DAQDisable();                           // disable DAQ controller
  sleep(1);                               // pause allow expt flush buffer
  //
  GetSysTime();                           // Current time
  CreateHeader(fOutBuff, EEndBuff);       // Create end buffer
  //sleep(1);                             // pause
  fEXP->StoreBuffer(fOutBuff);            // and store it
  //  sleep(1);                           // short pause
  //fIsRunTerm = kTRUE;              // not run finished
  fIsRunInit = kFALSE;             // run is initialised
  sprintf(fCommandLine, "Closing file %s at %s\n",
	  fFileName, fTime);
  PutString(fCommandLine);         // output message
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecKill(  )
{
  // Kill DAQ...this is allowed even if DAQ control is not enabled
  // Control is enabled send shutdown message to Data receiver
  if( !fIsRunInit || (fIsRunTerm) ){        // check if runs already ended
    if( fTrigMod ) fTrigMod->EndTrigCtrl(); // stop & disable triggers
    DAQDisable();                           // disable DAQ controller
  }
  if( fEXP->IsCtrl() && fEXP->IsStore() ){
    // send kill buffer
    CreateHeader(fOutBuff, EKillBuff);// Create header buffer
    fEXP->StoreBuffer(fOutBuff);      // and store it
  }
  PutString("Shutting down DAQ\n");
  if(fGUICtrl) delete fGUICtrl;             // wipe the GUI
  sleep(1);
  exit(0);
}

//-----------------------------------------------------------------------------
void TDAQsupervise::ExecBufferDump(  )
{
  // Print out the current data buffer to a text file
  //
  fIsBufferDump = kTRUE;
  if( fTrigMod ) fTrigMod->DisableTrigCtrl();
  DAQDisable();
  // Output file
  ARFile_t dmp("./DataDump.txt","w+",this,kFALSE);
  FILE* fd = dmp.GetStart();
  UInt_t* datum = (UInt_t*)(fEXP->GetOutBuff()->GetStore());
  fprintf(fd,"Data Buffer Header: %x (hex)\n",*datum++);
  // loop round all events in buffer
  for(;;){
    if( *datum == EEndEvent ) break;           // end of buffer exit
    fprintf(fd,"\nEvent: %d\n",*datum++);      // event number
    if( fEXP->IsMk2Format() )                    // event size Mk2 only
      fprintf(fd,"Event size: %u\n",*datum++);
    fprintf(fd,"  Index  Value  Index  Value  Index  Value  Index  Value\n");
    Int_t nl = 0;
    // loop round single event in buffer
    for(;;){
      if( *datum == EEndEvent ){ datum++;  break; }   // end of event exit
      switch (*datum) {
      default:
	// adc value and index
	fprintf(fd,"%7u%7u",*datum & 0xffff, *datum>>16);
	nl++;
	datum++;
	if(nl >= 4){ fprintf(fd,"\n"); nl = 0; }
	break;
      case EScalerBuffer:
	// Scaler read
	nl = 0;
	fprintf(fd,"\nScaler Dump Header:%x\n",*datum++);
	// Mk2 format scalers...have indices
	if( fEXP->IsMk2Format() ){
	  Int_t n = *datum;
	  fprintf(fd,"Scaler buffer size: %d bytes\n",n);
	  datum++;
	  fprintf(fd," Index  Value  Index  Value  Index  Value  Index  Value\n");
	  for( ;; ){
	    if( *datum == EScalerBuffer ) break;    // end of scaler block
	    if( *datum == EReadError ){             // read error
	      ReadErrorMk2_t* eb = (ReadErrorMk2_t*)datum;
	      fprintf(fd,"\nRead Error: Index %d; ID %d; Error Code %d\n",
		      eb->fModIndex, eb->fModID, eb->fErrCode);
	      datum = (UInt_t*)(eb + 1);
	      break;
	    }
	    fprintf(fd,"%5u%9u",*datum,*(datum+1));
	    datum += 2;
	    nl++;
	    if(nl >= 4){ nl = 0; fprintf(fd,"\n"); }
	  }
	  fprintf(fd,"Scaler trailer word: %x\n",*datum);
	  datum++;
	}
	// Mk1 format no scaler indices
	else{
	  for( ;; ){
	    if( *datum == EEndEvent ) break;// end of scaler block = event end
	    if( *datum == EReadError ){     // check read error
	      ReadError_t* eb = (ReadError_t*)datum;
	      fprintf(fd,"\nRead Error: Index %d; ID %d; Error Code %d\n",
		      eb->fBus, eb->fCrate, eb->fCode);
	      datum = (UInt_t*)(eb + 1);
	      break;
	    }
	    fprintf(fd,"%12u",*datum);
	    datum++;
	    nl++;
	    if(nl >= 4){ nl = 0; fprintf(fd,"\n"); }
	  }
	}
	break;
      case EReadError:
	// Decode error block
	// Mk2 data format
	if( fEXP->IsMk2Format() ){
	  ReadErrorMk2_t* eb = (ReadErrorMk2_t*)datum;
	  fprintf(fd,"\nRead Error: Index %d; ID %d; Error Code %d\n",
		  eb->fModIndex, eb->fModID, eb->fErrCode);
	  datum = (UInt_t*)(eb + 1);
	}
	// Mk1 data format
	else{
	  ReadError_t* eb = (ReadError_t*)datum;
	  fprintf(fd,"\nRead Error: Index %d; ID %d; Error Code %d\n",
		  eb->fBus, eb->fCrate, eb->fCode);
	  datum = (UInt_t*)(eb + 1);
	}
	break;
      }
    }
  }
  DAQEnable();
  if( fTrigMod ) fTrigMod->EnableTrigCtrl();
  fIsBufferDump = kFALSE;
  PutString("Buffer dumped in file ./DataDump.txt\n");
}

//-----------------------------------------------------------------------------
void TDAQsupervise::DAQEnable( )
{
  // Enable triggers
  fIsRunTerm = kFALSE;
  //  fEXP->SetIRQEnable();
  fIRQMod->EnableIRQ();
  fIsRunning = kTRUE;
}

//-----------------------------------------------------------------------------
void TDAQsupervise::DAQDisable( )
{
  // Disable triggers
  //
  fIRQMod->DisableIRQ();
  fIsRunning = kFALSE;
}

//-----------------------------------------------------------------------------
void TDAQsupervise::GetSysTime()
{
  // Store and print the system time in ascii format
  //
  time_t tick = time(NULL);			// # sec. since 1/1/70
  strcpy( fTime, asctime( localtime(&tick) ) );
  sprintf( fCommandLine,"Superviser system time: %s", fTime );
  fputs( fCommandLine, fLogStream );
  //  PutString( fCommandLine );
  return;
}

//-----------------------------------------------------------------------------
void TDAQsupervise::CreateHeader( void* buff )
{
  // Create header buffer from information held in
  // TDAQexperiment and TDAQsupervise classes
  // Use Acqu Mk2 format, unless format explicitly set to Mk1

  if ( !fEXP->IsMk2Format() ){
    CreateMk1Header(buff);
    return;
  }
  UInt_t* header = (UInt_t*)buff;
  *header++ = EHeadBuff; 
  // Header info struct
  AcquMk2Info_t* h = (AcquMk2Info_t*)header;
  // Module info struct
  ModuleInfoMk2_t* m = (ModuleInfoMk2_t*)(h + 1);

  h->fMk2 = EHeadBuff;                            // this shows its Mk2
  strcpy( h->fTime, fTime );                      // start time ascii
  strcpy( h->fRunNote, fComment );                // particular run comment
  strcpy( h->fOutFile, fFileName );               // name of data file
  strcpy( h->fDescription, fEXP->GetRunDesc() );  // experiment description
  h->fRun = fRunNumber;                           // run number
  h->fNModule = fEXP->GetNModule();               // # DAQ modules
  h->fNADCModule = fEXP->GetNADC();               // # ADC modules
  h->fNScalerModule = fEXP->GetNScaler();         // # Scaler modules
  h->fRecLen = fEXP->GetOutBuff()->GetLenBuff();  // Data record length
  // Just in case these get initialised with crap
  h->fNADC = 0;
  h->fNScaler = 0;
  //
  TIter nextm( fEXP->GetModuleList() );           // list of modules
  //  TIter nexta( fEXP->GetADCList() );              // list of ADCs
  //  TIter nexts( fEXP->GetScalerList() );           // list of scalers
  TDAQmodule* mod;
  Int_t maxadc = 0;
  Int_t maxscaler = 0;
  while( ( mod = (TDAQmodule*)nextm() ) ){
    mod->ReadHeader(m);
    m++;
    // If its an ADC find the maximum range of ADC indices
    if( mod->IsType(EDAQ_ADC) || mod->IsType(EDAQ_VADC) ){
      maxadc = mod->GetBaseIndex() + mod->GetNChannel();
      if( h->fNADC < maxadc ) h->fNADC = maxadc;    // max ADC index
    }
    // If its a scaler find the maximum range of Scaler indices
    if( mod->IsType(EDAQ_Scaler) || mod->IsType(EDAQ_VScaler) ){
      maxscaler = mod->GetBaseIndex() + mod->GetNChannel();
      if( h->fNScaler < maxscaler ) h->fNScaler = maxscaler;// max Scaler index
    }
  }
}

//-----------------------------------------------------------------------------
void TDAQsupervise::CreateMk1Header( void* buff )
{
  // Create header buffer from information held in
  // TDAQexperiment and TDAQsupervise classes
  // Use Acqu Mk1 format

  UInt_t* header = (UInt_t*)buff;
  *header++ = EHeadBuff; 
  // Header info struct
  AcquExptInfo_t* h = (AcquExptInfo_t*)header;

  strcpy( h->fTime, fTime );                      // start time ascii
  strcpy( h->fRunNote, fComment );                // particular run comment
  strcpy( h->fOutFile, fFileName );               // name of data file
  strcpy( h->fDescription, fEXP->GetRunDesc() );  // experiment description
  h->fRun = fRunNumber;                           // run number
  h->fNmodule = fEXP->GetNModule();               // # DAQ modules
  h->fRecLen = fEXP->GetOutBuff()->GetLenBuff();  // Data record length
  //
  TIter nextm( fEXP->GetModuleList() );           // list of modules
  TDAQmodule* mod;
  Int_t maxadc = 0;
  Int_t tempMax = 0;
  Int_t maxscaler = 0;
  while( ( mod = (TDAQmodule*)nextm() ) ){
    // If its an ADC find the maximum range of ADC indices
    if( mod->IsType( EDAQ_ADC ) || mod->IsType( EDAQ_VADC ) ){
      if( mod->GetNChannel() ){
	tempMax = mod->GetBaseIndex() + mod->GetNChannel();
      }
    }
    if( tempMax > maxadc ) maxadc = tempMax;
    // If its a scaler find the maximum range of Scaler indices
    if( mod->IsType( EDAQ_Scaler ) || mod->IsType( EDAQ_VScaler) ){
      if( mod->GetNChannel() ){
	maxscaler += mod->GetNChannel();
      }
    }
  }
  h->fNspect = maxadc;              // max ADC index
  h->fNscaler = maxscaler;          // max Scaler index
  ADCInfo_t* padc = (ADCInfo_t*)(h + 1);
  for(Int_t i=0; i<maxadc; i++) padc[i].fModIndex = 0xffff; // initialise off
  ADCInfo_t* psca = padc + maxadc;
  ModuleInfo_t* m = (ModuleInfo_t*)(psca + maxscaler);
  nextm.Reset();                   // return to start of module list
  Int_t imod = 0;
  Int_t nsca = 0;
  // loop round all modules
  while( ( mod = (TDAQmodule*)nextm() ) ){
    mod->ReadHeader(m);            // save module info in Mk1 module header
    if( mod->IsType( EDAQ_ADC ) || mod->IsType( EDAQ_VADC ) ){
      if( mod->GetNChannel() ){
	for( Int_t j = m->fAmin,k=0; j <= m->fAmax; j++,k++ ){
	  padc[j].fModIndex = imod;
	  padc[j].fModSubAddr = k;
	}
      }
    }
    else if( mod->IsType( EDAQ_Scaler ) || mod->IsType( EDAQ_VScaler) ){
      if( mod->GetNChannel() ){
	for( Int_t j = m->fAmin,k=0; j <= m->fAmax; j++,k++ ){
	  psca[nsca].fModIndex = imod;
	  psca[nsca].fModSubAddr = k;
	  nsca++;
	}
      }
    }
    m++;
    imod++;
  }
}

//-----------------------------------------------------------------------------
void TDAQsupervise::CreateHeader( void* buff, UInt_t value )
{
  // Send End buffer to storage
  UInt_t* header = (UInt_t*)buff;
  *header++ = value;
  // Mk2 format repeat the operation
  if( fDataHeader == EMk2DataBuff ) *header++ = value;
}

//----------------------------------------------------------------------------
Char_t* TDAQsupervise::GetRunDesc()
{ 
  // Description of experiment
  return fEXP->GetRunDesc();
}

//----------------------------------------------------------------------------
void TDAQsupervise::SaveRunLog()
{ 
  // Save run number to log file
  ARFile_t r( "Run.log","w+", this );
  fprintf(r.GetStart(), "%d\n", fRunNumber+1); // save next run #
  return;  
}

//----------------------------------------------------------------------------
Int_t TDAQsupervise::GetRunLog()
{ 
  // Get run number from log file
  // Check if the file exists. Failure to get run number returns -1
  //
  Int_t run = -1;
  ARFile_t* r = new ARFile_t( "Run.log","r+", this, EFalse );
  if( !r->GetStart() ){
    delete r;
    r = new ARFile_t( "Run.log","w+", this );
  }
  else{
    Char_t* l = r->ReadLine();
    if( l )
      if( sscanf( l, "%d", &run ) < 1 ) run = -1; 
  }
  delete r;
  return run;
}

//----------------------------------------------------------------------------
void TDAQsupervise::ConfigTCS()
{
  // Run the re-configuration procedure of the Trigger Control System
  // if it exists
  TList* modList = fEXP->GetModuleList();
  TDAQmodule* mod;
  if( !modList ){
    PutString(" No list of experimental modules found\n");
    return;
  }
  TIter nextM( modList );
  while( (mod = (TDAQmodule*)nextM()) ){
    if( mod->InheritsFrom("TVME_CATCH_TCS") ){
      ((TVME_CATCH_TCS*)mod)->ReConfig();
      PutString(" TCS reconfigured\n");         // output message
      return;
    }
  }
  PutString(" No TCS Module found\n");         // output message
  return;
}

//----------------------------------------------------------------------------
void TDAQsupervise::SetTCSRunMode(UInt_t runmode)
{
  // Run the re-configuration procedure of the Trigger Control System
  // if it exists
  TList* modList = fEXP->GetModuleList();
  TDAQmodule* mod;
  if( !modList ){
    PutString(" No list of experimental modules found\n");
    return;
  }
  TIter nextM( modList );
  while( (mod = (TDAQmodule*)nextM()) ){
    if( mod->InheritsFrom("TVME_CATCH_TCS") ){
      ((TVME_CATCH_TCS*)mod)->SetRunMode(runmode);
      std::stringstream ss;
      ss << " Set TCS runmode to " << runmode << std::endl;
      PutString(ss.str().c_str());         // output message
      return;
    }
  }
  PutString(" No TCS Module found\n");         // output message
  return;
}

//----------------------------------------------------------------------------
void TDAQsupervise::CAMAC_IO(Int_t C, Int_t N, Int_t A, Int_t F, Int_t datum)
{
  // Execute a CAMAC CNAF
  // Return the datum (read or written) and the state of Q and X
  //
  TList* modList = fEXP->GetModuleList();
  void* addr;
  UShort_t datum1, datum2;
  TVME_CBD8210* mod;
  if( !modList ){
    PutString(" No list of experimental modules found\n");
    return;
  }
  TIter nextM( modList );
  while( (mod = (TVME_CBD8210*)nextM()) ){
    if( mod->InheritsFrom("TVME_CBD8210") ){
      addr = (Char_t*)mod->AddrS(C,N,A,F) + mod->GetVirtOffset();
      if( (F >= 16) && (F <= 20) ){
	mod->Write(addr, &datum);
	datum1 = datum;
      }
      else
	mod->Read(addr,&datum1);
      datum2 = mod->Read(0);
      sprintf( fCommandLine,"Datum = %d, X = %d, Q = %d\n",
	       datum1,(datum2 & 0x4000)>>14,(datum2 & 0x8000)>>15 ); 
      PutString( fCommandLine );         // output message
      return;
    }
  }
  PutString(" No CAMAC Branch Driver Module found\n");      // output message
  return;
}

//----------------------------------------------------------------------------
void TDAQsupervise::ConfigVUPROM(Char_t* line)
{
  // Issue commands to VUPROM
  // if it exists
  TList* modList = fEXP->GetModuleList();
  TDAQmodule* mod;
  if( !modList ){
    PutString(" No list of experimental modules found\n");
    return;
  }
  TIter nextM( modList );
  while( (mod = (TDAQmodule*)nextM()) ){
    if( mod->InheritsFrom("TVME_VUPROM") ){
      ((TVME_VUPROM*)mod)->CmdExe(line);
      PutString(((TVME_VUPROM*)mod)->GetCommandReply());
      return;
    }
  }
  PutString(" No VUPROM Module found\n");         // output message
  return;
}

//----------------------------------------------------------------------------
Int_t* TDAQsupervise::GetVUPROMparm(Int_t chan)
{
  // Get parameters in integer array from VUPROM
  // return NULL if no VUPROM
  TList* modList = fEXP->GetModuleList();
  TDAQmodule* mod;
  if( !modList )return NULL;
  TIter nextM( modList );
  while( (mod = (TDAQmodule*)nextM()) ){
    if( mod->InheritsFrom("TVME_VUPROM") ){
      return ((TVME_VUPROM*)mod)->GetParms(chan);
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
void TDAQsupervise::ConfigV874(Char_t* line)
{
  // Issue commands to VUPROM
  // if it exists
  Char_t nmod[32];
  sscanf(line,"%s",nmod);
  Char_t* line1 = line + strlen(nmod) + 1;
  TList* modList = fEXP->GetModuleList();
  TDAQmodule* mod;
  if( !modList ){
    PutString(" No list of experimental modules found\n");
    return;
  }
  TIter nextM( modList );
  while( (mod = (TDAQmodule*)nextM()) ){
    const Char_t* name = mod->GetName();
    if(strcmp(name,nmod)) continue;         // check name of module
    if( mod->InheritsFrom("TVME_V874") ){
      ((TVME_V874*)mod)->CmdExe(line1);
      PutString(((TVME_V874*)mod)->GetCommandReply());
      return;
    }
  }
  PutString(" No V874 Module found\n");         // output message
  return;
}
ClassImp(TDAQsupervise)
