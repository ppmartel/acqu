//--Author	JRM Annand    9th Jan 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...30th Sep 2003...new private variables
//--Rev 	JRM Annand...15th Oct 2003   Generalise methods  
//--Rev 	JRM Annand... 5th Feb 2004   Cuts & filling 
//--Rev 	JRM Annand...27th May 2004   Time walk 
//--Rev 	JRM Annand...22nd Jun 2004   Raw hits and patterns
//--Rev 	JRM Annand...24th Aug 2004   Trig patt. to TA2DataManager
//--Rev 	JRM Annand...18th Oct 2004   log files & periodic tasks
//--Rev 	JRM Annand...18th Mar 2005   Bug GetMaxHits()
//--Rev 	JRM Annand... 7th Jun 2005   add fEnergyScale
//--Rev 	JRM Annand...11th Nov 2005   add coordinate system shift
//--Rev 	JRM Annand... 2nd Dec 2005   finalise coordinate system shift
//--Rev 	JRM Annand... 8th Sep 2006   "ALL" memory leaks, indexed walk
//--Rev 	JRM Annand...25th Oct 2006   Add some getter inline fn.
//--Rev 	JRM Annand...22nd Jan 2007   4v0 update
//--Rev 	JRM Annand...22nd Jun 2007   Some housekeeping
//--Rev 	JRM Annand...30th Oct 2008   4v3 Multi hit TDC processing
//--Rev 	JRM Annand... 3rd Dec 2008   Walk correction input
//--Rev 	JRM Annand... 1st Sep 2009   delete[], fNelem++...HitD2A
//--Rev 	JRM Annand...22nd Dec 2009   Cleanup bug fix
//--Rev 	DL Hornidge..27th Jan 2011   fixed array-termination bug
//--Rev 	JRM Annand... 6th Jul 2011   PostInit (A.Mushkarenkov)
//--Rev 	JRM Annand... 6th Oct 2012   DecodeBasic bug fix time hits
//--Rev 	JRM Annand... 7th Mar 2013   DecodeBasic filter out time = -1
//--Update	JRM Annand...23rd Apr 2013   Add GetEnergyScalePtr() (git)
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Detector
//
//	Base class for any detector system...cannot be instantiated
//	Specify set of procedures which user-defined derivative
//	must implement
//	User-defined detector classes must inherit from this one
//      Provides basis to digital to energy/time conversion
//

#include "TA2Detector.h"
#include "TA2Analysis.h"

ClassImp(TA2Detector)



//-----------------------------------------------------------------------------
TA2Detector::TA2Detector(const char* name, TA2System* apparatus,
			 const Map_t* commands, const Map_t* hists )
  :TA2HistManager( name, commands, hists)
{
  // Constructor...do not allocate "new" memory
  // Check passed pointer is to a TA2Apparatus class
  // Zero private variables prior to initialisationm

  if( !apparatus->InheritsFrom("TA2Apparatus") )
    fprintf(fLogStream," %s constructed with a non-TA2Apparatus pointer\n",
	   this->ClassName() );
  else fParent = (TA2DataManager*)apparatus;

  // Pointers to setup command/response maps
  fDetectorComm = commands;
  fDetectorHist = hists;

  // Zero some counters and array ptrs
  fNhits = fNhitsAll = fNADChits = fNTDChits = fNelement = fNelem = fMaxHits = 0;
  fTotalEnergy = 0.0;
  fEnergyScale = 1.0;
  fTimeOffset = 0.0;
  fElement = NULL;
  fEnergy = NULL;
  fEnergyOR = NULL;
  fTime = NULL;
  fTimeM = NULL;
  fTimeOR = NULL;
  fTimeORM = NULL;
  fTimeAll = NULL;
  fPosition = NULL;
  fMeanPos.SetXYZ(0.0,0.0,0.0);
  fShiftOp = NULL;
  fShiftValue = NULL;
  fNShiftOp = 0;
  fHits = NULL;
  fHitsM = NULL;
  fRawTimeHits = NULL;
  fRawEnergyHits = NULL;
  fIsECalib = fIsScaler = fIsTime = fIsEnergy = fIsPos = fIsRawHits = EFalse;
  fIsDecode = ETrue;
  fNMultihit = 0;
  fEnergyScale = 1.0;
  fTimeOffset = 0.0;
}

//-----------------------------------------------------------------------------
TA2Detector::~TA2Detector()
{
  // Free up allocated memory
  DeleteArrays();
}

//-----------------------------------------------------------------------------
void TA2Detector::SetupArrays( char* line )
{
  // Get # elements in detector and what is recorded per element
  UInt_t isenergy, istime, ispos, i;
  Int_t n = sscanf( line, "%d%d%d%d", &fNelement, &isenergy, &istime, &ispos );
  switch( n ){
  default:
    PrintError( line, "<Detector-Element-Setup>" );
    return;
  case 1:
    break;
  case 2:
    fIsEnergy = isenergy;
    break;
  case 3:
    fIsEnergy = isenergy;
    fIsTime = istime;
    break;
  case 4:
    fIsEnergy = isenergy;
    fIsTime = istime;
    fIsPos = ispos;    
  }
  // Create any necessary arrays
  fHits = new Int_t[fNelement];
  if( fIsEnergy | fIsTime | fIsPos ) fElement = new HitD2A_t*[fNelement];
  if( fIsPos ){
    fPosition = new TVector3*[fNelement];
    for( i=0; i<fNelement; i++ ) fPosition[i] = new TVector3(0,0,0);
  }
  if( fIsEnergy ){
    fEnergy = new Double_t[fNelement];
    for( i=0; i<fNelement; i++ ) fEnergy[i] = (Double_t)ENullADC;
    fEnergyOR = new Double_t[fNelement];
    fEnergyOR[0] = (Double_t)EBufferEnd;
  }
  if( fIsTime ){
    fTime = new Double_t[fNelement];
    for( i=0; i<fNelement; i++ ) fTime[i] = (Double_t)ENullADC;
    fTimeOR = new Double_t[fNelement];
    fTimeOR[0] = (Double_t)EBufferEnd;
    UInt_t max = fNelement*8;
    fTimeAll = new Double_t[max];
    fTimeAll[0] = (Double_t)EBufferEnd;
  }
}

//---------------------------------------------------------------------------
void TA2Detector::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Calculate mean position of detector
  // and apply any global corrdinate shifts

  TA2DataManager::PostInit();
  if( fIsPos ){
    // Mean position (centre) of detector
    UInt_t n;
    for( n=0; n<fNelem; n++ ) fMeanPos += *fPosition[n];
    fMeanPos *= 1.0/fNelem;
    // Apply any corrdinate shifts
    if( fNShiftOp ){
      for( n=0; n<fNelem; n++ ) ApplyShift( n );
    }
  }

}



//-----------------------------------------------------------------------------
void TA2Detector::DeleteArrays()
{
  // Free up memory allocated by "new" creation of arrays
  // Calibration structs
  if( fElement ){
    for( UInt_t i=0; i<fNelement; i++ ){
      delete fElement[i];
    }
    delete[] fElement;
  }    
  // Energy & Time
  if( fEnergy ){
    delete[] fEnergy;
    delete[] fEnergyOR;
  }
  if( fTime ){
    delete[] fTime;
    delete[] fTimeOR;
    delete[] fTimeAll;
  }
  // Hit Position
  if( fHits )   delete[] fHits;
  if( fPosition ){
    for( UInt_t i=0; i<fNelement; i++ ) delete fPosition[i];
    delete[] fPosition;
  }
  // multi-hit (TDC) stuff
  if( fNhitsM ){
    delete[] fNhitsM;
    for( UInt_t i=0; i<fNMultihit; i++ ) delete[] fHitsM[i];
    delete[] fHitsM;
    for( UInt_t i=0; i<fNMultihit; i++ ){
      delete[] fTimeM[i];
      delete[] fTimeORM[i];
    }
    delete[] fTimeM;
    delete[] fTimeORM;   // bug fix DLH 27.01.11
  }
  if( fShiftOp ) delete[] fShiftOp;
  if( fShiftValue ) delete[] fShiftValue;
  if( fRawEnergyHits ) delete[] fRawEnergyHits;
  if( fRawTimeHits ) delete[] fRawTimeHits;
  if( fDataCuts ) delete fDataCuts;
  if( fHistCuts ) delete fHistCuts;
}

//---------------------------------------------------------------------------
void TA2Detector::SetConfig( char* line, int key )
{
  // Load basic detector parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kDetectorKeys array at the top of the source .cc file
  // The following are setup...
  //	1. # Elements in the detector, is energy, time, position info input
  //	2. Calibration parameters for each element
  //	3. Any post initialisation
  //	4. Histograms for display...should be done after post-initialisation

  char pattname[256];

  switch( key ){
  case EDetectorSize:
    // # of elements in detector
    SetupArrays( line );
    break;
  case EDetectorElement:
    // parameters for each element...need to input # of elements 1st
    if( fNelem >= fNelement ){
      PrintError(line,"<Too many detector elements input>");
      break;
    }
    fElement[fNelem] = new HitD2A_t( line, fNelem, this ); fNelem++;
    break;
  case EDetectorAllElement:
    // set up all elements with the same parameters...diagnostic tool
    if( fNelem ){
      PrintError(line,"<Elements already entered...can't init all elements>");
      break;
    }
    for( fNelem=0; fNelem<fNelement;){
      fElement[fNelem] = new HitD2A_t( line, fNelem, this ); fNelem++;
    }
    break;
  case EDetectorTimeWalk:
    // time walk correction for individual detector element
    // the input line is expected to come immediately after the "element"
    // line...note fNelem is incremented after element setup hence fNelem-1
    UInt_t index;
    Double_t p1,p2;
    if( sscanf( line, "%d%lf%lf", &index, &p1, &p2 ) == 3 ){
      if( index >= fNelem ){
	PrintError( line, "<Indexed walk input, element not yet initialised>");
	break;
      }
      fElement[index]->SetWalk( line );
    }
    else PrintError( line, "<Parse error walk parameter input>" );
    break;
  case EDetectorAllTimeWalk:
    // set up all time walks with the same parameters...diagnostic tool
    for( UInt_t n=0; n<fNelement; n++)
      fElement[n]->SetWalk( line );
    break;
  case EDetectorRawHits:
    if( fIsTime ) fRawTimeHits = new Int_t[fNelement];
    if( fIsEnergy ) fRawEnergyHits = new Int_t[fNelement];
    fIsRawHits = ETrue;
    break;
  case EDetectorBitPattern:
    // Bit-pattern decode setup
    if( sscanf( line,"%s",pattname ) != 1 ){
      PrintError(line,"<Pattern File Name>");
      return;
    }
    fBitPattern = new TA2BitPattern(pattname,fADC);
    fPatternHits = fBitPattern->GetHits();
    fNPatternHits = fBitPattern->GetNHits();
    fIsBitPattern = ETrue;
    break;
  case EDetectorInit:
    // do some further initialisation
    PostInitialise();
    break;
  case EDetectorDisplay:
    // Histograms
    ParseDisplay(line);
    break;
  case EDetectorDataCut:
    // Conditions
    ParseCut(line);
    break;
  case EDetectorPeriod:
    // Periodic procedures frequency specify
    ParsePeriod( line );
    break;
  case EDetectorEndOfFile:
    // Turn on end-of-file procedures
    ParseEndOfFile( line );
    break;
  case EDetectorFinish:
    // Turn on end-of-run procedures
    ParseFinish( line );
    break;
  case EDetectorEnergyScale:
    // Global energy scaling factor
    if( sscanf( line, "%lf", &fEnergyScale ) != 1 )
      PrintError(line,"<Energy scale parse>");
    break;
  case EDetectorEnergyRandom:
    // Energy smearing
    if( sscanf( line, "%d", &fRandSeed ) != 1 )
      PrintError(line,"<Energy smear parse>");
    break;
  case EDetectorShiftCoord:
    // Read in global detector coordinate shift operations
    // Shift is performed at PostInit stage
    ParseShift( line );
    break;
  case EDetectorMultihit:
    // Set no. of multiple hits to record
    // Only if detector has time
    ParseMultihit( line );
    break;
   case EDetectorIgnoreElement:
    // Ignore a number of elements from read-out.
    UInt_t elem;
    Char_t tmp[256];
    if( sscanf( line, "%d", &elem ) != 1 ){
      PrintError(line,"<Ignore element parse>");
    }
    else
    {
      // check element index
      if( elem >= fNelement ){
        PrintError(line,"<Ignored detector element index out of range>");
        break;
      }
      fElement[elem]->SetIgnored(kTRUE);
      sprintf(tmp, "Ignoring detector element %d\n", elem);
      PrintMessage(tmp);
    }
    break;
  default:
    // Pass on any unrecognised line
    PrintError( line, "<Unrecognised Configuration keyword>" );
    break;
  }
  return;
}

//-----------------------------------------------------------------------------
void TA2Detector::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable
  // Start with any default data-manager variables
  TA2DataManager::LoadVariable(); 
  if( fIsEnergy ){
    TA2DataManager::LoadVariable("Energy",      fEnergy,        EDSingleX);
    TA2DataManager::LoadVariable("EnergyOR",    fEnergyOR,      EDMultiX);
    TA2DataManager::LoadVariable("Etot",        &fTotalEnergy,  EDSingleX);
    if( fIsRawHits ){
      TA2DataManager::LoadVariable("ADCHits",     fRawEnergyHits, EIMultiX);
      TA2DataManager::LoadVariable("NADChits",    &fNADChits,     EISingleX);
    }
  }
  if( fIsTime ){
    TA2DataManager::LoadVariable("Time",        fTime,          EDSingleX);
    TA2DataManager::LoadVariable("TimeOR",      fTimeOR,        EDMultiX);
    TA2DataManager::LoadVariable("TimeAll",     fTimeAll,       EDMultiX);
    if( fIsRawHits ){
      TA2DataManager::LoadVariable("TDCHits",     fRawTimeHits,   EIMultiX);
      TA2DataManager::LoadVariable("NTDChits",    &fNTDChits,     EISingleX);
    }
    if( fNMultihit ){
      // Ugly piece of coding....trying to construct histogram name in a loop
      // does not work...stick with brute force const chars for now
      // process up to 5 hits
      TA2DataManager::LoadVariable("NhitsM",      fNhitsM,        EISingleX);
      TA2DataManager::LoadVariable("HitsM0",      fHitsM[0],        EIMultiX);
      TA2DataManager::LoadVariable("TimeORM0",    fTimeORM[0],      EDMultiX); 
      TA2DataManager::LoadVariable("TimeM0",      fTimeM[0],        EDSingleX);
      if( fNMultihit > 1 ){ 
	TA2DataManager::LoadVariable("HitsM1",    fHitsM[1],        EIMultiX);
	TA2DataManager::LoadVariable("TimeORM1",  fTimeORM[1],      EDMultiX); 
	TA2DataManager::LoadVariable("TimeM1",    fTimeM[1],        EDSingleX);
      } 
      if( fNMultihit > 2 ){ 
	TA2DataManager::LoadVariable("HitsM2",    fHitsM[2],        EIMultiX);
	TA2DataManager::LoadVariable("TimeORM2",  fTimeORM[2],      EDMultiX); 
	TA2DataManager::LoadVariable("TimeM2",    fTimeM[2],        EDSingleX);
      } 
      if( fNMultihit > 3 ){ 
	TA2DataManager::LoadVariable("HitsM3",    fHitsM[3],        EIMultiX);
	TA2DataManager::LoadVariable("TimeORM3",  fTimeORM[3],      EDMultiX); 
	TA2DataManager::LoadVariable("TimeM3",    fTimeM[3],        EDSingleX);
      }   
      if( fNMultihit > 4 ){ 
	TA2DataManager::LoadVariable("HitsM4",    fHitsM[4],        EIMultiX);
	TA2DataManager::LoadVariable("TimeORM4",  fTimeORM[4],      EDMultiX); 
	TA2DataManager::LoadVariable("TimeM4",    fTimeM[4],        EDSingleX);
      }   
    }
  }
  TA2DataManager::LoadVariable("Hits",        fHits,          EIMultiX);
  TA2DataManager::LoadVariable("Nhits",       &fNhits,        EISingleX);    
  return;
}

//-----------------------------------------------------------------------------
void TA2Detector::ParseShift( Char_t* line )
{
  // Get global coordinate-shift variables from input line
  // A shift operation is characterised by a 2-char name and value
  // Up to 12 operations may input and the associated corrinate
  // transforms are performed in order they appear on the line
  // Allowed operations
  // Translate in X,Y or Z.....TX,TY,TZ by assocoaied value
  // Rotate about X,Y or Z.....RX,RY,RZ by associated value in deg.
  // Invert X,Y,Z..............IX,IY,IZ    (associated value ignored)
  // Rotate about mean pos
  // (Z only for now)     .....MX,MY,MZ by associated value in deg.

  const Char_t* opername[] = {
    "TX", "TY", "TZ", "RX", "RY", "RZ", "IX", "IY", "IZ", "MX", "MY", "MZ"
  };
  Int_t nname = sizeof(opername)/sizeof(Char_t*); // # allowed names
  Double_t vop[12];
  Char_t ops[48]; 
  Int_t nop = sscanf( line,
     "%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf",
		      ops,    vop,   ops+4,  vop+1, ops+8,  vop+2,
		      ops+12, vop+3, ops+16, vop+4, ops+20, vop+5,
		      ops+24, vop+6, ops+28, vop+7, ops+32, vop+8,
		      ops+36, vop+9, ops+40, vop+10,ops+44, vop+11 );
  fNShiftOp = nop/2;                        // # of operations
  fShiftOp = new Int_t[fNShiftOp];          // array of operation IDs
  fShiftValue = new Double_t[fNShiftOp];    // array of operation values
  for( Int_t i=0; i<fNShiftOp; i++ ){
    fShiftOp[i] = -1;                       // initialise ID undefined
    for( Int_t j=0; j<nname; j++ ){         // match with allowed names?
      if( strncmp( ops + 4*i, opername[j], 2 ) == 0 ){
	fShiftOp[i] = j;
	fShiftValue[i] = vop[i];
	break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void TA2Detector::ParseMultihit( Char_t* line )
{
  // Turn on analysis of multihit TDCs
  //
  if( !fIsTime ){
    PrintError(line,"< Multihit TDCs analysed only if detector has time >");
    return;
  }
  if( sscanf(line,"%d",&fNMultihit) != 1 ){
    PrintError(line,"< Parse error #multiple TDC hits to analyse >");
    return;
  }
  fNhitsM = new UInt_t[fNMultihit];
  fHitsM = new Int_t*[fNMultihit];
  for( UInt_t i=0; i<fNMultihit; i++ ) fHitsM[i] = new Int_t[fNelement];
  fTimeM = new Double_t*[fNMultihit];
  fTimeORM = new Double_t*[fNMultihit];
  for( UInt_t m=0; m<fNMultihit; m++ ){
    fTimeORM[m] = new Double_t[fNelement];
    fTimeM[m] = new Double_t[fNelement];
    fTimeORM[m][0] = (Double_t)EBufferEnd;
    // Bug fix DLH 27.01.11
    for( UInt_t i=0; i<fNelement; i++ ) fTimeM[m][i] = (Double_t)ENullADC;
  }
}

//-----------------------------------------------------------------------------
void TA2Detector::ApplyShift( Int_t i )
{
  // Perform series of coordinate transformations on position
  // vector of detector element. The transform operation IDs are specified
  // in the integer array fShiftOp. The numerical value to apply in the
  // transformation are given in double array fShiftValue

  TVector3* pos = fPosition[i];
  Double_t val;
  for(Int_t i=0; i<fNShiftOp; i++){
    switch( fShiftOp[i] ){
    case E_TX:
      // translate X, x = x + value
      val = pos->X() + fShiftValue[i]; pos->SetX( val );
      break;
    case E_TY:
      // translate Y, y = y + value
      val = pos->Y() + fShiftValue[i]; pos->SetY( val );
      break;
    case E_TZ:
      // translate Z, z = z + value
      val = pos->Z() + fShiftValue[i]; pos->SetZ( val );
      break;
    case E_RX:
      // Rotate about X axis
      pos->RotateX( fShiftValue[i]*TMath::DegToRad() );
      break;
    case E_RY:
      // Rotate about Y axis
      pos->RotateY( fShiftValue[i]*TMath::DegToRad() );
      break;
    case E_RZ:
      // Rotate about Z axis
      pos->RotateZ( fShiftValue[i]*TMath::DegToRad() );
      break;
    case E_IX:
      // Invert X
      val = -pos->X(); pos->SetX( val );
      break;
    case E_IY:
      // Invert Y
      val = -pos->Y(); pos->SetY( val );
      break;
    case E_IZ:
      // Invert Z
      val = -pos->Z(); pos->SetZ( val );
      break;
    case E_MX:
      // Not yet implemented
      break;
    case E_MY:
      // Not yet implemented
      break;
    case E_MZ:
      // Rotate about vector to mean position to detector
      pos->Rotate( fShiftValue[i]*TMath::DegToRad(), fMeanPos );
      break;
    default:
      break;
    }
  }
}
