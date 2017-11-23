//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand       5th Mar 2004   Use standard data buffer decode
//--Rev 	JRM Annand      22nd Apr 2004   TAPS MergeHeader
//--Rev 	JRM Annand       5th Nov 2006   nsrc
//--Update	JRM Annand...    3rd Jun 2008...const Char_t*...gcc 4.3
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
//	TA2TAPSFormat
//	TAPS data stream, 99 format for AcquRoot analysis
//
//---------------------------------------------------------------------------

#include "TA2TAPSFormat.h"
#include "TA2Mk1Format.h"
#include "TA2Mk2Format.h"

//	ClassImp.....for ROOT implementation of classes
//	Macro defined in $ROOTSYS/include/Rtypes.h


TA2TAPSFormat::TA2TAPSFormat(char* n, Int_t l, Int_t nsrc) :
  TA2DataFormat(n, l, nsrc)
{
  // Anything TAPS-format specific here...
  // Initial TAPS try doesn't manage data in well-defined buffer boundaries
  //  fIsBoundary = EFalse;
}

void TA2TAPSFormat::Initialise( void* head, TA2RingBuffer* sort,
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
  TA2Mk1Format* f = new TA2Mk1Format((Char_t*)"TAPSFormat",fRecLen,0);
  f->ConstructHeader(fHeader);
  delete f;
  
  AcquExptInfo_t* A = (AcquExptInfo_t*)((char*)fHeader + sizeof(UInt_t));
  strcpy(A->fDescription,"TAPS Data Format: No experimental description");
  strcpy(A->fRunNote,"TAPS Data Format: No run note");
  strcpy(A->fTime,"No time supplied");

  // Make a partial event buffer...just in case
  fPartEvent = new char[rin->GetLenBuff()];
}

//---------------------------------------------------------------------------
void TA2TAPSFormat::MergeHeader( void* h1, void* h2, Int_t orl, Int_t irl  )
{
  // Merge contents of header buffers, pointers h1, h2
  // into header buffer....h1 MUST previously have been created

  if( ! *(UInt_t*)h1 )
    PrintError("","<TAPS-Format data source must not be the Master Source>",
	       EErrFatal);
  void* h = new char[orl];
  memcpy( h, h1, irl );
  AcquExptInfo_t* p = (AcquExptInfo_t *)((UInt_t*)h + 1);
  AcquExptInfo_t* p1 = (AcquExptInfo_t *)((UInt_t*)h1 + 1);
  AcquExptInfo_t* p2 = (AcquExptInfo_t *)((UInt_t*)h2 + 1);
  // Counters for the new merged buffer
  p->fNmodule = p1->fNmodule + p2->fNmodule;
  ADCInfo_t* a = (ADCInfo_t*)(p + 1);
  ADCInfo_t* s = a + p->fNspect;
  ModuleInfo_t* m = (ModuleInfo_t*)(s + p->fNscaler);
  // Parameters of header 1
  ADCInfo_t* a1 = (ADCInfo_t*)(p1 + 1);
  ADCInfo_t* s1 = a1 + p1->fNspect;
  ModuleInfo_t* m1 = (ModuleInfo_t*)(s1 + p1->fNscaler);
  // Parameters of header 2
  ADCInfo_t* a2 = (ADCInfo_t*)(p2 + 1);
  ADCInfo_t* s2 = a2 + p2->fNspect;
  ModuleInfo_t* m2 = (ModuleInfo_t*)(s2 + p2->fNscaler);
  
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
}

ClassImp(TA2TAPSFormat)
