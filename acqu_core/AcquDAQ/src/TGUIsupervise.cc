//--Author	JRM Annand    5th Dec 2007 
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Update	JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fixes
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TGUIsupervise
// Supervise and control operation of AcquDAQ
//

#include "TGUIsupervise.h"
#include "TDAQexperiment.h"
#include "TDAQmodule.h"
#include "ARSocket_t.h"
#include "ARFile_t.h"
#include "TDAQguiCtrl.h"

#include <time.h>


//-----------------------------------------------------------------------------
TGUIsupervise::TGUIsupervise( Char_t* name,  TDAQexperiment* exp,
			      Char_t* line )
: TDAQsupervise( name, exp, line )
{
  // Import the pointer to the experiment class and
  // ensure local variables marked unitialised
  // file by default is NULL...output directed to the terminal
  //  SetInputMode(line);
}

//-----------------------------------------------------------------------------
TGUIsupervise::~TGUIsupervise( )
{
  // Clean up
  //  if( fCtrlBar ) delete fCtrlBar;
}

//---------------------------------------------------------------------------
Char_t* TGUIsupervise::GetString( const Char_t* string )
{
  // Zero the command-line buffer
  // Then read from input stream
  //
  for( Int_t i=0; i<ELineSize; i++ ) fCommandLine[i] = 0;
  strcpy( fCommandLine, string );
  fSocket->WriteChunked(fCommandLine, ELineSize, ELineSize );
  fSocket->ReadChunked( fCommandLine, ELineSize, ELineSize );
  return fCommandLine;
}

//---------------------------------------------------------------------------
void TGUIsupervise::PutString( Char_t* line )
{
  // GUI control output
  if( fGUIBuffer ){
    fGUICtrl->LoadMsgBuffer(line);
    fGUICtrl->Update();
  }
  fprintf( fLogStream, "%s", line );
}

//-----------------------------------------------------------------------------
void TGUIsupervise::ExecRemote( const Char_t* line )
{
  // Start the DAQ, but do not start a new run
  // Check slave mode, data storage, run already initialised
  // enable IRQ
  //
  fSocket->WriteChunked((void*)line, ELineSize, ELineSize );
  fSocket->ReadChunked( fCommandLine, ELineSize, ELineSize );
  PutString(fCommandLine);
  return;
}

//----------------------------------------------------------------------------
Int_t TGUIsupervise::GetRunNumber()
{ 
  // Get the run number from DAQ
  Int_t run;
  sscanf( GetString("RunNumber:"),"%d", &run);
  return run;
}

//----------------------------------------------------------------------------
void TGUIsupervise::SetRunNumber( Int_t r )
{
  Char_t runN[24];
  sprintf( runN, "RunNumber: %d", r );
  sscanf( GetString( runN ), "%d", &fRunNumber );
}

//----------------------------------------------------------------------------
Bool_t TGUIsupervise::IsRunInit()
{
  // Ask DAQ if a run is in progress
  Int_t result;
  sscanf( GetString("IsRunning:"), "%d", &result );
  return (Bool_t)result;
}

//----------------------------------------------------------------------------
void TGUIsupervise::SetAuto( Bool_t state )
{
  // Set/unset auto run changing
  //  Char_t* a;
  if( state )
    //    a = GetString("Auto:");
    GetString("Auto:");
  else
    //    a = GetString("NoAuto:");
    GetString("NoAuto:");
}

//----------------------------------------------------------------------------
Bool_t TGUIsupervise::IsDataReady()
{ 
  return fSocket->IsReady();
}

ClassImp(TGUIsupervise)
