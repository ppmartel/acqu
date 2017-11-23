//--Author	JRM Annand   30th Sep 2005
//--Rev 	JRM Annand...
//--Update	JRM Annand...29th Apr 2009  Memory map listing
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQcontrol
// Characteristics of a control module
// Adds on to standard TDAQmodule
//

#include "TDAQcontrol.h"
#include "TDAQmodule.h"


//-----------------------------------------------------------------------------
TDAQcontrol::TDAQcontrol( TDAQmodule* mod )
{
  // Basic initialisation
  fMod = mod;
  fSlave = new TList();
  fSlaveADC = new TList();
  fSlaveBase = NULL;
  fNSlave = fNSlaveADC = 0;
  fNmap = 0;
  fMapList = NULL;
}

//-----------------------------------------------------------------------------
TDAQcontrol::~TDAQcontrol( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TDAQcontrol::AddSlave( TDAQmodule* slave )
{
  //
  if( !slave->InheritsFrom("TDAQmodule") ){
    fMod->PrintError("","<Invalid TDAQmodule class supplied>");
    return;
  }
  fSlave->AddLast( slave );
  if( slave->IsType( EDAQ_ADC ) ) fSlaveADC->AddLast( slave );
}

//-----------------------------------------------------------------------------
TDAQmodule* TDAQcontrol::GetSlave( Char_t* name )
{
  // Get module from list
  if( !fSlave ){
    fMod->PrintError("","<TDAQmodule list not initialised>");
    return NULL;
  }
  return (TDAQmodule*)fSlave->FindObject( name );
}

//-----------------------------------------------------------------------------
void* TDAQcontrol::SlaveADCRead( void** out )
{
  // Readout list of slave ADC's controlled by module
  TIter next( fSlaveADC );
  TDAQmodule* mod;
  while( ( mod = (TDAQmodule*)next() ) ) mod->ReadIRQ( out );
  return out;
}

//-----------------------------------------------------------------------------
Int_t TDAQcontrol::AddMap( DAQMemMap_t* map )
{
  // Add a slave memory mapping to the linked list

  if( !fMapList ) fMapList = new TList();
  fMapList->AddLast( map );
  fNmap++;
  return fNmap;
}
ClassImp(TDAQcontrol)
