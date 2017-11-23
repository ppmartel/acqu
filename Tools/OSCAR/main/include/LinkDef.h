// SVN Info: $Id: LinkDef.h 1633 2013-01-24 14:00:17Z werthm $

/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// LinkDef.h                                                            //
//                                                                      //
// OSCAR dictionary header file.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifdef __CINT__ 

// turn everything off
#pragma link off all globals; 
#pragma link off all classes; 
#pragma link off all functions; 
#pragma link off all typedef; 

#pragma link C++ nestedclasses; 
#pragma link C++ nestedtypedef; 

// Module A2
#pragma link C++ enum EA2Detector; 
#pragma link C++ typedef A2Detector_t; 
#pragma link C++ enum EA2TAPSType; 
#pragma link C++ typedef A2TAPSType_t; 
#pragma link C++ enum EA2TrigBitL1; 
#pragma link C++ typedef A2TrigBitL1_t; 
#pragma link C++ enum EA2TrigBitL2; 
#pragma link C++ typedef A2TrigBitL2_t; 
#pragma link C++ enum EA2BeamHelBit; 
#pragma link C++ typedef A2BeamHelBit_t; 
#pragma link C++ class TOA2Detector+; 
#pragma link C++ class TOA2Trigger+; 
#pragma link C++ class TOA2BaseDetParticle+; 
#pragma link C++ class TOA2DetParticle+; 
#pragma link C++ class TOA2RecParticle+; 
#pragma link C++ class TOA2RecMeson2g+;
#pragma link C++ class TOA2PSA+; 

// Module Main
#pragma link C++ namespace TOGlobals; 

// Module MC
#pragma link C++ class TOMCParticle+; 

// Module Utils
#pragma link C++ enum EOKinCut; 
#pragma link C++ typedef OKinCut_t; 
#pragma link C++ namespace TOSUtils; 
#pragma link C++ namespace TOLoader; 
#pragma link C++ class TOASCIIReader+; 
#pragma link C++ class TONumberReader+; 
#pragma link C++ class TOArray2D-;
#pragma link C++ class TOKinCut+;

#endif

