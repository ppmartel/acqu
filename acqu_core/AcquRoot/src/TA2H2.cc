//--Author	JRM Annand	22nd Feb 2003
//--Rev 	JRM Annand       4th Feb 2004  Fill conditions
//--Rev 	JRM Annand      10th Feb 2005  Template issues gcc 3.4
//--Update	JRM Annand      10th May 2005  Complete weight installation
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data.
//
// TA2H2
// Wrapper class for TH2F 2D histogram class
// adds pointers to x,y variables to be histogramed and a pointer to a
// weight variable. Able to handle any combination of single/multi value
// variables through child classes of an abstract  base class

#include "TA2H2.h"

//ClassImpT(TA2H1,T)

//-----------------------------------------------------------------------------
//template< typename T >TA2H1<T>::
//TA2H1( char* name, const char* title, Int_t xchan, Axis_t xlow,
//	Axis_t xhigh, T* databuff ) :
//  TH1F(name, title, xchan, xlow, xhigh)
//{
  // Constructor...don't allocate any "new" memory here (Root will wipe it)
  // Save the data buffer pointer for filling the histogram
//  fHistBuff = databuff;
//}

//-----------------------------------------------------------------------------
//template< typename T >TA2H1<T>::
//TA2H1( char* name, const char* title, Int_t xchan, Double_t* xchanval,
//	T* databuff ) :
//  TH1F(name, title, xchan, xchanval )
//{
  // Constructor...don't allocate any "new" memory here (Root will wipe it)
  // Save the data buffer pointer for filling the histogram
//  fHistBuff = databuff;
//}

//-----------------------------------------------------------------------------
//template< typename T >TA2H1<T>::~TA2H1()
//{
  // Free up allocated memory
//}

