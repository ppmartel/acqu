//--Author	JRM Annand	22nd Feb 2003
//--Rev 	JRM Annand       4th Feb 2004
//--Rev         Sven Schumann   25th Mar 2004  SetBinContent() fix
//--Rev 	JRM Annand      31st Mar 2004  Scaler plot
//--Rev 	JRM Annand      24th Jun 2004  Scaler plot upper range bug
//--Rev 	JRM Annand      27th Jun 2004  Rate or multi-scaler plot
//--Rev 	JRM Annand      10th Feb 2005  Template issues gcc 3.4
//--Update	JRM Annand      10th May 2005  Complete weight installation
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data.
//
// TA2H1
// Wrapper class for TH1F 1D histogram class
// adds pointer to x variable to be histogramed and a pointer to a
// weight variable. Able to handle single or multi value
// variables through child classes of an abstract  base class

#include "TA2H1.h"

//ClassImpT(TA2H1,T)

