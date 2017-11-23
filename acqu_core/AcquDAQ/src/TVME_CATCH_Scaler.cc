//--Author	JRM Annand	 7th Jun 2007
//--Rev 	JRM Annand
//--Rev 	JRM Annand..28th Apr 2009..remove TDAQmemmap.h
//--Rv   	JRM Annand..25th Oct 2009..further development
//--Rev          B. Oussena  24 Jul 2010    have changed PostInit()
//--Rev         B. Oussena  13th Aug 2010  more cleaning 
//--Update      JRM Annand   8th Sep 2010  ReadIRQScaler, TCS eventID moved
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_CATCH_Scaler
// COMPASS CATCH hardware
//

#include "TVME_CATCH_Scaler.h"
#include "TDAQexperiment.h"


static Map_t kCATCH_Scaler_Keys[] = {
  {NULL,                  -1},
};

//-----------------------------------------------------------------------------
TVME_CATCH_Scaler::TVME_CATCH_Scaler( Char_t* name, Char_t* file, FILE* log,
				      Char_t* line ):
  TVME_CATCH( name, file, log, line )
{
  // Scaler specific stuff here 
  fType = EDAQ_Scaler;                        // Scaler module
  AddCmdList( kCATCH_Scaler_Keys );           // scaler-specific setup commands
}

//-----------------------------------------------------------------------------
TVME_CATCH_Scaler::~TVME_CATCH_Scaler( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TVME_CATCH_Scaler::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file
  // Use standard CATCH
  //
  switch( key ){
  default:
    // default try commands of TDAQmodule
    TVME_CATCH::SetConfig(line, key);
    break;
  }
}

//-------------------------------------------------------------------------
void TVME_CATCH_Scaler::PostInit( )
{
  // Check if any general register initialisation has been performed
  // If not perform base CATCH setup
  //
  if( fIsInit ) return;
  TVME_CATCH::PostInit();
  if( fInitLevel >= EExpInit1 ){   // level >=1 init. only

    ProgFPGA(fFPGAfile, ECATCHport);
    sleep(1);
    ProgFPGA(fTCSRecFile, ETCSport);
    sleep(1);
    // 1st round of register programing
    fIWrt = WrtCatch( 0 );
    sleep(1);
  }

  return;
}

//---------------------------------------------------------------------------
void TVME_CATCH_Scaler::ReadIRQScaler( void** outBuffer )
{
  // Read Catch Data Buffer via VMEbus
  //
  UInt_t nword;	                     // words in data buffer
  // Read the CATCH spy (VMEbus) buffer
  if( !(nword = SpyRead(outBuffer) ) ) return;
  UInt_t* pdatum = fSpyData + 3;       // skip S-Link header & CMC header
  Int_t i = fBaseIndex;
  Int_t j,k;
  for( j=0; j<4; j++ ){                // 4 CMC scaler cards per catch
    for( k=0; k<32; k++ ){             // 32 channels per CMC board
      ScalerStore( outBuffer, *pdatum & 0x7fffffff, i++ );
      pdatum++;
    }
    pdatum += 4;                       // skip trailer & next header
  }
  //  fTCSEventID = fSpyData[0]; // move the TVME_CATCH
  return;
}

ClassImp(TVME_CATCH_Scaler)
