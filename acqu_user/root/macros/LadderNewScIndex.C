UInt_t lookupTable[3][352];

void LadderNewScIndex(Char_t* taggfile)
{
// Apply offset to scaler indices found in a TA2Detector configuration file
//
  Char_t taggmod[128];                         // new file to create
  Char_t line[256];                            // input line from file
  Char_t desc[32];                             // general purpose char string
  Char_t* p[16];                               // parameters read from line
  for(Int_t i=0; i<16; i++) p[i] = new Char_t[16];

  strcpy(taggmod, taggfile);                   // copy file of xxx = xxx.align
  strcat(taggmod,".NewScIndex");
  FILE* tfile;                                 // original file pointer
  FILE* mfile;                                 // copy file pointer

  // Open original and copy files
  printf("Offsetting scalers found in  %s\n",taggfile);
  if( (tfile = fopen(taggfile,"r")) == NULL ){ 
    printf("File %s open failed\n",taggfile);
    return;
  }
  if( (mfile = fopen(taggmod,"w")) == NULL ){
    printf("File %s open failed\n",taggmod);
    return;
  }
  // Generate the new Vuprom scaler indices
  initVupromScalers();

  // Print message at top of auto-generated file
  fprintf(mfile,"##     Auto-generated AcquRoot Detector file\n");
  fprintf(mfile,"##     Macro LadderNewScIndex, J.R.M. Annand, 23rd Jan 2015\n");
  // Read lines from original until EOF
  Int_t jj = 0;
  while( fgets( line, 256, tfile ) ){
    sscanf( line, "%s", desc );            // get line descriptor
    if( !strcmp( desc, "Element:") ){      // and check if its an element line
      Int_t n = sscanf( line,"%*s%s%s%s%s%s%s%s%s",
			p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
      n += sscanf( line,"%*s%*s%*s%*s%*s%*s%*s%*s%*s%s%s%s%s%s%s%s%s",
		   p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15] );
      if( n != 16){
	printf("Bad file format\n%s\n",line);
	return;
      }
      Int_t scal;
      sscanf(p[15],"%d",&scal);
      printf("Old index: %d    New index: %d\n",scal,lookupTable[0][jj]);
      scal = lookupTable[0][jj];
      jj++;
      sprintf( p[15],"%d",scal );
      fprintf(mfile,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n", "Element:",
	      p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],
	      p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15] );
    }
    else fputs( line, mfile ); // if not an element line copy it to new file
  }
  // delete any allocated memory and close input and output files 
  for(Int_t i=0; i<16; i++) delete p[i];  
  fclose(tfile);
  fclose(mfile);
  
  return;
}
//                                           
void initVupromScalers(){   
  int index=0;
  int vuprom[4][3]= {  { 128, 224,  32 },      //The start  index of each block of 32 in a 0-352 range of the tagger
		       {  96, 256, 160 },
		       {  64, 288, 192 },
		       {  0,  320,  -1 }};
  
  //The mapping of the channels in the AB block which is all mixed up due to funny cabling
  int abmap[32]   =    {12, 0, 13, 1, 14, 2, 15, 3, 16, 4, 17, 5, 18, 6, 19, 7, 20, 8, 21, 9, 22, 10, 23, 11, 24, 25, 26, 27, 28, 29, 30, 31};
  
  //The no of blocks in the groups for each vuprom module
  int blocks[4]   =   {3,3,3,2};
 

  //fill into the correct histograms of ungated, gated,  and delayed  
  for(int m=0;m<4;m++){                  //for each vuprom module
    for(int g=0;g<3;g++){                   //for each group in the module
      for(int b=0;b<blocks[g];b++){            //for each block in the group
	for(int chan=0;chan<32;chan++){           //for each channel in the block
	  index=2000 + (m*288)+g*(32*blocks[m])+(b*32)+chan; //get the index
	  if(vuprom[m][b]==-1) continue;              //not valid block in vuprom
	  else if(vuprom[m][b]==0) lookupTable[g][vuprom[m][b]+abmap[chan]]=index;
	  else lookupTable[g][vuprom[m][b]+chan]=index;
	}
      }
    }
  }
  return;
}
