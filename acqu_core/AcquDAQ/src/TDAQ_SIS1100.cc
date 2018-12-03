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

#include "TDAQ_SIS1100.h"


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

char devAxxDxx[] = "/dev/sis1100_00remote";

static Map_t kSIS1100Keys[] = {
  {NULL,           -1}
};

extern int errno;

//-----------------------------------------------------------------------------
TDAQ_SIS1100::TDAQ_SIS1100( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation
  fType = EDAQ_Ctrl;                         // controller board (has slaves)
  fCtrl = new TDAQcontrol( this );           // init control functiond
  AddCmdList( kSIS1100Keys );            // SIS1100-specific cmds
}

//-----------------------------------------------------------------------------
TDAQ_SIS1100::~TDAQ_SIS1100( )
{
  // Disconnect
#ifdef DAQ_SIS1100
 close(fdevAxxDxx);
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SIS1100::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  switch( key ){
  default:
    TDAQmodule::SetConfig(line,key);  // <<---   Baya
    break;
  }
}


//-----------------------------------------------------------------------------
void TDAQ_SIS1100::PostInit()
{
  // Open VMEbus device handler
  // and open descriptor for mapping virtual to physical memory
  // Failure to open the descriptor is a fatal error
#ifdef DAQ_SIS1100
  //
  // Open descriptor to device driver
  if (  (fdevAxxDxx = open( devAxxDxx, O_RDWR | O_SYNC)) == -1) 
    {                                   
      PrintError(strerror(errno),"<Open VMEbus-bridge device SIS1100>",
		 EErrFatal);
    }
#endif
}

//-----------------------------------------------------------------------------
DAQMemMap_t*  TDAQ_SIS1100::MapSlave( void* addr, Int_t size, Int_t am )
{
  // Map section of virtual memory to an address associated
  // Failure constutes a fatal error

  ULong_t vmeaddr = (ULong_t)addr;
  UInt_t paddr;                      // physical memory address
  ULong_t vaddr;                     // virtual memory (mapped from physical)
  paddr = vaddr = vmeaddr;
  DAQMemMap_t* map = new DAQMemMap_t(paddr, size, vaddr, fMemFd, this);
  return map;
}

//-------------------------------------------------------

ClassImp(TDAQ_SIS1100)
