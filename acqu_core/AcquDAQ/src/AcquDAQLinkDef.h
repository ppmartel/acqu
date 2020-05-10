//--Author	JRM Annand   30th Sep 2005  Start AcquDAQ
//--Rev 	JRM Annand
//--Rev 	JRM Annand...11th Jan 2008  V775 V1190 addition
//--Rev 	B.Oussena    13th Jul 2010  FB-18XX, 4413SPILL addition
//--Rev 	JRM Annand   25th Jan 2012  GSI4800 scaler addition
//--Rev 	JRM Annand    9th Jan 2013  CAEN V874 (TAPS) addition
//--Rev 	K Livingston..7th Feb 2013  Support for handling EPICS buffers
//--Rev 	JRM Annand  ..6th Jul 2013  CAEN V965 addition
//--Rev 	JRM Annand  ..3rd Sep 2013  VITEC addition
//--Rev 	JRM Annand  .22nd Sep 2013  VUPROMT (TAPS) addition
//--Rev 	JRM Annand  ..3rd Nov 2016  add TDAQ_VPE2xX86_64+, TVME_V785
//--Update	JRM Annand  .19th Oct 2017  ensure SIS1100 and V1290 added
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// List for CINT linkage of AcquDAQ classes

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

//#pragma link C++ global gEXP;

//#pragma link C++ class TDAQsystem+;
#pragma link C++ class TDAQexperiment+;
#pragma link C++ class TDAQsupervise+;
#pragma link C++ class TGUIsupervise+;
//#pragma link C++ class TDAQmemmap+;
#pragma link C++ class TDAQmodule+;
#pragma link C++ class TVirtualModule+;
#pragma link C++ class TEPICSmodule+;
#pragma link C++ class TDAQcontrol+;
#pragma link C++ class TDAQstore+;
//#pragma link C++ class TDAQsocket+;
//#pragma link C++ class TDAQCtrlMod+;
#pragma link C++ class TVMEmodule+;
#pragma link C++ class TCAMACmodule+;
//
#pragma link C++ class TDAQ_V2718+;
#pragma link C++ class TDAQ_KPhI686+;
#pragma link C++ class TDAQ_VPE2xX86_64+;
#pragma link C++ class TDAQ_VPE2xX86_64A+;
#pragma link C++ class TDAQ_SIS1100+;
#pragma link C++ class TDAQ_SY1527+;
#pragma link C++ class TVME_CBD8210+;
#pragma link C++ class TVME_KPhSMI+;
#pragma link C++ class TVME_V792+;
#pragma link C++ class TVME_V965+;
#pragma link C++ class TVME_V775+;
#pragma link C++ class TVME_V785+;
#pragma link C++ class TVME_V874+;
#pragma link C++ class TVME_V1190+;
#pragma link C++ class TVME_V1290+;
#pragma link C++ class TVME_CATCH+;
#pragma link C++ class TVME_CATCH_TDC+;
#pragma link C++ class TVME_CATCH_Scaler+;
#pragma link C++ class TVME_CATCH_TCS+;
#pragma link C++ class TVME_GeSiCA+;
#pragma link C++ class TVME_VUPROM+;
#pragma link C++ class TVME_VUPROMT+;
#pragma link C++ class TVME_VUPROM_Scaler+;
#pragma link C++ class TVME_VUPROM_Moeller+;
#pragma link C++ class TVME_VUPROM_Pattern+;
#pragma link C++ class TVME_SIS3820+;
#pragma link C++ class TVME_VITEC+;
//
#pragma link C++ class TCAMAC_4508+;
#pragma link C++ class TCAMAC_2373+;
#pragma link C++ class TCAMAC_2323+;
#pragma link C++ class TCAMAC_4413SPILL+;
#pragma link C++ class TCAMAC_4413DAQEnable+;
#pragma link C++ class TCAMAC_GSI4800+;
//
#pragma link C++ class TFBmodule+;
#pragma link C++ class TFB_1821SMI+;
#pragma link C++ class TFB_LRS1800+;
#pragma link C++ class TFB_Phil10C+;
#pragma link C++ class TFB_STR200+;
//
#pragma link C++ class TDAQguiCtrl+;
//
#pragma link C++ class DAQMemMap_t+;

#endif
