//--Author	JRM Annand     16th Aug 2017
//--Rev
//--Update
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_SIS1100
// SIS 1100/3100 Optical VMEbus to PCI interface

#ifndef __TDAQ_SIS1100_h__
#define __TDAQ_SIS1100_h__

#include "TDAQmodule.h"

#ifdef DAQ_SIS1100
#include "sis3100_vme_calls.h"  // SIS1100 read/write calls
#endif

class TDAQ_SIS1100 : public TDAQmodule {
 protected:
  Int_t fMemFd;
  Int_t fdevAxxDxx;                      // File desc. 

public:
  TDAQ_SIS1100( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TDAQ_SIS1100();
  void SetConfig( Char_t*, Int_t );         // set and map PCI images
  virtual void PostInit(); 
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  virtual void Read(void*, void*);
  virtual void Write(void*, void*);
  virtual DAQMemMap_t* MapSlave(void*, Int_t, Int_t);
  Int_t GetMemFd(){ return fMemFd; }
  //
  ClassDef(TDAQ_SIS1100,1)   
    };

//-----------------------------------------------------------------------------
inline void TDAQ_SIS1100::Read( void* addr, void* data, Int_t am, Int_t dw )
{
#ifdef DAQ_SIS1100
  // General purpose read
  Int_t err;
  switch(dw){
  case 2:
  default:
    if( am == 0x09 )
      err = vme_A32D16_read(fdevAxxDxx,(UInt_t)addr,(UShort_t*)data);
    else
      err = vme_A24D16_read(fdevAxxDxx,(UInt_t)addr,(UShort_t*)data);
    break;
  case 4:
    if( am == 0x09 )
      err = vme_A32D32_read(fdevAxxDxx,(UInt_t)addr,(UInt_t*)data);
    else
      err = vme_A24D32_read(fdevAxxDxx,(UInt_t)addr,(UInt_t*)data);
    break;
  }    
  if( err ){
    printf("Error %d SIS1100 read: addr %x, am = %x, dw 0 %d\n",err,(UInt_t)addr,am,dw);
  }  
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_SIS1100::Write( void* addr, void* data, Int_t am, Int_t dw )
{
#ifdef DAQ_SIS1100
  // General purpose write
  Int_t err;
  switch(dw){
  case 2:
  default:
    if( am == 0x09 )
      err = vme_A32D16_write(fdevAxxDxx,(UInt_t)addr,*(UShort_t *)data);
    else
      err = vme_A24D16_write(fdevAxxDxx,(UInt_t)addr,*(UShort_t *)data);
    break;
  case 4:
    if( am == 0x09 )
      err = vme_A32D32_write(fdevAxxDxx,(UInt_t)addr,*(UInt_t *)data);
    else
      err = vme_A24D32_write(fdevAxxDxx,(UInt_t)addr,*(UInt_t *)data);
    break;
  }   
  if( err ){
    printf("Error %d SIS1100 write: addr %x, am = %x, dw 0 %d\n",err,(UInt_t)addr,am,dw);
  }  
   
#endif
}

//-----------------------------------------------------------------------------
inline void TDAQ_SIS1100::Read( void* addr, void* data )
{
  //
}

//-----------------------------------------------------------------------------
inline void TDAQ_SIS1100::Write( void* addr, void* data )
{
  //
}



#endif
