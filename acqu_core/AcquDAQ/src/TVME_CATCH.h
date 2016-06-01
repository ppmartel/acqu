//--Author	JRM Annand      7th Jun 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand..   28th Apr 2009..remove TDAQmemmap.h
//--Rev 	JRM Annand..   25th Oct 2009..further development
//--Rev         B. Oussena          Jun 2010  Added new cmd EcatchMapSize
//                                            Added virtual mapping access 
//                                            Fixed an error in TVME_CATCH()
//--Rev         B. Oussena    24th Jul 2010  PostInit() performs only mapping 
//--Rev         B. Oussena    26th Jul 2010  fix front panel id display: UInt_t
//--Rev         B. Oussena    13th Aug 2010  more cleaning 
//--Rev         JRM Annand     5th Sep 2010  Bug constructer rm fSpyData=NULL 
//--Rev         JRM Annand     6th Sep 2010  fMaxSpy = spy buffer size
//                                           Add GetEventID()
//--Rev         JRM Annand     8th Sep 2010  Set TCS event ID in spy-buffer read
//--Rev         JRM Annand    12th Sep 2010  New WrtCatch(UInt_t, UInt_t)
//--Rev         JRM Annand     6th Feb 2011  Pause = usleep(100)...was usleep(1)
//                                           Use WrtCatch(UInt_t,UInt_t) with
//                                           usleep(1) for reg programming
//--Rev         JRM Annand     1st May 2011  Command codes start 200
//--Rev         JRM Annand     5th Jul 2011  gcc4.6-x86_64 ULong_t <-> UInt_t*
//--Update      JRM Annand     2nd Sep 2012  Send event ID to remote
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CATCH
// COMPASS CATCH hardware
//

#ifndef __TVME_CATCH_h__
#define __TVME_CATCH_h__

#include "TVMEmodule.h"

enum {
  ECATCH_BuffSize = 0x1000,   	// To hold Catch Buffer
  ECATCH_ScalerSize = 147,      // expected length of scaler data buffer
  ECATCH_Trailer = 0xcfed1200,	// end of data marker
  ECATCH_ErrFlag = 0x80000000,	// error bit in catch datum
  ECATCH_CmdSize = 0x100,       // size of Catch write-cmd array
  ECATCH_SpyReset = 0x63,       // reset (flush) the spy buffer
  ECATCH_CMCReset = 0xf,        // reset CMC A,B,C,D
  ECATCH_Timeout = 1000000,
};

// Catch setup command codes

enum{ EFPGAFile=200, ETCSFile, ETCSFile1, ETCSFile2, ETCSRecFile, EF1File, 
      ECatchWrt, ECatchParm, ECatchMapSize};//<--- Baya

// Communications port
enum { ECATCHport = 1, ETCSport = 2, ESLINKport = 3};

// Default register indices
enum {
  ECATCH_IID,
  ECATCH_IStatus,
  ECATCH_ISpyCtrl,
  ECATCH_ISpyData,
  ECATCH_IF1Setup,
};

//-----------------------------------------------------------------------
// struct to hold Catch setup address, data and catch-id
//-----------------------------------------------------------------------
struct CatchWrt_t{
  UInt_t* addr;              // virtual address
  UInt_t value;              // value to write
};

class TVME_CATCH : public TVMEmodule {
 protected:
  CatchWrt_t* fCWrt;                // register write commands
  UInt_t* fSpyData;                 // VME data buffer
  UInt_t fMaxSpy;                   // max length spy data
  Int_t fIWrt;                      // register writes counter
  UInt_t fCatchID;                  // ID of mother board
  UInt_t fTCSEventID;
  Char_t* fFPGAfile;
  Char_t* fTCSRecFile;
  Char_t* fF1File;
  //  Long_t VirtAddr;    //<<----------------------  Baya
 public:
  TVME_CATCH( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_CATCH();
  virtual void SetConfig( Char_t*, Int_t );   // configure VME module
  virtual void PostInit( );
  void InitCatchWrt( Char_t* );
  Int_t WrtCatch( Int_t );
  void WrtCatch( UInt_t, UInt_t );
  Int_t ProgFPGA(Char_t*, Int_t );
  void Pause();
  void Din(ULong_t, Int_t);
  Int_t InitFlag(Int_t, Int_t);
  Int_t Done(Int_t, Int_t);
  Int_t Busy(Int_t);
  void Din32(Int_t, UInt_t);
  Int_t Fileout(FILE*, Int_t, Int_t, Int_t);
  //  UShort_t* GetCtrl1(){ return fCtrl1; }
  //  virtual void SpyError( UInt_t*, Int_t, Int_t );
  virtual void SpyReset( );
  virtual Int_t SpyRead( void** );
  virtual UInt_t GetEventID(){ return fTCSEventID; }
  ClassDef(TVME_CATCH,1)   

    };
/*
//---------------------------------------------------------------------------
inline void TVME_CATCH::SpyError( UInt_t* spyctrl, Int_t header,
				  Int_t error )
{
  // CATCH Error handler...
  // Write standard ACQU error block to the data and ensure
  // that the spy buffer is reset

  //ErrorStore(void** out, error );
  //  *spyctrl = CATCH_SPY_RESET;
  //  return;
}
*/

//---------------------------------------------------------------------------
inline void TVME_CATCH::SpyReset( )
{
  // Reset the spy buffers of all Catch TDC's
  Write( ECATCH_ISpyCtrl, ECATCH_SpyReset );
  return;
}

//---------------------------------------------------------------------------
inline Int_t TVME_CATCH::SpyRead( void** outBuff )
{
  UInt_t status = Read(ECATCH_IStatus);            // status register
  UInt_t nWordStatus = status & 0xffff;            // # words currently in spy
  if( nWordStatus == 0 ){
    ErrorStore( outBuff, 6 ); // error code 6
    fData[ECATCH_ISpyCtrl] = ECATCH_SpyReset;
    Write(ECATCH_ISpyCtrl);                         // ensure spy buffer reset
    return 0;
  }

  /*
  UInt_t nWordTries = 0;
  while(nWordStatus == 0){
    status = Read(ECATCH_IStatus);
    nWordStatus = status & 0xffff;    
    if(nWordTries>200) {
      ErrorStore( outBuff, 7 );                    // error code 7
      return 0;
    }
    nWordTries++;
  }
  */

  // Transfer data from Spy buffer to local data buffer
  //  UInt_t* spydata = creg->Data;
  UInt_t header = Read(ECATCH_ISpyData);          // could be zero
  if( !header ) header = Read(ECATCH_ISpyData);   // if so try again
  if( header & ECATCH_ErrFlag ) {                 // immediate error check
    ErrorStore( outBuff, 1);                      // error code 1
    return 0;
  }

  UInt_t nWordHeader = header & 0xffff;           // # words expected in spy
  nWordStatus--;

  if( !header ){                                  // header still zero, why?
    ErrorStore( outBuff, 5 );                     // error code 5
    return 0;
  }

  if(nWordHeader != nWordStatus) fprintf(fLogStream,"CATCH - %d words in header, %d words in status\n",nWordHeader,nWordStatus);

  if( nWordHeader > fMaxSpy ){                          // overflow ?
    ErrorStore( outBuff, 2 );                     // error code 2
    return 0;
  }
  // Make nWordHeader reads from the spy buffer
  for( UInt_t n=0; n<nWordHeader; n++ ) fSpyData[n] = Read(ECATCH_ISpyData);

  // Check that trailer is the last word in buffer
  if(fSpyData[nWordHeader-1] != ECATCH_Trailer){
    ErrorStore( outBuff, 3 );                     // error code 3
    return 0;
  }

  // Further error check
  if((fSpyData[1]&0x00ff0000)>>16){
    ErrorStore( outBuff, 4 );                     // error code 4
    return 0;
  }

  fData[ECATCH_ISpyCtrl] = ECATCH_SpyReset;
  Write(ECATCH_ISpyCtrl);                         // ensure spy buffer reset
  fTCSEventID = fSpyData[0];                      // 1st word = TCS event ID
  // if the system specifies that this catch sends the event ID to a remote
  // system do it here
  if( fEventSendMod ) fEventSendMod->SendEventID( fTCSEventID );
  return nWordHeader;
}

//---------------------------------------------------------------------------
inline void TVME_CATCH::WrtCatch( UInt_t offset, UInt_t value )
{
  // Single write to register
  // This is to implement "extra" configs when trying to get everything in synch
  // as in the old config_tcs script run on lynxos systems
  Write( GetVirtAddr(offset), &value, 0x39, 4 );
  //usleep(1);
}

#endif
