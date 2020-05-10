//--Author	JRM Annand   20th Dec 2005
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 7th Feb 2007  Add generic VMEbus 0x0000
//--Rev 	JRM Annand...17th May 2007  Add virtual module
//--Rev 	JRM Annand...11th Jan 2008  Add CAEN V775
//--Rev 	B Oussena.   Jan -Mar 2010  added some news modules
//--Rev         B Oussena.   13th Jul 2010  added LRS_4413,LRS_4413SPILL
//--Rev         JRM Annand   27th Jul 2010  added LRS_1880
//--Rev         B.Oussena    2nd Aug 2010  set "LRS_1885" to  ELRS_1800
//--Rev         JRM Annand   11th Sep 2010  Add EDAQ_VADC, EDAQ_VScaler
//                                          ECAMAC_4413DAQEnable
//--Rev         JRM Annand   17th Sep 2010  VME addr modifiers now here
//--Rev         JRM Annand   25th Jan 2012  Add GSI 4800
//--Rev         JRM Annand   19th May 2012  Add Vuprom
//--Rev         JRM Annand    1st Sep 2012  Add SIS 3820
//--Rev         JRM Annand    3rd Sep 2012  EDAQ_ADC_Scaler added
//--Rev         JRM Annand    9th Jan 2013  ECAEN_V874 added
//--Rev 	K Livingston..7th Feb 2013  Support for handling EPICS buffers
//--Rev 	JRM Annand  ..6th Jul 2013  ECAEN_V965 added
//--Rev 	JRM Annand  ..3rd Sep 2013  VITEC added
//--Rev 	JRM Annand  .22nd Sep 2013  VUPROMT added
//--Rev 	JRM Annand  .14th Oct 2016  TDAQ_VPE2xX86_64 added
//--Rev 	JRM Annand  ..3rd Nov 2016  TVME_V785 added
//--Rev 	JRM Annand  .16th Aug 2017  TDAQ_SIS1100 added
//--Update	JRM Annand  .19th Oct 2017  Ensure TVME_V1290 added
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// ModuleIndex.h
// Electronic hardware identification
// General scheme  0000 - 0fff     VMEbus
//                 1000 - 1fff     CAMAC
//	           2000 - 2fff     FASTBUS
//                 X000 - X0ff     CES interfaces
//                 X100 - X1ff     LeCroy
//                 X200 - X2ff     Phillips
//                 X300 - X3ff     CAEN
//		   X400 - X4ff     Mainz Kph
//		   X500 - X5ff     INFN
//		   X600 - X6ff     Struck
//		   X700 - X7ff     GanElec
//		   X800 - X8ff     SEN
//		   X900 - X9ff     Hytec
//		   Xa00 - Xaff     Borer
//		   Xb00 - Xbff     CEN Saclay
//		   Xc00 - Xcff     GSI
//		   Xd00 - Xdff     Glasgow
//		   Xe00 - Xeff     Kinetic Systems
//		   Xf00 - Xfff     Miscellaneous

#ifndef __ModuleIndex_h__
#define __ModuleIndex_h__

// Not defined
enum { EID_Undef = -1 };

// Controllers
enum { ECAEN_V2718 = 0xf000, ESIS_1100, EKPH_I686, ECAEN_SY1527, EVPE2xX86_64,
       EVPE2xX86_64A };

// VMEbus
enum { EVMEbus, ECBD_8210, EHSM_8170, EVIC_8250, ESLV_8250 };
enum { ELRS_1131 = 0x0100 };
enum { ESTR_8090 = 0x0200, ESTR_8080, ESIS_3820 };
enum { EKPH_SMI = 0x0400, EKPH_VITEC };
enum { ECAEN_1465 = 0x0300, ECAEN_1488, ECAEN_V792, ECAEN_V775, ECAEN_V1190,
       ECAEN_V874, ECAEN_V965, ECAEN_V785, ECAEN_V1290 };
enum { EINFN_56001 = 0x0500 };
enum { EFIADC_64 = 0x0600, ECATCH_TDC, ECATCH_SCA, EGeSiCA, ECATCH_TCS };
enum { EGSI_VUPROM = 0x0700, EGSI_VUPROMT, 
       EGSI_VUPROM_Scaler, EGSI_VUPROM_Moeller, EGSI_VUPROM_Pattern };
// VMEbus address modifier codes
enum{ EVME_A32=0x9, EVME_A24=0x39, EVME_A16=0x29 };

// CAMAC  
enum { ELRS_2249 = 0x1100, ELRS_2259, ELRS_2228, ELRS_2251, ELRS_2277 };
enum { ELRS_4413 = 0x1110, ELRS_4418, ELRS_4508, ELRS_4516, ELRS_4448,
       ELRS_4434, ELRS_4416, ELRS_2341, ELRS_2373, ELRS_3420,
       ELRS_2323, ELRS_2891, ELRS_2132,ELRS_4413SPILL, ELRS_4413DAQEnable };

enum { ELRS_4299 = 0x1120, ELRS_2749 };
enum { ELRS_4300 = 0x1130, ELRS_4301, ELRS_3377, ELRS_3304 };
enum { EPH_7166  = 0x1200 };
enum { ECAEN_243 = 0x1300, ECAEN_208 };
enum { EKPH_UNSETZ = 0x1400, EKPH_PATT, EKPH_VUCAM };
enum { EGAN_FCC8 = 0x1700, EGAN_1612F, EGAN_812F, EGAN_816 };
enum { ESEN_2048 = 0x1800, ESEN_2049, ESEN_16P, ESEN_2088 };
enum { EHYT_310 = 0x1900 };
enum { EBOR_1413 = 0x1a00 };
enum { EDPHN_PU48 = 0x1b00, EDPHN_SEPMTC1 };
enum { EGSI_4800 = 0x1c00 };
enum { ECCF_8000 = 0x1d00 };
enum { E_A2CTRL = 0x1f00, E_INHIB, E_VIRTUAL };
enum { ECAMAC = 0x1fff };

// FASTBUS
enum { ELRS_1821 = 0x2100, ELRS_1800, ELRS_1875, ELRS_1872, ELRS_1885,
       ELRS_1877, ELRS_1879, ELRS_1881 };
enum { EPH_10C6 = 0x2200, EPH_10C2 };
enum { ESTR_200 = 0x2600, ESTR_136 };
enum { EFASTBUS = 0x2fff };
// The Buses
enum { E_BusUndef, E_VMEbus, E_CAMAC, E_FASTBUS, E_I2C, E_PCI, E_RS232 };

// general types (functionality) of DAQ modules
enum { EDAQ_Undef=0, EDAQ_ADC=0x1, EDAQ_Scaler=0x2, EDAQ_SlowCtrl=0x4,
       EDAQ_Ctrl=0x8, EDAQ_SCtrl=0x10, EDAQ_IRQ=0x20,
       EDAQ_VADC=0x40, EDAQ_VScaler=0x80, EDAQ_ADC_Scaler=0x3,
       EDAQ_Epics=0x100 };

// virtual module
enum { EDAQ_Virtual=0xffff };

// Name - ID association
const Map_t kExpModules[] = {
  // Controllers
  {"CAEN_V2718", ECAEN_V2718},
  {"KPH_I686",   EKPH_I686},
  {"VPE2xX86_64",EVPE2xX86_64},
  {"VPE2xX86_64A",EVPE2xX86_64A},
  {"SIS_1100",   ESIS_1100},
  // VMEbus
  {"VMEbus",     EVMEbus},
  {"CBD_8210",   ECBD_8210},
  {"HSM_8170",   EHSM_8170},
  {"VIC_8250",   EVIC_8250},
  {"SLV_8250",   ESLV_8250},
  {"LRS_1131",   ELRS_1131},
  {"KPH_SMI",    EKPH_SMI},
  {"KPH_VITEC",  EKPH_VITEC},
  {"CAEN_1465",  ECAEN_1465},
  {"CAEN_1488",  ECAEN_1488},
  {"CAEN_V792",  ECAEN_V792},
  {"CAEN_V775",  ECAEN_V775},
  {"CAEN_V785",  ECAEN_V785},
  {"CAEN_V874",  ECAEN_V874},
  {"CAEN_V1190", ECAEN_V1190},
  {"CAEN_V1290", ECAEN_V1290},
  {"CAEN_V965",  ECAEN_V965},
  {"CAEN_SY1527",ECAEN_SY1527},
  {"CAEN_208",   ECAEN_208},
  {"INFN_56001", EINFN_56001},
  {"FIADC_64",   EFIADC_64},
  {"CATCH_TDC",  ECATCH_TDC},
  {"CATCH_SCA",  ECATCH_SCA},
  {"GeSiCA",     EGeSiCA},
  {"CATCH_TCS",  ECATCH_TCS},
  {"GSI_VUPROM", EGSI_VUPROM},
  {"GSI_VUPROMT",EGSI_VUPROMT},
  {"GSI_VUPROM_Scaler", EGSI_VUPROM_Scaler},
  {"GSI_VUPROM_Moeller", EGSI_VUPROM_Moeller},
  {"GSI_VUPROM_Pattern", EGSI_VUPROM_Pattern},
  {"SIS_3820",   ESIS_3820},
  // CAMAC
  {"CAMAC",      ECAMAC},
  {"VUCAM",      EKPH_VUCAM},
  {"LRS_4508",   ELRS_4508},
  {"LRS_4516",   ELRS_4516},
  {"LRS_4434",   ELRS_4434},
  {"LRS_2341",   ELRS_2341},
  {"LRS_2373",   ELRS_2373},
  {"LRS_3420",   ELRS_3420},
  {"LRS_2323",   ELRS_2323},
  {"LRS_2891",   ELRS_2891},
  {"LRS_2132",   ELRS_2132},
  {"LRS_4413",   ELRS_4413},
  {"LRS_4413SPILL",   ELRS_4413SPILL},
  {"LRS_4413DAQEnable",   ELRS_4413DAQEnable},
  {"GSI_4800",   EGSI_4800},
 

  // FASTBUS
  {"FASTBUS",    EFASTBUS}, 
  {"LRS_1821",   ELRS_1821},
  {"LRS_1875",   ELRS_1875}, 
  {"LRS_1872",   ELRS_1872},
  {"LRS_1885",   ELRS_1800}, 
  {"LRS_1877",   ELRS_1877},
  {"LRS_1879",   ELRS_1879}, 
  {"LRS_1881",   ELRS_1881},
  {"PH_10C6",    EPH_10C6}, 
  {"PH_10C2",    EPH_10C2},
  {"STR_200",    ESTR_200},
  {"STR_136",    ESTR_136},
  // I2C
  // PCI
  {"Virtual",    EDAQ_Virtual},
  {"EPICS",      EDAQ_Epics},
};

#endif
