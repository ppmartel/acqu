//--Author	JRM Annand    2nd Sep 2009  Convert from acqu 3v7
//--Rev         B. Oussena   15th Jan 2010  Start updates
//--Rev         B. Oussena   15th Jan 2010  Add Cmd E1821SMI_INIT & TFB_1821SMI::Init_SMI1821()
//--Rev         B. Oussena   30th Jan 2010  fix seg fault: set access to virt. addresses
//--Rev         B. Oussena    1st Feb 2010  Add  TFB_1821SMI::Add_FbADC()
//--Rev         B. Oussena   24th Feb 2010  fix bug : set status=0x20
//--Rev         B. Oussena   28th Feb 2010  Add  TFB_1821SMI::Add_FbSCA()
//--Rev 	JRM Annand   27th Jul 2010  Some cleaning up
//--Rev 	B. Oussena   2nd  Aug 2010   fixed a bug in the constructor.
//--Rev 	B. Oussena   3rd  Aug 2010  fixed a bug in TFB_1821SMI::HM_1821: set W=0.
//--Rev 	JRM Annand    9th Sep 2010  ESMIStatusTry reset to 1000
//--Rev 	JRM Annand   14th Sep 2010  PedAddr() n<<8
//--Rev  	JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fixes
//--Rev 	B. Oussena    7th Nov 2011  have re-organised the parameters lines from the methods. Have seemed before not to be working
//--Rev 	B. Oussena    7th Nov 2011  added one printf in method  Exec1821
//--Update	B. Oussena    7th Nov 2011  Have reduced the waiting loop for sequence to finish in  Exec1821
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TFB_1821SMI
// LeCroy 1821 FASTBUS Segment Interface Manager (SMI)
// FASTBUS crate controller
//
#include "TFB_1821SMI.h"
#include "ARFile_t.h"


enum { E1821SMI_RAM=100, E1821SMIRdReg,E1821SMI_INIT };
static Map_t k1821SMIKeys[] = {
  {"INIT:",  E1821SMI_INIT},
  {NULL,           -1}
};



//-----------------------------------------------------------------------------
TFB_1821SMI::TFB_1821SMI( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log)
{
  // Basic initialisation
  fCtrl = new TDAQcontrol( this );           // tack on control functions
  fType = EDAQ_SCtrl;                      // secondary controller
  AddCmdList( k1821SMIKeys );                  // 1821SMI-specific cmds
  fBus = E_FASTBUS;          
  fADCList = new TList;  
  fScalerList = new TList;
}

//-----------------------------------------------------------------------------
TFB_1821SMI::~TFB_1821SMI( )
{
  // Disconnect
}

//-----------------------------------------------------------------------------
void TFB_1821SMI::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  switch( key ){
  case E1821SMI_INIT:
    if( fCtrlMod->InheritsFrom("TVME_KPhSMI"))
      fFBCtrlMod = (TVME_KPhSMI*)fCtrlMod;
    else
      PrintError(line,"<SMI has invalid controller/interface>",EErrFatal);
    fFBCtrlMod->AddSMI(this);
    Init_SMI1821(line);
    break;
  default:
    // try general module setup commands
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TFB_1821SMI::PostInit()
{
  Int_t err;
  // Initialise PCI-VME hardware
  if( fIsInit ) return;
  err = HS_1821(fRAM);   // Load readout RAM
  if(err != 0){
    PrintError("","<Fatal error init SMI in PostInit TFB_1821SMI>",EErrFatal);
  }
  TDAQmodule::PostInit();
}

//-----------------------------------------------------------------------------
void TFB_1821SMI::BlkRead()
{
}

//-----------------------------------------------------------------------------
void TFB_1821SMI::BlkWrite()
{
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::Exec1821( UShort_t addr )
{
  //	Host routine to call 1821 sequencer routine via 2891A Camac interface.
  //	Parameter addr....jumptable address from which routine is called
  //	function returns....	0 completed OK
  //				1 SMI macro not completed
  //				-n, n = no of errors on fCtrlMod->W_KPH, fCtrlMod->R_KPH
  //	NB changed    status & 0x003f   to    status & 0x001f
  //
  UShort_t status;	// returned SMI status 
  Int_t i;              // SMI exe time counter 
 
  static UShort_t j=0;
 
  fFBCtrlMod->R_KPH(0,&status);		// get reg 0 status 
  status = (status & 0x001f) | 0x9900;	// PADDR,PDATA src/dest 
  fFBCtrlMod->W_KPH(0,status | 0x0080);	// prog load bit 
  fFBCtrlMod->W_KPH(1,0);		// load prog 
  fFBCtrlMod->W_KPH(0,status);		// prog exec config 
  fFBCtrlMod->W_KPH(1,addr*8);		// load start address 
  fFBCtrlMod->W_KPH(7,0x0001);		// ignite sequencer 
  //	check for execution, if not finished after ESMIStatusTry loops
  //	signal error

 
  i = 0;					// init exec time counter 
  status = 0x20; //<-----------Baya
  do{
    fFBCtrlMod->R_KPH(7,&status);	// get status reg 7 
    if((status & 0x20) == 0)	// check if SMI still active
    {
      // fprintf( fLogStream,"SMI TIME OK on routine @addr  %x  after looping  %d times, j  %d  \n",addr,i,j++);
       break;
    }			// exit loop if finished 

  }while(i++ <= 3); //ESMIStatusTry);   ------>> BAYA         
              
  if(i > 3){   //ESMIStatusTry) {   --------->> BAYA                            
    fprintf( fLogStream,"SMI TIME OUT on routine @addr  %x  after looping  %d times, j  %d  \n",addr,i,j++);
  return 1;
  }
  return 0;
  
}
//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::ExecRtime( UShort_t pipe )

			    
{
  //	Special SMI sequencer exec to boot it into the real time loop.
  // the parameters is : 
			   // 1- configure smi pipeline 
  //	i.e. Start SMI macro at address REAL_TM
  //
  UShort_t status;		       // SMI status 
  Int_t i;	  // SMI exe time counter  // baya has added for control status 
  
  fFBCtrlMod->W_KPH(3,pipe);	       // data smi pipeline ->personality card 	
  fFBCtrlMod->R_KPH(0,&status);	       // get reg 0 status 
  status = (status & 0x001f) | 0x9900; // PADDR,PDATA src/dest 
  fFBCtrlMod->W_KPH(0,status | 0x0080);// prog load bit 
  fFBCtrlMod->W_KPH(1,0);	       // load prog 
  fFBCtrlMod->W_KPH(0,status);	       // prog exec config 
  fFBCtrlMod->W_KPH(1,ESMIRealTm*8);   // load start addr real tm 
  fFBCtrlMod->W_KPH(7,0x0001);	       // ignite sequencer 

  /*   Baya has replace this with the followed inst ...	
       X += fFBCtrlMod->R_KPH(7,&status);	       // read SMI status 
       if((status & 0x20) == 0){            // check if active 
       fprintf( fLogStream,"SMI not in active state\n"); // not active...fail 
       return(1);			       // bad return 
       }
  */
  status = 0x20; // Baya
  do{
    fFBCtrlMod->R_KPH(7,&status);	// get status reg 7 
    if((status & 0x20) == 0)	        // check if SMI still active
      break;			        // exit loop if finished 
  }while(i++ <= ESMIStatusTry);
  if(i > ESMIStatusTry){
    fprintf( fLogStream,"SMI not in active state\n"); // not active...failure 
    return(1);
  }
  return 0;				// return 0 if all OK 
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::StopRtime()
{
  //	Special sequencer exec to retrieve it from the real time loop.
  //	Method: set the host attention bit in reg 0, which is tested
  //	every iteration of the loop
  Int_t i = 0;		        // loop counter 
  UShort_t status;		// return status  

  fFBCtrlMod->R_KPH(0,&status);	// get R0 contents 
  status |= 0x40;		// set the host attention bit 
  fFBCtrlMod->W_KPH(0,status);	// and load it into R0
  //	check if sequencer active
  status = 0x20;                //<-   Baya
  fFBCtrlMod->R_KPH(7,&status);	// get R7 conts 
  while(status & 0x20){		// sequencer active? until timeout 
    if(i++ == ESMIStatusTry){
      fprintf( fLogStream,"SMI time out on return from loop\n");
      return(1);	          // error return 
    }
    fFBCtrlMod->R_KPH(7,&status); // get R7 again 
  }
  return 0;			  // return 0 if no errors 
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::FB_read(UInt_t *P_data)
{
  //	Fastbus read from attached single slave.
  //	Parameter P_data....pointer to data read
  UShort_t data,X; 
 
  fFBCtrlMod->W_KPH(3,0x840e);	      // No ped subtr or mem wrt, pedsub source 
  fFBCtrlMod->W_KPH(6,0x0000);	      // no auto mem advance 
  X = Exec1821(0x0010);	              // exec 1821 read macro 
  fFBCtrlMod->R_KPH(4,&data);	      // get least sig 16 bits 
  *P_data = data;
  fFBCtrlMod->R_KPH(5,&data);	      // get most sig. 16 bits 
  *P_data |= (data<<16);	      // or in msb's 
  return X;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::FB_wrt( UInt_t data )
{
  //	Fastbus write to attached single slave.
  //	Parameter data......value written
  UShort_t sh_data,X;
  
  fFBCtrlMod->W_KPH(0,0x9900);	
  sh_data = data;		      	// load 32 bit number into 32 bit reg 
  fFBCtrlMod->W_KPH(2,sh_data);		// this is done in to 16 bit chunks 
  X = Exec1821(0x0030);
  sh_data = (data>>16);		        // the 2nd chunk 
  fFBCtrlMod->W_KPH(2,sh_data);
  X += Exec1821(0x0031);
  X += Exec1821(0x0016);		// exec write cycle 
  return X ;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::IO_1821( UShort_t slot, UInt_t sub_addr, UInt_t *P_data, char RW, char space )
{
  //	General purpose addressing and read/write routine for Fastbus using
  //	the LeCroy 1821 ROM macros. Parameters as follows:
  //	slot......Fastbus slot (Geographical address)
  //	sub_addr..module secondary address
  //	P_data....pointer to read/write data
  //	RW........character switch for read/write (= r or w)
  //	space.....character wsitch for addressing csr/dsr registers (= c or d)
  //	
  //	return value = no. of 2891A camac interface operations which did not
  //				   give Q (increment 1) or X (increment 2)
  //	ie return = 0 for Q always
  //
  UShort_t addr_sp,X,sh_data;
  
  if(space == 'c')		  // set addressing to csr or data registers 
    addr_sp = 0x000c;
  else if(space == 'd')
    addr_sp = 0x0008;
  else{
    fprintf( fLogStream, " IO_1821 unknown csr/dsr variable: %c \n",space);
    return -1;
  }
  //  Primary address cycle load slot(=geog addr) into 1821 input reg 2
  //  NB this assumes that the 1821 SMI has already grabed bus mastership 
  fFBCtrlMod->W_KPH(2,slot);	// load slot addr 
  X = Exec1821(addr_sp);	// primary addressing to csr or data space 
  
  //	secondary address cycle, load sub_addr into 1821 32bit reg. 
  sh_data = sub_addr;			// load 32 bit number into 32 bit reg 
  fFBCtrlMod->W_KPH(0,0x9900);
  fFBCtrlMod->W_KPH(2,sh_data);		// this is done in to 16 bit chunks 
  X += Exec1821(0x0030);
  sh_data = (sub_addr>>16);		// the 2nd chunk 
  fFBCtrlMod->W_KPH(2,sh_data);
  X += Exec1821(0x0031);
  X += Exec1821(0x0019);		// secondary address cycle 
  if(RW == 'r'){
    X += FB_read(P_data);
    return X;
  }
  else if(RW == 'w'){
    X += FB_wrt(*P_data);
    return X;
  }
  else{
    fprintf( fLogStream,
	     " IO_1821 unknown read/write variable: %c \n",RW);
    return -2;
  }
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_1821( UShort_t slot, UInt_t sub_addr, UInt_t *P_data, char space)
{
  //	Read from Fastbus slave...uses IO_1821()
  if(IO_1821(slot,sub_addr,P_data,'r',space) == 0)
    return 0;
  else
    return -1 ;
}
	
//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::W_1821(UShort_t slot, UInt_t sub_addr, UInt_t data, char space)
{
  //	Write to Fastbus slave...uses IO_1821()
  UInt_t temp_data;
  temp_data = data;
  if(IO_1821(slot,sub_addr,&temp_data,'w',space) == 0)
    return 0;
  else
    return -1;
}
	
//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_Block_L( UShort_t slot,UInt_t sub_addr,  UInt_t *data,  Int_t *n_word)	
{
  // General Purpose Fastbus block readout routine
  // The parameters are : 
                                // 1- Fastbus station 
			     	// 2- Internal data to read 
			   	// 3- -> to data buffer 
			   	// 4- no. of words read out 
			  
  // This one works with sequencer code smi 0
  // Slow Standard LeCroy SMI code for diagnostics
  //
  Int_t X;	        // error counter 
  Int_t n_a;		// no. of chans read..loop counter  <------- was short
  UShort_t status;	// status bits read from SMI 
  UShort_t lsw,msw;	// least and most sig. 16bits of 32bit word in data 
  UShort_t addr = 0x8;	// to pass addresses to SMI 
  
  fFBCtrlMod->W_KPH(2,slot);		// load primary address 
  X = Exec1821(addr);		// in DATA space (0x8) 
  addr = sub_addr;		// for secondary address 
  fFBCtrlMod->W_KPH(0,0x9900);
  fFBCtrlMod->W_KPH(2,addr);
  X += Exec1821(0x30);		// load 16 lsb of 32b reg. 
  addr = (sub_addr >> 16);
  fFBCtrlMod->W_KPH(2,addr);
  X += Exec1821(0x31);		// 16 msb = 0 
  X += Exec1821(0x19);		// secondary address write 
  
  fFBCtrlMod->W_KPH(6,0x3000);	// set up SMI data memory 
  fFBCtrlMod->W_KPH(7,0x10);	// strobe SMI DMAR load 
  fFBCtrlMod->W_KPH(3,0x840c);	// no pipeline compression 
  X += Exec1821(0x2c);		// run SMI block read routine 
  
  fFBCtrlMod->R_KPH(6,&status);	// get no.words read into SMI mem
  *n_word = n_a = status & 0x0fff;
  
  fFBCtrlMod->W_KPH(3,0x8002);
  fFBCtrlMod->W_KPH(7,0x10);
  while(n_a-- > 0){
    fFBCtrlMod->R_KPH(5,&msw);	// most significant word 
    fFBCtrlMod->R_KPH(4,&lsw);	// least significant word 
    *data++ = lsw | (msw<<16);	// fill data buffer
  }
  return X;	
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_Block_Tpin( UInt_t broadcast, UInt_t *data, Int_t *n_word, UShort_t pipe)	
{
  // General Purpose Fastbus block readout routine
  // The parameters are :
                              // 1- broadcast code 
			      // 2- -> to data buffer 
			      // 3- no. of words read out 
			      // 4- for SMI data compression 
  // This one works with sequencer code smi 0
  // T-Pin scan for modules to read
  //
  Int_t X;	        // error counter 
  Int_t n_a;		// no. of chans read..loop counter 
  UShort_t status;	// status bits read from SMI 
  UShort_t lsw,msw;     // least and most sig. 16bits of 32bit word in data 
  
  fFBCtrlMod->W_KPH(6,0x3000);		// set up SMI data memory 
  fFBCtrlMod->W_KPH(7,0x10);		// strobe SMI DMAR load 
  fFBCtrlMod->W_KPH(3,pipe);		// for pipeline compression 
  fFBCtrlMod->W_KPH(2,(UShort_t)broadcast);
  X = Exec1821(0x33);		        // run SMI T-pin scan 
  
  fFBCtrlMod->R_KPH(6,&status);	        // get no.words read into SMI mem
  *n_word = n_a = status & 0x0fff;
  
  fFBCtrlMod->W_KPH(3,0x8002);
  fFBCtrlMod->W_KPH(7,0x10);
  while(n_a-- > 0){
    fFBCtrlMod->R_KPH(5,&msw);		// most significant word 
    fFBCtrlMod->R_KPH(4,&lsw);		// least significant word 
    *data++ = lsw | (msw<<16);	        // fill data buffer
  }
  return X;	
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_Block(UShort_t addr, UInt_t *data, Int_t *n_word, UShort_t pipe )

{
  //	General Purpose Fastbus block readout routine
  // The parameters are :
			   	 // 1- Fastbus addr (station | read register) 
			  	 // 2- -> to data buffer 
			         // 3- no. of words read out 
			   	 // 4- set up smi pipeline 
			 
  //	NB 8bit slot & 8bit addr only allowed (0-255)
  //	This one works with sequencer code smi_5 - 7
  //	Slow transfer.

  Int_t X;	// error counter 
  Int_t n_a;		// no. of chans read..loop counter 
  UShort_t status;	// status bits read from SMI 
  UShort_t lsw,msw;	// least and most sig. 16bits of 32bit word in data 
  //  UInt_t *xdata;	// for debugging purposes 
  


  //  xdata = data;
	
  fFBCtrlMod->W_KPH(6,0x3000);	// set up SMI data memory 
  fFBCtrlMod->W_KPH(7,0x10);	// strobe SMI DMAR load 
  fFBCtrlMod->W_KPH(3,pipe);	// set up SMI pipeline -> data memory wrt 
  fFBCtrlMod->W_KPH(2,addr);
  X = Exec1821(ESMIBlkRd);	// run SMI block read routine 
  
  fFBCtrlMod->R_KPH(6,&status);	// get number of words read into SMI memory 
  *n_word = n_a = status & 0x0fff;
  
  fFBCtrlMod->W_KPH(3,0x8002);
  fFBCtrlMod->W_KPH(7,0x10);
  while(n_a-- > 0){
    fFBCtrlMod->R_KPH(5,&msw);	// most significant word 
    fFBCtrlMod->R_KPH(4,&lsw);	// least significant word 
    *data++ = lsw | (msw<<16);	// fill data buffer
  }


  return X;	
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_Block_f(UInt_t *data, Int_t *n_word, UShort_t exec_st,UShort_t pipe)

			    
{
  // General Purpose Fastbus block readout routine.
  // To be used with SMI list mode addressing
  // The parameters are :
			     // 1- -> to data buffer 
			     // 2- no. of words read out 
			     // 3- start addr SMI adc or scaler 
			     // 4- set up smi pipeline 
  // Non-standard SMI sequencer code....smi_1 - 4
  Int_t X=0;	// error counter 
  Int_t n_a;		// no. of chans read..loop counter 
  UShort_t status;	// status bits read from SMI 
  UShort_t lsw,msw;	// least and most sig. 16bits of 32bit word in data 
  
  fFBCtrlMod->W_KPH(6,0x3000);	// set up SMI data memory 
  fFBCtrlMod->W_KPH(7,0x10);	// strobe SMI DMAR load 
  fFBCtrlMod->W_KPH(3,pipe);	// set up SMI pipe -> data mem wrt 
  X = Exec1821(exec_st);	// run SMI block read routine 
  
  fFBCtrlMod->R_KPH(6,&status);	// get no.words read into SMI mem 
  *n_word = n_a = status & 0x0fff;// strip extraneous bits 
  
  fFBCtrlMod->W_KPH(3,0x8002);	// set up data transfer from SMI 
  fFBCtrlMod->W_KPH(7,0x10);
  while(n_a-- > 0){		// data transfer loop 
    fFBCtrlMod->R_KPH(5,&msw);	// most significant word 
    fFBCtrlMod->R_KPH(4,&lsw);	// least significant word 
    *data = lsw | (msw<<16);	// fill data buffer
    data++;
  }
  return X;			// return here if no error 
}

//-----------------------------------------------------------------------------
//	Now come functions associated with the setup of the 1821 SMI
//	..Shunting microcode etc.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::MS_1821(UShort_t rom )
{
  // Routine to download LeCroy 1821 sequencer machine code from
  // ROM/RAM memory into sequencer.
  // Parameter ROM.....memory 0-7 (0 = standard ROM)
  
  Int_t i;
  UShort_t val;
  
  fFBCtrlMod->W_KPH(3,0);		// reset the sequencer 
  fFBCtrlMod->W_KPH(3,0x840c);	        // data path: pipeline->dmem 
  fFBCtrlMod->W_KPH(0,0xfb80);	        // assure PMAR & sequ attention 
  for(i=1; i!=3 && i<=6; i++){	        // init other I/O registers 
    fFBCtrlMod->W_KPH(i,0);
  }
  fFBCtrlMod->W_KPH(7,0x0012);	        // DMARL & PMADZ strobes 
  fFBCtrlMod->W_KPH(0,0x1180+rom);	// set up the download 
  fFBCtrlMod->W_KPH(7,0x0006);	        // start it 
  do{
    fFBCtrlMod->R_KPH(7,&val);	        // check it has finished 
  }while((val & 0x0020) != 0);
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::SH_1821()
{
  //  Routine to upload code from 1821 sequencer to host using 2891A Camac
  //  interface. Code deposited on disk file .data/SMIcode.dat

  UShort_t i,j,k;
  FILE *SMIcode_fp;
  UShort_t code[8],word[8];			// line of SMI code 
  
  SMIcode_fp = fopen("./data/SMIcode.dat","w");	// open for write "w" 
  fprintf(SMIcode_fp,
	  "*     upload of resident SMI sequencer code\n");
  fFBCtrlMod->W_KPH(0,0x9680);	// PADDR: R1->seq; PDATA: seq->R2
  for(i=0; i<=255; i++){
    fprintf(SMIcode_fp,"%3x",i);	// formatted dump of 
    // code	into the file 
    fprintf(stdout, "%3x",i);    	// formatted dump of 
    // code	on the screen 
    for(j=0; j<=7; j++){			
      k = i*8 + j;
      fFBCtrlMod->W_KPH(1,k);
      fFBCtrlMod->R_KPH(2,&code[j]);
      code[j] &= 0x00ff;
      switch(j){	// now change to the Oussena format 
      case 0:
	word[0] = code[0] >> 4;
	code[0] &= 0xf;
	break;
      case 1:
	word[1] = code[0]<<4 | code[1]>>4;
	break;
      case 2:
	word[2] = code[1] & 0xf;
	break;
      case 3: case 4:
	word[j] = code[j-1];
	break;
      case 5:
	word[5] = code[4]>>4;
	break;
      case 6:
	word[6] = code[4] & 0xf;
	break;
      case 7:
	word[7] = code[5];
	break;
      }
      fprintf(SMIcode_fp,"%3x",word[j]);
      fprintf(stdout,"%3x",word[j]);
    }
    fprintf(SMIcode_fp,"\n");	// <CR> at end of line 
    fprintf(stdout,"\n");	// <CR> at end of line 
  }
  fclose(SMIcode_fp);	        // close file 
  return 0;		       	// X=0 if all OK 
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::HM_1821( UShort_t ram )
{
  //  Routine to download microcode from Host to Smi Memory
  //  Code assumed to reside on disk file /dd/ACQU/data/smi_X.d
  //  where X=0,1,2,3,4,5,6,7 is the smi ROM/RAM to load
  //  This is passed as a parameter.
  //  Returns........0  Completed without error
  //  -1  Disk file open error
  //  -2  Format error on input line
  //  -3  Bad line sequence (should increase monotonically)
  //  -4  SMI write failure(s) detected

  Char_t s_ram[2];		// to convert RAM short to char 
  Char_t* smiFileName;	// string = name of disk file 
  UShort_t ln;	// line number read from input line 
  Short_t l_curr;		// current line number 
  UShort_t W;	// SMI write error count 
  UShort_t code[8];	// 8 code bytes per line, SMI format	
  UShort_t word[8];	// same line but Oussena format	
  Short_t nl;		// no. parameters on line...8 or 10 
  Char_t* line;

  W=0;  //<<----------------------------------------------------------  Baya
  
  //	Get the right file	
  sprintf(s_ram,"%1d",ram);	// convert short to character 
  smiFileName = BuildName("./data/smi_",s_ram,".d");
  ARFile_t smiFile(smiFileName,"r",this);

  //	Read file line by line and download to SMI until EOF or err 
  //	Assume 132 chars max in line	
  
  fFBCtrlMod->W_KPH(0,0x8800 + ram);	  // set up download to SMI RAM 
  l_curr = -1;                    // init line number 
  while( (line = smiFile.ReadLine()) ) {
    if((nl = sscanf(line,"%hx%hx%hx%hx%hx%hx%hx%hx%hx",
		    &ln,&word[0],&word[1],&word[2],&word[3],
		    &word[4],&word[5],&word[6],&word[7])) != 9 &&
       nl != 7){
      fprintf( fLogStream," Format error line %x\n",l_curr);
      return(-2);
    }
    else if(l_curr >= (short)ln){
      fprintf( fLogStream," Bad line sequence %x %x\n",
	       l_curr,ln);
      return(-3);
    }
    else{
      // Line OK, change from Oussena to SMI format and download it	
      ch_format(word,code,nl);
      W += W_line(ln,code,0x400);
      l_curr = ln;	// update current line 
    }
  }
  fprintf( fLogStream," End of SMI_RAM %d download at line %x\n",
	   ram, l_curr);
  if(W != 0){
    fprintf( fLogStream," %d write errors during load\n",W);
    return(-4);
  }
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::HS_1821( UShort_t ram )
{
  //  Routine to download microcode from Host to Smi Sequencer
  //  Code assumed to reside on disk file /dd/ACQU/data/smi_X.d
  //  where X=0,1,2,3,4,5,6,7 is the smi ROM/RAM to load
  //  This is passed as a parameter.
  //  Returns........0  Completed without error
  //  -1  Disk file open error
  //  -2  Format error on input line
  //  -3  Bad line sequence (should increase monotonically)
  //  -4  SMI write failure(s) detected

  Char_t s_ram[2];	// to convert RAM short to char 
  Char_t* smiFileName;	// string = name of disk file 
  UShort_t ln;	        // line number read from input line 
  Short_t l_curr;	// current line number 
  UShort_t W;	        // SMI write error count 
  UShort_t code[8];	// 8 code bytes per line SMI format	
  UShort_t word[8];	// same line but Oussena format 	
  Short_t nl;		// no. parms on line 
  Char_t* line;

  W=0; //<<<-------------------------------------------------------------------- Baya
  
  //	Get the right file	
  sprintf(s_ram,"%1d",ram);	// convert short to character 
  smiFileName = BuildName("./data/smi_",s_ram,".d");
  ARFile_t smiFile( smiFileName,"r",this );
  
  //	Read file line by line and download to SMI until EOF or err 
  //	Assume 132 chars max in line	
  
  fFBCtrlMod->W_KPH(0,0x9980);	// set up download direct to sequencer 
  l_curr = -1;			// init line number 
  while( (line = smiFile.ReadLine()) ){
    if((nl=sscanf(line,"%hx%hx%hx%hx%hx%hx%hx%hx%hx",
		  &ln,&word[0],&word[1],&word[2],&word[3],
		  &word[4],&word[5],&word[6],&word[7])) != 9 &&
       nl != 7){
      fprintf( fLogStream," Format error after line %x\n",
	       l_curr);
      return(-2);
    }
    else if(l_curr >= (short)ln){
      fprintf( fLogStream," Bad line sequence %x %x\n",
	       l_curr,ln);
      return(-3);
    }
    else{
      //	Line OK, change from Oussena to SMI format and download it	
      ch_format(word,code,nl);
      W += W_line(ln,code,0x100);
      l_curr = ln;	// update current line 
    }
  }
  fprintf( fLogStream," End of direct Sequencer %d download at line %x\n",
	   ram, l_curr);
  if(W != 0){
    fprintf( fLogStream," %d write errors during load\n",W);
    return(-4);
  }
  return(0);
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::SMI_code_gen(short istart_a, short istart_s, short panel)

{
  //  Generate SMI code lines for primary and secondary address
  // The parameters are : 
				// 1- adc start index module table 
			       	// 2- scaler start index module table 
			       	// 3- read mode: ECL/SIB bus 
				
  //  sequences from Fastbus module IRQ readout table. Address info
  //  slot(primary) subaddress(secondary) is held in 3rd byte of line
  //  Error returns	0	OK good return
  //  -W	W SMI write errors during operation
  Int_t W = 0;

  /*
    static UShort_t prim_mask[] = {	// code mask primary address 
    0x40,	0x04,	0x00,	0x08,	0x02,	0x3f,	0x00,	0x00,
    };
    static UShort_t sec_mask[] = {	// code mask secondary address
    0x40,	0x04,	0x00,	0x01,	0x02,	0x1f,	0x00,	0x00,
    };
    static UShort_t end_mask[] = {	// code mask end of addr list 
    0xc0,	0x04,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    };
    UShort_t ln;			// code line no. 
    UShort_t i,i_max;			// FB module index 
    UShort_t W = 0;			// error indicator
  
    struct FB_module *P_f;		// temp module struct ptr 
    char mode;				// read mode of module 
  
    ln = START_ADC;			// smi start line for adc's 
    //
    return address from module read list depends on mode of readout
    3v4...change FRONT_F to FRONT
  
    switch(panel){
    case REAR:			// rear panel ECL bus 
    end_mask[2] = ESMIEndRear;// return real time loop 
    break;
    case FRONT:			// front panel SIB 
    end_mask[2] = ESMIEndFront;
    break;
    default:			// this should not happen 
    fprintf( fLogStream,"ERROR SMI I/O spec.\n");	// error 
    return(-1);
    }
    i_max = DM->F_IRQ;			// gives no of adc modules 
    for(i=istart_a; i<i_max; i++){		// loop round adc's 
    P_f = DM->FB_tbl[i];		// -> adc module struct 
    if(((mode = P_f->read_mode) != 'm') && (mode != 'b')){
    continue;//next loop iter if not addressable module 
    }
    prim_mask[2] = P_f->n;			// station no. 
    sec_mask[2] = P_f->d_read & 0xff;	// subaddress 
    W += W_line(ln++,prim_mask,0x100);	// prim.addr line 
    W += W_line(ln++,sec_mask,0x100);	// sec.addr line 
    }
    W += W_line(ln,end_mask,0x100);		// end line adc's 
  
    ln = START_SCALER;			// smi start line for scalers 
    i_max = DM->F_IRQ_S;			// gives no of scaler modules 
    end_mask[2] = L_END_FRT;		// jump addr at end of list 
    for(i=istart_s; i<i_max; i++){		// loop round scalers 
    P_f = DM->FB_tbl_s[i];		// -> scaler module struct 
    if(P_f->read_mode != 'z'){
    continue;//next loop iter if not addressable module 
    }
    prim_mask[2] = P_f->n;			// station no. 
    sec_mask[2] = P_f->d_read & 0xff;	// subaddress 
    W += W_line(ln++,prim_mask,0x100);	// prim.addr line 
    W += W_line(ln++,sec_mask,0x100);	// sec.addr line 
    }
    W += W_line(ln,end_mask,0x100);		// end line adc's 
    if(W != 0)
    fprintf( fLogStream," %d write errors during SMI code generation\n\n"
    ,-W);
  */
  return W;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::ch_format(
			     UShort_t word[],		// Oussena formatted line	
			     UShort_t code[],		// line for download to SMI	
			     register UShort_t nl	                // 8 or 10 parameters 
			     )
{
  //  SMI microcode line....format change from style of B.Oussena
  //  to form suitable for download to SMI
  //
  Short_t i;	// loop counter 
  if(nl == 9){
    code[0] = word[0]<<4 | word[1]>>4;
    word[1] &= 0xf;
    code[1] = word[1]<<4 | word[2];
    code[2] = word[3];
    code[3] = word[4];
    code[4] = word[5]<<4 | word[6];
    code[5] = word[7];
  }
  else if(nl == 7){
    for(i=0; i<6; i++)
      code[i] = word[i];
  }
  code[6] = code[7] = 0;
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::W_line( UShort_t ln,  UShort_t code[],UShort_t strobe )

{
  //  Write single line of SMI microcode to previously selected SMI RAM
  // The parameters are :
			  	// 1- code line number 
			 	// 2- 8 bytes of code for line 
			  	// 3- write strobe 0x100 for sequencer,0x400 for RAM 			 
  //  Error returns	0	good return
  //  -W	no. of SMI write errors in operation
  UShort_t l;	        // index for line 
  UShort_t W = 0;	// error return variable 
  
  for(l=0; l<8; l++){
    fFBCtrlMod->W_KPH(1,ln*8+l);	//RAM addr
    fFBCtrlMod->W_KPH(2,code[l]);  	//code wrd
    fFBCtrlMod->W_KPH(7,strobe);	
  }
  return W ;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::smi_err_decode(UShort_t addr, UShort_t error)
				
{
  //  Decode the diagnostic number read via the SMI PDREG
  //  which is given if a failure occurs on any of the 3
  // The parameters are :
			// 1 - contains station and data reg 
			// 2- number returned by SMI 

  //  read cycles:
  //  1) primary address lock
  //  2) secondary address write
  //  3) block transfer
  UShort_t i,j;		// general index 
  
  //	1st print station and data (DSR) register at which error occurred 
  fprintf( fLogStream,"SMI Error n=%d, register=%d, event=%d\n",
	   (addr >> 8),(addr & 0xff), fEXP->GetNEvent());
  
  // Now loop round to find the appropriate message to go with the error code	
  for(i=ESMIPrimAddr,j=1; ;i++,j++){
    if((i == error) || (i == ESMIDefault)){
      fprintf( fLogStream,"Error # %d\n",error);
      break;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::smi_error( UInt_t **i_buff, void **p_buff, 
			      Int_t *n_word )
{
  // Decode error detected after a Fastbus list mode data transfer..
  // Either ECL bus or SIB bus
 
  /* 
     UShort_t i;	// general index 
     struct read_error *P1;	// error block structure 
     struct read_error *P2;	// error block structure 
     struct FB_module *fb=NULL;	// -> fastbus module struct 
     short line;			// temp hold error info 
     char flag='?';	      	// adc/scaler flag 
  
     P1 = (struct read_error *)(*i_buff);	// set error block pointer 
     P2 = (struct read_error *)(*p_buff);	// set error block pointer 
  
     //	Get error info output by SMI. Ordering changes ECL or SIB	
     if(DM->FB_fast){
     line = P1->bus;			// ECL 1st 16bit 
     P1->code = P1->crate & 0xff;	// error code 
     }
     else{
     line = P1->crate;		// save next word in buff 
     P1->code = P1->bus & 0xff;	// strip extraneous bits 
     }
     P1->module = line & 0xff;	// returned SS code 
     line = (line & 0xff00)>>8;	// get list line number 
  
     //	Check mode of readout to determine where to get fastbus
     module information	
     if(P_fb->panel == FRONT){	// slow SIB bus transfer ? 
     fb = P_fb;		// from existing structure 
     }
     else{				// fast SIB or ECL bus transfer 
     //	Determine if ADC or scaler line and get offset from start 
     if(line < START_ADC && line >= START_SCALER){
     line = (line - START_SCALER)/2;	//scaler list offset
     flag = 's';			// scaler flag 
     }	
     else if (line <= END_ADC && line >= START_ADC){
     line = (line - START_ADC)/2;	// ADC list offset 
     flag = 'a';			// adc flag 
     }
     //	Find Fastbus module where error occured	
    
     for(i=smilist->first; i<smilist->last; i++){
     if(flag == 'a'){
     fb = DM->FB_tbl[i];	// adc module ptr 
     if(fb->read_mode == 'm'||fb->read_mode == 'b')
     line--;		// check if addr 
     }
     else{
     fb = DM->FB_tbl_s[i];	//scaler module ptr 
     if(fb->read_mode == 'z')
     line--;		// check if addr 
     }
     if(line < 0)
     break;		        // break when line found 
     }
     }
  
     //	Proceed to fill rest of error block	
     P1->bus = 0;
     P1->crate = (smilist->smi) & 0xf;
     P1->station = fb->n;
     P1->sub_addr = (fb->d_read) & 0xff;
     P1->module = RDERR_UNDEF;

     //	Copy over error block to secondary buffer 
     memcpy (P2,P1,sizeof(struct read_error));
     // copy over block 
  
     //	Update buffer pointers and no. of words	
     *i_buff = (char *)(*i_buff) + sizeof(struct read_error);
     *p_buff = (char *)(*p_buff) + sizeof(struct read_error);
     *n_word -= sizeof(struct read_error)/4;
     */
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::W_ped( UShort_t n, UShort_t a, UShort_t data )
{
  // Write value to SMI pedestal subtract memory
  // Ped values 0 - +-511 or 0 - 1023

  UShort_t status;		// to save current status R3 
  
  fFBCtrlMod->R_KPH(3,&status);	// save R3 status 
  fFBCtrlMod->W_KPH(3,0x820b);	// configure ped mem write 
  fFBCtrlMod->W_KPH(5,PedAddr(n,a));// enter pedestal mem  addr R5 
  fFBCtrlMod->W_KPH(4,data);	// load data R4 
  fFBCtrlMod->W_KPH(7,0x4000);	// strobe ped mem address 
  fFBCtrlMod->W_KPH(7,0x8000);	// strobe data to pedestal mem 
  fFBCtrlMod->W_KPH(3,status);	// restore R3 status 
  return 0;			// return any error picked up 
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::R_ped( UShort_t n, UShort_t a, UShort_t *data )
{
  // Read value of SMI pedestal subtract memory to host
  // Ped values 0 - +-511 or 0 - 1023
  UShort_t status;		// to save current status R3   
  fFBCtrlMod->R_KPH(3,&status);	// save R3 status 
  fFBCtrlMod->W_KPH(3,0x820b);	// configure ped mem write 
  fFBCtrlMod->W_KPH(5,PedAddr(n,a));// enter pedestal mem  addr R5 
  fFBCtrlMod->W_KPH(7,0x4000);	// strobe ped mem address 
  fFBCtrlMod->W_KPH(3,0x820e);	// configure ped. mem. read 
  fFBCtrlMod->R_KPH(4,data);	// read pedestal value 
  fFBCtrlMod->W_KPH(3,status);	// restore R3 status 
  return 0;			// return any error picked up 
}

//-----------------------------------------------------------------------------
Int_t TFB_1821SMI::W_thr( register UShort_t data )
{
  // Write value to SMI threshold (data compress) registers
  // NB data in range 0-255, bits 8-15 truncated

  UShort_t status;		// to save R3 status 
  UShort_t addr;	        // "range" address 

  fFBCtrlMod->R_KPH(3,&status); // save R3 status   //Baya modified these calls
  fFBCtrlMod->W_KPH(3,0x800b);	// configure threshold reg write 
  addr = 0;			    // start low range 
  while(addr <= 0x80){		    // loop low and high range 
    fFBCtrlMod->W_KPH(5,addr);	    // range adddress R5 
    fFBCtrlMod->W_KPH(4,data);	    // data R4 
    fFBCtrlMod->W_KPH(7,0x4000);    // strobe range address 
    fFBCtrlMod->W_KPH(7,0x1000);    // strobe data to threshold reg 
    addr += 0x80;		    // next range 
  }
  fFBCtrlMod->W_KPH(3,status);	    // restore status 
  return 0;			// return any errors 
}
//-----------------------------------------------------------------------------
void TFB_1821SMI::Init_SMI1821(Char_t* line)//<-----   Baya
{
  short np;			// no. parameters
  short err = 0;	


  // SMI setup: read in min 3 max 6 parameters */
  np = sscanf(line,"%d%d%d%x%d%d",
	      &fSMI,&fRAM,&fSlots,&fPipe,&fThresh,&fVMEInterface);
  switch(np){
  default:
    PrintError(line, "<Parse Master Fastbus parameter file mess>");
    return;	               // file format error
  case 3:
    fPipe = ESMIPipeDisable;   // def disable pipe tests
  case 4:
    fThresh = 0;	       // default zero pipline threshold
  case 5:
    fVMEInterface = 0;         // default 0th SMI interface
  case 6:
    break;
  }

  fSMI |= fVMEInterface << 8;
  fIPanel = ESMIFront;	       // slow SIB

  fFBCtrlMod->S_KPH(fSMI);     // select smi
  err = W_thr(fThresh);	       // write threshold reg's
  err += MS_1821(0);           // load ROM LeCroy code
  err += HS_1821(0);           // any bug fix to standard code
  err += Exec1821(ESMIClrFB);  // clear fb lines (Baya)

  /*	If no slave slots defined leave SMI in slave state and loop next SMI */
  if(fSlots <= 0){
    Printf("Warning SMI set in slave mode ... no modules defined mess[17]");
    return;
  }
  err += Exec1821(ESMIBusMaster); // grab bus master <<-- Baya
  if(err){
    PrintError(line, "<Error initialising SMI>");
    return;	                  // error initialising smi
  }
  return;
}

//-----------------------------------------------------------------------
Int_t TFB_1821SMI::Add_FbADC(TDAQmodule* mod)
{
  fADCList->AddLast(mod);      // Add ADC to the SMI1821  list
  fNADC++;                     // increment no. of ADCs
  return(0);
}

//-----------------------------------------------------------------------
Int_t TFB_1821SMI::Add_FbSCA(TDAQmodule* mod)
{
  fScalerList->AddLast(mod);      // Add Scaler to the SMI1821 to list
  fNScaler++;                     // increment no. of scalers
  return(0);
}
ClassImp(TFB_1821SMI)
