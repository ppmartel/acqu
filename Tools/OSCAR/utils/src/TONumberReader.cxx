/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TONumberReader                                                       //
//                                                                      //
// This class reads numbers arranged in multiple columns row by row     //
// and stores them in a list of arrays.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include <string>
#include <sstream>
#include <fstream>

#include "TMath.h"

#include "TONumberReader.h"
#include "TOArray2D.h"
#include "TOSUtils.h"


//______________________________________________________________________________
TONumberReader::TONumberReader(const Char_t* inFileName, Int_t inColumns) 
    : TOASCIIReader(inFileName)
{
    // Construct a TONumberReader with the ASCII file name and the number
    // of data columns.

    fNcol = inColumns;
    fNrow = 0;

    // create 2-dim. array the read values. since the exact number of rows
    // is unknown allocate space for the number of lines in the file
    fData = new TOArray2D(fNcol, CountLines());
    
    ReadFile();
}

//______________________________________________________________________________
TONumberReader::~TONumberReader()
{
    // Destructor.
 
    if (fData) delete fData;
}

//______________________________________________________________________________
void TONumberReader::ParseLine(const Char_t* inLine)
{
    // Read in the numbers of one line and add them to the data structure.
    
    // skip empty and commented lines
    Char_t* l = TOSUtils::Trim(inLine);
    if (l[0] == '\0' || TOSUtils::IsComment(inLine)) return;
    
    std::string line(inLine);
    std::istringstream iss(line);
    Double_t value;
    
    // read all columns
    Int_t col = 0;
    while (col < fNcol) 
    {
        if (iss >> value)
        {
            fData->Set(col, fNrow, value);
            col++;
        }
    }

    // increment row counter
    fNrow++;
}

//______________________________________________________________________________
Double_t* TONumberReader::GetColumn(Int_t c) const
{
    // Return the array of the column 'c'.

    return fData->GetColumn(c);
}

//______________________________________________________________________________
Double_t* TONumberReader::GetSum(Int_t c1, Int_t c2, Double_t w1, Double_t w2) const
{
    // Returns an array containing the sum w1*c1 + w2*c2 of the two
    // columns c1 and c2.
    // WARNING: array has to be destroyed by the caller

    return fData->GetSum(c1, c2, w1, w2);
}

//______________________________________________________________________________
Double_t* TONumberReader::GetSum(Int_t c1, TONumberReader* r, Int_t c2, Double_t w1, Double_t w2) const
{
    // Returns an array containing the sum w1*c1 + w2*c2 of the columns c1
    // of this TONumberReader and c2 of the TONumberReader r.
    // WARNING: array has to be destroyed by the caller

    // check if TONumberReaders exists
    if (!r) 
    {
        Error("GetSum", "Cannot calculate column sum: TONumberReader r is 0");
        return 0;
    }

    // check number of rows
    if (fNrow != r->GetNrow())
    {
        Error("GetSum", "Cannot calculate column sum: Number of rows is different (%d != %d)", 
              fNrow, r->GetNrow());
        return 0;
    }

    // get columns
    Double_t* col1 = GetData()->GetColumn(c1); 
    Double_t* col2 = r->GetData()->GetColumn(c2); 

    // check columns
    if (!col1)
    {
        Error("GetSum", "Cannot calculate column sum: Column %d of this TONumberReader is empty", c1);
        return 0;
    }
    if (!col2)
    {
        Error("GetSum", "Cannot calculate column sum: Column %d of TONumberReader r is empty", c2);
        return 0;
    }

    // create result array
    Double_t* sum = new Double_t[fNrow];

    // fill result array
    for (Int_t i = 0; i < fNrow; i++)
    {
        sum[i] = w1 * col1[i] + w2 * col2[i];
    }

    return sum;
}

//______________________________________________________________________________
Int_t TONumberReader::CountLines()
{
    // Return the number of lines (ignoring blank and commented lines) in 
    // the opened file.
    
    // check if the file is open
    if (!fFile->is_open()) 
    {
        Error("TONumberReader::CountLines", "No open file to read!");
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
        if ((TOSUtils::Trim(line))[0] == '\0' || TOSUtils::IsComment(line)) continue;
        lines++;
    }

    return lines;
}

//______________________________________________________________________________
Double_t TONumberReader::GetColumnAverage(Int_t c)
{
    // Calculate the average of the values of column 'c'.
    
    // get column
    Double_t* col = GetData()->GetColumn(c); 

    // check column
    if (!col)
    {
        Error("GetColumnAverage", "Cannot calculate column average: Column %d is empty", c);
        return 0;
    }
    
    // sum up values
    Double_t sum = 0;
    for (Int_t i = 0; i < fNrow; i++) sum += col[i];

    return sum / (Double_t) fNrow;
}

//______________________________________________________________________________
Double_t TONumberReader::GetColumnAverageWeighted(Int_t c, Double_t* outErr)
{
    // Calculate the weighted average of the values of column 'c'. 
    // The c+1-th column is considered to contain the errors.
    // Save the average error in 'outErr' if 'outErr' is not zero.
    
    // check column
    if (c >= fNcol)
    {
        Error("GetColumnAverageWeighted", "Cannot calculate column average: Column %d is empty", c);
        return 0;
    }

    // sum up values
    Double_t val = 0;
    Double_t err = 0;
    for (Int_t i = 0; i < fNrow; i++) 
    {   
        // skip values with zero error
        if (fData->Get(c+1, i) == 0) continue;

        val += (fData->Get(c, i) / fData->Get(c+1, i) / fData->Get(c+1, i));
        err += (1. / fData->Get(c+1, i) / fData->Get(c+1, i));
    }
    
    // complete the calculation (order of statements is important)
    val /= err;
    err = 1. / TMath::Sqrt(err);

    // save error
    if (outErr) *outErr = err;

    return val;
}

//______________________________________________________________________________
Double_t TONumberReader::GetRowAverage(Int_t r)
{
    // Calculate the average of the values of row 'r'.
    
    // check row
    if (r >= fNrow)
    {
        Error("GetRowAverage", "Cannot calculate row average: Row %d is empty", r);
        return 0;
    }
    
    // sum up values
    Double_t sum = 0;
    for (Int_t i = 0; i < fNcol; i++) sum += fData->Get(i, r);

    return sum / (Double_t) fNcol;
}

//______________________________________________________________________________
Double_t TONumberReader::GetRowAverageWeighted(Int_t r, Double_t* outErr)
{
    // Calculate the weighted average of the values of row 'r'. The 2*i-th columns
    // are considered to contain the values, the 2*i+1-th columns the errors.
    // Save the average error in 'outErr' if 'outErr' is not zero.
    
    // check row
    if (r >= fNrow)
    {
        Error("GetRowAverageWeighted", "Cannot calculate row average: Row %d is empty", r);
        return 0;
    }

    // check number of columns
    if (fNcol % 2 != 0) 
    {
        Error("GetRowAverageWeighted", "Number of columns has to be even but is %d!", fNcol);
        return 0;
    }
    
    // sum up values
    Double_t val = 0;
    Double_t err = 0;
    for (Int_t i = 0; i < fNcol/2; i++) 
    {
        // skip values with zero errors
        if (fData->Get(2*i+1, r) == 0) continue;

        val += (fData->Get(2*i, r) / fData->Get(2*i+1, r) / fData->Get(2*i+1, r));
        err += (1. / fData->Get(2*i+1, r) / fData->Get(2*i+1, r));
    }
    
    // complete the calculation (order of statements is important)
    val /= err;
    err = 1. / TMath::Sqrt(err);

    // save error
    if (outErr) *outErr = err;

    return val;
}
ClassImp(TONumberReader)
