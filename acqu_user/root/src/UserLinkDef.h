//--Author	JRM Annand    9th Sep 2004
//--Rev
//--Update
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// ROOT dictionary specification
// All classes linked in libUserRoot.so must included in the ROOT
// dictionary so that they are recognised by the CINT command processor

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Global pointers to core TAcquRoot and TA2Analysis classes
#pragma link C++ global gAR;
#pragma link C++ global gAN;
#pragma link C++ global gUAN;

// Any new user written class must be included in this list
// Main control and analysis
#pragma link C++ class TA2UserControl+;
#pragma link C++ class TA2UserAnalysis+;

// Physics classes
#pragma link C++ class TA2Pi0Compton+;
#pragma link C++ class TA2OnlinePhys+;
#pragma link C++ class TA2UserPhysics+;
#pragma link C++ class TA2MesonPhysics+;
#pragma link C++ class TA2AccessSQL+;
#pragma link C++ class TA2GoAT+;
#pragma link C++ class TA2BasePhysics+;
#pragma link C++ class TA2TriggerPhysics+;
#pragma link C++ class TA2TAPSAnalysis+;
// TA2My... extension classes
//#pragma link C++ class TA2MyAnalysis+;
#pragma link C++ class TA2MyCaLib+;
#pragma link C++ class TA2MyPhysics+;
// Apparatus classes
#pragma link C++ class TA2GenericApp+;
#pragma link C++ class TA2Calorimeter+;
#pragma link C++ class TA2CosmicCal+;
#pragma link C++ class TA2CrystalBall+;
#pragma link C++ class TA2CB+;
#pragma link C++ class TA2Taps+;
#pragma link C++ class TA2CentralApparatus+;
#pragma link C++ class TA2BeamPolMon+;
#pragma link C++ class TA2LinearPolEpics+;

// Detector classes
#pragma link C++ class TA2PlasticPID+;
#pragma link C++ class TA2PlasticVETO+;
#pragma link C++ class TA2CalArray+;
#pragma link C++ class TA2TAPS_BaF2+;
#pragma link C++ class TA2TAPS_Veto+;
#pragma link C++ class TA2FPMicro;
#pragma link C++ class TA2PairSpec+;
#pragma link C++ class TA2Moeller+;
// Utility classes...components of detectors etc
#pragma link C++ class TA2Event+;
#pragma link C++ class TA2KFParticle+;
#pragma link C++ class TA2CBKinematicFitter+;
#pragma link C++ class TA2MwpcTrack++;
#pragma link C++ class TA2CylMwpc++;
#pragma link C++ class TA2CylMwpcWire;
#pragma link C++ class TA2CylMwpcStrip++;
#pragma link C++ class TA2CentralTrack++;
#pragma link C++ class TA2TrackLine++;
#pragma link C++ class TA2MwpcIntersection++;
#pragma link C++ class TA2GeomCalibPhysics+;
#pragma link C++ class TA2CylMwpcLayer+;


#endif
