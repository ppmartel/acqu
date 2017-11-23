//--Author	A Neiser      XXth Nov 2013 Adapt from TVME_VUPROM
//--Rev 	...
//--Update 	JRM Annand    30th Jan 2014 Fix register setup 
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROM_Pattern
// Hit pattern reading module, configurable via dat file

#include "TVME_VUPROM_Pattern.h"
#include "TDAQexperiment.h"
#include <sstream>
#include <iostream>
#include <string>


enum { EVUPP_ModuleChain=600, EVUPP_Pattern };

using namespace std;

static Map_t kVUPROMPatternKeys[] = {
  {"Pattern:",       EVUPP_Pattern},
  {NULL,      -1}
};



//-----------------------------------------------------------------------------
TVME_VUPROM_Pattern::TVME_VUPROM_Pattern( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{  
  // Basic initialisation 
  AddCmdList( kVUPROMPatternKeys );          // VUPROM-specific setup commands
    
  // number of Patterns and registers
  // are finally set in SetConfig/PostInit
  // since the number of Patterns can be configured
  fNreg = 0; 
  fNChannel = 0;
  fPatternOffset = 0;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Pattern::SetConfig( Char_t* line, Int_t key)
{
  stringstream ss(line);
  // Configuration from file
  switch(key) {
  case EVUPP_Pattern: {
    // ugly workaround for some global non-Pattern registers
    if(fNreg==0) {
      // we add global registers here (not in the constructor)
      // this ensures that fNreg is still zero when BaseSetup: is evaluated 
      // (needed to properly initialize the register pointers later in PostInit)
      // the first register entry is the firmware
      VMEreg_t firmware = {0x2f00, 0x0, 'l', 0};
      fVUPROMregs.push_back(firmware);  
      fNreg = fVUPROMregs.size();      
      fPatternOffset = fNreg; // offset where the Pattern blocks start
    }
    
    UInt_t offsetAddr; // usually submodule address    
    
    ss << hex; // enable hex mode for addresses
    if(!(ss >> offsetAddr)) {
      PrintError(line,"<VUPROM_Pattern offset address>",EErrFatal);
    }
   
    ss << dec; // the number is decimal again
   
    // create the corresponding registers
    VMEreg_t Patterns = {offsetAddr, 0x0, 'l', 0}; 
  
    
    // remember the Pattern block offset
    // before we add them. Note that the size of fVUPROMregs is not 
    // the correct offset due to the repeat attribute,
    fPatternOffsets.push_back(fPatternOffset);
    fVUPROMregs.push_back(Patterns);    
    // add it to the total number
    fNChannel += 2;
    // Pattern register offset, 
    fPatternOffset += 1;
    break;
  } 
  default:
    // default try commands of TVMEmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VUPROM_Pattern::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;

  // before we call InitReg, 
  // add an "end-marker" at the very end 
  // (well this happens if one uses C-style pointer hell...)
  VMEreg_t end = {0xffffffff, 0x0, 'l', 0};
  fVUPROMregs.push_back(end);
  // this also sets fNReg to the correct value finally!
  fNreg = 0;
  InitReg( fVUPROMregs.data() );
  
  // init the base class  
  TVMEmodule::PostInit();
}

//-------------------------------------------------------------------------
Bool_t TVME_VUPROM_Pattern::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read((UInt_t)0); // first one is firmware, see SetConfig()
  fprintf(fLogStream,"VUPROM Pattern firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VUPROM Pattern firmware ID error>",EErrFatal);
  return kFALSE;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Pattern::ReadIRQ( void** outBuffer )
{
 
  // iterate over the patterns, remember
  size_t n = fBaseIndex; // we start at the base index
  for(size_t patt=0;patt<fPatternOffsets.size();patt++) {
    size_t offset = fPatternOffsets[patt];
    UInt_t datum = Read(offset);
    UInt_t datum_low = datum & 0xffff;
    UInt_t datum_high = datum >> 16;    
    // use n to get all blocks consecutive
    ADCStore(outBuffer, datum_low, n); 
    n++;
    ADCStore(outBuffer, datum_high, n);
    n++; // for next iteration, increment!
  }
}
ClassImp(TVME_VUPROM_Pattern)
