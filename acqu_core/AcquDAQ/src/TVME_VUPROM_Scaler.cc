//--Author	A Neiser      XXth Nov 2013 Adapt from TVME_VUPROM
//--Rev 	...
//--Update 	JRM Annand  
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROM Scaler
// scaler reading module configurable via dat file

#include "TVME_VUPROM_Scaler.h"
#include "TDAQexperiment.h"
#include <sstream>
#include <iostream>
#include <string>


enum { EVUPS_ModuleChain=400, EVUPS_Scaler, EVUPS_UseEPT };

using namespace std;

static Map_t kVUPROMScalerKeys[] = {
  {"ModuleChain:",  EVUPS_ModuleChain}, 
  {"Scaler:",       EVUPS_Scaler},
  {"UseEPT:",       EVUPS_UseEPT},
  {NULL,      -1}
};

// controller of this chain of scalers,
// can be NULL, which means that start/stopping 
// is controlled externally (e.g. by Moeller module)
static TVME_VUPROM_Scaler* fChainCtrl = NULL;

//-----------------------------------------------------------------------------
TVME_VUPROM_Scaler::TVME_VUPROM_Scaler( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{  
  // Basic initialisation 
  AddCmdList( kVUPROMScalerKeys );          // VUPROM-specific setup commands
    
  // number of scalers and registers
  // are finally set in SetConfig/PostInit
  // since the number of scalers can be configured
  fNreg = 0; 
  fNScalerChan = 0; 
  fScalerOffset = 0;
  
  kChainIsLast = kFALSE;
  kUseEPT = kFALSE;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Scaler::SetConfig( Char_t* line, Int_t key)
{
  stringstream ss(line);
  // Configuration from file
  switch(key) {
  case EVUPS_ModuleChain: {
    string first, last;
    ss >> first;
    ss >> last;
    if(GetName()==first) {
      fChainCtrl = this;
    }
    if(GetName()==last) {
      kChainIsLast = kTRUE;
    }
    break;
  }
  case EVUPS_Scaler: {
    // ugly workaround for some global non-scaler registers
    if(fNreg==0) {
      // we add global registers here (not in the constructor)
      // this ensures that fNreg is still zero when BaseSetup: is evaluated 
      // (needed to properly initialize the register pointers later in PostInit)
     
      // the first register entry is the firmware
      VMEreg_t firmware = {0x2f00, 0x0, 'l', 0};
      fVUPROMregs.push_back(firmware);
      
      // the second one is used to stop/start the scalers
      VMEreg_t scaler_gate = {0x2200, 0x0, 'l', 0};
      fVUPROMregs.push_back(scaler_gate);     
      
      // the third one is used to enable the EPT mode
      VMEreg_t useEPT = {0x2210, 0x0, 'l', 0};
      fVUPROMregs.push_back(useEPT); 
      
      fNreg = fVUPROMregs.size();      
      fScalerOffset = fNreg; // offset where the scaler blocks start
    }
    
    UInt_t offsetAddr; // usually submodule address    
    UInt_t clearAddr;
    UInt_t loadAddr;
    UInt_t numberOfScalers;
    
    ss << hex; // enable hex mode for addresses
    if(!(ss >> offsetAddr)) {
      PrintError(line,"<VUPROM_Scaler offset address>",EErrFatal);
    }
    if(!(ss >> clearAddr)) {
      PrintError(line,"<VUPROM_Scaler clear address>",EErrFatal);
    }
    if(!(ss >> loadAddr)) {
      PrintError(line,"<VUPROM_Scaler load address>",EErrFatal);
    }    
    ss << dec; // the number is decimal again
    if(!(ss >> numberOfScalers)) {
      PrintError(line,"<VUPROM_Scaler number of scalers>",EErrFatal);
    }
    
    if(numberOfScalers==0)
      break;
    
    // create the corresponding registers
    VMEreg_t scalers = {offsetAddr, 0x0, 'l', numberOfScalers-1}; // use the repeat attribute    
    VMEreg_t clear = {offsetAddr+clearAddr, 0x0, 'l', 0};
    VMEreg_t load  = {offsetAddr+loadAddr,  0x0, 'l', 0};    
    
    // remember the scaler block offset
    // before we add them. Note that the size of fVUPROMregs is not 
    // the correct offset due to the repeat attribute,
    fScalerBlockOffsets.push_back(make_pair(fScalerOffset, numberOfScalers));
    // add them in some defined order, 
    // ReadIRQScaler() relies on that!
    fVUPROMregs.push_back(load);
    fVUPROMregs.push_back(clear);
    fVUPROMregs.push_back(scalers);    
    // add it to the total number
    fNScalerChan += numberOfScalers;
    // scaler register offset, 
    // +2 due to the load/clear registers in the beginning    
    fScalerOffset += numberOfScalers+2;
    break;
  } 
  case EVUPS_UseEPT: {
    kUseEPT = kTRUE;
    break;
  }
  default:
    // default try commands of TVMEmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VUPROM_Scaler::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;

  // before we call InitReg, 
  // add an "end-marker" at the very end 
  // (well this happens if one uses C-style pointer hell...)
  VMEreg_t end = {0xffffffff, 0x0, 'l', 0};
  fVUPROMregs.push_back(end);
  fNreg = 0;
  // this also sets fNReg to the correct value finally!
  InitReg( fVUPROMregs.data() );
  
  // init the base class  
  TVMEmodule::PostInit();
  
  // set the EPT mode
  // if not the last module
  // this is a pretty stupid workaround...
  if(!kChainIsLast) {
    if(kUseEPT) {
      Write((UInt_t)2, (UInt_t)1);
    }
    else {
      Write((UInt_t)2, (UInt_t)0);
    }
  }
  
  // last of all modules tells first one (the "controller") 
  // to start the scalers the first time
  if(kChainIsLast)
    fChainCtrl->StartScalers();
}

//-------------------------------------------------------------------------
Bool_t TVME_VUPROM_Scaler::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read((UInt_t)0); // first one is firmware, see SetConfig()
  fprintf(fLogStream,"VUPROM Scaler firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VUPROM Scaler firmware ID error>",EErrFatal);
  return kFALSE;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Scaler::ReadIRQScaler( void** outBuffer )
{
  // first of all scaler modules (the controller)
  // stops the scalers
  if(fChainCtrl==this)
    StopScalers();
  
  // iterate over the blocks, remember
  size_t n = 0; // total number of scalers stored
  for(size_t block=0;block<fScalerBlockOffsets.size();block++) {
    size_t offset = fScalerBlockOffsets[block].first;
    size_t nScalers = fScalerBlockOffsets[block].second;
    Write(offset+0, (UInt_t)1); // load scalers into buffer
    for(size_t i=0; i<nScalers; i++ ) {
      UInt_t datum = Read(offset+2+i); // +2 due to clear/load register in the beginning
      ScalerStore(outBuffer, datum, fBaseIndex+n); // use n to get all blocks consecutive
      n++;
    }
    Write(offset+1, 1); // clear the scalers
  }
  
  // last of all modules tells first one (the "controller") 
  // to start the scalers again 
  if(kChainIsLast)
    fChainCtrl->StartScalers();
}

void TVME_VUPROM_Scaler::StopScalers()
{
  // second register is global scaler gate, see SetConfig() 
  Write((UInt_t)1, (UInt_t)0);
}

void TVME_VUPROM_Scaler::StartScalers()
{
  // second register is global scaler gate, see SetConfig()
  Write((UInt_t)1, (UInt_t)1);
}

ClassImp(TVME_VUPROM_Scaler)
