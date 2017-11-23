//--Author	JRM Annand   17th May 2007  Test pseudo data
//--Rev 	JRM Annand
//--Rev 	JRM Annand   28th Nov 2007  WaitIRQ sleep(1) if DAQ not enabled
//--Rev 	JRM Annand   16th May 2011  Data > fMaxRand, set to 0
//--Update	JRM Annand   26th Nov 2012  Virtual event ID
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVirtualModule
// Generate randomly generated data to test data streams etc.
// OR read data from non-ACQU data bases into data stream
//

#include "TVirtualModule.h"
#include "TDAQexperiment.h"


enum { EVirtual_Random=100, EVirtual_DB };
static Map_t kVirtualKeys[] = {
  {"Virtual-Random:",     EVirtual_Random},
  {"Virtual-DB:",         EVirtual_DB},
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TVirtualModule::TVirtualModule( Char_t* name, Char_t* file, FILE* log, Char_t* line ):
  TDAQmodule( name, file, log )
{
  // Basic initialisation.
  // Read in base address, base readout index and # channels
  //
  Char_t type[32];
  AddCmdList( kVirtualKeys ); 
  if( line ){
    if( sscanf( line, "%*s%*s%*s%s%d%d%d",type,&fBaseIndex,&fNChannel,&fNBits )
	< 4 )
      PrintError( line, "<Virtual-Module setup line parse>", EErrFatal );
  }
  fType = GetModType(type);
  fBaseAddr = NULL;
  fBus = E_BusUndef;
  fRandFn = NULL;
  fChan = pow( 2, fNBits );
  fMinRand = 0;
  fMaxRand = (Int_t)fChan;
  fRandom = new TRandom();
  fEventID = 0;
}

//-----------------------------------------------------------------------------
TVirtualModule::~TVirtualModule( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVirtualModule::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  Char_t randFn[128];
  switch( key ){
  case EVirtual_Random:
    // Type of pseudo data (ADC, Scaler etc)
    if( sscanf(line,"%d%d%s", &fMinRand, &fMaxRand, randFn ) != 3 )
      PrintError( line, "<Virtual-Module setup line parse>", EErrFatal );
    fRandFn = BuildName( randFn );
    break;
  case EVirtual_DB:
    // If linking to data base e.g slow-control info to data stream
    break;
  default:
    // default try commands of TDAQmodule
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TVirtualModule::PostInit( )
{
  // Post-configuration setup of hardware
  // Write stored data to hardware
  //
  if( fIsInit ) return;
  TDAQmodule::PostInit();
}

//---------------------------------------------------------------------------
void TVirtualModule::ReadIRQ( void** outBuffer )
{
  // Generate random values and write to output data buffer
  //
  Double_t datum;
  UInt_t chan;
  UShort_t adcVal, adcIndex;                // adc value and index
  UInt_t scalerVal, scalerIndex;            // scaler value and index
  for( Int_t i=0; i<fNChannel; i++ ){
    datum = fChan * fRandom->Uniform(0,1);
    if( (datum < fMinRand) ){
      ErrorStore(outBuffer, 999);
      return;
    }
    else if (datum > fMaxRand ) datum = 0;
    chan = (Int_t)datum;
    if( (fType == EDAQ_ADC) && (datum > 0) ){
      adcVal = chan & 0xffff;                 // ADC value
      adcIndex = i + fBaseIndex;              // index offset
      ADCStore( outBuffer, adcVal, adcIndex );// store values
    }
    else if( fType == EDAQ_Scaler ){
      scalerVal = chan;
      scalerIndex = i + fBaseIndex;
      ScalerStore( outBuffer, scalerVal, scalerIndex );
    }
  }
}

//---------------------------------------------------------------------------
void TVirtualModule::WaitIRQ( )
{
  // Dummy wait for interrupt
  for(;;){
    if( fIsIRQEnabled ){       // "interrupt" enabled?
      usleep(10);
      break;
    }
    else sleep(1);
  }
}
//---------------------------------------------------------------------------
void TVirtualModule::ResetIRQ( )
{
  // Dummy reset interrupt
  usleep(10);
}

ClassImp(TVirtualModule)
