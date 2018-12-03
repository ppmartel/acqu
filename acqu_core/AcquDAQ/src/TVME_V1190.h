//--Author	JRM Annand   1st Feb 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand  11th Jan 2008  1st working version
//--Rev 	JRM Annand...3rd Jun 2008..const Char_t*...gcc 4.3
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..24th Aug 2012..Start debugging
//--Rev 	JRM Annand..25th Sep 2012.."Working version"
//--Rev 	JRM Annand..25th Sep 2013..Edge detection control
//--Update	JRM Annand...2nd Nov 2017..DMA support, extra config opt.
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_V1190
// CAEN VMEbus 32-channel QDC
// Also forms basis for classes to handle similar CAEN ADC hardware
//

#ifndef __TVME_V1190_h__
#define __TVME_V1190_h__

#include "TVMEmodule.h"

enum {
  EV1190_NMem = 4096,
  EV1190_NReg = 5,
  EV1190_NThresh = 32,
  EV1190_NROM = 9,
  EV1190_MWOK = 0x1,
  EV1190_MROK = 0x2,
  EV1190_MicroTimeout = 100000     
};

// Indices to internal registers
// See V1190reg[] for corresponding addresses
enum {
  EV1190_Ctrl,
  EV1190_Status,
  EV1190_IRQLevel,
  EV1190_IRQVect,
  EV1190_GeoAddr,
  EV1190_MCSTBaseAddr,
  EV1190_MCSTCtrl,
  EV1190_Reset,
  EV1190_Clr,
  EV1190_EvReset,
  EV1190_Trigger,
  EV1190_EvCnt,
  EV1190_EvStore,
  EV1190_AlmostFullLevel,
  EV1190_BLTEvent,
  EV1190_Firmware,
  EV1190_TestReg,
  EV1190_OutProg,
  EV1190_Micro,
  EV1190_MicroHShake,
  EV1190_SelFlash,
  EV1190_FlashMem,
  EV1190_SRAM,
  EV1190_EvFIFO,
  EV1190_EvFIFOStored,
  EV1190_EvFIFOStatus,
  EV1190_Dummy32,
  EV1190_Dummy16,
  EV1190_ID0,
  EV1190_ID1,
  EV1190_ID2,
  EV1190_ConfSRAM,
  EV1190_OutBuff,
};

// Micro controller op codes
enum {
  EM1190_TrigMatch =      0x0000,          // Set trigger matching
  EM1190_ContStore =      0x0100,          // Set continuous storage
  EM1190_RdAcqMod =       0x0200,          // Read acquisition mode
  EM1190_SetKeepToken =   0x0300,          // Set keep token
  EM1190_ClrKeepToken =   0x0400,          // Clear keep token
  EM1190_LoadDefConf =    0x0500,          // Load default configuration
  EM1190_SaveDefConf =    0x0600,          // Save user configuration
  EM1190_LoadUsrConf =    0x0700,          // load user configuration
  EM1190_AutoUsrConf =    0x0800,          // Set auto load user configuration
  EM1190_AutoDefConf =    0x0900,          // Set auto load default config.
  EM1190_SetWinWidth =    0x1000,          // Set window width
  EM1190_SetWinOff =      0x1100,          // Set window offset
  EM1190_SetSWmargin =    0x1200,          // Set extra search margin
  EM1190_SetRejMargin =   0x1300,          // Set reject margin
  EM1190_EnSubTrig =      0x1400,          // enable subtraction trigger time
  EM1190_DisSubTrig =     0x1500,          // disable subtraction trigger time
  EM1190_RdTrigConf =     0x1600,          // Read trigger configuration
  EM1190_SetDetection =   0x2200,          // Choose pair,leading/trailing edge
  EM1190_RdDetection =    0x2300,          // Read edge detection mode
  EM1190_SetTrigLeadLSB = 0x2400,          // Set LSB leading/trailing edge
  EM1190_SetPairRes =     0x2500,          // Set time & width res. when pair
  EM1190_RdReas =         0x2600,          // Read resolution
  EM1190_SetDeadTime =    0x2700,          // Set channel dead time between hits
  EM1190_RdDeadTime =     0x2800,          // Read chan. dead time between hits
  EM1190_EnHeadTrailer =  0x3000,          // Enable TDC header & trailer
  EM1190_DisHeadTrailer = 0x3100,          // Disable TDC header & trailer
  EM1190_RdHeadTrailer =  0x3200,          // Read TDC header/trailer status
  EM1190_SetEvSize =      0x3300,          // Set max. number hits per event
  EM1190_RdEvSize =       0x3400,          // Read max. number hits per event
  EM1190_EnErrMark =      0x3500,          // Enable TDC error mark
  EM1190_DisErrMark =     0x3600,          // Disable TDC error mark
  EM1190_EnErrBypass =    0x3700,          // Enable bypass TDC if error
  EM1190_DisErrByPass =   0x3800,          // Disable bypass TDC if error
  EM1190_SetErrType =     0x3900,          // Set TDC internal error type
  EM1190_RdErrType =      0x3a00,          // Read TDC internal error type
  EM1190_SetFIFOsize =    0x3b00,          // Set size readout FIFO
  EM1190_RdFIFOsize =     0x3c00,          // Read size readout FIFO
  EM1190_EnChan =         0x4000,          // Enable TDC channel n
  EM1190_DisChan =        0x4100,          // Disable TDC channel n
  EM1190_EnAllChan =      0x4200,          // Enable all TDC channels
  EM1190_DisAllChan =     0x4300,          // Disable all TDC channels
  EM1190_WrtEnPatt =      0x4400,          // Set TDC enable channel pattern
  EM1190_RdEnPatt =       0x4500,          // Read TDC enable channel pattern
  EM1190_WrtEnPatt32 =    0x4600,          // Set 32bit channel enable pattern
  EM1190_RdEnPatt32 =     0x4700,          // Read 32bit channel enable pattern
  EM1190_SetGlobOff =     0x5000,          // Set global offset
  EM1190_RdGlobOff =      0x5100,          // Read global offset
  EM1190_SetAdjChan =     0x5200,          // Set adjust channel n
  EM1190_RdAdjChan =      0x5300,          // Read adjust channel n
  EM1190_SetRCAdj =       0x5400,          // Set RC adjust of TDC i
  EM1190_RdRCAdj =        0x5500,          // Read RC adjust TDC i
  EM1190_SaveRCAdj =      0x5600,          // Save RC adjust to EEPROM
  EM1190_RdTDCid =        0x6000,          // Read programed id TDC i
  EM1190_RdMicroRev =     0x6100,          // Read firmware rev. micro ctrl
  EM1190_ResDLLPLL =      0x6200,          // Reset DLL and PLL
};

class TVME_V1190 : public TVMEmodule {
 protected:
  Bool_t* fIsIRQEnabled;
  Int_t fEdgeDet;
  Int_t fWindow;
  Int_t fWindowOffset;
  Int_t fMaxHits;
  Bool_t fIsContStore;
  Bool_t fIsDisSubTrig;
 public:
  TVME_V1190( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_V1190();
  virtual void SetConfig( Char_t*, Int_t );   // configure VME module
  virtual void PostInit( );
  virtual void ReadIRQ( void** );
  virtual Bool_t CheckHardID();
  //
  virtual void ResetData(){ Write(EV1190_Clr); }
  virtual void Reset(){ Write(EV1190_Reset); }
  virtual void ReadMicro( Int_t, Int_t=0, Int_t* =NULL );
  virtual void WrtMicro( Int_t, Int_t=0, Int_t* =NULL );
  virtual Bool_t IsDataReady(){ 
    Int_t datum = Read(EV1190_Status);
    return (Bool_t)(datum&0x1);
  }
  ClassDef(TVME_V1190,1)   

    };

//------------------------------------------------------------------------------

#endif

