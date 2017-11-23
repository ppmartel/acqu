#include "TProcessor.h"
#include "TRint.h"
#include "TThread.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

#define CORES 16

TProcessor* Proc[CORES];
TThread* Work[CORES];

/* specify the expected options */
static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"offline", no_argument, 0, 'o'},
	{"thread", required_argument, 0, 't'},
	{0, 0, 0, 0}
};


/* print a short help message and exit */
void print_help()
{
	std::cout << "To use the Woker run it this way: Worker [options] config-file" << std::endl
		<< "Options which can be used:" << std::endl
		<< " --help, -h     display this help and exit" << std::endl
		<< " --thread, -t   specify how many threads should be used," << std::endl
		<< "                number as argument needed" << std::endl
		<< " --offline, -o  run in offline mode" << std::endl;

	exit(EXIT_SUCCESS);
}


/* method checks if the given string is an unsigned decimal */
static unsigned int is_udec(char* const str)
{
	char *p = &str[0];

	while (*p)
		if (!isdigit(*p++))
			return 0;

	return str[0];
}

/* method checks if a given string is a decimal using is_udec() */
static unsigned int is_decimal(char* const str)
{
	return (is_udec(&str[(str[0] == '-') || (str[0] == '+')]));
}


//-----------------------------------------------------------------------------

void StartThread(Int_t Number)
{
  Proc[Number]->Run();
}


Int_t Negotiate(Int_t pid, Int_t nThreads)
{
  Char_t Buffer[256];
  Int_t instances = 0;
  Int_t runThreads = nThreads;
  std::vector<pid_t> pids;
  FILE* f;

  //Create flag file for this Worker process
  sprintf(Buffer, "/tmp/Worker%d", pid);
  f = fopen(Buffer, "w");
  if(!f)  //Terminate if flag file couldn't be created
  {
    printf("Error: Couldn't negotiate number of threads\n");
    exit(1);
  }

  //Search for running Worker instances and count corresponding threads
  char line[1024];
  FILE *cmd = popen("pidof Worker", "r");
  if(fgets(line, 1024, cmd));
  //pid_t pid = strtoul(line, NULL, 10);
  char *pid_ptr;
  //split pids of current Worker instances 
  pid_ptr = strtok(line, " ");
  while(pid_ptr != NULL){
    pids.push_back(strtoul(pid_ptr, NULL, 0));
    pid_ptr = strtok(NULL, " ");
  }
  pclose(cmd);
  //instances = pids.size();

  FILE* flag;
  int t;
  //Search for all Worker flag files and count running threads
  for(std::vector<pid_t>::iterator it = pids.begin(); it != pids.end(); ++it)
  {
    if(int(*it) == pid) continue;  //do not consider pid of this Worker
    sprintf(Buffer, "/tmp/Worker%d", *it);
    flag = fopen(Buffer, "r");
    if(flag)
    {
      if(fscanf(flag, "%d", &t));
      fclose(flag);
      instances += t;
    }
  }

  //Calculate (with rounding-down) a fair share of all cores
  if(instances) runThreads = ((4*CORES-instances) / pids.size())*4 / (nThreads/2+1);  //try to compute number of used threads regarding the total amount of running threads and the users desired thread count
  if(runThreads < 1) runThreads = 1;
  if(runThreads > nThreads) runThreads = nThreads;
  //write number of threads to file
  fprintf(f, "%d", runThreads);  //write running threads to file and read it to determine overall threads running via pid (with this method killed workers aren't considered)
  fclose(f);

  return runThreads;
}

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  Int_t CountSim = 0;
  Int_t CountDat = 0;
  Int_t Count;
  Int_t Process;
  Int_t runThreads;  //threads started within this Worker instance
  Int_t runThreadsOld;
  pid_t pid;
  Int_t nThreads = 8;  //user's choice of threads to run
  Bool_t Offline = false;
  Bool_t Busy;
  Char_t Config[256];
  Char_t Server[256];
  Char_t Buffer[2][256];
  Char_t Line[1024];
  Char_t NameSim[1024][256];
  Char_t NameDat[1024][256];
  Char_t ConfigText[524288];
  Char_t ServerText[524288];
  Int_t Records[1024][2];
  Int_t Number[2];
  FILE* ConfigFile;
  FILE* ServerFile;
  FILE* ThreadConfig;
  FILE* ThreadServer;

	// variables used for getopt
	opterr = 0;  // use own error messages instead of getopt buildin ones
	int opt;
	int option_index;
	// used to check for correct file argument
	bool file = false;

	/**
	 * Handle command-line options
	 *
	 * POSIX: if the first letter of the optstring is a colon, a colon is returned if an argument is missing, otherwise a questionmark
	 * With only a minus at the beginning, which is used to detect strings as parameters (files) in GNUs getopt implementation with the 'letter' ^A ('\1'), a questionmark is returned. But it _has_ to be at the beginning, otherwise strings are not detected. It seems to work also with a colon after the minus. That's why the string starts with "-:"
	 */
	while ((opt = getopt_long(argc, argv, "-:ht:c:o", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'o':
				Offline = true;
				break;
			case 't':
			case 'c':  // flag -c for backward compatibility
				if (!is_decimal(optarg)) {
					fprintf(stderr, "ERROR: parameter value for thread flag '%s' is not an integer!\n", optarg);
					exit(EXIT_FAILURE);
				}
				nThreads = atoi(optarg);
				if (!nThreads) {
					printf("WARNING: Given number of threads is zero, use 4 instead\n");
					nThreads = 4;
				}
				if (nThreads < 0) {
					printf("WARNING: Given number of threads is negative, use absolute value\n");
					nThreads *= -1;
				}
				if (nThreads > CORES)  // check if nThreads is low enough, otherwise set it to the maximum CORES
					nThreads = CORES;
				break;
			case 'h':
				print_help();
				break;
			case '\1':
				if (file) {
					fprintf(stderr, "Invalid parameters: only one file allowed as an argument\n");
					exit(EXIT_FAILURE);
				}
				// for path-less config filename, append 'data' directory
				if (!strrchr(optarg, '/')) {
					sprintf(Buffer[0], "data/%s", optarg);
					strcpy(Config, Buffer[0]);
				} else
					strcpy(Config, optarg);
				ConfigFile = fopen(Config, "r");
				if (!ConfigFile) {
					fprintf(stderr, "Error opening file %s: %s\n", Config, strerror(errno));
					exit(EXIT_FAILURE);
				}
				printf("Use config file: %s\n", optarg);
				file = true;
				break;
			case ':':
				// option argument is missing
				fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
				exit(EXIT_FAILURE);
			case '?':
			default:
				fprintf(stderr, "%s: option '-%c' is invalied\nUsage: %s file [-hto]\n", argv[0], optopt, argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (!file) {
		fprintf(stderr, "ERROR: no config file as parameter given\n");
		return EXIT_FAILURE;
	}

  //Get current system pid of this Worker process
  pid = getpid();
  //Negotiate number of threads (depending on number of already running Worker processes)
  runThreads = Negotiate((int)pid, nThreads);

  //Print useless startup information
  printf("\n*** Worker - parallel AcquRoot processing ***\n\n");
  if(Offline)
    printf("Using offline mode (processing simulated files)\n");
  else
    printf("Using Dataserver mode (processing data files)\n");
  printf("Using maximum %d threads\n", nThreads);
  printf("Using AcquRoot configuration from file %s\n", Config);

  // Use config file and search for a) simulation file names and b) server configuration file name
  while(!feof(ConfigFile))
  {
    if(!fgets(Line, sizeof(Line), ConfigFile)) break;
    Buffer[0][0] = '\0';
    sscanf(Line, "%s %s", Buffer[0], Buffer[1]);
    //If current line contains a simulation file name...
    if(!strcmp(Buffer[0], "TreeFile:"))
    {
      //...copy filename to processing list...
      strcpy(NameSim[CountSim++], Buffer[1]);
    }
    else if(!strcmp(Buffer[0], "ServerSetup:"))
    {
      //...otherwise pick up server configuration file...
      strcpy(Server, Buffer[1]);
    }
    else
    {
      //...or add line to config file template
      strcat(ConfigText, Line);
    }
  }
  fclose(ConfigFile);

  if(!Offline)
  {
    //For path-less server filename, append 'data' directory
    if (!strrchr(Server, '/')) {
      sprintf(Buffer[0], "data/%s", Server);
      strcpy(Server, Buffer[0]);
    }

    //Open server file and search for .dat file names
    ServerFile = fopen(Server, "r");
    if (!ServerFile) {
		fprintf(stderr, "Error opening server file %s: %s\n", Server, strerror(errno));
		exit(EXIT_FAILURE);
	}
    while(!feof(ServerFile))
    {
      if(!fgets(Line, sizeof(Line), ServerFile)) break;
      Buffer[0][0] = '\0';
      sscanf(Line, "%s %s %d %d", Buffer[0], Buffer[1], &Number[0], &Number[1]);
      //If current line contains a file name information...
      if(!strcmp(Buffer[0], "File-Name:"))
      {
        //...copy filename to processing list...
        strcpy(NameDat[CountDat], Buffer[1]);
        Records[CountDat][0] = Number[0];
        Records[CountDat][1] = Number[1];
        CountDat++;
      }
      else
      {
        //...otherwise add line to server file template
        strcat(ServerText, Line);
      }
    }
    fclose(ServerFile);
  }

  //Create worker threads
  for(Int_t Number=0; Number < nThreads; Number++)
  {
    sprintf(Buffer[0], "Processor%d", Number);
    Proc[Number] = new TProcessor(Buffer[0]);
    Proc[Number]->SetNumber(Number);
    Proc[Number]->SetRunning(false);
    Work[Number] = new TThread("ProcessorThread", (void(*)(void*))&(StartThread), 
                               (void*)(intptr_t)(Number), TThread::kNormalPriority);
    //Delete old log files
    sprintf(Buffer[0], "Thread%XLog.txt", Number);
    unlink(Buffer[0]);
  }

  if(Offline) Count = CountSim; else Count = CountDat;

  Process = 0;
  while(Process < Count)
  {
    for(Int_t Number=0; Number < runThreads; Number++)
    {
      if(!(Proc[Number]->GetRunning()))
      {
        //Lock thread
        Proc[Number]->SetRunning(true);

        //Create config and (if required) server file
        sprintf(Buffer[0], "data/Thread%XConfig.dat", Number);
        ThreadConfig = fopen(Buffer[0], "w");
        fprintf(ThreadConfig, "%s", ConfigText);
        if(Offline)
          fprintf(ThreadConfig, "\nTreeFile: %s\n", NameSim[Process]);
        else
        {
          sprintf(Buffer[1], "Thread%XServer.dat", Number);
          fprintf(ThreadConfig, "\nServerSetup: %s\n", Buffer[1]);
          sprintf(Buffer[1], "data/Thread%XServer.dat", Number);
          ThreadServer = fopen(Buffer[1], "w");
          fprintf(ThreadServer, "%s", ServerText);
          fprintf(ThreadServer, "\nFile-Name: %s %d %d\n", NameDat[Process], Records[Process][0], Records[Process][1]);
          fclose(ThreadServer);
        }
        fclose(ThreadConfig);

        //Create AcquRoot parameters and (if required) command-line flags
        if(Offline)
          sprintf(Buffer[0], "--offline Thread%XConfig.dat", Number);
        else
          sprintf(Buffer[0], "Thread%XConfig.dat", Number);
        Proc[Number]->SetConfig(Buffer[0]);
        //Start AcquRoot process
        Work[Number]->Run();
        //Print some process informations
        if(Offline)
          printf("Processing file %s (offline mode) on thread %2d\n", NameSim[Process], Number);
        else
          printf("Processing file %s (Dataserver mode) on thread %2d\n", NameDat[Process], Number);
        //Some delay to prevent starting all processes at the same time)
        sleep(5);

        Process++;
        if(Process == Count) break;
      }
    }

    //Go to sleep before checking again for free threads
    sleep(30);
    //Re-negotiate number of cores (might have changed due to newly started other Workers)
    runThreadsOld = runThreads;
    runThreads = Negotiate((int)pid, nThreads);
    if(runThreads != runThreadsOld)
      printf("Changing from %d to %d threads\n", runThreadsOld, runThreads);
  }

  //When all files have been distributed, wait for running threads to be finished
  Busy = true;
  while(Busy)
  {
    Busy = false;
    for(Int_t Number=0; Number < nThreads; Number++)
      Busy = (Busy || Proc[Number]->GetRunning());
    sleep(30);
  }

  //Remove up flag file for this Worker process
  sprintf(Buffer[0], "/tmp/Worker%d", pid);
  unlink(Buffer[0]);
  //We're done
  printf("Finished\n");
  return 0;
}

//-----------------------------------------------------------------------------

