/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOASCIIReader                                                        //
//                                                                      //
// This class provides the basic functionality for ASCII file reading.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOASCIIReader
#define OSCAR_TOASCIIReader

#include "TObject.h"

class TOASCIIReader : public TObject
{

protected:
    ifstream* fFile;                                    // ASCII file to read          
    Char_t fFileName[256];                              // Name of the file 

    Int_t ReadLines(Bool_t inCallParser);
    virtual void ParseLine(const Char_t* inLine) = 0;     // line parser method

public:
    TOASCIIReader() : fFile(0) { }
    TOASCIIReader(const Char_t* inFileName);
    virtual ~TOASCIIReader();
    
    void ReadFile();
    virtual Int_t CountLines();                         // line counter method

    ClassDef(TOASCIIReader, 0)  // abstract ASCII file reader
};

#endif

