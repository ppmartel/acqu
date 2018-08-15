//--Author	JRM Annand      6th Oct 2016  
//--Rev 	JRM Annand...
//--Rev         JRM Annand     14th Oct 2016 1st running version   
//--Update      JRM Annand     26th Oct 2017 Include DMA option  
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_VP2ExX86_64
// Concurrent Tech. VMEbus SBC quad Atom X86_64 processor 1.9 GHz
// Universe II bridge" to VMEbus

#include "TDAQ_VPE2xX86_64.h"


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <curses.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

enum { EVPE2xX86_64_A32, EVPE2xX86_64_A24, EVPE2xX86_64_A16, EVPE2xX86_64_SWAP,
       EVPE2xX86_64_DMA, EVPE2xX86_64_DMApkt  };
static Map_t kVPE2xX86_64Keys[] = {
  {"MAP:",       EVPE2xX86_64},
  {"SWAP:",      EVPE2xX86_64_SWAP},
  {"DMA:",       EVPE2xX86_64_DMA},
  /*
  {"DMApkt:",    EVPE2xX86_64_DMApkt},
  */
  {NULL,           -1}
};

extern int errno;

//-----------------------------------------------------------------------------
TDAQ_VPE2xX86_64::TDAQ_VPE2xX86_64( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation
  fType = EDAQ_Ctrl;                         // controller board (has slaves)
  fCtrl = new TDAQcontrol( this );           // init control functiond
  AddCmdList( kVPE2xX86_64Keys );            // VPE2xX86_64-specific cmds
  fNImage = 0;                               // no image maps defined yet
  fMemFd = -1;
  fDevCTL = -1;                              // vme device handlers not init
  for(Int_t i=0; i<ENImage; i++){
    fDev[i] = -1;
    fVMEaddr[i] = 0x0;
    fVMEsize[i] = 0x0;
    fPCIaddr[i] = 0x0;
    fDevName[i] = NULL;
    fDev[i] = -1;
  }
  fByteSwap = 0;   // swapping off
  fIsDMA = false;  // DMA needs to be turned on
}

//-----------------------------------------------------------------------------
TDAQ_VPE2xX86_64::~TDAQ_VPE2xX86_64( )
{
  // Disconnect
#ifdef DAQ_CCTVMEEN
  Int_t result;
  for(Int_t i=0; i<fNImage; i++){
    result = vme_disablePciImage( fDev[i] );
    if( result < 0 )
      PrintError(strerror(errno),"< Failed to disable PCI image >");
    vme_closeDevice(fDev[i]);
  }
  vme_closeDevice(fDevCTL);
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_VPE2xX86_64::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  Int_t ic;
  UInt_t iv,is,ip,ix;
  Char_t devN[8];
  switch( key ){
  case EVPE2xX86_64:      // PCI image for VME access
    ix = 0;               // default SCT
    if( sscanf(line,"%d %x %x %x %s %d",&ic,&iv,&is,&ip,devN,&ix) < 5 )
      PrintError(line,"<Parse error VME - PCI map>", EErrFatal);
    fAMcode[fNImage] = ic;
    fVMEaddr[fNImage] = iv;
    fVMEsize[fNImage] = is;
    fPCIaddr[fNImage] = ip;
    fDevName[fNImage] = new Char_t[8];
    strcpy(fDevName[fNImage],devN);
    fVMEcycle[fNImage] = ix;
    fNImage++;
    break;
  case EVPE2xX86_64_SWAP: // byte swapping by Universe II
    // bit 3 set: master VME byte swap set
    // bit 4 set: slave VME byte swap set
    // bit 5 set: fast write enables
    if( sscanf(line,"%x",&fByteSwap) != 1 )
      PrintError(line,"<Parse error byte swapping switch>", EErrFatal);
    break;
  case EVPE2xX86_64_DMA: // DMA transfer parameters
    if( sscanf(line,"%x%x%x",&fDMAdwidth,&fDMAvmecycle,&fDMAownership) != 3 )
      PrintError(line,"<Parse error DMA parameters>", EErrFatal);
    fIsDMA = true;
    break;
  default:
    //PrintError(line,"<Unrecognised configuration keyword>"); <<-- Baya
    TDAQmodule::SetConfig(line,key);  // <<---   Baya
    break;
  }
}


//-----------------------------------------------------------------------------
void TDAQ_VPE2xX86_64::PostInit()
{
  // Open Universe-II PCI-VMEbus device handlers
  // and open descriptor for mapping virtual to physical memory
  // Failure to open the descriptor is a fatal error
  // Optionally setup DMA transfer in linked-list mode
#ifdef DAQ_CCTVMEEN
  //
  Int_t result;

  // Byte swapping...open ctl device for basic driver control
  fDevCTL = vme_openDevice((char*)"ctl");
  if(fDevCTL < 0)
    PrintError(strerror(errno),"<Open VMEbus-bridge device CTL >", EErrFatal);
  result = vme_setByteSwap( fDevCTL, (Char_t)fByteSwap);
  if( result < 0 )
    PrintError(strerror(errno),"<Byte swapping device CTL >", EErrFatal);
  result = vme_closeDevice(fDevCTL);
  if( result < 0 )
    PrintError(strerror(errno),"<Close VMEbus-bridge device CTL >", EErrFatal);
  //
  // VMEbus access image setup
  for(Int_t i=0; i<fNImage; i++){
    Int_t width, space;
    switch(fAMcode[i]){
    case 16:
      width = EN_VME_D16;
      space = EN_VME_A16;
      break;
    case 24:
      width = EN_VME_D16;
      space = EN_VME_A24;
      break;
    case 32:
      width = EN_VME_D32;
      space = EN_VME_A32;
      break;
    default:
      break;
    }
    fDev[i] = vme_openDevice( fDevName[i] );
    if( fDev[i] < 0 )
      PrintError(strerror(errno),"<Open VMEbus-bridge device >", EErrFatal);
    if( strncmp(fDevName[i],"lsi",3) == 0 ){
      memset((char *)(fPCIData+i),0,sizeof(EN_PCI_IMAGE_DATA));
      fPCIData[i].pciAddress = fPCIaddr[i];
      fPCIData[i].pciAddressUpper = 0;
      fPCIData[i].vmeAddress = fVMEaddr[i];
      fPCIData[i].vmeAddressUpper = 0;
      fPCIData[i].size = fVMEsize[i];
      fPCIData[i].sizeUpper = 0;
      fPCIData[i].readPrefetch = 0;
      fPCIData[i].prefetchSize = 0;
      fPCIData[i].postedWrites = 1;
      fPCIData[i].dataWidth = width;
      fPCIData[i].addrSpace = space;
      fPCIData[i].type = EN_LSI_DATA;
      fPCIData[i].mode = EN_LSI_USER;
      fPCIData[i].vmeCycle = fVMEcycle[i]; // 0 SCT, 1 BLT, 2 MBLT
      fPCIData[i].sstMode = 0;
      fPCIData[i].vton = 0;                 // unused
      fPCIData[i].vtoff = 0;                // unused
      fPCIData[i].sstbSel = 0;              // TSI148
      fPCIData[i].pciBusSpace = 0;          // 0-mem 1-IO
      fPCIData[i].ioremap = 0;
      result = vme_enablePciImage( fDev[i], &fPCIData[i] );
    }
    else{
      PrintError(fDevName[i],"<Unknown PCI-VME image device>",EErrFatal);
    }
    if( result < 0 )
      PrintError(strerror(errno),"<Enable PCI-VME image>", EErrFatal);
  }
#endif
}

//-----------------------------------------------------------------------------
DAQMemMap_t*  TDAQ_VPE2xX86_64::MapSlave( void* addr, Int_t size, Int_t am )
{
  // Map section of virtual memory to a VMEbus address associated with
  // a VME slave module 
  // Failure constutes a fatal error

  ULong_t vmeaddr = (ULong_t)addr;
  Int_t iv = -1;
  for(Int_t i=0; i<fNImage; i++){
    if((vmeaddr >= fVMEaddr[i]) && (vmeaddr < fVMEaddr[i]+fVMEsize[i])){
      iv = i;
      break;
    }
  }
  if( iv == -1)
    PrintError(NULL,"<VMEbus address not defined on PCI map>",EErrFatal);
  //
  UInt_t paddr;                      // physical memory address
  ULong_t vaddr;                     // virtual memory (mapped from physical)
  Int_t result;
#ifdef DAQ_CCTVMEEN
  paddr = vmeaddr - fVMEaddr[iv];
  result = vme_mmap( fDev[iv], paddr, (ULong_t)size, &vaddr );
  if( result < 0 )
    PrintError(strerror(errno),"<vme_mmap Memory map PCI image >", EErrFatal);
#endif    
  DAQMemMap_t* map = new DAQMemMap_t(paddr, size, vaddr, fMemFd, this);
  return map;
}

//-----------------------------------------------------------------------------
void* TDAQ_VPE2xX86_64::InitDMA(Int_t buffsize)
{
  // Initialise linked-list DMA transfer
  // If buffer size not specified, use default 4096 byte = 1 memory page
  //
  if( !fIsDMA )
    PrintError("","<DMA not enabled for this device>",EErrFatal);
  // if DMA buffer already mapped just return the address + offset
  if(fMemDMA)
    return (Char_t*)fMemDMA + fDMAoffset;
#ifdef DAQ_CCTVMEEN
  fDevDMA = vme_openDevice( (Char_t*)"dma0" );
  if( fDevDMA < 0 )
    PrintError(strerror(errno),"<Open VMEbus-DMA device >", EErrFatal);
  ULong_t size = fDMAsize = buffsize;
  Int_t result = vme_allocDmaBuffer( fDevDMA, &size );
  if( result < 0 )
    PrintError(strerror(errno),"<Allocate DMA buffer>", EErrFatal);
  // Create data transfer struct
  memset((char*)&fDMAtxfer,0,sizeof(EN_VME_TXFER_PARAMS));
  fDMAtxfer.timeout = 100;                  // timeout in ticks 
  fDMAtxfer.vmeBlkSize = TSI148_4096;
  fDMAtxfer.vmeBackOffTimer = 0;
  fDMAtxfer.pciBlkSize = TSI148_4096;
  fDMAtxfer.pciBackOffTimer = 0;
  fDMAtxfer.ownership = fDMAownership;
  fDMAoffset = 0;		
  //
  ULong_t mem;
  // Map the DMA buffer into user space
  result = vme_mmap( fDevDMA, 0, size, &mem );
  if( result < 0 )
    PrintError(strerror(errno),"<Memory-map DMA buffer>", EErrFatal);
  fMemDMA = (void*)mem;
#endif    
  return fMemDMA;
}

//-----------------------------------------------------------------------------
void TDAQ_VPE2xX86_64::LinkDMA(void* addr, Int_t size, Int_t offset)
{
  // Initialise linked-list DMA transfer
  // If buffer size = 0, use default 1024 byte
#ifdef DAQ_CCTVMEEN
  EN_VME_CMD_DATA dmaData;
  memset((char*)&dmaData,0,sizeof(EN_VME_CMD_DATA));
  if(offset < 0)
    offset = fDMAoffset;
  dmaData.direction = 0;
  dmaData.vmeAddress = (ULong_t)(addr);
  dmaData.size = size;
  dmaData.offset = offset;
  dmaData.access.dataWidth = fDMAdwidth;
  dmaData.access.addrSpace = EN_VME_A32;
  dmaData.access.type = EN_LSI_DATA;     // data AM code 
  dmaData.access.mode = EN_LSI_USER;     // non-privileged
  dmaData.access.vmeCycle = fDMAvmecycle;// 0 = SCT, 1 = BLT...BLT causes error
  Int_t result = vme_addDmaCmdPkt(fDevDMA,&dmaData);
  if(result < 0)
    PrintError(strerror(errno),"<Add DMA command packet>", EErrFatal);
  fDMAnPkt++;
  fDMAoffset += size;
#endif
    return;
}

ClassImp(TDAQ_VPE2xX86_64)
