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


#include <cstdlib>

#include "TThread.h"
#include "TError.h"
#include "TRandom.h"

#include "TOSUtils.h"

// Init namespace member
Char_t** TOSUtils::fTmpCh = 0;
const Int_t TOSUtils::kTmpChLength = 4096;
const Int_t TOSUtils::kMaxThreads = 100;
Int_t TOSUtils::fNThread = 0;
TThread** TOSUtils::fThreads = 0;

//______________________________________________________________________________
Char_t* TOSUtils::GetOutputString()
{
    // Return the global output string to be used in the current thread.

    // only one thread here
    TThread::Lock();

    // create string array for threads if necessary
    if (!fTmpCh) fTmpCh = new Char_t*[kMaxThreads];
    
    // create thread array if necessary
    if (!fThreads) 
    {
        fThreads = new TThread*[kMaxThreads];
        for (Int_t i = 0; i < kMaxThreads; i++) fThreads[i] = 0;
    }

    // identify thread
    Int_t thr = -1;
    
    // loop over threads
    for (Int_t i = 0; i < fNThread; i++)
    {
        // leave when thread is found
        if (fThreads[i] == TThread::Self())
        {
            thr = i;
            break;
        }
    }
    
    // return the thread's output string
    if (thr == -1)
    {
        // register thread
        fThreads[fNThread] = TThread::Self();
        
        // create the thread's output string
        fTmpCh[fNThread] = new Char_t[kTmpChLength];
    
        // count thread
        fNThread++;

        // return output string
        TThread::UnLock();
        return fTmpCh[fNThread-1];
    }
    else 
    {   
        TThread::UnLock();
        return fTmpCh[thr];
    }
}

//______________________________________________________________________________
Int_t TOSUtils::IndexOf(const Char_t* s, Char_t c, UInt_t p)
{
    // Returns the position of the first occurrence of the character c
    // in the string s after position p. Returns -1 if c was not found.

    const Char_t* pos = strchr(s+p, (Int_t)c);
    if (pos) return pos-s;
    else return -1;
}

//______________________________________________________________________________
Int_t TOSUtils::IndexOf(const Char_t* s1, const Char_t* s2, UInt_t p)
{
    // Returns the position of the first occurrence of the string s2
    // in the string s1 after position p. Returns -1 if s2 was not found.

    const Char_t* pos = strstr(s1+p, s2);
    if (pos) return pos-s1;
    else return -1;
}

//______________________________________________________________________________
Int_t TOSUtils::LastIndexOf(const Char_t* s, Char_t c)
{
    // Returns the position of the last occurrence of the character c
    // in the string s. Returns -1 if c was not found.
    
    const Char_t* pos = strrchr(s, (Int_t)c);
    if (pos) return pos-s;
    else return -1;
}

//______________________________________________________________________________
Char_t* TOSUtils::ExtractDirectory(const Char_t* s)
{
    // Extracts the parent directory out of the full Unix path 's'.
    
    // get output string
    Char_t* out = GetOutputString();
    
    // Search last slash
    Int_t pos = LastIndexOf(s, '/');

    // Return the same string or copy substring
    if (pos == -1) strcpy(out, ".");
    else 
    {
        strncpy(out, s, pos);
        out[pos] = '\0';
    }

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::ExtractFileName(const Char_t* s)
{
    // Extracts the file name of a file given by its full Unix paths in
    // the string s.
    
    // get output string
    Char_t* out = GetOutputString();
  
    // Search last slash
    Int_t pos = LastIndexOf(s, '/');

    // Return the same string or copy substring
    if (pos == -1) strcpy(out, s);
    else strncpy(out, s+pos+1, strlen(s)-pos);
    
    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::ExtractPureFileName(const Char_t* s)
{
    // Extracts the file name and truncates the file ending of a file given 
    // by its full Unix path in the string s.
    
    // get output string
    Char_t* out = GetOutputString();
  
    // Get the file name
    Char_t fn[256];
    strcpy(fn, ExtractFileName(s));
    
    // Search last dot
    Int_t pos = LastIndexOf(fn, '.');
    
    // Strip file ending
    if (pos == -1) strcpy(out, fn);
    else
    {
        strncpy(out, fn, pos);
        out[pos] = '\0';
    }

    return out;
}

//______________________________________________________________________________
Int_t TOSUtils::ExtractRunNumber(const Char_t* s)
{
    // Extract the run number from the string 's' having the pattern "%s_RUN%s",
    // i.e. CB_RUN.dat or ARHistograms_CB_RUN.root.
    
    Char_t tmp[256];
    
    // find last underscore 
    Int_t undersc = TOSUtils::LastIndexOf(s, '_');
        
    // gather all digits between underscore and the next other character
    Int_t l = 1;
    while (kTRUE)
    {
        TString number(s+undersc+1, l);
        if (!number.IsDigit()) break;
        strcpy(tmp, number.Data());
        l++;
    }
    return atoi(tmp);
}

//______________________________________________________________________________
Char_t* TOSUtils::SubString(const Char_t* s, Int_t start, Int_t end)
{
    // Return a sub-string of the string 's' starting at the position 'start' 
    // and ending at 'end'.

    // get output string
    Char_t* out = GetOutputString();
  
    // check the start position (the first check is needed for ROOT macros
    // without type checking)
    if (start < 0) 
    {
        Warning("TOSUtils::SubString", "Start position too small (%d) - set to 0", start);
        start = 0;
    }
    else if (start >= (Int_t)strlen(s)) 
    {
        Error("TOSUtils::SubString", "Start position too large (%d) !", start);
        return 0;
    }

    // check the end position (the first check is needed for ROOT macros
    // without type checking)
    if (end < 0) 
    {
        Error("TOSUtils::SubString", "End position too small (%d) !", end);
        return 0;
    }
    else if (end >= (Int_t)strlen(s)) 
    {
        Warning("TOSUtils::SubString", "End position too large (%d) - set to %ld", 
                end, strlen(s)-1);
        end = strlen(s)-1;
    }

    // copy the substring
    strncpy(out, s+start, end-start+1);

    // terminate string
    out[end-start+1] = '\0';

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::Trim(const Char_t* s)
{
    // Removes the leading and trailing whitespace of the string s.

    Int_t l = strlen(s);
    Int_t posF = -1;
    Int_t posL = -1;

    // get output string
    Char_t* out = GetOutputString();
  
    // Search first non-whitespace
    for (Int_t i = 0; i < l; i++)
    {
        // leave loop when first non-whitespace was found
        if (s[i] != ' ')
        {
            posF = i;
            break;
        }
    }

    // handle empty string
    if (posF == -1)
    {
        out[0] = '\0';
        return out;
    }
 
    // Search last non-whitespace
    for (Int_t i = l-1; i >= 0; i--)
    {
        // leave loop when last non-whitespace was found
        if (s[i] != ' ')
        {
            posL = i;
            break;
        }
    }

    // Copy trimmed string
    Int_t j = 0;
    for (Int_t i = posF; i <= posL; i++)
    {
        out[j++] = s[i];
    }
    out[j] = '\0';

    return out;
}

//______________________________________________________________________________
Bool_t TOSUtils::IsComment(const Char_t* s)
{
    // Returns kTRUE if the string s is a comment, i.e. starts with a '#'
 
    // Trim string
    Char_t* st = Trim(s);
 
    // First character is a '#'
    if (st[0] == '#') return kTRUE;
    else return kFALSE;
}

//______________________________________________________________________________
Bool_t TOSUtils::Contains(const Char_t* s1, const Char_t* s2)
{
    // Returns kTRUE if the string 's1' contains the string 's2'.

    // search the string s2
    if (TOSUtils::IndexOf(s1, s2) == -1) return kFALSE;
    else return kTRUE;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatArrayList(Int_t n, Int_t* arr, const Char_t* format)
{
    // Returns a comma-separated list of all elements of the array 'arr' as 
    // a string using 'format' for formatting (default is %d).
    
    // get output string
    Char_t* out = GetOutputString();

    // clear string
    out[0] = '\0';
    
    // create formatting string
    Char_t fmt[8];
    sprintf(fmt, "%s", format);
     
    // format list
    Char_t tmp[512];
    for (Int_t i = 0; i < n; i++)
    {
        sprintf(tmp, fmt, arr[i]);
        strcat(out, tmp);
        if (i < n-1) strcat(out, ", ");
    }

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatArrayList(Int_t n, UInt_t* arr, const Char_t* format)
{
    // Returns a comma-separated list of all elements of the array 'arr' as 
    // a string using 'format' for formatting (default is %d).
    
    // get output string
    Char_t* out = GetOutputString();
 
    // clear string
    out[0] = '\0';
    
    // create formatting string
    Char_t fmt[8];
    sprintf(fmt, "%s", format);
     
    // format list
    Char_t tmp[512];
    for (Int_t i = 0; i < n; i++)
    {
        sprintf(tmp, fmt, arr[i]);
        strcat(out, tmp);
        if (i < n-1) strcat(out, ", ");
    }

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatArrayList(Int_t n, Float_t* arr, const Char_t* format)
{
    // Returns a comma-separated list of all elements of the array 'arr' as 
    // a string using 'format' for formatting (default is %e).
    
    // get output string
    Char_t* out = GetOutputString();
 
    // clear string
    out[0] = '\0';
    
    // create formatting string
    Char_t fmt[8];
    sprintf(fmt, "%s", format);
 
    // format list
    Char_t tmp[512];
    for (Int_t i = 0; i < n; i++)
    {
        sprintf(tmp, fmt, arr[i]);
        strcat(out, tmp);
        if (i < n-1) strcat(out, ", ");
    }

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatArrayList(Int_t n, Double_t* arr, const Char_t* format)
{
    // Returns a comma-separated list of all elements of the array 'arr' as 
    // a string using 'format' for formatting (default is %e).
    
    // get output string
    Char_t* out = GetOutputString();
 
    // clear string
    out[0] = '\0';
    
    // create formatting string
    Char_t fmt[8];
    sprintf(fmt, "%s", format);
 
    // format list
    Char_t tmp[512];
    for (Int_t i = 0; i < n; i++)
    {
        sprintf(tmp, fmt, arr[i]);
        strcat(out, tmp);
        if (i < n-1) strcat(out, ", ");
    }

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatBinary(Int_t n, Int_t length)
{
    // Return a string of the binary representation of the integer 'n' in binary.
    // If 'length' is non-zero the returned string has a minimum number of characters
    // of 'length'.
    // Source: http://www.sharms.org/blog/2009/07/programming-in-c-converting-an-integer-to-binary-int-to-bin

    // get output string
    Char_t* out = GetOutputString();
  
    //
    // get length of string
    //

    Int_t x = 1;
    Int_t i = 0;
    Int_t lastFound = 0;
    Int_t size = 0;

    // Takes care of n = 0, and we dont need to loop for 1st case
    if (n < 16) size = 4;
    else
    {
        while(1)
        {
            i++;
            if (n / x) lastFound = i;
            else break;
            x <<= 1;
        }

        // Not a multiple of 4, how much do we need to add to it
        if (lastFound % 4) lastFound += (4 - (lastFound % 4));
        size = lastFound;
    }

    // adjust length
    if (length)
    {
        if (length > size) size = length;
    }

    // 
    // create string
    //
    
    // check length
    if (size >= kTmpChLength)
    {
        Error("TOSUtils::FormatBinary", "Binary representation of %d is too long!", n);
        out[0] = '\0';
        return out;
    }

    // terminate string
    i = size;
    out[i] = '\0';

    // build string
    while(i > 0)
    {
        if (n & 0x01) out[--i] = '1';
        else out[--i] = '0';
        n >>= 1;
    }
    
    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::FormatTimeSec(Double_t seconds)
{
    // Return a time string in the format HH:MM:SS of the seconds 'seconds'.

    // get output string
    Char_t* out = GetOutputString();
 
    // convert seconds
    Int_t hours = Int_t(seconds / 3600);
    seconds -= hours * 3600;
    Int_t min = Int_t(seconds / 60);
    seconds -= min * 60;
    Int_t sec = Int_t(seconds);

    // format string
    sprintf(out, "%02d:%02d:%02d", hours, min, sec);

    return out;
}

//______________________________________________________________________________
Char_t* TOSUtils::GetRandomString(Int_t length, TRandom* rand)
{
    // Return a random character string with length 'length'.
    // If 'rand' is non-zero use it as random number generator, otherwise gRandom
    // is used.

    // get output string
    Char_t* out = GetOutputString();
  
    // loop over number of characters
    for (Int_t i = 0; i < length; i++)
    {
        // capital?
        Int_t cap = rand ? rand->Uniform(0, 2) : gRandom->Uniform(0, 2);
    
        // generate character
        Char_t ch;
        if (cap) ch = rand ? rand->Uniform(65, 91) : gRandom->Uniform(65, 91);
        else ch = rand ? rand->Uniform(97, 123) : gRandom->Uniform(97, 123);

        out[i] = ch;
    }
    
    // terminate string
    out[length] = '\0';

    return out;
}

