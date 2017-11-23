//--Author	A Neiser      XXth Nov 2013 Adapt from TVME_VUPROM
//--Rev 	...
//--Update 	JRM Annand    31st Jan 2014 Fix register setup 
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROM Moeller
// Readout of the Moeller data

#include "TVME_VUPROM_Moeller.h"
#include "TDAQexperiment.h"
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>


enum { EVUPM_CfgMode=500, EVUPM_ModuleChain, EVUPM_ChannelPairs, EVUPM_NBins,
      EVUPM_NReadsPerIRQ, EVUPM_ScalerModulo};

using namespace std;

static Map_t kVUPROMMoellerKeys[] = {
  {"CfgMode:",                EVUPM_CfgMode}, // use this module to setup channel selection
  {"ModuleChain:",            EVUPM_ModuleChain},  
  {"ChannelPairs:",           EVUPM_ChannelPairs},
  {"NBins:",                  EVUPM_NBins},
  {"NReadsPerIRQ:",           EVUPM_NReadsPerIRQ},
  {"ScalerModulo:",           EVUPM_ScalerModulo},  
  {NULL,      -1}
};

VMEreg_t VUPROMMoellerReg[] = {
  // this list is from 
  // http://wwwa2/intern/daqwiki/doku.php?id=trigger:moeller:vmeregisters
  {0x2f00,      0x0,  'l', 0},       // Firmware revision
  {0x2500,      0x0,  'l', 0},       // Global DAQ enable, sent out by first mod 
  {0x2510,      0x0,  'l', 0},       // NimOutMode (only set by "external" Tagger Channel Selector)
  {0x2020,      0x0,  'l', 0},       // DAQ status 
  {0x2030,      0x0,  'l', 0},       // DAQ reset 
  {0x2040,      0x0,  'l', 0},       // DAQ enable
  {0x2050,      0x0,  'l', 0},       // DAQ counter fix
  {0x2060,      0x0,  'l', 0},       // DAQ counter Livetime
  {0x2070,      0x0,  'l', 0},       // DAQ counter Livetime gated
  {0x2080,      0x0,  'l', 0},       // VN2 and VN1
  {0x2090,      0x0,  'l', 0},       // MAMI Energy Setting
  {0x20a0,      0x0,  'l', 0},       // InputChannelsDebugMode
  {0x20b0,      0x0,  'l', 0},       // InputChannelsDebugLeftStart
  {0x20c0,      0x0,  'l', 0},       // InputChannelsDebugRightStart
  // Scaler stuff
  {0x1800,      0x0,  'l', 0},       // Clear scalers
  {0x1804,      0x0,  'l', 0},       // Load scalers
  {0x1000,      0x0,  'l', EVUM_NMoellerScaler-1},      // Scaler registers 0-95
  // RAM interface for Histogram values
  {0x2c00,      0x0,  'l', 0},       // WEB
  {0x2c10,      0x0,  'l', 0},       // AddrB
  {0x2c20,      0x0,  'l', 0},       // DInB
  {0x2800,      0x0,  'l', 0},       // DOutB0 (this is an offset, see PostInit)
  {0xffffffff,  0x0,  'l', 0},       // end of list
};

// for the readout modules (no CfgMode),
// there's only one ChainCtrl module, thus it's a static variable
static TVME_VUPROM_Moeller* fChainCtrl = NULL;

//-----------------------------------------------------------------------------
TVME_VUPROM_Moeller::TVME_VUPROM_Moeller( Char_t* name, Char_t* file, FILE* log,
                                          Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  
  // Basic initialisation 
  AddCmdList( kVUPROMMoellerKeys );          // VUPROM-specific setup commands
  
  // set some defaults
  fNLeftChannels = 0;
  fNPairsPerCh = 0;
  fNBins = 0; 
  fNReadsPerIRQ = 0;
  fScalerModulo = 0;
  fOffsetLeft = fOffsetRight = 0;
  kCfgMode = kFALSE; // by default, we are a readout module
  
  // other init stuff
  ClearIndices();
  fNScalerIRQsSeen = 0;
  kReadoutStarted = false;
  fReadoutsDone = 0;
  kChainIsLast = false;
  
  // set by InitReg() in PostInit()
  fNreg = 0; 
  fNScalerChan = 106; 
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Moeller::ClearIndices()
{
  // all indices zero, important in ReadIRQ
  iBin = iHelicity = iLeftChannel = iPair = 0; 
  // but prev invalid, forces setting of ram address at the beginning
  iRamAddrPrev = -1;
}

void TVME_VUPROM_Moeller::StartMoellerDAQ()
{
  // set reset high and wait until it's low again (strobe reset)
  Write(EVUM_DAQ_reset, (UInt_t)0x1);  // 0x2030
  while(Read(EVUM_DAQ_reset));  // 0x2030
  // ensure correct NimOutMode
  Write(EVUM_NimOutMode, (UInt_t)0x0); // 0x2510
  // finally start a clean new run, 
  // this also starts the scalers
  if(kChainIsLast) {
    fChainCtrl->Write(EVUM_GlobalEnable, (UInt_t)0x1); // 0x2500
  }
}

void TVME_VUPROM_Moeller::StopMoellerDAQ()
{
  // ensure correct NimOutMode
  Write(EVUM_NimOutMode, (UInt_t)0x0); // 0x2510
  if(fChainCtrl==this)
    Write(EVUM_GlobalEnable, (UInt_t)0x0); // 0x2500
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Moeller::SetConfig( Char_t* line, Int_t key)
{
  // Configuration from file
  stringstream ss(line); // convert it to stringstream for easier conversion
  switch(key) {
  case EVUPM_CfgMode: {
    if(!(ss >> fOffsetLeft)) {
      PrintError(line,"<VUPROM_Moeller CfgMode left>",EErrFatal);
    }
    if(!(ss >> fOffsetRight)) {
      PrintError(line,"<VUPROM_Moeller CfgMode right>",EErrFatal);
    }
    kCfgMode = kTRUE;
    break;
  }
  case EVUPM_ModuleChain: {
    string first, last;
    ss >> first;
    ss >> last;
    if(GetName()==first) {
      fChainCtrl = this;
    }
    if(GetName()==last) {
      kChainIsLast = true;
    }
    break;
  }
  case EVUPM_ChannelPairs: {
    if(!(ss >> fNLeftChannels)) {
      PrintError(line,"<VUPROM_Moeller NoOfLeftChannels config line>",EErrFatal);
    }
    if(!(ss >> fNPairsPerCh)) {
      PrintError(line,"<VUPROM_Moeller NoOfPairsPerCh config line>",EErrFatal);
    }
    if(fNLeftChannels*fNPairsPerCh>(UShort_t)fNChannel) {
      PrintError(line,"<VUPROM_Moeller More than fNChannel pair combinations cannot be read out>",EErrFatal);      
    }
    break;
  } 
  case EVUPM_NBins: {
    if(!(ss >> fNBins)) {
      PrintError(line,"<VUPROM_Moeller NBins config line>",EErrFatal);
    }
    break;
  }
  case EVUPM_NReadsPerIRQ: {
    if(!(ss >> fNReadsPerIRQ)) {
      PrintError(line,"<VUPROM_Moeller NReadsPerIRQ config line>",EErrFatal);
    }
    break;
  }
  case EVUPM_ScalerModulo: {
    if(!(ss >> fScalerModulo)) {
      PrintError(line,"<VUPROM_Moeller ScalerModulo config line>",EErrFatal);
    }
    break;
  }  
  default:
    // default try commands of TVMEmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VUPROM_Moeller::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  // before we init the reg, we properly 
  // set the repeat attribute according to the config, 
  // we need one register more to the old readout code from Peter 
  // the histogram at EVUM_RAM_DOutB0 (k=0, see ReadIRQ) is also there
  // but not needed for analysis
  UInt_t rep = fNLeftChannels*fNPairsPerCh;
  // This is a filthy hack, but for now I can't think of better, jrma
  Int_t ldoutB0 = EVUM_RAM_DOutB0 - EVUM_NMoellerScaler + 1;
  VUPROMMoellerReg[ldoutB0].repeat = rep;
  InitReg( VUPROMMoellerReg );
  // init the base class  
  TVMEmodule::PostInit();
  
  // check if cfg mod only, then just do the config business
  if(kCfgMode) {  
    Write(EVUM_InputChannelsDebugLeftStart, fOffsetLeft); // 0x20b0
    Write(EVUM_InputChannelsDebugRightStart, fOffsetRight); // 0x20b0
    return;
  }
  
  // we are a read out module
  // ensure DAQ is stopped
  StopMoellerDAQ();
  // start the DAQ right now 
  // and hope that there are ReadIRQ's soon  
  StartMoellerDAQ();
}

//-------------------------------------------------------------------------
Bool_t TVME_VUPROM_Moeller::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read(EVUM_Firmware);
  fprintf(fLogStream,"VUPROM Moeller firmware version Read: %x  Expected: %x\n",
          id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VUPROM Moeller firmware ID error>",EErrFatal);
  return kFALSE;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Moeller::ReadIRQ( void** outBuffer ) 
{
  
  if(!kReadoutStarted)
    return;
  
  UInt_t nReads = 0;
  // a total readout of Moeller data needs
  // fNBins*2*fNLeftChannels*fNPairsPerCh = 25.600 VME reads
  // thus we need to distribute this over many IRQs
  while(iBin<fNBins) {
    while(iHelicity<2) { // two helicity states: 0 or 1
      // since we might enter this loop the first time,
      // we need this mechanism to set the RamAddr if it has changed
      // this also saves roughly half of all VME accesses!
      Int_t iRamAddr = iHelicity*fNBins+iBin;
      if(iRamAddr != iRamAddrPrev) {
        Write(EVUM_RAM_AddrB, iRamAddr); 
      }
      iRamAddrPrev = iRamAddr;
      
      // inner two loops
      // since now there is the real working loop to read the data
      while(iLeftChannel<fNLeftChannels) {        
        while(iPair<fNPairsPerCh) {          
          UInt_t k = iLeftChannel*fNPairsPerCh + iPair;
          UInt_t datum = Read(EVUM_RAM_DOutB0+k+1); // +1 since k=0 is special histogram
          nReads++;
          // ADCs can hold 16bit in AcquDAQ,
          // so project the read 32bit value
          UShort_t datumlow = datum & 0xffff;
          UShort_t datumhigh = datum >> 16;
          
          // also generate and index telling 
          // where this value actually came from
          // three "hits" for the same ADC index
          ADCStore(outBuffer, iRamAddr, fBaseIndex+k);
          ADCStore(outBuffer, datumlow, fBaseIndex+k);
          ADCStore(outBuffer, datumhigh, fBaseIndex+k);
          iPair++; // go to the next pair
          // if max reads reached, stop reading for this IRQ
          // will hopefully be resumed next IRQ!
          if(nReads==fNReadsPerIRQ) {
            return;
          }            
        }      
        iPair = 0;
        iLeftChannel++;
      }
      
      // outer, RAM-address loop
      iLeftChannel = 0;
      iHelicity++;    
    }
    iHelicity = 0;
    iBin++;
  }
  
  // reached end of all loops, 
  // so readout is finished
  kReadoutStarted = false;  
  fReadoutsDone++;
  
  // immediately start the DAQ again 
  // (we assume it has been stopped before readout)
  StartMoellerDAQ();
}

//-----------------------------------------------------------------------------
void TVME_VUPROM_Moeller::ReadIRQScaler(void** outBuffer)
{    
  // check if a Moeller readout is due
  fNScalerIRQsSeen++;
  if(fNScalerIRQsSeen % fScalerModulo != 0)
    return;
  
  // note that this is not the best way to do it
  // since we don't know if a new run has been started
  // or this is maybe the last ScalerRead in a run
  // so, it might be that in between runs the Moeller DAQ 
  // is still running and maybe some other tests were performed then..?!  
  
  if(kReadoutStarted) {
    cerr << "Readout started, and previous not finished! Skipping!" << endl;
    return;
  }
  
  // stop the acquisition and start the readout
  StopMoellerDAQ();
  ClearIndices();
  kReadoutStarted = true;    
  // also send some marker scalers, 
  // containing the current time
  if(fChainCtrl == this) {
    ScalerStore(outBuffer, 0xaa000000+(fReadoutsDone&0xffff), fBaseIndex+0);
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);    
    ScalerStore(outBuffer, t.tv_sec, fBaseIndex+1);
    ScalerStore(outBuffer, t.tv_nsec, fBaseIndex+2);   
  }
  // write the scalers at offset 10
  Write(EVUM_ScalerLoad, 1); 
  for(UInt_t i=0; i<96; i++){     // read 96 scaler channels
    UInt_t j = i + EVUM_Scaler;
    UInt_t datum = Read(j);
    ScalerStore( outBuffer, datum, fBaseIndex+10+i );
  }
  Write(EVUM_ScalerClr, 1);           // clear the scalers
}

ClassImp(TVME_VUPROM_Moeller)
