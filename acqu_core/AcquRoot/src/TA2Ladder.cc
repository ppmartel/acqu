//--Author	JRM Annand        12th Feb 2003
//--Rev 	JRM Annand...     26th Feb 2003...1st production version
//--Rev	        JRM Annand...      5th Feb 2004...3v8 compatibility
//--Rev 	K   Livingston... 20th Apr 2004...changed fLadder to fHits
//--Rev 	K   Livingston... 20th Apr 2004...PostInit() init fLaddDoubles
//--Rev 	K   Livingston... 16th Jun 2004...Prompt and rand timing wind
//--Rev 	K   Livingston... 25th Apr 2005...Multi random windows 10 max
//--Rev 	JRM Annand...     16th May 2005...ReadDecoded for Monter Carlo
//--Rev  	JRM Annand...     25th Jul 2005...SetConfig hierarchy
//--Rev 	JRM Annand...     26th Oct 2006...Adapt for TA2Detector upgrade
//--Rev 	JRM Annand...     17th Sep 2008...ReadDecoded & Decode doubles
//--Rev 	JRM Annand...     26th Aug 2009...Init fMuHits NULL
//--Rev 	JRM Annand...      1st Sep 2009...delete[], new HitD2A_t
//--Rev  	JRM Annand...     22nd Dec 2009...fDoubles MC
//--Update 	JRM Annand...      5th Sep 2012...ReadDecoded check iHit sensible
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Ladder
//
// "Linear" array of simple detector elements which provide hit
// timing and rates info. The hit-position is related to an energy scale
// e.g. as in focal-plane plastic scintillator array
// of the Glasgow-Mainz photon taggerTA2Ladder
// Expect this to be a high-rate device...so multiple hits.
//
// In general, a particle may go through a single element, or 2 overlapping 
// elements. Elements are defined as for TA2Detector, but have extra setup
// parameters relating to energy calibration and scalers.
// An element can have any of the following:
// QDC, TDC, Position, Energy, OverlapEnergy, Scaler
// For details, see the source code and the setupfile.

#include "TA2Ladder.h"
#include "TAcquFile.h"
#include "TA2Tagger.h"         // need this one for Monte Carlo decode

// constants for command-line maps below
enum { 
  ELaddMisc=100,
  ELaddTrig=101,
  ELaddInit=102,
  ELaddDisplay=103,
  ELaddWindow=104
};

static const Map_t kLaddKeys[]= {
  //  {"Size:",          EDetectorSize},
  //  {"Element:",       EDetectorElement},
  //  {"Initialise:",    ELaddInit},
  //  {"Display:",       ELaddDisplay},
  //  {"Data-Cut:",      EDetectorDataCut},
  {"Misc:",          ELaddMisc},
  {"Decode-Options:",ELaddMisc},
  {"Trigger:",       ELaddTrig},
  {"Window:",        ELaddWindow},
  {NULL,             -1}
};



//---------------------------------------------------------------------------
TA2Ladder::TA2Ladder( const char* name, TA2System* apparatus )
  :TA2Detector(name, apparatus)
{
  // Zero everything so that class is completely undefined
  // Everything must be explicitly defined

  fTrigger = NULL;
  fTrigg = fDoubles = fHitsAll = fWindows = NULL;
  fHitsRand = fHitsPrompt = fMuHits = NULL;
  fECalibration = fEelec = fEelecOR = fMeanEelecOR = fEOverlap = 
    fRandMin = fRandMax = NULL;
  fMeanTime = fMeanTimeOR = fMeanEnergy = fMeanEnergyOR = 
    fDiffTime = fDiffTimeOR = NULL;
  fTimingRes = fPromptRand = fPromptMin = fPromptMax = 0.0; 
  fNDoubles = fNThits = fNtrigger = fNtrig = 0;
  fNRandWindows = fNhitsPrompt = fNhitsRand  = fFence = 0;
  fIsOverlap = fIsTimeWindows = fIsFence = fIsMicro = EFalse;
  AddCmdList( kLaddKeys );                  // for SetConfig()

  fScalerPromptIndex = NULL;
  fScalerPromptCurr = NULL;
  fScalerPromptAcc = NULL;
  fScalerRandIndex = NULL;
  fScalerRandCurr = NULL;
  fScalerRandAcc = NULL;
}

//---------------------------------------------------------------------------
TA2Ladder::~TA2Ladder()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage

  if( fTrigger ){
    for( UInt_t i=1; i<fNtrig; i++ ) delete fTrigger[i];
    delete[] fTrigger;
  }
  if( fDoubles ) delete[] fDoubles;
  if( fECalibration ) delete[] fECalibration;
  if( fEWidth ) delete[] fEWidth;
  if( fEOverlap ) delete[] fEOverlap;
  if( fWindows ) delete[] fWindows;
  if( fEelec ) delete[] fEelec;
  if( fEelecOR  ) delete[] fEelecOR;
  if( fMeanEelecOR ) delete[] fMeanEelecOR;
  if( fHitsRand ) delete[] fHitsRand;
  if( fHitsPrompt ) delete[] fHitsPrompt;
  if( fMeanTime ) delete[] fMeanTime;
  if( fMeanTimeOR ) delete[] fMeanTimeOR;
  if( fMeanEnergy ) delete[] fEnergy;
  if( fMeanEnergyOR ) delete[] fEnergyOR;
  if( fDiffTime ) delete[] fDiffTime;
  if( fDiffTimeOR ) delete[] fDiffTimeOR;
  if( fTrigg ) delete[] fTrigg;
  if( fHitsAll ) delete[] fHitsAll;
  if( fScalerIndex ) delete[] fScalerIndex;
  if( fScalerCurr ) delete[] fScalerCurr;
  if( fScalerAcc ) delete[] fScalerAcc;
  if( fScalerPromptIndex ) delete[] fScalerPromptIndex;
  if( fScalerPromptCurr ) delete[] fScalerPromptCurr;
  if( fScalerPromptAcc ) delete[] fScalerPromptAcc;
  if( fScalerRandIndex )  delete[] fScalerRandIndex;
  if( fScalerRandCurr )  delete[] fScalerRandCurr;
  if( fScalerRandAcc )  delete[] fScalerRandAcc;
  if( fRandMin ) delete[] fRandMin;
  if( fRandMax ) delete[] fRandMax;
  if( fMuHits ) delete[] fMuHits;
  TA2Detector::DeleteArrays();
}

//-----------------------------------------------------------------------------
void TA2Ladder::LoadVariable(  )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup.
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  //                            name     	pointer         type-spec
  if(fIsTimeWindows){
    TA2DataManager::LoadVariable("HitsRand",   	fHitsRand,      EIMultiX);
    TA2DataManager::LoadVariable("HitsPrompt", 	fHitsPrompt,    EIMultiX);
  }
  if( fIsMicro ){
    TA2DataManager::LoadVariable("MuHits",   	fMuHits,        EIMultiX);
  }
  TA2DataManager::LoadVariable("NhitsPrompt",  	&fNhitsPrompt,  EISingleX);
  TA2DataManager::LoadVariable("NhitsRand",  	&fNhitsRand,    EISingleX);
  TA2DataManager::LoadVariable("NMuHits",  	&fNMuHits,      EISingleX);
  if( fIsScaler ){
    TA2DataManager::LoadVariable("ScalerCurr", 	fScalerCurr, 	EIScalerX);
    TA2DataManager::LoadVariable("ScalerAcc",  	fScalerAcc,  	EDScalerX);
  }
  if (fIsScaler ){
    TA2DataManager::LoadVariable("ScalerPromptCurr", fScalerPromptCurr, EIScalerX);
    TA2DataManager::LoadVariable("ScalerPromptAcc", fScalerPromptAcc, EDScalerX);
    TA2DataManager::LoadVariable("ScalerRandCurr", fScalerRandCurr, EIScalerX);
    TA2DataManager::LoadVariable("ScalerRandAcc", fScalerRandAcc, EDScalerX);
  }
  TA2DataManager::LoadVariable("NDoubles",	&fNDoubles,     EISingleX);
  if( fIsECalib ){
    TA2DataManager::LoadVariable("Eelect", 	fEelecOR,       EDMultiX);
    TA2DataManager::LoadVariable("Eelement", 	fEelec,       	EDSingleX);
  }
  TA2DataManager::LoadVariable("Trig", 		fTrigg,       	EIMultiX);
  TA2DataManager::LoadVariable("NTrig",		&fNThits,   	EISingleX);
  TA2DataManager::LoadVariable("Fence",    	&fFence,       	EISingleX);
  if( fIsOverlap && fIsTime ){
    TA2DataManager::LoadVariable("Doubles",	fDoubles,       EIMultiX);
    TA2DataManager::LoadVariable("MeanTime",   	fMeanTime,      EDSingleX);
    TA2DataManager::LoadVariable("MeanTimeOR", 	fMeanTimeOR,    EDMultiX);
    TA2DataManager::LoadVariable("DiffTime",   	fDiffTime,      EDSingleX);
    TA2DataManager::LoadVariable("DiffTimeOR", 	fDiffTimeOR,    EDMultiX);
  }
  if( fIsOverlap && fIsEnergy ){
    TA2DataManager::LoadVariable("MeanEnergy", 	fMeanEnergy,    EDSingleX);
    TA2DataManager::LoadVariable("MeanEnergyOR",fMeanEnergyOR,  EDMultiX);
  }
  //
  TA2Detector::LoadVariable();
}

//---------------------------------------------------------------------------
void TA2Ladder::SetConfig( char* line, int key )
{
  // Load Ladder parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kLaddKeys array at the top of the source .cc file
  // The following are setup...
  // Size:  element isenrgy istime  ispos (as for any detector)
  // Misc:  isscalr isecal  isoverl ntrig   Goffset TRes    QDCmin
  // Trigger:  adc low high a0 a1 tdc low high  a0   a1
  // Element: adc low high a0 a1 tdc low high a0 a1 x y z Ecent Eover scaler
  // Initialise: Any post initialisation
  // Disply: ...histogeams - should be done after post-initialisation

  Double_t calib,width, min, max;
  Int_t isscaler,isecalib,isoverlap,ismicro;
  Int_t scaler;

  // info elements, get the extra params, then do the default config
  if( key == EDetectorElement ){
    // first 10 params were already handled by TA2Detector::SetConfig()
    // here we deal with the extra params not handled by that
    if( fNelem >= fNelement ) goto error;
    if ( sscanf( line, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%lf%lf%d",
         &calib,&width,&scaler) != 3) goto error;
    if(fIsECalib) fECalibration[fNelem] = calib;
    fEWidth[fNelem] = width;
    if(fIsScaler) fScalerIndex[fNelem] = scaler;
    if(fIsScaler){ // read scaler indices for prompt and rand gates
      if(scaler<2800){
	fScalerPromptIndex[fNelem] = scaler + 96;
	fScalerRandIndex[fNelem] = scaler + 2*96;
      }
      if(scaler>2800){
	fScalerPromptIndex[fNelem] = scaler + 64;
	fScalerRandIndex[fNelem] = scaler + 2*64;
      }
    }
  }

  switch( key ){
  case ELaddMisc:
    // Optional extra decodes in ladder...
    // scalers, electron energy calib, overlap analysis, microscope overlap analysis
    // # differently times triggers, timing offset, and time coincidence resolution
    if( sscanf( line, "%d%d%d%d%d%lf%lf",
		&isscaler, &isecalib, &isoverlap, &ismicro, &fNtrigger, 
		&fTimeOffset, &fTimingRes ) != 7 ) goto error;
    fIsScaler = isscaler;
    fIsECalib = isecalib;
    fIsOverlap = isoverlap;
    fIsMicro = ismicro;

    fTrigger = new HitD2A_t*[fNtrigger+1];    //need at least one 
    if(fNtrigger==0) fTrigger[0] = NULL;      // make null if no defined trigs.
    if(fNelement){
      if( fIsECalib ){
	fECalibration = new Double_t[fNelement];
    fEWidth = new Double_t[fNelement];
    if( fIsOverlap ) fEOverlap = new Double_t[fNelement];
      }
      if( fIsScaler ){
	fScalerIndex = new UInt_t[fNelement];
	fScalerCurr = new UInt_t[fNelement];
	fScalerAcc = new Double_t[fNelement];
      }
      if( fIsScaler){
	fScalerPromptIndex = new UInt_t[fNelement];
	fScalerPromptCurr = new UInt_t[fNelement];
	fScalerPromptAcc = new Double_t[fNelement];
        fScalerRandIndex = new UInt_t[fNelement];
	fScalerRandCurr = new UInt_t[fNelement];
	fScalerRandAcc = new Double_t[fNelement];
      }
      if( fIsMicro ){
	fNMuElem = 2*(fNelement - 1) + 1;
	fMuHits = new Int_t[fNMuElem];
      }
    }
    break;
  case ELaddTrig:
    // trigger parameters...only needed if >1 trigger input
    // parameters are...adc index, low threshold, high threshold
    // TDC offset, TDC conversion gain
    if( fNtrig >= fNtrigger ) goto error;
    fTrigger[fNtrig] = new HitD2A_t( line, fNtrig, this ); fNtrig++;
    break;
    //  case ELaddInit:
  // do some further initialisation
    //    PostInitialise();
    //    break;
    //  case ELaddDisplay:
    //    ParseDisplay(line);
    //    break;
  case ELaddWindow:
    // set up prompt and random timing windows
    // check 1st if the arrays are defined
    if( !fRandMin ){
      fRandMin = new Double_t[EMaxRandWindows];
      fRandMax = new Double_t[EMaxRandWindows];
    }
    if( fNRandWindows >= EMaxRandWindows ){
      PrintError( line, "< Too many random wondows defined >" );
      break;
    }
    if( sscanf(line, "%lf%lf%lf%lf",
	       &min,&max,&fRandMin[fNRandWindows],&fRandMax[fNRandWindows])
	!= 4 ) goto error;
    if( !fNRandWindows ){
      fPromptMin = min;         // only store prompt vales from 1st line
      fPromptMax = max;
      fIsTimeWindows = ETrue;	// flag that windows are defined
    }
    fNRandWindows++;	        // increment the no of random windows
    break;
  default:
    // Try standard detector setup
    TA2Detector::SetConfig( line, key ); //do the default
    break;
  }
  return;
 error: PrintError( line );
  return;
}

//---------------------------------------------------------------------------
void TA2Ladder::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Start with alignment offsets
  // Create space for various output arrays


  UInt_t i;

  if( fIsOverlap ){
    for( i=0; i<fNelem-1; i++ ){
        fEOverlap[i]=(0.5*(fECalibration[i]+fECalibration[i+1])+0.25*(fEWidth[i]-fEWidth[i+1]));
    }
    if( fIsTime ){
      fMeanTime = new Double_t[fNelem];
      fMeanTimeOR = new Double_t[fNelem];
      fDiffTime = new Double_t[fNelem];
      fDiffTimeOR = new Double_t[fNelem];
      fDoubles = new UInt_t[fNelem];
      for( i=0; i<fNelem; i++ )
	fMeanTime[i] = fDiffTime[i] = (Double_t)ENullHit;
    }
    if( fIsEnergy ){
      fMeanEnergy = new Double_t[fNelem];
      fMeanEnergyOR = new Double_t[fNelem];
      for( i=0; i<fNelem; i++ )
	fMeanEnergy[i] = (Double_t)ENullHit;
    }
  }
  if( fIsECalib ){
    fEelec = new Double_t[fNelem];	//create required arrays
    fEelecOR = new Double_t[fNelem];
    fMeanEelecOR = new Double_t[fNelem];
  }
  if( !fNtrig ) fNtrig = 1;            	// at least 1 "trigger" loop
  fTrigg = new UInt_t[fNelem];
  fHitsAll = new UInt_t[fNelem];
  fWindows = new UInt_t[fNelem];
  if(fIsTimeWindows){ 	                // windows defined? prompt/rand ratio
    Double_t randTotal=0.0;
    for(UInt_t n=0;n<fNRandWindows;n++){
      randTotal+=(fRandMax[n]-fRandMin[n]);
    }
    fPromptRand=(fPromptMax-fPromptMin)/randTotal;
    fHitsRand = new Int_t[fNelem];
    fHitsPrompt = new Int_t[fNelem];
  }
  for( i=0; i<fNelem; i++ ){
    fTime[i]=fEelec[i]=(Double_t)ENullHit;
    for(UInt_t m=0; m<fNMultihit; m++) fTimeM[m][i] = (Double_t)ENullHit; 
  }
  fRandom = new TRandom();
  // Default cut list setup
  TA2DataManager::PostInit();
}

//---------------------------------------------------------------------------
void TA2Ladder::Decode( )
{
  // General basic detector decode procedure
  // Decode raw ADC, TDC and Scaler information into
  // Decode the ADC,TDC and Scaler information from each element
  // of the detector into "energy", "time" and hits arrays
  // Hit pattern, "Energy" pattern, aligned OR etc.
  // Adapted from TA2KensLadder::Decode() to extend TA2Detector functionality
  // Add Multihit TDC decoding

  fNADChits = fNTDChits = fNhits = fNhitsAll = 0;
  if( fHitsM ){
    for( UInt_t m=0; m<fNMultihit; m++ ) fNhitsM[m] = 0;
  }
  fTotalEnergy = 0.0;
  UInt_t i,j;
  HitD2A_t *trig,*elem;
  Double_t trigtime = 0.0;
  UShort_t datum;

  // If scaler event, and fIsScaler, fill scaler buffs
  // Histograms filled in general detector histogram loop
  if((fIsScaler)&&(gAR->IsScalerRead())){
    for(i=0;i<fNelem;i++){
      fScalerCurr[i] = fScaler[fScalerIndex[i]];
      fScalerAcc[i] = fScalerSum[fScalerIndex[i]];
    }
  }
  // Also fill Prompt and Rand scaler buffs
  if((fIsScaler)&&(gAR->IsScalerRead())){
    for(i=0;i<fNelem;i++){
      fScalerPromptCurr[i] = fScaler[fScalerPromptIndex[i]];
      fScalerPromptAcc[i] = fScalerSum[fScalerPromptIndex[i]];
      fScalerRandCurr[i] = fScaler[fScalerRandIndex[i]];
      fScalerRandAcc[i] = fScalerSum[fScalerRandIndex[i]];
    }
  }
  // loop over all triggers
  for( i=0; i<fNtrig; i++ ){
    if( (trig = fTrigger[i]) ){
      if( !trig->Calibrate() )
	continue;
      fNThits++;
      trigtime = trig->GetTime();
    }
    trigtime -= fTimeOffset;
    // loop over ladder hits
    for(j=0; j<fNelem; j++){
      elem = fElement[j];                   // element j characteristics
      if( fIsRawHits ){                     // diagnostic which defaults to OFF
	if( fIsEnergy ){
	  datum = elem->GetRawADCValue();
	  if( datum < (UShort_t)ENullStore )
	    fRawEnergyHits[fNADChits++] = j;// ADC hit pattern
	}
	if( fIsTime ){
	  datum = elem->GetRawTDCValue(); 
	  if( datum < (UShort_t)ENullStore )
	    fRawTimeHits[fNTDChits++] = j;  // TDC hit pattern
	}
      }
      if( !elem->Calibrate() ) continue;    // calibrated number within bounds 
      fTrigg[fNhits] = i;
      fHits[fNhits] = j;
      fHitsAll[fNhits] = j;
      if( fIsEnergy ){
	fTotalEnergy += elem->GetEnergy();
	fEnergyOR[fNhits] = elem->GetEnergy();
      }
      if( fIsTime ){
	fTime[j] -= trigtime;
	fTimeOR[fNhits] = fTime[j];
    if( fTimeM ){
	  for( Int_t m=0; m<elem->GetNhit(); m++ ){
	    fHitsM[m][fNhitsM[m]] = j;
	    fTimeM[m][j] -= trigtime; 
	    fTimeORM[m][fNhitsM[m]] = fTimeM[m][j];
	    fNhitsM[m]++;
	    fTimeAll[fNhitsAll++] = fTimeM[m][j];
	  }
	}
      }
      else fTimeAll[fNhitsAll++] = fTime[j];
      if( fIsECalib ){
	fEelecOR[fNhits] = fECalibration[j];
      }
      fNhits++;
    }
  }
  if( fIsRawHits ){
    if( fIsTime )fRawTimeHits[fNTDChits] = EBufferEnd;
    if( fIsEnergy ) fRawEnergyHits[fNADChits] = EBufferEnd;
  }

  fTrigg[fNhits] = EBufferEnd;

  fHitsAll[fNhits] = EBufferEnd;
  //fNhitsAll = fNhits;

  // Decode double hits
  DecodeDoubles();
  // Decode micro hits
  DecodeMicro();
  // Decode prompt and random coincidences if time and time windows defined
  DecodePrRand();
  //Dirk's fence plot
  if( fIsFence ) fFence = Fence();
  
  fHits[fNhits] = EBufferEnd;
  if(fIsTime){
    fTimeOR[fNhits] = (Double_t)EBufferEnd;
    fTimeAll[fNhitsAll] = (Double_t)EBufferEnd;
    for(UInt_t m=0; m<fNMultihit; m++){
      fHitsM[m][fNhitsM[m]] = EBufferEnd;
      fTimeORM[m][fNhitsM[m]] = EBufferEnd;
    }
  }
  if(fIsEnergy) fEnergyOR[fNhits] = (Double_t)EBufferEnd;
  if(fIsECalib) fEelecOR[fNhits] = (Double_t)EBufferEnd;
}

//---------------------------------------------------------------------------
void TA2Ladder::SaveDecoded( )
{
  // Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------
void TA2Ladder::ReadDecoded( )
{
  // Read photon beam parameters from GEANT simulation output
  // Convert to notional residual electron energy to be compatible with
  // what is expected by  TA2Tagger apparatus.
  // Set the energy according to the central point on the tagger
  // channel which holds the energy. 

  // A2 outputs photon energy in GeV...convert electron energy, MeV
  // Beam array...direction cosines followed by energy.
  fNhits = 0;
  fNDoubles = 0; 
  Float_t* energy = (Float_t*)(fEvent[EI_beam]);
  Double_t Ee = ((TA2Tagger*)fParent)->GetBeamEnergy() - 1000.0*energy[3];

  // Determine nearest channel with energy smaller than this energy
  UInt_t iHit = TMath::BinarySearch( fNelem, fECalibration, Ee );

  // Ensure that we do not go out of range, also handles the above returning -1
  if( iHit >= fNelem ) iHit = 0;

  Double_t centLo,centHi,nextLo;

  // If widths were set, use these to determine limits of the channel
  if( fEWidth[iHit] ){
      centLo = fECalibration[iHit] - 0.5*fEWidth[iHit];
      centHi = fECalibration[iHit] + 0.5*fEWidth[iHit];
  }
  // Otherwise set the limits as midpoints between this channel and its neighbors
  else{
      if( iHit > 0 ) centLo = 0.5*(fECalibration[iHit-1]+fECalibration[iHit]);
      else centLo = (fECalibration[iHit]-0.5*(fECalibration[iHit+1]-fECalibration[iHit]));
      if( iHit < (fNelem-1) ) centHi = 0.5*(fECalibration[iHit]+fECalibration[iHit+1]);
      else centHi = (fECalibration[iHit]+0.5*(fECalibration[iHit]-fECalibration[iHit-1]));
  }

  // Increment channel (without going out of range) if energy is beyond limit of this channel
  if( Ee > centHi && iHit < (fNelem-1) ){
      iHit++;

      // If widths were set, use these to determine limits of the channel
      if( fEWidth[iHit] ){
          centLo = fECalibration[iHit] - 0.5*fEWidth[iHit];
          centHi = fECalibration[iHit] + 0.5*fEWidth[iHit];
      }
      // Otherwise set the limits as midpoints between this channel and its neighbors
      else{
          if( iHit > 0 ) centLo = 0.5*(fECalibration[iHit-1]+fECalibration[iHit]);
          else centLo = (fECalibration[iHit]-0.5*(fECalibration[iHit+1]-fECalibration[iHit]));
          if( iHit < (fNelem-1) ) centHi = 0.5*(fECalibration[iHit]+fECalibration[iHit+1]);
          else centHi = (fECalibration[iHit]+0.5*(fECalibration[iHit]-fECalibration[iHit-1]));
      }
  }

  // Set limits for next channel to do double decoding
  // If widths were set, use these to determine limits of the channel
  if( iHit < (fNelem-1) && fEWidth[iHit] ) nextLo = fECalibration[iHit+1] - 0.5*fEWidth[iHit+1];
  else nextLo = centHi;

  // Fill central element if energy is within range, this check may seem extraneous but
  // it's necessary for cases outside of the range of the tagger, or for detectors like
  // the end-point tagger, which have gaps between detectors
  if( (Ee >= centLo) && (Ee < centHi) ){
      fHits[fNhits] = fHitsAll[fNhits] = iHit;
      if(fIsECalib) fEelecOR[fNhits] = fECalibration[iHit];
      if(fIsTime) fTimeOR[fNhits] = 0.0;
      if(fIsTime) fTimeAll[fNhits] = 0.0;
      fNhits++;

      // Fill next channel too for double hit
      if( Ee >= nextLo ){
          fHits[fNhits] = fHitsAll[fNhits] = iHit + 1;
          if(fIsECalib) fEelecOR[fNhits] = fECalibration[iHit+1];
          if(fIsTime) fTimeOR[fNhits] = 0.0;
          if(fIsTime) fTimeAll[fNhits] = 0.0;
          fNhits++;
      }
  }

  fNhitsAll = fNhits;

  DecodeDoubles();

  DecodePrRand();

  // Ensure some arrays properly terminated
  fHits[fNhits] = EBufferEnd;
  fHitsAll[fNhitsAll] = EBufferEnd;
  if(fIsECalib) fEelecOR[fNhits] = EBufferEnd;
  if(fIsTime) fTimeOR[fNhits] = EBufferEnd;
  if(fIsTime) fTimeAll[fNhitsAll] = EBufferEnd;
  return;
}

ClassImp(TA2Ladder)
