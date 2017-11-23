//--Author	JRM Annand    1st Dec 2004
//--Update	
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2GenericApparatus
//
// Generic apparatus class which may be instantiated
//

#include "TA2GenericApparatus.h"
#include "TAcquRoot.h"
#include "TA2GenericDetector.h"
#include "TA2LongScint.h"

// Default list of detector classes that the TA2GenericApparatus 
// apparatus may contain
enum { EGeneric, ELongScint };
static Map_t kValidDetectors[] = {
  {"TA2GenericDetector", EGeneric},
  {"TA2LongScint",       ELongScint},
  {NULL, 		-1}
};


//-----------------------------------------------------------------------------
TA2GenericApparatus::TA2GenericApparatus( const char* name, TA2System* analysis )
  :TA2Apparatus( name, analysis, kValidDetectors )
{
  // Pointers to detectors not initialised
  fDet = NULL;
  fNDet = 0;
}


//-----------------------------------------------------------------------------
TA2GenericApparatus::~TA2GenericApparatus()
{
  // Delete any owned "new" memory here
  DeleteArrays();
}

//-----------------------------------------------------------------------------
void TA2GenericApparatus::DeleteArrays()
{
  // Delete any owned "new" memory here
  TA2Apparatus::DeleteArrays();
  if( fDet ){
    for(Int_t i=0; i<fNDet; i++) delete fDet[i];
    delete fDet;
  }
}

//-----------------------------------------------------------------------------
TA2DataManager*  TA2GenericApparatus::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the TA2GenericApparatus
  // Valid detector types in the following switch
  // Create detector list if not already done.
  // Chech if the max number detectors exceeded

  if( !fDet ) fDet = new TA2Detector*[EMaxGenericDetectors];
  if( fNDet >= EMaxGenericDetectors ){
    PrintError( "<Too many detectors in Generic apparatus>");
    return NULL;
  }

  if( !name ) name = Map2String( dclass );
  switch (dclass){
  default:
    return NULL;
  case EGeneric:
    fDet[fNDet] = new TA2GenericDetector( name, this );
    break;
  case ELongScint:
    fDet[fNDet] = new TA2LongScint( name, this );
    break;
  }
  fNDet++;
  return fDet[fNDet-1];
}

//-----------------------------------------------------------------------------
void TA2GenericApparatus::PostInit( )
{
  // Some further initialisation after all setup parameters read in
  // default Cut setup
  TA2Apparatus::PostInit();
}


//-----------------------------------------------------------------------------
void TA2GenericApparatus::Reconstruct( )
{
  // The needful goes here
}

ClassImp(TA2GenericApparatus)
