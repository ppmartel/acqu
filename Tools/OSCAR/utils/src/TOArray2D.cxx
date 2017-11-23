/*************************************************************************
 * Author: Dominik Werthmueller, 2007-2009
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOArray2D                                                            //
//                                                                      //
// Serializable 2-dim. array for Double_t data types.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TMath.h"

#include "TOArray2D.h"


//______________________________________________________________________________
TOArray2D::TOArray2D(Int_t inColumns, Int_t inRows, const Char_t* inName) 
    : TNamed(inName, inName)
{
    // Construct a 2-dim. array with 'inColumns' columns and 'inRows' rows
    // and optionally name it 'inName'.

    if (inColumns <= 0 || inRows <= 0)
    {
        Error("TOArray2D", "Minimum array size is 1x1 (Input was %dx%d)", inColumns, inRows);
        fNcol = 0;
        fNrow = 0;
        fData = 0;
        return;
    }

    fNcol = inColumns;
    fNrow = inRows;

    // create the data array
    fData = new Double_t*[fNcol];
    for (Int_t i = 0; i < fNcol; i++) fData[i] = new Double_t[fNrow];
    
    // initialize
    for (Int_t i = 0; i < fNcol; i++)
    {
        for (Int_t j = 0; j < fNrow; j++) fData[i][j] = 0.;
    }
}

//______________________________________________________________________________
TOArray2D::~TOArray2D()
{
    // Destructor.
    
    if (fData)
    {
        for (Int_t i = 0; i < fNcol; i++) delete [] fData[i];
        delete [] fData;
    }
}

//______________________________________________________________________________
Bool_t TOArray2D::BoundsOk(const Char_t* inLoc, Int_t inColumn, Int_t inRow) const
{
    // Check if 'inColumn' and 'inRow' are within the bounds (return kTRUE) or not
    // (return kFALSE and print error message). Use 'inLoc' to pass the name of
    // the caller for the error output.

    if (inColumn >= 0 && inColumn < fNcol && inRow >= 0 && inRow < fNrow) return kTRUE;
    else 
    {
        if (inColumn < 0 || inColumn >= fNcol) Error(inLoc, "Column index %d out of bounds (size: %d)", inColumn, fNcol);
        if (inRow < 0 || inRow >= fNrow) Error(inLoc, "Row index %d out of bounds (size: %d)", inRow, fNrow);
        return kFALSE;
    }
}

//______________________________________________________________________________
void TOArray2D::Set(Int_t inColumn, Int_t inRow, Double_t inValue)
{
    // Set the value in column 'inColumn' and row 'inRow' to the value 'inValue'.

    if (BoundsOk("Set", inColumn, inRow)) fData[inColumn][inRow] = inValue;
}

//______________________________________________________________________________
void TOArray2D::ResetNaN(Double_t reset)
{
    // Reset all NaN entries to 'reset'.
    
    // loop over rows
    for (Int_t i = 0; i < fNrow; i++)
    {
        // loop over columns
        for (Int_t j = 0; j < fNcol; j++) 
        {
            if (TMath::IsNaN(fData[j][i])) fData[j][i] = reset;
        }
    }
}

//______________________________________________________________________________
Double_t TOArray2D::Get(Int_t inColumn, Int_t inRow) const
{
    // Return the value in column 'inColumn' and row 'inRow'.
    
    if (BoundsOk("Get", inColumn, inRow)) return fData[inColumn][inRow];
    else return 0;
}

//______________________________________________________________________________
Double_t* TOArray2D::GetColumn(Int_t inColumn) const
{
    // Return a pointer to the array of column 'inColumn'.
    
    // check for out-of-bounds
    if (BoundsOk("GetColumn", inColumn, 0)) return fData[inColumn];
    else return 0;
}

//______________________________________________________________________________
Double_t* TOArray2D::GetSum(Int_t c1, Int_t c2, Double_t w1, Double_t w2) const
{
    // Return an array containing the sum w1*c1 + w2*c2 of the two
    // columns 'c1' and 'c2'.
    //
    // NOTE: The array has to be destroyed by the caller

    // check if columns exist
    if (!BoundsOk("GetSum", c1, 0)) return 0;
    if (!BoundsOk("GetSum", c2, 0)) return 0;

    // create array
    Double_t* sum = new Double_t[fNrow];

    // fill array
    for (Int_t i = 0; i < fNrow; i++)
    {
        sum[i] = w1 * fData[c1][i] + w2 * fData[c2][i];
    }

    return sum;
}

//______________________________________________________________________________
void TOArray2D::Scale(Double_t c)
{
    // Scale the values of this array by 'c'.
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // set value
            Set(i, j, c * Get(i, j));
        }
    }
}

//______________________________________________________________________________
void TOArray2D::Abs()
{
    // Set the values of this array to their absolute values.
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // set value
            Set(i, j, TMath::Abs(Get(i, j)));
        }
    }
}

//______________________________________________________________________________
Bool_t TOArray2D::Add(TOArray2D* d, Double_t c)
{
    // Add 'c'-times the values of 'd' to this array.
    // Return kFALSE if the operation failed, otherwise kTRUE.

    // check number of columns
    if (fNcol != d->GetNcol())
    {
        Error("Add", "Cannot add arrays! Numbers of columns differ (%d != %d)", fNcol, d->GetNcol());
        return kFALSE;
    }

    // check number of rows
    if (fNrow != d->GetNrow())
    {
        Error("Add", "Cannot add arrays! Numbers of rows differ (%d != %d)", fNrow, d->GetNrow());
        return kFALSE;
    }
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // get values
            Double_t a = Get(i, j);
            Double_t b = d->Get(i, j);

            // set value
            Set(i, j, a + c*b);
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TOArray2D::Multiply(TOArray2D* d)
{
    // Multiply the values of this array by the values of 'd'.
    // Return kFALSE if the operation failed, otherwise kTRUE.

    // check number of columns
    if (fNcol != d->GetNcol())
    {
        Error("Multiply", "Cannot multiply arrays! Numbers of columns differ (%d != %d)", fNcol, d->GetNcol());
        return kFALSE;
    }

    // check number of rows
    if (fNrow != d->GetNrow())
    {
        Error("Multiply", "Cannot multiply arrays! Numbers of rows differ (%d != %d)", fNrow, d->GetNrow());
        return kFALSE;
    }
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // get values
            Double_t a = Get(i, j);
            Double_t b = d->Get(i, j);

            // set value
            Set(i, j, a * b);
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TOArray2D::Divide(TOArray2D* d)
{
    // Divide the values of this array by the values of 'd'.
    // Return kFALSE if the operation failed, otherwise kTRUE.

    // check number of columns
    if (fNcol != d->GetNcol())
    {
        Error("Divide", "Cannot divide arrays! Numbers of columns differ (%d != %d)", fNcol, d->GetNcol());
        return kFALSE;
    }

    // check number of rows
    if (fNrow != d->GetNrow())
    {
        Error("Divide", "Cannot divide arrays! Numbers of rows differ (%d != %d)", fNrow, d->GetNrow());
        return kFALSE;
    }
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // get values
            Double_t a = Get(i, j);
            Double_t b = d->Get(i, j);

            // set value and error
            if (b == 0)
                Set(i, j, 0);
            else
                Set(i, j, a / b);
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TOArray2D::Max(TOArray2D* d)
{
    // Replace the values of this array by the maximum of the local values and
    // the values stored in 'd'.
    // Return kFALSE if the operation failed, otherwise kTRUE.

    // check number of columns
    if (fNcol != d->GetNcol())
    {
        Error("Max", "Numbers of columns differ (%d != %d)", fNcol, d->GetNcol());
        return kFALSE;
    }

    // check number of rows
    if (fNrow != d->GetNrow())
    {
        Error("Max", "Numbers of rows differ (%d != %d)", fNrow, d->GetNrow());
        return kFALSE;
    }
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // set maximum
            Set(i, j, TMath::Max(Get(i, j), d->Get(i, j)));
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TOArray2D::Min(TOArray2D* d)
{
    // Replace the values of this array by the minimum of the local values and
    // the values stored in 'd'.
    // Return kFALSE if the operation failed, otherwise kTRUE.

    // check number of columns
    if (fNcol != d->GetNcol())
    {
        Error("Min", "Numbers of columns differ (%d != %d)", fNcol, d->GetNcol());
        return kFALSE;
    }

    // check number of rows
    if (fNrow != d->GetNrow())
    {
        Error("Min", "Numbers of rows differ (%d != %d)", fNrow, d->GetNrow());
        return kFALSE;
    }
    
    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        // loop over rows
        for (Int_t j = 0; j < fNrow; j++)
        {
            // set maximum
            Set(i, j, TMath::Min(Get(i, j), d->Get(i, j)));
        }
    }

    return kTRUE;
}

//______________________________________________________________________________
void TOArray2D::Print(const Char_t* format) const
{
    // Print all values to stdout using 'format' for the number formatting.
    
    // create formatting string
    Char_t fmt[256];
    if (format[0] == '\0') strcpy(fmt, "%.2f  ");
    else sprintf(fmt, "%s  ", format);

    // loop over rows
    for (Int_t i = 0; i < fNrow; i++)
    {
        // loop over columns
        for (Int_t j = 0; j < fNcol; j++) 
        {
            printf(fmt, fData[j][i]);
        }
        printf("\n");
    }
}

//______________________________________________________________________________
void TOArray2D::PrintColumn(Int_t inColumn, const Char_t* format) const
{
    // Print the values of the column 'inColumn' to stdout using 'format'
    // for the number formatting.
    
    // check for out-of-bounds
    if (!BoundsOk("PrintColumn", inColumn, 0)) return;
    
    // create formatting string
    Char_t fmt[256];
    if (format[0] == '\0') strcpy(fmt, "%.2f\n");
    else sprintf(fmt, "%s\n", format);

    // loop over rows
    for (Int_t i = 0; i < fNrow; i++)
    {
        printf(fmt, fData[inColumn][i]);
    }
}

//______________________________________________________________________________
void TOArray2D::PrintRow(Int_t inRow, const Char_t* format) const
{
    // Print the values of the row 'inRow' to stdout using 'format' for
    // the number formatting.
    
    // check for out-of-bounds
    if (!BoundsOk("PrintRow", 0, inRow)) return;
    
    // create formatting string
    Char_t fmt[256];
    if (format[0] == '\0') strcpy(fmt, "%.2f  ");
    else sprintf(fmt, "%s  ", format);

    // loop over columns
    for (Int_t i = 0; i < fNcol; i++)
    {
        printf(fmt, fData[i][inRow]);
    }
    printf("\n");
}

//______________________________________________________________________________
void TOArray2D::Streamer(TBuffer& R__b)
{
    // Stream a TOArray2D object.
    
    UInt_t R__s, R__c; 
    
    if (R__b.IsReading())
    { 
        Version_t R__v = R__b.ReadVersion(&R__s, &R__c); 
        if (R__v) { } 
        
        // call streamer of parent class
        TNamed::Streamer(R__b); 

        // copy data members
        R__b >> fNcol; 
        R__b >> fNrow;
        
        // create the data array
        fData = new Double_t*[fNcol];
        for (Int_t i = 0; i < fNcol; i++) fData[i] = new Double_t[fNrow];
 
        // copy the 2-dim array
        for (Int_t i = 0; i < fNcol; i++) R__b.ReadFastArray(fData[i], fNrow);
    
        // check byte count
        R__b.CheckByteCount(R__s, R__c, TOArray2D::IsA()); 
    } 
    else 
    { 
        R__c = R__b.WriteVersion(TOArray2D::IsA(), kTRUE); 
        
        // call streamer of parent class
        TNamed::Streamer(R__b); 

        // copy data members
        R__b << fNcol; 
        R__b << fNrow; 
        
        // copy the 2-dim array
        for (Int_t i = 0; i < fNcol; i++) R__b.WriteFastArray(fData[i], fNrow);
        
        // set byte count
        R__b.SetByteCount(R__c, kTRUE); 
    } 
}
ClassImp(TOArray2D)
