/*************************************************************************
 * Author: Dominik Werthmueller, 2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2Detector                                                         //
//                                                                      //
// This class offers some methods for the handling of A2 detector       //
// calibration files.                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TError.h"

#include "TOA2Detector.h"


//______________________________________________________________________________
TOA2Detector::TOA2Detector()
    : TObject()
{
    // Default Constructor.

}

//______________________________________________________________________________
TOA2Detector::~TOA2Detector()
{
    // Destructor.
 
}

//______________________________________________________________________________
Bool_t TOA2Detector::IsCBHole(Int_t elem)
{
    // Check if the element 'elem' belongs to a "CB hole".

    Int_t nholes = 48;
    Int_t cb_holes[] = {26, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40,
                        311, 315, 316, 318, 319,
                        353, 354, 355, 356, 357, 358, 359, 
                        360, 361, 362, 363, 364, 365, 366,
                        400, 401, 402, 405, 408,
                        679, 681, 682, 683, 684, 685, 686, 687, 688, 689,
                        691, 692};
    
    // loop over holes
    for (Int_t i = 0; i < nholes; i++)
    {
        if (cb_holes[i] == elem) return kTRUE;
    }

    return kFALSE;
}

//______________________________________________________________________________
Int_t TOA2Detector::GetVetoInFrontOfElement(Int_t id, A2TAPSType_t type) 
{
    // Return the index of the veto that is installed in front of the
    // BaF2 or PWO element with the index 'id' of TAPS type 'type'.
    // This number is equal to the original TAPS 384 BaF2 numbering.
    
    // check TAPS setup
    switch (type)
    {
        case kTAPS_2007:
        {
            return id;
        }
        case kTAPS_2008:
        {
            // 1st PWO ring
            if (id >=   0 && id <=   3) return   0;
            if (id >=  67 && id <=  70) return  64;
            if (id >= 134 && id <= 137) return 128;
            if (id >= 201 && id <= 204) return 192;
            if (id >= 268 && id <= 271) return 256;
            if (id >= 335 && id <= 338) return 320;
            
            // other elements
            else return id - 3*(id/67 + 1);
        }
        case kTAPS_2009:
        {
            // 1st PWO ring
            if (id >=   0 && id <=   3) return   0;
            if (id >=  73 && id <=  76) return  64;
            if (id >= 146 && id <= 149) return 128;
            if (id >= 219 && id <= 222) return 192;
            if (id >= 292 && id <= 295) return 256;
            if (id >= 365 && id <= 368) return 320;
            
            // 2nd PWO ring
            if (id >=   4 && id <=   7) return   1;
            if (id >=   8 && id <=  11) return   2;
            if (id >=  77 && id <=  80) return  65;
            if (id >=  81 && id <=  84) return  66;
            if (id >= 150 && id <= 153) return 129;
            if (id >= 154 && id <= 157) return 130;
            if (id >= 223 && id <= 226) return 193;
            if (id >= 227 && id <= 230) return 194;
            if (id >= 296 && id <= 299) return 257;
            if (id >= 300 && id <= 303) return 258;
            if (id >= 369 && id <= 372) return 321;
            if (id >= 373 && id <= 376) return 322;
            
            // other elements
            else return id - 9*(id/73 + 1);
        }
        default:
        {
            ::Error("GetVetoInFrontOfElement", "TAPS type could not be identified!");
            return 0;
        }
    }
}

//______________________________________________________________________________
Int_t TOA2Detector::GetTAPSRing(Int_t id, A2TAPSType_t type)
{
    // Return the number of the ring (1 to 11) the TAPS element
    // 'id' belongs to in the TAPS type 'type'.
    
    // element layout (first and last elements of a ring)
    Int_t ringRange[11][2] = {{0, 0}, {1, 2}, {3, 5}, {6, 9}, {10, 14}, {15, 20},
                              {21, 27}, {28, 35}, {36, 44}, {45, 54}, {55, 63}};
             
    // get corresponding TAPS 2007 element number
    Int_t elem = GetVetoInFrontOfElement(id, type);
    
    // map element to first block
    Int_t mid = elem % 64;

    if      (mid >= ringRange[0][0]  && mid <= ringRange[0][1])  return 1;
    else if (mid >= ringRange[1][0]  && mid <= ringRange[1][1])  return 2;
    else if (mid >= ringRange[2][0]  && mid <= ringRange[2][1])  return 3;
    else if (mid >= ringRange[3][0]  && mid <= ringRange[3][1])  return 4;
    else if (mid >= ringRange[4][0]  && mid <= ringRange[4][1])  return 5;
    else if (mid >= ringRange[5][0]  && mid <= ringRange[5][1])  return 6;
    else if (mid >= ringRange[6][0]  && mid <= ringRange[6][1])  return 7;
    else if (mid >= ringRange[7][0]  && mid <= ringRange[7][1])  return 8;
    else if (mid >= ringRange[8][0]  && mid <= ringRange[8][1])  return 9;
    else if (mid >= ringRange[9][0]  && mid <= ringRange[9][1])  return 10;
    else if (mid >= ringRange[10][0] && mid <= ringRange[10][1]) return 11;
    else 
    {
        ::Error("GetTAPSRing", "TAPS ring could not be identified!");
        return 99;
    }
}

//______________________________________________________________________________
Int_t TOA2Detector::GetTAPSBlock(Int_t id, A2TAPSType_t type)
{
    // Return the number of the block (1 to 6) the TAPS element
    // 'id' belongs to in the TAPS type 'type'.
    
    // check TAPS setup
    switch (type)
    {
        case kTAPS_2007:
        {
            return (id / 64) + 1;
        }
        case kTAPS_2008:
        {
            return (id / 67) + 1;
        }
        case kTAPS_2009:
        {
            return (id / 73) + 1;
        }
        default:
        {
            ::Error("GetTAPSBlock", "TAPS type could not be identified!");
            return 0;
        }
    }
}

//______________________________________________________________________________
Bool_t TOA2Detector::IsTAPSPWO(Int_t id, A2TAPSType_t type)
{
    // Return kTRUE if the element 'id' belongs to the PWO rings of TAPS, otherwise
    // return kFALSE.
    
    // check TAPS setup
    switch (type)
    {
        case kTAPS_2007:
        {
            return kFALSE;
        }
        case kTAPS_2008:
        {
            if (GetTAPSRing(id, type) == 1) return kTRUE;
            else return kFALSE;
        }
        case kTAPS_2009:
        {
            if (GetTAPSRing(id, type) <= 2) return kTRUE;
            else return kFALSE;
        }
        default:
        {
            ::Error("IsTAPSPWO", "TAPS type could not be identified!");
            return kFALSE;
        }
    }
}
ClassImp(TOA2Detector)
