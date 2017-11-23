//--Author	JRM Annand    9th Jan 2003
//--Rev		JRM Annand...26th Feb 2003...1st "production" version
//--Rev 	JRM Annand...21st Jan 2007...Included in TA2System
//--Update	JRM Annand... 8th Feb 2013...Add EPICS buffer delimiter
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// EnumConst.h
//
// Constants used by AcquRoot

#ifndef _EnumConst_h_
#define _EnumConst_h_


enum { EMk1 = 1, EMk2 };               // Acqu data formats
enum { EMaxCmdList = 32 };             // max number different command lists
enum { ELineSize = 1024 };             // length in characters of line of text
enum { EKeyWordSize = 256 };           // length in characters of keyword
enum { EErrNonFatal, EErrFatal };      // error level
enum { ENOKeyWord, EKeyWord };         // Config file readin...line format
enum { ENsem = 16 };                   // # of semaphores	               
enum { EHostLen = 64 };	               // max length of host name (socket)
enum { ESkDefPacket = 1024 };   // Default transfer length for stream socket
enum { ESkLocal, ESkRemote };   // local/remote socket connect
enum { ESkBacklog = 8 };        // max length pending connection queue (socket)
enum { ESkInitBuff = 2 };       // # ints in initial handshake buffer (socket)
enum { EMaxDataLength = 65536 };// max data buffer length
enum { EMaxInputFiles = 1024 }; // size of input-file pointer buffer
enum { EFalse, ETrue };         // Logic...should use kTRUE, kFALSE

// ACQU Mk1 data buffer header types and data delimiters
enum{ EHeadBuff = 0x10101010,      // header buffer (experimental parameters
      EDataBuff = 0x20202020,      // standard data buffer
      EEndBuff = 0x30303030,       // end-of-file buffer
      EKillBuff = 0x40404040,      // shut-down buffer
      EPhysBuff = 0x50505050,      // reserved
      EHeadPhysBuff = 0x60606060,  // reserved
      EMk2DataBuff = 0x70707070,   // Mk2 data buffer
      EEndEvent = 0xFFFFFFFF,      // end of event marker
      EBufferEnd = 0xFFFFFFFF,     // end of file marker
      EScalerBuffer = 0xFEFEFEFE,  // start of scaler read out
      EEPICSBuffer = 0xFDFDFDFD,   // start of EPICS read out
      EReadError = 0xEFEFEFEF,     // start of error block (hardware error)
      ENullADC = -1,               // undefined ADC value
      ENullHit = 0xFFFFFFFF,       // undefined hit index (end of hit buffer)
      ENullStore = 0x8000,         // for multi-hit ADC handling
      ENullFloat = -999999999      // optional null indicator
};

// Constants for ROOT storage and analysis
enum{ EMaxEventSize = 32768, EMaxName = 256 };

// Definitions for ADC setup
enum{ EUndefinedADC = 0,          // ADC index not registered in analysis
      EPatternADC = 0xffff,       // its a bit-pattern unit
      EForeignADC = 0x3000,       // foreign data formats
      EForeignScaler = 0x4000,    // ditto
      EMultiADC = 0x10000,        // multi-hit ADC
      EFlashADC = 0x20000         // flash ADC
}; 

#endif
