//--Author	JRM Annand	22nd Oct 1998
//--Rev	        JRM Annand	 4th Jun 1999	New names system wrappers
//--Rev	        JRM Annand	31st Mar 1999	Separate modules for classes
//--Rev	        JRM Annand	31st Apr 2002	Compressed input files
//--Rev	        JRM Annand	15th Feb 2003	NULL-terminated lists
//--Rev	        JRM Annand	28th Feb 2003	map ReadKey
//--Rev	        S Schumann	10th Oct 2003	ReadLine() mod
//--Rev	        JRM Annand	10th Oct 2003	ReadKey addition
//--Rev 	JRM Annand	 3rd Mar 2005	line buffer flush before read
//--Rev 	JRM Annand	29th Jan 2007	convertion from TAcquFile
//--Rev 	JRM Annand	29th Nov 2007	Open-fail non fatal,check-del
//--Rev 	JRM Annand	 3rd Jun 2008	const Char_t* gcc 4.3
//--Update	JRM Annand	 1st Sep 2009	delete[]
//--Update  A Neiser...   6th June 2013  Always add filename to fatal fopen 
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// ARFile_t
// Wrapper class for file handling UNIX system calls.
// Not included in the root dictionary
//
//---------------------------------------------------------------------------


#ifndef _ARFile_t_h_
#define _ARFile_t_h_

#define _FILE_OFFSET_BITS 64
#include "TA2System.h"                  // AcquRoot base class
#include "EnumConst.h"			// integer constant defs

// Unix system headers....these screw up the ROOT dictionary generator
// hence ARFile_t is not rootified
extern "C"{
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}
 
class ARFile_t{
 private:
  FILE* fStart;		        	// file pointer from fopen();
  Char_t* fName;                        // file name
  TA2System* fSys;                      // calling class
  Int_t fPath;			        // file descriptor from open();
  Int_t fBlockSize;			// binary write block size
  Char_t fLine[ELineSize];		// read line buffer
  Char_t* fL;			        // start of line buffer
 public:
  // open for text I/O using fopen(), default read only
  ARFile_t( const Char_t*, const Char_t* = "r", TA2System* = NULL,
	    Bool_t = kFALSE );
  // open for binary I/O using open(), default create if non-existent r/w
  ARFile_t( const Char_t*, Int_t=O_RDWR|O_CREAT, 
	    mode_t=S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, TA2System* = NULL );
  virtual ~ARFile_t();	        	// closes the file
  Char_t* ReadLine();		        // get line of text
  Int_t ReadKey( const Map_t* );        // key from file keyword via Map_t
  Int_t ReadKey( const Map_t** );       // key from file keyword via Map_t
  void AdvanceLine( Char_t* );          // check line ptr after keyword found
  // Short Inline functions
  Char_t* GetTotalLine(){ return fLine; } // original start of line
  Char_t* GetLine(){ return fL; }	  // current position on line
  FILE* GetStart(){ return fStart; }      // start of file
  Char_t* GetName(){ return fName; }      // name of file
  Int_t GetPath(){ return fPath; }    	  // path descriptor
  void Rewind(){ rewind(fStart); }        // reposition at start of file
  // clear line buffer
  void FlushLine(){ for(Int_t i=0; i<ELineSize; i++) fLine[i] = 0; }
  // clear line buffer and read in line from file
  Char_t* GetString(){ FlushLine(); return fgets(fLine, ELineSize, fStart); }
  // call TA2System error handler
  void PrintError( const Char_t*, Int_t = EErrNonFatal );
  void WriteBuffer( void* buff, Int_t recl ){
    if( write( fPath, buff, recl ) != recl )
      PrintError("<Incomplete binary write to data file>", EErrFatal);
  }
};

#endif
