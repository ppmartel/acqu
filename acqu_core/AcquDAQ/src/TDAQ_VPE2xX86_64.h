//--Author	JRM Annand      6th Oct 2016  
//--Rev 	JRM Annand...
//--Rev         JRM Annand     14th Oct 2016 1st running version   
//--Update      JRM Annand     26th Oct 2017 Include DMA option  
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_VPE2xX86_64
// Concurrent Tech. VMEbus SBC quad Atom X86_64 processor 1.9 GHz
// Universe II bridge" to VMEbus

#ifndef __TDAQ_VPE2xX86_64_h__
#define __TDAQ_VPE2xX86_64_h__

#include "TDAQmodule.h"

#ifdef DAQ_CCTVMEEN
//#include "vme_api_en.h"
#include "cctvmeen.h"        // defs abd structs for Universe device driver
#endif
#define ENImage 8            // Max number PCI image maps
// device names for driver access

class TDAQ_VPE2xX86_64 : public TDAQmodule {
 protected:
  Char_t* fDevName[ENImage];          // name of device
  Int_t fDev[ENImage];                // device handler IDs
  Int_t fDevCTL;                      // Control device
  Int_t fDevDMA;                      // DMA device
  Int_t fMemFd;
  UInt_t fVMEaddr[ENImage];           // start VME addresses PCI images
  UInt_t fVMEsize[ENImage];           // length PCI images
  UInt_t fPCIaddr[ENImage];           // start PCI address windows
  Int_t fAMcode[ENImage];             // VME addressing scheme
  Int_t fVMEcycle[ENImage];           // SCT, BLT etc.
  Int_t fNImage;                      // # PCI images
  Int_t fByteSwap;                    // Apply hardware byte swap
  Int_t fDMAoffset;                   // data offset in DMA buffer 
  Int_t fDMAdwidth;                   // data width DMA transfer
  Int_t fDMAvmecycle;                 // 0-SCT/1-BLT
  Int_t fDMAownership;                // VME on/off counters
#ifdef DAQ_CCTVMEEN
  EN_PCI_IMAGE_DATA fPCIData[ENImage]; // PCI-VME image structs
  //EN_VME_CMD_DATA fDMAData;
  EN_VME_TXFER_PARAMS fDMAtxfer;
#endif
 public:
  TDAQ_VPE2xX86_64( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TDAQ_VPE2xX86_64();
  void SetConfig( Char_t*, Int_t );         // set and map PCI images
  virtual void PostInit(); 
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  virtual void Read(void*, void*);
  virtual void Write(void*, void*);
  virtual void* DMAread();
  virtual DAQMemMap_t* MapSlave(void*, Int_t, Int_t);
  Int_t GetMemFd(){ return fMemFd; }
  virtual void* InitDMA(Int_t);
  virtual void LinkDMA(void*, Int_t, Int_t);

  //
  ClassDef(TDAQ_VPE2xX86_64,1)   
    };

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64::Read( void* addr, void* data, Int_t am, Int_t dw )
{
  //#ifdef VME_HOST
  // General purpose read
  //memcpy(data, (const void*)addr, (size_t)dw);
  //
  switch(dw){
  case 2:
  default:
    *((UShort_t*)data) = *((UShort_t*)addr);
    break;
  case 4:
    *((UInt_t*)data) = *((UInt_t*)addr);
    break;
  }
  //
  //#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64::Write( void* addr, void* data, Int_t am, Int_t dw )
{
  //#ifdef VME_HOST
  // General purpose write
  memcpy(addr, (const void*)data, (size_t)dw);
  /*
  switch(dw){
  case 2:
  default:
    *((UShort_t*)addr) = *((UShort_t*)data); 
    break;
  case 4:
    *((UInt_t*)addr) = *((UInt_t*)data);
    break;
  }
  */
  //#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64::Read( void* addr, void* data )
{
  //
}

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64::Write( void* addr, void* data )
{
  //
}

//-----------------------------------------------------------------------------
inline void* TDAQ_VPE2xX86_64::DMAread()
{
  //
#ifdef DAQ_CCTVMEEN
  Int_t result = vme_dmaListTransfer( fDevDMA, &fDMAtxfer );
  //  if ( result < 0 ){
  //    printf("Error - DMA transfer failed\n");
  //    return NULL;
  //  }
#endif
  return fMemDMA;
}

#endif
