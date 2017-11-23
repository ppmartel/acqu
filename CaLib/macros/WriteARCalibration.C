// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// WriteARCalibration.C                                                 //
//                                                                      //
// Write AcquRoot calibration files using CaLib.                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void WriteARCalibration()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // write tagger calibration file
    TCWriteARCalib w(kDETECTOR_TAGG, "FP.dat");
    w.Write("new_FP.dat", "LD2_Dec_07", 13840);

    // write CB calibration file
    TCWriteARCalib w1(kDETECTOR_CB, "NaI.dat");
    w1.Write("new_NaI.dat", "LD2_Dec_07", 13840);

    // write TAPS calibration file
    TCWriteARCalib w2(kDETECTOR_TAPS, "BaF2.dat");
    w2.Write("new_BaF2.dat", "LD2_Dec_07", 13090);

    // write PID calibration file
    TCWriteARCalib w3(kDETECTOR_PID, "PID.dat");
    w3.Write("new_PID.dat", "LD2_Dec_07", 13840);

    // write Veto calibration file
    TCWriteARCalib w4(kDETECTOR_VETO, "Veto.dat");
    w4.Write("new_Veto.dat", "LD2_Dec_07", 13840);

    gSystem->Exit(0);
}

