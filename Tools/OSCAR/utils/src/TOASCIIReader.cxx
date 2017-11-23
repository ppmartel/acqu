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


#include <fstream>

#include "TOASCIIReader.h"


//______________________________________________________________________________
TOASCIIReader::TOASCIIReader(const Char_t* inFileName)
{
    // Constructor with the file name as parameter.

    // create and open the file
    fFile = new ifstream();
    fFile->open(inFileName, ifstream::in);

    // error if file could not be opened, otherwise save file name
    if (!fFile->is_open()) Error("TOASCIIReader", "File '%s' could not be opened!", inFileName);
    strcpy(fFileName, inFileName);
}

//______________________________________________________________________________
TOASCIIReader::~TOASCIIReader()
{
    // Destructor.

    // close and destroy the file
    if (fFile)
    {
        if (fFile->is_open()) fFile->close();
        delete fFile;
    }
}

//______________________________________________________________________________
Int_t TOASCIIReader::ReadLines(Bool_t inCallParser)
{
    // Read the file line by line and call ParseLine() for every line
    // if inCallParser is true.
    //
    // Return the number of lines read. 
 
    // check if the file is open
    if (!fFile->is_open()) 
    {
        Error("TOASCIIReader::ReadFile", "No open file to read!");
        return 0;
    }

    // ensure we're at the beginning of the file -> reopen the file
    fFile->close();
    fFile->open(fFileName);

    // read line by line and call ParseLine()
    Char_t line[1024];
    Int_t lines = 0;
    while (!fFile->eof())
    {
        fFile->getline(line, 1024);
        if (inCallParser) ParseLine(line);
        lines++;
    }

    return lines-1;
}

//______________________________________________________________________________
void TOASCIIReader::ReadFile()
{
    // Read the file line by line and call the ParseLine method for every line.
 
    ReadLines(kTRUE);
}

//______________________________________________________________________________
Int_t TOASCIIReader::CountLines()
{
    // Return the number of lines in the opened file.

    return ReadLines(kFALSE);
}
ClassImp(TOASCIIReader)
