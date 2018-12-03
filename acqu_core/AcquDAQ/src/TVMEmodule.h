//--Author	JRM Annand   30th Sep 2005  Basis of VMEbus hardware
//--Rev 	JRM Annand
//--Rev 	JRM Annand   10th Feb 2007  Integrate to Acqu++
//--Rev 	JRM Annand   11th Jan 2008  Additions for working DAQ
//--Rev 	JRM Annand   22nd Jan 2008  Write(index,data)
//--Rev         B.Oussena    1st  Jan 2009  started update for KpHboard
//--Rev 	JRM Annand   24th Jul 2009  Multiple InitReg()
//--Rev         B.Oussena    25th Sep 2009  Fixed bug in InitReg()
//--Rev  	JRM Annand   25th Oct 2009  Multiple InitReg() more mods
//--Rev 	JRM Annand   13th Apr 2010  64-bit address void* <-> ULong_t
//--Rev 	B.Oussena     5th Sep 2010  Bug InitReg()
//--Rev 	JRM Annand    6th Jul 2011  gcc4.6-x86_64 warning fixes
//--Rev 	JRM Annand   25th Jan 2012  Read fNBits in constructor
//--Rev 	JRM Annand   20th May 2012  Check AM reg initialised
//                                          Implement "repeat" in reg init
//--Rev 	JRM Annand   24th Aug 2012  'w' = 2 bytes (not 'm')
//--Rev 	JRM Annand    7th Sep 2013  Do NOT neglect A16
//--Rev 	JRM Annand   23rd Sep 2013  Add WriteChk()
//--Update	JRM Annand    2nd Nov 2017  DMA support
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVMEmodule
// Basis of all VMEbus control/readout software
// General-purpose VMEbus hardware with no specialist read/write procedures
// may use this class
//

#ifndef __TVMEmodule_h__
#define __TVMEmodule_h__

#include "TDAQmodule.h"

struct VMEreg_t{
  UInt_t offset;	// register offset from device base address
  UInt_t data;       	// data written to reg
		       	// offset) to initialisae with data
  Char_t type;		// l,w,b for 32,16,8 bit respectvely
  UInt_t repeat;	// number consecutive locations (starting at
};


class TVMEmodule : public TDAQmodule {
 protected:
  Int_t* fAM;                               // AM (address modifer) codes
  Bool_t* fIsWrt;                           // Write initialisation data?
  Int_t fHardIDReg;                         // ID register index 
  Int_t fBlkParm;                           // end of block readout control
  Int_t fBlkSize;                           // size of block read
  UInt_t* fBlkBuff;                         // for block read
  Bool_t fIsBlk;                            // block transfer readout?
 public:
  TVMEmodule( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVMEmodule();
  virtual void SetConfig( Char_t*, Int_t );   // configure VME module
  virtual void PostInit( );
  virtual void InitReg( Char_t* );            // initialise and store register
  virtual void InitReg( UInt_t, Int_t, Int_t, Int_t, UInt_t=0 );
  virtual void InitReg( VMEreg_t* );
  virtual void Read(void*, void*, Int_t, Int_t);
  virtual void Write(void*, void*, Int_t, Int_t);
  virtual UInt_t Read(Int_t);
  virtual void Write(Int_t);
  virtual void Write(Int_t, UInt_t);
  virtual void WriteChk(Int_t, UInt_t);
  virtual void Write(Int_t, void*);
  virtual void Read(void*, void*);
  virtual void Write(void*, void*);
  virtual Bool_t CheckHardID();
  virtual Bool_t IsBlk(){ return fIsBlk; }
  //
  Int_t* GetAM(){ return fAM; }
  Bool_t* GetIsWrt(){ return fIsWrt; }

  ClassDef(TVMEmodule,1)   

    };

//-----------------------------------------------------------------------------
inline void TVMEmodule::Read( void* addr, void* data, Int_t am, Int_t dw )
{
  // Controller read
  fCtrlMod->Read( addr, data, am, dw );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Write( void* addr, void* data, Int_t am, Int_t dw )
{
  // Controller write
  fCtrlMod->Write( addr, data, am, dw );
}

//-----------------------------------------------------------------------------
inline UInt_t TVMEmodule::Read( Int_t i )
{
  // Controller read
  fPCtrlMod->Read( fReg[i], fData+i, fAM[i], fDW[i] );
  return fData[i];
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Write( Int_t i )
{
  // Controller write
  fPCtrlMod->Write( fReg[i], fData+i, fAM[i], fDW[i] );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Write( Int_t i, UInt_t datum )
{
  // Controller write
  fData[i] = datum;
  fPCtrlMod->Write( fReg[i], fData+i, fAM[i], fDW[i] );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Write( Int_t i, void* data )
{
  // Controller write
  fPCtrlMod->Write( fReg[i], data, fAM[i], fDW[i] );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Read( void* addr, void* data )
{
  // Controller read
  fCtrlMod->Read( addr, data, *fAM, *fDW );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::Write( void* addr, void* data )
{
  // Controller write
  fCtrlMod->Write( addr, data, *fAM, *fDW );
}

//-----------------------------------------------------------------------------
inline void TVMEmodule::WriteChk( Int_t i, UInt_t datum )
{
  // Write a value to a register and read it back to check
  // Will only work for a read/write register
  fData[i] = datum;
  fPCtrlMod->Write( fReg[i], fData+i, fAM[i], fDW[i] );
  datum = Read(i);
  if( datum != fData[i] )
    PrintError("","<Write check failure>",EErrFatal);
  return;
}

#endif
