//--Author	JRM Annand   22nd Sep 2013 Adapt TVME_VUPROM for TAPS trigger
//--Rev 	...
//--Update 	JRM Annand  
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROMT
// GSI VUPROM FPGA based VMEbus module
// Has a multitude of functions, depending on the programing of the FPGA
// TAPS trigger condition master processor
// DAQ control master, of up to 14 slave VME systems
// Operate as stand-alone master or slave of overall experiment master
// 96 channel scaler
//

#include "TVME_VUPROMT.h"
#include "TDAQexperiment.h"


enum { EVUPT_CFD=300, EVUPT_LED1, EVUPT_LED2, EVUPT_Veto, EVUPT_PbWO4,
       EVUPT_PbWO4Veto, EVUPT_Pulser, EVUPT_TAPSMode, EVUPT_FrontEnd,
       EVUPT_BaF2F, EVUPT_VetoF, EVUPT_PbWO4F};

static Map_t kVUPROMTKeys[] = {
  {"Set-CFD:",         EVUPT_CFD},
  {"Set-LED1:",        EVUPT_LED1},
  {"Set-LED2:" ,       EVUPT_LED2},
  {"Set-Veto:" ,       EVUPT_Veto},
  {"Set-PbWO4:" ,      EVUPT_PbWO4},
  {"Set-PbWO4-Veto:" , EVUPT_PbWO4Veto},
  {"Set-Pulser:" ,     EVUPT_Pulser},
  {"TAPS-Mode:" ,      EVUPT_TAPSMode},
  {"TAPS-FrontEnd:" ,  EVUPT_FrontEnd},
  {"Set-Front-BaF2:",  EVUPT_BaF2F},
  {"Set-Front-Veto:" , EVUPT_VetoF},
  {"Set-Front-PbWO4:" ,EVUPT_PbWO4F},
  {NULL,                  -1}
};

VMEreg_t VUPROMTreg[] = {
  {0x2f00,      0x0,  'l', 0},       // Firmware revision
  {0x2300,      0x0,  'l', 0},       // Select TAPS mode (master/slave)
  {0x2310,      0x0,  'l', 0},       // TAPS status
  {0x2a00,      0x0,  'l', 0},       // Write event ID
  {0x2a10,      0x0,  'l', 0},       // Trigger event ID send
  {0x2a20,      0x0,  'l', 0},       // Send event ID status
  {0x2e00,      0x0,  'l', 0},       // Debug status
  {0x2e10,      0x0,  'l', 0},       // Debug channel 0
  {0x2e20,      0x0,  'l', 0},       // Debug channel 1
  {0x2e30,      0x0,  'l', 0},       // Debug channel 2
  {0x2e40,      0x0,  'l', 0},       // Debug channel 3
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
  {0x2400,      0x0,  'l', 0},       // CPU Control
  {0x2410,      0x0,  'l', 0},       // Trigger Control
  {0x2430,      0x0,  'l', 0},       // CPU reset
  {0x2450,      0x0,  'l', 0},       // End of readout acknowledge
  {0x2460,      0x0,  'l', 0},       // IRQ status
  //
  {0x2200,      0x0,  'l', 0},       // CFD sector mask
  {0x2210,      0x0,  'l', 0},       // LED1 sector mask
  {0x2220,      0x0,  'l', 0},       // LED2 sector mask
  {0x2230,      0x0,  'l', 0},       // Veto sector mask
  {0x2240,      0x0,  'l', 0},       // PbWO4 sector mask
  {0x2250,      0x0,  'l', 0},       // PbWO4 Veto sector mask
  {0x2100,      0x0,  'l', 0},       // CFD OR prescale factor
  {0x2110,      0x0,  'l', 0},       // LED1 OR prescale factor
  {0x2120,      0x0,  'l', 0},       // LED2 OR prescale factor
  {0x2130,      0x0,  'l', 0},       // Veto OR prescale factor
  {0x2140,      0x0,  'l', 0},       // CFD M2+ prescale factor
  {0x2150,      0x0,  'l', 0},       // LED1 M2+ prescale factor
  {0x2160,      0x0,  'l', 0},       // LED2 M2+ prescale factor
  {0x2170,      0x0,  'l', 0},       // Veto M2+ prescale factor
  {0x2180,      0x0,  'l', 0},       // Pulser prescale factor
  {0x2190,      0x0,  'l', 0},       // PbWO4 OR prescale factor
  {0x21a0,      0x0,  'l', 0},       // PbWO4 M2+ prescale factor
  {0x21b0,      0x0,  'l', 0},       // PbWO4 Veto OR prescale factor
  {0x21c0,      0x0,  'l', 0},       // PbWO4 Veto M2+ prescale factor
  {0x2800,      0x0,  'l', 0},       // Readout Bitpattern
  //
  {0x1800,      0x0,  'l', 0},       // Clear scalers
  {0x1804,      0x0,  'l', 0},       // Load scalers
  {0x1000,      0x0,  'l', 95},      // Scaler registers 0-95
  {0xffffffff,  0x0,  'l', 0},       // end of list
};

//-----------------------------------------------------------------------------
TVME_VUPROMT::TVME_VUPROMT( Char_t* name, Char_t* file, FILE* log,
			    Char_t* line ):
  TVME_VUPROM( name, file, log, line )
{
  // Basic initialisation 
  AddCmdList( kVUPROMTKeys );               // VUPROM-specific setup commands
  fNScalerChan = 96;                        // number scaler channels
  fNreg = EVU_Scaler + fNScalerChan + 2;    // control and scaler registers
  fTAPSMode = 0;                            // default TAPS master
  fIsFrontEnd = kFALSE;                     // by default master trigger
  fIsBaF = fIsVeto = fIsPbWO4 = kFALSE;     // default off
  for(Int_t i=0; i<= EM_PbWO4V; i++) fMask[i] = 0x0;     // zero mask values
  for(Int_t i=0; i<= EP_PbWO4VM2; i++) fPreSc[i] = 0x0;  // zero prescale values
  for(Int_t i=0; i<EMF_BaF2; i++) fMaskF[i] = 0x0;       // front end mask BaF2
  for(Int_t i=0; i<EMF_Veto; i++) fMaskFV[i] = 0x0;      // front end mask Veto
}

//-----------------------------------------------------------------------------
TVME_VUPROMT::~TVME_VUPROMT( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_VUPROMT::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  // unused Int_t i,j;
  switch( key ){
  case EVUPT_CFD:
    // Input CFD trigger parameters 
    if(sscanf(line,"%i%i%i",fMask+EM_CFD,fPreSc+EP_CFD,fPreSc+EP_CFDM2) != 3)
      PrintError(line,"<TAPS CFD trigger configure>",EErrFatal);
    break;
  case EVUPT_LED1:
    // Input LED1 trigger parameters 
    if(sscanf(line,"%i%i%i",fMask+EM_LED1,fPreSc+EP_LED1,fPreSc+EP_LED1M2) != 3)
      PrintError(line,"<TAPS LED1 trigger configure>",EErrFatal);
    break;
  case EVUPT_LED2:
    // Input LED2 trigger parameters 
    if(sscanf(line,"%i%i%i",fMask+EM_LED2,fPreSc+EP_LED2,fPreSc+EP_LED2M2) != 3)
      PrintError(line,"<TAPS LED2 trigger configure>",EErrFatal);
    break;
  case EVUPT_Veto:
    // Input Veto trigger parameters 
    if(sscanf(line,"%i%i%i",fMask+EM_Veto,fPreSc+EP_Veto,fPreSc+EP_VetoM2) != 3)
      PrintError(line,"<TAPS Veto trigger configure>",EErrFatal);
    break;
  case EVUPT_PbWO4:
    // Input PbWO4 trigger parameters 
    if(sscanf(line,"%i%i%i",
	      fMask+EM_PbWO4,fPreSc+EP_PbWO4,fPreSc+EP_PbWO4M2) != 3)
      PrintError(line,"<TAPS PbWO4 trigger configure>",EErrFatal);
    break;
  case EVUPT_PbWO4Veto:
    // Input PbWO4Veto trigger parameters 
    if(sscanf(line,"%i%i%i",
	      fMask+EM_PbWO4V,fPreSc+EP_PbWO4V,fPreSc+EP_PbWO4VM2) != 3)
      PrintError(line,"<TAPS PbWO4Veto trigger configure>",EErrFatal);
    break;
  case EVUPT_Pulser:
    // Input Pulser prescale factor
    if(sscanf(line,"%i",fPreSc+EP_Pulser) != 1)
      PrintError(line,"<TAPS pulser configure>",EErrFatal);
    break;
  case EVUPT_TAPSMode:
    // Input TAPS Mode
    if(sscanf(line,"%i",&fTAPSMode) != 1)
      PrintError(line,"<TAPS mode configure>",EErrFatal);
    break;
    // Following commands for a front end trigger module only
    // Must not be used for the master module as they will screw up the
    // interrupt delay registers
  case EVUPT_FrontEnd:
    // Set to front end trigger-module mode
    fIsFrontEnd = kTRUE;
    break;
  case EVUPT_BaF2F:
    // Input front-end CFD, LED1, LED2 trigger masks 
    if( !fIsFrontEnd || fIsVeto || fIsPbWO4 ){
      PrintError(line,"<Front-end CFD mask setting not permitted>");
      break;
    }
    fIsBaF = kTRUE;
    if(sscanf(line,"%x%x%x%x%x%x",
	      fMaskF,fMaskF+1,fMaskF+2,fMaskF+3,fMaskF+4,fMaskF+5) != 6)
      PrintError(line,"<TAPS CFD front-end trigger configure>",EErrFatal);
    break;
  case EVUPT_VetoF:
    // Input front-end Veto trigger masks 
    if( !fIsFrontEnd || fIsBaF  ){
      PrintError(line,"<Front-end Veto mask setting not permitted>");
      break;
    }
    fIsVeto = kTRUE;
    if(sscanf(line,"%x%x%x%x",fMaskFV,fMaskFV+1,fMaskFV+2,fMaskFV+3) != 4)
      PrintError(line,"<TAPS Veto front-end trigger configure>",EErrFatal);
    break;
  case EVUPT_PbWO4F:
    // Input front-end PbWO4 trigger masks 
    if( !fIsFrontEnd || fIsBaF ){
      PrintError(line,"<Front-end PbWO4 mask setting not permitted>");
      break;
    }
    fIsPbWO4 = kTRUE;
    if(sscanf(line,"%x",&fMaskFPbWO4) != 1)
      PrintError(line,"<TAPS PbWO4 front-end trigger configure>",EErrFatal);
    break;
  default:
    // default try commands of TVME_VUPROM...and so on
    TVME_VUPROM::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ) return;
  InitReg( VUPROMTreg );
  TVMEmodule::PostInit();
  // Front-end vuprom only
  // Write the mask registers
  if( fIsFrontEnd ){
    if( fIsBaF )
      for(Int_t i=0; i<EMF_BaF2; i++) WriteChk(EVUT_IntDelay0+i,fMaskF[i]);
    else if( fIsVeto )
      for(Int_t i=0; i<EMF_Veto; i++) WriteChk(EVUT_IntDelay0+i,fMaskFV[i]);
    if( fIsPbWO4 )
      WriteChk(EVUT_IntDelay0+4,fMaskFPbWO4);
    return;
  }
  WriteChk(EVUT_CPUctrl,fEnableCPU);           // which CPUs enabled
  // Interrupt delays cpu's 0-14
  for( Int_t i=0; i<EVUT_MaxCPU; i++ )
    SetIntDelay(i, fIntDelay[i]);
  // Switch internal signals to debug outputs, if this is enabled
  if( fDebugOut[0] >= 0 ) SetDebugOut(0, fDebugOut[0]);
  if( fDebugOut[1] >= 0 ) SetDebugOut(1, fDebugOut[1]);
  if( fDebugOut[2] >= 0 ) SetDebugOut(2, fDebugOut[2]);
  if( fDebugOut[3] >= 0 ) SetDebugOut(3, fDebugOut[3]);
  // Write mask values
  for(Int_t i=0; i<= EM_PbWO4V; i++){
    Int_t ir = EVUT_CFDMask + i;        // register ID
    WriteChk(ir,fMask[i]);
  }
  // Write prescale values
  for(Int_t i=0; i<= EP_PbWO4VM2; i++){
    Int_t ir = EVUT_CFDPreSc + i;        // register ID
    WriteChk(ir,fPreSc[i]);
  }
  Write(EVUT_TAPSMode,fTAPSMode);    // stand alone or coupled
  EndTrigCtrl();                     // ensure triggers disabled
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::RunTrigCtrl()
{
  // Start trigger control system and enable triggers
  // mask all channels except 0 (0-15) and execute F25 test
  // beginning of spill
  fEXP->PostReset();              // any additional resetting after start
  //Write(EVUT_ScalerClr,1);      // clear the scalers
  EnableTrigCtrl();               // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::EndTrigCtrl()
{
  // Stop trigger control system and mark "end of spill"
  // and set overall inhibit
  DisableTrigCtrl();
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::ResetTrigCtrl()
{
  // Disable triggers and reset the trigger control system.
  // Issue a stop followed by start pulse
  //
  DisableTrigCtrl();
  fEXP->PostReset();              // any additional resetting after start
  EnableTrigCtrl();               // Enable DAQ triggers
  return;
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::EnableTrigCtrl()
{
  // Issue master reset and enable triggers
  //
  Write( EVUT_CPUreset, 1 );
  //Write( EVUT_MasterReset, 1 );
  Write( EVUT_TrigCtrl, (UInt_t)0x0 );
}

//-------------------------------------------------------------------------
void TVME_VUPROMT::DisableTrigCtrl()
{
  // Disable triggers
  //
  Write( EVUT_TrigCtrl, 0x1 );
}

//-------------------------------------------------------------------------
Bool_t TVME_VUPROMT::CheckHardID( )
{
  // Read firmware version from register
  // Fatal error if it does not match the hardware ID
  Int_t id = Read(EVUT_Firmware);
  fprintf(fLogStream,"VUPROMT firmware version Read: %x  Expected: %x\n",
	  id,fHardID);
  if( id == fHardID ) return kTRUE;
  else
    PrintError("","<VUPROMT firmware ID error>",EErrFatal);
  return kFALSE;
}
//--------------------------------------------------------------------------
void TVME_VUPROMT::SetDebugOut(Int_t i, Int_t j)
{
  // Switch internal signal #j to debug output #i
  //
  Int_t k;
  printf("Switching signal %d to debug output: %d\n",i,j);
  // Select which debug register to set
  k = EVUT_SetDebug0 + i;
  WriteChk(k,j);
  // read back status of selected debug port and print out
  printf("Read back status: %x after debug switch: %d\n",Read(EVUT_ReadDebug),k);
  return;
}
//


//--------------------------------------------------------------------------
void TVME_VUPROMT::SetIntDelay(Int_t cpu, Int_t delay)
{
  // write interrupt delay value (ns*10) for particular CPU
  //
  if( cpu >= EVUT_MaxCPU ){
    printf(" VUPROMT Invalid CPU number: %d\n",cpu);
    return;
  }
  Int_t creg = EVUT_IntDelay0 + cpu;
  WriteChk(creg, delay);
  return;
}
/*
//--------------------------------------------------------------------------
void TVME_VUPROMT::CmdExe(Char_t* input)
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
    Write(EVUT_CPUctrl,i);           // which CPUs enabled
    fEnableCPU = i;
    sprintf(fCommandReply,"Pattern 0x%x, written to CPU enable\n",i);
    break;
  case EVUP_IntDelay:
    // Interrupt delay
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <Interrupt delay input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>=EVUT_MaxCPU)){
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
    Write(EVUT_FCDelay,i);             // fast clear delay
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
    Write(EVUT_L1mask, i);           // enable/disable L1 inputs
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
    // Prescale values for L1 output signals (4-bit)
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <L1 prescale input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>7)){
      sprintf(fCommandReply,"L1 prescale: channel %d outside valid range\n", i);
      break;
    }      
    SetPrescale(2, i, j);
    fL1Prescale[i] = j;
    sprintf(fCommandReply,"L1 %d prescale factor set to %d\n",i,(j+1));
    break;
  case EVUP_L2Prescale:
    // Prescale values for L2 output signals (4-bit)
    if(sscanf(parm,"%i%i",&i,&j) != 2){
      sprintf(fCommandReply,"%s <L2 prescale input Format>\n", parm);
      break;
    }
    if( (i<0) || (i>7)){
      sprintf(fCommandReply,"L2 prescale: channel %d outside valid range\n", i);
      break;
    }      
    SetPrescale(3, i, j);
    fL2Prescale[i] = j;
    sprintf(fCommandReply,"L2 %d prescale factor set to %d\n",i,(j+1));
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
    Write(EVUT_L1Width,fL1Width);             // shape output 1 fL1Width units
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
      Write(EVUT_L1Delay,fL1Delay);             // Set L1 strobe delay
    }
    else if( i==2 ){
      fL2Delay = j;
      Write(EVUT_L2Delay,fL2Delay);             // Set L2 strobe delay
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
  default:
    sprintf(fCommandReply,"<Unrecognised VUPROM command>\n");
    break;
  }
}

//--------------------------------------------------------------------------
Int_t* TVME_VUPROMT::GetParms(Int_t chan)
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
*/
//-----------------------------------------------------------------------------
void TVME_VUPROMT::ReadIRQScaler( void** outBuffer )
{
  // Read the scaler registers from the VUPROM
  //
  Int_t i,j;
  UInt_t datum;
  Write(EVUT_ScalerLoad, 1);          // load scalers into buffer
  for(i=0; i<fNScalerChan; i++){      // read 96 scaler channels
    j = i + EVUT_Scaler;
    datum = Read(j);
    ScalerStore( outBuffer, datum, fBaseIndex+i );
  }
  Write(EVUT_ScalerClr,1);           // clear the scalers
}

//---------------------------------------------------------------------------
void TVME_VUPROMT::ReadIRQ( void** outBuffer )
{
  // Read and store the trigger pattern register 32 bits and multiplicity input 2 x 31 bits
  // 16 lsb L1 pattern
  // 16 msb L2 pattern
  // 
  UInt_t datum, datumlow, datumhigh;
  datum = Read(EVUT_ReadoutBitpattern);         // HitPattern/ReadoutPattern
  Int_t j = fBaseIndex; // ADC numbering starts at the fBaseIndex
  datumlow = datum & 0xffff;
  datumhigh = datum >> 16;
  ADCStore( outBuffer, datumlow, j );
  j++; 
  ADCStore( outBuffer, datumhigh, j );
}

ClassImp(TVME_VUPROMT)
