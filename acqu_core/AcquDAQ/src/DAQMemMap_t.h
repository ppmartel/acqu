//--Author	JRM Annand     4th Jul 2003  1st Go
//--Rev		JRM Annand...  3rd Oct 2005  Root integration
//--Update	Baya Oussena   28th Sep 2009 added fMemFd and use MAP_SHARED
//--Description
//
//  TDAQMemMap
//  Inherited class of TDAQsystem
//  Map physical memory location to block of virtual memory
//---------------------------------------------------------------------------

#ifndef __DAQMemMap_t_h__
#define __DAQMemMap_t_h__

#include "TA2System.h"			// integer constant defs

class DAQMemMap_t : public TObject {
 private:
  Int_t fID;		// memory system ID
  Int_t fPerm;          // access permission
  Int_t fFlags;         // create flags
  Int_t fAM;		// AM code for VMEbus operations
  Int_t fMemFd;         // Physical memory descriptor <------Baya
  Int_t fSize;		// Memory length in bytes
  void* fPhysAddr;	// Start physical address
  void* fVirtAddr;	// Start virtual address
  TA2System* fSys;
 public:
  DAQMemMap_t( void*, Int_t, Int_t, TA2System* );  //<--- Baya
  DAQMemMap_t( ULong_t, Int_t, ULong_t, Int_t, TA2System* );
  virtual ~DAQMemMap_t();
  void SetConfig(char*, int){ return; }
  Int_t GetID(){ return fID; }
  Int_t GetPerm(){ return fPerm; }
  Int_t GetFlags(){ return fFlags; }
  Int_t GetAM(){ return fAM; }
  Int_t GetSize(){ return fSize; }
  void* GetPhysAddr(){ return fPhysAddr; } 
  void* GetVirtAddr(){ return fVirtAddr;}

  ClassDef( DAQMemMap_t,1 )                  // Root linkage
    };

#endif
