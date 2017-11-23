//--Author	JRM Annand   30th Sep 2005  Start AcquDAQ
//--Rev 	JRM Annand... 1st Oct 2005  New DAQ software
//--Rev 	JRM Annand...10th Feb 2006  1st hardware test version
//--Rev 	JRM Annand....1st Feb 2007  Integrate with Acqu++
//--Rev 	JRM Annand...17th May 2007  Virtual Module
//--Rev 	JRM Annand...29th May 2007  StoreBuffer: ensure buff ptr reset
//--Rev 	JRM Annand....9th Jun 2007  AddModule SetIndex()
//--Rev 	JRM Annand....2nd Dec 2007  Link AcquRoot
//--Rev 	JRM Annand...11th Jan 2008  AddModule procedure, new hardware
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev         B Oussena    18th Mar 2010  Add modules for Fastbus
//--Rev         B Oussena    13th Jul 2010  Add LRS4413SPILL start/stop ctrl
//--Rev 	JRM Annand...19th Jul 2010..Mods for slave mode and start ctrl
//--Rev 	JRM Annand... 2nd Sep 2010..Check slave mode RunIRQ, Mk1/2 stuff
//--Rev 	JRM Annand... 5th Sep 2010..Event Synchronisation module
//--Rev 	JRM Annand... 6th Sep 2010..Start/trigger mod to superviser
//                                          Implement event ID synch stuff
//--Rev 	JRM Annand... 7th Sep 2010..Call ResetTrigCtrl end scaler read
//--Rev 	JRM Annand... 8th Sep 2010..CATCH list for general spy reset
//                                          after TCS reset
//--Rev 	JRM Annand... 9th Sep 2010..fStartSettleDelay after CATCH reset
//                                          implement PostReset(), add fCtrl
//--Rev 	JRM Annand...11th Sep 2010..Add class TCAMAC_4413DAQEnable
//                                          and ELRS_2323
//--Rev 	JRM Annand...12th Sep 2010..NetSlave superviser mode
//                                          Mk1/2 format scaler block
//--Rev 	JRM Annand... 5th Feb 2011..Flush text buffer StartExperiment
//                                          StoreBuffer() update
//--Rev 	JRM Annand...12th May 2011..ADC/Scaler error counters
//                                          Local GUI ctrl...part of main prog.
//--Rev 	JRM Annand... 6th Jul 2011..gcc4.6-x86_64 warning fix
//--Rev 	B.Oussena .. 24th Nov 2011  correct a bug in AddModule 
//                                          at add LRS2323 call
//--Rev         JRM Annand... 9th Jan 2012..Baya's bug find..check fNScaler
//--Rev         JRM Annand   25th Jan 2012  add GSI 4800 scaler
//--Rev         JRM Annand   19th May 2012  add VUPROM
//--Rev         JRM Annand   21st May 2012  StoreEvent() mod for nostore mode
//                                          Move TCS reset slow ctrl loop
//--Rev         JRM Annand    2nd Sep 2012  Add fEventSendMod..event ID send
//--Rev         B. Oussena    9th Nov 2012  sleep(1) after launching data store
//--Rev         JRM Annand    9th Jan 2013  add CAEN V874 TAPS module
//--Rev 	K Livingston..7th Feb 2013  Support for writing EPICS buffers
//--Rev 	JRM Annand    2nd Mar 2013  EPICS read in conditional block
//--Rev 	JRM Annand    6th Jul 2013  Add V965 QDC
//--Rev 	JRM Annand    3rd Sep 2013  Add VITEC interrupt/event-ID card
//--Rev 	JRM Annand    9th Sep 2013  Add event-ID master functionality
//--Rev 	JRM Annand   22nd Sep 2013  Add VUPROMT, remove SlowCtrl thread
//--Rev 	JRM Annand   24nd Sep 2013  Don't start ctrl thread if slave
//                                          End-run scaler read for slaves
//--Update	JRM Annand   29nd Mar 2014  Slave ev-ID read before scaler read
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQexperiment
// Container for a complete DAQ session.
// Lists of hardware, configurations methods
// Starts separate threads for event-driven readout, scaler readout
// Slow-control monitoring and control of the experiment

#include "TDAQexperiment.h"
#include "TAcquRoot.h"
#include "ModuleIndex.h"
#include "TDAQguiCtrl.h"
//
#include "TDAQ_V2718.h"
#include "TDAQ_KPhI686.h"
#include "TVME_CBD8210.h"
#include "TVME_V792.h"
#include "TVME_V775.h"
#include "TVME_V874.h"
#include "TVME_V1190.h"
#include "TCAMACmodule.h"
#include "TCAMAC_4508.h"
#include "TCAMAC_2373.h"
#include "TCAMAC_2323.h"  // <---  Nov 24th/11  Baya
#include "TCAMAC_4413SPILL.h" // < ---Jul 13/10 Baya
#include "TCAMAC_4413DAQEnable.h"
#include "TCAMAC_GSI4800.h"
#include "TVME_CATCH_TDC.h"
#include "TVME_CATCH_Scaler.h"
#include "TVME_CATCH_TCS.h"
#include "TVME_GeSiCA.h"
#include "TVME_KPhSMI.h"
#include "TFBmodule.h"
#include "TFB_1821SMI.h"
#include "TFB_LRS1800.h"
#include "TFB_STR200.h"
#include "TVirtualModule.h"
#include "TVME_VUPROM.h"
#include "TVME_VUPROMT.h"
#include "TVME_VUPROM_Scaler.h"
#include "TVME_VUPROM_Moeller.h"
#include "TVME_VUPROM_Pattern.h"
#include "TVME_SIS3820.h"
#include "TEPICSmodule.h"
#include "TVME_V965.h"
#include "TVME_VITEC.h"


// recognised setup keywords
enum { EExpModule, EExpControl, EExpIRQCtrl, EExpStartCtrl, EExpDescription,
       EExpDataOut, EExpEvCnt, EExpDesc, EExpFName, EExpRunStart,
       EExpLocalAR, EExpMk1DataFormat, EExpSynchCtrl, EExpResetCtrl,
       EExpEventIDSend, EExpEventIDMaster };
static Map_t kExpKeys[] = {
  {"Control:",       EExpControl},         // mode of operator control
  {"Module:",        EExpModule},          // add electronic module
  {"IRQ-Ctrl:",      EExpIRQCtrl},         // specify trigger control module
  {"Start-Ctrl:",    EExpStartCtrl},       // specify trigger control module
  {"Event-Counters:",EExpEvCnt},           // event counters
  {"Description:",   EExpDesc},            // DAQ description
  {"File-Name:",     EExpFName},           // file name basis
  {"Start-Run:",     EExpRunStart},        // start run number
  {"Local-AR:",      EExpLocalAR},         // special for GUI control
  {"Mk1-Format:",    EExpMk1DataFormat},   // switch to Mk1 data format
  {"Synch-Ctrl:",    EExpSynchCtrl},       // event-number synchronisation
  {"Reset-Ctrl:",    EExpResetCtrl},       // reset current control module
  {"EventID-Send:",  EExpEventIDSend},     // module to send event ID to remote
  {"EventID-Master:",EExpEventIDMaster},   // has control of the event ID
  {NULL,              -1}
};

//-----------------------------------------------------------------------------
TDAQexperiment::TDAQexperiment( const Char_t* name, const Char_t* input, const Char_t* log,
				TAcquRoot* ar, Int_t initLevel):
  TA2System( name, kExpKeys, input, log )
{
  // Base initialisation of DAQ object
  // Setup input config stream if file name supplied
  // Save log output stream....use standard out (tty) if not supplied
  //
  fAR = ar;
  fSupervise = NULL;
  fStore = NULL;
  fOutBuff = NULL;
  fEventBuff = NULL;
  fModuleList = new TList;
  fADCList = new TList;
  fScalerList = new TList;
  fCtrlList = new TList;
  fSlowCtrlList = new TList;
  fCATCHList = NULL;
  fEPICSList =  new TList;
  fEPICSTimedList =  new TList;
  fEPICSCountedList =  new TList;
  fIRQModName = fStartModName = fSynchModName = fEventSendModName = NULL;
  fIRQMod = fStartMod = fSynchMod = fCtrlMod = fEventSendMod = NULL;
  fSynchIndex = -1;
  fStartSettleDelay = 0;
  fNModule = fNADC = fNScaler = fNCtrl = fNSlowCtrl =
    fNEPICS = fNEPICSTimed = fNEPICSCounted = 0;
  fDataOutMode = EStoreDOUndef;
  fScReadFreq = 0;
  fSlCtrlFreq = 0;
  fRingSize = fRecLen = fNRec = fNRecMax = 0;
  fDataHeader = EMk2DataBuff;
  fIsSwap = fIsSlowCtrl = fIsCtrl = fIsStore = fIsLocalAR = 
    fIsEvIDMaster = kFALSE;
  fInitLevel = initLevel;
  fNADCError = fNScalerError = 0;
  // fIsTrigCtrl = kFALSE;
  // default strings
  strcpy( fRunDesc, "AcquDAQ Experimental Data, in MkII format\n" );
  strcpy( fFileName, "AcquDAQ" );
  fRunStart = -1;
  fIRQThread = 0;
  fDAQCtrlThread = 0;
}

//-----------------------------------------------------------------------------
TDAQexperiment::~TDAQexperiment()
{
  // Tidy up memory and opened files
  // incomplete...

  if( fLogStream ) fclose(fLogStream);
  if( fModuleList ){ 
    TIter next( fModuleList );
    TDAQmodule* mod;
    while( ( mod = (TDAQmodule*)next() ) ) delete mod;
    delete fModuleList;
  }
}

//-----------------------------------------------------------------------------
void TDAQexperiment::SetConfig(Char_t* line, Int_t key )
{
  // DAQ configuration from a line of text (file or tty)
  // The line is already parsed for a keyword, which sets key

  Char_t mode[32];
  switch( key ){
  case EExpControl:
    // Decide on mode of control...local keyboard, network etc.
    if( sscanf( line,"%s",mode ) < 1 ){
      PrintError( line,"<Parse superviser I/O mode>",EErrFatal);
    }
    if( !fSupervise ){
      if( !strcmp( mode, "GUI-Net" ) )
	fSupervise = new TGUIsupervise((Char_t*)"GUI-superviser",this,line);
      else
	fSupervise = new TDAQsupervise((Char_t*)"DAQ-superviser",this,line);
    }
    fIsCtrl = !(fSupervise->IsSlave()); // Control enabled if not a slave
    break;
  case EExpModule:
    // add hardware to the experiment
    AddModule(line);
    break;
  case EExpIRQCtrl:
    // Interrupt control module
    if( sscanf( line,"%s",mode ) < 1 ){
      PrintError( line,"<Parse IRQ module>",EErrFatal);
    }
    fIRQModName = BuildName( mode );
    break;
  case EExpStartCtrl:
    // Trigger start/stop control module
    if( sscanf( line,"%s%d",mode, &fStartSettleDelay ) < 2 ){
      PrintError( line,"<Parse Trigger-Start/Stop module>",EErrFatal);
    }
    fStartModName = BuildName( mode );
    break;
  case EExpSynchCtrl:
    // Event number synchronisation control module and synch value index
    if( sscanf( line,"%s%d",mode, &fSynchIndex ) < 2 ){
      PrintError( line,"<Parse Event Synch module & Index>",EErrFatal);
    }
    fSynchModName = BuildName( mode );
    break;
  case EExpResetCtrl:
    // Reset current control module...module must already be declared
    if( sscanf( line,"%s",mode ) < 1 ){
      PrintError( line,"<Parse reset current control module>",EErrFatal);
    }
    fCtrlMod = (TDAQmodule*)( fModuleList->FindObject( mode ) );
    if( !fCtrlMod )
      PrintError(fIRQModName,"<Reset control module hardware not found>",
		 EErrFatal);
    break;
  case EExpEventIDSend:
    // Module to send event ID
    if( sscanf( line,"%s",mode ) < 1 ){
      PrintError( line,"<Parse event ID send module>",EErrFatal);
    }
    fEventSendModName = BuildName( mode );
    break;
  case EExpEventIDMaster:
    // This class has control of the event ID, rather than extern hardware
    if( sscanf( line,"%d",&fSynchIndex ) < 1 ){
      PrintError( line,"<Parse Event Synch Index for Master>",EErrFatal);
    }
    fIsEvIDMaster = kTRUE;
    break;
  case EExpEvCnt:
    // event counters
    if( sscanf( line,"%d%d", &fScReadFreq,&fSlCtrlFreq ) < 2 ){
      PrintError( line,"<Parse Event Counters>",EErrFatal);
    }
    if( fSlCtrlFreq ) fIsSlowCtrl = kTRUE;
    break;
  case EExpDesc:
    // one-line description of the experiment
    for(Int_t i=0; i<EMk2SizeDesc; i++) fRunDesc[i] = 0;
    strcpy(fRunDesc, line);
    break;
  case EExpFName:
    // file name basis, assume data storage if this command given
    for(Int_t i=0; i<EMk2SizeFName; i++) fFileName[i] = 0;
    if( sscanf( line,"%s%d%d%d",fFileName, &fRingSize,
		&fNRecMax, &fRecLen  ) < 4 )
      PrintError(line, "<Data output file name not parsed>", EErrFatal);
    if( !fIsLocalAR )
      fStore = new TDAQstore((Char_t*)"DAQ-storage", NULL, fLogStream, line);
    fIsStore = fStore->IsStore();
    break;
  case EExpRunStart:
    // starting run number
    if( sscanf(line,"%d",&fRunStart) != 1 )
      PrintError(line,"<Parse start run number line>");
    break;
  case EExpLocalAR:
    // flag that DAQ is run as a thread of AcquRoot analyser
    fIsLocalAR = kTRUE;
    break;
  case EExpMk1DataFormat:
    // Switch data format to AcquRoot Mk1. Default is Mk2 AcquRoot
    fDataHeader = EDataBuff;
    break;
  default:
    PrintError(line,"<Unrecognised configuration keyword>");
    break;
  }
}

//-----------------------------------------------------------------------------
void TDAQexperiment::PostInit( )
{
  // Call PostInit procedures of all modules (if any)
  // Check for interrupt and trigger start/stop modules
  // Setup DAQ supervisor and data storage

  if( !fNModule ) return;
  if( fModuleList ){ 
    TIter next( fModuleList );
    TDAQmodule* mod;
    while( ( mod = (TDAQmodule*)next() ) ) mod->PostInit();
  }

if (fInitLevel != 2) //<<------------------------------- Baya
{

  // Interrupt controller
  if( !fIRQModName )
    PrintError("","<Trigger (IRQ) hardware not specified>",EErrFatal);
  fIRQMod = (TDAQmodule*)( fModuleList->FindObject( fIRQModName ) );
  if( !fIRQMod )
    PrintError(fIRQModName,"<Trigger (IRQ) hardware not found>",EErrFatal);
  // DAQ start/stop controller
  if( fStartModName ){
    fStartMod = (TDAQmodule*)( fModuleList->FindObject( fStartModName ) );
    if( !fStartMod )
      PrintError(fIRQModName,"<DAQ Start/Stop hardware not found>",EErrFatal);
  }
  // Check if DAQ synch is by a number supplied by external hardware
  if( fSynchModName ){
    fSynchMod = (TDAQmodule*)( fModuleList->FindObject( fSynchModName ) );
    if( !fSynchMod )
      PrintError(fIRQModName,"<DAQ Event Synchronisation hardware not found>",
		 EErrFatal);
  }
  // check if event ID has to be sent explicitly to a remote system
  if( fEventSendModName ){
    fEventSendMod =
      (TDAQmodule*)( fModuleList->FindObject( fEventSendModName ) );
    if( !fEventSendMod )
      PrintError(fIRQModName,"<DAQ Event ID send hardware not found>",
		 EErrFatal);
    if(fSynchMod) fSynchMod->SetEventSendMod( fEventSendMod );
  }
  // If this class determines the event ID number then there must be a
  // module defined to send it to the rest of the DAQ system
  // If fSynchMod has been defined then undefine it, as it will screw up
  // what the mast sends
  if( fIsEvIDMaster ){
    if( !fEventSendMod ){
      PrintError("TDAQexperiment","<EvID master: no Event ID send hardware>",
		 EErrFatal);
    }
    if( fSynchMod ){
      PrintError(fSynchModName,"<EVID master: fSynchMod set NULL>");
      fSynchMod = NULL;
    } 
  }    
  // DAQ supervisor....it may already be created...
  // if so ensure that it has trigger control (if this exists)
  // also ensure that it has the IRQ module
  if( !fSupervise ){
    fSupervise = new TDAQsupervise((Char_t*)"DAQ-superviser",this,
				   (Char_t*)"Local 0 0");
  }
  if( fStartMod ) fSupervise->SetTrigMod( fStartMod );
  if( fIRQMod ) fSupervise->SetIRQMod( fIRQMod );

  // Data storage
  if( fStore ) fStore->PostInit();
  fOutBuff = new TA2RingBuffer( fRecLen, fRingSize );
  if( fStore ) fStore->SetOutBuff( fOutBuff );
  fOutBuff->WaitEmpty();
  fEventBuff = new Char_t[fOutBuff->GetLenBuff()];
}   //<<------------------------------------------Baya

}

//-----------------------------------------------------------------------------
void TDAQexperiment::AddModule( Char_t* line )
{
  // Setup software control of particular piece of hardware
  // Add it to the experimental list
  Char_t module[128];
  Char_t name[128];
  Char_t file[128];
  if( sscanf( line, "%s%s%s", module, name, file ) != 3 ){
    PrintError( line, "<Parse hardware module>" );
    return;
  }
  Int_t key = Map2Key( module, (Map_t*)kExpModules );
  if( key == -1 ){
    PrintError( module, "<Unknown hardware module>" );
    return;
  }
  TDAQmodule* mod;
  switch( key ){
  case ECAEN_V2718:
    // PCI - VMEbus controller (primary controller)
    mod = new TDAQ_V2718( name, file, fLogStream, line);
    break;
  case EKPH_I686:
    // KPH, Pentium M based, SBC VMEbus controller (primary controller)
    mod = new TDAQ_KPhI686( name, file, fLogStream, line);
    break;
  case EVMEbus:
    // Generic VMEbus module
    mod = new TVMEmodule( name, file, fLogStream, line);
    break;
  case ECBD_8210:
    // VMEbus - CAMAC parallel branch driver (secondary controller)
    mod = new TVME_CBD8210( name, file, fLogStream, line );
    break;
  case EKPH_SMI: 
    // VMEbus to Fastbus SMI register map
    mod = new TVME_KPhSMI( name, file, fLogStream, line );
    break;
  case EKPH_VITEC: 
    // Interrupt and event-ID handler
    mod = new TVME_VITEC( name, file, fLogStream, line );
    break;
  case ECAEN_V792:
    // VMEbus - CAEN 32 channel QDC
    mod = new TVME_V792( name, file, fLogStream, line );
    break;
  case ECAEN_V965:
    // VMEbus - CAEN 16 channel dual-range QDC
    mod = new TVME_V965( name, file, fLogStream, line );
    break;
  case ECAEN_V775:
    // VMEbus - CAEN 32 channel TDC
    mod = new TVME_V775( name, file, fLogStream, line );
    break;
  case ECAEN_V874:
    // VMEbus - CAEN 4 channel TAPS module
    mod = new TVME_V874( name, file, fLogStream, line );
    break;
  case ECAEN_V1190:
    // VMEbus - CAEN 128 channel, multi-hit TDC
    mod = new TVME_V1190( name, file, fLogStream, line );
    break;
  case ECATCH_TDC:
    // VMEbus/CATCH - 128 channel, multi-hit TDC
    if( !fCATCHList ) fCATCHList = new TList();
    mod = new TVME_CATCH_TDC( name, file, fLogStream, line );
    fCATCHList->AddLast( mod );                 // add to list
    break;
  case ECATCH_SCA:
    // VMEbus/CATCH - 128 channel scaler
    if( !fCATCHList ) fCATCHList = new TList();
    mod = new TVME_CATCH_Scaler( name, file, fLogStream, line );
    fCATCHList->AddLast( mod );                 // add to list
    break;
  case ECATCH_TCS:
    // VMEbus/CATCH - Trigger Controller
    mod = new TVME_CATCH_TCS( name, file, fLogStream, line );
    break;
  case EGeSiCA:
    // VMEbus/CATCH - Trigger Controller
    mod = new TVME_GeSiCA( name, file, fLogStream, line );
    break;
  case EGSI_VUPROM:
    // VMEbus/CATCH - Trigger Controller
    mod = new TVME_VUPROM( name, file, fLogStream, line );
    break;
  case EGSI_VUPROMT:
    // VMEbus/CATCH - Trigger Controller (TAPS version)
    mod = new TVME_VUPROMT( name, file, fLogStream, line );
    break;
  case EGSI_VUPROM_Scaler:
    // VUPROM Simple scaler module
    mod = new TVME_VUPROM_Scaler( name, file, fLogStream, line );
    break;
  case EGSI_VUPROM_Moeller:
    // VUPROM Moeller module
    mod = new TVME_VUPROM_Moeller( name, file, fLogStream, line );
    break;
  case EGSI_VUPROM_Pattern:
    // VUPROM Simple Pattern module
    mod = new TVME_VUPROM_Pattern( name, file, fLogStream, line );
    break;  
  case ESIS_3820:
    // SIS 3820 I/O register and scaler
    mod = new TVME_SIS3820( name, file, fLogStream, line );
    break;
  case ECAMAC:
    // General CAMAC module. No specialist initialisation or read out
    mod = new TCAMACmodule( name, file, fLogStream, line );
    break;
  case ELRS_4508:
    // LeCroy 4508 MLU
    mod = new TCAMAC_4508( name, file, fLogStream, line );
    break;
  case ELRS_2373:
    // LeCroy 2373 MLU
    mod = new TCAMAC_2373( name, file, fLogStream, line );
    break;
  case ELRS_2323:
    // LeCroy 2323 Gate and Delay
    mod = new TCAMAC_2323( name, file, fLogStream, line ); //<<--- baya was TCAMAC_2373
    break;
  case ELRS_4413SPILL:
    // LeCroy 4413 discriminator...special for trigger start/stop operations
    mod = new TCAMAC_4413SPILL( name, file, fLogStream, line );
    break;   
  case ELRS_4413DAQEnable:
    // LeCroy 4413 discriminator...special for trigger start/stop operations
    mod = new TCAMAC_4413DAQEnable( name, file, fLogStream, line );
    break;
  case EGSI_4800:
    // GSI 4800 48 channel CAMAC scaler
    mod = new TCAMAC_GSI4800( name, file, fLogStream, line );
    break;
  case ELRS_1821:
    //  Fastbus- LRS1821 Segment Manager Interface
    mod = new TFB_1821SMI( name, file, fLogStream, line );
    break;
  case ELRS_1800:
    //  Fastbus- LRS18XX QDCs TDCs
    mod = new TFB_LRS1800( name, file, fLogStream, line );
    break;
    /*
      case ELRS_1875:
      //  Fastbus- LRS1875 TDC 
      mod = new TFB_LRS1875( name, file, fLogStream, line );
      break;
    */
  case ESTR_200:
    //  Fastbus- STRUCK 200 SCALER 
    mod = new TFB_STR200( name, file, fLogStream, line );
    break;
  case EDAQ_Virtual:
    // Virtual Module...input line contains type (ADC, scaler etc.)
    mod = new TVirtualModule( name, file, fLogStream, line );
    break;
  case EDAQ_Epics:
    // Epics Module...input line contains type (ADC, scaler etc.)
    if(!IsMk2Format()){   //Exit if trying to include EPICS with mk1 format.
      PrintError( module, "EPICS modules can only be used in Mk2 format",EErrFatal);
    }
    else{
      mod = new TEPICSmodule( name, file, fLogStream, line);
    }
    break;
    
  default:
    PrintError( module, "<Unknown hardware module>" );
    return;
  }

  // Save ID and index
  mod->SetID( key );                         // AcquDAQ ID
  mod->SetIndex( fNModule );                 // module list index
  mod->SetInitLevel( fInitLevel );           // level hardware initialisation
  mod->SetDAQexperiment( this );             // link back to this

  // Check if module if controller or interface
  // If not save pointer to its immediate controller
  if( mod->IsType( EDAQ_Ctrl ) ){ 
    fCtrlMod = mod;                            // primary controller
    fCtrlList->AddLast( mod );                 // add to list
    fNCtrl++;
  }
  if( mod->IsType( EDAQ_SCtrl ) ){             // secondary controller
    mod->SetCtrl( fCtrlMod );                  // save upper controller
    fCtrlMod = mod;                            // and reset self as ctrl
    fCtrlList->AddLast( mod );
    fNCtrl++;
  }
  else mod->SetCtrl( fCtrlMod );               // slave store immediate ctrl

  // Configure Module from file
  mod->FileConfig( file );                     // parameters from file
  if( mod->IsType( EDAQ_ADC ) ){               // ADC module
    fADCList->AddLast( mod );
    fNADC++;
  }
  if( mod->IsType( EDAQ_Scaler ) ){            // Scaler module
    fScalerList->AddLast( mod );
    fNScaler++;
  }
  if( mod->IsType( EDAQ_SlowCtrl ) ){          // Slow control module
    fSlowCtrlList->AddLast( mod );
    fNSlowCtrl++;
  }
  if( mod->GetType() == EDAQ_Epics){          // EPICS module
    fEPICSList->AddLast( mod );
    ((TEPICSmodule *)mod)->SetEpicsIndex(fNEPICS++);
    if( ((TEPICSmodule *)mod)->IsTimed()){
      fEPICSTimedList->AddLast( mod );
      fNEPICSTimed++;
    }
    else if(((TEPICSmodule *)mod)->IsCounted()){
      fEPICSCountedList->AddLast( mod );
      fNEPICSCounted++;
    }
  }
  fModuleList->AddLast( mod );                 // all modules
  fNModule++;
}

//---------------------------------------------------------------------------
void TDAQexperiment::StartExperiment()
{
  // Start the various threads which comprise the DAQ system running
  // but may produce an occasional message.
  // 1st data storage (if enabled)
  // 2nd slow-control functions (if enabled)
  // 3rd interrupt-driven readout (if enabled)
  // Then start the superviser command loop for control of the DAQ

  fflush(fLogStream);
  if( fNModule ){
    // Data storage
    if( fIsStore && (!fIsLocalAR) ){
      if( !fStore ){
	PrintError("","<Start data-store thread before TDAQstore initialised>");
	return;
      } 
      if( fStoreDataThread ){
	printf(" Warning...deleting old StoreDataThread and starting new one\n");
	fStoreDataThread->Delete();
      }
      fStoreDataThread = new TThread( "RunStoreDataThread",
				      (void(*) (void*))&(RunStoreDataThread),
				      (void*)this );
      fStoreDataThread->Run();
      sleep(1);                     // Inserted by Baya....otherwise seg. fault problems
    }
    
    // Slow-control procedures....removed 22/9/13...was doing nothing
    /*
    if( fIsSlowCtrl ){
      if( fSlowCtrlThread ){
	printf(" Warning...deleting old SlowCtrlThread and starting new one\n");
	fSlowCtrlThread->Delete();
      }
      fSlowCtrlThread = new TThread( "RunSlowCtrlThread",
				     (void(*) (void*))&(RunSlowCtrlThread),
				     (void*)this );
      fSlowCtrlThread->Run();
    }
    */
    
    // Interrupt-driven readout
    if( fIRQMod ){
      if( fIRQThread ){
	printf(" Warning...deleting old IRQThread and starting new one\n");
	fIRQThread->Delete();
      }
      fIRQThread = new TThread( "RunIRQThread",
				(void(*) (void*))&(RunIRQThread),
				(void*)this );
      fIRQThread->Run();
    }
  }
  // DAQ control command loop
  if( fSupervise->GetExpCtrlMode() == EExpCtrlGUILocal )
    fSupervise->CommandLoop();
  else if(fSupervise->GetExpCtrlMode() != EExpCtrlSlave){
    if( fDAQCtrlThread ){
      printf(" Warning...deleting old IRQThread and starting new one\n");
      fDAQCtrlThread->Delete();
    }
    fDAQCtrlThread = new TThread("RunDAQCtrlThread",
				 (void(*) (void*))&(RunDAQCtrlThread),
				 (void*)this );
    fDAQCtrlThread->Run();
  }
  return;
}

//---------------------------------------------------------------------------
void TDAQexperiment::RunIRQ()
{
  // Event-by-event readout of data
  // 1/9/10 if running slave mode call fSupervise->ExecAutoStart()
  //
  if( !(fNADC + fNScaler) ){
    printf("Warning: <RunIRQ: no IRQ modules loaded>\n");
  }
  TIter nexta( fADCList );
  TIter nexts( fScalerList );
  TIter nextc( fSlowCtrlList );
  TIter nexte( fEPICSList );
  TIter nextet( fEPICSTimedList );
  TIter nextec( fEPICSCountedList );

  TDAQmodule* mod;
  void* out;              // output buffer pointer
  Int_t scCnt = 0;
  Int_t slCtrlCnt = 0;
  Int_t* scLen;
  Bool_t scEpicsFlag = kFALSE;    //for epics to know it was a scaler read. 
  Int_t* evLen;                   // place to put event length
  UShort_t* evID;                 // place to put event ID info
  fIsRunTerm = kFALSE;            // ensure run terminated flag off
  // If in slave mode run the autostart procedure, bypass any local control
  if( (fSupervise->GetExpCtrlMode() == EExpCtrlSlave) ||
      (fSupervise->GetExpCtrlMode() == EExpCtrlNetSlave) )
    fSupervise->ExecAutoStart();
  UInt_t nevID = 0;
  UInt_t nevIDprev = 0;
  UInt_t readoutPatternOffset = fSynchMod ?
              fSynchMod->GetReadoutPatternOffset() :
              0;
  for( ; ; ){
    fIRQMod->WaitIRQ();
    out = fEventBuff;
    BuffStore(&out, fNEvent);
    // Mk2 data format only
    if( IsMk2Format() ){
      evLen = (Int_t*)out;
      out = evLen + 1;
    }
    // If this class controls the event ID
    // send it off before doing anything else
    // If end-of-run detected set msb of event ID
    if( fIsEvIDMaster ){
      if( fSupervise->IsRunTerm() && fIsStore ){   // end of run detected?
	fIsRunTerm = kTRUE;            // local end-of-run flag
	nevID |= EExpEvIDEnd;          // set the msb
	scCnt = fScReadFreq - 1;       // force a scaler read
      }
      fEventSendMod->SendEventID(nevID);
      evID = (UShort_t*)out;
      *evID = fSynchIndex; evID++;     // save event ID index
      *evID = nevID; evID++;           // save event ID number
      out = evID;
      nevID++;
      if(nevID > 0xffff) nevID = nevIDprev = 0;    // reset the event ID
    }
    // Space for synchronisation event ID (if this is defined)
    // and readout pattern
    else if( fSynchMod ){
      evID = (UShort_t*)out;
      if(readoutPatternOffset>0) {
        out = evID + 8; // four 16bit ADC indeces and values
      }
      else {
         out = evID + 2; // three 16bit ADC index and value
      }
    }
    scCnt++;
    slCtrlCnt++;
    nexta.Reset();
    while( ( mod = (TDAQmodule*)nexta() ) ) mod->ReadIRQ(&out);

    // Move slave event ID stuff here before scaler and EPICS read
    // so that detection of end-of-run in the slave event ID can trigger
    // a scaler read etc.
    // JRMA 29/3/14
    if( fSynchMod ){
      // event id
      *evID = fSynchIndex; evID++;
      nevID = fSynchMod->GetEventID();
      *evID = nevID; 
      // readout bitpattern (for TAPS only)
      if(readoutPatternOffset>0) {
        // move to the next 16bit word
        evID++;        
        UInt_t status = fSynchMod->GetReadoutPatternStatus(); 
        UInt_t readoutPattern = fSynchMod->GetReadoutPattern();
        *evID = readoutPatternOffset+fSynchIndex+0; evID++;
        *evID = status & 0xffff; evID++; // status reg
        *evID = readoutPatternOffset+fSynchIndex+1; evID++;
        *evID = readoutPattern & 0xffff; evID++; // lower 16bit
        *evID = readoutPatternOffset+fSynchIndex+2; evID++;
        *evID = (readoutPattern >> 16) & 0xffff; // upper 16 bit
        // evID++ not needed here, it's the last 16bit word
      }
      
      // Check if end-of-run marker supplied and data storage enabled
      if((nevID & EExpEvIDEnd) && fIsStore ){
	fIsRunTerm = kTRUE;
	scCnt = fScReadFreq;       // force a scaler read
      }
    }

    // Check if scaler read due. If so loop round linked list of scaler modules
    // If a trigger control module is defined run its ResetTrigCtrl method
    // after the scaler read is performed
    // JRMA add check on fNScaler 9/2/12
    if( (scCnt == fScReadFreq) && (fNScaler) ){
      BuffStore(&out, EScalerBuffer);  // scaler-buffer marker
      if( IsMk2Format() ){
	scLen = (Int_t*)out;                      // scaler-buffer size Mk2 only
	out = scLen + 1;                          // update buffer ptr
      }
      nexts.Reset();
      while( ( mod = (TDAQmodule*)nexts() ) ) mod->ReadIRQScaler(&out);
      // Scaler-block length & end of scaler block marker Mk2 data format only
      if( IsMk2Format() ){
	*scLen = (Char_t*)out - (Char_t*)scLen;   // save buffer size
	BuffStore(&out, EScalerBuffer);// scaler-end marker
      }
      scCnt = 0;                                  // reset scaler counter
      scEpicsFlag=kTRUE;   // flag for epics that this was a scaler read event
    }
    if( (slCtrlCnt == fSlCtrlFreq) && (fNSlowCtrl) ){
      nextc.Reset();
      while( ( mod = (TDAQmodule*)nextc() ) ) mod->ReadIRQSlow(&out);
      slCtrlCnt = 0;
      // If there is a TCS reset it. After that do any other necessary resetting
      // e.g. any CATCH TDCs or scaler must also have their spy buffers reset
      if( fStartMod ){
	fStartMod->ResetTrigCtrl(); // trigger control reset
      }
    }
    //------------------------------------------------------------------------
    // Start of EPICS stuff
    // Check if any timed EPICS reads due - a separate buffer for each 
    // If there's more than one due (unlikely but possible)
    // We'll only be here if it's Mk2 format, otherwise exited earlier fatally.
    // Enter this block only if there is some EPICS in the system
    if( fNEPICS ){
      //if 1st event, read all EPICS modules then start timers & counters
      if(fNEvent==0){
	nexte.Reset();
	while( ( mod = (TDAQmodule*)nexte() ) ){    // loop all epics modules
	  BuffStore(&out, EEPICSBuffer); // epics-buffer marker
	  ((TEPICSmodule*)mod)->WriteEPICS(&out);   // write 
	}
	//start any EPICS timers / counters
	nextet.Reset();                             // ones timed in ms
	while( ( mod = (TDAQmodule*)nextet() ) ){
	  ((TEPICSmodule*)mod)->Start();
	}
	nextec.Reset();                         // ones counted in scaler reads
	while( ( mod = (TDAQmodule*)nextec() ) ){
	  ((TEPICSmodule*)mod)->Start();
	}
      }	
      //
      nextet.Reset();
      while( ( mod = (TDAQmodule*)nextet() ) ){    // loop timed epics modules
	if(((TEPICSmodule*)mod)->IsTimedOut()){    // Check if read is due
	  BuffStore(&out, EEPICSBuffer);// epics-buffer marker
	  ((TEPICSmodule*)mod)->WriteEPICS(&out);  // write 
	  ((TEPICSmodule*)mod)->Start();           // restart the timer 
	}
      }
      //
      //Now if it was a scaler event, see in any scaler counted EPICS reads
      if( (scEpicsFlag) && (fNScaler) ){
	nextec.Reset();
	while( ( mod = (TDAQmodule*)nextec() ) ){
	  ((TEPICSmodule*)mod)->Count();             // increment EPICS counter
	  if(((TEPICSmodule*)mod)->IsCountedOut()){  // if counted out
	    BuffStore(&out, EEPICSBuffer);// epics-buffer marker
	    ((TEPICSmodule*)mod)->WriteEPICS(&out);  // write
	    ((TEPICSmodule*)mod)->Start();           // restart the counter
	  }
	}
	scEpicsFlag=kFALSE;                          //reset the flag
      }
    }
    //-end of EPICS block
    //-----------------------------------------------------------------------
    if( IsMk2Format() ) *evLen = (Char_t*)out - (Char_t*)evLen;
    // Event ID determined by external hardware...get the ID from that module
    // Move this block upstream JRMA 29/3/14
    /*
    if( fSynchMod ){
      // event id
      *evID = fSynchIndex; evID++;
      nevID = fSynchMod->GetEventID();
      *evID = nevID; 
      // readout bitpattern (for TAPS only)
      if(readoutPatternOffset>0) {
        // move to the next 16bit word
        evID++;        
        UInt_t status = fSynchMod->GetReadoutPatternStatus(); 
        UInt_t readoutPattern = fSynchMod->GetReadoutPattern();
        *evID = readoutPatternOffset+fSynchIndex+0; evID++;
        *evID = status & 0xffff; evID++; // status reg
        *evID = readoutPatternOffset+fSynchIndex+1; evID++;
        *evID = readoutPattern & 0xffff; evID++; // lower 16bit
        *evID = readoutPatternOffset+fSynchIndex+2; evID++;
        *evID = (readoutPattern >> 16) & 0xffff; // upper 16 bit
        // evID++ not needed here, it's the last 16bit word
      }
      // Check if end-of-run marker supplied and data storage enabled
      if((nevID & EExpEvIDEnd) && fIsStore ){
	fIsRunTerm = kTRUE;
	// if there are scaler on the system do a scaler read
	//
	//if(fNScaler){
	//  BuffStore(&out, EScalerBuffer);  // scaler-buffer marker
	//  if( IsMk2Format() ){
	//    scLen = (Int_t*)out;           // scaler-buffer size Mk2 only
	//    out = scLen + 1;               // update buffer ptr
	//  }
	//  nexts.Reset();
	//  while( ( mod = (TDAQmodule*)nexts() ) ) mod->ReadIRQScaler(&out);
	  // Scaler-block length & end of scaler block marker Mk2 data format
	//  if( IsMk2Format() ){
	//    *scLen = (Char_t*)out - (Char_t*)scLen;   // save buffer size
	//    BuffStore(&out, EScalerBuffer);           // scaler-end marker
	//  }
	//  scCnt = 0;                                  // reset scaler counter
	//  scEpicsFlag=kTRUE;// flag for epics that this was a scaler read event
	//	}
      }
    }
    */
    if(nevID>0 && nevID != (nevIDprev+1) )
      printf("nevID=%d  nevIDprev=%d\n",nevID,nevIDprev);
    nevIDprev = nevID;
    BuffStore(&out, EEndEvent);
    StoreEvent( (Char_t*)(out) );
    // Check file length
    // End run is greater than max number data buffers
    // Start new run if on automatic file change
    if( fNRec > fNRecMax ){
      fNRec = 0;
      // GUI controller
      if( fSupervise->IsGUI() ){
	fSupervise->GetGUICtrl()->AutoEnd();
	if( fSupervise->IsAuto() ) fSupervise->GetGUICtrl()->AutoRun();
      }
      // Text window controller
      else{
	fSupervise->ExecEnd();
	if( fSupervise->IsAuto() ) fSupervise->ExecRun();
      }
    }
    fNEvent++;
    fIRQMod->ResetIRQ();
  }
}

//---------------------------------------------------------------------------
void* RunIRQThread( void* exp )
{
  //  Threaded running of event-by-event data readout
  //  so that other tasks may be performed
  //  Check that exp points to a TDAQexperiment object 1st
  //  ADC/Scaler modules must already be loaded into TDAQexperiment

  if( !((TObject*)exp)->InheritsFrom("TDAQexperiment") ){
    TThread::Printf("<RunIRQ: TDAQexperiment class not supplied>\n");
    return NULL;
  }
  TDAQexperiment* ex = (TDAQexperiment*)exp;
  ex->RunIRQ();
  return NULL;
}

//---------------------------------------------------------------------------
void TDAQexperiment::RunSlowCtrl()
{
  // Event-by-event readout of data
  fIsSlowCtrl = kTRUE;
  TIter next( fSlowCtrlList );
  //  TDAQmodule* mod;
  //  void* out;              // output buffer pointer
  //  out = fOutBuff;
  for(;;){
    //    while( ( mod = (TDAQmodule*)next() ) ) mod->EventRead(out);
  }

}

//---------------------------------------------------------------------------
void* RunSlowCtrlThread( void* exp )
{
  //  Threaded running of event-by-event data readout
  //  so that other tasks may be performed
  //  Check that exp points to a TDAQexperiment object 1st
  //  ADC/Scaler modules must already be loaded into TDAQexperiment

  if( !((TObject*)exp)->InheritsFrom("TDAQexperiment") ){
    TThread::Printf("<RunSlowCtrl: TDAQexperiment class not supplied>\n");
    return NULL;
  }
  TDAQexperiment* ex = (TDAQexperiment*)exp;
  if( !(ex->GetNADC() + ex->GetNScaler()) ){
    TThread::Printf("<RunSlowCtrl: no SlowCtrl modules loaded>\n");
    return NULL;
  }
  ex->RunSlowCtrl();
  return NULL;
}

//---------------------------------------------------------------------------
void TDAQexperiment::RunDAQCtrl()
{
  // Command loop
  //  fIsCtrl = kTRUE;
  fSupervise->CommandLoop();
}

//---------------------------------------------------------------------------
void* RunDAQCtrlThread( void* exp )
{
  //  Threaded running of event-by-event data readout
  //  so that other tasks may be performed
  //  Check that exp points to a TDAQexperiment object 1st
  //  ADC/Scaler modules must already be loaded into TDAQexperiment

  if( !((TObject*)exp)->InheritsFrom("TDAQexperiment") ){
    TThread::Printf("<RunDAQCtrl: TDAQexperiment class not supplied>\n");
    return NULL;
  }
  TDAQexperiment* ex = (TDAQexperiment*)exp;
  ex->RunDAQCtrl();
  return NULL;
}

//---------------------------------------------------------------------------
void TDAQexperiment::RunStoreData()
{
  // Storage of data buffers.
  // Set flag for storage running
  fIsStore = kTRUE;
  fStore->Run();
}

//---------------------------------------------------------------------------
void* RunStoreDataThread( void* exp )
{
  //  Threaded running of data storage
  //  so that other tasks may be performed
  //  Check that exp points to a TDAQexperiment object 1st

  if( !((TObject*)exp)->InheritsFrom("TDAQexperiment") ){
    TThread::Printf("<RunStoreData: TDAQexperiment class not supplied>\n");
    return NULL;
  }
  TDAQexperiment* ex = (TDAQexperiment*)exp;
  ex->RunStoreData();
  return NULL;
}

//---------------------------------------------------------------------------
void TDAQexperiment::PostReset( )
{
  // Perform any necessary actions after a general reset of the system
  // e.g. reset of the trigger control system, or after starting run
  // Check 1st for CATCH stuff
  if( fCATCHList ){
    TIter nextC( fCATCHList );                    // iterate throught CATCH mods
    TDAQmodule* mod;
    while( (mod = (TDAQmodule*)nextC()) )
      ((TVME_CATCH*)mod)->SpyReset();             // reset spy buffer
    usleep(fStartSettleDelay);                    // settle time  
  }
  // anything else here
}

ClassImp(TDAQexperiment)
