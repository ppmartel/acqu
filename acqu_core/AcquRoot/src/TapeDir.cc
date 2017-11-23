//--Author	JRM Annand    4th Apr 2003
//--Rev 	JRM Annand... 8th Jan 2004..TA2Control mod.
//--Update	JRM Annand...18th Feb 2005..Run to TA2Control
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Utility to read and print header blocks read from files on tape
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <string.h>
#include "TA2Mk1Format.h"

void PrintRun( Char_t* );

//______________________________________________________________________________
int main(int argc, char **argv)
{
  // Set default tape device and record length
  // Command-line arguments can override these

  Int_t recLength = 32768;             // default record length
  Char_t device[64];
  strcpy( device,"/dev/nst0" );        // default linux tape device

  switch (argc ){
  case 1:
    break;
  default:
  case 3:                             // override record length
    if( sscanf( argv[2], "%d", &recLength ) != 1 ){
      printf(" TapeDir: invalid command-line arguments %s %s\n",
	     argv[1], argv[2] );
      exit(-1);
    }
    // don't break here
  case 2:                            // override device
    strcpy( device, argv[1] );
    break;
  }
  printf(" Tape Directory on device: %s, data record length %d\n\n",
	 device, recLength );
  int path = open(device,O_RDONLY);
  if( path < 0 ){
    printf(" Failed to open device: %s\n\n", device);
    exit(-1);
  }
  struct mtop mtOp;                 // for raw tape control
  mtOp.mt_op = MTREW;               // rewind
  mtOp.mt_count = 1;                // once
  if( ioctl(path,MTIOCTOP,&mtOp) == -1 ){
    printf(" Failed to rewind tape device %s\n\n",device);
    close( path );
    exit(-1);
  }
  printf(" Tape device %s rewinding\n", device);
  Char_t* buff = new Char_t[recLength];
  Int_t nByte;

  for(;;){
    nByte = read( path, buff, recLength );
    if( nByte != recLength ){
      printf(" Incomplete read %d bytes of expected %d\n", nByte, recLength);
      break;
    }
    PrintRun(buff);
    mtOp.mt_op = MTFSF;
    if( ioctl(path,MTIOCTOP,&mtOp) == -1 ){
      printf(" Failed to skip to next file on tape device %s\n\n",device);
      break;
    }
  }
  printf(" TapeDir....no more data on drive: %s\n\n", device);
  close( path );
}

//______________________________________________________________________________
void PrintRun( Char_t* buff )
{
  AcquExptInfo_t* ah = (AcquExptInfo_t*)(buff + sizeof(Int_t));
  printf(" Run Number: %d, Date/Time: %s\n", ah->fRun, ah->fTime );
  printf(" %s\n", ah->fDescription );
  printf(" %s\n", ah->fRunNote );
  printf(" ____________________________________________________________\n\n");
}
 

