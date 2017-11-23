#ifndef __TA2Control_h__
#define __TA2Control_h__

//--Author	JRM Annand    4th Apr 2003
//--Rev 	JRM Annand... 1st Oct 2003  Interface TA2Analysis
//--Rev 	JRM Annand... 8th Jan 2004  More flexible analyser choice
//--Rev 	JRM Annand... 9th Mar 2004  default online
//--Rev 	JRM Annand...18th Oct 2004  banner print, gAR->SetIsOnline
//--Rev 	JRM Annand... 1st Dec 2004  MC analysis no longer needed
//--Rev 	JRM Annand...18th Feb 2005  Mods for batch processing
//--Rev 	JRM Annand... 3rd Mar 2005  Mods for batch processing
//--Rev 	JRM Annand...11th Mar 2005  Mod default TRint call
//--Rev 	JRM Annand...22nd Jan 2007  4v0 changes
//--Rev 	JRM Annand...17th May 2007  Offline...check LinkADC worked
//--Rev 	JRM Annand... 6th Jul 2007  MC check manual ADC initialisation
//--Rev 	JRM Annand...20th Nov 2007  MC=offline, into TDAQexperiment
//--Rev 	JRM Annand...29th Nov 2007  LinkDataServer() to gAR->Start()
//--Update	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data.
//
// TA2Control
//
// Provide a ROOT command-line interface for Acqu-Root analyser.
// Create TAcquRoot Acqu-Root interface and TA2Analysis data analyser.
// Attempt to configure and run the system as a separate thread. 

#include "TRint.h"
//#include "AcquKeys.h"
#include "TAcquRoot.h"


// Global pointers recognised by CINT
R__EXTERN class TAcquRoot*   gAR;         // global ptr to Acqu-Root interface
R__EXTERN class TA2Analysis*  gAN;        // global ptr to Acqu-Root analyser
R__EXTERN class TA2DataServer*  gDS;      // global ptr to DataServer
R__EXTERN class TDAQexperiment*  gDAQ;    // global ptr to local DAQ
R__EXTERN class TA2Control*  gCTRL;       // global ptr to local application

struct Map_t;

class TA2Control : public TRint {

protected:
  static TA2Control*  fExists;            // check interface already existing?
  Bool_t fBatch;
public:
  TA2Control( const char* appClassName, int* argc, char** argv,
		void* options = NULL, int numOptions = -1, 
		Bool_t noLogo = 0 );
  virtual ~TA2Control();
  virtual void PrintLogo();
  virtual Int_t GetAnalyser(const Map_t*);
  virtual void CreateAnalyser();
  virtual void StartAnalyser();

  ClassDef(TA2Control,1)
};

#endif
