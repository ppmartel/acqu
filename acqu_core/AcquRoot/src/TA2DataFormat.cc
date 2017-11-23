//--Author	JRM Annand   12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand   22nd Apr 2004   Bug-fix header merge
//--Rev	        JRM Annand   12th Apr 2006   Add SetEventIndex(Int_t)
//--Rev 	JRM Annand    5th Nov 2006   fStartOffset, fNsrc
//--Rev 	JRM Annand...22nd Jan 2007...4v0 update
//--Rev 	JRM Annand...27th Apr 2007...Flexi header info getters
//--Rev 	JRM Annand...11th Jun 2007...Error sort flag
//--Rev 	JRM Annand...21st Apr 2008...UpdateInfo(),ConstructHeader()
//--Rev 	JRM Annand...21st Nov 2012...Stuff for Mk2 merging
//--Update	JRM Annand... 8th Feb 2013...Add GetNEPICS() prototype
//--Update  A Neiser...   6th June 2013  Make char_t* const
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
//	TA2DataFormat
//	Basis of all event-by-event data stream formats
//	recognised by AcquRoot for Data Serving purposes
//
//---------------------------------------------------------------------------

#include "TA2DataFormat.h"
#include "ARFile_t.h"

static const Map_t kForeignFormat[] = {
  {"Totals:",       EForeignTot},
  {"Module:",       EForeignModule},
  {NULL,          -1}
};


//---------------------------------------------------------------------------
TA2DataFormat::TA2DataFormat(const Char_t* n, Int_t l, Int_t nsrc) :
  TA2System(n,NULL)
{
  // Basic setup format class.
  // Zero unknown variables
  //
  fRecLen = l;                  // output record length
  fIsHeader = EFalse;	        // default no header buffer in data
  fDataFileName = NULL;	        // default no named data file
  fADCoffset = 0;               // default no offset to ADC numbers
  fIsBoundary = ETrue;          // default defined buffer boundaries
  fIsPartEvent = EFalse;        // default not a partial event
  fPartEvent = NULL;            // default partial events not seen
  fEventID = 0xffff;            // default no event ID marker
  fEventIndex = 0xffff;         // default no event ID marker
  fStartOffset = 4;             // default 4-byte header at start of event
  fNsrc = nsrc;                 // Data source #
  fIsErrorSort = EFalse;        // default no error sorting support
}

//-----------------------------------------------------------------------------
void TA2DataFormat::Initialise( void* head, TA2RingBuffer* sort, 
				       TA2RingBuffer* rin, UInt_t mid )
{
  // Store pointers to the header source  and data buffers
  // If the data format doesn't provide a header buffer
  // create space for a private header
  // Non-zero mid = fEventIndex means no event stamp in data
  //
  if( fIsHeader ) fHeader = head;
  else fHeader = new char[fRecLen];
  fOut = sort;
  fIn = rin;
  fEventIndex = mid;                // event marker "adc" index
}

//---------------------------------------------------------------------------
void TA2DataFormat::SetFileName( AcquExptInfo_t* A, const Char_t* filename, 
				 Int_t run )
{
  // Write the file name in an ACQU-formated header buffer.
  // Some options about the most appropriate name are tested
  //
  const Char_t* a;
  Int_t s = sizeof(A->fOutFile) - 1;	// max string length in header
  A->fOutFile[s] = '\0';	      	// ensure null termination

  // construct name from supplied filename
  if( (a = CheckName(filename)) ){	// try filename 1st
    strncpy(A->fOutFile, a, s);		// its OK, save it in header
    fDataFileName = A->fOutFile;       	// save ptr to file name string
    return;
  }

  // get name from header buffer
  if( (a = CheckName(A->fOutFile)) ){	// next try existing header
    char* b = new char[s];		// temporary buffer
    strncpy(b,a,s);			// b contains stripped fOutFile
    strncpy(A->fOutFile,b,s);		// save it
    delete[] b;
    fDataFileName = A->fOutFile;       	// save ptr to file name string
    return;
  }
  // default...construct name
  char runstr[8];
  sprintf(runstr,"%d",run);
  a = BuildName( "test", "_", runstr, ".dat" );
  strcpy(A->fOutFile, a);
  fDataFileName = A->fOutFile;		// save ptr to file name string
  return;
}

//---------------------------------------------------------------------------
void TA2DataFormat::InitForeign()
{
  // For foreign data formats.
  // Create various intermediate data buffers and initialise
  // an ACQU-format header
  //
  ConstructHeader();
}

//---------------------------------------------------------------------------
Bool_t TA2DataFormat::IsMaybeMk2() 
{
  UInt_t* marker = (UInt_t*)((char*)fHeader + sizeof(UInt_t));
  return *marker == EHeadBuff;
}

ClassImp(TA2DataFormat)
