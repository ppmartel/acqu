//--Author	JRM Annand   30th Sep 2003
//--Rev 	JRM Annand...12th Dec 2003   Multi ADC's
//--Rev 	JRM Annand... 4th Feb 2004...Data-cuts
//--Rev 	JRM Annand...26th Jun 2004...GetGrandChild check child
//--Rev 	JRM Annand...24th Aug 2004...Add rate-mon and bit-pattern stuff
//--Rev 	JRM Annand...18th Oct 2004...AddChild pass log stream, periodic
//--Rev 	JRM Annand...21st Oct 2004...Data read/save ROOT format
//--Rev 	JRM Annand...17th Nov 2004...EOF, finish macros
//--Rev 	JRM Annand...14th Jan 2005...fTreeFileName
//--Rev 	JRM Annand...23rd Mar 2005...Tree file title
//--Rev 	JRM Annand... 6th Apr 2005...2D cuts, misc cut scope
//--Rev 	JRM Annand...14th Apr 2005...Wild-card cut options
//--Rev 	JRM Annand... 7th Jun 2005...Random generator
//--Rev 	JRM Annand... 1st Aug 2005...SetCut == in if()
//--Rev 	JRM Annand...26th Oct 2006...Local scalers
//--Rev 	JRM Annand...29th Jan 2007...4v0 TA2System compatible
//--Rev 	JRM Annand...25th Nov 2008...GetChildType() nth instance
//--Update	JRM Annand... 1st Sep 2009   delete[]
//--Update	K Livingston..7th Feb 2013   Support for handling EPICS buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2DataManager
//
// Base class for any analysis entity (analysis, apparatus, detector) to
// enable transfer of raw or analysed data between parent and child classes
//

#include "TA2DataManager.h"


//-----------------------------------------------------------------------------
TA2DataManager::TA2DataManager( const Char_t* name, const Map_t* comlist )
  :TA2System( name, comlist )
{
  // Constructor...do not allocate "new" memory
  // Check passed pointer is to a TA2Apparatus class
  // "Cadge" ADC and Scaler pointers via hook to analysis class

  fRandom = NULL;
  fRandSeed = 999999;
  fParent = NULL;
  fChildren = NULL;
  fVariables = NULL;
  fTreeFile = NULL;
  fTree = NULL;
  fBranch = NULL;
  fNbranch = gAR->GetNbranch();
  fDataCuts = NULL;
  fHistCuts = NULL;
  fMiscCuts = NULL;
  fCompoundCut = NULL;
  fRateMonitor = NULL;
  fBitPattern = NULL;
  fPatternHits = NULL;
  fNPatternHits = NULL;
  fADC = gAR->GetADC( );
  fADCdefined = gAR->GetADCdefined( );
  fMulti = gAR->GetMulti();
  fScaler = gAR->GetScaler( );
  fScalerBuff = gAR->GetScalerBuff();
  fScalerSum = gAR->GetScalerSum( );
  fEvent = gAR->GetEvent();
  fMaxADC = gAR->GetMaxADC( );    
  fMaxScaler = gAR->GetMaxScaler( );
  fScalerIndex = fScalerCurr = NULL;
  fScalerAcc = NULL;
  fNPeriod = 0;
  fMaxPeriod = 0;
  fPeriodCmd = fEpicsCmd = fEndOfFileCmd = fFinishCmd = NULL;
  fTreeFileName = NULL;
  fIsRawDecode = fIsDecode = fIsReconstruct = EFalse;
  fIsRawDecodeOK = fIsDecodeOK = fIsReconstructOK = EFalse;
  fIsCut = EFalse;
  fIsBitPattern = EFalse;
  fIsRateMonitor = EFalse;
  fIsEndOfFile = fIsFinish = fIsSaveEvent = EFalse;
  kValidChild = NULL;
}

//-----------------------------------------------------------------------------
TA2DataManager::~TA2DataManager()
{
  // Free up any allocated memory
}

//-----------------------------------------------------------------------------
void TA2DataManager::AddChild( TA2DataManager* child, const char* type )
{
  // Add child to list, after checking its a child
  // Check that the child list is initialised

  if( !fChildren ) fChildren = new TList;

  if( !child->InheritsFrom(type) ){
    fprintf(fLogStream," Error: %s attempted add of invalid child  pointer\n",
	   this->ClassName());
    return;
  }
  fChildren->AddLast( child );
  fprintf(fLogStream," Child class %s linked to %s parent\n", 
	 child->ClassName(), this->ClassName() );
}

//-----------------------------------------------------------------------------
void TA2DataManager::AddChild( char* line, TA2DataManager** dm )
{
  // Add child class to list....info from command line
  // Configure child if parameter file specified on command line
  // Output log stream passed by parent

  char name[EMaxName];
  char type[EMaxName];
  char setup[EMaxName];
  Int_t n;
  Int_t dclass;

  if( (n = sscanf( line,"%s%s%s",name,type,setup )) < 2 ){
    PrintError( line, "<Add-Child-Class>" );
    return;
  }   
  if( (dclass = Map2Key( type, kValidChild )) == -1 ){
    PrintError( line, "<Add-Child-Class>" );
    return;
  }
  TA2DataManager* d = CreateChild( (const char*)name, dclass );
  d->SetLogStream( fLogStream );
  if( dm ) *dm = d;
  else AddChild( d, type );
  if( n > 2 ) d->FileConfig( setup );
}

//-----------------------------------------------------------------------------
TA2DataManager* TA2DataManager::GetChildType( const Char_t* type, Int_t n )
{
  // Search list of children for nth instance of particular class type

  if( !fChildren ) return NULL;
  TIter nextchild( fChildren );
  TA2DataManager* child;
  while( (child = (TA2DataManager*)nextchild() ) ){
    if( (child->InheritsFrom(type)) && (!n) ) return child;
    n--;
  }
  PrintError((char*)type,"<Find Acqu-Root class type>");
  return NULL;
}

//-----------------------------------------------------------------------------
TA2DataManager* TA2DataManager::GetChild( const char* name,  const char* type )
{
  // Search list of children for particular class

  if( !fChildren ) return NULL;
  TObject* dat = fChildren->FindObject(name);
  if( dat ){
    if( dat->InheritsFrom(type) ) return (TA2DataManager*)dat;
  }
  printf("No child object of name %s, type %s found\n",
	  name, type );
  return NULL;
}

//-----------------------------------------------------------------------------
TA2DataManager* TA2DataManager::GetGrandChild( const char* name,
					       const char* type )
{
  // Search list of children's children for particular class

  if( !fChildren ) return NULL;
  TIter nextchild( fChildren );
  TA2DataManager* child;
  TA2DataManager* grandchild;

  while( (child = (TA2DataManager*)nextchild() ) ){
    if( !child ) continue;
    grandchild = 
	  (TA2DataManager*)((child->GetChildren())->FindObject(name));
    if( !grandchild ) continue;
    if( grandchild->InheritsFrom(type) ) return grandchild;
    else continue;
  }
  printf(" Class %s of type %s not a grandchild of %s\n\n",
	  name, type, this->ClassName() );
  return NULL;
}

//-----------------------------------------------------------------------------
void TA2DataManager::ParseCut( char* line )
{
  // General-purpose data-cut setup
  // Parse cut information from "Data-Cut:" command line
  // Check initialisation completed first...do postintialisation if not.
  // After working out what's desired, from the input line
  // the SetCut() method is called.

  const Map_t cutscope[] = {            // name specifies type
    // Different type of condition
    {"histogram",      ECutHistogram},  // applies to histograms
    {"data",           ECutData},       // applies to all local output data
    {"misc",           ECutMisc},       // miscellaneous filtering
    {"compound-hist",  ECutCHist},      // compound histogram cut
    {"compound-data",  ECutCData},      // compound data cut
    {"compound-misc",  ECutCMisc},      // compound miscellaneous cut
    {"element",        ECutElement},    // element of compound condition cut
    // Logical linkage of conditions
    {"OR",             ECutOR},         // OR linked to previous cut
    {"AND",            ECutAND},        // AND linked to previous cut
    {"NOR",            ECutNOR},        // NOT(result) OR linked
    {"NAND",           ECutNAND},       // NOT(result) AND linked
    {NULL,             -1}
};

  if( !fIsInit ){
    fprintf(fLogStream, " %s running PostInitialise before data-cut setup\n\n",
	   this->ClassName() );
    PostInitialise();
  }

  Double_t lx,hx;                       // low/high limits of X axes
  Int_t ix,iy;                          // single array element offset
  Int_t sc;                             // scope of cut
  UInt_t cutlink;                       // cut AND/OR linkage flag
  Char_t varX[EMaxName];	       	// X-axis name (variable)
  Char_t varY[EMaxName];	       	// Y-axis name (variable)
  Char_t cutfile[EMaxName];             // name of 2D polygon spec. file
  Char_t cutname[EMaxName];             // name of 2D polygon spec. cut
  Char_t scope[EMaxName];	       	// Cut class name
  Char_t index[16];                     // for 2D names
  Char_t dimension[8];                  // 1D or 2D
  Char_t link[8];                       // condition linkage AND/OR
  Double_t opt[8];                      // wild-card options

  // Read the input line and work out the basics
  Int_t nscan = sscanf(line, "%s%s%s%s", dimension, varX, link, scope);
  if( nscan != 4 ){
    PrintError(line,"<Filter cut: specification line format>");
    return;
  }
  // Cut linkage (for chain of cut conditions
  switch( (cutlink = Map2Key( link, cutscope )) ){    
  case ECutAND:
  case ECutNAND:
  case ECutOR:
  case ECutNOR: break;
  default: PrintError(line,"<Filter cut: unknown linkage type>"); return;
  }
  // Scope of cut action and single/compound info
  switch( (sc = Map2Key( scope, cutscope )) ){
    // Cut applies to histogram but doesn't affect data flow
  case ECutHistogram:
    // Cut controls flow of data and hence also histogram filling
  case ECutData:
    // Wild-card filtering
  case ECutMisc:
    // Element of compound cut 
  case ECutElement:
    if( !GetN2V(varX) ){               // Check the X variable is known
      PrintError(line,"<Filter cut: X variable not found>");
      return;
    }
    break;
  case ECutCData:
    // Compound data flow cut
  case ECutCHist:
    // Compound histogram cut
  case ECutCMisc:
    // Compound miscellaneous cut
    // Compound cuts...don't have variables of their own
    // and so don't need any variable spec.
    SetCut( varX, cutlink, sc, 0, (Double_t*)NULL, 0., 0. );
    fIsCut = ETrue;
    return;
  default: PrintError(line,"<Filter cut: unknown cut scope>");
    return;
  }

  for( Int_t i=0;i<8;i++ )opt[i] = (Double_t)EBufferEnd; // default no options

  // Now parse the rest of the input line
  // The following is for a 1D low-high window cut on single variable
  if( !strcmp(dimension,"1D") ){
    nscan = sscanf(line, "%*s%*s%*s%*s%lf%lf%d%lf%lf%lf%lf%lf%lf%lf%lf",
		   &lx,&hx,&ix,opt,opt+1,opt+2,opt+3,opt+4,opt+5,opt+6,opt+7);
    if( nscan < 2 ){
      // Check if its a compund cut
      if( (sc != ECutCData) && (sc != ECutCHist) ){
	PrintError(line,"<1D filter cut: insufficient parameters on line>");
	return;           // wasn't a compound cut so line format wrong
      }
    }
    else if( nscan == 2 ) ix = 0;
    else{ 
      sprintf(index,"%d",ix);
      strcat(varX,index);
    }
    if( IsInt( varX ) )           // integer variables
      SetCut( varX, cutlink, sc, GetVarType(varX), 
	      (GetVar<Int_t>(varX))+ix, (Int_t)lx, (Int_t)hx,
	      (Int_t*)NULL,(TCutG*)NULL,opt );
    else                          // double variables
      SetCut( varX, cutlink, sc, GetVarType(varX),
	      (GetVar<Double_t>(varX))+ix, lx, hx, 
	      (Double_t*)NULL, (TCutG*)NULL, opt );
  }
  // Polygon cut on 2D X-Y correlation
  else if( !strcmp(dimension,"2D") ){
    ix = iy = 0;                        // default scaler variables
    nscan = 
      sscanf(line, "%*s%*s%*s%*s%s%s%s%d%d%lf%lf%lf%lf%lf%lf%lf%lf",
	     varY,cutname,cutfile,&ix,&iy,
	     opt,opt+1,opt+2,opt+3,opt+4,opt+5,opt+6,opt+7);
    if( nscan < 3 ){
      PrintError(line,"<2D filter cut: insufficient parameters on line>");
      return;
    }
    if( !GetN2V(varY) ){
      PrintError(line,"<2D filter cut: Y variable not found>");
      return;
    }
    Char_t* fname = BuildName("./data/",cutfile);  // assume file in data dir
    TFile* cutf = new TFile(fname,"READ");
    delete[] fname;
    if( !cutf->IsOpen() ){
      PrintError(line,"<2D filter cut: missing ROOT file for polygon>");
      return;
    }
    TCutG* cut = (TCutG*)cutf->Get(cutname);      // locate TCutG
    // check if the cut was found to prevent usage of null pointers
    if(!cut){
      PrintError(line,"<2D filter cut: specified cut name does not exist>");
      cutf->Close();
      return;
    }
    // Create TA2Cut from TCutG and link into desired cuts list
    // X-Y should be either integer of double (both the same)
    if( IsInt( varX ) )
      SetCut( cutname, cutlink, sc, GetVarType(varX), 
	      (GetVar<Int_t>(varX))+ix,0,0,(GetVar<Int_t>(varY))+iy,cut,opt );
    else
      SetCut( cutname, cutlink, sc,GetVarType(varX),
	      (GetVar<Double_t>(varX))+ix,
	      0.,0.,(GetVar<Double_t>(varY))+iy,cut,opt );
    cutf->Close();
    delete cutf;
  }
  else{
    PrintError(line,"<Filter Cut: must specify 1D or 2D cut>");
    return;
  }

  fIsCut = ETrue;
  return;
}

//-----------------------------------------------------------------------------
template<typename T>
void TA2DataManager::SetCut( Char_t* name, UInt_t link, Int_t scope,
			     Int_t type, T* vX, T lX, T hX, T* vY, TCutG* cXY,
			     Double_t* opt )
{
  // Create a 1D or 2D cut and add it to the desired cut list.
  // 2D cuts are signaled by a non-null cXY parameter

  TA2Cut* cut;
  TA2MultiCut* mcut;

  // Decide which cut list to use
  // Initialise the list if not already done
  switch( scope ){
  case ECutData:
    // Single data cut, reset any compound cut chain
    fCompoundCut = NULL;
    mcut = fDataCuts;
    break;
  case ECutHistogram:
    // Single histogram cut, reset any compound cut chain
    fCompoundCut = NULL;
    mcut = fHistCuts;
    break;
  case ECutMisc:
    // Single miscellaneous cut, reset any compound cut chain
    fCompoundCut = NULL;
    mcut = fMiscCuts;
    break;
  case ECutCData:
    // Compound data cut
    fCompoundCut = new TA2MultiCut(name);
    fDataCuts->AddCut( fCompoundCut );
    return;
  case ECutCHist:
    // compound histogram cut
    fCompoundCut = new TA2MultiCut(name);
    fHistCuts->AddCut( fCompoundCut );
    return;
  case ECutCMisc:
    // compound miscellaneous cut
    fCompoundCut = new TA2MultiCut(name);
    fMiscCuts->AddCut( fCompoundCut );
    return;
  case ECutElement:
    // element of compound cut
    if(!fCompoundCut){
      PrintError(name,"<Element specified for NULL compound cut>");
      return;
    }
    mcut = fCompoundCut;
    break;
  default:
    break;
  }
  // Create 1D cut
  if( !cXY){
    if( type == ECutSingleX ) cut = new TA2Cut1DS<T>( name,link,lX,hX,vX,opt);
    else if(type == ECutMultiX)cut = new TA2Cut1DM<T>( name,link,lX,hX,vX,opt);
  }
  // Create 2D cut
  else{
    if( type == ECutSingleX )
      cut = new TA2Cut2DS<T>( name, link, vX, vY, cXY, opt);
    else if( type == ECutMultiX )
      cut = new TA2Cut2DM<T>( name, link, vX, vY, cXY, opt);
  }
  // Add the cut to the desired multi-cut list
  mcut->AddCut( cut );
  return;
}

//---------------------------------------------------------------------------
void TA2DataManager::PostInit()
{
  // Some further initialisation after main setup parameters read in
  // and usually before cuts and display setup
  char name[EMaxName];
  strcpy(name, GetName());
  char* nptr = name + strlen(name);
  strcpy(nptr,"_DataCuts");
  fDataCuts = new TA2MultiCut(name);
  strcpy(nptr,"_HistCuts");
  fHistCuts = new TA2MultiCut(name);
  strcpy(nptr,"_MiscCuts");
  fMiscCuts = new TA2MultiCut(name);
  // Load name-variable associations for histograms and cuts
  LoadVariable();
}

//---------------------------------------------------------------------------
void TA2DataManager::LoadVariable()
{
  // Load any plottable variables
  // Bit-pattern variables
  if( fIsBitPattern ){
    LoadVariable("NPatternHits",fNPatternHits,  EISingleX);
    for( Int_t i=0; i<fBitPattern->GetNPattern(); i++ ){
      LoadVariable( fBitPattern->GetPatternName(i),
				    fPatternHits[i],EIMultiX);
    }
  }
}

//---------------------------------------------------------------------------
void TA2DataManager::ParsePeriod( Char_t* line)
{
  // Read period info from line of text
  // Interval (in number of events) between periodic procedure
  // Name of Macro file containing the periodic procedure
  // Name of the periodic procedure to call
  // including any parameters to pass to the
  // periodic procedure

  Char_t macro[256];
  Char_t procedure[256];
  Char_t* loadcmd;

  if( sscanf( line, "%d%s%s", &fMaxPeriod,macro,procedure ) < 3 ){
    PrintError(line,"<Periodicity Input Parse>");
    return;
  }
  loadcmd = BuildName(".L ", macro);
  fPeriodCmd = BuildName(procedure);    // save procedure string

  fprintf(fLogStream," %s periodic command: %s   Interval set to %d\n",
	  this->ClassName(), fPeriodCmd, fMaxPeriod );

  gROOT->ProcessLine(loadcmd);          // load the macro
  return;
}

//---------------------------------------------------------------------------
void TA2DataManager::ParseEpics( Char_t* line)
{
  // Read Epics info from line of text
  // Name of Macro file containing the periodic procedure
  // Name of the periodic procedure to call
  // including any parameters to pass to the
  // periodic procedure

  Char_t macro[256];
  Char_t procedure[256];
  Char_t* loadcmd;
  
  if( sscanf( line, "%s%s", macro,procedure ) < 2 ){
    PrintError(line,"<Epics: Input Parse>");
    return;
  }
  loadcmd = BuildName(".L ", macro);
  fEpicsCmd = BuildName(procedure);    // save procedure string
  
  fprintf(fLogStream," %s Epics command: %s\n",
	  this->ClassName(), fEpicsCmd);

  gROOT->ProcessLine(loadcmd);          // load the macro
  return;
}

//---------------------------------------------------------------------------
void TA2DataManager::ParseEndOfFile( Char_t* line)
{
  // Read any end-of-file macro spec
  // Name of Macro file containing the periodic procedure
  // Name of the periodic procedure to call
  // including any parameters to pass to the end-of-file procedure

  Char_t macro[256];
  Char_t procedure[256];
  Char_t* loadcmd;

  if( sscanf( line, "%s%s", macro,procedure ) == 2 ){
    loadcmd = BuildName(".L ", macro);
    fEndOfFileCmd = BuildName(procedure);    // save procedure string
    fprintf(fLogStream," %s End-of-file command: %s \n",
	    this->ClassName(), fEndOfFileCmd );
    gROOT->ProcessLine(loadcmd);          // load the macro
  }
  SetEndOfFile();
  return;
}

//---------------------------------------------------------------------------
void TA2DataManager::ParseFinish( Char_t* line)
{
  // Read any end-of-run macro spec
  // Name of Macro file containing the periodic procedure
  // Name of the periodic procedure to call
  // including any parameters to pass to the end-of-file procedure

  Char_t macro[256];
  Char_t procedure[256];
  Char_t* loadcmd;

  if( sscanf( line, "%s%s", macro,procedure ) == 2 ){
    loadcmd = BuildName(".L ", macro);
    fFinishCmd = BuildName(procedure);    // save procedure string
    fprintf(fLogStream," %s End-of-Run command: %s \n",
	    this->ClassName(), fFinishCmd );
    gROOT->ProcessLine(loadcmd);          // load the macro
  }
  SetFinish();
  return;
}

//---------------------------------------------------------------------------
void TA2DataManager::SetEndOfFile()
{
  // Enable End-of-File Procedures
  fIsEndOfFile = ETrue;
  fprintf(fLogStream,"End of File procedures enabled for %s\n\n",
	    this->ClassName());
}

//---------------------------------------------------------------------------
void TA2DataManager::SetFinish()
{
  // Enable End-of-Run Procedures
  fIsFinish = ETrue;
  fprintf(fLogStream,"End of Run procedures enabled for %s\n\n",
	    this->ClassName());
}

//---------------------------------------------------------------------------
void TA2DataManager::SetSaveEvent()
{
  // Enable saving of "events" ....
  // pieces of data to be saved every experimental event
  // This could be e.g
  // raw data ( ADC index/value strings,
  // or "physical" data (4-Vectors with particle ID)
  fIsSaveEvent = ETrue;
  fprintf(fLogStream,"Reduced data saved in TTree format for class %s\n\n",
	    this->ClassName());
}

//---------------------------------------------------------------------------
void TA2DataManager::InitSaveTree( Char_t** format, void** buff,
				   Char_t* tname, Char_t** bname )
{
  // Initialise output tree for data, Create ROOT file of name: fTreeFileName,
  // Initialise TTree data structure to be written to the ROOT file
  // tree data format: format, data buffer: buff, (optional) tree name: tname

  // if tname not given, use name of calling class as default
  if( !fIsSaveEvent ) return;                        // no saved output
  Char_t* treename;
  Char_t* branchname;
  Char_t* filename;
  // If the tree file isn't already created, do it now
  if( ! fTreeFile ){
    if( gAR->GetTreeDir() )
      filename = BuildName( gAR->GetTreeDir(), fTreeFileName );
    else filename = BuildName( fTreeFileName );
    fTreeFile = new TFile(filename,"RECREATE","AR_Offline_Analysis");
    delete[] filename;
  }
  // If the tree doesn't exist, create it now
  // build the name of the tree (name of class if not supplied)
  if( ! fTree ){
    if( tname ) treename = BuildName( tname );
    else treename = BuildName( (Char_t*)this->ClassName(), "_AR_Save" ); 
    fTree = new TTree(treename, "AcquRoot Analysis");
    delete[] treename;
  }
  // branch name(s)
  Char_t brsuff[16];
  fBranch = new TBranch*[fNbranch];
  for( Int_t i=0; i<fNbranch; i++ ){
    if(!bname){
      strcpy(brsuff,"_BR_");
      sprintf(brsuff+4, "%d", i);
      branchname = BuildName((Char_t*)this->ClassName(),brsuff);
    }
    else branchname = BuildName(bname[i]);
    fTree->Branch(branchname,buff[i],format[i]);  // Map branch to data mem
    fBranch[i] = fTree->GetBranch(branchname);
    delete[] branchname;
  }

}

//---------------------------------------------------------------------------
void TA2DataManager::InitSaveTree( )
{
  // Sucessively call init-saves of children
  // default calling function doesn't perform any init-save 
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->InitSaveTree();
}

//-----------------------------------------------------------------------------
void TA2DataManager::Periodic( )
{
  // Optional tasks to perform every time fMaxPeriod events have
  // been processed. If fMaxPeriod = 0 nothing done
  // If any periodic macro has been defined then execute it,
  // then step through the list of child analysis entities
  // and call any defined child periodic macro

  if( !UpdatePeriod() ) return;  // no periodic procedure
  MacroExe( fPeriodCmd );        // call any defined macro

  TIter nextchild(fChildren);    // start loop throught children
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) ){
    child->Periodic();
  }
}

//-----------------------------------------------------------------------------
void TA2DataManager::Epicsodic( )
{
  // Optional tasks to perform every time there's an EPICS event
  // then step through the list of child analysis entities
  // and call any defined child periodic macro

  if( !fEpicsCmd ) return;       // no epicsodicdic procedure
  MacroExe( fEpicsCmd );         // call any defined macro

  TIter nextchild(fChildren);    // start loop throught children
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) ){
    child->Epicsodic();
  }
}

//-----------------------------------------------------------------------------
void TA2DataManager::EndFile( )
{
  // Tasks to perform when the analysis of a data file is complete
  // Call any locally defined macro
  // Step through list of child analysis entities
  // and call the child EndFile method for each

  if( ! fIsEndOfFile ) return;        // check if enabled
  MacroExe( fEndOfFileCmd );          // local macro, if defined
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->EndFile();
}

//-----------------------------------------------------------------------------
void TA2DataManager::Finish( )
{
  // Tasks to perform when the analysis of all data files is complete
  // Call any locally defined macro
  // Step through list of child analysis entities
  // and call the child Finish method for each

  if( ! fIsFinish ) return;        // check if enabled
  MacroExe( fFinishCmd );          // local macro, if defined
  TIter nextchild(fChildren);
  TA2DataManager* child;
  while( ( child = (TA2DataManager*)nextchild() ) )
    child->Finish();
}

//----------------------------------------------------------------------------
void TA2DataManager::CloseEvent()
{
  // Save event to Tree
  // Check event saving enabled
  if( !fIsSaveEvent ) return;
  if( !fTree ) return;
  TFile* f = fTree->GetCurrentFile();
  f->Write();
  fTree->Delete();
  f->Close();
  fTree = NULL;
  fTreeFile = NULL;
}

ClassImp(TA2DataManager)
