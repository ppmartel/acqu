//--Author	JRM Annand	25th Jan 2006
//--Rev 	JRM Annand
//--Rev 	JRM Annand       6th Dec 2007 PostInit check fData
//--Rev 	JRM Annand      11th Jan 2008 Add ReadIRQ
//--Rev 	JRM Annand       5th Sep 2010 Datum in hex CNAF input
//--Update	JRM Annand      16th Sep 2010 ReadIRQScaler...24bit read
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TCAMACmodule
// Basis of all CAMAC modules.
// This contains general-purpose procedures to setup and readout of CAMAC
// General-purpose CAMAC may be instantiated using this class

#include "TCAMACmodule.h"
#include "TDAQexperiment.h"


// For command-line setup of module
enum { ECAMACcnaf=100 };
static Map_t kCAMACKeys[] = {
  { "CAMAC-CNAF:",         ECAMACcnaf },
  {NULL,                  -1}
};

//-----------------------------------------------------------------------------
TCAMACmodule::TCAMACmodule( Char_t* name, Char_t* input, FILE* log,
			    Char_t* line ):
  TDAQmodule( name, input, log )
{
  // Basic initialisation
  // Read Crate and Station geographical address
  AddCmdList( kCAMACKeys );                  // camac-specific cmds
  if( sscanf( line, "%*s%*s%*s%d%d", &fC, &fN ) != 2 )
    PrintError( line, "<Setup line parse>", EErrFatal );
  fInitReg = fIRQReg = NULL;
  fNInitReg = fNIRQReg = 0;
  fBus = E_CAMAC;
}

//-----------------------------------------------------------------------------
TCAMACmodule::~TCAMACmodule( )
{
  // Clean up
}

//-----------------------------------------------------------------------------
void TCAMACmodule::SetConfig( Char_t* line, Int_t key )
{
  // Configuration from file 
  switch( key ){
  case ECAMACcnaf:    // Store CNAF
    InitReg( line );
    break;
  default:
    // default try commands of TDAQmodule
    TDAQmodule::SetConfig(line, key);
    break;
  }
}

//-----------------------------------------------------------------------------
void TCAMACmodule::Init( )
{ 
  // General purpose CAMAC initialisation
  for( Int_t i=0; i<fNInitReg; i++ )
    Write( fInitReg[i], fData + i );

}

//-----------------------------------------------------------------------------
void TCAMACmodule::PostInit( )
{ 
  // General purpose 2nd-phase CAMAC initialisation
  // For F9 type commands set fData[i] to some value > 0xffff to force
  // a read instead of write
  TDAQmodule::PostInit();
  fInitReg = fReg;
  fTestReg = fReg + fNInitReg;
  fIRQReg = fTestReg + fNTestReg;
  for( Int_t i=0; i<fNInitReg; i++ ){
    if( fData[i] <= 0xffff )Write(i);
    else Read(i);
  }
}

//-----------------------------------------------------------------------------
void TCAMACmodule::InitReg( Char_t* line )
{ 
  // Parse the input line for device register information
  // Store the register

  if( fNreg > fMaxReg ){
    PrintError( line, "<Too many CNAFs or fMaxReg not yet specified>");
    return;
  }
  if( !fCtrlMod ){
    PrintError( line, "<CNAF Setup...CAMAC controller not yet specified>");
    return;
  }
  UInt_t datum;
  Int_t F,A0,A1,n,itype;
  Char_t type[16];
  if( (n = sscanf( line,"%x%d%d%d%s",&datum,&F,&A0,&A1,type )) < 5 ){
    PrintError( line, "<Register Setup parse>");
    return;
  }
  if( (itype = GetRegType(type)) == -1 ){
    PrintError(line,"<Unknown register type specified>");
    return;
  }
  for(Int_t i=A0; i<=A1; i++){
    fReg[fNreg] = CNAF(fC,fN,i,F);
    fDW[fNreg] = 2;
    fData[fNreg] = datum;
    fNreg++;
    switch( itype ){
    case EModIRQ:
      fNIRQReg++;
      break;
    case EModTest:
      fNTestReg++;
      break;
    default:
      fNInitReg++;
      break;
    }
  }
}

ClassImp(TCAMACmodule)
