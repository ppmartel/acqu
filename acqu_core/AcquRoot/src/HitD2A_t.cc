//--Author	JRM Annand    2nd Oct 2003
//--Rev 	JRM Annand...12th Dec 2003 Multi-hit ADC's
//--Rev 	JRM Annand...18th Feb 2004 D2AM_t for Multi-hit ADC's
//--Rev 	JRM Annand...27th May 2004 Time walk
//--Rev 	JRM Annand...21st Oct 2005 Getter for cuts, global energy scale
//--Rev 	JRM Annand....8th Sep 2006 SetWalk check mem leak
//--Rev 	JRM Annand...30th Oct 2008 Multihit TDCs
//--Rev 	JRM Annand...29th Nov 2008 Remove D2A_t and Cut_t
//--Rev 	JRM Annand... 3rd Dec 2008 Time walk options
//--Rev 	JRM Annand... 1st Sep 2009 constructer no incr nelem
//--Rev 	JRM Annand...11th Oct 2012 add time over threshold
//--Rev 	JRM Annand....8th Nov 2012 init fA2, fT2 zero
//--Rev 	JRM Annand...27th Mar 2013 Incorporate Basle mods
//--Update	JRM Annand...23rd Apr 2013 Ensure all updates incorporated
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// HitD2A_t
// Digital-to-analogue conversion for a single detector element
// The element may have amplitude (ADC) and time (TDC) digitised
// readout. Window cuts are applied
//

#include "TA2Detector.h"
#include "HitD2A_t.h"
#include "TVector3.h"

//---------------------------------------------------------------------------
HitD2A_t::HitD2A_t(char* line, UInt_t nelem, TA2Detector* det , Bool_t ignorePosition)
{
  // Create assembly of conversion-gain calibration parameters
  // to convert digital ADC (QDC,VDC) and TDC numbers to
  // pulse amplitude (energy in MeV) and time (ns)

  Int_t iadc;                             // #adcs and tdcs
  Int_t madc;                             // multiplicity index for adc/tdc
  char adcstr[16],tdcstr[16];             // adc/tdc specification strings
  UShort_t* padc;                         // adc/tdc pointers
  Double_t xpos, ypos, zpos;
  UShort_t* adc = det->GetADC();
  Double_t* energyptr = det->GetEnergy();
  Double_t* timeptr = det->GetTime();
  Double_t** timeMptr = det->GetTimeM();
  TVector3** posptr = det->GetPosition();
  fEnergyScalePtr = det->GetEnergyScalePtr();
  fEnergyScale = det->GetEnergyScale();

  // Default "turn off" variables
  fIadc = fItdc = iadc = ENullADC;
  fADC = fTDC = fTDCtothr = NULL;
  fTDCM = NULL;
  fIsMultiADC = fIsMultiTDC = EFalse;
  fA0 = fA1 = fA2 = fT0 = fT1 = fT2 = 0.0;
  fWalk = NULL;
  fMode = 0;
  fNMultihit = 0;
  fTimeM = NULL;
  fIsIgnored = kFALSE;

  Int_t i = sscanf( line, "%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf",
		    adcstr, &fEnergyLowThr, &fEnergyHighThr, &fA0, &fA1,
		    tdcstr, &fTimeLowThr, &fTimeHighThr, &fT0, &fT1,
		    &xpos, &ypos, &zpos );

  // in case we have MC data, set fA1 to 1 and fT0 to 0
  if (gAR->GetProcessType() == EMCProcess) {
    fA1 = 1.;
    fT0 = 0.;
  }

  if( i < 10 ){
    // Error in input line...ignore it
    printf(" Error ignore create HitD2A from input line:\n %s\n",line);
    return;
  }
  // > 10 parameters read OK
  // Any ADCs ?
  if( det->IsEnergy() ){
    int n = sscanf(adcstr,"%d%*c%d",&iadc,&madc);
    // Normal ADC
    if( n == 1 )
      fADC = adc + iadc;
    // SADC (dynamic ped subtract)
    else if( n == 2 ){
      fADC = (UShort_t*)(det->GetMulti(iadc))->GetHitPtr(madc);
      fIsMultiADC = ETrue;
    }
    fIadc = iadc;
    fMode += 1;
  }
  // Any TDCs ?
  if( det->IsTime() ){
    int n = sscanf(tdcstr,"%d%*c%d",&iadc,&madc);
    if( n == 1 )
      // Single-hit TDC
      fTDC = adc + iadc;
    else if( n == 2 ){
      // Multihit TDC
      fIsMultiTDC = ETrue;
      // 1st hit only?
      if( !det->GetNMultihit() )
	fTDC = (UShort_t*)(det->GetMulti(iadc))->GetHitPtr(madc);
      // Analyse multiple hits
      else{
	fTDCM = new UShort_t*[det->GetNMultihit()];
	fTimeM = new Double_t*[det->GetNMultihit()];
	for( Int_t m=madc;; m++ ){
	  padc = (UShort_t*)(det->GetMulti(iadc))->GetHitPtr(m);
	  if( !padc ) break;
	  fTDCM[m-madc] = padc;
	  fTimeM[m-madc] = timeMptr[m-madc] + nelem; 
	  fNMultihit++;
	  if( fNMultihit >= (Int_t)det->GetNMultihit() ) break;
	}
	fTDC = fTDCM[0];
      }
    }
    fItdc = iadc;
    fMode += 2;
  }
  // Any position info
  if(!ignorePosition && det->IsPos() ){
    if( i < 13 ){
      printf(" Error ignore create HitD2A from input line:\n %s\n",line);
      return;
    }
    posptr[nelem]->SetXYZ(xpos, ypos, zpos); // position as read in
  }
  fEnergy = energyptr + nelem;
  fTime = timeptr + nelem;
  //  nelem++;                               // increment element counter
  return;
}

//---------------------------------------------------------------------------
HitD2A_t::~HitD2A_t( )
{
  // Free memory allocated to store calibration parameters

  if( fADC ){
    delete fADC;
  }
  if( fTDC ){
    delete fTDC;
  }
  if( fWalk ){
    delete fWalk;
  }
}

//---------------------------------------------------------------------------
void HitD2A_t::SetWalk( Char_t* line )
{
  // Store leading-edge walk correction parameters...optional
  // 3 walk algorithms available

  Double_t wp[6];
  Int_t n = sscanf(line, "%*d%lf%lf%lf%lf%lf%lf",wp,wp+1,wp+2,wp+3,wp+4,wp+5);
  switch( n ){
  case 2:
    // Basic algorithm
    fWalk = new TimeWalk_t();
    break;
  case 4:
    // Sergey Prakov's version
    fWalk = new TimeWalkSergey_t();
    break;
  case 6:
    // Sven Schuman's version
    fWalk = new TimeWalkSven_t();
    break;
  default:
    printf(" Error ignore HitD2A TimeWalk from input line:\n %s\n",line);
    return;
  }
  fWalk->SetWalk( wp );
}


void HitD2A_t::SetToThr( Char_t* line, TA2Detector* det )
{
  // Store Time-over-threshold TDC parameters
  Char_t tdcstr[16];
  Int_t n = sscanf(line, "%*d%s%lf%lf",tdcstr,&fToThr0,&fToThr1);
  if( n < 3 ){
    printf(" Error ignore HitD2A Time-over-Threshold input line:\n %s\n",line);
    return;
  }
  Int_t iadc, madc;
  n = sscanf(tdcstr,"%d%*c%d",&iadc,&madc);
  // Normal TDC
  if( n == 1 ){
    UShort_t* adc = det->GetADC();
    fTDCtothr = adc + iadc;
  }
  // Multihit TDC
  else if( n == 2 ){
    fTDCtothr = (UShort_t*)(det->GetMulti(iadc))->GetHitPtr(madc);
  }
}

