//--Author	JRM Annand   19th May 2012
//--Rev 	...
//--Rev 	JRM Annand    4th Jul 2012 Working version
//--Rev 	JRM Annand    7th Jul 2012 Add some functions
//--Rev  	JRM Annand   12th Jul 2012 Add online control
//--Rev 	JRM Annand   29th Aug 2012 Various L1 width, strobe delay
//--Rev 	JRM Annand    2nd Sep 2012 TAPS enable
//--Rev 	JRM Annand    3rd Sep 2012 Default fType Slow-Ctrl
//--Rev 	B Oussena    19th Sep 2012 Bug fix L1,2 prescale address
//--Rev 	JRM Annand   26th Sep 2012 Add registers for multiplicity
//--Rev 	JRM Annand   28th Sep 2012 Pattern readout and extra ctrl fns
//--Rev 	JRM Annand   29th Nov 2012 Timeout 100 us Event ID read
//--Rev  	JRM Annand    1st Dec 2012 Scaler read only, Ref TDC width ctrl
//--Rev 	JRM Annand    2nd Dec 2012 Mod RAM download (buff size & <=) 
//--Rev 	JRM Annand   28th Feb 2013 Modified L1,L2 prescale setup 
//--Update 	JRM Annand   10th Sep 2013 Reg. map changes for expanded DAQ 
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROM
// GSI VUPROM FPGA base VMEbus module
// Has a multitude of functions, depending on the programing of the FPGA
// Trigger condition processor
// DAQ control master, up to 7 slave VME systems
// 196 channel scaler
// Hit pattern register
//

#include "TVME_VUPROM.h"
#include "TDAQexperiment.h"


enum { EVUP_EnCPU=200, EVUP_IntDelay, EVUP_FCDelay, EVUP_RAMDownload,
       EVUP_InputMask, EVUP_InputPrescale, EVUP_L1Prescale, EVUP_L2Prescale,
       EVUP_EnPattRead, EVUP_EnScalerRead, EVUP_SetDebug, EVUP_L1Width,
       EVUP_SetDelay, EVUP_EnableTAPS, EVUP_HelicityInhibit, EVUP_SetMThresh, 
       EVUP_GUIpermit, EVUP_RefTDCWidth, EVUP_ResetTrigCtrl };
static Map_t kVUPROMKeys[] = {
  {"Enable-CPU:",         EVUP_EnCPU},
  {"Int-Delay:",          EVUP_IntDelay},
  {"FC-Delay:" ,          EVUP_FCDelay},
  {"RAM-Download:" ,      EVUP_RAMDownload},
  {"Input-Mask:" ,        EVUP_InputMask},
  {"Input-Prescale:" ,    EVUP_InputPrescale},
  {"L1-Prescale:" ,       EVUP_L1Prescale},
  {"L2-Prescale:" ,       EVUP_L2Prescale},
  {"En-Pattern-Read:" ,   EVUP_EnPattRead},
  {"En-Scaler-Read:" ,    EVUP_EnScalerRead},
  {"Set-Debug-Out:" ,     EVUP_SetDebug},
  {"L1-Width:" ,          EVUP_L1Width},
  {"Strobe-Delay:" ,      EVUP_SetDelay},
  {"Enable-TAPS:" ,       EVUP_EnableTAPS},
  {"Helicity-Inhibit:" ,  EVUP_HelicityInhibit},
  {"Set-M-Threshold:" ,   EVUP_SetMThresh},
  {"GUI-permit:" ,        EVUP_GUIpermit},
  {"RefTDC-Width:" ,      EVUP_RefTDCWidth},
  {"ResetTrigCtrl:" ,     EVUP_ResetTrigCtrl},
  {NULL,                  -1}
};

// address offsets for internal VUPROM registers
VMEreg_t VUPROMreg[] = {
  {0x2f00,      0x0,  'l', 0},       // Firmware version
  {0x2100,      0x0,  'l', 0},       // Multiplicity threshold 1
  {0x2110,      0x0,  'l', 0},       // Multiplicity threshold 2
  {0x2150,      0x0,  'l', 0},       // L1 mask register
  {0x2160,      0x0,  'l', 0},       // Input prescale reg.1
  {0x2170,      0x0,  'l', 0},       // Input prescale reg.2
  {0x2190,      0x0,  'l', 0},       // Read Multiplicity value
  {0x21a0,      0x0,  'l', 0},       // Read Multiplicity pattern 1
  {0x21b0,      0x0,  'l', 0},       // Read Multiplicity pattern 2
  {0x2200,      0x0,  'l', 0},       // RAM1 enable
  {0x2210,      0x0,  'l', 0},       // RAM1 address
  {0x2220,      0x0,  'l', 0},       // RAM1 data
  {0x2230,      0x0,  'l', 0},       // RAM1 readback
  {0x2240,      0x0,  'l', 0},       // RAM2 enable
  {0x2250,      0x0,  'l', 0},       // RAM2 address
  {0x2260,      0x0,  'l', 0},       // RAM2 data
  {0x2270,      0x0,  'l', 0},       // RAM2 readback
  /*
  {0x2280,      0x0,  'l', 0},       // L1 prescale
  {0x2290,      0x0,  'l', 0},       // L2 prescale
  */
  {0x2300,      0x0,  'l', 0},       // L1 prescale
  {0x2310,      0x0,  'l', 0},       // L1 prescale-1
  {0x2320,      0x0,  'l', 0},       // L1 prescale-2
  {0x2330,      0x0,  'l', 0},       // L1 prescale-3
  {0x2340,      0x0,  'l', 0},       // L2 prescale
  {0x2350,      0x0,  'l', 0},       // L2 prescale-1
  {0x2360,      0x0,  'l', 0},       // L2 prescale-2
  {0x2370,      0x0,  'l', 0},       // L2 prescale-3
  {0x22a0,      0x0,  'l', 0},       // Input pattern read
  {0x22b0,      0x0,  'l', 0},       // Read Trigger fired pattern
  {0x22c0,      0x0,  'w', 0},       // Width of shaped L1 output
  {0x22d0,      0x0,  'w', 0},       // L1 strobe internal delay
  {0x22e0,      0x0,  'w', 0},       // L2 strobe internal delay
  {0x22f0,      0x0,  'w', 0},       // Ref TDC output width
  {0x2400,      0x0,  'l', 0},       // CPU Control
  {0x2410,      0x0,  'l', 0},       // Trigger Control
  {0x2420,      0x0,  'l', 0},       // Enable TAPS
  {0x2430,      0x0,  'l', 0},       // CPU reset
  {0x2440,      0x0,  'l', 0},       // Master reset
  {0x2450,      0x0,  'l', 0},       // End of readout acknowledge
  {0x2460,      0x0,  'l', 0},       // IRQ reg.
  {0x2470,      0x0,  'l', 0},       // TCS Control
  {0x2480,      0x0,  'l', 0},       // MAMI Beam Helicity Inhibit  
  {0x2490,      0x0,  'l', 0},       // MAMI Beam Helicity Pattern 
  {0x2500,      0x0,  'l', 0},       // Interrupt delay CPU-0
  {0x2510,      0x0,  'l', 0},       // Interrupt delay CPU-1
  {0x2520,      0x0,  'l', 0},       // Interrupt delay CPU-2
  {0x2530,      0x0,  'l', 0},       // Interrupt delay CPU-3
  {0x2540,      0x0,  'l', 0},       // Interrupt delay CPU-4
  {0x2550,      0x0,  'l', 0},       // Interrupt delay CPU-5
  {0x2560,      0x0,  'l', 0},       // Interrupt delay CPU-6
  {0x2570,      0x0,  'l', 0},       // Interrupt delay CPU-7
  {0x2580,      0x0,  'l', 0},       // Interrupt delay CPU-8
  {0x2590,      0x0,  'l', 0},       // Interrupt delay CPU-9
  {0x25a0,      0x0,  'l', 0},       // Interrupt delay CPU-10
  {0x25b0,      0x0,  'l', 0},       // Interrupt delay CPU-11
  {0x25c0,      0x0,  'l', 0},       // Interrupt delay CPU-12
  {0x25d0,      0x0,  'l', 0},       // Interrupt delay CPU-13
  {0x25f0,      0x0,  'l', 0},       // Fast clear delay (all)
  {0x2a00,      0x0,  'l', 0},       // Event ID (send) register
  {0x2a10,      0x0,  'l', 0},       // Event ID (send) trigger
  {0x2a20,      0x0,  'l', 0},       // Event ID status
  {0x2e00,      0x0,  'l', 0},       // Read back debug switch
  {0x2e10,      0x0,  'l', 0},       // Set debug output 0
  {0x2e20,      0x0,  'l', 0},       // Set debug output 1
  {0x2e30,      0x0,  'l', 0},       // Set debug output 2
  {0x2e40,      0x0,  'l', 0},       // Set debug output 3
  {0x1800,      0x0,  'l', 0},       // Clear scalers
  {0x1804,      0x0,  'l', 0},       // Load scalers
  {0x1000,      0x0,  'l', 195},     // Scaler registers 0-195
  {0xffffffff,  0x0,  'l', 0},       // end of list
};

//-----------------------------------------------------------------------------
TVME_VUPROM::TVME_VUPROM( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = new TDAQcontrol( this );         // tack on control functions
  fType = EDAQ_SlowCtrl;                   // primarily slow ctrl
  AddCmdList( kVUPROMKeys );               // VUPROM-specific setup commands
  fTCSresetDelay = 550000;                 // default TCS reset wait 0.55s
  fFCDelay = 0;                            // default fast clear delay
  fL1Width = 1;                            // default width output 1
  fL1Delay = fL2Delay = 2;                 // default internal delays
  fRefTDCWidth = 5;
  for(Int_t i=0; i<EVU_MaxCPU; i++) fIntDelay[i] = 0;
  for(Int_t i=0; i<16; i++) fInputPrescale[i] = 0;
  for(Int_t i=0; i<16; i++) fL1Prescale[i] = 0;
  for(Int_t i=0; i<16; i++) fL2Prescale[i] = 0;
  for(Int_t i=0; i<16; i++) fDebugOut[i] = -1;
  for(Int_t i=0; i<16; i++) fMThresh[i] = 0;
  fRAMid = 0;
  fInputMask = 0;
  fNScalerChan = 196;                      // number scaler channels
  fNreg = EVU_Scaler + fNScalerChan + 2;   // control and scaler registers
  fIsPattRead = kFALSE;                    // default no pattern read
  fIsScalerRead = kFALSE;                  // default not only a scaler
  fTAPSEnable = 0;                         // default TAPS not in busy circuit
  fHelicityInhibit = 0;                    // default Helicity not in busy circuit
  fIsGUIpermit = kFALSE;                   // default no change from GUI
  fIsTCSStarted = kTRUE;                   // VUPROM will stop it by default
}

//-----------------------------------------------------------------------------
TVME_VUPROM::~TVME_VUPROM( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_VUPROM::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  Int_t i,j;
  switch( key ){
  case EVUP_EnCPU:
    // Input number to specify which CPUs are enabled. Bit set to 1 = enabled 
    if(sscanf(line,"%x",&fEnableCPU) != 1)
      PrintError(line,"<CPU enable switch>",EErrFatal);
    break;
  case EVUP_IntDelay:
    // Set the interrupt delays of up to 14 coupled CPUs
    if(sscanf(line,"%d%d",&i,&j) != 2)
      PrintError(line,"<Interrupt Delay Spec. Format>",EErrFatal);
    if(i >= EVU_MaxCPU)
      PrintError(line,"<Interrupt Delay Invalid CPU ID>",EErrFatal);
    fIntDelay[i] = j;
    break;
  case EVUP_FCDelay:
    // Set the Fast Clear delay (until re-arm trigger) 1 unit = 10ns
    if(sscanf(line,"%d",&fFCDelay) != 1)
      PrintError(line,"<Fast Clear Delay Format>",EErrFatal);
    break;
  case EVUP_RAMDownload:
    // Specify the ID number for the trigger files to load into RAM
    if(sscanf(line,"%d",&fRAMid) != 1)
      PrintError(line,"<Trigger file ID Format>",EErrFatal);
    break;
  case EVUP_InputMask:
    // Masking bits for input signals. 16 inputs. Bit set to 1 == enabled
    if(sscanf(line,"%x",&fInputMask) != 1)
      PrintError(line,"<Input Mask input Format>",EErrFatal);
    break;
  case EVUP_InputPrescale:
    // Prescale values for Input trigger signals
    if(sscanf(line,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
	      fInputPrescale,   fInputPrescale+1, fInputPrescale+2, fInputPrescale+3,
	      fInputPrescale+4, fInputPrescale+5, fInputPrescale+6, fInputPrescale+7,
	      fInputPrescale+8, fInputPrescale+9, fInputPrescale+10,fInputPrescale+11,
	      fInputPrescale+12,fInputPrescale+13,fInputPrescale+14,fInputPrescale+15 ) != 16)
      PrintError(line,"<Input Prescale input Format>",EErrFatal);
    break;
  case EVUP_L1Prescale:
    // Prescale values for Level-1 trigger signals
    if(sscanf(line,"%d%d%d%d%d%d%d%d",
	      fL1Prescale,   fL1Prescale+1, fL1Prescale+2, fL1Prescale+3,
	      fL1Prescale+4, fL1Prescale+5, fL1Prescale+6, fL1Prescale+7 ) != 8)
      PrintError(line,"<L1 Prescale input Format>",EErrFatal);
    break;
  case EVUP_L2Prescale:
    // Prescale values for Level-2 trigger signals
    if(sscanf(line,"%d%d%d%d%d%d%d%d",
	      fL2Prescale,   fL2Prescale+1, fL2Prescale+2, fL2Prescale+3,
	      fL2Prescale+4, fL2Prescale+5, fL2Prescale+6, fL2Prescale+7 ) != 8)
      PrintError(line,"<L2 Prescale input Format>",EErrFatal);
    break;
  case EVUP_EnPattRead:
    // Enable readout of L1, L2 and multiplicity input patterns
    // In this case no trigger control is performed
    fIsPattRead = kTRUE;
    fNChannel = 6+1+1+1; // patterns + multiplicity + helicity + trigger fired
    fType = EDAQ_ADC;
    break;
  case EVUP_EnScalerRead:
    // Enable readout of scalers only
    // All trigger control not enabled
    fIsScalerRead = kTRUE;
    fType = EDAQ_Scaler;
    fNScalerChan = fNChannel;
    break;
  case EVUP_SetDebug:
    // Choose which internal signal is switched into the debug outputs
    if(sscanf(line,"%i%i",&i,&j) != 2)
      PrintError(line,"<Debug switching input Format>",EErrFatal);
    fDebugOut[i] = j;
    break;
  case EVUP_L1Width:
    // Width of L1 output 1
    if(sscanf(line,"%i",&i) != 1)
      PrintError(line,"<L1 width input Format>",EErrFatal);
    fL1Width = i;
    break;
  case EVUP_RefTDCWidth:
    // Width of reference TDC output
    if(sscanf(line,"%i",&i) != 1)
      PrintError(line,"<Ref TDC width input Format>",EErrFatal);
    fRefTDCWidth = i;
    break;
  case EVUP_SetDelay:
    // Choose internal L1 and L2 strobe delays
    if(sscanf(line,"%i%i",&fL1Delay,&fL2Delay) != 2)
      PrintError(line,"<Internal delays input Format>",EErrFatal);
    break;
  case EVUP_EnableTAPS:
    // Switch TAPS busy signal into the total dead time circuit
    if(sscanf(line,"%i",&i) != 1)
      PrintError(line,"<TAPS enable input Format>",EErrFatal);
    fTAPSEnable = i;
    break;
  case EVUP_HelicityInhibit:
	  // Helicity bit setting into the total dead time circuit
	  if(sscanf(line,"%i",&i) != 1)
		  PrintError(line,"<Helicity Inhibit input Format>",EErrFatal);
	  fHelicityInhibit = i;
	  break;
  case EVUP_SetMThresh:
    // Set threshold values on M signals
    if(sscanf(line,"%i%i%i%i",fMThresh,fMThresh+1,fMThresh+2,fMThresh+3) != 4)
      PrintError(line,"<M Thresholds input Format>",EErrFatal);
    break;
  case EVUP_GUIpermit:
    // Enable change of trigger conditions via GUI
    fIsGUIpermit = kTRUE;
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VUPROM::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  InitReg( VUPROMreg );
  TVMEmodule::PostInit();
  if( fIsPattRead || fIsScalerRead ) return;
  Write(EVU_L1mask, fInputMask);           // enable/disable L1 inputs
  Write(EVU_CPUctrl,fEnableCPU);           // which CPUs enabled
  Write(EVU_FCDelay,fFCDelay);             // fast clear delay
  Write(EVU_L1Width,fL1Width);             // shape output 1 fL1Width units
  Write(EVU_L1Delay,fL1Delay);             // Set L1 strobe delay
  Write(EVU_L2Delay,fL2Delay);             // Set L2 strobe delay
  Write(EVU_RefTDCWidth,fRefTDCWidth);     // Set Ref TDC output width
  // Interrupt delays cpu's 0-7
  for( Int_t i=0; i<EVU_MaxCPU; i++ )
    SetIntDelay(i, fIntDelay[i]);
  // Switch internal signals to debug outputs, if this is enabled
  if( fDebugOut[0] >= 0 ) SetDebugOut(0, fDebugOut[0]);
  if( fDebugOut[1] >= 0 ) SetDebugOut(1, fDebugOut[1]);
  if( fDebugOut[2] >= 0 ) SetDebugOut(2, fDebugOut[2]);
  if( fDebugOut[3] >= 0 ) SetDebugOut(3, fDebugOut[3]);
  // load trigger conditions in to RAM
  if( fRAMid ){
    RAMDownload(1);
    RAMDownload(2);
  }
  // load trigger prescale values
  for(Int_t i=0; i<4; i++)
    SetPrescale(i);
  Write(EVU_EnableTAPS,fTAPSEnable); // enable TAPS busy if desired
  Write(EVU_HelicityInhibit,fHelicityInhibit); // Helicity inhibit if desired
  SetMThresh(0);                     // set the M thresholds
  EndTrigCtrl();                     // ensure triggers disabled & TCS stopped
  return;
}


//-------------------------------------------------------------------------
void TVME_VUPROM::RunTrigCtrl()
{
  // Start trigger control system and enable triggers
  // mask all channels except 0 (0-15) and execute F25 test
  // beginning of spill
  StartTCS();                 // Issue TCS start
  usleep(fTCSresetDelay);     // allow time TCS to reset itself
  fEXP->PostReset();          // any additional resetting after start
  Write(EVU_ScalerClr,1);     // clear the scalers
  EnableTrigCtrl();           // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROM::EndTrigCtrl()
{
  // Stop trigger control system and mark "end of spill"
  // and set overall inhibit
  DisableTrigCtrl();
  usleep(10);
  StopTCS();      // Issue TCS stop
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROM::ResetTrigCtrl()
{
  // Disable triggers and reset the trigger control system.
  // Issue a stop followed by start pulse
  //
  DisableTrigCtrl();
  // Tell TCS end of spill
  StopTCS();                  // Issue TCS stop
  usleep(1);                  // pause micro-sec
  // Tell TCS beginning of spill
  StartTCS();                 // Issue TCS start
  usleep(fTCSresetDelay);     // allow time TCS to reset itself
  fEXP->PostReset();          // any additional resetting after start
  EnableTrigCtrl();           // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROM::EnableTrigCtrl()
{
  // Issue master reset and enable triggers
  //
  //  Write( EVU_CPUreset, 1 );
  Write( EVU_MasterReset, 1 );
  Write( EVU_TrigCtrl, (UInt_t)0x0 );
}

//-------------------------------------------------------------------------
void TVME_VUPROM::DisableTrigCtrl()
{
  // Disable triggers
  //
  Write( EVU_TrigCtrl, 0x1 );
}

//-------------------------------------------------------------------------
void TVME_VUPROM::RAMDownload(Int_t Lx, Int_t ramid)
{
  // Load file to specify trigger patterns into VUPROM RAM
  // The files are created using P.Otte's web based tool
  // See http://wwwa2intern.office.a2.kph/intern/trigger/
  Char_t trFile[64];
  if( ramid >=  0) fRAMid = ramid;                       // if ramid provided, use it
  sprintf(trFile,"data/TriggerRAML%d_%d.dat",Lx,fRAMid);
  FILE* ftr = fopen(trFile,"rt");
  if( ftr )
    fprintf(fLogStream,"Reading L%d trigger conditions from %s\n",Lx,trFile);
  else
    PrintError(trFile,"<Trigger data file not found>",EErrFatal);
  UInt_t iaddr = 0;
  Long_t datum;
  Long_t data[0x10000];
  Char_t line[80];
  Int_t renable,raddr,rdata,rread;
  // Choose registers for Level 1 or Level 2 configuration
  if( Lx == 1 ){
    renable = EVU_R1enable; raddr = EVU_R1addr;
    rdata = EVU_R1data; rread = EVU_R1read;
  }
  else{
    renable = EVU_R2enable; raddr = EVU_R2addr;
    rdata = EVU_R2data; rread = EVU_R2read;
  }
  //  A frig suggested by P.O.
  //  Write(renable,1);
  while(fgets(line, 80, ftr)!= NULL) {
    sscanf (line, "%li", &datum);
    data[iaddr] = datum;
    Write(raddr,iaddr);
    Write(rdata,datum);
    Write(renable,1);
    Write(renable,(UInt_t)0);
    iaddr++;
  }
  //  Write(renable,(UInt_t)0);
  fclose(ftr);
  if( (iaddr-1) != 0xffff )
    PrintError(trFile,"<Incorrect number (%d) lines read>",EErrFatal);
  // Now read back RAM contents to check if OK
  Int_t err = 0;
  for( iaddr = 0; iaddr <=0xffff; iaddr++ ){
    Write(raddr,iaddr);
    datum = Read(rread);
    if( datum != data[iaddr] ) err++;
  }
  if( err == 0 )
    fprintf(fLogStream,"Trigger L%d RAM downloaded successfully\n",Lx);
  else{
    fprintf(fLogStream,"%d Readback errors detected\n",err);
    PrintError("","<Readback errors on RAM download>",EErrFatal);
  }
  return;
}

//-----------------------------------------------------------------------------
void TVME_VUPROM::ReadIRQScaler( void** outBuffer )
{
  // Read the scaler registers from the VUPROM
  //
  Int_t i,j;
  UInt_t datum;
  Write(EVU_ScalerLoad, 1);          // load scalers into buffer
  for(i=0; i<fNScalerChan; i++){     // read 196 scaler channels
    j = i + EVU_Scaler;
    datum = Read(j);
    ScalerStore( outBuffer, datum, fBaseIndex+i );
  }
  Write(EVU_ScalerClr,1);           // clear the scalers
}

//---------------------------------------------------------------------------
void TVME_VUPROM::ReadIRQ( void** outBuffer )
{
  // Read and store the trigger pattern register 32 bits and multiplicity input 2 x 31 bits
  // 16 lsb L1 pattern
  // 16 msb L2 pattern
  // 
  UInt_t datum[3], datumlow, datumhigh;
  datum[0] = Read(EVU_InPattRead);         // L1 and L2 Patterns
  datum[1] = Read(EVU_RdMPattern1);        // 1st set multiplicity
  datum[2] = Read(EVU_RdMPattern2);        // 2nd set multiplicity
  Int_t j = fBaseIndex; // ADC numbering starts at the fBaseIndex
  for( Int_t i=0; i<3; i++ ){
    datumlow = datum[i] & 0xffff;
    datumhigh = datum[i] >> 16;
    ADCStore( outBuffer, datumlow, j );
    j++; 
    ADCStore( outBuffer, datumhigh, j );
    j++;
  }
  datumlow = Read(EVU_RdMValue);          // read calculated multiplicity
  ADCStore( outBuffer, datumlow, j );
  j++;
  
  // also read the beam helicity bit register,
  // only bit3-bit0 are actually of interest
  datumlow = Read(EVU_HelicityPattern) & 0xffff;
  ADCStore( outBuffer, datumlow, j );
  j++;
  
  // read the pattern which trigger has fired
  // only lowest 8 bits (7-0) are relevant
  datumlow = Read(EVU_TriggerFiredPatt) & 0xffff;
  ADCStore( outBuffer, datumlow, j );
  j++;
}

//-------------------------------------------------------------------------
Bool_t TVME_VUPROM::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read(EVU_Firmware);
  fprintf(fLogStream,"VUPROM firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VUPROM firmware ID error>",EErrFatal);
  return kFALSE;
}
//--------------------------------------------------------------------------
void TVME_VUPROM::SetDebugOut(Int_t i, Int_t j)
{
  // Switch internal signal #j to debug output #i
  //
  Int_t k;
  printf("Switching signal %d to debug output: %d\n",i,j);
  // Select which debug register to set
  switch(i){
  case 0:
    k = EVU_SetDebug0;
    break;
  case 1:
    k = EVU_SetDebug1;
    break;
  case 2:
    k = EVU_SetDebug2;
    break;
  case 3:
    k = EVU_SetDebug3;
    break;
  default:
    break;
  }
  Write(k,j);
  // read back status of selected debug port and print out
  printf("Read back status: %x after debug switch: %d\n",Read(EVU_ReadDebug),k);
  return;
}
//

//--------------------------------------------------------------------------
void TVME_VUPROM::SetPrescale(Int_t section, Int_t chan, Int_t prescale)
{
  // Set prescaling values for trigger signals
  // prescale value 2^prescale. 0 = no prescale, 0xf = off
  Int_t nchan = 8;
  Int_t port;
  UInt_t datum = 0;
  Int_t* start;
  Int_t offset = 0;
  Int_t pre;
  switch(section){
  case 0:
    // 1st 8 input channels
    port = EVU_InpPrescale1;
    start = fInputPrescale;
    break;
  case 1:
    // 2nd 8 input channels
    port = EVU_InpPrescale2;
    start = fInputPrescale + 8;
    break;
  case 2:
    // L1 output channels
    port = EVU_L1prescale;
    start = fL1Prescale;
    break;
  case 3:
    // L2 output channels
    port = EVU_L2prescale,
    start = fL2Prescale;
    break;
  default:
    printf("Invalid section %d\n",section);
    break;
  }
  // write trigger input prescale datum to register
  if( section < 2 ){
    for( Int_t i=0; i<nchan; i++ ){
      offset = i * 4;
      if( i == chan ) pre = start[i] = prescale; // take new value
      else pre = start[i];                       // use preloaded value
      datum |= (pre & 0xf)<<offset;              // only 4 bits
    }
    printf("Section %d prescale pattern: %x\n", section, datum);
    Write(port, datum);       // write to the VUPROM prescale register
  }
  // write L1 or L2 prescale value to registers
  else{
    // Check for any new prescale value, else use preloaded values
    for( Int_t i=0; i<nchan; i++ ){
      if( i == chan ) start[i] = prescale; 
    }
    for( Int_t i=0; i<nchan; i+=2){
      Int_t lsb = (start[i] & 0xffff);
      Int_t msb = (start[i+1] & 0xffff);
      datum = lsb | (msb << 16) ;
      Int_t j = i/2;
      printf("Section %d, Chan:%d Prescale:%d; Chan:%d Prescale:%d\n",
	     section,i,lsb,i+1,msb);
      Write(port+j,datum);
    }
  }
  return;
}

//--------------------------------------------------------------------------
void TVME_VUPROM::SetIntDelay(Int_t cpu, Int_t delay)
{
  // write interrupt delay value (ns*10) for particular CPU
  //
  if( cpu >= EVU_MaxCPU ){
    printf(" VUPROM Invalid CPU number: %d\n",cpu);
    return;
  }
  Int_t creg = EVU_IntDelay0 + cpu;
  Write(creg, delay);
  return;
}

//--------------------------------------------------------------------------
void TVME_VUPROM::SetMThresh(Int_t chan, Int_t val)
{
  // Write threshold value for multiplicity
  // chan = 0-3, val = 7 significant bits (0-127)
  // If val not provided use the pre-set fMThresh[i] values
  if( val != -1 ){
    fMThresh[chan] = val;
  }
  Int_t tval1 = (fMThresh[0] & 0x7f) | ((fMThresh[1] & 0x7f)<<16);
  Int_t tval2 = (fMThresh[2] & 0x7f) | ((fMThresh[3] & 0x7f)<<16);
  Write(EVU_MThresh1, tval1);
  Write(EVU_MThresh2, tval2);
  return;
}

//--------------------------------------------------------------------------
void TVME_VUPROM::CmdExe(Char_t* input)
{
  // Command interface
  // Use same key words as SetConfig
  //
  Int_t key;
  Char_t keyword[64];
  if( sscanf( input, "%s", keyword ) != 1 ){
    sprintf(fCommandReply,"<No command keyword supplied>\n");
    return;
  }
  if( (key = Map2Key( keyword, kVUPROMKeys )) == -1){
    sprintf(fCommandReply,"<Unrecognised VUPROM Command %s>\n",keyword);
    return;
  }
  if( !fIsGUIpermit ){
    sprintf(fCommandReply,"<Change to trigger condition not permitted>\n");
    return;
  }
  Char_t* parm = input + strlen(keyword) + 1;  // point to past the keyword
  Int_t i,j;
  switch(key){
  case EVUP_EnCPU:
    // Choose which slave CPUs included
    if(sscanf(parm,"%i",&i) != 1){
      sprintf(fCommandReply,"%s <Slave CPU enable input Format>\n", parm);
      break;
    }
    Write(EVU_CPUctrl,i);           // which CPUs enabled
    fEnableCPU = i;
    sprintf(fCommandReply,"Pattern 0x%x, written to CPU enable\n",i);
    break;
  case EVUP_IntDelay:
    // Interrupt delay
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Interrupt delay input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>=EVU_MaxCPU)){
      sprintf(fCommandReply,"Interrupt delay: channel %d outside valid range\n", i);
      break;
    }      
    SetIntDelay(i,j);
    fIntDelay[i] = j;
    sprintf(fCommandReply,"Interrupt delay %d ns to CPU %d\n",j*10,i);
    break;
  case EVUP_FCDelay:
    // Fast clear delay
    if(sscanf(parm,"%i",&i) != 1){
      sprintf(fCommandReply,"%s <Fast clear delay input Format>\n", parm);
      break;
    }
    Write(EVU_FCDelay,i);             // fast clear delay
    fFCDelay = i;
    sprintf(fCommandReply,"Fast clear delay %d ns\n",i*10);
    break;
  case EVUP_RAMDownload:
    // Load trigger conditions into RAM
    // disregard channel
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <RAM load input Format>\n", parm);
      break;
    }
    RAMDownload(1,j);
    RAMDownload(2,j);
    fRAMid = j;
    sprintf(fCommandReply,"Trigger condition %d to RAM L1 & L2\n",j);
    break;
  case EVUP_InputMask:
    // Mask for input signals 1 = ON
    if(sscanf(parm,"%i",&i) != 1){
      sprintf(fCommandReply,"%s <Input mask input Format>\n", parm);
      break;
    }
    Write(EVU_L1mask, i);           // enable/disable L1 inputs
    fInputMask = i;
    sprintf(fCommandReply,"Input mask set to 0x%x\n",i);
    break;
  case EVUP_InputPrescale:
    // Prescale values for input signals (4-bit)
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Input prescale input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>15)){
      sprintf(fCommandReply,"Input prescale: channel %d outside valid range\n", i);
      break;
    }      
    if( i < 8 )                 // 1st 8 channels
      SetPrescale(0, i, j);
    else
      SetPrescale(1, i-8, j);   // 2nd 8 channels
    sprintf(fCommandReply,"Input %d prescale set to 2^%d\n",i,(j&0xf));
    fInputPrescale[i] = j;
    break;
  case EVUP_L1Prescale:
    // Prescale values for L1 output signals (16-bit)
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <L1 prescale input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>7)){
      sprintf(fCommandReply,"L1 prescale: channel %d outside valid range\n", i);
      break;
    }
    j = (j & 0xffff);
    SetPrescale(2, i, j);
    fL1Prescale[i] = j;
    sprintf(fCommandReply,"L1 %d prescale factor set to %d\n",i,j);
    break;
  case EVUP_L2Prescale:
    // Prescale values for L2 output signals (16-bit)
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <L2 prescale input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>7)){
      sprintf(fCommandReply,"L2 prescale: channel %d outside valid range\n", i);
      break;
    }
    j = (j & 0xffff);
    SetPrescale(3, i, j);
    fL2Prescale[i] = j;
    sprintf(fCommandReply,"L2 %d prescale factor set to %d\n",i,j);
    break;
  case EVUP_EnPattRead:
    // Enable pattern register read
    // Do nothing
    sprintf(fCommandReply,"<Enable pattern read not permitted>");
    break;
  case EVUP_EnScalerRead:
    // Enable scaler read
    // Do nothing
    sprintf(fCommandReply,"<Enable scaler read not permitted>");
    break;
  case EVUP_SetDebug:
    // Connect internal signal to debug output
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Debug switching input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>3)){
      sprintf(fCommandReply,"Set debug output: channel %d outside valid range\n", i);
      break;
    }      
    SetDebugOut(i, j);
    sprintf(fCommandReply,"Signal %d switched to debug output %d\n",j,i);
    break;
  case EVUP_L1Width:
    // Gate output width
    if(sscanf(parm,"%i",&i) != 1){
      sprintf(fCommandReply,"%s <Gate width input format>\n", parm);
      break;
    }
    fL1Width = i;
    Write(EVU_L1Width,fL1Width);             // shape output 1 fL1Width units
    sprintf(fCommandReply,"L1 Gate width set to %d\n",i);
    break;
  case EVUP_SetDelay:
    // Strobe delays
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Strobe delay input format>\n", parm);
      break;
    }
    if( (i<1) || (i>2)){
      sprintf(fCommandReply,"Strobe delay: channel must be 1 or 2 not %d\n", i);
      break;
    }
    if( i==1 ){
      fL1Delay = j;
      Write(EVU_L1Delay,fL1Delay);             // Set L1 strobe delay
    }
    else if( i==2 ){
      fL2Delay = j;
      Write(EVU_L2Delay,fL2Delay);             // Set L2 strobe delay
    }      
    sprintf(fCommandReply,"L%d strobe delay set to %d\n",i,j);
    break;
  case EVUP_SetMThresh:
    // Multiplicity threshold levels
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Multiplicity threshold input format>\n", parm);
      break;
    }
    if( (i<0) || (i>3)){
      sprintf(fCommandReply,"Multiplicity threshold: channel %d outside valid range\n", i);
      break;
    }      
    SetMThresh(i, j);
    sprintf(fCommandReply,"Multiplicity channel %d: threshold set to %d\n",i,j);
    break;
  case EVUP_ResetTrigCtrl:
    ResetTrigCtrl();
    sprintf(fCommandReply, "ResetTrigCtrl executed.\n");
    break;
  default:
    sprintf(fCommandReply,"<Unrecognised VUPROM command>\n");
    break;
  }
}

//--------------------------------------------------------------------------
Int_t* TVME_VUPROM::GetParms(Int_t chan)
{
  // Provide trigger settings to outside world
  if( chan >= 16 ) return NULL;
  fTrigParm[0] = fEnableCPU;
  fTrigParm[1] = fIntDelay[chan];
  fTrigParm[2] = fFCDelay;
  fTrigParm[3] = fRAMid;
  fTrigParm[4] = fInputMask;
  fTrigParm[5] = fInputPrescale[chan];
  fTrigParm[6] = fL1Prescale[chan];
  fTrigParm[7] = fL2Prescale[chan];
  fTrigParm[8] = fL1Width;
  if( chan==1 )fTrigParm[9] = fL1Delay;
  else if(chan==2)fTrigParm[9] = fL2Delay;
  else fTrigParm[9] = 0;
  fTrigParm[10] = fMThresh[chan];
  return fTrigParm;
}

ClassImp(TVME_VUPROM)
