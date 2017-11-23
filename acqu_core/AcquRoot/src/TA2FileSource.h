//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand      22nd Jan 2007   4v0 update
//--Update	JRM Annand       7th Jan 2013   Implement max number of buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2FileSource
// Specify data stream from disk or tape file for AcquRoot analysis
//
//---------------------------------------------------------------------------

#ifndef _TA2FileSource_h_
#define _TA2FileSource_h_

#include "TA2DataSource.h"
#include <string>

class ARFile_t;

class TA2FileSource : public TA2DataSource
{
 protected:
  Char_t** fFilename;			        // input file name list
 public:
  TA2FileSource( Char_t*, Int_t, Int_t=EFalse, Int_t=1, Int_t=EFalse );
  virtual ~TA2FileSource();
  void Process();				  // main data processor
  virtual void InputList(Char_t*, UInt_t, UInt_t);// get input file list
  void GetFileInfo(Int_t i, std::string& filename, UInt_t &start, UInt_t &stop);
  
  ClassDef(TA2FileSource,1)
};

#endif
