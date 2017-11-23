// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// LinkDef.h                                                            //
//                                                                      //
// CaLib dictionary header file.                                        //
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

// enums
#pragma link C++ enum ECalibDetector;
#pragma link C++ enum ERawFileType;
#pragma link C++ enum ERawFileFormat;

// typedefs
#pragma link C++ typedef CalibDetector_t;
#pragma link C++ typedef RawFileType_t;
#pragma link C++ typedef RawFileFormat_t;

// common classes
#pragma link C++ namespace TCConfig;
#pragma link C++ namespace TCUtils;
#pragma link C++ class TCFileManager+;
#pragma link C++ class TCReadConfig+;
#pragma link C++ class TCConfigElement+;
#pragma link C++ class TCReadARCalib+;
#pragma link C++ class TCWriteARCalib+;
#pragma link C++ class TCARElement+;
#pragma link C++ class TCARTimeWalk+;
#pragma link C++ class TCARNeighbours+;
#pragma link C++ class TCReadACQU+;
#pragma link C++ class TCACQUFile+;
#pragma link C++ class TCMySQLManager+;
#pragma link C++ class TCContainer+;
#pragma link C++ class TCRun+;
#pragma link C++ class TCCalibration+;
#pragma link C++ class TCCalibData+;
#pragma link C++ class TCCalibType+;
#pragma link C++ class TCCalib+;
#pragma link C++ class TCCalibPed+;
#pragma link C++ class TCCalibDiscrThr+;
#pragma link C++ class TCCalibTime+;
#pragma link C++ class TCCalibEnergy+;
#pragma link C++ class TCCalibQuadEnergy+;

// misc calibration classes
#pragma link C++ class TCCalibTargetPosition+;

// Tagger calibration classes
#pragma link C++ class TCCalibTaggerTime+;

// CB calibration classes
#pragma link C++ class TCCalibCBEnergy+;
#pragma link C++ class TCCalibCBQuadEnergy+;
#pragma link C++ class TCCalibCBTime+;
#pragma link C++ class TCCalibCBRiseTime+;
#pragma link C++ class TCCalibCBTimeWalk+;
#pragma link C++ class TCCalibCBLED+;

// TAPS calibration classes
#pragma link C++ class TCCalibTAPSEnergyLG+;
#pragma link C++ class TCCalibTAPSEnergySG+;
#pragma link C++ class TCCalibTAPSPedLG+;
#pragma link C++ class TCCalibTAPSPedSG+;
#pragma link C++ class TCCalibTAPSPedVeto+;
#pragma link C++ class TCCalibTAPSQuadEnergy+;
#pragma link C++ class TCCalibTAPSTime+;
#pragma link C++ class TCCalibTAPSLED1+;
#pragma link C++ class TCCalibTAPSLED2+;
#pragma link C++ class TCCalibTAPSCFD+;
#pragma link C++ class TCCalibTAPSPSA+;

// PID calibration classes
#pragma link C++ class TCCalibPIDPhi+;
#pragma link C++ class TCCalibPIDDroop+;
#pragma link C++ class TCCalibPIDEnergy+;
#pragma link C++ class TCCalibPIDEnergyTrad+;
#pragma link C++ class TCCalibPIDTime+;

// Veto calibration classes
#pragma link C++ class TCCalibVetoCorr+;
#pragma link C++ class TCCalibVetoEnergy+;
#pragma link C++ class TCCalibVetoTime+;
#pragma link C++ class TCCalibVetoLED+;

// Misc
#pragma link C++ class TIndicatorLine+;

#endif

