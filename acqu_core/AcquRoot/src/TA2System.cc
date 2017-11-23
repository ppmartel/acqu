//--Author	JRM Annand    9th Jan 2003
//--Rev		JRM Annand...26th Feb 2003...1st "production" version
//--Rev 	JRM Annand...27th Feb 2003...Separate histogram stuff
//--Rev 	JRM Annand...18th Oct 2004   Diagnostic test to log file
//--Rev 	JRM Annand...18th Oct 2004...Adapt TDAQsystem from TA2System
//--Rev 	JRM Annand...20th Jan 2007...Feed back from TDAQsystem(AcquDAQ)
//--Rev 	JRM Annand...14th Aug 2007...Include TMath.h,TROOT.h Root 5.16
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Update	JRM Annand... 1st Sep 2009   delete[], cstdlib?
//--Update  A Neiser...   6th June 2013  Make char_t* const
//--Update  A Neiser...   6th June 2013  Always add filename to fatal fopen 
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2System
//
// Connects to the Root hierarchy via TObj...a Root object with
// name and title, which can be retrieved via name
// All major AcquRoot objects should inherit from this one
// It cannot itself be instantiated...pure virtual SetConfig().
// Contains methods for...
// 1. Configuring derived classes with data from file or command line
// 2. Error handling
// 3. Message handling
// 4. File handling for input data

#include "TA2System.h"
#include "ARFile_t.h"
#include <sstream>


//-----------------------------------------------------------------------------
TA2System::TA2System( const Char_t* name, const Map_t* maplist,
			const Char_t* file, const Char_t* log ):
  TNamed( name, "AcquRoot" )
{
  // Base initialisation of Acqu++ object
  // Setup input config stream if file name supplied
  // Save log output stream....use standard out (tty) if not supplied
  //
  if( !log ){
    fLogStream = stdout;
    fLogFile = NULL;
  }     
  else{ 
    Char_t* logfile = BuildName( log,".log" );
    fLogFile = new ARFile_t( logfile, "w+", this );
    fLogStream = fLogFile->GetStart();
    if( !fLogStream )
      PrintError( logfile, "<Log file open>", EErrFatal );
    delete[] logfile;
  }
  BaseInit( maplist, file );
}

//-----------------------------------------------------------------------------
TA2System::TA2System( const Char_t* name, const Map_t* maplist,
			const Char_t* file, FILE* log ):
  TNamed( name, "AcquRoot" )
{
  // Base initialisation of Acqu++ object
  // Setup input config stream if file name supplied
  // Save log output stream....use standard out (tty) if not supplied

  if( !log ) fLogStream = stdout;        
  else fLogStream = log;
  BaseInit( maplist, file );
}

//-----------------------------------------------------------------------------
void TA2System:: BaseInit( const Map_t* maplist, const Char_t* file )
{
  // Basic initialisation of AcquRoot class
  // Open config file if name supplied
  // Fatal error of open fails
  //
  fIsInit = kFALSE;
  fIsError = kFALSE;
  fNPrintMsg = 0;
  fMaxPrintMsg = 1000;
  fCmdList[0] = maplist;
  fCmdList[1] = NULL;
  if( !file ){                  // no configuration file
    fInputFile = NULL;
  }
  else{                         // open config file
    Char_t* inputName;
    if( !strstr( file, "/") ) inputName = BuildName( "./data/",file );
    else inputName = BuildName( file );
    fInputFile = new ARFile_t( inputName,"r",this );
    if( !fInputFile->GetStart() )
      PrintError(inputName, "<Configuration File Open>", EErrFatal);
    delete[] inputName;
  }
}

//-----------------------------------------------------------------------------
void TA2System::SetLogFile( const Char_t* logname )
{
  // Create a log file for error messages etc.
  // If NULL name give use stdout.
  // If log file already open close it and open a new one
  //
  if( !logname ){
    fLogStream = stdout;
    fLogFile = NULL;
    return;
  }
  if( fLogFile ){
    fprintf( fLogStream, " Closing log file %s\n",fLogFile->GetName() );
    delete fLogFile;
  }
  fLogFile = new ARFile_t( logname, "w+", this );
  if( !(fLogStream = fLogFile->GetStart()) )
    PrintError(logname, "<Diagnostic Log File Open>", EErrFatal);
}

//-----------------------------------------------------------------------------
TA2System::~TA2System()
{
  // Tidy up memory and opened files
  //
  if( fInputFile ) delete fInputFile;
  if( fLogFile ) delete fLogFile;
}

//-----------------------------------------------------------------------------
void TA2System::AddCmdList(const Map_t* maplist )
{
  // Add further list of commands for setup purposes
  //
  Int_t i;
  for( i=0; i<EMaxCmdList; i++ ){
    if( !fCmdList[i] ){
      fCmdList[i] = maplist;
      fCmdList[i+1] = NULL;
      break;
    }
  }
  if( i == EMaxCmdList ) 
    PrintError((Char_t*)"","<Exhausted store for extra command list>");
}


//-----------------------------------------------------------------------------
void TA2System::PrintError( const Char_t* line, const Char_t* operation,
			    Int_t errorlevel, Bool_t cntSuppr )
{
  // Print diagnostic if setup error detected
  // and set error flag
  //
  if (!cntSuppr || (cntSuppr && fNPrintMsg < fMaxPrintMsg)){
    std::stringstream ss;
    if( operation )
      ss << " Error in operation " << operation << " of class "
         << this->ClassName() << " at command line:\n" << line;
    else
      ss << " Error in setup of class "
         << this->ClassName() << " at command line:\n" << line;
    fIsError = kTRUE;
    fprintf(fLogStream, ss.str().c_str());
    fflush(fLogStream);
    if( errorlevel == EErrFatal ){
      fprintf(fLogStream, " FATAL ERROR...exiting AcquRoot\n" );
      printf("AcquRoot FATAL ERROR...please examine .log files or syslog\n");
      exit( -1 );
    }
  }
  if (cntSuppr && fNPrintMsg == fMaxPrintMsg){
    std::stringstream ss;
    ss << " Reached limit on printed messages (" << fMaxPrintMsg << ") in class "
       << this->ClassName() << "\n"
       << " Some frequently occuring messages will be suppressed from now on.\n";
    fprintf(fLogStream, ss.str().c_str());
  }
  if (cntSuppr) fNPrintMsg++;
}

//-----------------------------------------------------------------------------
void TA2System::PrintMessage( const Char_t* mess, Bool_t cntSuppr )
{
  // General diagnostics
  //
  if (!cntSuppr || (cntSuppr && fNPrintMsg < fMaxPrintMsg)){
    std::stringstream ss;
    ss << " " << this->ClassName() << ": " << mess;
    fprintf( fLogStream, ss.str().c_str());
    fflush(fLogStream);
  }
  if (cntSuppr && fNPrintMsg == fMaxPrintMsg){
    std::stringstream ss;
    ss << " Reached limit on printed errors (" << fMaxPrintMsg << ") in class "
       << this->ClassName() << "\n"
       << " Some frequently occuring messages will be suppressed from now on.\n";
    fprintf(fLogStream, ss.str().c_str());
  }
  if (cntSuppr) fNPrintMsg++;
}

//-----------------------------------------------------------------------------
void TA2System::FileConfig( Char_t* filename )
{
  // Set up an Acqu-Root Class on the basis of
  // information read from a file
  // If no filename supplied look for any file specified at construction
  // If the filename contains character '/' assume absolue path
  // otherwise assume file is in $acqu/data/ 
  //
  ARFile_t* cfile;
  // no file name supplied
  if( !filename ){
    if( !fInputFile )
      PrintError("","<FileConfig no file specified>",EErrFatal);
    else cfile = fInputFile;
  }
  // filename supplied...use it
  else{
    Char_t* inputName;
    if( strchr( filename, '/' ) ) inputName = BuildName( filename );
    else inputName = BuildName("./data/", filename);
    // make setup file open fatal, prevents segfault
    cfile = new ARFile_t(inputName,"r",this, kTRUE);
    delete[] inputName;
  }
  Int_t key;
  while( (key = cfile->ReadKey(fCmdList)) != -1 ){
    Char_t* l = cfile->GetLine();	  // starts after keyword
    SetConfig(l,key);		          // do the business
  }
  // close any temp input file
  if( cfile != fInputFile ) delete cfile;
}

//-----------------------------------------------------------------------------
void TA2System::CommandConfig( Char_t* command )
{
  // Read in info to an AcquRoot Class on the basis of
  // information read from the keyboard
  //
  Int_t key;
  Char_t keyword[EKeyWordSize];
  if( sscanf( command, "%s", keyword ) != 1 ){
    printf("<No command keyword supplied>\n");
    return;
  }
  if( (key = Map2Key( keyword, fCmdList )) == -1){
    printf("<Unrecognised Input Command from terminal>\n");
    return;
  }
  Char_t* l = command + strlen(keyword);  // point to past the keyword
  SetConfig(l,key);		       // do the business
}

//-----------------------------------------------------------------------------
Int_t TA2System:: Map2Key(Char_t* function, const Map_t* map)
{
  // Return integer matched to string from map array
  // -1 return on error
  //
  if( !map ) map = fCmdList[0];
  if( !map ) return -1;
  const Char_t* f;
  for( Int_t i=0; ; i++ ){
    f = map[i].fFnName;
    if( !f ) return -1;
    if( !strcmp(function, f)  ) return map[i].fFnKey;
  }
}

//-----------------------------------------------------------------------------
Int_t TA2System:: Map2Key(Char_t* function, const Map_t** map)
{
  // Check all loaded map arrays for match to string
  // -1 return on error
  //
  if( !map ) map = fCmdList;
  Int_t i = 0;
  Int_t j = -1;
  // do all maps until null terminator or match found
  while( map[i] ){
    if( (j = Map2Key( function, map[i] )) != -1 ) break;
    i++;
  }
  return j;
}

//-----------------------------------------------------------------------------
Char_t* TA2System:: Map2String(Int_t key, const Map_t* map)
{
  // Return string matched to integer index from map array
  // Null return on error
  //
  if( !map ) map = fCmdList[0];
  if( !map ) return NULL;
  const Char_t* f;
  for( Int_t i=0; ; i++ ){
    f = map[i].fFnName;
    if( !f ) return NULL;
    if( key == map[i].fFnKey  ) return (Char_t*)f;
  }
}

//-----------------------------------------------------------------------------
Int_t TA2System:: GetMapKey(Int_t i, const Map_t* map)
{
  // get key from i-th element of map
  //
  if( !map ) map = fCmdList[0];
  if( !map ) return -1;
  return map[i].fFnKey;
}
  
//-----------------------------------------------------------------------------
Char_t* TA2System:: GetMapString(Int_t i, const Map_t* map)
{
  // get string from i-th element of map
  //
  if( !map ) map = fCmdList[0];
  if( !map ) return NULL;
  return (Char_t*)map[i].fFnName;
}
  
//---------------------------------------------------------------------------
Char_t* TA2System::BuildName( const Char_t* c0, const Char_t* c1, 
			      const Char_t* c2, const Char_t* c3, 
			      const Char_t* c4, const Char_t* c5,
			      const Char_t* c6, const Char_t* c7 )
{
  // Concatenate up to 8 strings to build file names etc.
  // Check that concatenated string length doesn't exceed maximum
  //
  enum {maxlength = 1024};                     // max char buffer length
  const Char_t* c[] = { c0, c1, c2, c3, c4, c5, c6, c7 };
  Int_t i;
  Int_t length = 0;                           // concatenated string length
  Int_t nstring = sizeof(c)/sizeof(Char_t*);  // max number strings
  
  for( i=0; i<nstring; i++){                  // find total length
    if( c[i] ) length += strlen( c[i] );
    else break;                               // 1st NULL terminates
  }
  if( (length == 0) || (length >= maxlength) ){
    PrintError( c0, "<BuildName: max string length exceeded>" );
    return NULL;
  }
  
  Char_t* buildline = new Char_t[length + 1]; // create string buffer
  strcpy( buildline, c[0] );                  // and fill it
  for( i=1; i<nstring; i++ ){
    if( c[i] ) strcat( buildline, c[i] );
    else break;
  }  
  return buildline;                           // return ptr to string buffer
}

//---------------------------------------------------------------------------
const Char_t*  TA2System::CheckName(const Char_t* name )
{
  // Basic file-name checker.
  // Strips off device/directory spec, brief check for tape spec.
  //
  const Char_t* a;
  
  if((a = strrchr(name,'/')) == NULL)  // check for last '/'
    a = name;                          // not found, start at 1st char
  else
    a++;                               // advance past '/'
  if(strlen(a) == 0) return NULL;      // nothing there
  if(strstr(a,"rmt")) return NULL;     // tape spec ?
  if(strstr(a,"nst")) return NULL;     // tape spec ?
  
  return a;
}

ClassImp(TA2System)
