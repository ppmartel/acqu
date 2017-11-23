//--Author	JRM Annand     21st Jan 2006  
//--Rev 	JRM Annand...
//--Rev         B.Oussena       1st Jan Start updates for KpHBoard
//--Rev 	JRM Annand..   28th Apr 2009  memory mapping
//--Rev         B. Oussena     10th Jul 2009  Add  cmd EKPhI686Board
//--Rev         B. Oussena     15th Jul 2009  Add Init()
//--Rev         B. Oussena     20th Jul 2009  fix bugs in SetConfig(), MapMem()
//--Rev         B. Oussena     3rd Mar 2010   Set KpHboard  to perform 32/24bits
//--Rev         B. Oussena     17th Sep 2010  VME addr modifiers to ModIndex.h
//--Update      JRM Annand      8th Jul 2013  32-bit addr set range
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_KPhI686
// Mainz Pentium M based VMEbus SBC
// "Direct bridge" to VMEbus, addresses by firmware
// Default setup 32-bit addressing to be bits 28-31 = 0xe
// Other rangles, ie bits 28-31 = 0x1,2,3..... can be chosen by setting
// the fRange variable to 0x10000000,0x20000000....
// All 32-bit address slaves in the crate must follow that address scheme

#include "TDAQ_KPhI686.h"


extern "C"{
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
}

enum { EKPhI686Board, EKPhI686Range };
static Map_t kKPhI686Keys[] = {
  {"Board:",       EKPhI686Board},
  {"Range:",       EKPhI686Range},
  {NULL,           -1}
};


//-----------------------------------------------------------------------------
TDAQ_KPhI686::TDAQ_KPhI686( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation
  fType = EDAQ_Ctrl;                         // controller board (has slaves)
  fCtrl = new TDAQcontrol( this );           // init control functiond
  AddCmdList( kKPhI686Keys );                // KPhI686-specific cmds
  fMemFd = -1;
  fRange = 0xe0000000;                       // default e-range
  Init();  //  <---- Baya
}

//-----------------------------------------------------------------------------
TDAQ_KPhI686::~TDAQ_KPhI686( )
{
  // Disconnect
  if( fMemFd != -1 ) close( fMemFd );
}

//-----------------------------------------------------------------------------
void TDAQ_KPhI686::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  switch( key ){
  case EKPhI686Range:  // switch to another range (addr bits 28-31)
    if( sscanf(line,"%x",&fRange) != 1 )
      PrintError(line,"<Parse error address range>", EErrFatal);
    break;
  case EKPhI686Board:  // board ID <<--------------   Added by Baya
#ifdef VME_HOST
    void *BaseReg;
    Long_t *VirtMemReg;
    DAQMemMap_t *MemReg;

    BaseReg = (void*)0xaa000000;
    MemReg = new DAQMemMap_t(BaseReg, 0x1000, fMemFd, this); 
    
    VirtMemReg= (Long_t *)MemReg->GetVirtAddr();

    *VirtMemReg= 0xe0000000 & fRange; //save 3 first bits of FBaseAddr
#endif
    break;

  default:
    //PrintError(line,"<Unrecognised configuration keyword>"); <<-- Baya
    TDAQmodule::SetConfig(line,key);  // <<---   Baya
    break;
  }
}


//-----------------------------------------------------------------------------
void TDAQ_KPhI686::Init()
{
#ifdef VME_HOST
  // Store VMEbus start addresses
  // and open descriptor for mapping virtual to physical memory
  // Failure to open the descriptor is a fatal error
  if( (fMemFd = open("/dev/mem", O_RDWR)) == -1 )
    PrintError("","<Virtual memory descriptor open>", EErrFatal);
#endif
}

//-----------------------------------------------------------------------------
DAQMemMap_t*  TDAQ_KPhI686::MapSlave( void* addr, Int_t size, Int_t am )
{
  // Map section of virtual memory to an address associated
  // with the VMEbus.
  // STD = A24... parm[0] = 0
  // EXT = A32... parm[0] = 1
  // SIO = A16... parm[0] = 2
  // parm[1] = VMEbus start address. Must be on page boundary
  // parm[2] = length of map in bytes. Must be integral multiple of page size.
  // Failure constutes a fatal error

  void* paddr;                      // physical memory address

  Long_t adr;
  adr = (Long_t)addr;               // set to zero first 3bits for 32Bits
  addr = (void*)(adr & 0x1fffffff); // to allow adr mode access 24/32 Bits

  switch( am ){
  case EVME_A32:
    paddr = (Char_t*)addr + EVMEbusA32;
    break;
  case EVME_A24:
  default:
    paddr = (Char_t*)addr + EVMEbusA24;
    break;
  case EVME_A16:
    paddr = (Char_t*)addr + EVMEbusA16;
    break;
  }
  DAQMemMap_t* map = new DAQMemMap_t(paddr, size, fMemFd, this); //  <-- Baya
  return map;
}

//-------------------------------------------------------
ClassImp(TDAQ_KPhI686)
