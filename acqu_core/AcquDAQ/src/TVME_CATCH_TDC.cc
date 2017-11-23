//--Author	JRM Annand	 7th Jun 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..25th Oct 2009..further development
//
//--Rev         B. Oussena  24 Jul 2010 modified PostInit()
//--Rev         B. Oussena  24 Jul 2010 set access to virt adr in F1Setup()
//--Rev         B. Oussena  13th Aug 2010  more cleaning 
//--Update      JRM Annand   8th Sep 2010  TCS event ID to TVME_CATCH
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CATCH_TDC
// COMPASS CATCH hardware
//

#include "TVME_CATCH_TDC.h"
#include "TDAQexperiment.h"
//#include "TDAQmemmap.h"


static Map_t kCATCH_TDC_Keys[] = {
  {NULL,                  -1},
};

//-----------------------------------------------------------------------------
TVME_CATCH_TDC::TVME_CATCH_TDC( Char_t* name, Char_t* file, FILE* log,
				Char_t* line ):
  TVME_CATCH( name, file, log, line )
{
  // Basic initialisation 
  AddCmdList( kCATCH_TDC_Keys );                // CATCH-specific setup commands
}

//-----------------------------------------------------------------------------
TVME_CATCH_TDC::~TVME_CATCH_TDC( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_CATCH_TDC::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  //  Int_t ithr, thr, i;
  switch( key ){
  default:
    // default try commands of TDAQmodule
    TVME_CATCH::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_CATCH_TDC::PostInit( )
{
  // Do the default CATCH initialisation
  // If full initialisation demanded setup the F1 TDC chip
  // and do 2nd round of register setup
  //
  if( fIsInit ) return;
  TVME_CATCH::PostInit();

  if( fInitLevel >= EExpInit1 ){   // level >=1 init. only

	  if(ProgFPGA(fFPGAfile, ECATCHport) != 0) {
		  PrintError("","Programming CATCH failed",EErrFatal);
	  }
    sleep(1);
    if(ProgFPGA(fTCSRecFile, ETCSport) != 0) {
	    PrintError("","Programming TCS failed",EErrFatal);
    }
    sleep(1);
    // 1st round of register programing
    fIWrt = WrtCatch( 0 );
    sleep(1);
  }



  if( fInitLevel >= EExpInit1 ){
    // F1 TDC chip setup
    if ( F1ReadFile(fF1File, &fF1data) == 0 ){
      F1Setup(&fF1data);
    }
    // 2nd and final round of register programing
    fIWrt = WrtCatch( fIWrt );
  }
  return;
}

//-------------------------------------------------------------------------
void TVME_CATCH_TDC::ReadIRQ( void** outBuffer )
{
  // Read and decode Catch Data Buffer via VMEbus Spy Buffer
  //
  UInt_t datum;                    // temp TDC datum
  UShort_t value;                  // value read from buffer
  UShort_t nword,n;	           // words in data buffer
  UShort_t a;	                   // TDC index
  // Read the CATCH spy (VMEbus) buffer
  if( !(nword = SpyRead(outBuffer) ) ) return;
  nword--;
  // Store TCS event number for event-stamp distribution
  // Moved to TVME_CATCH 08/09/10
  //  fTCSEventID = fSpyData[0];
  // Now decode and store the TDC values
  // Leave reference decoding to the AcquRoot analyser
  for( n=2; n<nword; n++ ){
    datum = fSpyData[n];
    value = (datum & 0xffff);
    a = ((datum & 0x003f0000)>>16) + ((datum & 0x03000000)>>19) + fBaseIndex;
    ADCStore( outBuffer, value, a );// store values
  }
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH_TDC::F1ReadFile(char* filename, F1Data_t* f1data){

  // decode file of type *.f1 written by 'f1conf' 
  // with all register settings, tdc selection and board number
  //
  Int_t len,s,i;
  ULong_t val;
  Char_t line[255], var[255];
  FILE *ipf;
  
  ipf = fopen(filename, "r");
  if (ipf == NULL){
    fprintf(stderr, "f1setup: can't open %s\n", filename);
    return -1;
  }

  f1data->tdc = 0;
  f1data->board = 0;
  f1data->Xcatch = 0;
  len=255;
  while (fgets(line, len, ipf) != NULL) {
    // '#' at beginning of line is comment character
    if (line[1] != '#') {
      // variables are defined as 'set variable value'
      s = sscanf(line,"set %s %li", var, &val);
      // remove the warning if you can 
      if (s!=0) {
	if (strcmp(var,"board") == 0) {
	  f1data->board = val ;
	} else if (strcmp(var,"catchx") == 0) {
	  f1data->Xcatch = val ;
	} else if (strcmp(var, "tdcid") == 0) {
	  f1data->tdc =   f1data->tdc | val << 1 ;
	} else if (strcmp(var, "tdcall") == 0) {
	  f1data->tdc =   f1data->tdc | val ;
	} else if (strncmp(var, "reg",3) == 0) {
	  sscanf(var,"reg%i",&i);
	  f1data->registers[i] = val;
	} 
      }
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
Int_t TVME_CATCH_TDC::F1Setup(F1Data_t* f1data){
  // Initialise the F1 TDC piggy back
  //  
  UInt_t val;
  UInt_t i;

  const Int_t p[] = {16 ,10, 15, 2, 3, 4, 5, 6, 8, 9, 11, 12, 13, 14, 0, 1, 7};
  
  /*
    xor = 0;
    for (i=1;i<16;i++) xor=xor^f1data->registers[i];
    //  do not use: reg0: all, reg1: bit5,10, reg7: bit 15, reg 15: bit4-12 
    xor = xor ^ (f1data->registers[1] & 0x420);
    xor = xor ^ (f1data->registers[7] & 0x8000);
    xor = xor ^ (f1data->registers[15] & 0x1FF0);
    xor = (xor>>12) ^ (xor % 0x1000);
    xor = (xor>>6)  ^ (xor % 0x40);
    xor = (xor>>3)  ^ (xor % 0x8);
    xor = (xor>>2) ^ ((xor % 4)>>1) ^ (xor % 2);
  */
  
  // add TDC address to setup data   tdc = 1
  for (i=0;i<16;i++)
    f1data->registers[i] = f1data->registers[i] + (( ( 1 << 4) + i) << 16);
  // unset beinit: (send first)
  f1data->registers[16] = 
    ((( 1 << 4) + 7) << 16) + (f1data->registers[7] & 0x7FFF);

  // Download data to CATCH->f1 */
  for (i=0;i<17;i++) {
    val = f1data->registers[p[i]];
    Write(ECATCH_IF1Setup, val);
    usleep(3); // seems to be important, or missing check status bit here? 
  }
  return 0;
}

ClassImp(TVME_CATCH_TDC)
