//--Author	JRM Annand   30th Sep 2005  Start AcquDAQ
//--Rev 	JRM Annand... 1st Oct 2005  New DAQ software
//--Rev 	JRM Annand...10th Feb 2006  1st hardware test version
//--Rev 	JRM Annand... 9th Jun 2007  Mk2 Module indices, & error block
//--Rev 	JRM Annand...11th Jan 2008  Additions for working DAQ
//--Rev 	JRM Annand...29th Apr 2009  Memory mapping stuff
//--Rev 	JRM Annand...24th Jul 2009  Store fEXP pointer
//--Rev 	JRM Annand...19th Jul 2010  Mods for Mk1 data format
//--Rev 	JRM Annand...20th Jul 2010  Dummy trigger ctrl functions
//--Rev 	Baya Oussena..3rd Aug 2010  Chg Rd format for fHardID in SetConfig (%x)
//--Rev         B. Oussena    3rd Aug 2010  Fix bugs in SetConfig() & PostInit()
//--Rev         JRM Annand    4th Sep 2010  Chk Mk1/Mk2 data, fBaseIndex setting
//--Rev         JRM Annand    6th Sep 2010  Add GetEventID()
//--Rev         JRM Annand    9th Sep 2010  kModTypes."NULL"=undefined
//--Rev         JRM Annand   11th Sep 2010  kModTypes.add EDAQ_VADC,EDAQ_VScaler
//--Rev         JRM Annand   12th Sep 2010  ReadHeader(mod)..debug
//--Rev         JRM Annand   13th Sep 2010  ErrorStore Mk1 ensure totally init
//--Rev         JRM Annand   17th Sep 2010  fVirtOffset and GetVirtAddr()
//--Rev         JRM Annand   12th May 2011  Incr. error counters in fEXP
//--Rev         JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fix
//--Rev         JRM Annand   21st May 2012  Default ReadIRQSlow...do nothing
//--Rev         JRM Annand   28th Aug 2012  SendEventID()
//--Rev         JRM Annand   29th Aug 2012  Add fNScalerChan
//--Rev         JRM Annand    2nd Sep 2012  Add fEventSendMod
//--Rev         JRM Annand    3rd Sep 2012  Module types changes SetConfig
//--Rev         JRM Annand    6th Jun 2013  Save fBaseIndex in error block
//--Rev          A Neiser...  6th Jun 2013  Make char_t* const
//--Update      JRM Annand   26th Oct 2017  Add prototypes for DMA
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQmodule
// Basic characteristics of all DAQ hardware
// Classes to characterise all programable electronics derive from this
//

#include "TDAQmodule.h"
#include "TA2Mk2Format.h"
#include "TA2Mk1Format.h"
#include "TDAQexperiment.h"


enum { EModBaseAddr, EModRegister, EModPostInit, EModBaseIndex, EModHardID };
static Map_t kModuleKeys[] = {
  {"BaseSetup:",          EModBaseAddr},
  {"Register:",           EModRegister},
  {"Init:",               EModPostInit},
  {"BaseIndex:",          EModBaseIndex},
  {"Hardware-ID:",        EModHardID},
  {NULL,                  -1}
};

// Types of register function
static Map_t kRegTypes[] = {
  { "Init",         EModInit },         // initialise
  { "IRQ",          EModIRQ },          // per-event readout
  { "Test",         EModTest },         // testing
  {NULL,                  -1}
};
// Types of module
static Map_t kModTypes[] = {
  { "ADC",         EDAQ_ADC },          // ADC...read every event
  { "Scaler",      EDAQ_Scaler },       // Scaler...read every n events
  { "SlowCtrl",    EDAQ_SlowCtrl },     // Slow Control
  { "P-Ctrl",      EDAQ_Ctrl },         // Primary controller
  { "S-Ctrl",      EDAQ_SCtrl },        // Secondary controller
  { "IRQ",         EDAQ_IRQ },          // Trigger control
  { "VADC",        EDAQ_VADC },         // ADC not directly read
  { "VScaler",     EDAQ_VScaler },      // Scaler not directly read
  { "ADC-Scaler",  EDAQ_ADC_Scaler },   // Both ADC and Scaler functions
  { "NULL",        EDAQ_Undef },        // Undefined
  { NULL,          EDAQ_Undef }         // undefined
};

//-----------------------------------------------------------------------------
TDAQmodule::TDAQmodule(const Char_t* name, const Char_t* file, FILE* log ):
  TA2System( name, kModuleKeys, file, log )
{
  // Basic initialisation
  fPCtrlMod = fCtrlMod = fEventSendMod = NULL;
  fCtrl = NULL;
  fMemMap = NULL;
  fBaseAddr = NULL;
  fVirtOffset = 0;
  fReg = fInitReg = fTestReg = fIRQReg = NULL;
  fDW = NULL;
  fData = NULL;
  fMapSize = 0;
  fMaxReg = fNreg = fNInitReg = fNTestReg = fNIRQReg = 0;
  fErrorCode = 0;
  fBus = E_BusUndef;
  fID = EID_Undef;
  fType = EDAQ_Undef;
  fBaseIndex = -1;             // unphysical number to show not initialised
  fNChannel = fNScalerChan = 0;
  fNBits = 0;
  fInitLevel = EExpInit0;
  fIsError = kFALSE;
  // initialise no DMA
  fIsDMA = false;
  fDMAsize = 0;
  fDMAtrans = NULL;
  fDMAaddr = NULL;
  fDMAnPkt = 0;
  fMemDMA = NULL;
}

//-----------------------------------------------------------------------------
TDAQmodule::~TDAQmodule( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TDAQmodule::SetConfig( Char_t* line, Int_t key )
{
  // Basic module configuration from a line of text (file or tty)
  // The line is already parsed for a keyword, which sets key

  UInt_t addr;
  Int_t base;
  Int_t maxreg;
  Int_t mapsize = 0;
  Int_t type;
  Char_t modtype[16];
  switch( key ){
  case EModBaseAddr:
    // Base address, maximum number of registers and module type
    if( sscanf( line, "%x%d%s%x", &addr, &maxreg, modtype, &mapsize ) < 3 ){
      PrintError( line, "<Base address parse>");
      return;
    }
    // Set memory map size if not already done
    if( !fMapSize ) fMapSize = mapsize;
    // Set base address ONLY if not previously done
    if( !fBaseAddr )fBaseAddr = (void*)((ULong_t)addr);
    // Save maximum registers ONLY if not previously set
    if( fNreg > maxreg ) maxreg = fNreg;
    if( !fMaxReg ) fMaxReg = maxreg;
    else fMaxReg += maxreg;
    // prevent allocating "zero" memory (fMaxReg==0), 
    // this ensures that if(!fReg)
    if(fMaxReg>0) {
      fReg = new void*[fMaxReg];
      fDW = new Int_t[fMaxReg];
      fData = new UInt_t[fMaxReg];
    }
    for( Int_t i=0; i<fMaxReg; i++ ) fData[i] = 0;
    // Update type....input "NULL" if no change from default set in constructor
    type = Map2Key( modtype, kModTypes );
    if( type == -1 ){
      fprintf(fLogStream,"<Unknown module type %s>\n",modtype);
      type = EDAQ_Undef;
    }
    fType = fType | type;
    break;
  case EModRegister:
    // register offset
    InitReg( line );
    break;
  case EModPostInit:
    // Post-configuration initialisation
    PostInit( );
    break;
  case EModBaseIndex:
    // Base index of module # readout channels and # significant bits in
    // readout word
    if( sscanf(line,"%d%d%d",&base,&fNChannel, &fNBits) < 3 )
      PrintError(line,"<Parse Base Index, Channels, Bits>");
    if( fBaseIndex == -1 ) fBaseIndex = base;
    break;
  case EModHardID:
    // Hardware identification code, stored in module
    if( sscanf(line,"%x",&fHardID) < 1 ) //<<--------- chg format from d to x Baya
      PrintError(line,"<Parse hardware ID code>");
    break;
  default:
    PrintError(line,"<Unrecognised configuration keyword>");
    break;
  }
}

//-----------------------------------------------------------------------------
void TDAQmodule::InitReg( Char_t* line )
{ 
  // Parse the input line for device register information
  // Store the register

  if( !fReg ){
    PrintError( line, "<Register Setup...fMaxReg not yet specified>");
    return;
  }
  void* absaddr;
  UInt_t addr, datum;
  Int_t width, number, n; 
  if( (n = sscanf( line, "%x%x%x%d", &addr,&width,&datum,&number )) < 2 ){
    PrintError( line, "<Register Setup parse>");
    return;
  }
  if( n < 3 ) datum = 0;
  if( n < 4 ) number = 0;
  absaddr = ( (Char_t*)fBaseAddr + addr );
  for(Int_t i = 0; i<=number; i++){
    fReg[fNreg] = absaddr;
    fDW[fNreg] = width;
    if( number ) fData[fNreg] = datum;
    fNreg++;
    if( i >= (number-1) ) break;
  }
}

//-----------------------------------------------------------------------------
void TDAQmodule::MapReg( )
{
  // Convert VMEbus to Virtual-memory register addresses
  // Check if the current module or its immediate controller has
  // memory map....if so convert register addresses

  DAQMemMap_t* map = NULL;
  Long_t baddr;
  if( fMemMap ){
    map = fMemMap;
    baddr = (Long_t)fBaseAddr;
  }
  else if( fCtrlMod ){
    map = fCtrlMod->GetMemMap();
    baddr = (Long_t)(fCtrlMod->GetBaseAddr());
  }
  if( !map ) return;
  fVirtOffset = (Long_t)(map->GetVirtAddr()) - baddr;
  for( Int_t i=0; i<fNreg; i++ ){
    fReg[i] = (Char_t*)fReg[i] + fVirtOffset;
  }
}
 
//-----------------------------------------------------------------------------
void TDAQmodule::PostInit( )
{ 
  // Default 2nd phase initialisation
  if( fIsInit ) return;
  // default system stuff...set flags etc.
  MapReg();
  TA2System::PostInit();
}

//-----------------------------------------------------------------------------
void TDAQmodule::SetCtrl( TDAQmodule* ctrl )
{ 
  // Save pointer to the module which controls this one...
  // only if the current module is not a primary controller.
  // Check if the supplied module is itself a controller
  // and also that control functions are implemented

  // Current is Primary
  if( IsType( EDAQ_Ctrl ) ){
    PrintError("","<Cannot control Primary-control module>");
    fCtrlMod = NULL;
    fPCtrlMod = NULL;
    return;
  }
  // Current is Secondary
  if( ( ctrl->IsType(EDAQ_Ctrl) || ctrl->IsType(EDAQ_SCtrl) )
      && ctrl->fCtrl != NULL ){
    fCtrlMod = ctrl;
    fCtrl = fCtrlMod->fCtrl;
    if( ctrl->fPCtrlMod )
      fPCtrlMod = ctrl->fPCtrlMod;
    else
      fPCtrlMod = ctrl;
  }
  // Detected inconsistencies on characteristics of supplied controller module
  // Reset control module pointers to zero
  else{
    PrintError("","<Invalid control class supplied>");
    fPCtrlMod = NULL;
    fCtrlMod = NULL;
    fCtrl = NULL;
  }
  return;
}


//-----------------------------------------------------------------------------
Int_t TDAQmodule::GetRegType( Char_t* type )
{
  // Return key showing type of register
  //
  return Map2Key( type, kRegTypes );
}

//-----------------------------------------------------------------------------
Int_t TDAQmodule::GetModType( Char_t* type )
{
  // Return key showing type of module
  //
  return Map2Key( type, kModTypes );
}

//-----------------------------------------------------------------------------
void TDAQmodule::ReadHeader( ModuleInfoMk2_t* mod )
{
  // Return key showing type of module
  // Mk2 format data
  //
  mod->fModID = fID;		        // Acqu hardware index of module
  mod->fModIndex = fIndex;              // list index of module
  // type of module, ADC, Scaler control etc.
  switch ( fType ){
  case EDAQ_VADC:
    // Virtual ADC (indirectly read)....tell AcquRoot its an ADC
    mod->fModType = EDAQ_ADC;
    break;
  case EDAQ_VScaler:
    // Virtual Scaler (indirectly read)....tell AcquRoot its a Scaler
    mod->fModType = EDAQ_Scaler;
    break;
  default:
    mod->fModType = fType;
    break;
  }
  mod->fAmin = fBaseIndex;	       	// 1st subaddress
  mod->fNChannel = fNChannel;	        // last subaddress
  mod->fNScChannel = fNScalerChan;      // if its an ADC & scaler
  mod->fBits = fNBits;	       	        // max no. bits from output word
}

//-----------------------------------------------------------------------------
void TDAQmodule::ReadHeader( ModuleInfo_t* mod )
{
  // Return key showing type of module
  // Mk1 format data
  // 
  mod->fVicCrate = fID;		          // Acqu hardware index of module
  mod->fBusType = fIndex;                 // list index of module
  mod->fModType = fType;
  // type of module, ADC, Scaler control etc.
  switch( fType ){
  default:
    mod->fAmin = 0;
    mod->fAmax = 0;
    break;
  case EDAQ_VADC:
    // Virtual ADC (indirectly read)....tell AcquRoot its an ADC
    mod->fModType = EDAQ_ADC;
  case EDAQ_ADC:
    // Directly readout ADC
    mod->fAmin = fBaseIndex;	       	    // 1st subaddress
    mod->fAmax = fBaseIndex + fNChannel -1; // last subaddress
    break;
  case EDAQ_VScaler:
    // Virtual Scaler (indirectly read)....tell AcquRoot its a Scaler
    mod->fModType = EDAQ_Scaler;
  case EDAQ_Scaler:
    // Directly read scaler
    mod->fAmin = 0;
    mod->fAmax = fNChannel - 1;
    break;
  }
  mod->fBits = fNBits;	       	          // max no. bits from output word
}

//-----------------------------------------------------------------------------
inline void TDAQmodule::ErrorStore( void** out, Int_t errcode )
{
  // Write error block to data buffer
  // 2/9/10 Check if Mk1 or Mk2 data format
  // 6/6/13 Mk2 format, write fBaseIndex instead if fModIndex
  //
  if( fEXP->IsMk2Format() ){
    ReadErrorMk2_t* errbl = (ReadErrorMk2_t*)(*out); // Mk2 format
    errbl->fHeader = EReadError;
    errbl->fModID = fID;
    errbl->fModIndex = fBaseIndex;
    errbl->fErrCode = errcode;
    errbl->fTrailer = EReadError;
    *out = errbl + 1;
  }
  else{
    ReadError_t* errbl = (ReadError_t*)(*out);      // Mk1 format
    errbl->fHeader = EReadError;
    errbl->fBus = fIndex;
    errbl->fCrate = fID;
    errbl->fStation = 0;
    errbl->fSubAddr = 0;
    errbl->fCode = errcode;
    errbl->fAlign = 0;
    *out = errbl + 1;
  }
  if( IsType(EDAQ_ADC) )fEXP->IncADCError();
  else if( IsType(EDAQ_Scaler) )fEXP->IncScalerError();
}

ClassImp(TDAQmodule)
