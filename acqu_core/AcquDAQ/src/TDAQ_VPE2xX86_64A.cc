//--Author	JRM Annand      6th Oct 2016  
//--Rev 	JRM Annand...
//--Update      JRM Annand     14th Oct 2016 1st running version   
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_VP2ExX86_64A
// Concurrent Tech. VMEbus SBC quad Atom X86_64 processor 1.9 GHz
// Universe II bridge" to VMEbus

#include "TDAQ_VPE2xX86_64A.h"


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

extern int errno;

//-----------------------------------------------------------------------------
TDAQ_VPE2xX86_64A::TDAQ_VPE2xX86_64A( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQ_VPE2xX86_64( name, file, log, line )
{
  /*
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
#ifdef DAQ_CCTVMEEN
    memset((char *)(fBrData+i),0,sizeof(EN_PCI_IMAGE_DATA));
    memset((char *)(fBrDataV+i),0,sizeof(EN_VME_IMAGE_DATA));
#endif
  }
  fByteSwap = 0;   // swapping off
  */
}
/*
//-----------------------------------------------------------------------------
TDAQ_VPE2xX86_64A::~TDAQ_VPE2xX86_64A( )
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
*/
/*
//-----------------------------------------------------------------------------
void TDAQ_VPE2xX86_64A::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  Int_t ic;
  UInt_t iv,is,ip;
  Char_t devN[8];
  switch( key ){
  case EVPE2xX86_64:      // PCI image for VME access
    if( sscanf(line,"%d %x %x %x %s",&ic,&iv,&is,&ip,devN) != 5 )
      PrintError(line,"<Parse error VME - PCI map>", EErrFatal);
    fAMcode[fNImage] = ic;
    fVMEaddr[fNImage] = iv;
    fVMEsize[fNImage] = is;
    fPCIaddr[fNImage] = ip;
    fDevName[fNImage] = new Char_t[8];
    strcpy(fDevName[fNImage],devN);
    fNImage++;
    break;
  case EVPE2xX86_64_SWAP: // byte swapping by Universe II
    // bit 3 set: master VME byte swap set
    // bit 4 set: slave VME byte swap set
    // bit 5 set: fast write enables
    if( sscanf(line,"%x",&fByteSwap) != 1 )
      PrintError(line,"<Parse error byte swapping switch>", EErrFatal);
    break;
  default:
    //PrintError(line,"<Unrecognised configuration keyword>"); <<-- Baya
    TDAQmodule::SetConfig(line,key);  // <<---   Baya
    break;
  }
}
*/

//-----------------------------------------------------------------------------
void TDAQ_VPE2xX86_64A::PostInit()
{
  // Open VMEbus device handler
  // and open descriptor for mapping virtual to physical memory
  // Failure to open the descriptor is a fatal error
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
    default:
      break;
    }
    fDev[i] = vme_openDevice( fDevName[i] );
    if( fDev[i] < 0 )
      PrintError(strerror(errno),"<Open VMEbus-bridge device >", EErrFatal);
    if( strncmp(fDevName[i],"lsi",3) == 0 ){
      fPCIData[i].pciAddress = fPCIaddr[i];
      fPCIData[i].pciAddressUpper = 0;
      fPCIData[i].vmeAddress = fVMEaddr[i];
      fPCIData[i].vmeAddressUpper = 0;
      fPCIData[i].size = fVMEsize[i];
      fPCIData[i].sizeUpper = 0;
      fPCIData[i].readPrefetch = 0;
      fPCIData[i].prefetchSize = 0;
      fPCIData[i].postedWrites = 0;
      fPCIData[i].dataWidth = width;
      fPCIData[i].addrSpace = space;
      fPCIData[i].type = EN_LSI_DATA;
      fPCIData[i].mode = EN_LSI_USER;
      fPCIData[i].vmeCycle = 0;
      fPCIData[i].sstMode = 0;
      fPCIData[i].vton = 0;                 // unused
      fPCIData[i].vtoff = 0;                // unused
      fPCIData[i].sstbSel = 0;              // TSI148
      fPCIData[i].pciBusSpace = 0;          // 0-mem 1-IO
      fPCIData[i].ioremap = 1;
      result = vme_enablePciImage( fDev[i], &fPCIData[i] );
    }
    else{
      PrintError("","<vme device not permitted>",EErrFatal);
    }
    if( result < 0 )
      PrintError(strerror(errno),"<Enable VME-PCI image>", EErrFatal);
  }
  //
  //
#endif
}

//-----------------------------------------------------------------------------
DAQMemMap_t*  TDAQ_VPE2xX86_64A::MapSlave( void* addr, Int_t size, Int_t am )
{
  // Map section of virtual memory to an address associated
  // Failure constutes a fatal error

  ULong_t vmeaddr = (ULong_t)addr;
  Int_t iv = -1;
  for(Int_t i=0; i<fNImage; i++){
    if((vmeaddr >= fVMEaddr[i]) && (vmeaddr < fVMEaddr[i]+fVMEsize[i])){
      iv = i;
      fDevAM[am] = iv;
      break;
    }
  }
  if( iv == -1)
    PrintError(NULL,"<VMEbus address not defined on PCI map>",EErrFatal);
  //
  UInt_t paddr;                      // physical memory address
  ULong_t vaddr;                     // virtual memory (mapped from physical)
  vaddr = paddr = vmeaddr - fVMEaddr[iv];
  DAQMemMap_t* map = new DAQMemMap_t(paddr, size, vaddr, fMemFd, this);
  return map;
}

//-------------------------------------------------------

ClassImp(TDAQ_VPE2xX86_64A)
