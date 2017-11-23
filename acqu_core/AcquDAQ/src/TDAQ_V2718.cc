//--Author	JRM Annand   20th Dec 2005
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 5th Dec 2007 Int_t fHandle for libCAENVME
//--Rev 	JRM Annand... 3rd Jun 2008  Conditional compilation
//--Update	JRM Annand... 6th Jul 2011  gcc4.6-x86_64 warning fix
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_V2718
// CAEN A2818 PCI <--> V2718 VMEbus controller via optical fibre
// Assumes that a2818 linux driver is loaded
// Uses control functions from libCAENVME.so

#include "TDAQ_V2718.h"


enum { EV2718SetReg=100, EV2718RdReg };
static Map_t kV2718Keys[] = {
  {"SetReg:",       EV2718SetReg},
  {"ReadReg:",      EV2718RdReg},
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
#ifdef AD_CAENVME
  fBoard = cvV2718;                          // show its a V2718
#endif
  fAM = 0x09;
  if( line ){
    if( sscanf( line,"%*s%*s%*s%hd%hd",&fLink,&fBdNum ) != 2 )
      PrintError( line, "<Board ID parse>", EErrFatal );
  }
}

//-----------------------------------------------------------------------------
TDAQ_V2718::~TDAQ_V2718( )
{
  // Disconnect
#ifdef AD_CAENVME
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
#ifdef AD_CAENVME
  fErrorCode = CAENVME_Init(fBoard, fLink, fBdNum, &fHandle);
#endif
  TDAQmodule::PostInit();
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::BlkRead()
{
#ifdef AD_CAENVME
  //
  //  fErrorCode = CAENVME_BLTReadCycle(long Handle, unsigned long Address,
  //				    unsigned char *Buffer, int Size,
  //				    CVAddressModifier AM,
  //				    CVDataWidth DW, int *count);
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_V2718::BlkWrite()
{
#ifdef AD_CAENVME
  //
  //  fErrorCode = CAENVME_BLTWriteCycle(fHandle, unsigned long Address,
  //				     unsigned char *Buffer, int size,
  //				     CVAddressModifier AM,
  //				     CVDataWidth DW, int *count);
#endif
}

ClassImp(TDAQ_V2718)
