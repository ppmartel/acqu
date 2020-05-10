void NewFPDGenRecabled(Int_t tdcStart=800, Int_t scalerStart=2000,
	       Int_t n=328, Char_t* calFile=NULL, Double_t eBeam=0.0)
{
  Int_t OrderTDC[] =
    {17,  0,  1, 16, 19,  2,  3, 18, 21,  4,  5, 20, 23,  6,  7, 22,
     25,  8,  9, 24, 27, 10, 11, 26, 29, 12, 13, 28, 31, 14, 15, 30};

  Int_t BaseTDC[] = {32,0,160,128,288,256};

  Int_t OrderScaler[] =
    { 3,  0,  2,  1,  7,  4,  6,  5, 11,  8, 10,  9, 15, 12, 14, 13,
     19, 16, 18, 17, 23, 20, 22, 21, 27, 24, 26, 25, 31, 28, 30, 29};

  Int_t BaseScaler[] = {64, 864, 352, 0, 608, 320, 288, 576, 32, 640, 928, 896};

  Int_t Channel = 0;
  Int_t OrderIndex = 0;

  Int_t TDC;
  Int_t Scaler;
  for(Int_t l=0; l<8; l++){
    for(Int_t i=0; i<=1; i++){
      for(Int_t j=0; j<6; j++){
	for(Int_t k=0; k<4; k++){
	  if(Channel>327) continue;
	  TDC = tdcStart+BaseTDC[j]+OrderTDC[k+OrderIndex]+(64*i);
	  Scaler = scalerStart+BaseScaler[j+(6*i)]+OrderScaler[k+OrderIndex];
	  printf("Channel %3d\tTDC %4d\tScaler %4d\n",Channel,TDC,Scaler);
	  Channel++;
	}
      }
    }
    OrderIndex += 4;
  }
}
