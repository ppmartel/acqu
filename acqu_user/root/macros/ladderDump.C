void ladderDump(char *file = "data/ladder_scalers.dat"){
  FILE *fp;
  
    fp=fopen(file,"w");
    fprintf(fp,"# 'F' must be included if file has three columns!\n");
    fprintf(fp,"F\n");
    fprintf(fp,"# Contents of current scaler buffer\n");
    fprintf(fp,"# Channel\tContent\tGatedDifference\n");
    for(int n=1;n<=352;n++){
      fprintf(fp,"%d %10.1f %10.1f\n",n,FPD_ScalerAcc->GetBinContent(n),(FPD_ScalerPromptAcc->GetBinContent(n) - FPD_ScalerRandAcc->GetBinContent(n)));
    }
    fclose(fp);
}
