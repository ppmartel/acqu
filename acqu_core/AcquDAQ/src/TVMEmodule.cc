//--Author	JRM Annand   30th Sep 2005  Basis of VMEbus hardware
//--Rev 	JRM Annand
//--Rev 	JRM Annand   10th Feb 2007  Integrate to Acqu++
//--Rev 	JRM Annand   11th Jan 2008  Additions for working DAQ
//--Rev 	JRM Annand   22nd Jan 2008  Write(index,data)
//--Rev         B.Oussena    1st Jan  2009  Started Update for KphBoard
//--Rev 	JRM Annand   24th Jul 2009  Multiple InitReg()
//--Rev         B.Oussena    25th Sep 2009  Fixed bug in InitReg()
//--Rev 	JRM Annand   25th Oct 2009  Multiple InitReg() more mods
//--Rev 	JRM Annand   13th Apr 2010  64-bit address void* <-> ULong_t
//--Rev 	B.Oussena     5th Sep 2010  Bug InitReg()
//--Rev 	JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fixes
//--Rev 	JRM Annand   25th Jan 2012  Read fNBits in constructor
//--Rev 	JRM Annand   20th May 2012  Check AM reg initialised
//                                          Implement "repeat" in reg init
//--Rev 	JRM Annand   24th Aug 2012  'w' = 2 bytes (not 'm')
//--Rev 	JRM Annand    7th Sep 2013  Do NOT neglect A16
//--Rev 	JRM Annand   23rd Sep 2013  Add WriteChk()
//--Update	JRM Annand    2nd Nov 2017  DMA support
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVMEmodule
// Basis of all VMEbus control/readout software
// General-purpose VMEbus hardware with no specialist read/write procedures
// may use this class
//

#include "TVMEmodule.h"
#include "TDAQexperiment.h"


enum { EVME_AMDW=100, EVME_Register, EVME_IDRegister, EVME_DMA };
static Map_t kVMEKeys[] = {
  {"Address-Data:",       EVME_AMDW},
  {"Register:",           EVME_Register},
  {"ID-Register:",        EVME_IDRegister},
  {"DMA:",                EVME_DMA},
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TVMEmodule::TVMEmodule( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation.
  // Read in base address, base readout index and # channels
  // 
  AddCmdList( kVMEKeys );                  // physics-specific cmds
  ULong_t addr;
  if( line ){
    if( sscanf( line, "%*s%*s%*s%lx%d%d%d",
		&addr,&fBaseIndex,&fNChannel,&fNBits ) < 3 )
      PrintError( line, "<VMEbus setup line parse>", EErrFatal );
    fBaseAddr = (void*)addr;
  }
  fAM = NULL;
  fIsWrt = NULL;
  fBus = E_VMEbus;
  fHardID = 0;
  fHardIDReg = -1;
  fBlkBuff = NULL;
  fBlkSize = 0;
  fBlkParm = 0;
  fIsBlk = false;
}

//-----------------------------------------------------------------------------
TVMEmodule::~TVMEmodule( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVMEmodule::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  switch( key ){
  case EVME_Register:
    // VME register offset
    InitReg( line );
    break;
  case EVME_IDRegister:
    // Register to read module identity
    if( sscanf( line, "%d", &fHardID ) != 1 )
      PrintError(line,"<Parse hardware ID>");
    fHardIDReg = fNreg - 1;
    break;
  case EVME_DMA:
    // turn on direct memory access....VMEbus controller must support this
    fIsDMA = true;
    if( sscanf( line, "%d", &fDMAsize ) != 1 )
      PrintError(line,"<Parse DMA buffer size>",EErrFatal);
    break;
  default:
    // default try commands of TDAQmodule
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TVMEmodule::InitReg( Char_t* line )
{ 
  // Parse the input line for device register information
  // Store the register

  if( !fReg ){
    PrintError( line, "<Register Setup...fMaxReg not yet specified>");
    return;
  }
  if( !fAM ) fAM = new Int_t[fMaxReg];
  if( !fIsWrt ){
    fIsWrt = new Bool_t[fMaxReg];
    for( Int_t i=0; i<fMaxReg; i++ ) fIsWrt[i] = kFALSE;
  }
  ULong_t addr;
  UInt_t datum;
  Int_t width, am, number, n; 
  if( (n = sscanf( line,"%lx%x%x%d%x",&addr,&width,&am,&datum,&number )) < 3 ){
    PrintError( line, "<Register Setup parse>");
    return;
  }
  if( n < 4 ) datum = 0;
  if( n < 5 ) number = 1;
  for( Int_t i=0; i<number; i++ ){
    if( fNreg >= fMaxReg )
      PrintError( line,"<Too many registers input>", EErrFatal);
    if( n >= 5 ) fIsWrt[fNreg] = kTRUE;
    InitReg( addr, width, am, fNreg, datum );
    addr = addr + width;
    fNreg++;
  }
}

//-----------------------------------------------------------------------------
void TVMEmodule::InitReg( VMEreg_t* list)
{ 
  // Setup set of registers

  if(!fReg){
    VMEreg_t* l = list;
    while( l->offset != 0xffffffff ) {
      fNreg += l->repeat+1; // respect the repeat property
      l++;
    }
    fMaxReg = fNreg;
    fReg = new void*[fMaxReg];
    fDW = new Int_t[fMaxReg];
    fAM = new Int_t[fMaxReg];
    fData = new UInt_t[fMaxReg];
    fIsWrt = new Bool_t[fMaxReg];
    for( Int_t i=0; i<fMaxReg; i++ ){ fData[i] = 0; fIsWrt[i] = kFALSE; }
    fprintf( fLogStream, "InitReg on list of %d registers at base addr %lx\n",
	     fNreg, (ULong_t)fBaseAddr );
  }
  // 20/05/12 check that AM code and Is-write registers initialised
  else if( !fAM ){
    fAM = new Int_t[fMaxReg];
    fIsWrt = new Bool_t[fMaxReg];
    for( Int_t i=0; i<fMaxReg; i++ ) fIsWrt[i] = kFALSE;
  }
  ULong_t addr;
  Int_t width, am, i;
  i = 0;
  while( list->offset != 0xffffffff ){
    // Bug!!    addr = list->offset + (ULong_t)fBaseAddr;
    addr = list->offset;
    if( fBaseAddr >= (void*)0x01000000 ) am = 0x09;     // 32 bit
    else if( fBaseAddr >= (void*)0x010000 ) am = 0x39;  // 24 bit
    else am = 0x29;                                     // 16 bit
    switch( list->type ){
    case 'l':
      width = 4;
      break;
    case 'w':
      width = 2;
      break;
    case 'b':
      width = 1;
      break;
    default:
      width = 4;
      break;
    }
    // 20/05/12 Implement list->repeat, ie #repeat consecutive reg address
    Int_t nr = list->repeat;
    for(Int_t n=0; n<=nr; n++){
      InitReg( addr, width, am, i, list->data );
      addr += width;
      i++;
    }
    list++;
  }
}

//-----------------------------------------------------------------------------
void TVMEmodule::InitReg( UInt_t addr, Int_t width, Int_t am, Int_t i,
			  UInt_t datum )
{
  // Store complete VMEbus address spec and (optional) datum to write
  fReg[i] =  (Char_t*)fBaseAddr + addr;
  fDW[i] = width;
  fAM[i] = am;
  fData[i] = datum;
}

//-----------------------------------------------------------------------------
void TVMEmodule::PostInit( )
{
  // Post-configuration setup of hardware
  // If the primary controller uses memory mapping create the map here
  // Write stored data to hardware
  //
  if( fIsInit ) return;
  if( fMapSize )
    fMemMap = fPCtrlMod->MapSlave( fBaseAddr, fMapSize, fAM[0] );

  TDAQmodule::PostInit();

  if( !CheckHardID() ) return;
  if( fNreg != fMaxReg )
    PrintError("VME registers","<Not all VMEbus registers defined>");
  for(Int_t i=0; i<fNreg; i++){
    if( fIsWrt[i] )
      Write( i );
  }
  if(fIsDMA){
    fMemDMA = fPCtrlMod->InitDMA();          // buffer address
    fPCtrlMod->LinkDMA(fBaseAddr, fDMAsize); // this address, max transfer size
    fDMAnPkt = fPCtrlMod->GetDMAnPkt();      // position on DMA linked list
  }
}

//-----------------------------------------------------------------------------
Bool_t TVMEmodule::CheckHardID( )
{
  // Default hardware ID read. If not setup return true.
  if( fHardIDReg == -1 ) return kTRUE;
  if( Read(fHardIDReg) != (UInt_t)fHardID )
    PrintError("","<VMEbus module ID not recognised>", EErrFatal);
  return kTRUE;    
}

ClassImp(TVMEmodule)
