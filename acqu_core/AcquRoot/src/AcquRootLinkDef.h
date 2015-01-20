//--Author	JRM Annand    4th Feb 2003
//--Rev 	JRM Annand...21st Jan 2007   4v0 update remove net socket
//--Rev 	JRM Annand...21st Jan 2007   Add gDAQ, gCTRL
//--Update	JRM Annand... 6th Mar 2013   Add TA2TAPSMk2Format
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// AcquRootLinkDef.h
// AcquRoot dictionary generation
//
#ifdef __CINT__
//
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
//
// Global pointers
//
#pragma link C++ global gAR;
#pragma link C++ global gAN;
#pragma link C++ global gDS;
#pragma link C++ global gDAQ;
#pragma link C++ global gCTRL;
//
// Main system classes
//
#pragma link C++ class TA2System+;
#pragma link C++ class TA2HistManager+;
#pragma link C++ class TA2DataManager+;
#pragma link C++ class TA2Control+;
#pragma link C++ class TAcquRoot+;
#pragma link C++ class TA2Analysis+;
//
// Data reconstruction classes
//
#pragma link C++ class TA2Physics+;
#pragma link C++ class TA2GenericPhysics+;
#pragma link C++ class TA2Apparatus+;
#pragma link C++ class TA2GenericApparatus+;
#pragma link C++ class TA2Tagger+;
#pragma link C++ class TA2Detector+;
#pragma link C++ class TA2GenericDetector+;
#pragma link C++ class TA2ClusterDetector+;
#pragma link C++ class crystal_t+;
#pragma link C++ class TA2LongScint+;
#pragma link C++ class TA2Ladder+;
#pragma link C++ class TA2WireChamber+;
#pragma link C++ class TA2H+;
#pragma link C++ class TA2Cut+;
#pragma link C++ class TA2MultiCut+;
#pragma link C++ class TA2BitPattern+;
#pragma link C++ class TA2RateMonitor+;
#pragma link C++ class TA2ParticleID+;
#pragma link C++ class TA2Particle+;
#pragma link C++ class TA2Track+;
#pragma link C++ class TA2WCLayer+;
#pragma link C++ class TA2CylWire+;
#pragma link C++ class TA2CylStrip+;
//
// Data Server classes
//
#pragma link C++ class TA2DataServer+;
#pragma link C++ class TA2DataSource+;
#pragma link C++ class TA2DataFormat+;
#pragma link C++ class TA2NetSource+;
#pragma link C++ class TA2FileSource+;
#pragma link C++ class TA2LocalSource+;
#pragma link C++ class TA2Mk1Format+;
#pragma link C++ class TA2Mk2Format+;
#pragma link C++ class TA2TAPSFormat+;
#pragma link C++ class TA2TAPSMk2Format+;
//
#endif
