//--Author	JRM Annand     2nd Sep 2009  Convert from acqu 3v7
//--Rev 	JRM Annand  
//--Rev         B. Oussena   15th Jan 2010 Start updates for FASTBUS
//--Rev         B. Oussena    1st feb 2010 Add cmd EFB_RDMode, SetRDMode()
//--Rev         B. Oussena   10th Mar 2010 Add decode Scaler and ADC methods
//--Rev  	JRM Annand   27th Jul 2010 Generalise 1800 module class
//--Rev         B. Oussena    3rd Aug 2010 Fix bugs in SetConfig() & PostInit()
//--Rev         JRM Annand    4th Sep 2010 Chk Mk1/Mk2 data, fBaseIndex setting
//--Rev         JRM Annand    9th Sep 2010 Ordering of CSR config
//--Update	JRM Annand    6th Jul 2011 gcc4.6-x86_64 warning fixes
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TFBmodule
// Basic class for FASTBUS slave modules
// General-purpose FASTBUS hardware with no specialist read/write procedures
// may use this class
//

#include "TFBmodule.h"
#include "TDAQexperiment.h"
#include "TFB_1821SMI.h"


enum { EFB_AMDW=100, EFB_Register, EFB_IDRegister, EFB_RDMode }; //<-- Baya
static Map_t kFBKeys[] = {
  {"Address-Data:",       EFB_AMDW},
  {"FB-Register:",        EFB_Register},
  {"ID-Register:",        EFB_IDRegister},
  {"RD-Mode:",            EFB_RDMode},
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TFBmodule::TFBmodule( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation.
  // 
  AddCmdList( kFBKeys );                  // physics-specific cmds

  //  fIsWrt = NULL;
  fBus = E_FASTBUS;
  fHardID = 0;
  fP_SMI = 0;
  fPanel = 0;
  fReadMode = 'b';
  fTest = 'y';
  fGeog = 'y';
  fC0_Reset = 0;
  fC0_Config = 0;
  fC0_Test = 0;
  fC0_Acqu = 0;
  fC0_Acqu_M = 0;
  fC0_Acqu_S = 0;
  fC0_Acqu_E = 0;
  fD_MBlk = 0;
  fD_Blk = 0;
  fD_Read = 0;
  fSMIctrl = NULL;    
  Char_t r_mode[2];
  if( line ){
    Int_t n, base;
    if( (n = sscanf( line, "%*s%*s%*s%d%s%d",&fN, r_mode, &base )) < 2 )
      PrintError( line, "<FB_Bus setup line parse>", EErrFatal );
    if( n == 3 ) fBaseIndex = base;
  }
  fReadMode = r_mode[0];
  fTest = 'n';
}

//-----------------------------------------------------------------------------
TFBmodule::~TFBmodule( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TFBmodule::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  if( !fSMIctrl ){
    if( fCtrlMod->InheritsFrom("TFB_1821SMI") )
      fSMIctrl = (TFB_1821SMI*)fCtrlMod;
    else
      PrintError(line,"<FB module controller not an 1821 SMI>",EErrFatal);
  }
  switch( key ){
  case  EFB_RDMode:
    // Readout Module setting
    SetRDMode();
    break;
  default:
    // default try commands of TDAQmodule
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TFBmodule::InitReg( Char_t* line )
{ 
  // Baya --->> this method has been changed to cope with Fastbus 
  // Parse the input line for device register information
  // Store the register

  UInt_t cData, cDataH, np;
  UShort_t a, cReg;
  Char_t type[3]; 

  if( !fReg ){
    PrintError(line,"<Module registers not yet created>",EErrFatal);
  }
  //  Int_t ncReg = 0;
  Int_t err = 0;
 
  sscanf(line,"%2s",type);                    // get type of line
  if(strcmp(type,"0x") == 0){	              // hex C reg input
    np = sscanf(line,"%*2s%hx%x", &cReg, &cData);
    err +=  fSMIctrl->W_1821(fN, cReg, cData,'c');
    fReg[fNreg] = (void*)((ULong_t)cReg); fData[fNreg] = cData; fNreg++; 
  }			
  else if(strcmp(type,"pd") == 0){            // pedestal register
    np = sscanf(line,"%*2s%hd%d%d", &a, &cData, &cDataH);
    err +=  fSMIctrl->W_ped(fN, a, cData);
    if(np == 3){
      a += 128;                               // high range select
      err +=  fSMIctrl->W_ped(fN, a, cDataH);
    }
  }
  if( np < 2 )
    PrintError(line,"<Parse error Fastbus register input>",EErrFatal);
  if( fNreg > fMaxReg )
    PrintError(line,"<Too many Fastbus register input>",EErrFatal);
  return;
}


//-----------------------------------------------------------------------------
void TFBmodule::PostInit( )
{
  // Post-configuration setup of hardware
  // If the primary controller uses memory mapping create the map here
  // Write stored data to hardware
  //
  if( fIsInit ) return;
  TDAQmodule::PostInit();
  if( !CheckHardID() ) return;
  return;
}


//-----------------------------------------------------------------------------
Bool_t TFBmodule::CheckHardID( )
{
  // Check hardware ID of fastbus module

  UInt_t mod_id; // fastbus module id code
  fSMIctrl->R_1821(fN, 0, &mod_id, 'c');
  if(  mod_id>>16 != (UInt_t)fHardID )
    PrintError("","<FASTBUS module ID not recognised>", EErrFatal);
  return kTRUE;    
}

//-----------------------------------------------------------------------------
void TFBmodule::SetRDMode( )
{
  // Now set up module into readout mode according to desired mode eg block, 
  // megablock etc

  Int_t err = 0;

  switch(fReadMode){  // switch according to selected read  mode 
  case('m'):  case('s'):  case('e'):  case('b'): 
    fSMIctrl->Add_FbADC(this);
    switch(fReadMode){
    case('m'):		// megablock master 
      fD_Read = (fN << 8) | (fD_MBlk & 0xff);
      err += 	 fSMIctrl->W_1821(fN,0,fC0_Acqu_M,'c');
      break;
    case('s'):		// megablock simple link
      err += 	fSMIctrl->W_1821(fN,0,fC0_Acqu_S,'c');
      break;
    case('e'):		// megablock end link 
      err += 	fSMIctrl->W_1821(fN,0,fC0_Acqu_E,'c');
      break;
    case('b'):		// normal block readout 
      fD_Read = (fN << 8) | (fD_Blk & 0xff);
      err += fSMIctrl->W_1821(fN,0,fC0_Acqu,'c');
      if( this->InheritsFrom("TFB_LRS1800") ){
	err += fSMIctrl->W_1821(fN,0,fC0_Acqu_M,'c');
	err += fSMIctrl->W_1821(fN,0,fC0_Acqu_S,'c');
	err += fSMIctrl->W_1821(fN,0,fC0_Acqu_E,'c');
      }		
      break;
    }
    break;
  case('z'):
    // scaler block readout  
    fSMIctrl->Add_FbSCA(this);
    fD_Read = (fN << 8) | (fD_Blk & 0xff);
    err += fSMIctrl->W_1821(fN,0,fC0_Acqu,'c');// now set up scaler ID table 
    break;		
  default:		// default no IRQ readout of module 
    err += fSMIctrl->W_1821(fN,0,fC0_Acqu,'c');
    break;
  }
  if( err )
    PrintError("SetRDMode","<Error setting readout of FB module>",EErrFatal);
  return;
}

//----------------------------------------------------------------------------
void TFBmodule::DecodeScaler( UInt_t **inBuff, void **outBuff, Int_t *n_word)
{
  // Default decode of block of scaler information
  // NB scalers are 32-bit
  // 1st check for any read error 
  if(**inBuff == EReadError){
    fSMIctrl-> smi_error(inBuff,outBuff,n_word);
    return;
  }
  *n_word = *n_word - fNChannel;
  Int_t a = fBaseIndex;
  for(Int_t i=0; i<fNChannel; i++,a++){
    ScalerStore(outBuff, **inBuff, a);
    (*inBuff)++;
  }
  return;
}

ClassImp(TFBmodule)
