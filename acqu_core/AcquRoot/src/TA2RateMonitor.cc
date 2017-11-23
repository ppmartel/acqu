//--Author	JRM Annand   27th Jun 2004
//--Rev 	JRM Annand...
//--Update	JRM Annand...27th Mar 2007  Decode divide-by-zero check
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2RateMonitor
// Time evolution of scalers counts or ratios of scaler counts
// e.g. scaler/clock-scaler = rate
//

#include "TA2RateMonitor.h"


//---------------------------------------------------------------------------
TA2RateMonitor::TA2RateMonitor( char* fname, UShort_t* adc, UInt_t* scaler,
				Double_t* scalersum, Bool_t* sflag ) :
  TA2System( fname, kRateDefaultKeys )
{
  // Setup the pattern from config file named fname
  // Pass the adc array, scaler array and sum-scaler array
  fADC = adc;
  fScaler = scaler;
  fScalerSum = scalersum;
  fIsScalerRead = sflag;
  fRateName = NULL;
  fNRate = fNrate = 0;
  fScalerIndex = NULL;
  fClockIndex = NULL;
  fFrequency = NULL;
  fNorm = NULL;
  fRate = NULL;
  FileConfig(fname);
  return;
}

//---------------------------------------------------------------------------
TA2RateMonitor::~TA2RateMonitor( )
{
  // Free memory allocated for pattern arrays

  if( fRateName ){ 
    for(Int_t i=0; i<fNRate; i++ )delete fRateName[i];
    delete fRateName;
  }
  if( fScalerIndex ) delete fScalerIndex;
  if( fClockIndex ) delete fClockIndex;
  if( fFrequency ) delete fFrequency;
  if( fNorm ) delete fNorm;
  if( fRate ) delete fRate;
}

//---------------------------------------------------------------------------
void TA2RateMonitor::SetConfig( char* line, int key )
{
  // Load basic multi-scaler or rate monitor parameters from file 
  // or command line
  char name[256];
  switch( key ){
  case ENRates:
    // Number of rate monitors to create
    if( sscanf( line, "%d", &fNRate ) != 1 ){
      PrintError(line,"<Number of rate-monitor spectra>");
      return;
    }
    fRateName = new Char_t*[fNRate];
    fScalerIndex = new Int_t[fNRate];
    fClockIndex = new Int_t[fNRate];
    fFrequency = new Int_t[fNRate];
    fNorm  = new Double_t[fNRate];
    fRate  = new Double_t[fNRate];
    break;
  case ERateData:
    // # of elements in individual hit pattern
    // check sufficient patterns declared to enter this one
    if( fNrate >= fNRate ){
      PrintError(line,"<Too many pattern spectra input>");
      return;
    }
    if( sscanf( line,"%s%d%d%d%lf", name, fScalerIndex+fNrate,
		fClockIndex+fNrate, fFrequency+fNrate, fNorm+fNrate ) != 5 ){
      PrintError(line,"<Rate parameter setup>");
      return;
    }
    fRateName[fNrate] = new Char_t[strlen(name) + 1];
    strcpy(fRateName[fNrate],name);
    fNrate++;
    break;
  default:
    // Pass on any unrecognised line
    PrintError(line,"<Unrecognised rate input command>");
    break;
  }
  return;
}
ClassImp(TA2RateMonitor)
