//--Author	JRM Annand      4th Apr 2003
//--Rev 	JRM Annand...   8th Jan 2004..TA2Control mod.
//--Rev  	JRM Annand...  18th Feb 2005..Run to TA2Control
//--Rev 	Baya Oussena...28th Jun 2012..Control valid typed commands
//--Update	JRM Annand...  09th Jul 2012..Allow commands with parameters
//--Update  A Neiser...   6th June 2013  Make char_t* const

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// Main program to run the system
//

#include "TDAQexperiment.h"
#include "ARSocket_t.h"

#include <sys/select.h>

// Input command keys
const static Char_t *cmd[] = {
  "Start:",
  "Go:",
  "Run:",
  "Pause:",
  "Stop:",
  "End:",
  "Kill:",
  "Dump:",
  "Experiment:",
  "Comment:",
  "File:",
  "RunNumber:",
  "Auto:",
  "NoAuto:",
  "IsRunning:",
  "ConfigTCS:",
  "SetTCSRunMode:",
  "Camio:",
  "VUPROM:",
  "V874:",
  "NULL:"
};




//______________________________________________________________________________
int main(int argc, char **argv)
{
  // Remote connection to DAQ

  Int_t port;
  Char_t ClientIP[ELineSize];
  Char_t line[ELineSize];
  Char_t command[64];
  switch ( argc ){
  default:
    strcpy( ClientIP, "slarti.a2.kph");
    port = 4000;
    break;
  case 2:
    strcpy( ClientIP, argv[1] );
    port = 4000;
    break;
  case 3:
    strcpy( ClientIP, argv[1] );
    if( sscanf( argv[2],"%d",&port ) < 1 ){
      printf("port variable not scanned\n");
      exit(-1);
    }
    break;
  }
  TDAQexperiment* exp = new TDAQexperiment("Exp",NULL,"RemoteSupervise");
  ARSocket_t* remote = new ARSocket_t("Remote-DAQ",ClientIP,port,ESkRemote,
				      0,0,exp);
  remote->Initialise();
  
  // unused Int_t id = remote->GetID();


  Int_t i;
  // unused Int_t cmp;
  // unused Int_t cmp2;
  
  //  timeval t;
  //  t.tv_sec = 5; // 5 sec timeout
  //  t.tv_usec = 0;
  //  fd_set rfds;
  //  fd_set wfds;
  // FD_ZERO(&rfds);
  // FD_SET( id, &rfds );
  // Int_t status = select( 1, &rfds, NULL, NULL, &t );
  // -1 error, 0 nothing there, + something there

  char *retstr;
  for(;;){
    printf("\nAcquDAQ> ");
    retstr = fgets(line, ELineSize, stdin);
    // unused Int_t length = strlen(line);
    printf("\n %s", line);
    sscanf(line,"%s",command);
    i = 0;
 
    while( ( strcmp(command,cmd[i]) )  && ( strcmp(cmd[i],"NULL:") ) ){     
      i++;
    }

    if ( !strcmp(command,cmd[i])  )
    {
    remote->WriteChunked( line, ELineSize, ELineSize );
    remote->ReadChunked( line, ELineSize, ELineSize );
    printf(line);
    if( !strcmp(line,"Shutting down DAQ\n") ) break;
    }
    else { 
      printf("Valid Commands are : \n");
      i=0;
      while( strcmp(cmd[i],"NULL:") )
	{  
	  printf("%s \n",cmd[i]);  
	  i++;
	}
    }  
  }
}


