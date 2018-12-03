//--Author	JRM Annand   30th Sep 2005  Start AcquDAQ
//--Rev 	JRM Annand... 1st Oct 2005  New DAQ software
//--Rev 	JRM Annand...10th Feb 2006  1st hardware test version
//--Rev 	JRM Annand... 9th Jun 2007  Mk2 Module indices, & error block
//--Rev 	JRM Annand...11th Jan 2008  Additions for working DAQ
//--Rev 	JRM Annand...29th Apr 2009  Memory mapping stuff
//--Rev 	JRM Annand...24th Jul 2009  Store fEXP pointer
//--Rev         B. Oussena   15th Jan 2010 Start updates for FASTBUS
//--Rev         B. Oussena    1st feb 2010 Add cmd EFB_RDMode, SetRDMode()
//--Rev         B. Oussena   10th Mar 2010 Add decode Scaler and ADC methods
//--Rev  	JRM Annand   27th Jul 2010 Generalise 1800 module class
//--Rev 	JRM Annand...19th Jul 2010  Mods for Mk1 data format
//--Rev 	JRM Annand...20th Jul 2010  Dummy trigger ctrl functions
//--Rev 	B. Oussena... 3rd Aug 2010  Chg Rd format fHardID in SetConfig 
//--Rev 	B. Oussena...13th Aug 2010  Add fActualHardID for generic module
//--Rev         B. Oussena    3rd Aug 2010  Fix bugs in SetConfig() & PostInit()
//--Rev         JRM Annand    4th Sep 2010  Chk Mk1/Mk2 data, fBaseIndex setting
//--Rev         JRM Annand    6th Sep 2010  Add GetEventID()
//--Rev         JRM Annand    9th Sep 2010  kModTypes."NULL"=undefined
//--Rev         JRM Annand   11th Sep 2010  kModTypes.add EDAQ_VADC,EDAQ_VScaler
//--Rev         JRM Annand   12th Sep 2010  ReadHeader(mod)..debug
//--Rev         JRM Annand   13th Sep 2010  ErrorStore Mk1 ensure totally init
//--Rev         JRM Annand   17th Sep 2010  fVirtOffset and GetVirtAddr()
//--Rev         JRM Annand   12th May 2011  Incr. error counters in fEXP
//--Rev         JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fix
//--Rev         JRM Annand   21st May 2012  Default ReadIRQSlow...do nothing
//--Rev         JRM Annand   28th Aug 2012  SendEventID()
//--Rev         JRM Annand   29th Aug 2012  Add fNScalerChan
//--Rev         JRM Annand    2nd Sep 2012  Add fEventSendMod
//--Rev         JRM Annand    3rd Sep 2012  Module types changes SetConfig
//--Rev         JRM Annand    6th Jun 2013  Save fBaseIndex in error block
//--Rev          A Neiser...  6th Jun 2013  Make char_t* const
//--Update      JRM Annand   26th Oct 2017  Add prototypes for DMA
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQmodule
// Basic characteristics of all DAQ hardware
// Classes to characterise all programable electronics derive from this
//

#ifndef __TDAQmodule_h__
#define __TDAQmodule_h__

#include "TA2System.h"
#include "ModuleIndex.h"
#include "TDAQcontrol.h"
#include "DAQMemMap_t.h"
#include "TDAQexperiment.h"

class ModuleInfoMk2_t;
class ModuleInfo_t;

// default size to allocate register array
enum { E_MaxReg = 8192 };

// General function types of a module
enum { EModInit, EModIRQ, EModTest };

// Error Codes
enum { EErrRead, EErrWrite, EErrDataFormat };
class TDAQmodule : public TA2System {
 protected:
  TDAQexperiment* fEXP;              // info on the main experiment
  TDAQmodule* fPCtrlMod;             // Primary (top) control module (if any)
  TDAQmodule* fCtrlMod;              // control module (if any)
  TDAQcontrol* fCtrl;                // if controller, control stuff (if any)
  TDAQmodule* fEventSendMod;         // module which send event to remote sys (if any)
  DAQMemMap_t* fMemMap;              // virtual memory map (if any)
  void* fBaseAddr;                   // base address
  Long_t fVirtOffset;                // offset from physical to virtual address
  void** fReg;                       // register ptr array
  void** fInitReg;                   // init CNAF-equivalent addresses
  void** fTestReg;                   // test sequences
  void** fIRQReg;                    // per-event read CNAF-equiv addresses
  Int_t* fDW;                        // register sizes in bits
  UInt_t* fData;                     // initialisation data for registers
  Int_t fMapSize;                    // Size of virtual memory block mapped to VMEbus or other phys.
  Int_t fMaxReg;                     // maximum number of registers
  Int_t fNreg;                       // # registers                  
  Int_t fNInitReg;                   // # intiialise commands
  Int_t fNTestReg;                   // testing commands
  Int_t fNIRQReg;                    // # per-event read commands
  Int_t fErrorCode;                  // error returns
  Int_t fBus;                        // VMEbus, CAMAC etc.
  Int_t fID;                         // Hardware ID in Acqu
  Int_t fHardID;                     // Any ID number read from hardware
  Int_t fActualHardID;               // Actual ID Hardware for generic modules
  Int_t fIndex;                      // list index in Acqu
  Int_t fType;                       // ADC, scaler, slow control etc.
  Int_t fBaseIndex;                  // readout base index
  Int_t fNChannel;                   // # ADC/scaler channels in module
  Int_t fNScalerChan;                // # Scaler channels,if both ADC and scaler
  Int_t fNBits;                      // # significant bits adc/scaler word
  Int_t fInitLevel;                  // hardware initialisation
  Bool_t fIsError;                   // any error
  //
  Bool_t fIsDMA;                     // DMA transfer?
  Int_t fDMAsize;                    // DMA buffer length
  Int_t* fDMAtrans;                  // DMA transfer length
  void** fDMAaddr;                   // DMA VME start address
  Int_t fDMAnPkt;                    // # DMA command packets
  void* fMemDMA;                     // DMA map address
 public:
  friend class TDAQcontrol;          // access to protected variables
  TDAQmodule(const Char_t*, const Char_t*, FILE* );
  virtual ~TDAQmodule();
  virtual void SetConfig( Char_t*, Int_t ); // configure module
  virtual void PostInit();                  // 2nd phase initialisation
  virtual void InitReg( Char_t* );          // initialise and store register
  virtual void MapReg( );                   // registers to virtual memory addr
  virtual void SetCtrl( TDAQmodule* );      // record controller
  virtual void Read(){}                     // default null read function
  virtual void Write(){}                    // default null write function
  virtual UInt_t Read(Int_t);
  virtual void Write( Int_t );
  virtual void MultiRead( Int_t, Int_t );
  virtual void MultiWrite( Int_t, Int_t );
  virtual void Read(void*, void*,Int_t){}
  virtual void Write(void*, void*,Int_t){}
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  virtual void Read(void*, void*){}
  virtual void Write(void*, void*){}
  virtual void ADCStore( void**, UShort_t, UShort_t );
  virtual void ScalerStore( void**, UInt_t );
  virtual void ScalerStore( void**, UInt_t, UInt_t );
  virtual void ErrorStore( void**, Int_t=0 );
  virtual void InitWrite(){}
  virtual void Test(){}                     // default null test function
  virtual void ReadHeader( ModuleInfoMk2_t* );
  virtual void ReadHeader( ModuleInfo_t* );
  virtual void* DMAread(){return NULL;}
  // Calculate address
  virtual UShort_t* AddrS( UInt_t, UInt_t=0, UInt_t=0, UInt_t=0 )
  { return NULL; }
  virtual UInt_t* AddrL( UInt_t, UInt_t=0, UInt_t=0, UInt_t=0 )
  { return NULL; }
  virtual void ReadIRQ(void**){}     // interrupt driven readout procedure
  virtual void ReadIRQScaler(void** buff){ ReadIRQ(buff);}  // for scalers
  virtual void ReadIRQSlow(void** buff){ return;}           // for slow ctrl
  virtual void DecodeIRQ(){}         // decode data to common format
  virtual void WaitIRQ(){}           // interrupt state poller
  virtual void ResetIRQ(){}          // procedure to re-enable interrupts
  virtual void EnableIRQ(){}         // procedure to enable interrupts
  virtual void DisableIRQ(){}        // procedure to disable interrupts
  virtual void RunTrigCtrl(){}       // start and enable DAQ session
  virtual void EndTrigCtrl(){}       // end and disable DAQ session
  virtual void ResetTrigCtrl(){}     // reset DAQ trigger control system
  virtual void EnableTrigCtrl(){}    // enable triggers
  virtual void DisableTrigCtrl(){}   // disable triggers
  virtual void Reset(){}             // module reset...usually to default
  virtual void ResetData(){}         // internal memory reset
  virtual Int_t GetRegType(Char_t*); // key for type of register
  virtual Int_t GetModType(Char_t*); // key for type of module
  virtual UInt_t GetEventID(){ return 0;} // Read event ID
  virtual UInt_t GetReadoutPatternOffset(){ return 0;} // Read BitPattern ADC Index Offset
  virtual UInt_t GetReadoutPatternStatus(){ return 0;} // Read VITEC status reg..
  virtual UInt_t GetReadoutPattern(){ return 0;} // Read BitPattern
  virtual void SendEventID(UInt_t){ }     // Send event ID to remote system
  //
  void SetDAQexperiment( TDAQexperiment* fexp ){ fEXP = fexp; }
  TDAQmodule* GetPCtrlMod(){ return fPCtrlMod; }
  TDAQmodule* GetCtrlMod(){ return fCtrlMod; }
  TDAQcontrol* GetCtrl(){ return fCtrl; }
  DAQMemMap_t* GetMemMap(){ return fMemMap; }
  void* GetBaseAddr(){ return fBaseAddr; }
  Long_t GetVirtOffset(){ return fVirtOffset; }
  void* GetVirtAddr(UInt_t offset){ 
    return (Char_t*)fBaseAddr + fVirtOffset + offset; }
  void** GetReg(){ return fReg;}
  void** GetInitReg(){ return fInitReg; }
  void** GetIRQReg(){ return fIRQReg; }
  void** GetTestReg(){ return fTestReg; }
  Int_t* GetDW(){ return fDW; }
  UInt_t* GetData(){ return fData; }
  Int_t GetMapSize(){ return fMapSize; }
  Int_t GetMaxReg(){ return fMaxReg; }
  Int_t GetNreg(){ return fNreg; }
  Int_t GetNInitReg(){ return fNInitReg; }
  Int_t GetNIRQReg(){ return fNIRQReg; }
  Int_t GetNTestReg(){ return fNTestReg; }
  Int_t GetErrorCode(){ return fErrorCode; }
  Int_t GetBus(){ return fBus; }
  Int_t GetID(){ return fID; }
  Int_t GetHardID(){ return fHardID; }
  virtual Bool_t CheckHardID(){ return kTRUE; }
  Int_t GetIndex(){ return fIndex; }
  void SetID( Int_t id ){ fID = id; }
  void SetIndex( Int_t ind ){ fIndex = ind; }
  Int_t GetType(){ return fType; }
  Bool_t IsType( Int_t type ){ return (Bool_t)( type & fType ); }
  Int_t GetBaseIndex(){ return fBaseIndex; }
  void SetBaseIndex( Int_t i ){ fBaseIndex = i; }
  Int_t GetNChannel(){ return fNChannel; };
  virtual Int_t GetNScChannel(){ return fNChannel; }
  void SetNChannel( Int_t i ){ fNChannel = i; }
  Int_t GetNBits(){ return fNBits; }
  Int_t GetInitLevel(){ return fInitLevel; }
  void SetInitLevel( Int_t lev ){ fInitLevel = lev; }
  void SetEventSendMod( TDAQmodule* evsend ){ fEventSendMod = evsend; }
  Bool_t IsError(){ return fIsError; }
  virtual DAQMemMap_t* MapSlave( void*, Int_t, Int_t ){ return NULL; }
  //
  // DMA parameter getters
  Bool_t IsDMA(){ return fIsDMA; }
  Int_t GetDMAsize(){ return fDMAsize; }
  Int_t* GetDMAtrans(){ return fDMAtrans; }
  void** GetDMAaddr(){ return fDMAaddr; }
  Int_t GetDMAnPkt(){ return fDMAnPkt; }
  virtual void* InitDMA(Int_t=0x1000){ return NULL; }
  virtual void LinkDMA(void*, Int_t, Int_t=-1){ return; }

  ClassDef(TDAQmodule,1)   
    };

//-----------------------------------------------------------------------------
inline UInt_t TDAQmodule::Read( Int_t i )
{
  // Top (Primary) Controller read
  fPCtrlMod->Read( fReg[i], fData+i, fDW[i] );
  return fData[i];
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::Write( Int_t i )
{
  // Top (Primary) Controller write
  fPCtrlMod->Write( fReg[i], fData+i, fDW[i] );
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::MultiRead( Int_t i, Int_t j )
{
  // Multiple Primary Controller read
  while( i <= j ){
    fPCtrlMod->Read( fReg[i], fData+i, fDW[i] );
    i++;
  }
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::MultiWrite( Int_t i, Int_t j )
{
  // Multiple Primary Controller write
  while( i <= j ){
    fPCtrlMod->Write( fReg[i], fData+i, fDW[i] );
    i++;
  }
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::Read( void* addr, void* data, Int_t am, Int_t dw )
{
  // Controller read
  fPCtrlMod->Read( addr, data, am, dw );
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::Write( void* addr, void* data, Int_t am, Int_t dw )
{
  // Controller write
  fPCtrlMod->Write( addr, data, am, dw );
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::ADCStore( void** out, UShort_t adc, UShort_t index )
{
  // Write ADC value/index to buffer
  UShort_t* buff = (UShort_t*)(*out);
  *buff++ = index;
  *buff++ = adc;
  *out = buff;
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::ScalerStore( void** out, UInt_t scaler )
{
  // Write Scaler value/index to buffer
  UInt_t* buff = (UInt_t*)(*out);
  *buff++ = scaler;
  *out = buff;
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::ScalerStore( void** out, UInt_t scaler, UInt_t index )
{
  // Write Scaler value/index to buffer
  UInt_t* buff = (UInt_t*)(*out);
  if( fEXP->IsMk2Format() ) *buff++ = index;
  *buff++ = scaler;
  *out = buff;
}


#endif
