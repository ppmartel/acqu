//--Author	JRM Annand      7th Jun 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand..   28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..   25th Oct 2009..further development
//--Rev         B. Oussena          Jun 2010  Added new cmd EcatchMapSize
//                                            Added virtual mapping access 
//                                            Fixed an error in TVME_CATCH()
//--Rev         B. Oussena    24th Jul 2010  PostInit() performs only mapping 
//--Rev         B. Oussena    26th Jul 2010  fix front panel id display: UInt_t
//--Rev         B. Oussena    13th Aug 2010  more cleaning 
//--Rev         JRM Annand     5th Sep 2010  Bug constructer rm fSpyData=NULL 
//--Rev         JRM Annand     5th Sep 2010  Bug constructer rm fSpyData=NULL 
//--Rev         JRM Annand     6th Sep 2010  fMaxSpy = spy buffer size
//                                           Add GetEventID()
//--Rev         JRM Annand     8th Sep 2010  Set TCS event ID in spy-buffer read
//--Rev         JRM Annand    12th Sep 2010  New WrtCatch(UInt_t, UInt_t)
//--Rev         JRM Annand     6th Feb 2011  Pause = usleep(100)...was usleep(1)
//                                           Use WrtCatch(UInt_t,UInt_t) with
//                                           usleep(1) for reg programming
//--Rev         JRM Annand     1st May 2011  Command codes start 200
//--Rev         JRM Annand     5th Jul 2011  gcc4.6-x86_64 ULong_t <-> UInt_t*
//--Update      JRM Annand     2nd Sep 2012  Send event ID to remote
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CATCH
// COMPASS CATCH hardware
//

#include "TVME_CATCH.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"



static Map_t kCATCHKeys[] = {
  {"FPGAFile:",   EFPGAFile},
  {"TCSRecFile:", ETCSRecFile},
  {"F1File:",     EF1File},
  {"CatchWrt:",   ECatchWrt},
  {"CatchParm:",  ECatchParm},
  {"CatchMapSize:",  ECatchMapSize}, //<<<---- Baya
  {NULL,                  -1},
};

// downloading setup parameters for {CATCH1 , CATCH-X/CATCH}
UInt_t p_PROG_L[] =  {0x04, 0x04} ; 
UInt_t p_PROG_H[] =  {0x08, 0x08};
UInt_t p_DIN_H[]  =  {0x10, 0x40};
UInt_t p_DIN_L[]  =  {0x20, 0x80}; 
UInt_t p_CCLK_H[] =  {0x40, 0x10};
UInt_t p_CCLK_L[] =  {0x80, 0x20}; 
UInt_t p_ENA_H[]  =  {0x01, 0x01};
UInt_t p_ENA_L[]  =  {0x02, 0x02};
UInt_t p_ENAT_H[]  =  {0x0, 0x01};
UInt_t p_ENAT_L[]  =  {0x0, 0x02};
UInt_t p_ENAS_H[]  =  {0x0, 0x04};
UInt_t p_ENAS_L[]  =  {0x0, 0x08};
UInt_t OFF_ENA[] = {0x0088, 0x010};
UInt_t OFF_ENA_TS[] = {0x0, 0x018};
UInt_t OFF_PROG[] = {0x0088, 0x100};
UInt_t OFF_STATUS[] = {0x0010, 0x004};
UInt_t OFF_PROG_FAST[] = {0x0000, 0x020};
UInt_t DONE[] = {0x4000 ,0x400000  };
UInt_t BUSY[] = {0x00000,0x1000000 };
UInt_t INIT[] = {0x08000,0x800000  };
UInt_t TDONE[] = {0x00 ,0x20000000  };
UInt_t TINIT[] = {0x00 ,0x40000000  };
UInt_t SDONE[] = {0x00 , 0x8000000  };
UInt_t SINIT[] = {0x00 ,0x10000000  };
//
//	Catch Register Offsets
//
VMEreg_t CatchReg[] = {
  {0x0,		0x0,	'l',	0},           // ID register
  {0x4,		0x0,	'l',	0},           // Status
  {0x1804,      0x0,	'l',	0},           // Spy Buffer control
  {0x8000,     	0x0,	'l',	0},           // Spy data buffer
  {0x440,     	0x0,	'l',	0},           // F1 access register
  {0xffffffff,  0,       0,     0}            // terminator
};


//-----------------------------------------------------------------------------
TVME_CATCH::TVME_CATCH( Char_t* name, Char_t* file, FILE* log,
			Char_t* line ):
  TVMEmodule( name, file, log, line )
{
  // Basic initialisation
  // Some variables may be defined by the subsequent specific constructor
  //

  fSpyData = new UInt_t[ECATCH_BuffSize];
 
  fCtrl = NULL;                            // no control functions
  fType = EDAQ_ADC;                        // analogue to digital converter
  AddCmdList( kCATCHKeys );                // CATCH-specific setup commands
  ULong_t temp = (ULong_t)fBaseAddr;
  fCWrt = NULL;
  fMaxSpy = ECATCH_BuffSize;
  fIWrt = 0;
  fCatchID = (temp & 0x00ff0000)>>16;//Baya 21 Jul (temp & 0xff000000)>>16;
  fTCSEventID = 0;
  fFPGAfile = fTCSRecFile = fF1File = NULL;
}

//-----------------------------------------------------------------------------
TVME_CATCH::~TVME_CATCH( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_CATCH::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  //  Int_t ithr, thr, i;
  //  Int_t nparm;
  Int_t parms[16];
  Char_t fpgaFile[64];               // name of FPGA program file

  Int_t mapsize = 0;//<<<---------------------------------- Baya
  switch( key ){
  case EFPGAFile:
    // Catch FPGA download
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<FPGA file name>",EErrFatal);
    fFPGAfile = BuildName(fpgaFile);
    break;
  case ETCSRecFile:
    // TCS receiver FPGA download
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<TCS receiver FPGA file name>",EErrFatal);
    fTCSRecFile = BuildName(fpgaFile);
    break;
  case EF1File:
    // F1 TDC FPGA file spec...leave the download to later
    // some register preprograming necessary
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<F1 TDC FPGA file name>",EErrFatal);
    fF1File = BuildName( fpgaFile );
    break;
  case ECatchParm:
    // General ADC or scaler parameters
    //    if( !nparm ) return -1;
    sscanf(line,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
	   parms,    parms+1,  parms+2,  parms+3,
	   parms+4,  parms+5,  parms+6,  parms+7, 
	   parms+8,  parms+9,  parms+10, parms+11, 
	   parms+12, parms+13, parms+14, parms+15);
    break;
  case ECatchWrt:
    // General wild-card register setup
    InitCatchWrt(line);
    break;

    //------------------------------------------------------ Baya
  case ECatchMapSize:
    // Read  Memory Map Size   
    if(sscanf(line, "%x", &mapsize)<1)
      PrintError(line, "Base address parse");
    fMapSize = mapsize;
    break;
    //------------------------------------------------------- Baya

  default:
    // default try commands of TDAQmodule
    TVMEmodule::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_CATCH::PostInit( )
{
  // General CATCH initialisation
  // Setup device registers
  // If full initialisation set download FPGA files and do
  // 1st round of register setup
  //
 
  if( fIsInit ) return;
  InitReg( CatchReg );
  
  TVMEmodule::PostInit();
  //  DAQMemMap_t* map = NULL; //<<---------------------------------  Baya
  //  map = fMemMap; //<<-------------------------------------------  Baya
  //  VirtAddr = (Long_t)(map->GetVirtAddr()); //<<------------------ Baya
  return;
}

//---------------------------------------------------------------------------
void TVME_CATCH::InitCatchWrt( Char_t* line )
{
  // Setup a series of register write commands

  //char* baseaddr = (char*)fBaseAddr;  //<<---------------------------  Baya


  Int_t offset, value;    // register address offset & initialisation value
  Int_t id = 0;           // add catchid to value...default no
  CatchWrt_t* cw;       // -> current cmd struct element

  // read line from setup file...at least 2 values necessary
  // offset is assumed a byte offset
  if( sscanf( line, "%x%x%x", &offset, &value, &id ) < 2 )
    PrintError(line,"<CATCH command parse>",EErrFatal);

  if( !fCWrt ) fCWrt = new CatchWrt_t[ECATCH_CmdSize];
  cw = fCWrt + fIWrt;

  // this specifies a break in the register setup chain...eg if
  // a FPGA setup needs to be done and then resume register setup
  if( offset == -1 ) cw->addr = NULL;
  // normal address setup
  else cw->addr = (UInt_t*)((ULong_t)offset);
       
  //
  if( id ) value += fCatchID;
  cw->value = value;
  fIWrt++;                // update # commands
  (cw + 1)->addr = NULL;  // in case this is the last command
  return;
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH::WrtCatch( Int_t iwrt )
{
  // Sequence of writes to registers, which may be interrupted
  // and then restarted. iwrt is the start index in the sequence

  ULong_t offset;

  struct CatchWrt_t *cw;  // -> current cmd struct element
  cw = fCWrt + iwrt;
  // do series of register write commands until zero address found
  while( cw->addr ){

    offset = (ULong_t)(cw->addr);   
    WrtCatch( offset, cw->value );//<<--26 Jul--- Baya
    cw++;
    iwrt++;
  }
  iwrt++;        // go past zero entry in case there is more to do
  return iwrt;   // return current position in command chain
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH::ProgFPGA(Char_t* filename, Int_t port)
{
  // Download binary code to FPGA
  // lprogcatch.c lib-progcatch.
  // Derived from Freiburg's "progcatch.c", 
  // Apr. 23 03 - dk - started.
  // JRMA..integrate into general ACQU++ catch software
  Int_t i, c, slow, retry, err;
  UInt_t buffer;
  FILE *fp;
  retry = 0;
  fprintf(fLogStream, "Downloading FPGA CATCH #%x\n", fCatchID);
  // Open "binary" file
  fp = fopen(filename, "r");
  if (fp == NULL){
    printf("Can't open %s\n", filename);
    return -1;
  }
  // Check CATCH type
  Read( GetVirtAddr(0x0), &buffer, 0x39, 4 );  //Baya
  fprintf(stderr, "Board: %x - ", buffer);
  if (buffer >>16 == 0xCA01)      { c = 1; slow=1; port = ECATCHport; }
  else if (buffer >>16 == 0xCA04) { c = 1; slow=3; }
  else if (buffer >>16 == 0xCA10) { c = 0; slow=3; port = ECATCHport; }
  else {
    fprintf(stderr, "Unknown CATCH Board: %x\n", buffer);
    return -4;
  }
  do {
	  err=0; // reset error
    if (port == ETCSport) 
      fprintf(stderr, " TCS receiver."); 
    else if (port == ESLINKport) fprintf(stderr, "SLINK multiplexer.");
    //
    if (port == ECATCHport) 
      WrtCatch( OFF_ENA[c], p_ENA_H[c]);//Baya
    else if (port == ETCSport)
      WrtCatch( OFF_ENA_TS[c], p_ENA_H[c]);//Baya
    else if (port== ESLINKport)
      WrtCatch( OFF_ENA_TS[c], p_ENA_H[c]); //Baya
    if (slow==2) {
      // Configuration via VME enabled (slow method)
      WrtCatch( OFF_PROG[c], p_ENA_H[c]); //Baya
    }
    // Reset the FPGA
    WrtCatch( OFF_ENA[c], p_PROG_L[c]);//Baya
    Pause(); 
    
    // Initialise the FPGA (PROG is 1) */
    WrtCatch( OFF_ENA[c], p_PROG_H[c]);//Baya
    Pause(); 
    
    for( i=0; i<=(Int_t)ECATCH_Timeout; i++ ){
      if( InitFlag(port, c) ) break;
    }
    if ( i == ECATCH_Timeout){
      PrintError("Timeout", "<CATCH Init Flag>");
      err=3;
    } 
    else err = Fileout(fp, slow, port, c); 
  
    Pause(); 
    if(Done(port, c) == 0){
      fprintf(stderr, " ERROR!\n");
      if (err != 5) err = 1;
      for( i=0; i<=15; ++i){
	Din(1, c);
      } 
    }
    else 
      fprintf(stderr, " OK.\n");

    if (port == ECATCHport)
      WrtCatch( OFF_ENA[c], p_ENA_L[c]); //Baya
    else if (port == ETCSport)
      WrtCatch( OFF_ENA_TS[c], p_ENAT_L[c]);// Baya
    else if (port == ESLINKport)
      WrtCatch( OFF_ENA_TS[c], p_ENAS_L[c]); //Baya
    
    if (slow==2) { 
      WrtCatch( OFF_PROG[c], p_ENA_L[c]); //Baya
    }
    if (err == 5) /* retry programming if init was low */ 
      {
	      fprintf(stderr, "Retry programming...\n");
	      retry-- ;
      }
    else
      retry=0;
  }
  while(retry>0);
  
  fclose(fp);
  
  if (err) return (-err);
  else return 0;
}

//---------------------------------------------------------------------------
void TVME_CATCH::Pause()
{
  // Mickey Mouse delay
  //usleep(100);
  printf(".");
}

//---------------------------------------------------------------------------
void TVME_CATCH::Din(ULong_t a, Int_t c)
{
  // ??
  if (a == 1)
    WrtCatch( OFF_PROG[c], p_DIN_H[c] );
  else
    WrtCatch( OFF_PROG[c], p_DIN_L[c] );
  
  WrtCatch( OFF_PROG[c], p_CCLK_H[c] );
  WrtCatch( OFF_PROG[c], p_CCLK_L[c] );
}


//---------------------------------------------------------------------------
Int_t TVME_CATCH::InitFlag(Int_t port, Int_t c)
{
  // ??
  UInt_t buffer;
  Read( GetVirtAddr(OFF_STATUS[c]), &buffer, 0x39, 4 ); // Baya
  switch (port) {
  case ECATCHport: return (buffer & INIT[c] ? 1 : 0);
  case ETCSport:   return (buffer & TINIT[c] ? 1 : 0);
  case ESLINKport: return (buffer & SINIT[c] ? 1 : 0);
  }
  return -1;
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH::Done(Int_t port, Int_t c)
{
  // ??
  UInt_t buffer;
  Read( GetVirtAddr(OFF_STATUS[c]), &buffer, 0x39, 4 ); // Baya
  switch (port) {
  case ECATCHport: return (buffer & DONE[c]  ? 1 : 0);
  case ETCSport:   return (buffer & TDONE[c] ? 1 : 0);
  case ESLINKport: return (buffer & SDONE[c] ? 1 : 0);
  }
  return -1;
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH::Busy(Int_t c)
{
  // ??
  UInt_t buffer; 
  Read( GetVirtAddr(OFF_STATUS[c]), &buffer, 0x39, 4 ); //Baya
  return (buffer & BUSY[c] ? 1 : 0);
}

//---------------------------------------------------------------------------
void TVME_CATCH::Din32(Int_t c, UInt_t a)
{
  // ??
  WrtCatch( OFF_PROG_FAST[c], a);  //Baya
}


//---------------------------------------------------------------------------
Int_t TVME_CATCH::Fileout(FILE *ipf, Int_t slow, Int_t port, Int_t c)
{
  // Scan and decode a line from the FPGA file
  Char_t str;
  Int_t i, k, err;
  Char_t *endptr1;
  Long_t ret1;
  UInt_t bits, pos;

  rewind(ipf);
  while(!feof(ipf)){
    bits = 0;
    pos = 1 ;
    k = -1 ;
    for( i=0; i<=7; i++) {
      fscanf(ipf,"%c",&str);
      if(str == '\r') 
	break;
      if(str == '\n') 
	break;
      k = i ;
      ret1 = strtol(&str, &endptr1, 16);
      if (i%2 == 0) 
	bits += ret1 * 16 * pos ;
      else {
	bits += ret1 * pos ;
        pos = pos << 8 ;
      }
    }
    if (c) {
      if (slow == 2) {
	for(i=0; i<=k*4+3; i++){
	  if((bits & 1) == 1)
	    Din(1, c); 
	  else 
	    Din(0, c);
	  bits = bits >> 1 ;
	};
	if (InitFlag(port, c)==0) {
		fprintf(stderr, "Init is low! Programming stopped\n");
		err = 5;
	  return err;
	}
      }
      else {
	Din32(c, bits); 
	if (InitFlag(port, c)==0) {
	  fprintf(stderr, "Init is low! Programming stopped\n");
	  err = 5;
	  return err;
	}
	k=0;
	/*mywait();*/
	while (Busy(c)==1) k++;
      }
    }
    else {
      for(i=0; i<=k*4+3; i++){
	if((bits & 1) == 1)
	  Din(1, c); 
	else 
	  Din(0, c);
	bits = bits >> 1 ;
      }
      /*if (InitFlag(port)==0) {
	printf("Init is low! Programming stopped\n");
	err = 5;
	return err;
	};*/
    }
  }
  return 0;
}

ClassImp(TVME_CATCH)
