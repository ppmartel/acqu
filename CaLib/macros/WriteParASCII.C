// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// WriteParASCII.C                                                      //
//                                                                      //
// Write calibration parameters from an ASCII file.                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void WriteParASCII()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // configuration
    const Char_t inputFile[] = "gain.dat";
    const Char_t data[] = "Data.TAPS.SG.E1";
    const Char_t calibration[] = "LD2_Dec_07";
    Int_t set = 0;

    // par array
    Double_t par[720];
    Int_t nPar = 0;

    // read from file (one value per line)
    Char_t* line[256];
    FILE* f = fopen(inputFile, "r");
    while (fgets(line, 256, f)) sscanf(line, "%lf", &par[nPar++]);
    fclose(f);

    // show paramters
    for (Int_t i = 0; i < nPar; i++) printf("Par %03d : %lf\n", i, par[i]);
    printf("Read %d paramters\n", nPar);
    
    // ask user
    Char_t answer[128];
    printf("Write to set %d of calibration '%s'? (yes/no) : ", 
           set, calibration);
    scanf("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        gSystem->Exit(0);
    }

    // write to database
    TCMySQLManager::GetManager()->WriteParameters(data, calibration, set, par, nPar);
  
    gSystem->Exit(0);
}

