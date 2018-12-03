//--Author	JRM Annand   20th Dec 2005
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 5th Dec 2007 Int_t fHandle for libCAENVME
//--Rev 	JRM Annand... 3rd Jun 2008  Conditional compilation
//--Rev 	JRM Annand... 6th Jul 2011  gcc4.6-x86_64 warning fix
//--Update	JRM Annand...18th Apr 2018  Add daq-trigger control functions
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_V2718
// CAEN A2818 PCI <--> V2718 VMEbus controller via optical fibre
// Assumes that a2818 linux driver is loaded
// Uses control functions from libCAENVME.so
// Optional daq-trigger control uses internal pulser & input/output connect

#ifndef __TDAQ_V2718_h__
#define __TDAQ_V2718_h__

#include "TDAQmodule.h"
//
#ifdef DAQ_CAENV2718
// CAEN library of control procedures for the A2818/V2718 VMEbus controller
#define LINUX                   // so the CAEN stuff knows its not windows
extern "C" {
#include "CAENVMElib.h"
}
#endif

class TDAQ_V2718 : public TDAQmodule {
 protected:
  Int_t fHandle;
  Int_t fBoard;
  Short_t fLink;
  Short_t fBdNum;
  Short_t fAM;
  Bool_t  fIsIRQ;
  Bool_t  fIsIRQEnabled;
 public:
  TDAQ_V2718( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TDAQ_V2718();
  void SetConfig( Char_t*, Int_t );         // configure v2718
  virtual void PostInit();
  //  virtual void Read(void*, void*, Int_t= cvA24_U_DATA, Int_t= cvD16_swapped);
  //  virtual void Write(void*, void*, Int_t= cvA24_U_DATA, Int_t= cvD16_swapped);
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  //  virtual void Read(void*, void*, Int_t);
  //  virtual void Write(void*, void*, Int_t);
  virtual void Read(void*, void*);
  virtual void Write(void*, void*);
  virtual void BlkRead();
  virtual void BlkWrite();
  virtual void WaitIRQ();
  virtual void ResetIRQ();
  virtual void EnableIRQ();
  virtual void DisableIRQ(){ fIsIRQEnabled = kFALSE; }
  Bool_t IsIRQEnabled(){ return fIsIRQEnabled; }
  //
  Int_t GetHandle(){ return fHandle; }
  Short_t GetLink(){ return fLink; }
  Short_t GetBdNum(){ return fBdNum; }
  Short_t GetAM(){ return fAM; }
  void SetAM( Int_t i ){ fAM = (UShort_t)i; }

  //
  ClassDef(TDAQ_V2718,1)   
    };

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::Read( void* vaddr, void* data, Int_t am, Int_t dw )
{
  //
#ifdef DAQ_CAENV2718
  UInt_t addr = (ULong_t)vaddr;
  fErrorCode = CAENVME_ReadCycle( fHandle, addr, data,
				  (CVAddressModifier)am, (CVDataWidth)dw );
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::Write( void* vaddr, void* data, Int_t am, Int_t dw )
{
  //
#ifdef DAQ_CAENV2718
  UInt_t addr = (ULong_t)vaddr;
  fErrorCode = CAENVME_WriteCycle(fHandle, addr, data,
				  (CVAddressModifier)am, (CVDataWidth)dw );
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::Read( void* vaddr, void* data )
{
  // Read with default AM code and data size
#ifdef DAQ_CAENV2718
  UInt_t addr = (ULong_t)vaddr;
  fErrorCode = CAENVME_ReadCycle( fHandle, addr, data,
				  (CVAddressModifier)fAM, (CVDataWidth)*fDW );
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::Write( void* vaddr, void* data )
{
  // Write at default AM code and data size
  //
#ifdef DAQ_CAENV2718
  UInt_t addr = (ULong_t)vaddr;
  fErrorCode = CAENVME_WriteCycle(fHandle, addr, data,
				  (CVAddressModifier)fAM, (CVDataWidth)*fDW );
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::WaitIRQ( )
{
  // Poll the state of Pulser A, via the cvPulsAOut bit of the input register
  // Pulser A is set when a signal is received on Input 0
  // 
#ifdef DAQ_CAENV2718
  UInt_t datum;
  for(;;){
    if( fIsIRQEnabled ){       // "interrupt" enabled?
      CAENVME_ReadRegister(fHandle, cvInputReg, &datum);
      if(datum & cvPulsAOutBit)
      //if(datum & cvIn0Bit)
	break;
    }
  }
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::ResetIRQ( )
{
  // Stop pulser which provides busy signal, ie reset busy
  //
  //CAENVME_SetOutputRegister(fHandle,cvPulsBResetBit);
#ifdef DAQ_CAENV2718
  CAENVME_StopPulser(fHandle,cvPulserA);
  CAENVME_PulseOutputRegister(fHandle,cvOut1Bit);
  //
  //Int_t ifr = 0;
  //Write(fIFR, &ifr);
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_V2718::EnableIRQ( )
{
  // Enable triggers
  // Ensure that pulser A stopped
  //
#ifdef DAQ_CAENV2718
  fIsIRQEnabled = kTRUE;
  //CAENVME_SetOutputRegister(fHandle,cvPulsBResetBit);
  CAENVME_StopPulser(fHandle,cvPulserA);
  CAENVME_PulseOutputRegister(fHandle,cvOut1Bit);
#endif
}


#endif
