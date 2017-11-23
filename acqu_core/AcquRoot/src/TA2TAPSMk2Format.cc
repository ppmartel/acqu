//--Author	JRM Annand	 6th Mar 2013   New class for Mk2 data
//--Rev     
//--Update 
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
//	TA2TAPSMk2Format
//	TAPS data stream, 99 format for AcquRoot Mk2 data format
//
//---------------------------------------------------------------------------

#include "TA2TAPSMk2Format.h"
#include "TA2Mk1Format.h"
#include "TA2Mk2Format.h"

//	ClassImp.....for ROOT implementation of classes
//	Macro defined in $ROOTSYS/include/Rtypes.h


TA2TAPSMk2Format::TA2TAPSMk2Format(char* n, Int_t l, Int_t nsrc) :
  TA2DataFormat(n, l, nsrc)
{
  // Anything TAPS-format specific here...
  // Initial TAPS try doesn't manage data in well-defined buffer boundaries
  //  fIsBoundary = EFalse;
}

void TA2TAPSMk2Format::Initialise( void* head, TA2RingBuffer* sort,
				TA2RingBuffer* rin, UInt_t mid )
{
//	Initialise input/output buffer pointers and
//	create any necessary intermediate buffers.
//	Call the base-class initialiser 1st

  TA2DataFormat::Initialise( head, sort, rin, mid );

  //  fT = (TapsExptHeader_t*)i;
  // -> header struct at start of each buffer

  // foreign data formats initialisation
  //  InitForeign();
  TA2Mk2Format* f = new TA2Mk2Format((Char_t*)"TAPSMk2Format",fRecLen,0);
  f->ConstructHeader(fHeader);
  delete f;
  
  AcquMk2Info_t* A = (AcquMk2Info_t*)((char*)fHeader + sizeof(UInt_t));
  strcpy(A->fDescription,"TAPS Data Format: No experimental description");
  strcpy(A->fRunNote,"TAPS Data Format: No run note");
  strcpy(A->fTime,"No time supplied");

  // Make a partial event buffer...just in case
  fPartEvent = new char[rin->GetLenBuff()];
}

//---------------------------------------------------------------------------
void TA2TAPSMk2Format::MergeHeader( void* h1, void* h2, Int_t orl, Int_t irl  )
{
  // Merge contents of header buffers, pointers h1, h2
  // into header buffer....h1 MUST previously have been created

  // Parameters of 1st header buffer
  //
  if( ! *(UInt_t*)h1 )
    PrintError("","<TAPS-Format data source must not be the Master Source>",
	       EErrFatal);
  void* h = new char[orl];
  memcpy( h, h1, irl );
  AcquMk2Info_t* p = (AcquMk2Info_t *)((UInt_t*)h + 1);
  AcquMk2Info_t* p1 = (AcquMk2Info_t *)((UInt_t*)h1 + 1);
  AcquMk2Info_t* p2 = (AcquMk2Info_t *)((UInt_t*)h2 + 1);
  // Counters for the new merged buffer
  if( p2->fNADC > p1->fNADC ) p->fNADC = p2->fNADC;
  if( p2->fNScaler > p1->fNScaler ) p->fNScaler = p2->fNScaler;
  p->fNModule = p1->fNModule + p2->fNModule;
  p->fNADCModule = p1->fNADCModule + p2->fNADCModule;
  p->fNScalerModule = p1->fNScalerModule + p2->fNScalerModule;
  
  // accumulate hardware module info...contains adc/scaler info
  ModuleInfoMk2_t* m = (ModuleInfoMk2_t*)(p + 1);
  ModuleInfoMk2_t* m1 = (ModuleInfoMk2_t*)(p1 + 1);
  ModuleInfoMk2_t* m2 = (ModuleInfoMk2_t*)(p2 + 1);
  Int_t i;
  for( i=0; i<p1->fNModule; i++ ) m[i] = m1[i];
  for( i=0; i<p2->fNModule; i++ ) m[i+p1->fNModule] = m2[i];
  memcpy( h1, h, orl );
  delete (char*)h;
  fExpInfo = p1;
  fModInfo = m1;
  fDataFileName = p1->fOutFile;
  //
  /*
  p->fNmodule = p1->fNmodule + p2->fNmodule;
  ADCInfo_t* a = (ADCInfo_t*)(p + 1);
  ADCInfo_t* s = a + p->fNspect;
  ModuleInfoMk2_t* m = (ModuleInfoMk2_t*)(s + p->fNscaler);
  // Parameters of header 1
  ADCInfo_t* a1 = (ADCInfo_t*)(p1 + 1);
  ADCInfo_t* s1 = a1 + p1->fNspect;
  ModuleInfoMk2_t* m1 = (ModuleInfoMk2_t*)(s1 + p1->fNscaler);
  // Parameters of header 2
  ADCInfo_t* a2 = (ADCInfo_t*)(p2 + 1);
  ADCInfo_t* s2 = a2 + p2->fNspect;
  ModuleInfoMk2_t* m2 = (ModuleInfoMk2_t*)(s2 + p2->fNscaler);
  UInt_t i;
  // Module info from 1 and 2
  for( i=0; i<p1->fNmodule; i++ ) m[i] = m1[i];
  for( i=0; i<p2->fNmodule; i++ ) m[i+p1->fNmodule] = m2[i];
  // ADC info from 1 and 2
  for( i=0; i<p1->fNspect; i++ ) a[i] = a1[i];
  for( i=0; i<p1->fNscaler; i++ )s[i] = s1[i];
  memcpy( h1, h, orl );
  fDataFileName = p1->fOutFile;
  delete (char*)h;
  */
}

ClassImp(TA2TAPSMk2Format)
