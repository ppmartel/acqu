//--Author	JRM Annand     4th Jul 2003  1st Go
//--Rev		JRM Annand...  3rd Oct 2005  Root integration
//--Update	Baya Oussena   28th Sep 2009 added fMemFd and use MAP_SHARED
//--Description
//
//  TDAQMemMap
//  Inherited class of TDAQsystem
//  Map physical memory location to block of virtual memory
//---------------------------------------------------------------------------

#include "DAQMemMap_t.h"
extern "C"{
#include <sys/mman.h>
}


DAQMemMap_t::DAQMemMap_t( void* addr, Int_t size, Int_t fMemFd, TA2System* sys )
: TObject()  
{
  fPerm = PROT_READ | PROT_WRITE;
  fFlags = MAP_SHARED; 
  fPhysAddr = addr; 
  fSize = size;
  fSys = sys;
#ifdef VME_HOST
  fVirtAddr = mmap( NULL, fSize, fPerm, fFlags, fMemFd, (off_t)fPhysAddr);

  if( fVirtAddr == (void*)-1 ){
    fSys->PrintError("","<Virtual-Physical Memory Map>", EErrFatal);
  }
#endif
  return;  
}
//
DAQMemMap_t::DAQMemMap_t( ULong_t addr, Int_t size, ULong_t vaddr,
			  Int_t fMemFd, TA2System* sys )
: TObject()  
{
  fPerm = PROT_READ | PROT_WRITE;
  fFlags = MAP_SHARED; 
  fPhysAddr = (void*)addr; 
  fSize = size;
  fSys = sys;
  fVirtAddr = (void*)vaddr;
  return;  
}

//----------------------------------------------

DAQMemMap_t::~DAQMemMap_t()
{
  //
  if( munmap( fVirtAddr, fSize ) == -1)
    fSys->PrintError("","<Virtual-Memory unmapping failure>");
  

  return; 
}

ClassImp(DAQMemMap_t)               // Root linkage
