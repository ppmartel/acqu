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

#include "TDAQ_V2718.h"


enum { EV2718SetReg=100, EV2718RdReg, EV2718IRQ };
static Map_t kV2718Keys[] = {
  {"SetReg:",       EV2718SetReg},
  {"ReadReg:",      EV2718RdReg},
  {"SetIRQ:",       EV2718IRQ},
  {NULL,           -1}
};


//-----------------------------------------------------------------------------
TDAQ_V2718::TDAQ_V2718( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation
  fCtrl = new TDAQcontrol( this );           // tack on control functions
  fType = EDAQ_Ctrl;                         // controller board (has slaves)
  AddCmdList( kV2718Keys );                  // V2718-specific cmds
  fHandle = 0;                               // descriptor
  fIsIRQ = false;                            // default no IRQ handling
  fIsIRQEnabled = kFALSE;
#ifdef DAQ_CAENV2718
  fBoard = cvV2718;                          // show its a V2718
#endif
  fAM = 0x09;
  if( line ){
    if( sscanf( line,"%*s%*s%*s%hd%hd",&fLink,&fBdNum ) != 2 )
      PrintError( line, "<Board ID parse>", EErrFatal );
  }
  else{
    fLink = 0;
    fBdNum = 0;
  }
}

//-----------------------------------------------------------------------------
TDAQ_V2718::~TDAQ_V2718( )
{
  // Disconnect
#ifdef DAQ_CAENV2718
  CAENVME_End(fHandle);
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  UInt_t reg,data;
  switch( key ){
  case EV2718SetReg:
    // General internal register write
    if( fNreg >= fMaxReg ){
      PrintError( line, "<Register store not initialised>" );
      return;
    }
    if( sscanf( line,"%x%x",&reg,&data ) != 2 )
      PrintError( line, "<Register write parse>", EErrFatal );
    fData[fNreg] = data;
    fReg[fNreg] = (void*)((ULong_t)reg);
    fNreg++;
    break;
  case EV2718RdReg:
    // General internal register read
    if( sscanf( line,"%x",&reg ) != 1 )
      PrintError( line, "<Register read parse>", EErrFatal );
    break;
  case EV2718IRQ:
    // Set in IRQ handle mode
    fIsIRQ = true;
    break;
  default:
    // try general module setup commands
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::PostInit()
{
  // Initialise PCI-VME hardware
  if( fIsInit ) return;
#ifdef DAQ_CAENV2718
  CVBoardTypes brd = (CVBoardTypes)fBoard;
  CVErrorCodes err;
  err = CAENVME_Init(brd, fLink, fBdNum, &fHandle);
  if(err != cvSuccess)
    PrintError(CAENVME_DecodeError(err),"<V2718 Initialise>",EErrFatal);
  /*
  err = CAENVME_SystemReset(brd);
  if(err != cvSuccess)
    PrintError(CAENVME_DecodeError(err),"<V2718 System Reset>",EErrFatal);
  */
  Char_t* fw;
  err = CAENVME_BoardFWRelease(fHandle,fw);
  printf(" V2718 Firmware release: %s\n",fw);
  if(fIsIRQ){
    // pulser A configuration: period 255x104ms, width 254x104ms
    // trigger on Input-0 signal, reset by software
    //err = CAENVME_SetPulserConf(fHandle,cvPulserA,255,254,
    //cvUnit104ms,1,cvInputSrc0,cvManualSW);
    err = CAENVME_SetPulserConf(fHandle,cvPulserA,255,254,
				cvUnit1600ns,0,cvInputSrc0,cvManualSW);
    if(err != cvSuccess)
      PrintError(CAENVME_DecodeError(err),
		 "<V2718 Pulser A Initialise>",EErrFatal);
    //
    err = CAENVME_SetPulserConf(fHandle,cvPulserB,10,9,
				cvUnit25ns,1,cvManualSW,cvManualSW);
    //cvUnit104ms,1,cvInputSrc0,cvManualSW);
    if(err != cvSuccess)
      PrintError(CAENVME_DecodeError(err),
		 "<V2718 Pulser B Initialise>",EErrFatal);
    //
    // Tie pulser A output to Lemo output 0
    err = CAENVME_SetOutputConf(fHandle,cvOutput0,cvDirect,
				cvActiveHigh,cvMiscSignals);
    if(err != cvSuccess)
      PrintError(CAENVME_DecodeError(err),
		 "<V2718 Output 0 Initialise>",EErrFatal);
    //
    // Tie software control to Lemo output 1
    err = CAENVME_SetOutputConf(fHandle,cvOutput1,cvDirect,
				cvActiveHigh,cvManualSW);
    if(err != cvSuccess)
      PrintError(CAENVME_DecodeError(err),
		 "<V2718 Output 1 Initialise>",EErrFatal);
    //
  }
#endif
  TDAQmodule::PostInit();
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::BlkRead()
{
#ifdef DAQ_CAENV2718
  //
  //  err = CAENVME_BLTReadCycle(long Handle, unsigned long Address,
  //				    unsigned char *Buffer, int Size,
  //				    CVAddressModifier AM,
  //				    CVDataWidth DW, int *count);
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::BlkWrite()
{
#ifdef DAQ_CAENV2718
  //
  //  err = CAENVME_BLTWriteCycle(fHandle, unsigned long Address,
  //				     unsigned char *Buffer, int size,
  //				     CVAddressModifier AM,
  //				     CVDataWidth DW, int *count);
#endif
}

ClassImp(TDAQ_V2718)
