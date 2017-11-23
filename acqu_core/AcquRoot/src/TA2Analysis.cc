//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Rev 	JRM Annand... 5th Feb 2004   Implement cuts
//--Rev 	JRM Annand...31st Mar 2004   Scaler plots
//--Rev 	JRM Annand...26th Jun 2004   Sum scaler zero
//--Rev 	JRM Annand...27th Jun 2004   Rate plots, trig patterns
//--Rev 	JRM Annand...24th Aug 2004   Rate,trig patt. to TA2DataManager
//--Rev 	JRM Annand...21st Oct 2004   Log file, Periodic, ROOT save/rd
//--Rev 	JRM Annand...17th Nov 2004   EOF, finish macros
//--Rev 	JRM Annand... 1st Dec 2004   generic apparatus, physics
//--Rev 	JRM Annand...24th Feb 2005   check if I/P file name changes
//--Rev 	JRM Annand... 3rd Mar 2005   batch-mode log-file directory
//--Rev 	JRM Annand...23rd Mar 2005   particle ID stuff
//--Rev 	JRM Annand...18th May 2005   reduced data save mods (scalers)
//--Rev 	JRM Annand...12th Apr 2006   Scalers() stops at fMaxScaler
//--Rev 	JRM Annand... 7th Jun 2006   Scalers() 4/line, fTreeFileName
//--Rev 	JRM Annand...22nd Jan 2007   4v0 changes
//--Rev 	JRM Annand...20th Jun 2007   Error analysis and display
//--Rev 	JRM Annand... 1st May 2008   InitSaveTree gAR->GetScalerBuff()
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand...25th Nov 2008...add GenericPhysics
//--Rev 	JRM Annand...29th Apr 2009...fix TTree branch format strings
//--Rev 	JRM Annand... 1st Sep 2009...delete[]
//--Rev 	JRM Annand...29th Sep 2012   Add histogram of pattern #hits
//--Update	K Livingston..7th Feb 2013   Support for handling EPICS buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Analysis
//
//	Main data analyser class to implement Online or Offline analysis
//	Take events worth of data from TAcquRoot and process in the
//	manner specified by the configuration files read in at the
//	initialisation stage.

#include "TA2Analysis.h"
#include "TA2Apparatus.h"
#include "TA2Detector.h"               // base detector class
#include "TAcquRoot.h"
#include "TA2GenericApparatus.h"
#include "TA2GenericPhysics.h"

//#include "TAcquFile.h"

// Keys for setting up various options
enum { 
  // Main setup command keys
  ERARaw, ERADecode, ERAReconstruct, ERAPhysics,
  ERAApparatus, ERADisplay, ERAInitialise, ERAFlashADC, ERAMultiADC,
  ERABitPattern, ERARateMonitor, ERAPeriod, ERAEpics, ERAEndOfFile, ERAFinish,
  ERASaveEvent,
  // Valid Apparatus keys
  EA2GenericApparatus,
  // Valid Physics keys
  EA2Physics, EA2GenericPhysics,
  // Histgram option keys
  ERAHistAll, ERAHistADC, ERAHist2DS, ERAHistScaler,
  ERAHistSumScaler, ERAHistADChits, ERAHistFShape, ERAHistFSum,
  ERAHistFPed, ERAHistFTail, ERAHistMulti, ERAHistRate, ERAHistPattern, ERAHistPatternNHits
};


// Valid Keywords for command-line setup of Acqu-Root analysis
// Must be NULL terminated
static const Map_t kRAKeys[] = {
  {"Raw-Analysis:",         ERARaw},
  {"Decode-Analysis:",      ERADecode},
  {"Reconstruct-Analysis:", ERAReconstruct}, 
  {"Physics-Analysis:",     ERAPhysics},
  {"Apparatus:",            ERAApparatus},
  {"Display:",              ERADisplay}, 
  {"Initialise:",           ERAInitialise},
  {"MultiADC:",             ERAMultiADC},
  {"FlashADC:",             ERAFlashADC},
  {"BitPattern:",           ERABitPattern},
  {"RateMonitor:",          ERARateMonitor},
  {"Period:",               ERAPeriod},
  {"Epics:",                ERAEpics},
  {"EndFile:",              ERAEndOfFile},
  {"Finish:",               ERAFinish},
  {"Save-Event:",           ERASaveEvent},
  {NULL,                    -1}
};

// Recognised apparatus/physics classes. Anything not in this table must be
// instantiated outside of TA2Analysis and then added
static const Map_t kKnownChild[] = {
  // Apparati
  {"TA2GenericApparatus",  EA2GenericApparatus},
  // Physics
  {"TA2GenericPhysics",    EA2GenericPhysics},
  {"TA2Physics",           EA2Physics},
  {NULL,             -1}
};

// Recognised display (histograming) options
static const Map_t kRAHist[] = {
  {"All",        ERAHistAll},
  {"ADC",        ERAHistADC},
  {"2D",         ERAHist2DS},
  {"Scaler",     ERAHistScaler},
  {"SumScaler",  ERAHistSumScaler},
  {"Rate",       ERAHistRate},
  {"Pattern",    ERAHistPattern},
  {"Pattern-N",  ERAHistPatternNHits},
  {"ADChits",	 ERAHistADChits},
  {"FlashShape", ERAHistFShape},
  {"FlashSum",   ERAHistFSum},
  {"FlashPed",   ERAHistFPed},
  {"FlashTail",  ERAHistFTail},
  {"MultiHit",   ERAHistMulti},
  {NULL,          -1}
};


//-----------------------------------------------------------------------------
TA2Analysis::TA2Analysis( const char* name )
  :TA2HistManager( name, kRAKeys, kRAHist )
{
  // Don't allocate any "new" memory here...Root will wipe it
  // Pass command string/key lists to TA2HistManager and TA2System.
  // Zero private class variables.
  // TA2HistManager and TA2DataManager do further setup

  //  fAppCuts = NULL;			// list of conditions
  fRawData = NULL;			// event pointer
  fFlash = NULL;                        // flash ADC list
  fMulti = NULL;                        // multi-hit ADC list
  fPrevFileName = NULL;                 // previous file name pointer
  fNhits = 0;				// # ADC hits
  kValidChild = kKnownChild;            // valid children list
  fPhysics = NULL;                      // physics class
  fIsPhysics = EFalse;
  fIsProcessComplete = EFalse;
  fNEvent = fNEvAnalysed = 0;           // event counters
  fParticleID = NULL;                   // PDG info

  Char_t* logfile;
  // check log output redirection for batch mode
  if( gAR->IsBatch() ){
    if( gAR->GetBatchDir() )
      logfile = BuildName(gAR->GetBatchDir(),"Analysis.log");
  }
  else logfile = BuildName("Analysis.log");
  if (gAR->IsLogFile()) SetLogFile(logfile);                  // output to log file
  else SetLogFile(NULL);
  delete[] logfile;

  fprintf(fLogStream," %s analysis linked to Acqu-to-Root interface\n\n",
	 this->ClassName() );

  // reserve some space for the array
  // should be somewhat larger
  // than the number of MultiADC hits
  // in one event
  fMultiADCIds.reserve(1000);
}


//-----------------------------------------------------------------------------
TA2Analysis::~TA2Analysis()
{
  // Free up allocated memory
}

//-----------------------------------------------------------------------------
void TA2Analysis::SetConfig(char* line, int key)
{
  // Set up internal parameters of TA2Analysis on the
  // basis of a text line read from file or tty
  //	Modes of analysis...raw, decode, reconstruct, physics
  //	Apparatus classes, histogramming options, initialisation

  Int_t n,n1,n2,n3,n4,n5,n6,n7,n8,n9;
  char name[256];

  switch( key ){
  case ERARaw:				// name of Root Tree
  case ERADecode:
  case ERAReconstruct:
    // Apparatus reconstruction
    fIsReconstruct = ETrue;
    PrintMessage(" Reconstruction procedure enabled in event loop\n");
    break;
  case ERAPhysics:
    // Physics reconstruction....special child not added to children list
    // Only works for "known" physics types
    AddChild( line, (TA2DataManager**)(&fPhysics) );
    if( !fPhysics ) return;
    fIsPhysics = ETrue;
    PrintMessage(" Physics procedure enabled in event loop\n");
    break;
  case ERAApparatus:
    // Add an apparatus to the fChildren list
    // This works here only for "known" apparatis...see kKnownChild[] above
    // Overwise create apparatus externally and then AddChild()
    AddChild( line );
    break;
  case ERADisplay:
    // General display setup
    ParseDisplay( line );
    break;
  case ERAInitialise:
    // Initialisation
    PostInitialise();
    break;
  case ERAMultiADC:
    // Multi-hit ADC (Usually TDC or SG-ADC) handler
    if( !fIsInit ){
      PrintMessage(" Running PostInitialise before setting up Multi-Hit ADC's\n\n");
      PostInitialise();
    }
    n = sscanf(line,"%d%d%d%d%d%d",&n1,&n2,&n3,&n4,&n5,&n6);
    switch( n ){
    default:
      // Format fault in input line
      PrintError("Multi-hit ADC", line); return;
    case 2:
      // Single standard Multi-hit ADC
      n3 = n2; n4 = 0; break;
    case 3:
      // Multiple standard Multi-hit ADC
      n4 = 0; break;
    case 4:
      // Multiple Catch TDC's....need to subtract a reference value
      n5 = 0; break;
    case 6:
      break;
    }
    if( !fMulti ){
      fMulti = new MultiADC_t*[fMaxADC];
      for( n=0; n<fMaxADC; n++ ) fMulti[n] = NULL;
      gAR->SetMulti( fMulti );
    }
    for( n=n2; n<=n3; n++ ){
      if( n >= fMaxADC )goto SetErr;
      if( !n4 ) fMulti[n] = new MultiADC_t( n, n1 );
      else if( !n5 ) fMulti[n] = new CatchTDC_t( n, n1, fADC+n4);
      else fMulti[n] = new SG3sumADC_t( n, n4, n5, n6 );
      fADCdefined[n] = EMultiADC;
    } 
    break;
  case ERAFlashADC:
    // Flash ADC handler
    if( !fIsInit ){
      PrintMessage("Running PostInitialise before setting up Flash ADC's\n\n");
      PostInitialise();
    }
    if( (n = sscanf(line,"%d%d%d%d%d%d%d%d%d",
		    &n1,&n2,&n3,&n4,&n5,&n6,&n7,&n8,&n9)) < 8 )
      goto SetErr;
    if( n < 9 ) n9 = n8;
    if( !fFlash ){
      fFlash = new FlashADC_t*[fMaxADC];
      for( n=0; n<fMaxADC; n++ ) fFlash[n] = NULL;
    }
    for( n=n8; n<=n9; n++ ){
      if( n >= fMaxADC )goto SetErr;
      fFlash[n] = new FlashADC_t( n, n1, n2, n3, n4, n5, n6, n7 );
      fADCdefined[n] = EFlashADC;
    } 
    break;
  case ERABitPattern:
    // Bit Pattern setup
    if( sscanf( line,"%s",name ) != 1 ){
      PrintError(line,"<Bit-pattern File Name>");
      return;
    }
    fBitPattern = new TA2BitPattern(name,fADC);
    fIsBitPattern = ETrue;
    break;
  case ERARateMonitor:
    // Continuous rates monitor setup
    if( sscanf( line,"%s",name ) != 1 ){
      PrintError(line,"<Rate-Monitor File Name>");
      return;
    }
    fRateMonitor = new TA2RateMonitor(name,fADC,fScaler,fScalerSum,
				     gAR->GetScalerRead());
    fIsRateMonitor = ETrue;
    break;
  case ERAPeriod:
    // Periodic procedures frequency specify
    ParsePeriod( line );
    break;
  case ERAEpics:
    // Periodic procedures frequency specify
    ParseEpics( line );
    break;
  case ERAEndOfFile:
    // Turn on end-of-file procedures
    ParseEndOfFile( line );
    break;
  case ERAFinish:
    // Turn on end-of-run procedures
    ParseFinish( line );
    break;
  case ERASaveEvent:
    // Enable data saving in Tree format
    SetSaveEvent();
    break;
  default:
    // Unrecognised command option
    fprintf(fLogStream," %s configuration: ignored unrecognised line\n%s\n",
	   this->ClassName(), line);
    break;
  }
  return;
  // General purpose error handler
  SetErr: PrintError(line );
  return;
}

//-----------------------------------------------------------------------------
void TA2Analysis::ParseDisplay( Char_t* line )
{
  // Pull out histogram information from "Display:" command line
  // Check initialisation completed first...otherwise this will fail.
  // RootAnalysis (raw ADC/Scaler analysis) can histogram the following:
  //	1. Experimental ADC's, all of them or specified subsets.
  //	   Optionally use default (from Acqu data base) channel limits
  //	   or set them from the command line
  //	2. ADC hit multiplicity
  //	3. 2D spectra...one ADC against the other.
  //	   Must supply channel limits for both x and y
  //	4. Single scalers...one histogram channel for each read
  //	5. Accumulated values of a set of scalers
  //    6. Flash ADC pulse form.
  //    7. Flash ADC sum etc.
  // After working out what's desired, the generic Setup1D() or Setup2D()
  // functions are called.

  if( !fIsInit ){
    PrintMessage(" TA2Analysis: running PostInitialise before display setup\n\n");
    PostInitialise();
  }

  Int_t j,k;					// general indices
  Int_t n1,n2;                     		// ADC indices
  Char_t n1str[16],n2str[16];                   // ADC spec string
  Int_t l1,l2,h1,h2;	  			// histogram limits
  Int_t ch1,ch2;        			// histogram channels
  Int_t m1,m2;                                  // multi-hit index
  Char_t histspec[EMaxName];			// contains 1/2D spec.
  Char_t* nhName;                               // for pattern #hits histogram

  if( sscanf(line, "%s", histspec) != 1 ){
    PrintError(line,"<Display line format>");
    return;
  }
  k = Map2Key( histspec, kRAHist );
  switch( k ){
  case ERAHistAll:
  case ERAHistADC:
  case ERAHistFShape:
  case ERAHistFSum:
  case ERAHistFPed:
  case ERAHistFTail:
  case ERAHistMulti:
    // Ensure derived-class ParseADC() doesn't take over
    TA2Analysis::ParseADC( line, k );
    break;
  case ERAHist2DS:
    // Histogram one ADC against another in a 2D plot
    // Must specify channel numbers and limits for each ADC
    j = sscanf(line, "%*s%s%d%d%d%s%d%d%d",
	       n1str,&ch1,&l1,&h1,n2str,&ch2,&l2,&h2);
    if( j != 8 ){
      PrintError(line,"<Display line format>");
      return;
    }

    sprintf(histspec,"TwoD%sv%s %d %d %d %d %d %d",
	    n1str,n2str,ch1,l1,h1,ch2,l2,h2 );
    j = sscanf(n1str,"%d%*c%d",&n1,&m1);
    k = sscanf(n2str,"%d%*c%d",&n2,&m2);
    if( (j == 1) && (k == 1) ) Setup2D( histspec, &fADC[n1], &fADC[n2] );
    else if( (j == 2) && (k == 1) )
      Setup2D( histspec, fMulti[n1]->GetHitPtr(m1), &fADC[n2] );
    else if( (j == 1) && (k == 2) )
      Setup2D( histspec, &fADC[n1], fMulti[n2]->GetHitPtr(m2) );
    else if( (j == 2) && (k == 2) )
      Setup2D( histspec, fMulti[n1]->GetHitPtr(m1),fMulti[n2]->GetHitPtr(m2) );
    break;
  case ERAHistScaler:
    // Single-read scalers
    if( ((j = sscanf(line, "%*s%d%d",&l1,&h1)) != 2) || (h1 > fMaxScaler ) ){
      PrintError(line,"<Scaler Display line format>");
      return;;
    }
    sprintf(histspec,"Scalers%dto%d %d 0 0",l1,h1,(h1-l1+1));
    Setup1D( histspec, fScaler+l1, EHistScalerX );
    break;
  case ERAHistSumScaler:
    // Scaler sums
    if( ((j = sscanf(line, "%*s%d%d",&l1,&h1)) != 2) || (h1 > fMaxScaler ) ){
      PrintError(line,"<SumScaler Display line format>");
      return;;
    }
    sprintf(histspec,"SumScalers%dto%d %d 0 0",l1,h1,(h1-l1+1));
    Setup1D( histspec, fScalerSum+l1, EHistScalerX );
    break;
  case ERAHistPattern:
    // Scalers as a function of time
    // l1 rate-mon index, ch1 #chan in spectrum
    if( (j = sscanf(line,"%*s%d%d%d%d",&l1,&ch2,&l2,&h2)) != 4 ){
      PrintError(line,"<Bit pattern display line format>");
      return;;
    }
    sprintf(histspec,"%s %d %d %d",fBitPattern->GetPatternName(l1),ch2,l2,h2);
    Setup1D( histspec, fBitPattern->GetHits(l1), EHistMultiX );
    break;
  case ERAHistPatternNHits:
    // No of hits in the pattern spectrum
    if( (j = sscanf(line,"%*s%d%d%d%d",&l1,&ch2,&l2,&h2)) != 4 ){
      PrintError(line,"<Bit pattern NHits display line format>");
      return;;
    }
    nhName = BuildName(fBitPattern->GetPatternName(l1),"_NHits");
    sprintf(histspec,"%s %d %d %d",nhName,ch2,l2,h2);
    Setup1D( histspec, fBitPattern->GetNHits(l1) );
    delete nhName;
    break;
  case ERAHistRate:
    // Scalers as a function of time
    // l1 rate-mon index, ch1 #chan in spectrum
    if( (j = sscanf(line,"%*s%d%d",&l1,&ch1)) != 2 ){
      PrintError(line,"<Rate Display line format>");
      return;;
    }
    sprintf(histspec,"%s %d %d 0",fRateMonitor->GetRateName(l1),ch1,
	    fRateMonitor->GetFrequency(l1));
    Setup1D( histspec, (fRateMonitor->GetRate()+l1), EHistRateX );
    break;
  case ERAHistADChits:
    j = sscanf(line, "%*s,%d%d%d",&ch1, &l1, &h1);
    if( j != 3 ){
      ch1 = fMaxADC; l1 = 0; h1 = ch1 - 1;
    }
    sprintf(histspec,"ADC-Hits %d %d %d", ch1, l1, h1);
    Setup1D(histspec, &fNhits);
    break;
  default:
    // Try parent TA2Histogram display parsing
    TA2HistManager::ParseDisplay( line ); 
    return;
  }
  fIsDisplay = ETrue;
  return;
}

//-----------------------------------------------------------------------------
void TA2Analysis::LoadVariable( )
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
  //
  TA2DataManager::LoadVariable("NHardwareError",&fNHardwareError,  EISingleX);
  TA2DataManager::LoadVariable("HardwareError", fHardwareError,    EIMultiX);
  return;
}

//-----------------------------------------------------------------------------
void TA2Analysis::ParseADC( char* line, Int_t opt )
{
  // Histogram defined ADC's, according to their definition...
  //  1. Undefined (from Acqu data base info)...skip it.
  //  2. Pattern unit...will do later
  //  3. Multi-hit ADC, often a TDC, choose which hit 1st, 2nd etc to histogram
  //  4. Flash ADC choose one of the following...
  //     i.   Signal sum
  //     ii.  Pedestal
  //     iii. Signal tail
  //     iv.  Signal shape (time dependence)
  //  5. Default is standard single-value ADC...use the channel limits
  //     taken from the data base (hardware # bits) or
  //     Override channel numbers and channel limits if desired

  Int_t i,n,n1,n2,ch1,l1,h1,hit;
  char histspec[EMaxName];			// contains 1D spec.

  n = sscanf(line,"%*s%d%d%d%d%d%d",&n1,&n2,&ch1,&l1,&h1,&hit);
  if( opt == ERAHistAll ){ n1 = 0; n2 = fMaxADC; n = 2; }
  if( n < 1 ){ PrintError("ADC Display Setup"); return; }
  if( n == 1) n2 = n1;

  for( i=n1; i<=n2; i++ ){
    if( i >= fMaxADC ) break;
    switch( fADCdefined[i] ){
    case EUndefinedADC:                        // Not defined, do nothing
      break;
    case EPatternADC:                          // Pattern Unit
      fprintf(fLogStream," Sorry pattern unit %d not yet coded\n", i );
      break;
    case EMultiADC:                            // Multi-Hit ADC
      if( opt != ERAHistMulti ) break;
      if( n < 6 ){ PrintError("Multi-Hit ADC"); return; }
      sprintf(histspec,"ADC%dM%d %d %d %d",i,hit,ch1,l1,h1);
      Setup1D( histspec, fMulti[i]->GetHitPtr(hit) );
      break;
    case EFlashADC:                            // Flash ADC
      switch( opt ){
      case ERAHistFSum:                        // Sum spectrum
	if( n < 5 ){ PrintError("Flash-ADC Sum"); return; }
	sprintf(histspec,"FSum%d %d %d %d",i,ch1,l1,h1);
	Setup1D( histspec, fFlash[i]->GetSumPtr() );
	break;
      case ERAHistFPed:                        // Pedestal spectrum
	if( n < 5 ){ PrintError("Flash-ADC Pedestal"); return; }
	sprintf(histspec,"FPed%d %d %d %d",i,ch1,l1,h1);
	Setup1D( histspec, fFlash[i]->GetPedPtr() );
	break;
      case ERAHistFTail:                       // Tail spectrum
	if( n < 5 ){ PrintError("Flash-ADC Tail"); return; }
	sprintf(histspec,"FTail%d %d %d %d",i,ch1,l1,h1);
	Setup1D( histspec, fFlash[i]->GetTailPtr() );
	break;
      case ERAHistFShape:                      // Shape spectrum
	ch1 = fFlash[i]->GetChan();
	sprintf(histspec,"FShape%d %d 0 0",i,ch1);
	Setup1D( histspec, fFlash[i]->GetAccum(), EHistShapeX );
	break;
      default:                                 // not a flash option
	break;
      }
      break;
    default:                                   // Standard single-value ADC
      if( n < 5 ){ ch1 = fADCdefined[i];  l1 = 0;  h1 = ch1 -1; }
      sprintf(histspec,"ADC%d %d %d %d",i,ch1,l1,h1);
      Setup1D( histspec, &fADC[i] );
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void TA2Analysis::Scalers( Char_t* fname )
{
  // Print the contents of the current and accumulated scaler buffers
  // If file name given put the info in a file of that name
  // Check there are scalers to print 1st
  if( !fMaxScaler ){
    printf(" Sorry no scalers defined on system");
    return;
  }

  FILE* scstream;
  if( fname ){
    scstream = fopen( fname, "w+");
    if( !scstream ) PrintError( fname,"<Opening scaler dump file>" );
    scstream = stdout;
  }
  else scstream = stdout;

  Int_t i,j;

  fprintf(scstream,
	  " %d Scalers defined by DAQ system, %d scaler-read errors\n\n",
	  fMaxScaler, GetScalerError() );
  fprintf(scstream," Printing last-read scaler buffer\n");
  for( i=0; i<fMaxScaler; i+=4 ){
    fprintf(scstream,"%6d : ", i);
    for( j=i; j<i+4; j++ ){
      if( j >= fMaxScaler ) break;
      fprintf(scstream,"%16d", fScaler[j]);
    }
    fprintf(scstream,"\n");
  }
  fprintf(scstream," Printing total accumulated scaler buffer\n");
  for( i=0; i<fMaxScaler; i+=4 ){
    fprintf(scstream,"%6d : ", i);
    for( j=i; j<i+4; j++ ){
      if( j >= fMaxScaler ) break;
      fprintf(scstream,"%16g", fScalerSum[j]);
    }
    fprintf(scstream,"\n");
  }
  fclose( scstream );
  return;
}

//-----------------------------------------------------------------------------
void TA2Analysis::ZeroSumScalers( )
{
  // Zero the sum-scaler buffer
  for( Int_t i=0; i<fMaxScaler; i++ ) fScalerSum[i] = 0;
}

//-----------------------------------------------------------------------------
void TA2Analysis::PostInitialise( )
{
  // Store pointers to ADC's, scalers etc.
  // Set up histograms if desired.
  if( fIsInit ) return;
  if( !fMaxADC ){
    fMaxADC = gAR->GetMaxADC(); 
    fMaxScaler = gAR->GetMaxScaler();
    fADCdefined = gAR->GetADCdefined();
    fADC = gAR->GetADC();
    fScaler = gAR->GetScaler();
    fScalerSum = gAR->GetScalerSum();
  }
  // module indices or errors
  fHardwareError = new Int_t[gAR->GetNModule() + 1];
  //  fRawDataCuts = new TA2MultiCut("AnalysisRaw");
  // Call the default init
  TA2DataManager::PostInit();

  fIsInit = ETrue;
}

//-----------------------------------------------------------------------------
TA2DataManager* TA2Analysis::CreateChild( const char* name, Int_t a )
{
  // Create apparatus class
  // This method only works for those apparati on "known" list
  // TA2GenericApparatus...just a container for detector
  // TA2Physics............basis for Physics implementations
  // Anything else should be created externally and linked in
  // via the AddChild method.

    switch( a ){
    case EA2GenericApparatus:
      return new TA2GenericApparatus( name, this );
    case EA2Physics:
      return new TA2Physics( name, this );
    case EA2GenericPhysics:
      return new TA2GenericPhysics( name, this );
    default:
      break;
    }
    PrintError((char*)name,
	       "<Unknown apparatus..must create outwith base analyser>");
    return NULL;
}

//-----------------------------------------------------------------------------
void TA2Analysis::SaveHist()
{
  // Dump ROOT memory to file
  // Default procedure at end of file or end of analysis

  // Create root file and write base root memory (all histograms)
  Char_t* logfile = NULL;
  if( gAR->IsBatch() ){
    if( gAR->GetBatchDir() )
      logfile = BuildName(gAR->GetBatchDir(),"ARHistograms.root");
  }
  else logfile = BuildName("ARHistograms.root");
  if( logfile ){
    TFile f(logfile,"recreate");
    gROOT->GetList()->Write();
    f.Close();
    delete[] logfile;
  }
  fprintf(fLogStream,"All histograms saved to ARHistograms.root\n\n");

}
//-----------------------------------------------------------------------------
void TA2Analysis::EndFile()
{
  // Tasks to carry out when analysis of a data file is finished
  // Execute any end-of-file macro
  // else save histograms to a ROOT file on disk

  if( ! fIsEndOfFile ) return;     // check if enabled
  TA2DataManager::EndFile();       // call any child EndFile's
  if( !fEndOfFileCmd ) SaveHist(); // save memory to file
}

//-----------------------------------------------------------------------------
void TA2Analysis::Finish()
{
  // Tasks to carry out when analysis of a data file is finished
  // Close any ROOT data file
  // Execute any finsh macro
  // else save histograms to a ROOT file on disk

  if( ! fIsFinish ) return;     // check if enabled
  TA2DataManager::Finish();     // call any child Finish's
  if( !fFinishCmd ) SaveHist(); // save memory to file
}

//-----------------------------------------------------------------------------
void TA2Analysis::InitSaveTree( Char_t* fname )
{
  // Setup raw data output in ROOT Tree format
  // Process data file name using InitTreeFileName
  // Check 1st if supplied file name is different from the previous
  // one supplied....do nothing if no change
  //
  // Usually a tree with 3 branches is created for output
  // branch 0 has the event-by-event ADC information
  //   formated "nhit/i:ndaqevent/i:ntotevent/i:hit[nhit]/i:" 
  //   hit[nhit] has index/value pairs
  // branch 1 has scaler info...every scaler read stored as separate entry
  //   formated "scevent/i:scaler[%d]/i:"
  // branch 2 has a single entry giving type/range info on ADC's
  //   formated "maxadc/I:maxscaler/I:adcdev[%d]/I:"
  //
  if( ! fIsSaveEvent ) return;                   // check if data save enabled
  // if there was a previous file name compare it with the current
  // ie check the name has changed....return here if not changed
  if( fPrevFileName ){
    if( strcmp(fname, fPrevFileName) == 0 ) return;
  }
  InitTreeFileName(fname);
  // Save the file name so that it can be checked subsequently
  // delete mem space for any previous last file name
  if( fPrevFileName ) delete[] fPrevFileName;
  fPrevFileName = BuildName(fname);

  // Construct format strings for branches of ROOT tree
  Char_t headerform[64];
  Char_t scalerform[64];
  Char_t** format = new Char_t*[fNbranch];
  // Event hits
  if( fNbranch > EARHitBr ) format[EARHitBr] =
    (Char_t*)"nhit/i:ndaqevent/i:ntotevent/i:hit[nhit]/i";
  // Scalers
  if( fNbranch > EARScalerBr ){
    sprintf(scalerform,"scevent/i:scaler[%d]/i",fMaxScaler);
    format[EARScalerBr] = scalerform;
    fEvent[EARScalerBr] = fScalerBuff = gAR->GetScalerBuff();
  }
  // Header record
  if( fNbranch > EARHeaderBr ){
    sprintf( headerform, "maxadc/I:maxscaler/I:adcdev[%d]/I", fMaxADC);
    format[EARHeaderBr] = headerform;
  }
  // Create tree and get branch addresses
  TA2DataManager::InitSaveTree( format, fEvent, gAR->GetTreeName(),
				gAR->GetBranchName());
  // Save ADC and Scaler info
  if( fNbranch > EARHeaderBr ){
    Int_t* hb = (Int_t*)fEvent[EARHeaderBr];
    *hb++ = fMaxADC;                     // # ADC
    *hb++ = fMaxScaler;                  // # Scaler
    for(Int_t i=0; i<fMaxADC; i++) *hb++ = fADCdefined[i]; // ADC info
    SaveEvent(EARHeaderBr);              // write (once only) header to tree
  }

  // Recursively call child tree-initialise procedures
  TA2DataManager::InitSaveTree();
}

//-----------------------------------------------------------------------------
void TA2Analysis::ChangeTreeFile( Char_t* fname )
{
  // Change the file to which a ROOT Tree is saved
  // when the input data file name has changed

  if( !fIsSaveEvent ) return;                        // no saved output
  CloseEvent();                                      // close tree & file
  InitSaveTree( fname );                             // new tree & file
}

//-----------------------------------------------------------------------------
void TA2Analysis::InitTreeFileName( Char_t* fname )
{
  // Construct name of reduced data output file for input name fname
  // Process data file name....look for suffix .dat, .rd0 etc.
  // .dat Raw data file in Acqu Mk1 format
  // .rd0 Raw data file in ROOT tree format....1st filter pass of analysis
  // .rdN Nth filter pass of analysis.
  // If the (Mk1) input file is XXX.dat, the (tree) output file will be XXX.rd0
  // If the (tree) input file is XXX.rd0, the output file will be XXX.rd1...
  // ...and so on
  // If the supplied filename is such that the naming algorithm fails
  // then a default rdX suffix is used.
  // The name string is stored in fTreeFileName

  // Look for directory spec., if found strip it away
  Char_t* start = strrchr( fname, '/');
  if( start ) start++;                           // advance past '/'
  else start = fname;                            // no directory spec.
  // Look for '.' before file-type spec.
  Char_t* point = strrchr( start, '.' );
  if( ! point ){                                 // file name must have '.'
    PrintError(fname,"<Init-Save no ',' in filename>");
    return;
  }
  point++;                                       // past '.'
  Char_t name[256];
  Int_t nsuff;
  Char_t suff[8];
  // Now try to work out the suffix for the new file name
  name[0] = 0;                                   // ensure null string
  strncat( name, start, (point-start) );
  if( !strcmp(point, "dat") ) strcat( name, "rd0" );
  else if( !strncmp( point, "rd", 2 ) ){
    point += 2;
    if(sscanf( point,"%d",&nsuff ) == 1 ){
      nsuff++;
      sprintf( suff,"rd%d",nsuff );
      strcat( name, suff);
    }
    else strcat( point, "rdX" );
  }
  else strcat( point, "rdX" );
  if( fTreeFileName ) delete[] fTreeFileName;
  fTreeFileName = BuildName( name );
}

ClassImp(TA2Analysis)
