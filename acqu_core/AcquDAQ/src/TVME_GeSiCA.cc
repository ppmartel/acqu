#include "TVME_GeSiCA.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <unistd.h>

using namespace std;

//------------------------------------------------------------------------------
Map_t kGeSiCA_Keys[] = {
  {"NADCBoards:",    ENADCBoards},
  {"GeSiCAFile:",    EGeSiCAFile},
  {"GeSiCAReg:",     EGeSiCAReg},
  {"GeSiCAMode:",    EGeSiCAMode},
  {"SADCReg:",       ESADCReg},
  {"SADCFile:",      ESADCFile},
  {"SADCThreshold:", ESADCThresh},
  {"SADCSum:",       ESADCSum},
  {"GeSiCALogFile:",  EGeSiCALog},
};

//
// GeSiCA register address
//
VMEreg_t GeSiCAReg[] = {
  // offset,    data,  type,  repeat
  // Onboard GeSiCA registers
  {0x0,         0x1,    'l',    0},  // EIBase    0 Base
  {0x14,        0x0,    'l',    0},  // EIFPGAReg 1 FPGA download
  {0x20,        0x0,    'l',    0},  // EIMStatus 2 Module Status
  {0x24,	      0x0,	  'l',	  0},  // EIDStatus 3 Data-Buffer Status
  {0x28,	      0x0,	  'l',	  0},  // EIDatum   4 Data-Buffer
  {0x2c,        0x0,    'l',    0},  // EIHlPort  5 Hot-Link Port selection
  // I2C-registers
  {0x48,        0x0,	'l',	0},  // EI2CAddrCtl 6 Address and Control
  {0x4c,        0x0,	'l',	0},  // EI2CStatus  7 Status
  {0x40,        0x0,	'l',	0},  // EI2CdwLow   8 Data Write Low
  {0x54,        0x0,	'l',	0},  // EI2CdwHigh  9 Data Write High 3/11/03
  {0x44,        0x0,	'l',	0},  // EI2CdrLow  10 Data Read Low
  {0x58,        0x0,	'l',	0},  // EI2CdrHigh 12 Data Read High 3/11/03
  {0x50,        0x0,	'l',	0},  // EI2CPortSelect  13 Select i2c-port
  {0xffffffff, 0, 0, 0}              // terminator
};


//------------------------------------------------------------------------------
TVME_GeSiCA::TVME_GeSiCA( Char_t* name, Char_t* file, FILE* log,
                          Char_t* line ):
  TVME_CATCH( name, file, log, line )
{
  // Basic initialisation 
  AddCmdList( kGeSiCA_Keys );                // CATCH-specific setup commands
  fNSample = fNSampleWd = fADCmode = fLatency = fOpMode = fNSADC = fNChip = 0;
  fSADCfile = NULL;
  fSADCport = fSADCchan = fSADCthresh = NULL;
  fNthresh=0;
  fNBits = 13;                             // 13-bit ADC
  fPedFactor = 0.0;
  for(Int_t i=0; i<3; i++){ fSamStart[i] = fSamWidth[i] = 0; }
}

//-----------------------------------------------------------------------------
TVME_GeSiCA::~TVME_GeSiCA( )
{
  // Clean up
}

//---------------------------------------------------------------------------
void TVME_GeSiCA::ReadIRQ( void** outBuffer )
{
  // Read GeSiCA (SG-ADC) Data Buffer via Spy Buffer (VMEbus)

  UInt_t* pdatum;
  Int_t adcMode;                // 0: latch-all, 1: sparsified
  Int_t chipID;                 // at the moment 1: top / 0: bottom chip
  Int_t adcBlock;
  UShort_t index, value;
  
  UInt_t nword = SpyRead(outBuffer);
  if( !nword ) return;
  // Now decode the contents of the local buffer previously filled
  // from GeSiCA. Skip SLINK header...1st datum should be an ADC header
  // should be n_sgadc * 2 ADC-headers in the buffer
  pdatum = fSpyData + 2;
  adcMode = 0x1   & (*pdatum >> 24);        // 0: latch_all; 1: sparsified
  if( adcMode ){
    //Loop over all ADC Headers
    for(Int_t i = 0; i < fNChip; i++){
      chipID  = 0xf   & (*pdatum >> 26);        // ID of chip (0 or 1)
      adcBlock = (0xfff & (*pdatum >> 12)) - 1; // ADC data block size
      pdatum++;
      switch(fOpMode){
      case ESG_Triple: 
        // Save the 3 sums calculated by the SG-ADC
        // 3 values (sum 0..2) for every ADC channel over pedestal
        for(Int_t j=0; j<adcBlock; j+=3) {
          index = 16 * chipID + (0xf & (*pdatum >> 26)) + fBaseIndex;//index
          for(Int_t k=0; k<3; k++){
            value = 0xffff & *pdatum++;
            ADCStore(outBuffer,value,index);
          }
        }
        break;
      case ESG_Sum1:
        // Save single sum calculated by the SADC
        for(Int_t j=0; j<adcBlock; j++){
          index = 16 * chipID + (0xf & (*pdatum >> 26)) + fBaseIndex;//adc ind
          value = 0xffff & *pdatum++;
          ADCStore(outBuffer, 0, index);
          ADCStore(outBuffer, value, index);
          ADCStore(outBuffer, 0, index);
        }
        break;
      default:
        // Anything else do nothing
        // It should not happen for the moment!
        return;
      }
    }
    return;
  }
  // Still to code in non-sparcified readout
  else return;
}

//---------------------------------------------------------------------------
Int_t TVME_GeSiCA::SpyRead( void** outBuffer )
{
  // Read the GeSiCA spy (VMEbus) buffer
  // Check that the module is active..ie has received a trigger
  // If any error detected return 0 words read, reset spy buffer
  // and write error block into data stream
  
  UInt_t datum = 0;
  UInt_t* pStatus = (UInt_t*)fReg[EIDStatus];
  for( Int_t k=0; k<=EGeSiCATimeout; k++ ){  // less equal is important here!
    datum = *pStatus;
    if( datum & 0x1 )break;
    if( k == EGeSiCATimeout ){
      ErrorStore(outBuffer, 7);                 // error code 7
      SpyReset();
      return 0;
    }
  }
  UInt_t* pDatum = (UInt_t*)fReg[EIDatum];
  
  // Examine header, the first data word...should be 0x0
  UInt_t header = *pDatum;
  if( header != 0 ){
    ErrorStore( outBuffer, 1 );                // error code 1
    SpyReset();
    return 0;
  }
  
  // Check for error flag in 1st non-zero header datum
  header = *pDatum;
  if( header & ECATCH_ErrFlag ){
    ErrorStore( outBuffer,0x1 );
    SpyReset();
    return 0;
  }
  
  // Check consistency of data-buffer header and data-status register
  UInt_t nWordHeader = header & 0xffff;
  UInt_t nWordStatus = 0;
  UInt_t nWordTries = 0;
  do {
    nWordStatus = (*pStatus >> 16) & 0xffff;    
    if(nWordTries>200) {
      // reached maximum number of tries, this is 
      // the NEW error 4 (previously it only checked one time...)
      ErrorStore( outBuffer, 4 );                  // error code 4
      SpyReset();
      return 0;
    }
    nWordTries++;
  }
  while(nWordHeader != nWordStatus);
  
  // Check too many data words
  if( nWordHeader > fMaxSpy ){                            // overflow ?
    ErrorStore( outBuffer, 2 );                     // error code 2
    SpyReset();
    return 0;
  }
  
  // Make reads from buffer until we 
  // find the magic ECATCH_Trailer word, 
  // or fail after fMaxSpy reads
  UInt_t nWordRead = 0;
  while(true) {
    datum = *pDatum;
    fSpyData[nWordRead] = datum;
    nWordRead++;
    // be careful not to use *pDatum but datum, 
    // since this reads another word!
    if(datum == ECATCH_Trailer) {
      break;
    }
    
    
    if(nWordRead == fMaxSpy) {
      // this is a pretty bad error, 
      // one should probably better stop the DAQ somehow if this occurs...
      ErrorStore( outBuffer, 5 );                     // error code 5
      SpyReset();
      return 0;
    }
    
  }
  
  // Check if expected number of words match
  // number of words in spybuffer
  if(nWordRead != nWordHeader) {
    ErrorStore( outBuffer, 6 );                     // error code 6
    SpyReset();
    return 0;
  }
  
  
  // Check and buffer status reg. is 0
  datum = *pStatus;
  if( datum != 0 ) { 
    // we should make sure that the buffer is empty
    // so read until the status reg becomes zero
    UInt_t n = 0;
    do {
      datum = *pDatum;
      n++;
      if(n == fMaxSpy) {
        // mark this as an error
        ErrorStore( outBuffer, 3 );                     // error code 8
        SpyReset();
        return 0;
      }
    }
    while( *pStatus != 0);
    // mark this as an error
    ErrorStore( outBuffer, 3 );                     // error code 3
    SpyReset();
    return 0;
  }
  
  // Don't forget this: without saving the event ID there is no synch
  fTCSEventID = fSpyData[0];                      // 1st word = TCS event ID
  
  // if the system specifies that this GeSiCA sends the event ID to a remote 
  // system do it here
  if( fEventSendMod ) 
    fEventSendMod->SendEventID( fTCSEventID );  
  
  // no errors at all, return number of read words from SpyBuffer
  return (Int_t)nWordRead;
}

//---------------------------------------------------------------------------
void TVME_GeSiCA::SpyReset( )
{
  // Reset the GeSiCA module
  // Igor tells us, not to do this because he does
  // not know, what happens to the onboard TCS-Receiver. But anyway,
  // to have an empty buffer after BOS/EOS-procedures, I clear it, to
  // start catching "good data" in later events. We will need a "read the
  // buffer empty"-procedure on BOS/EOS.
  Write(EIBase,1);
  return;
}

//---------------------------------------------------------------------------
void TVME_GeSiCA::PostInit( )
{
  // Initialisation after all config data has been read in.
  //
  if( fIsInit ) return;
  // report on some critical stuff:
  //  printf(" Status of current (ID=0x%x) GeSiCA: 0x%x\n",
  //	 0xffffff & Read(EIBase), Read(EIMStatus));
  //  printf(" Data-Buffer-Status: 0x%x\n", Read(EIDStatus));
  InitReg( GeSiCAReg );
  TVMEmodule::PostInit();

  if(fInitLevel == EExpInit0) {
    // init the Gesica always,
    // this also checks if the Gesica are properly
    // connected via i2c
    if(!init_gesica()) {
      PrintError("","Could not init i2c Gesica...see output above",EErrFatal);
    }
    return;
  }

  // start deep init of the GeSiCA / SADCs cards
  

  // check if Gesica is there at all, no i2c business yet
  if(!init_gesica(true)) {
    PrintError("","Could not find Gesica...see output above",EErrFatal);
  }
  UInt_t nTries = 10;

  do {
    ProgFPGA();           // GeSiCA FPGA file
    // try to init the i2c then,
    // if it fails, try programming the GeSiCa again
    // failure is usually indicated by red Rs LED on SADC module
    if(init_gesica()) 
      break;
    
    nTries--;
    if(nTries==0) {
      PrintError("","Could init i2c eventually...see output above",EErrFatal);
    }
    
    cerr << "Retrying GeSiCa programming" << endl;
    usleep(10000);
  }
  while(nTries>0);

  // then do the rest
  ProgSADC();           // SADC FPGA files
  
  ProgOpMode();         // GeSiCA operational mode
  ProgSampleSum();      // Sample integration boundaries
  ProgThresh();         // SADC thresholds
}

//---------------------------------------------------------------------------
void TVME_GeSiCA::SetConfig( Char_t* line, Int_t key )
{
  // Initialise GeSiCA and SG-ADC modules via VMEbus
  // Configuration from file 
  Char_t fpgafile[128];               // name of FPGA program file
  switch( key ){
  case ENADCBoards:
    // Get number SADCs connected to GeSiCA
    if(sscanf(line,"%d", &fNSADC ) != 1 )
      PrintError(line,"<Number ADC Parse Error>",EErrFatal);
    if ((fNSADC <= 0) || (fNSADC > 8))
      PrintError(line,"<Invalid Number ADC>",EErrFatal);
    fNChannel = 32*fNSADC;   // # ADC channels
    fNChip = 2*fNSADC;       // # ADC chips
    fSADCport = new Int_t[fNChannel];
    fSADCchan = new Int_t[fNChannel];
    fSADCthresh = new Int_t[fNChannel];
    break;
  case EGeSiCAFile:
    // GeSiCA FPGA file name
    if(sscanf(line,"%s", fpgafile ) != 1 )
      PrintError(line,"<SetConfig GeSiCA FPGA file parse error>",EErrFatal);
    fFPGAfile = BuildName(fpgafile);
    break;
  case EGeSiCAReg:
    // Write value to GeSiCA register at address offset
    // Not currently supported
    break;
  case EGeSiCAMode:
    // Set readout mode for GeSiCA
    if( sscanf(line,"%d%d%d%d",
               &fADCmode, &fLatency, &fNSample, &fOpMode )!= 4 )
      PrintError(line,"<SetConfig GeSiCA Mode Parse Error>",EErrFatal);
    fNSampleWd = fNSample/3;// 3 SADC data in each 32-bit word from GeSiCA
    break;
  case ESADCReg: 
    // "Expert Tool": Directly write to SADCs registers
    // Not currently supported
    fprintf(fLogStream,"<SADCReg not supported>\n");
    break;
  case ESADCFile:
    // Read the file for download to the SADCs
    if(sscanf(line,"%s", fpgafile ) != 1 )
      PrintError(line,"<SetConfig SADC FPGA file parse error>",EErrFatal);
    fSADCfile = BuildName(fpgafile);
    break;
  case ESADCSum:
    // Read the three sample regions for integrations:
    if( sscanf(line,"%d%d%d%d%d%d", fSamStart, fSamWidth,
               fSamStart+1, fSamWidth+1, fSamStart+2, fSamWidth+2 )!= 6 )
      PrintError(line,"<Parse Error sample boundaries>",EErrFatal);
    fPedFactor = fSamWidth[1]/fSamWidth[0];
    break;
  case ESADCThresh:
    // Read SADC threshold
    if( sscanf(line,"%d%d%d", fSADCport+fNthresh, fSADCchan+fNthresh,
               fSADCthresh+fNthresh) != 3 )
      PrintError(line,"<SetConfig Parse Threshold Error>",EErrFatal);
    fNthresh++;
    break;
  case EGeSiCALog:
    fprintf(fLogStream,"<Separate Log File not supported>\n");
    break;
  default:
    // default try commands of TDAQmodule
    TVME_CATCH::SetConfig(line, key);
    break;
  }
  return;
}

//-----------------------------------------------------------------------------
void TVME_GeSiCA::ProgFPGA()
{
  // check if TCS clocks are locked, then skip programming
  // this should make initing the system less painful...
  //UInt_t gesica_SCR = Read(EIMStatus); // *(gesica+0x20/4);
  //if((gesica_SCR & 0b11) == 0b11) {
  //  cout << "Skip programming of GeSiCa, TCS is already locked" << endl;
  //  return;
  //}
  // load the RBT file
  vector<UInt_t> rbt_data;
  if(!load_rbt(fFPGAfile, rbt_data)) {
    cerr << "Cannot load RBT file...exit" << endl;
    exit (EXIT_FAILURE);      
  }

  // the CPLD register 0x14 is used to program the GeSiCa FPGA
  // there are three relevant bits (see manual)

  // init chip
  Write(EIFPGAReg, 0x7); //*(gesica+0x14/4) = 0x7;
  Write(EIFPGAReg, 0x2); //*(gesica+0x14/4) = 0x2;

  // wait for init high (bit0)
  UInt_t nTries = 0;
  while((Read(EIFPGAReg) & 0x1) == 0) {
    nTries++;
    if(nTries==10000) {
      cerr << "Reached maximum wait time for init programming. "
           << "Last value 0x14 = " << hex << Read(EIFPGAReg)<< dec << endl;
      exit (EXIT_FAILURE); 
    }
  }
  UInt_t status = Read(EIFPGAReg);
  cout << "Start programming, status = 0x" << hex << status << dec << endl;


  // transmit data bitwise, per byte msb first
  for(UInt_t i=0;i<rbt_data.size();i++) {
    for(int j=7;j>=0;j--) { // j must be signed here
      UInt_t bit = (rbt_data[i] >> j) & 0x1;
      // first write the bit, set CCLK low
      UInt_t datum = (bit << 2);
      Write(EIFPGAReg, datum); //*(gesica+0x14/4) = datum;
      // then set CCLK => rising edge of CCLK
      datum |= 0x2;
      Write(EIFPGAReg, datum); //*(gesica+0x14/4) = datum;
    }
    if(i % (1 << 14) == 0)
      cout << "." << flush;
  }
  cout << endl;

  // check status again
  status = Read(EIFPGAReg); //*(gesica+0x14/4);
  if((status & 0x2) == 0) {
    cerr << "Status bit1 not high (not DONE), programming failed." << endl;
    exit (EXIT_FAILURE); 
  }
  cout << "Programming done, status = 0x" << hex << status << dec << endl;

  // check if clocks are locked (if not there's a maybe ConfigTCS needed...?!)
  // SCR = status and control register
  UInt_t gesica_SCR = 0;
  UInt_t tries = 0;
  do {
     gesica_SCR = Read(EIMStatus); // *(gesica+0x20/4);
     tries++;
     if(tries>1000000) {
       cerr << "Timeout while waiting for any clocks getting locked..." << endl;
       exit(EXIT_FAILURE);
     }
  }
  while((gesica_SCR & 0b11) != 0b11);

  if((gesica_SCR & 0x1) == 0) {
    cerr << "TCS clock not locked: Status = 0x"
         << hex << gesica_SCR << dec << endl;
    exit(EXIT_FAILURE);
  }
  if((gesica_SCR & 0x2) == 0) {
    cerr << "Internal clocks not locked. Status = 0x"
         << hex << gesica_SCR << dec << endl;
    exit(EXIT_FAILURE);
  }
}

//-----------------------------------------------------------------------------
void TVME_GeSiCA::ProgSADC() {
  // load the RBT file
  vector<UInt_t> rbt_data;
  if(!load_rbt(fSADCfile, rbt_data)) {
    cerr << "Cannot load RBT file...exit" << endl;
    exit (EXIT_FAILURE);      
  }
  
  // Download FPGA files to SADCs via optical link
  for(size_t i=0; i<fNSADC;i++) {
    cout << ">>>> Programming SADC at Port = " << i << endl;
    i2c_set_port(i);
    if(!ProgramSgFPGA(rbt_data)) {
      cerr << "Try reprogramming once, sometimes necessary after GeSiCa reprogramming..." << endl;
      if(!ProgramSgFPGA(rbt_data)) {
	cerr << "Could not successfully program, exit." << endl;
	exit(EXIT_FAILURE);
      }
    }
  }
}

bool TVME_GeSiCA::ProgramSgFPGA(vector<UInt_t> rbt_data)
{
  // reset FPGA
  if(!i2c_write(1, 2, 0x0))
    return false;
  
  // set program bit = bit2
  if(!i2c_write(1, 2, 0x4))
    return false;
  
  // wait for init
  UInt_t nTries = 0;
  UInt_t status = 0;
  do {
    if(!i2c_read(1, 2, status))
      return false;
    nTries++;
    if(nTries==10000) {
      cerr << "Reached maximum wait time for init programming. "
           << "Last status = " << hex << status << dec << endl;
      return false;
    }
  }
  while((status & 0x1) == 0);
  
  // write the file, 2 bytes at once
  cout << "Programming SADC..." << endl;
  for(UInt_t i=0;i<rbt_data.size();i+=2) {
    UInt_t i2c_data = (rbt_data[i+1] << 8) + rbt_data[i];
    if(!i2c_write(2, 3, i2c_data)) {
      cerr << "Failed writing at bytes=" << i << endl;
      return false;
    }
    if(i % (1 << 14) == 0)
      cout << "." << flush;
  }
  cout << endl;
  
  
  // check status, bit1 = FPGA0 done, bit3 = FPGA1 done
  if(!i2c_read(1, 2, status))
    return false;
  if(status != 0xe) {
    cerr << "Status = " << hex << status  << dec
         << " != 0xe (not both FPGAs indicate DONE), programming failed." << endl;
    return false;
  }
  
  return true;
}

//-----------------------------------------------------------------------------
void TVME_GeSiCA::ProgOpMode(){
  // Program the GeSiCA operational mode
  // ie read out all samples or integrate samples in 3 windows
  // 3 sums or 1 sum may be read
  
  bool check = true;
  
  for(UInt_t i=0; i<fNSADC; i++){
    i2c_set_port(i);

    // set all adcs to latch or sparse mode
    check &= i2c_write_reg(0, 0x3, fADCmode, true); 
    check &= i2c_write_reg(1, 0x3, fADCmode, true);
    // set latency
    check &= i2c_write_reg(0, 0x0, fLatency, true);
    check &= i2c_write_reg(1, 0x0, fLatency, true);
    // set n_samples @ adcs
    check &= i2c_write_reg(0, 0x1, fNSample, true);
    check &= i2c_write_reg(1, 0x1, fNSample, true);
    
  }
  printf(" *** SADC operational mode on GeSiCA %s programmed ***\n",
         this->GetName());
}

//-----------------------------------------------------------------------------
void TVME_GeSiCA::ProgSampleSum(){
  // Program the sample integration boundaries when operating in
  // sample integrating mode
  
  bool check = true;
  for(UInt_t i=0; i<fNSADC; i++){ // loop over all adc-boards   
    i2c_set_port(i);
    
    check &= i2c_write_reg(0, 0x4, fSamStart[0], true);  // sample 0 pedestal
    check &= i2c_write_reg(1, 0x4, fSamStart[0], true);
    check &= i2c_write_reg(0, 0x5, fSamWidth[0], true);
    check &= i2c_write_reg(1, 0x5, fSamWidth[0], true);
    check &= i2c_write_reg(0, 0x6, fSamStart[1], true);  // sample 1 signal
    check &= i2c_write_reg(1, 0x6, fSamStart[1], true);
    check &= i2c_write_reg(0, 0x7, fSamWidth[1], true);
    check &= i2c_write_reg(1, 0x7, fSamWidth[1], true);
    check &= i2c_write_reg(0, 0x8, fSamStart[2], true);  // sample 2 tail
    check &= i2c_write_reg(1, 0x8, fSamStart[2], true);
    check &= i2c_write_reg(0, 0x9, fSamWidth[2], true);
    check &= i2c_write_reg(1, 0x9, fSamWidth[2], true);
  }
  if(!check)
    PrintError("","Failed writing integration windows...", EErrFatal);
  printf(" *** SADC sample sums on GeSiCA %s programmed ***\n",
         this->GetName());
  
}

//-----------------------------------------------------------------------------
void TVME_GeSiCA::ProgThresh(){
  // Program the SADC thresholds
  Int_t chip, reg;
  if( fNthresh != fNChannel )
    PrintError("","<Incompatible number thresholds read in>", EErrFatal);
  bool check = true;
  for( Int_t i=0; i<fNthresh; i++ ){
    i2c_set_port(fSADCport[i]);

    if( fSADCchan[i] < 16 ){
      chip = 0;
      reg = fSADCchan[i] + 0x10;
    }
    else {
      chip = 1;
      reg = fSADCchan[i] - 16 + 0x10;
    }
    i2c_write_reg(chip, reg, fSADCthresh[i], true);
    fprintf(fLogStream, "SADCThreshold: %s %4d %4d %4d\n", 
            GetName(), fSADCport[i], fSADCchan[i], fSADCthresh[i]);
  }
  if(!check)
    PrintError("","Failed writing thresholds...", EErrFatal);
  printf(" *** SADC thresholds on GeSiCA %s programmed ***\n",
         this->GetName());
}



// I2C routines by A. Neiser

bool TVME_GeSiCA::i2c_wait(bool check_ack) {
  // poll status register 0x4c bit 0,
  // wait until deasserted
  for(UInt_t n=0;n<1000;n++) {
    UInt_t status = Read(EI2CStatus); // *(gesica+0x4c/4);
    if((status & 0x1) == 0) {
      //cout << "# After " << n << " reads: 0x4c = 0x" << hex << (status & 0x7f) << dec << endl;
      // check acknowledge bit
      if(check_ack && (status & 0x8) != 0) {
        cerr << "Address or data was not acknowledged " << endl;
        return false;
      }
      return true;
    }
  }
  cerr << "Did not see Status Bit deasserted, timeout." << endl;
  return false;
}

bool TVME_GeSiCA::i2c_reset() {
  Write(EI2CAddrCtl, 0x40); // *(gesica+0x48/4) = 0x40;
  return i2c_wait(false);
}

void TVME_GeSiCA::i2c_set_port(UInt_t port_id, bool broadcast) {
  Write(EIHlPort, port_id & 0xff ); // *(gesica+0x2c/4) = port_id & 0xff;
  if(broadcast) {
    Write(EI2CPortSelect, 0xff); // *(gesica+0x50/4) = 0xff; // should be broadcast mode
  }
  else {
    Write(EI2CPortSelect, port_id & 0xff); // *(gesica+0x50/4) = port_id & 0xff;
  }
}

bool TVME_GeSiCA::i2c_read(UInt_t bytes, UInt_t addr, UInt_t& data) {
  // bytes must be between 1 and 4
  
  // write in address/control register (acr)
  // always set bit7 (trigger i2c transmission),
  // and bit4 (read mode)
  UInt_t acr = (1 << 7) + (1 << 4);
  // this nicely configures the i2c number of bytes
  acr |= bytes << 2;
  // set the address in the upper byte
  acr |= (addr << 8) & 0x7f00;
  //cout << "# Read Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  Write(EI2CAddrCtl, acr); // *(gesica+0x48/4) = acr;
  
  if(!i2c_wait(true))
    return false;
  
  
  // always read the lower data register 0x44
  data = Read(EI2CdrLow); //data = *(gesica+0x44/4);
  
  // take care of upper data reg 0x58 and masking
  UInt_t mask = bytes % 2 == 0 ? 0xffff : 0xff;
  if(bytes>2) {
    data |= (Read(EI2CdrHigh) & mask) << 16;
  }
  else {
    data &= mask;
  }
  return true;
}

bool TVME_GeSiCA::i2c_write(UInt_t bytes, UInt_t addr, UInt_t data) {
  // bytes must be between 1 and 4
  
  // always write 16bits to low register 0x40
  Write(EI2CdwLow, data & 0xffff); // *(gesica+0x40/4) = data & 0xffff;
  
  // if needed, use upper data register 0x54
  if(bytes>2) {
    Write(EI2CdwHigh, (data >> 16) & 0xffff); //*(gesica+0x54/4) = (data >> 16) & 0xffff;
  }
  
  // write in address/control register (acr)
  // set bit7 (trigger i2c transmission),
  // but bit4=0 (write mode)
  UInt_t acr = 1 << 7;
  // assuming bytes>0 && bytes<4 (the caller should know that)
  // this nicely configures the i2c number of bytes
  acr |= bytes << 2;
  // set the address in the upper byte
  acr |= (addr << 8) & 0x7f00;
  //cout << "# Write Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  Write(EI2CAddrCtl, acr); //*(gesica+0x48/4) = acr;
  
  return i2c_wait(true);
}

UInt_t i2c_make_reg_addr(UInt_t adc_side, UInt_t reg) {
  // in order to access the registers of the
  // ZR chips via the HL chip on the SADC,
  // we set the highest bit6 in i2c addr
  UInt_t addr = 1 << 6;
  // select the chip via bit5 (adc_side=1 and adc_side=0)
  addr |= (adc_side & 0x1) << 5;
  // and set the actual register (bit4-bit0)
  addr |= reg & 0x1f;
  return addr;
}

bool TVME_GeSiCA::i2c_read_reg(UInt_t adc_side, UInt_t reg, UInt_t& data) {
  
  UInt_t addr = i2c_make_reg_addr(adc_side, reg);
  
  // this write before reading is really necessary...
  // does it tell the HL chip to access this address of one of the ZRs?
  if(!i2c_write(1, addr, 0))
    return false;
  
  // read the response, maximum two bytes
  if(!i2c_read(2, addr, data))
    return false;
  
  return true;
}

bool TVME_GeSiCA::i2c_write_reg(UInt_t adc_side, UInt_t reg, UInt_t data, bool check) {
  
  UInt_t addr = i2c_make_reg_addr(adc_side, reg);
  
  // tell the HL chip to access this address of the ZR?
  // this was not in the old code, but a complete i2c_read_reg read was before that
  // to make it more reliable........?!?!?!
  //if(!i2c_write(gesica, 1, addr, 0))
  //  return false;
  
  // construct our data, the lower 8 bits must be zero?
  UInt_t i2c_data = data << 8;
  if(!i2c_write(3, addr, i2c_data))
    return false;
  
  // check if write was successful
  if(check) {
    UInt_t read_data;
    if(!i2c_read_reg(adc_side, reg, read_data))
      return false;
    if(read_data != data)
      return false;
  }  
  
  return true;
}

bool TVME_GeSiCA::load_rbt(const char* rbt_filename, vector<UInt_t>& data) {
  ifstream rbt_file(rbt_filename);
  if(!rbt_file.is_open()) {
    cerr << "Could not open RBT file " << rbt_filename << endl;
    return false;
  }
  cout << "Opened RBT file " << rbt_filename << endl;
  string line;
  UInt_t lineno = 0;
  UInt_t numOfBits = 0;
  while(getline(rbt_file,line)) {
    lineno++;
    if(lineno==7) {
      stringstream ss(line);
      ss >> line; //  dump leading string
      ss >> numOfBits;
    }
    if(lineno<=7)
      continue;
    
    // convert bit line to number
    bitset<32> bits(line);
    UInt_t word = bits.to_ulong();
    data.push_back((word >> 24) & 0xff);
    data.push_back((word >> 16) & 0xff);
    data.push_back((word >>  8) & 0xff);
    data.push_back((word >>  0) & 0xff);
  }
  // check
  if(data.size() != numOfBits/8) {
    cerr << "Not enough bits read as promised in header" << endl;
    return false;
  }
  return true;
}


bool TVME_GeSiCA::init_gesica(bool skip_i2c) {
  
  // the CPLD module ID is at 0x0, should be 0x440d5918
  if(Read(EIBase) != 0x440d5918) {
    cerr << "Gesica firmware invalid, wrong address?" << endl;
    return false;
  }

  // enable readout via VME, but disable everything else like debugging pulsers
  // and all modules
  Write(EIMStatus, 0x4); // *(gesica+0x20/4) = 0x4;

  // if the Gesica FPGA itself is programmed,
  // the TCS (and thus i2c) might not be working...
  if(skip_i2c)
    return true;

  
  // Init connected iSADC cards
  UInt_t init_tries = 10;
  
  do {
    UInt_t gesica_SCR = Read(EIMStatus); // *(gesica+0x20/4);
    vector<UInt_t> ports;  
    for(UInt_t port_id=0;port_id<fNSADC;port_id++) {
      if( (gesica_SCR & (1 << (port_id+8))) == 0) {
	//cout << "Skipped..." << endl;
	// silently ignore unconnected cards...
	continue;
      }
      // set to broadcast mode
      i2c_set_port(port_id, true);
      // read hardwired id
      UInt_t hard_id;
      if(!i2c_read(1, 0x0, hard_id))
	continue;
      // write geo id as port_id:
      // hard_id as the lower 8 bits, port id the higher 8 bits!
      if(!i2c_write(2, 0x1, hard_id  + (port_id <<8)))
	continue;
      // readback geo id
      UInt_t geo_id;
      if(!i2c_read(1, 0x1, geo_id))
	continue;
      if(geo_id != port_id) {
	cerr << "Setting Geo ID for port " << port_id << " failed: GeoID=" << geo_id << endl;
	continue;
      }
      ports.push_back(port_id);
      // enable interface for readout (two VME accesses, too lazy too use bitset here...)
      //*(gesica+0x20/4) |= 1 << (port_id+16);
      UInt_t data = Read(EIMStatus);
      data |= 1 << (port_id+16);
      Write(EIMStatus, data);
      //cout << "Found..." << endl;
    }
    init_tries--;

    if(ports.size() == fNSADC)
      break;
    
    if(init_tries==0) {
      cerr << "Expected " << fNSADC << " SADCs connected, " 
	   << " but found " << ports.size() << endl;
      cerr << "Try replugging the optical cable connection, or powercycling the crate" << endl;
      return false;
    }
    
    cerr << "Retrying initing the SADC modules. Sometimes necessary after GeSiCa reprogramming." << endl;

    // give the GeSiCa some time to find the SADC modules...
    usleep(10000);
  }
  while(init_tries>0);

  return true;
}


