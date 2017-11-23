//--Author	JRM Annand   2nd Sep 2009  Adapt from acqu 3v7
//--Rev         B. Oussena   1st Dec 2009  Start Adapt from acqu 3v7
//--Rev         B. Oussena   25th Dec 2009 Add cmd MapReg, mod PostInit()
//--Rev         B. Oussena        Jan 2010 Add AddSMI()
//--Rev         B. Oussena        Feb 2010 Add Readout ADCs and Scalers
//--Rev         JRM Annand  27th Jul 2010  Clean up a bit
//--Rev         B. Oussena   6th Aug 2010  add  fNADC and fNScaler in ReadIRQ
//--Rev         JRM Annand   4th Sep 2010  ReadIRQScaler....the name
//--Rev         JRM Annand   9th Sep 2010  Slow read/write via usleep(1)
//--Rev         JRM Annand  14th Sep 2010  Scaler read..pipe suppress disabled
//--Update      JRM Annand  30th Mar 2014  Get rid of usleep in read/write
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_KPhSMI
// VMEbus interface to 8 control registers of LRS1821 SMI
// Interface can handle up to 16 SMIs
//

#include "TVME_KPhSMI.h"
#include "TDAQexperiment.h"
#include "TFB_1821SMI.h"




enum { EKPhSMI_MapReg=200 };               
static Map_t kKPhSMIKeys[] = {            
  {"MapReg:",      EKPhSMI_MapReg},   
  {NULL,           -1}
};

//-----------------------------------------------------------------------------
TVME_KPhSMI::TVME_KPhSMI( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = new TDAQcontrol( this );              // tack on control functions
  fType = EDAQ_SCtrl | EDAQ_ADC | EDAQ_Scaler;  // secondary ctrl, ADC & Scaler
  AddCmdList( kKPhSMIKeys );                    // KPhSMI-specific setup command
  fSmiList = new TList;                         // 
  fSMIreg = NULL;                               // SMI reg.offset not defined
  fNsmi = 0;                                    // no SMIs yet
  fSMIbuff = new UInt_t[ESMIbufflen];           // raw readout buffer
}

//-----------------------------------------------------------------------------
TVME_KPhSMI::~TVME_KPhSMI( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_KPhSMI::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file  
  switch( key ){
  case EKPhSMI_MapReg: 
    TVMEmodule::PostInit();
    break;
  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_KPhSMI::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not do the default here
  if( fIsInit ){ 

    return;  
  }  
  //TVMEmodule::PostInit(); //<< ----  has already been performed
  return;
}

//-----------------------------------------------------------------------
void TVME_KPhSMI::AddSMI(TDAQmodule *mod)
{
  // Add 1821 SMI to list
  fSmiList->AddLast(mod);    // Add SMI1821 to the SMI list
  fNsmi++;                   // increment no of SMIs
  return;
}

//-----------------------------------------------------------------------
void TVME_KPhSMI::ReadIRQ(void  **P_buff) /*-> P_buff data buffer memory */
{
  //	Routine to transfer data from FASTBUS crates to the swinging
  //	buffer. 	
  //        Front panel SIB bus readout. For each addressed module (not
  //	multiblock simple or end link) read data from module to SMI
  //	mem to primary buffer in CPU RAM. Decode into swinging buffer
  //	as ECLbus.
  //	3v4 mod...determine readout Function on the basis of
  //	SMI RAM number
  //	0	T-pin scan
  //	1-4	Fast (same SMI code as rear panel)
  //	5	Slow (as used in 93 experiments)

  Int_t nadc;	// no. ADCs
  Int_t i,j;
  Int_t n_word;			/* no. words read during IRQ */
  UInt_t *irq_buff = fSMIbuff;
  TFBmodule *p_fb;	// -> fastbus module 
  TFB_1821SMI *fb_smi;	// -> fastbus SMI
  TIter smi(fSmiList);


  for(i=0; i<fNsmi; i++){
    fb_smi=(( TFB_1821SMI *) smi.Next()); // Get the pointer to Object 1821SMI

    nadc = fb_smi->fNADC;  // num of total FB ADC modules -> last module
    if (nadc == 0) return; 

    S_KPH(fb_smi->fSMI);    	// select SMI
    TIter smi_adc(fb_smi->fADCList);  

    switch(fb_smi->fRAM){  
    case 0:
      // Standard LeCroy Tpin-test readout
      fb_smi->R_Block_Tpin(0x9,irq_buff,&n_word,fb_smi->fPipe);
      for(j=0; j<nadc; j++){
	p_fb = (( TFBmodule *) smi_adc.Next()); 
        p_fb->Decode(&irq_buff,P_buff,&n_word);
	if(n_word <= 0)
	  break;
      }
      break;
    case 1: case 2: case 3: case 4:
      // Use specialist sequencer readout...reads entire crate
      // then decode each modules contribution
      fb_smi->R_Block_f(irq_buff,&n_word,ESMIADCRd,fb_smi->fPipe);
      for(j=0; j<nadc; j++){
	p_fb = (( TFBmodule *) smi_adc.Next());     
        p_fb->Decode(&irq_buff,P_buff,&n_word);
	if(n_word <= 0)
	  break;			/* exit loop if no data */
      }
      break;
    case 5:
      // Use standard LeCroy code...reads and then decodes individual modules

      for(j=0; j<nadc; j++){
	p_fb = (( TFBmodule *) smi_adc.Next());
	if((p_fb->GetReadMode() == 'm') || (p_fb->GetReadMode() == 'b'))
	  fb_smi->R_Block(p_fb->GetD_Read(),irq_buff,&n_word,fb_smi->fPipe);
	p_fb->Decode(&irq_buff,P_buff,&n_word);
      }
      break;
    default:
      PrintError("ReadIRQ", "Unknown SMI readout mode", EErrFatal );
      break;
    }
  }
  return;
}

//----------------------------------------------------------------------------
void TVME_KPhSMI::ReadIRQScaler(void  **P_buff)
{
  // Routine to transfer data from FASTBUS Scalers to primary  
  // and then swinging buffer
  // Front panel SIB bus readout. For each addressed module (not
  // multiblock simple or end link) read data from module to SMI
  // mem to primary buffer in CPU RAM. Decode into swinging buffer
  // as ECLbus.
  // 3v4 mod...determine readout Function on the basis of
  // SMI RAM number
  // 0	T-pin scan
  // 1-4	Fast (same SMI code as rear panel)
  // 5	Slow (as used in 93 experiments)

  Int_t nScaler;	// no. scalers
  Int_t i,j;	        // general index 
  Int_t n_word;	        // no. words read during IRQ 
  UInt_t *irq_buff = fSMIbuff;
  TFBmodule *p_fb;	// -> fastbus module <<-------------------- */
  TFB_1821SMI *fb_smi;	// -> fastbus smi    << ------------------- */
  TIter smi(fSmiList);

  for(i=0; i<fNsmi; i++){
    fb_smi=(( TFB_1821SMI *) smi.Next()); // Get the pointer to Object 1821SMI
   
    nScaler = fb_smi->fNScaler;// num of total FB Scaler modules -> last module
    if (nScaler == 0) return; 

    S_KPH(fb_smi->fSMI);    	/* select SMI */
    TIter smi_scaler(fb_smi->fScalerList);

    switch(fb_smi->fRAM){   
    case 1: case 2: case 3: case 4:
      // Fast readout with special SMI code
      fb_smi->R_Block_f(irq_buff,&n_word,ESMIScalerRd,ESMIPipeDisable);
      for(j=0; j<nScaler; j++){
	p_fb = (( TFBmodule *) smi_scaler.Next());     
	p_fb->DecodeScaler(&irq_buff,P_buff,&n_word);
	if(n_word <= 0)
	  break;			/* exit loop if no data */
      }
      break;
    case 0: case 5:
      // Standard readout with standard LeCroy sequencer code
      for(j=0; j<nScaler; j++){
	p_fb = (( TFBmodule *) smi_scaler.Next());
	fb_smi->R_Block(p_fb->GetD_Read(),irq_buff,&n_word,ESMIPipeDisable);
	p_fb->DecodeScaler(&irq_buff,P_buff,&n_word);
      }
      break;
    default:
      PrintError("ReadIRQS", "Unknown SMI readout mode", EErrFatal );
      break;								
    }
  }
  return;
}
ClassImp(TVME_KPhSMI)
