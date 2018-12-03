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
//--Rev 	K Livingston  7th Feb 2013  Support for writing EPICS buffers
//--Rev 	JRM Annand    2nd Mar 2013  EPICS read in conditional block
//--Rev 	JRM Annand    6th Jul 2013  Add V965 QDC
//--Rev 	JRM Annand    3rd Sep 2013  Add VITEC interrupt/event-ID card
//--Rev 	JRM Annand    9th Sep 2013  Add event-ID master functionality
//--Rev 	JRM Annand   22nd Sep 2013  Add VUPROMT, remove SlowCtrl thread
//--Rev 	JRM Annand   24nd Sep 2013  Don't start ctrl thread if slave
//                                          End-run scaler read for slaves
//--Rev 	JRM Annand   29nd Mar 2014  Slave ev-ID read before scaler read
//--Rev 	JRM Annand   14th Oct 2016  Add TDAQ_VP2ExX86_64
//--Rev 	JRM Annand    3rd Nov 2016  Add TVME_V785
//--Update	JRM Annand   19th Oct 2017  Ensure add TVME_V1290,TDAQ_SIS1100
//--Rev         K Livingston 20th Jun 2018  Fixed bug to allow proper timed periodic EPICS buffers
//--Update      K Livingston  3rd Jul 2018  Added EPICS trigger mode to write EPICS buffer triggered by ADC value.
//                                          Using AND,OR, threshold or window of ADC[x].datum with value y. 
//                                          Where x,y specified in setup line.
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

#ifndef __TDAQexperiment_h__
#define __TDAQexperiment_h__

#include "TA2System.h"
#include "TThread.h"                 // for threaded event loop
#include "TGUIsupervise.h"
#include "TDAQstore.h"
#include "TA2Mk2Format.h"
#include "TA2Mk1Format.h"

class TAcquRoot;

void* RunIRQThread( void* );         // per-event triggered data readout
void* RunSlowCtrlThread( void* );    // periodic slow-control procedures
void* RunDAQCtrlThread( void* );     // control of DAQ
void* RunStoreDataThread( void* );   // data storage

class TDAQmodule;

enum { EExpInit0, EExpInit1, EExpInit2, EExpInit3 };
enum { EExpEvIDEnd = 0x80000000, EExpEvIDStop = 0x40000000 };

class TDAQexperiment : public TA2System {
 protected:
  TAcquRoot* fAR;                   // AcquRoot analyser
  TThread* fIRQThread;              // per-event readout as separate thread
  TThread* fSlowCtrlThread;         // slow-control as separate thread
  TThread* fDAQCtrlThread;          // control input as separate thread
  TThread* fStoreDataThread;        // data storage as separate thread
  TDAQsupervise* fSupervise;        // DAQ system supervision
  TDAQstore* fStore;                // data storage
  TA2RingBuffer* fOutBuff;          // output data buffer
  Char_t* fEventBuff;               // holds a single event
  TList* fModuleList;               // linked list all DAQ modules
  TList* fADCList;                  // linked list all ADCs
  TList* fScalerList;               // linked list all scalers
  TList* fCtrlList;                 // linked list of all hardware controllers
  TList* fSlowCtrlList;             // linked list of all slow control 
  TList* fCATCHList;                // linked list of all CATCH modules
  TList* fEPICSList;                // linked list of all EPICS modules
  TList* fEPICSTimedList;           // EPICS modules with period in time (mn)
  TList* fEPICSCountedList;         // EPICS mod with period in scaler counts
  TList* fEPICSTriggeredList;       // EPICS mod dependent on a trigger
  Char_t* fIRQModName;              // name of Interrupt control module
  Char_t* fStartModName;            // name of Trigger start/stop module
  Char_t* fSynchModName;            // name of Event synchronisation module
  Char_t* fEventSendModName;        // name of module which sends event ID
  Char_t fRunDesc[EMk2SizeDesc];    // run description
  Char_t fFileName[EMk2SizeFName];  // file name basis
  TDAQmodule* fIRQMod;              // -> IRQ control hardware
  TDAQmodule* fStartMod;            // -> trigger start/stop hardware
  TDAQmodule* fSynchMod;            // -> event synchronisation hardware
  TDAQmodule* fCtrlMod;             // current control module
  TDAQmodule* fEventSendMod;        // module to send event ID to remote
  Int_t fSynchIndex;                // "adc index" of synchronisation value
  Int_t fStartSettleDelay;          // delay usec after any TCS/CATCH reset
  Int_t fRunStart;                  // starting run number
  Int_t fNModule;                   // total # hardware modules
  Int_t fNADC;                      // # ADCs
  Int_t fNScaler;                   // # Scalers
  Int_t fNCtrl;                     // # controllers
  Int_t fNSlowCtrl;                 // # slow control "modules"
  Int_t fNEPICS;                    // # All EPICS "modules"
  Int_t fNEPICSTimed;               // # No of EPICS "modules" periodic readout ms
  Int_t fNEPICSCounted;             // # No of EPICS "modules" periodic readout scaler 
  Int_t fNEPICSTriggered;           // # No of EPICS "modules" periodic readout triggered
  Int_t fDataOutMode;               // mode of data output
  Int_t fScReadFreq;                // scaler read frequency (events)
  Int_t fSlCtrlFreq;                // slow control scan frequency (events)
  Int_t fRingSize;                  // # elements in ring buffer
  Int_t fRecLen;	   	    // no. of bytes in record
  Int_t fNRecMax;                   // max # records in file
  Int_t fNRec;                      // # records in file
  Int_t fNADCError;                 // # detected ADC errors
  Int_t fNScalerError;              // # detected scaler errors
  UInt_t fNEventMax;                // maximum number of events
  UInt_t fNEvent;                   // event number
  UInt_t fDataHeader;               // data header either Mk2 or Mk1
  Int_t fInitLevel;                 // hardware initialisation
  UInt_t fTimeOut;                  // timeout wait for Supervisor end run
  Bool_t fIsSwap;                   // byte-swap data? big/little endian
  Bool_t fIsSlowCtrl;               // any slow-control functions?
  Bool_t fIsCtrl;                   // DAQ control enabled (run start/stop)?
  Bool_t fIsStore;                  // Data storage enabled?
  Bool_t fIsLocalAR;                // DAQ is run as thread of AcquRoot
  Bool_t fIsEvIDMaster;             // Control of the event ID?
  Bool_t fIsRunTerm;                // local run terminated flag
 public:
  TDAQexperiment( const Char_t*, const Char_t*, const Char_t*, TAcquRoot* = NULL,
		  Int_t = EExpInit0 );
  virtual ~TDAQexperiment();
  void SetConfig( Char_t*, Int_t );   // configure experiment
  void AddModule( Char_t* );          // add prog electronics
  void PostInit();                    // 2nd phase init
  void StartExperiment();             // start threads running
  void RunIRQ();                      // loop for event-by-event readout
  void RunSlowCtrl();                 // loop for slow-control monitor
  void RunDAQCtrl();                  // loop for DAQ control input
  void RunStoreData();                // loop for data storage
  void StoreEvent( Char_t* );         // write one event to buffer
  void StoreBuffer( Char_t* );        // write one complete buffer of data
  void BuffStore( void**, UInt_t );   // write a 4-bytes to the output buffer
  void PostReset();                   // stuff to be done after general reset
  void ZeroNEvent(){ fNEvent = 0; }   // zero event number
  UInt_t GetNEvent(){ return fNEvent; }   // current event #
  UInt_t GetNEventMax(){ return fNEventMax; }   // max event number
  TAcquRoot* GetAR(){ return fAR; }
  TDAQsupervise* GetSupervise(){ return fSupervise; }
  TDAQstore* GetStore(){ return fStore; }
  TList* GetModuleList(){ return fModuleList; }
  TList* GetADCList(){ return fADCList; }
  TList* GetScalerList(){ return fScalerList; }
  TList* GetSlowCtrlList(){ return fSlowCtrlList; }
  TA2RingBuffer* GetOutBuff(){ return fOutBuff; }
  Char_t* GetIRQModName(){ return fIRQModName; }
  Char_t* GetStartModName(){ return fStartModName; }
  Char_t* GetRunDesc(){ return fRunDesc; }
  Char_t* GetFileName(){ return fFileName; }
  TDAQmodule* GetIRQMod(){ return fIRQMod; }
  TDAQmodule* GetStartMod(){ return fStartMod; }
  TDAQmodule* GetSynchMod(){ return fSynchMod; }
  Int_t GetRunStart(){ return fRunStart; }
  Int_t GetNModule(){ return fNModule; }
  Int_t GetNADC(){ return fNADC; }
  Int_t GetNEPICS(){ return fNEPICS; }
  Int_t GetNScaler(){ return fNScaler; }
  Int_t GetNCtrl(){ return fNSlowCtrl; }
  Int_t GetNSlowCtrl(){ return fNSlowCtrl; }
  Int_t GetDataOutMode(){ return fDataOutMode; }
  Int_t GetScReadFreq(){ return fScReadFreq; }
  Int_t GtSlCtrlFreq(){ return fSlCtrlFreq; }
  Int_t GetRingSize(){ return fRingSize; }
  Int_t GetRecLen(){ return fRecLen; }
  Int_t GetNRec(){ return fNRec; }
  Int_t GetNRecMax(){ return fNRecMax; }
  Int_t GetInitLevel(){ return fInitLevel; }
  UInt_t GetDataHeader(){ return fDataHeader; }
  Int_t GetNADCError(){ return fNADCError; }
  Int_t GetNScalerError(){ return fNScalerError; }
  Bool_t IsMk2Format(){
    if( fDataHeader == EMk2DataBuff ) return kTRUE;
    else return kFALSE;
  }
  Bool_t IsSwap(){ return fIsSwap; }
  Bool_t IsSlowCtrl(){ return fIsSlowCtrl; }
  Bool_t IsCtrl(){ return fIsCtrl; }
  Bool_t IsStore(){ return fIsStore; }
  Bool_t IsLocalAR(){ return fIsLocalAR; }
  Bool_t IsEvIDMaster(){ return fIsEvIDMaster; }
  Bool_t IsRunTerm(){ return fIsRunTerm; }
  //  Bool_t IsTrigCtrl(){ return fIsTrigCtrl; }
  //  void SetTrigCtrl(){ fIsTrigCtrl = kTRUE; }
  void IncADCError(){ fNADCError++; }
  void IncScalerError(){ fNScalerError++; }

  ClassDef(TDAQexperiment,1)   
};

//---------------------------------------------------------------------------
inline void TDAQexperiment::StoreEvent( Char_t* evPos )
{
  // Store event buffer in output ring buffer
  // If it will not fit try the next buffer
  // Write end-of-buffer marker if space
  // 21/05/12 JRMA don't wait for buffer empty if not storing data (diagnostic)
  // 09/09/13 JRMA end-of-run detected force send of current data buffer.
  //
  Char_t* pbuff;
  Int_t evsize = evPos - fEventBuff;              // size of event in bytes
  pbuff = (Char_t*)fOutBuff->Update( fEventBuff, evsize );   // write event
  if( !pbuff ){                                              // no space
    fOutBuff->FNext();                                       // next buffer
    fNRec++;                                                 // record counter
    if(fIsStore)                                             // if storing...
      fOutBuff->WaitEmpty();                                 // check empty
    fOutBuff->Header(fDataHeader);                           // ensure header
    pbuff = (Char_t*)fOutBuff->Update( fEventBuff, evsize ); // write event
  }
  if( pbuff <= ((Char_t*)fOutBuff->GetStore() + 
		fOutBuff->GetLenBuff() - sizeof(UInt_t)) )
    fOutBuff->Trailer();
  // If its run end flush the current buffer and prepare the next
  // only happens if storing
  // reset of fIsRunTerm delayed...hack for now until more intelligent
  // handshake with supervisor devised
  if( fIsRunTerm ){
    fOutBuff->FNext();
    fNRec++;
    fOutBuff->WaitEmpty();                                   // check empty
    if( !fIsEvIDMaster ) fOutBuff->Header(fDataHeader);
    sleep(1);                                                // wait a bit
    fIsRunTerm = kFALSE;
  }
}

//---------------------------------------------------------------------------
inline void TDAQexperiment::StoreBuffer( Char_t* buffData )
{
  // Store buffer-full of data
  // Check that it fits
  //
  if( !fOutBuff->IsEmpty() ){
    fOutBuff->FNext();
    fNRec++;
  }
  fOutBuff->ResetBuffPtr();   // ensure buffer pointer reset
  Char_t* pbuff =
    (Char_t*)fOutBuff->Update( buffData );
  //    (Char_t*)fOutBuff->Update( buffData, fOutBuff->GetLenBuff() );
  if( !pbuff )
    PrintError("StoreBuffer","<Buffer write failed>",EErrFatal);
  fOutBuff->FNext();         // show its full and go to next
  fNRec++;                   // increment record counter
}

//---------------------------------------------------------------------------
inline void TDAQexperiment::BuffStore( void** out, UInt_t scaler )
{
  // Write 4-byte value to output data buffer
  UInt_t* buff = (UInt_t*)(*out);
  *buff++ = scaler;
  *out = buff;
}

#endif

