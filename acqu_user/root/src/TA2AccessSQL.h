//*************************************************************************
//* Author: Patrik Ott, Andreas Neiser (extracted code from TA2MyPhysics)
//*************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TA2AccessSQL                                                         //
//                                                                      //
// Class used for reading the SQL config database.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef __TA2AccessSQL_h__
#define __TA2AccessSQL_h__

#include "TOSUtils.h"

#include "TA2Physics.h"
#include "CaLibReader_t.h"
#include "TA2Tagger.h"
#include "TA2Ladder.h"
#include "TA2CentralApparatus.h"
#include "TA2CrystalBall.h"
#include "TA2CalArray.h"
#include "TA2PlasticPID.h"
#include "TA2Taps.h"
#include "TA2TAPS_BaF2.h"
#include "TA2TAPS_Veto.h"
#include "TA2LinearPolEpics.h"
#include "TA2PairSpec.h"
#include "TA2LongScint.h"
#include "TA2GenericApparatus.h"
#include "TA2GenericDetector.h"
#include "TA2Moeller.h"


enum {
  ESQL_USE_CALIB = 20200,
  ESQL_CALIB_TAGG,
  ESQL_CALIB_CB,
  ESQL_CALIB_TAPS,
  ESQL_CALIB_PID,
  ESQL_CALIB_VETO,
  ESQL_CALIB_MCRUNNR
};

static const Map_t AccessSQLConfigKeys[] = {
  // General keys
  {"Use-CaLib:"                    , ESQL_USE_CALIB},                      // key for CaLib activation
  {"Use-CaLib-TAGG:"               , ESQL_CALIB_TAGG},                     // key for CaLib tagger configuration
  {"Use-CaLib-CB:"                 , ESQL_CALIB_CB},                       // key for CaLib CB configuration
  {"Use-CaLib-TAPS:"               , ESQL_CALIB_TAPS},                     // key for CaLib TAPS configuration
  {"Use-CaLib-PID:"                , ESQL_CALIB_PID},                      // key for CaLib PID configuration
  {"Use-CaLib-Veto:"               , ESQL_CALIB_VETO},                     // key for CaLib Veto configuration
  {"MC-Run-Number:"                , ESQL_CALIB_MCRUNNR},                  // key to determine run number for MC files
  // Termination
  {NULL        , -1           }
};



class TA2AccessSQL : public TA2Physics
{
private:
  
  void	LoadDetectors(TA2DataManager* parent, Int_t depth);
  void 	ApplyCaLib();
  
protected:
  CaLibReader_t*	fCaLibReader;                           // CaLib reader
  Int_t			fRunNumber;
  Int_t fAnalysisMode;                                    // analysis mode (raw, MC, ...)
  Bool_t fIsMC;                                           // flag for MC analysis mode
  
  
  Double_t 				      fTargetPosition;  // target position in z-direction    
  TA2Tagger* 				    fTagger;          // pointer to the Tagger
  TA2Ladder* 				    fLadder;     			// pointer to the Ladder
  TA2CentralApparatus* 	fCB;    			   	// pointer to the Crystal Ball
  TA2CalArray* 			    fNaI;    					// pointer to the NaI elements
  TA2PlasticPID* 			  fPID;            	// pointer to the PID
  TA2CylMwpc*				    fMWPC;  				  // pointer to the Mwpc
  TA2Taps* 				      fTAPS; 					  // pointer to TAPS
  TA2TAPS_BaF2* 			  fBaF2PWO;     		// pointer to the BaF2 (or the BaF2/PWO) array
  TA2TAPS_Veto* 			  fVeto;         		// pointer to the TAPS Vetos
  TA2LinearPolEpics* 		fLinPol;          // pointer to the Linear Polarisation class
  TA2PairSpec* 		fPairSpec;          // pointer to the Pair Spec class  
  TA2GenericDetector* fPbWO4;                             // pointer to the PbWO4 ring of TAPS
  TA2LongScint* fTOF;                                     // pointer to the TOF wall
  TA2GenericApparatus* fPbGlassApp;                       // pointer to the Pb glass detector apparatus
  TA2GenericDetector* fPbGlass;                           // pointer to the Pb glass detector
  TA2Moeller* fMoeller; // pointer to the Moeller detector
  
public:
  TA2AccessSQL(const char* name, TA2Analysis* analysis);
  virtual ~TA2AccessSQL();
  
  virtual void SetConfig(Char_t* line, Int_t key);
  virtual void PostInit();
  virtual void LoadVariable();
  virtual	void Reconstruct();
  
  ClassDef(TA2AccessSQL, 1)
};



#endif
