//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand       5th Nov 2006   fStartOffset, nsrc
//--Rev 	JRM Annand      22nd Jan 2007   4v0 update
//--Rev 	JRM Annand      27th Apr 2007   Standard header interface
//--Rev 	JRM Annand      13th Jul 2007   Bug Mk1 merge header
//--Rev 	JRM Annand      29th Nov 2007   GetModNChan() returns fAmax + 1
//--Rev 	JRM Annand      12th Feb 2008   Check if module already ADC
//--Rev 	JRM Annand      21st Apr 2008   UpdateInfo(),ConstructHeader()
//--Update	JRM Annand       6th Sep 2008   UpdateInfo() m[j].fAmax
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Mk1Format
// Specify standard Mk1 (old) ACQU formatted data stream
//
//---------------------------------------------------------------------------

#include "TA2Mk1Format.h"
#include "ARFile_t.h"
#include "CMakeConfig.h"

// Foreign data setup file keys
static const Map_t kForeignFormat[] = {
  {"Totals:",       EForeignTot},
  {"Module:",       EForeignModule},
  {NULL,          -1}
};


//---------------------------------------------------------------------------
TA2Mk1Format::TA2Mk1Format( Char_t* n, Int_t l, Int_t nsrc ) :
  TA2DataFormat(n, l, nsrc)
{
  // Flag that the data stream starts with a header buffer
  // Set offset to start of data in event....
  // 0th data source offset = 4 byte (include event number)
  // ith data source offset = 8 byte (exclude event number)
  fIsHeader = ETrue;
  if( nsrc ) fStartOffset = 8;
  else fStartOffset = 4;
}

//---------------------------------------------------------------------------
void TA2Mk1Format::CheckHeader( )
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
  AcquExptInfo_t* P = (AcquExptInfo_t*)((char*)inbuff + sizeof(UInt_t));
  if(P->fRecLen == fRecLen){
    fIsSwap = EFalse;
  }
  else{
    fIsSwap = ETrue;			// Try swapping
    if(Swap2Byte(P->fRecLen) != fRecLen){
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
void TA2Mk1Format::UpdateHeader( void* header, void* in )
{
  // Update "top" information in ACQU Mk1 header buffer
  // Ensure file name pointer set

  AcquExptInfo_t* oh = (AcquExptInfo_t*)((UInt_t*)header + 1);
  AcquExptInfo_t* ih = (AcquExptInfo_t*)((UInt_t*)in + 1);
  Int_t bytes = (char*)&(oh->fNslave) - (char*)oh;
  memcpy(oh, ih, bytes);
  fDataFileName = oh->fOutFile;
}

//---------------------------------------------------------------------------
void TA2Mk1Format::SwapHeader(void* header)
{
// Byte-swap standard ACQU header buffer.
// This is complicated by the presence of string variables
// and so cannot be performed in a single loop operation
// Advance past the header marker 1st.

  header = (char*)header + sizeof(UInt_t);
  AcquExptInfo_t* A = (AcquExptInfo_t*)header;
  ModuleInfo_t* M;			// -> DAQ hardware info
  UShort_t* p1;				// general pointers
  UShort_t* p2;
  Int_t i;				// loop counter

  // Main header 1st
  p1 = &(A->fRun);			// 1st non-string variable
  p2 = &(A->fRecLen);			// last non-string variable
  while(p1 <= p2){
    *p1 = Swap2Byte(*p1);		// 2-byte swap
    p1++;                               // next word
  }

  // P_spect and P_scaler arrays..p1 should already point to the start
  // p2 points to the end
  p2 = p1 + ( sizeof(ADCInfo_t)/sizeof(UShort_t) * 
	      (A->fNspect + A->fNscaler) );
  while(p1 < p2){
    *p1 = Swap2Byte(*p1);	        // 2-byte swap
    p1++;                               // next P_spect
  }

  // DAQ hardware headers...p1 should already point to the 1st
  M = (ModuleInfo_t*)p1;
  for(i=0; i<A->fNmodule; i++){		// loop all modules
    p1 = &(M->fVicCrate);		// -> 1st non-string variable
    p2 = &(M->fBits);		        // -> last variable
    while(p1 <= p2){
      *p1 = Swap2Byte(*p1);
      p1++;
    }
    M++;				// -> next module
  }
}

//---------------------------------------------------------------------------
void TA2Mk1Format::MergeHeader( void* h1, void* h2, Int_t orl, Int_t irl  )
{
  // Merge contents of header buffers, pointers h1, h2
  // into new header buffer and then copy into h1
  // Update adc, scaler and module counters in header struct
  // 4v0 setup flexible interface to extract header info

  // Parameters of 1st header buffer
  AcquExptInfo_t* p1 = (AcquExptInfo_t *)((UInt_t*)h1 + 1);
  ADCInfo_t* a1 = (ADCInfo_t*)(p1 + 1);
  ADCInfo_t* s1 = a1 + p1->fNspect;
  ModuleInfo_t* m1 = (ModuleInfo_t*)(s1 + p1->fNscaler);
  Int_t i,j;
  // Single stream
  if( ! *(UInt_t*)h1 ){
    memcpy( h1, h2, irl );
    p1 = (AcquExptInfo_t *)((UInt_t*)h1 + 1);
    a1 = (ADCInfo_t*)(p1 + 1);
    s1 = a1 + p1->fNspect;
    m1 = (ModuleInfo_t*)(s1 + p1->fNscaler);
  }
  // Multiple streams
  else{
    void* h = new Char_t[orl];
    memcpy( h, h1, irl );

    // header struct pointers
    AcquExptInfo_t* p = (AcquExptInfo_t *)((UInt_t*)h + 1);
    AcquExptInfo_t* p2 = (AcquExptInfo_t *)((UInt_t*)h2 + 1);

    // Counters for the new merged buffer
    if( p2->fNspect > p1->fNspect ) p->fNspect = p2->fNspect;
    p->fNscaler = p1->fNscaler + p2->fNscaler;
    p->fNmodule = p1->fNmodule + p2->fNmodule;

    // accumulate hardware module info...contains adc/scaler info
    ADCInfo_t* a = (ADCInfo_t*)(p + 1);
    ADCInfo_t* s = a + p->fNspect;
    ModuleInfo_t* m = (ModuleInfo_t*)(s + p->fNscaler);
    // Parameters of header 2
    ADCInfo_t* a2 = (ADCInfo_t*)(p2 + 1);
    ADCInfo_t* s2 = a2 + p2->fNspect;
    ModuleInfo_t* m2 = (ModuleInfo_t*)(s2 + p2->fNscaler);
    
    // Module info from 1 and 2
    for( i=0; i<p1->fNmodule; i++ ) m[i] = m1[i];
    for( i=0; i<p2->fNmodule; i++ ) m[i+p1->fNmodule] = m2[i];
    // ADC info from 1 and 2
    for( i=0; i<p1->fNspect; i++ ) a[i] = a1[i];
    for( i=p1->fNspect; i<p2->fNspect; i++ ) a[i].fModIndex = 0xffff;
    for( i=0; i<p2->fNspect; i++ ){
      if( a2[i].fModIndex == 0xffff ) continue;
      if( i < p1->fNspect ){
	if( a1[i].fModIndex != 0xffff ){
	  PrintError("Merge-Header","<ADC numbering clash>");
	  continue;
	}
      }
      a[i] = a2[i];
      a[i].fModIndex += p1->fNmodule;
    }
    // Scaler info from 1 and 2
    for( i=0; i<p1->fNscaler; i++ )s[i] = s1[i];
    for( i=0; i<p2->fNscaler; i++ ){
      j = i + p1->fNscaler;
      s[j] = s2[i];
      s[j].fModIndex += p1->fNmodule;
    }
    memcpy( h1, h, orl );
    delete (char*)h;
    s1 = a1 + p1->fNspect;
    m1 = (ModuleInfo_t*)(s1 + p1->fNscaler);
  }
  fExpInfo = p1;
  fModInfo = m1;
  fDataFileName = p1->fOutFile;
  return;
}

//---------------------------------------------------------------------------
void TA2Mk1Format::ConstructHeader( void* h )
{
  // Initialisation for foreign data formats which supply no header info
  // This creates a (Mk1) Acqu-style header record using info read from file.
  // Info concerns mainly spec for ADCs and Scalers expected in the data
  //
  if( !h )h = fHeader;		// running input-buffer pointer
  AcquExptInfo_t* P;		// -> ACQU header struct
  ADCInfo_t* Padc;		// start of ADC info in header
  ADCInfo_t* Pscaler;		// start of scaler info in header
  //  ADCInfo_t* Ps;		// -> ADC/scaler info in header
  ModuleInfo_t* Pm;		// -> module info in header
  Char_t* pline;		// -> line from parameter file
  Char_t cbuff[8];		// for string manipulation
  Int_t nparm;			// no. of parameters read
  Int_t m;			// module loop counter
  Int_t key;                    // for file input
  
  // Primary initialisation of output buffer
  memset(h,0,(size_t)fRecLen);	// initialise the lot to zero
  *((UInt_t*)h) = EHeadBuff;	// Header ID marker
  h = (char*)h + sizeof(UInt_t);// past the ID marker
  P = (AcquExptInfo_t*)h;	// for writing header info
  P->fRecLen = fRecLen;         // record length
  h = (char*)(P + 1);		// move ptr past header
  Padc = (ADCInfo_t *)h;	// start ADC info
  
  // ADC info file name to be $acqu/data/XXX_setup.dat, where XXX
  // is the name of the particular calling (TNamed) class
  Char_t* fn = BuildName(ENV_OR_CMAKE("acqu",CMAKE_ACQU_USER),"/data/",(char*)GetName(),"Setup.dat");
  ARFile_t formatFile(fn, "r", this);
  delete fn;

  // Read and decode the parameter file
  // m is the non-comment line counter (and hence module counter)
  m = 0;
  while( (key = formatFile.ReadKey(kForeignFormat)) != -1 ){
    pline = formatFile.GetLine();
    switch( key ){
    case EForeignTot:
      nparm = sscanf(pline,"%hd%hd",&(P->fNspect), &(P->fNscaler));
      if( nparm != 2 )
	PrintError(pline,"<Foreign Data setup file format>",EErrFatal);
      // Initialise pointers to scaler and module sections of buffer
      // increment non-comment line counter, go to next loop
      P->fNspect++;
      Pscaler = Padc + P->fNspect;
      for( Int_t i=0; i<P->fNspect; i++ ) Padc[i].fModIndex = 0xffff;
      h = (char *)(Pscaler + P->fNscaler);
      Pm = (ModuleInfo_t*)h;
      break;
    case EForeignModule:
    //	One ADC/scaler module per line, 7 parameters read
      nparm = sscanf(pline,"%s%s%hd%hd%hd%hd%hd",
		       cbuff, Pm->fName, &(Pm->fCrate), &(Pm->fStation),
		       &(Pm->fAmin), &(Pm->fAmax), &(Pm->fBits));
      if(nparm < 7)
	PrintError(pline,"<Foreign Data setup file format>",EErrFatal);
      // Flag the setup comes from config file, not from Acqu header in data
      // Set up module struct for each ADC/scaler
      // cbuff shows what kind of module
      if(strcmp(cbuff,"adc") == 0) Pm->fVicCrate = EForeignADC;
      else Pm->fVicCrate = EForeignScaler;
      Pm++;				    // update module ptr
      m++;				    // update line/module
      break;
    default:
      PrintError(pline,"<Unrecognised Foreign-Format config line>");
      break;
    }
  }
  P->fNmodule = m;			    // store no. modules
}

//---------------------------------------------------------------------------
void TA2Mk1Format::UpdateInfo( )
{
  // Recalculate pointer to module info for multi data source operation
  // Check the old style module header and make compatible with new AcquRoot
  // Header element fVicCrate is used as the module type ID
  // Use ADCInfo_t arrays to confirm which modules are scalers or ADCs

  AcquExptInfo_t* p = fExpInfo;
  ADCInfo_t* a = (ADCInfo_t*)(p + 1);
  ADCInfo_t* s = a + p->fNspect;
  ModuleInfo_t* m = fModInfo = (ModuleInfo_t*)(s + p->fNscaler);
  Int_t i,j,k;

  // Subsequent stuff moved from MergeHeader
  // Set everything as "undefined" to begin with
  for( i=0; i<p->fNmodule; i++ ){
    switch( m[i].fVicCrate ){
    case EForeignADC:
    case EForeignScaler:
      break;
    default:
      (m[i]).fVicCrate = EDAQ_Undef;
      break;
    }
  }
  // Now check ADCs
  for( i=0; i<p->fNspect; i++ ){
    j = (a[i]).fModIndex;
    if( j >= p->fNmodule ) continue;
    (m[j]).fVicCrate = EDAQ_ADC;
    (m[j]).fAmin = i;
    //    if( m[j].fAmax )                 // remove 4v2.06.09.08
    //      i += (m[j]).fAmax;
    //   else{
    for( k=i; ;k++ ){                      // always work from a[k].fModIndex
      if( a[k].fModIndex == j ) continue;
      else{
	k--;
	if( (k-i) != m[j].fAmax ){ 
	  fprintf(fLogStream,"Resetting fAmax of module %d from %d to %d\n",
		  j, m[j].fAmax, k-i);
	  m[j].fAmax = k-i;
	}
	i = k;
	break;
      }
    }
    //    }
  }
  // Now check scalers
  for( i=0; i<p->fNscaler; i++ ){
    j = (s[i]).fModIndex;
    if( j >= p->fNmodule ) continue;
    if( m[j].fVicCrate == EDAQ_ADC )
      PrintError("Module is an ADC","<Label module Scaler>");
    else{
      (m[j]).fVicCrate = EDAQ_Scaler;
      i += (m[j]).fAmax;
    }
  }
}

ClassImp(TA2Mk1Format)
