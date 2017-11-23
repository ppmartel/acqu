//--Author	JRM Annand	19th Nov 2007    For PCI-VMEbus AcquDAQ systems
//--Rev 	JRM Annand
//--Update	JRM Annand
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2LocalSource
// Specify local-node-read, e.g via a PCI-VME link, buffer for 
// AcquRoot analysis, ie make a link to TDAQperiment's TA2RingBuffer and
// make the link-TA2Ringbuffer available to TA2DataServer
// A wrapper for event saving in the RunIRQ procedure of TDAQexperiment
// which allows it to connect directly to the receiving procedure of
// Process/MultiProcess in TA2DataServer via a TA2RingBuffer. 
// TA2DataServer assumes that it connects to a data source via a derivative
// of TDataSource.
//
//---------------------------------------------------------------------------

#include "TA2LocalSource.h"
#include "ARFile_t.h"
#include "Semaphore_t.h"
#include "TDAQexperiment.h"


TA2LocalSource::TA2LocalSource( char* name, Int_t irecl,
			      Int_t save, Int_t rsize, Int_t swap ) :
  TA2DataSource( name, save, rsize )
{
  // Save data record lengths
  fInRecLen = irecl;
  fIsSwap = swap;
  fIsProcess = kTRUE;
}

TA2LocalSource::~TA2LocalSource()
{
  // delete local storage
}

//---------------------------------------------------------------------------
void TA2LocalSource::Process()
{
  // No processing done here
  PrintMessage("No processing to perform...returning\n\n");
}

//---------------------------------------------------------------------------
void TA2LocalSource::Initialise()
{
  // Initialise link to DAQ ring buffer of TDAQexperimenet
  fBuffer = new TA2RingBuffer( gDAQ->GetOutBuff() );
  if( !fInRecLen ) fInRecLen = fBuffer->GetLenBuff();
}

ClassImp(TA2LocalSource)
