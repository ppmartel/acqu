//----------------------------------------------------------------------------
//			Acqu++
//		Data Acquisition and Analysis
//			for
//		Nuclear Physics Experiments
//
//		    *** TA2H1S.cc ***
//--Description
//	TA2System
//	Inherited class of TObject, ROOT dictionary compatible
//      Online or Offline data input
//      Online: in buffers provided by DataServer
//	Offline:from a chain of Root files containing a TTree
//         This may be Raw data as written by DataStore
//         or partially analysed data written by Acqu-Root
//
//--Author	JRM Annand	22nd Feb 2003
//--Update	JRM Annand
//
//---------------------------------------------------------------------------
#include "TA2H1S.h"


//-----------------------------------------------------------------------------
template< typename T > TA2H1S<T>::
TA2H1S( char* name, const char* title, Int_t xchan, Axis_t xlow,
	Axis_t xhigh, T* databuff ) :
  TH1F(name, title, xchan, xlow, xhigh)
{
  // Constructor...don't allocate any "new" memory here (Root will wipe it)
  // Save the data buffer pointer for filling the histogram
  fHistBuff = databuff;
}

//-----------------------------------------------------------------------------
template< typename T >TA2H1S<T>::
TA2H1S( char* name, const char* title, Int_t xchan, Double_t* xchanval,
	T* databuff ) :
  TH1F(name, title, xchan, xchanval )
{
  // Constructor...don't allocate any "new" memory here (Root will wipe it)
  // Save the data buffer pointer for filling the histogram
  fHistBuff = databuff;
}

//-----------------------------------------------------------------------------
template< typename T >TA2H1S<T>::~TA2H1S()
{
  // Free up allocated memory
}

ClassImpT(TA2H1S,T)
