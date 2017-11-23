//--Author	JRM Annand    8th Feb 2007   Adapt from Mk1 format
//--Rev 	JRM Annand
//--Rev 	JRM Annand ...27th Apr 2007   Standard header-info interface
//--Rev 	JRM Annand ... 9th Jun 2007   Upgrade module & error struct
//--Rev 	JRM Annand ...21st Apr 2008   UpdateInfo(),ConstructHeader()
//--Rev 	JRM Annand ...31st Aug 2012   Mk2 module info fNScChannel
//--Rev 	JRM Annand ...21st Nov 2012   Stuff for merging data
//--Rev 	K Livingston.. 7th Feb 2013   Support for handling EPICS buffers
//--Rev 	JRM Annand ....1st Mar 2013   More stuff for merging data
//--Rev 	JRM Annand ....4th Mar 2013   EpicsHeaderInfo_t time_t to UInt_t
//--Rev 	JRM Annand ....6th Mar 2013   Fix TAPS(foreign) ConstructHeader
//--Update 	JRM Annand ...18th Apr 2013   Fix UpdateHeader()
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Mk2Format
// Specify Mk2 (new) ACQU formatted data stream
//
//---------------------------------------------------------------------------

#include "TA2Mk2Format.h"
#include "ARFile_t.h"
#include "CMakeConfig.h"


// Foreign data setup file keys
static const Map_t kForeignFormat[] = {
  {"Totals:",       EForeignTot},
  {"Module:",       EForeignModule},
  {NULL,          -1}
};

//---------------------------------------------------------------------------
TA2Mk2Format::TA2Mk2Format( Char_t* n, Int_t l, Int_t nsrc ) :
  TA2DataFormat(n, l, nsrc)
{
  // Flag that the data stream starts with a header buffer
  // Set offset to start of data in event....
  // 0th data source offset = 4 byte (include event number)
  // ith data source offset = 8 byte (exclude event number)
  fIsHeader = ETrue;
  if( nsrc ) fStartOffset = 12;
  else fStartOffset = 4;
  fIsErrorSort = ETrue;
}

//---------------------------------------------------------------------------
void TA2Mk2Format::CheckHeader( )
{
//  Check the header buffer which is read in standard ACQU format.
//  Check if its actually a header buffer and check if byte swapping
//  is necessary. This will override any command-line swap directive

  void* inbuff = fIn->GetStore();

  switch( *(UInt_t *)inbuff ){   // check the buffer is a header
  case EHeadBuff:			    // standard header
  case EHeadPhysBuff:		            // physical data header
    break;				// that's OK
  default:			// anything else is error
    PrintError("","<Invalid Header Buffer ID Marker>",EErrFatal);
  }
  // Check the record length tallies with what you think it is
  AcquMk2Info_t* P = (AcquMk2Info_t*)((char*)inbuff + sizeof(UInt_t));
  if(P->fRecLen == fRecLen){
    fIsSwap = EFalse;
  }
  else{
    fIsSwap = ETrue;			// Try swapping
    if((Int_t)Swap4Byte(P->fRecLen) != fRecLen){
      fprintf(fLogStream,"Header record length %d,...%d was specified\n",
	      P->fRecLen, fRecLen);
      PrintError("","<Non-matching record length in Header Buffer>",EErrFatal);
    }
    SwapHeader(P);
  }
//	Check filename only if offline. Online the remote machine makes it
//	if( !fOnline ) SetFileName(P,filename,run);
}

//---------------------------------------------------------------------------
void TA2Mk2Format::UpdateHeader( void* header, void* in )
{
  // Update "top" information in ACQU Mk1 header buffer
  // Ensure file name pointer set

  AcquMk2Info_t* oh = (AcquMk2Info_t*)((UInt_t*)header + 1);
  AcquMk2Info_t* ih = (AcquMk2Info_t*)((UInt_t*)in + 1);
  //  memcpy(oh, ih, sizeof(struct AcquMk2Info_t));
  // Selectively copy only parameters which should change
  oh->fRun = ih->fRun;
  strcpy(oh->fTime,ih->fTime);
  strcpy(oh->fRunNote,ih->fRunNote);
  strcpy(oh->fOutFile,ih->fOutFile);
  fDataFileName = oh->fOutFile;
}

//---------------------------------------------------------------------------
void TA2Mk2Format::SwapHeader(void* header)
{
// Byte-swap standard ACQU header buffer.
// This is complicated by the presence of string variables
// and so cannot be performed in a single loop operation
// Advance past the header marker 1st.

  header = (char*)header + sizeof(UInt_t);
  AcquMk2Info_t* A = (AcquMk2Info_t*)header;
  ModuleInfoMk2_t* M;			// -> DAQ hardware info
  Int_t* p1;				// general pointers
  Int_t* p2;

  // Main header 1st
  p1 = &(A->fRun);			// 1st non-string variable
  p2 = &(A->fRecLen);			// last non-string variable
  while(p1 <= p2){
    *p1 = Swap4Byte(*p1);		// 4-byte swap
    p1++;                               // next word
  }

  // DAQ hardware headers...p1 should already point to the 1st
  M = (ModuleInfoMk2_t*)p1;
  for(Int_t i=0; i<A->fNModule; i++){	// loop all modules
    p1 = &(M->fModIndex);		// -> 1st variable
    p2 = &(M->fBits);		        // -> last variable
    while(p1 <= p2){
      *p1 = Swap4Byte(*p1);
      p1++;
    }
    M++;				// -> next module
  }
}

//---------------------------------------------------------------------------
void TA2Mk2Format::MergeHeader( void* h1, void* h2, Int_t orl, Int_t irl  )
{
  // Merge contents of header buffers, pointers h1, h2
  // into new header buffer and then copy into h1
  // Update adc, scaler and module counters in header struct
  
  // Parameters of 1st header buffer
  AcquMk2Info_t* p1 = (AcquMk2Info_t *)((UInt_t*)h1 + 1);
  ModuleInfoMk2_t* m1 = (ModuleInfoMk2_t*)(p1 + 1);
  // Single stream
  if( ! *(UInt_t*)h1 ) memcpy( h1, h2, irl );
  // Multiple streams
  else{
    void* h = new Char_t[orl];        // temporary working space
    memcpy( h, h1, irl );

    // header struct pointers
    AcquMk2Info_t* p = (AcquMk2Info_t *)((UInt_t*)h + 1);
    AcquMk2Info_t* p1 = (AcquMk2Info_t *)((UInt_t*)h1 + 1);
    AcquMk2Info_t* p2 = (AcquMk2Info_t *)((UInt_t*)h2 + 1);

    // Counters for the new merged buffer
    if( p2->fNADC > p1->fNADC ) p->fNADC = p2->fNADC;
    //    p->fNScaler = p1->fNScaler + p2->fNScaler;
    if( p2->fNScaler > p1->fNScaler ) p->fNScaler = p2->fNScaler;
    p->fNModule = p1->fNModule + p2->fNModule;
    p->fNADCModule = p1->fNADCModule + p2->fNADCModule;
    p->fNScalerModule = p1->fNScalerModule + p2->fNScalerModule;

    // accumulate hardware module info...contains adc/scaler info
    ModuleInfoMk2_t* m = (ModuleInfoMk2_t*)(p + 1);
    ModuleInfoMk2_t* m2 = (ModuleInfoMk2_t*)(p2 + 1);
    Int_t i;
    for( i=0; i<p1->fNModule; i++ ) m[i] = m1[i];
    for( i=0; i<p2->fNModule; i++ ) m[i+p1->fNModule] = m2[i];
    memcpy( h1, h, orl );
    delete (char*)h;
  }
  fExpInfo = p1;
  fModInfo = m1;
  fDataFileName = p1->fOutFile;
}

//---------------------------------------------------------------------------
void TA2Mk2Format::ConstructHeader( void* h )
{
  // Initialisation for foreign data formats which supply no header info
  // This creates a (Mk1) Acqu-style header record using info read from file.
  // Info concerns mainly spec for ADCs and Scalers expected in the data
  //
  if( !h )h = fHeader;		// running input-buffer pointer
  AcquMk2Info_t* P;		// -> ACQU header struct
  ModuleInfoMk2_t* Pm;		// -> module info in header
  Char_t* pline;		// -> line from parameter file
  Char_t cbuff[8];		// for string manipulation
  Int_t nparm;			// no. of parameters read
  Int_t m;			// module loop counter
  Int_t key;                    // for file input
  
  // Primary initialisation of output buffer
  memset(h,0,(size_t)fRecLen);	// initialise the lot to zero
  *((UInt_t*)h) = EHeadBuff;	// Header ID marker
  h = (char*)h + sizeof(UInt_t);// past the ID marker
  P = (AcquMk2Info_t*)h;	// for writing header info
  P->fRecLen = fRecLen;         // record length
  Pm = (ModuleInfoMk2_t*)(P + 1);// move ptr past header
  
  // ADC info file name to be $acqu/data/XXX_setup.dat, where XXX
  // is the name of the particular calling (TNamed) class
  Char_t* fn = BuildName(ENV_OR_CMAKE("acqu",CMAKE_ACQU_USER),"/data/",(char*)GetName(),"Setup.dat");
  ARFile_t formatFile(fn, "r", this);
  delete fn;

  // Read and decode the parameter file
  // m is the non-comment line counter (and hence module counter)
  m = 0;
  Char_t name[16];
  Int_t dummy;
  while( (key = formatFile.ReadKey(kForeignFormat)) != -1 ){
    pline = formatFile.GetLine();
    switch( key ){
    case EForeignTot:
      nparm = sscanf(pline,"%d%d",&(P->fNADC), &(P->fNScaler));
      if( nparm != 2 )
	PrintError(pline,"<Foreign Data setup file format>",EErrFatal);
      // Initialise pointers to scaler and module sections of buffer
      // increment non-comment line counter, go to next loop
      P->fNADC++;
      break;
    case EForeignModule:
    //	One ADC/scaler module per line, 7 parameters read
      nparm = sscanf(pline,"%s%s%d%d%d%d%d",
		     cbuff,name,&(Pm->fAmin),&(Pm->fModIndex),&dummy,
		     &(Pm->fNChannel),&(Pm->fBits));
      if(nparm < 7)
	PrintError(pline,"<Foreign Data setup file format>",EErrFatal);
      // Flag the setup comes from config file, not from Acqu header in data
      // Set up module struct for each ADC/scaler
      // cbuff shows what kind of module
      Pm->fNChannel += 1;
      if(strcmp(cbuff,"adc") == 0) Pm->fModType = EDAQ_ADC;
      else Pm->fModType = EDAQ_Scaler;
      Pm++;				    // update module ptr
      m++;				    // update line/module
      break;
    default:
      PrintError(pline,"<Unrecognised Foreign-Format config line>");
      break;
    }
  }
  P->fNModule = m;			    // store no. modules
}

//---------------------------------------------------------------------------
void TA2Mk2Format::UpdateInfo(  )
{
  // Recalculate fModInfo pointer after multi-data source merging
  // probably not necessary for Mk2 format
  fModInfo = (ModuleInfoMk2_t*)(fExpInfo + 1);
}

ClassImp(TA2Mk2Format)
