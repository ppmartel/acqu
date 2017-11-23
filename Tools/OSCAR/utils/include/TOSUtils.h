/*************************************************************************
 * Author: Dominik Werthmueller, 2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOSUtils                                                             //
//                                                                      //
// This namespace contains some often used functions and operations     //
// for strings.                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOSUtils
#define OSCAR_TOSUtils

#include "Rtypes.h"

class TThread;
class TRandom;

namespace TOSUtils
{
    extern Char_t** fTmpCh;
    extern const Int_t kTmpChLength;
    extern const Int_t kMaxThreads;
    extern Int_t fNThread;
    extern TThread** fThreads;

    Char_t* GetOutputString();

    Int_t IndexOf(const Char_t* s, Char_t c, UInt_t p = 0);
    Int_t IndexOf(const Char_t* s1, const Char_t* s2, UInt_t p = 0);
    Int_t LastIndexOf(const Char_t* s, Char_t c);
    
    Char_t* ExtractDirectory(const Char_t* s);
    Char_t* ExtractFileName(const Char_t* s);
    Char_t* ExtractPureFileName(const Char_t* s);
    Int_t ExtractRunNumber(const Char_t* s);
    
    Char_t* SubString(const Char_t* s, Int_t start, Int_t end);
    Char_t* Trim(const Char_t* s);
    
    Bool_t IsComment(const Char_t* s);
    Bool_t Contains(const Char_t* s1, const Char_t* s2);

    Char_t* FormatArrayList(Int_t n, Int_t* arr, const Char_t* format = "%d");
    Char_t* FormatArrayList(Int_t n, UInt_t* arr, const Char_t* format = "%d");
    Char_t* FormatArrayList(Int_t n, Float_t* arr, const Char_t* format = "%e");
    Char_t* FormatArrayList(Int_t n, Double_t* arr, const Char_t* format = "%e");
    Char_t* FormatBinary(Int_t n, Int_t length = 0);
    Char_t* FormatTimeSec(Double_t seconds);

    Char_t* GetRandomString(Int_t length, TRandom* rand = 0);
}

#endif

