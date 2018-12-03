//--Author	JRM Annand      6th Oct 2016  
//--Rev 	JRM Annand...
//--Update      JRM Annand     14th Oct 2016 1st running version   
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_VPE2xX86_64A
// Concurrent Tech. VMEbus SBC quad Atom X86_64 processor 1.9 GHz
// Universe II bridge" to VMEbus

#ifndef __TDAQ_VPE2xX86_64A_h__
#define __TDAQ_VPE2xX86_64A_h__

#include "TDAQ_VPE2xX86_64.h"


class TDAQ_VPE2xX86_64A : public TDAQ_VPE2xX86_64 {
  /*
 protected:
  Char_t* fDevName[ENImage];        // name of device
  Int_t fDev[ENImage];             // device handler IDs
  Int_t fDevCTL;
  Int_t fMemFd;
  UInt_t fVMEaddr[ENImage];            // start VME addresses PCI images
  UInt_t fVMEsize[ENImage];            // length PCI images
  UInt_t fPCIaddr[ENImage];           // start PCI address windows
  Int_t fAMcode[ENImage];             // VME addressing scheme
  Int_t fNImage;
  Int_t fByteSwap;
#ifdef DAQ_CCTVMEEN
  EN_PCI_IMAGE_DATA fBrData[ENImage]; // PCI image structs
  EN_VME_IMAGE_DATA fBrDataV[ENImage];// VME image structs
#endif
  */
  Int_t fDevAM[1024];
  Int_t fErr;
  UChar_t fBuff[8];
 public:
  TDAQ_VPE2xX86_64A( Char_t*, Char_t*, FILE*, Char_t* );
  // virtual ~TDAQ_VPE2xX86_64A();
  //void SetConfig( Char_t*, Int_t );         // set and map PCI images
  virtual void PostInit(); 
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  virtual DAQMemMap_t* MapSlave(void*, Int_t, Int_t);
  Int_t GetMemFd(){ return fMemFd; }
  //
  ClassDef(TDAQ_VPE2xX86_64A,1)   
    };

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64A::Read( void* addr, void* data, Int_t am, Int_t dw )
{
#ifdef DAQ_CCTVMEEN
  fErr = 
    vme_read(fDev[fDevAM[am]],(ULong_t)addr,(UChar_t*)data,dw);
  //vme_read(fDev[fDevAM[am]],(ULong_t)addr,fBuff,8);
  if(fErr < 0){
    printf("VME read error\n");
    //strerror(errno);
  }
  if(dw == 4)
    *((UInt_t*)data) = *(UInt_t*)fBuff;
  else if(dw == 2)
    *((UShort_t*)data) = *(UShort_t*)fBuff;
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_VPE2xX86_64A::Write( void* addr, void* data, Int_t am, Int_t dw )
{
#ifdef DAQ_CCTVMEEN
  if(dw == 4)
    *((UInt_t*)fBuff) = *(UInt_t*)data;
  else if(dw == 2)
    *((UShort_t*)fBuff) = *(UShort_t*)data;
  fErr = 
    vme_write(fDev[fDevAM[am]],(ULong_t)addr,fBuff,8);
  if(fErr < 0){
    printf("VME write error\n");
    // strerror(errno);
  }
#endif
}


#endif
