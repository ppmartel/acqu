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
//

#ifndef __TA2System_h__
#define __TA2System_h__

#include "TROOT.h"
#include "TNamed.h"
#include "EnumConst.h"
#include "TMath.h"
#include <cstdlib>

// Struct to associate character string with integer key
struct Map_t{
  const Char_t* fFnName;
  const Int_t fFnKey;
};

class ARFile_t;
class ARSocket_t;
class DAQMemMap_t;

class TA2System : public TNamed {
  friend class ARFile_t;
  friend class ARSocket_t;
  friend class DAQMemMap_t;
 private:
  const Map_t* fCmdList[EMaxCmdList+1];
 protected:
  ARFile_t* fInputFile;                    // config input file if any
  ARFile_t* fLogFile;                      // diagnostic log file
  FILE* fLogStream;                        // file ptr for logging output
  void BaseInit( const Map_t*, const Char_t* );  // 1st-level initialisation
  void PrintError( const Char_t*, const Char_t* = NULL, Int_t = EErrNonFatal,
                   Bool_t cntSuppr = kFALSE );
  void PrintMessage( const Char_t*, Bool_t cntSuppr = kFALSE ); // general message
  Bool_t fIsInit;                          // initialisation complete?
  Bool_t fIsError;                         // any setup errors?
  Bool_t fIsConfigPass;                    // depreciated config handler
  Int_t fNPrintMsg;                        // number of printed (error) messages
  Int_t fMaxPrintMsg;                      // limit on number of printed (error) messages
 public:
  TA2System( const Char_t*, const Map_t*, const Char_t* = NULL, const Char_t* = NULL );
  TA2System( const Char_t*, const Map_t*, const Char_t*, FILE* );
  virtual ~TA2System();
  Int_t Map2Key( Char_t*, const Map_t* = NULL);    // get key from string
  Int_t Map2Key( Char_t*, const Map_t**);          // get key from string
  Char_t* Map2String( Int_t, const Map_t* = NULL); // get string from key
  Int_t GetMapKey(Int_t, const Map_t* = NULL);     // get key from index
  Char_t* GetMapString(Int_t, const Map_t* = NULL);// get string from index
  void AddCmdList( const Map_t* );                 // add command map
  void FileConfig( Char_t* = NULL );               // Setup class from file
  void CommandConfig( Char_t* = NULL );            // Setup class from tty
  virtual void ParseMisc( char* ){ }               // wild-card initialisation
  virtual void SetConfig( Char_t*, Int_t ) = 0;    // class-dependent setup
  virtual void PostInit(){ fIsInit = kTRUE; }      // class initialisation
  // Text input handling stuff
  Char_t* BuildName( const Char_t*, const Char_t* = NULL, const Char_t* = NULL,
		     const Char_t* = NULL, const Char_t* = NULL,
		     const Char_t* = NULL, const Char_t* = NULL,
		     const Char_t* = NULL);          // string construction
  const Char_t* CheckName(const Char_t* name );
  // Inlined getters
  ARFile_t* GetInputFile(){ return fInputFile; }
  ARFile_t* GetLogFile(){ return fLogFile; }
  FILE* GetLogStream(){ return fLogStream; }
  void SetLogStream( FILE* str ){ fLogStream = str; }
  void SetLogFile( const Char_t* = NULL );
  Bool_t IsInit(){ return fIsInit; }
  Bool_t IsError(){ return fIsError; }

  ClassDef(TA2System,1)   
};

#endif

