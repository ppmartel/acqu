//--Author	JRM Annand	 7th Jun 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..25th Oct 2009..further development
//--Rev         B.Oussena   22nd Jul 2010 Added new cmd EcatchMapSize
//--Rev         B.Oussena                 fix an error kVME_CATCH_TCSKeys[]
//--Rev         B.Oussena                 set access to Virtual adr
//--Rev         B.Oussena                 modified PostInit()
//--Rev         JRM Annand  12th Sep 2010 Add mickey-mouse method ReConfig
//--Update      JRM Annand   6th Jul 2011 gcc4.6-x86_64 warning fix
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CATCH_TCS
// COMPASS Trigger Controller for CATCH and GeSiCA
//

#include "TVME_CATCH_TCS.h"
#include "TDAQexperiment.h"
#include "ARFile_t.h"


static Map_t kVME_CATCH_TCSKeys[] = {
  {"TCSFile:",    ETCSFile}, 
  {"TCSFile1:",   ETCSFile1},
  {"TCSFile2:",   ETCSFile2},
};

//-----------------------------------------------------------------------------
TVME_CATCH_TCS::TVME_CATCH_TCS( Char_t* name, Char_t* file, FILE* log,
				Char_t* line ):
  TVME_CATCH( name, file, log, line )
{
  // Basic initialisation 
  fCtrl = NULL;                            // no control functions
  fType = EDAQ_SlowCtrl;                   // slow control function
  AddCmdList( kVME_CATCH_TCSKeys );        // CATCH-specific setup commands
  fTCSfile = fTCSfile1 = fTCSfile2 = NULL; // undefined file names
}

//-----------------------------------------------------------------------------
TVME_CATCH_TCS::~TVME_CATCH_TCS( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_CATCH_TCS::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  Char_t fpgaFile[256];
  switch( key ){
  case ETCSFile:
    // TCS FPGA download new-style
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<TCS FPGA file name>",EErrFatal);
    fTCSfile = BuildName(fpgaFile);
    break;
  case ETCSFile1:
    // TCS FPGA download old style...1st FPGA download
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<TCS FPGA file name>",EErrFatal);
    fTCSfile1 = BuildName( fpgaFile );
    break;
  case ETCSFile2:
    // Old style TCS needs 2nd FPGA download
    if(sscanf(line,"%s", fpgaFile ) != 1 )
      PrintError(line,"<TCS FPGA file name>",EErrFatal);
    fTCSfile2 = BuildName(fpgaFile);
    break;
  default:
    // default try commands of TDAQmodule
    TVME_CATCH::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_CATCH_TCS::PostInit( )
{
  // Only if absolute full (level 2) initialisation demanded
  // Download 1 of 3 options for TCS FPGA file...
  // depends on the hardware version (old needs 2 downloads)
  // Then do some register manipulation black magic
  //

  if( fIsInit ) return;
  TVME_CATCH::PostInit(); //<<<  ------------ 22 Jul -------  Baya

  if(fInitLevel < EExpInit2 ) return;    // level 2 initialisation only
  if( fTCSfile ) ProgTCS( fTCSfile, ETCS_ConReg0 );
  else if( fTCSfile1 ){
    ProgTCS( fTCSfile1, ETCS_ConReg1 );
    if( fTCSfile2 ) ProgTCS( fTCSfile2, ETCS_ConReg2 );
  }
  // apparently this register setup needs to be done twice...
  fIWrt = WrtCatch( 0 );
  usleep(10);
  fIWrt = WrtCatch( 0 );
  return;
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH_TCS::ProgTCS( Char_t* filename, UInt_t conOffset )
{
  // Download .rbt file contents to TCS FPGA
  //
  Int_t length;
  Bool_t* data = NULL;
  UShort_t datum;
  //  int testit=0;
  Int_t i;
  

  // read data
  data = FillBuffer(filename,&length);

 
  
  //UShort_t* conAddr = (UShort_t*)fBaseAddr + conOffset; //<<  Baya 22 Jul
  //  UShort_t* conAddr = (UShort_t*)VirtAddr + conOffset; // -- Baya 22 Jul
  // JRMA 4/2/11
  // NB the conOffset is offset in 2-byte words
  // GetVirtAddr is working in 1-byte pointers
  // Thus multiply conOffset by 2
  void* conAddr = GetVirtAddr(conOffset*2);


  Int_t am = 0x39;
  datum = 0x7;                               // all bits high: PROGRAM=1
  Write( conAddr, &datum, am, 2 );
  usleep(1);
  datum = ETCS_CClk ;
  Write( conAddr, &datum, am, 2 );

  // Wait for initialised flag  
  for( i=0; ;i++ ){
    Read( conAddr, &datum, am, 2 );
    if( datum & ETCS_Init )break;
    if( i >= ETCS_MaxInitTry )
      PrintError("","<Timeout TCS Initialised>",EErrFatal);
    usleep(1);
  }

  // Do the FPGA download
  Int_t done = 0;
  for( i=0; i<length; i++) {
    // first step
    datum = data[i] << 2; 
    Write( conAddr, &datum, am, 2 );
    // second step
    datum |= ETCS_CClk;       
    Write( conAddr, &datum, am, 2 );
    // Get status
    Read( conAddr, &datum, am, 2 );
    if( (datum & ETCS_Done) ) done++;     
    // make some screen show!
    if ( (i % 100000) ==0 )
      fprintf(fLogStream, "## TCS %i datawords send!\n",i);
  }
  if (done>2) {
    //Transmitting data done
    fprintf(fLogStream,"TCS download rbt file of %d bytes\n\
    done flag received %d times\n", length, done);
    done =0;
  } 
  else {
    PrintError("FPGA download","<FPGA signals not complete>");
    done=1;
  }
  if( data ) delete data;
  return done;
}



//---------------------------------------------------------------------------
Bool_t* TVME_CATCH_TCS::FillBuffer(Char_t* name, Int_t* length)
{ 
  // Read binary data (strings of 0/1 characters) from
  // Xilinx Bitstream .rbt file into a memory buffer
  // The address of the buffer as well as the length is returned.
  // Any file-format errors considered fatal
  //
  FILE* fp;
  Int_t len = 0;
  Char_t dummy[16];
  Char_t line[128];
  Char_t bLine[ETCS_BinLine];
  Bool_t* data;
  Bool_t* d;
  Int_t i,n;
  
  // open file
  if ( (fp=fopen(name,"r")) == NULL )
    PrintError("name","<TCS Binary file open error>",EErrFatal);
  
  // Look for "Bits:" directive in header...
  // # subsequent binary bits to read
  for (i=0; i<16; i++){
    fgets(line, sizeof(line), fp);
    n = sscanf(line,"%s %d",dummy,&len);
    if( (n==2) && (strncmp(dummy,"Bits:",5)==0) )break; 
  }
  if( (i==6) && (n==2) ) data = new Bool_t[len + 1];
  else
    PrintError(name,"<Incorrect TCS binary file header>",EErrFatal);

  // Read the binary lines 32 bits per line
  d = data;
  i = 0;
  while( fgets( line, sizeof(line), fp ) ){
    n = sscanf( line,"%s",bLine );
    for( Int_t k=0; k<ETCS_BinLine; k++ ){
      if( bLine[k] == '0' ) d[k] = 0;
      else if( bLine[k] == '1' ) d[k] = 1;
      else PrintError(name,"<Incorrect TCS binary line format>",EErrFatal);
    }
    d += ETCS_BinLine;
    i += ETCS_BinLine;
    if( i > len )
      PrintError(name,"<TCS # binary data>", EErrFatal);
  }
  if( i != len )
    PrintError(name, "<TCS incompatible # bits>", EErrFatal);
  // All seems OK
  fprintf( fLogStream,"TCS %d binary bits read from file %s\n", len, name );
  *length = len;
  return data;

}

//-------------------------------------------------------------------------
void TVME_CATCH_TCS::SetRunMode(UInt_t runmode)
{
  // note that only the bit0 should be changed,
  // but ReConfig() sets 0x24 to 0x101, so we stick to this! 
  // note that the TCS controller only reacts on this
  // bit when an End-of-Spill / Begin-of-Spill procedure is
  // executed, this can be done by ResetTrigCtrl() by the VUPROM
  if(runmode == 0x0) {
    WrtCatch(0x24, 0x100);
  }
  else if(runmode == 0x1) {
    WrtCatch(0x24, 0x101);
  }
}

//-------------------------------------------------------------------------
void TVME_CATCH_TCS::ReConfig( )
{
  // Run the re-synchronisation procedure
  // This mimics the config_tcs script originally implemented
  // for acqu 3v7 running on the old LynxOS systems
  // Mickey-Mouse solution just to get running
  WrtCatch(0x28, 0x18800);
  // REC 0
  WrtCatch(0x28, 0x80080);
  WrtCatch(0x28, 0xd0001);
  // REC 1
  WrtCatch(0x28, 0x80480);
  WrtCatch(0x28, 0xd0101);
  // REC 2
  WrtCatch(0x28, 0x80880);
  WrtCatch(0x28, 0xd0201);
  // REC 3
  WrtCatch(0x28, 0x80c80);
  WrtCatch(0x28, 0xd0301);
  // REC 4
  WrtCatch(0x28, 0x81080);
  WrtCatch(0x28, 0xd0401);
  // REC 5
  WrtCatch(0x28, 0x81480);
  WrtCatch(0x28, 0xd0501);
  // REC 6
  WrtCatch(0x28, 0x81880);
  WrtCatch(0x28, 0xd0601);
  // REC 7
  WrtCatch(0x28, 0x81c80);
  WrtCatch(0x28, 0xd0701);
  // REC 8
  WrtCatch(0x28, 0x82080);
  WrtCatch(0x28, 0xd0801);
  // REC 9
  WrtCatch(0x28, 0x82480);
  WrtCatch(0x28, 0xd0901);
  // REC 10
  WrtCatch(0x28, 0x82880);
  WrtCatch(0x28, 0xd0a01);
  // REC 11
  WrtCatch(0x28, 0x82c80);
  WrtCatch(0x28, 0xd0b01);
  // REC 12
  WrtCatch(0x28, 0x83080);
  WrtCatch(0x28, 0xd0c01);
  // REC 13
  WrtCatch(0x28, 0x83480);
  WrtCatch(0x28, 0xd0d01);
  // REC 14
  WrtCatch(0x28, 0x83880);
  WrtCatch(0x28, 0xd0e01);
  // REC 15
  WrtCatch(0x28, 0x83c80);
  WrtCatch(0x28, 0xd0f01);
  // REC 16
  WrtCatch(0x28, 0x84080);
  WrtCatch(0x28, 0xd1001);
  // REC 17
  WrtCatch(0x28, 0x84480);
  WrtCatch(0x28, 0xd1101);
  // Reset FE
  // WrtCatch(0x28, 0x20001  # declare MainDAQ as running
  WrtCatch(0x28, 0x10800);
  // configure controller
  // deadtime
  WrtCatch(0x64, 0x10);
  WrtCatch(0x84, 0x4);
  WrtCatch(0xA4, 0x640);
  // start run: only MainDAQ
  WrtCatch(0x44, 0xfffff);
  // see also SetRunMode when changing this value!
  WrtCatch(0x24, 0x101);
  return;
}
ClassImp(TVME_CATCH_TCS)
