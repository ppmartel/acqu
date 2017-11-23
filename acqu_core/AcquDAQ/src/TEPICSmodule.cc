//--Author	Ken Livingston    23nd Sep 2012  Added support for EPICS
//--Rev 	
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TEPICSModule
// Basic class for handling EPICS slow control data
//
// 2 modes:
// Read / Write - To Write EPICS channels into the data stream as EPICS buffers.
// 
// Read Only    - To decode EPICS buffers in the data stream
//                Independent of needing any EPICS installation on the analysis side.
//
//
// Here's what an epics event looks like in the buffer (see EPICS structs in TA2Mk2Format.h)
//
// Descr: | Header                 | Channel 0       | Channel 1       | ..... | Channel N       |   
// Bytes: | 32   8   2   2   2   2 | 32  2  2  2  D1 | 32  2  2  2  D2 | ..... | 32  2  2  2  DN |
// Key:   | H0  H1  H2  H3  H4  H5 | C0 C1 C2 C3  D1 | C0 C1 C2 C3  D2 | ..... | C0 C1 C2 C3  DN |
//
//struct EpicsHeaderInfo_t {      //header for epics buffers and channels
//  Char_t name[32];              //H0 Name of EPICs module
//  time_t time;                  //H1 Time of event
//  Short_t index;                //H2 index of module
//  Short_t period;               //0 = start only, +ve = period in counts, -ve = -1 * period in ns.  
//  Short_t id;                   //H3 id of epics module (user set)
//  Short_t nchan;                //H4 no of EPICs channels in module
//  Short_t len;                  //H5 length of EPICS buffer
//};
  
//struct EpicsChannelInfo_t{      //header for channel info
//  Char_t pvname[32];            //Process variable name
//  Short_t bytes;                //No of bytes for this channel
//  Short_t nelem;                //No of elements in array
//  Short_t type;                 //Type of element
//};
 
#include "TEPICSmodule.h"
#include "inttypes.h"
 
#ifdef ACQUROOT_EPICS      //only include channel access if it's available (ie EPICS environment is defined
#include "ezcaRoot.h"      //header for the library of root wrappers for ezca channel access functions
#endif


enum  EepicsType{              EepicsBYTE,    EepicsSTRING,     EepicsSHORT,    EepicsLONG,    EepicsFLOAT,    EepicsDOUBLE, EepicsNULL};
const char *epicsTypeName[] = {"epicsBYTE",   "epicsSTRING",    "epicsSHORT",   "epicsLONG",   "epicsFLOAT",   "epicsDOUBLE",      NULL};
enum  EepicsTypeSize{           ESizeBYTE = 1, ESizeSTRING = 40, ESizeSHORT = 2, ESizeLONG = 8, ESizeFLOAT = 4, ESizeDOUBLE = 8};
const int   epicsTypeSize[] = { ESizeBYTE,     ESizeSTRING,      ESizeSHORT,     ESizeLONG,     ESizeFLOAT,     ESizeDOUBLE};
//For some reason there's no INT in ezca channel access - not sure why
//And STRING doesn't work

enum EModes{ EEPICS_TIMER_MODE, EEPICS_SCALER_MODE};

//defs for reading the setup file
enum { EEPICS_SETUP, EEPICS_CHANNEL};

static Map_t kEPICSKeys[] = {
  {"EPICSBaseSetup:",       EEPICS_SETUP},
  {"EPICSchannel:",         EEPICS_CHANNEL},
  {NULL,                    -1}
}; 


//-----------------------------------------------------------------------------

TEPICSmodule::TEPICSmodule(  Char_t* name, Char_t* file, FILE* log, Char_t* line):TDAQmodule(name, file, log){
  
  if(name==NULL){     //no need init anything, just using buffer decoding functions 
    fReadOnly=kTRUE;  //set read only flag and return
    return;
  }
  
  
#ifndef ACQUROOT_EPICS
  PrintError( line, "<EPICS Channel Access support not built. Set up EPICS environment and remake AcquDAQ, or remove EPICS from setup file>", EErrFatal );
#endif
  
  FILE *fp;                                   //to have a sneaky look at the config file
  Char_t line1[128];
  Char_t CommString[128];
  Int_t nChannel=0;
  Int_t n=0;
  Char_t mode[16];

  // Basic initialisation.
  // 
  AddCmdList( kEPICSKeys );                  // physics-specific cmds

  fEpicsIndex=0;  
  fReadOnly = kFALSE;
  fType = EDAQ_Epics;
  fBuffLen = 0;                              //init some things
  fNChannels = 0;
  fTimer = NULL;
  fVerbose = kFALSE;
  fIsTimedOut = kFALSE;
  fIsCountedOut = kFALSE;
  fIsTimed = kFALSE;
  fIsCounted = kFALSE;
  if( line ){                              
    if( (n = sscanf( line, "%*s%*s%*s%s%ld",mode,&fPeriod )) < 2 ) //scan in the mode and period
      PrintError( line, "<EPICS setup line parse>", EErrFatal );
  }

  //set the period mode to timer or scaler 
  if ( strcmp(mode,"timer") == 0 ) fPeriodMode=EEPICS_TIMER_MODE;
  else if ( strcmp(mode,"scaler") == 0 ) fPeriodMode=EEPICS_SCALER_MODE;
  else PrintError( line, "<EPICS setup line parse1>", EErrFatal );
  
  if((fPeriodMode==EEPICS_TIMER_MODE)&&(fPeriod)){
    fTimer=new TTimer(this, fPeriod, kFALSE);             //timer mode and period non-zero, set up the timer.
    fIsTimed = kTRUE; 
    }
  if((fPeriodMode==EEPICS_SCALER_MODE)&&(fPeriod)){
    fIsCounted = kTRUE; 
  }
  
  //open the config file an decide how many EPICS_CHANNEL lines there are to set the sizes of some arrays
  //the actual parsing of this gets done later in SetConfig
  if ( (fp = fopen(file,"r")) == NULL ) {
    PrintError(file,"<File not Found>",EErrFatal);
  }
  while( fgets(line1,sizeof(line1),fp) != NULL ) {           // read till EOF
    sscanf(line1,"%s",CommString);                           //scan the 1st field
    if(strcmp(CommString,"EPICSchannel:")==0) nChannel++;    //count the EPICSchannel lines
  }
  fclose(fp);
  
  //make some arrays of the appropriate size
  fNelem        = new Int_t[nChannel];
  fChannelType  = new Int_t[nChannel];
  fChannelAddr  = new void *[nChannel];
  fChannelData  = new void *[nChannel];
  fPVName       = new Char_t *[nChannel];
  for(int n=0;n<nChannel;n++){fPVName[n]=new Char_t[32];}
}


//-----------------------------------------------------------------------------
void TEPICSmodule::SetConfig( Char_t* line, Int_t key ){
  int n=0;
  int typeIndex=0;
  char typeName[32];

  if(fReadOnly){
    fprintf(stderr,"Warning epics is in ReadOnly mode\n");
    return;
  }

  // Configuration from file
  switch( key ){
  case  EEPICS_SETUP:
    if( (n = sscanf( line, "%hd%s",&fEPICSID,fName )) < 2 ) //scan in the period
      PrintError( line, "<EPICS setup line parse>", EErrFatal );
    break;
    
  case EEPICS_CHANNEL:
   if( (n = sscanf( line, "%s%d%s",fPVName[fNChannels],&fNelem[fNChannels],typeName)) < 3 ) //scan in the period
     PrintError( line, "<EPICS setup line parse>", EErrFatal );
   
   for(typeIndex=0;typeIndex<=EepicsNULL;typeIndex++){
     if(strcmp(epicsTypeName[typeIndex],typeName)==0){
       fChannelType[fNChannels]=typeIndex;
       break;
     }
   }
   if(typeIndex>=EepicsNULL) 
     PrintError( line, "<EPICS setup line parse>", EErrFatal ); 
   
   fNChannels++;
   break;

  default:
    // default try commands of TDAQmodule
    TDAQmodule::SetConfig(line, key);
    break;
  }

  

}

void TEPICSmodule::PostInit(){
  Char_t *ptr;                                 //running pointer
  EpicsHeaderInfo_t *bhead;                    //buffer header
  EpicsChannelInfo_t *chead;                   //channel header
  if(fReadOnly){
    fprintf(stderr,"Warning epics is in ReadOnly mode\n");
    return;
  }

  //Work out the length of the EPICS buffer
  fBuffLen=sizeof(EpicsHeaderInfo_t);                          //size of the header
  for(int n=0; n<fNChannels; n++){                             //loop over all the channels and add space
    fBuffLen += sizeof(EpicsChannelInfo_t);                    //chan header
    fBuffLen+=fNelem[n]*epicsTypeSize[fChannelType[0]];        //size of array
  }
  fBuffLen=fBuffLen+(sizeof(UInt_t)-(fBuffLen%sizeof(UInt_t))); //AcquRoot assumes bufflen is multiple of UInt_t
                                                                //this pads it out
  fDataBuffer = new Char_t[fBuffLen];         //create the buffer
  ptr = (char *)fDataBuffer;                  //init ptr to start of data buffer

  bhead=(EpicsHeaderInfo_t*)fDataBuffer;      //fill the buffer header
  bhead->index=(Short_t)fEpicsIndex;
  bhead->id=fEPICSID;
  strncpy(bhead->name,fName,32);

  if(fPeriodMode==EEPICS_TIMER_MODE){
    bhead->period=-1*fPeriod;
  }
  else  if(fPeriodMode==EEPICS_SCALER_MODE){
    bhead->period=fPeriod;
  }
  else{
   bhead-> period=0;
  }
   
  bhead->nchan=fNChannels;
  bhead->len=fBuffLen;
  ptr+=sizeof(EpicsHeaderInfo_t);             //move over the header

  for(int n=0; n<fNChannels; n++){            //loop over all the channels
    chead=(EpicsChannelInfo_t*)ptr;           //channel header
    strncpy(chead->pvname,fPVName[n],32);       //copy name to buffer and move on
    chead->bytes=sizeof(EpicsChannelInfo_t)+(fNelem[n]*epicsTypeSize[fChannelType[n]]); // work out how many bytes of data
    chead->nelem=fNelem[n];
    chead->type=fChannelType[n];
    ptr+=sizeof(EpicsChannelInfo_t);          //move over the header   
    fChannelData[n]=(void *)ptr;              //store the data address of each channel
    ptr+=(fNelem[n]*epicsTypeSize[fChannelType[n]]);
  }     
  //Now fill up dome module information - misuse some of the standard hardware module params 
  //These get put in the header using the 
  fID=EDAQ_Epics;                     // Acqu ID # of module
  fType=EDAQ_Epics;	              // 
  fBaseIndex=fBuffLen;                // length of the epics buffer for this module ->fAmin in header
  fNChannel=fNChannels;               // n epics channels in this epics module
  fNBits=fEpicsIndex;                 // use this to store the index of the EPICS module
}

//Write EPICS variables through channel access into data buffer
void TEPICSmodule::WriteEPICS(void **outBuffer){
  
  if(fReadOnly){
    fprintf(stderr,"Warning epics is in ReadOnly mode\n");
    return;
  }

  EpicsHeaderInfo_t *bhead;                    //buffer header
  Char_t *buff = (Char_t *)(*outBuffer);
  Char_t *localBuff = fDataBuffer;

  //Write time of the EPICS event into the header
  bhead=(EpicsHeaderInfo_t*)localBuff;         //fill the buffer header
  bhead->time = time(NULL);

  // Read out the channels
  for(int n=0; n<fNChannels; n++){           //loop over all the channels
    //    fprintf(stderr, "Name: %s, type:  %hd, Elements: %d\n",fPVName[n],(Short_t)fChannelType[n],fNelem[n]);

#ifdef ACQUROOT_EPICS // if compiled with EPICS support, do the actual readout if not, we've already exited with FATAL ERROR in constructor.
    epicsGet(fPVName[n],(Char_t)fChannelType[n],fNelem[n],fChannelData[n]);    
#endif
    
  }
  if(fVerbose)DumpBuffer(NULL,NULL);         //if we're running verbose, dump the buffer to stdout  
  
  for(int n=0;n<fBuffLen;n++){               //copy the data to the output buffer
    *buff++ = *localBuff++;
  }
  *outBuffer = buff;                         //move the pointer in the output buffer
}

void TEPICSmodule::DumpBuffer(Char_t *buffer, Char_t *outfile){
  //dump the buffer using only the information in the buffer
  EpicsHeaderInfo_t *bhead;                    //buffer header
  EpicsChannelInfo_t *chead;                   //channel header
  time_t timestamp;
  FILE *fp=stdout;
  Char_t *ptr=NULL;

    
  if(buffer) ptr = buffer; //if external buffer is given, use it
  else ptr = fDataBuffer;  //else do the local puffer

  if(outfile){
    if( (fp=fopen(outfile,"a")) == NULL){
      fprintf(stderr,"FATAL ERROR failed to open %s\n",outfile);
      exit(1);
    }
  }

  bhead=(EpicsHeaderInfo_t*)ptr;                 //map the buffer header 
  timestamp = bhead->time;
  if(bhead->period<0){
    fprintf(fp, "\n****  Epics Event: Module Index: %hd, Event ID: %d, Event Name: \"%s\", Period:%hd(ns), Recorded at %s\n",
	    bhead->index, bhead->id=fEPICSID, bhead->name, -1*(bhead->period), asctime(localtime(&timestamp)));  //print header info
  }
  else if(bhead->period==0){
    fprintf(fp, "\n****  Epics Event: Module Index: %hd, Event ID: %d, Event Name: \"%s\", Period: Start Only, Recorded at %s\n",
	    bhead->index, bhead->id=fEPICSID, bhead->name, asctime(localtime(&timestamp)));  //print header info
  }
  else{
    fprintf(fp, "\n****  Epics Event: Module Index: %hd, Event ID: %d, Event Name: \"%s\",  Period:%hd(sclr), Recorded at %s\n",
	    bhead->index, bhead->id=fEPICSID, bhead->name,  bhead->period, asctime(localtime(&timestamp)));  //print header info
  }
  
  ptr+=sizeof(EpicsHeaderInfo_t);                //move past the header

  for(int n=0; n<bhead->nchan;n++){              //loop over all the channels
    chead=(EpicsChannelInfo_t*)ptr;              //point to the channel header

    if(chead->nelem==1)fprintf(fp,"Epics Channel: \"%s\" (%s)\nValue:\t",chead->pvname,epicsTypeName[chead->type]);    //if it's single variable
    else fprintf(fp,"Epics Channel: \"%s\" (%s[%d])\nValue:\t",chead->pvname,epicsTypeName[chead->type],chead->nelem); //if array

    ptr+=sizeof(EpicsChannelInfo_t);            //move past channel header to data

    for(int e=0;e<chead->nelem;e++){             //loop over all elements in array
      switch(chead->type){                       //work out type and print formatted as appropriate
      case EepicsBYTE:
	fprintf(fp, "%c\t",*((Char_t *)(ptr)));
	break;
      case EepicsSTRING:
	fprintf(fp, "%s\t",((Char_t *)(ptr)));
	break;
      case EepicsSHORT:
	fprintf(fp, "%hd\t",*((Short_t *)(ptr)));
	break;
      case EepicsLONG:
	fprintf(fp, "%ld\t",*((Long_t *)(ptr)));
	break;
      case EepicsFLOAT:
	fprintf(fp, "%e\t",*((Float_t *)(ptr)));
	break;
      case EepicsDOUBLE:
	fprintf(fp, "%le\t",*((Double_t *)(ptr)));
	break;
      default:
	fprintf(fp, "WARNING: Unknown epics data type: %d\n",chead->type);
	return;
	break;
      }
      ptr+=epicsTypeSize[chead->type];
      if((e+1)%8==0)fprintf(fp,"\n\t");    //print out in rows of 8 values if it's an array
    }
    fprintf(fp,"\n\n");
    
  }
  fprintf(fp,"*****************************************************************\n\n");      
 if(outfile)fclose(fp);
}  


Bool_t TEPICSmodule::HandleTimer(TTimer* timer){
  if(fReadOnly){
    fprintf(stderr,"Warning epics is in ReadOnly mode\n");
    return kFALSE;
  }

  if(timer!=fTimer) return kFALSE;
  else{
    fIsTimedOut=kTRUE;  //Called when the timer times out
    return kTRUE;
  }
};

Int_t TEPICSmodule::GetBufferInfo(Short_t *index, Short_t* period, Short_t *id, Char_t *EventName, Short_t *nchan, Char_t *epics_buffer, time_t *timestamp){

  EpicsHeaderInfo_t *bhead=(EpicsHeaderInfo_t*)epics_buffer;     //buffer header

  *index = bhead->index;
  *period = bhead->period;
  *id = bhead->id;
  strcpy(EventName,bhead->name);
  *nchan = bhead->nchan;
  *timestamp = bhead->time;
  return 0;
}

void *TEPICSmodule::GetChannel(Char_t *PVName, Int_t *type, Char_t *epics_buffer, Float_t *outbuff, Int_t *nElem){
  // Return a pointer to the data of an EPICS channel
  // PVName       - name of the channel (process variable in EPICS language
  // type         - from enumerator at top here
  // epics_buffer - pointer to the epics buffer 
  // outbuffer    - Copy the data to the outbuffer as floats
  // nElem        - variable to hold the no of elements in the array for the channel
  
  // For ease of histogramming, all numbers get converted to floats and put into the outbuff 
  // But also, the address of the actual data array for the channel is saved in case needed.
  // If a string variable is requested (can't see the point), the address of the string is returned and nothing copied to outbuf.
  // 
  Int_t n=0;
  void *databuffer=NULL;                       //pointer to the channel data
  Char_t *ptr = NULL;
  EpicsHeaderInfo_t *bhead;                    //buffer header
  EpicsChannelInfo_t *chead;                   //channel header

  bhead=(EpicsHeaderInfo_t*)epics_buffer;      //buffer header

  ptr = epics_buffer + sizeof(EpicsHeaderInfo_t); //point to the start of the channel data
    
  //loop over all channels and look for PVName
  for(n=0;n<=bhead->nchan;n++){
    chead=(EpicsChannelInfo_t*)ptr;              //point to the channel header
    if(strcmp(PVName,chead->pvname )==0) break;
    ptr+=chead->bytes;                           //set pointer to start of next channel
  }
  if(n>=bhead->nchan){                           //failed to find the channel
    fprintf(stderr,"Warning, can't file EPICS channel %s in the EPICS buffer\n",PVName);
    return NULL;                                 //return a NULL pointer
  }
  
  //get the info for the channel
  *nElem = (Int_t)chead->nelem;                           //no of elements
  *type =  (Int_t)chead->type;                            //type
  databuffer = ptr + sizeof(EpicsChannelInfo_t);          //addr of data array
  
  if(*type == EepicsSTRING) return databuffer;            //for strings, don't copy data to float array
  
  ptr=(Char_t*)databuffer;                     
  
  for(n=0;n<chead->nelem;n++){                          //for all elements in data array
    switch(chead->type){                                //work out type and print formatted as appropriate
    case EepicsBYTE:
      outbuff[n]=(Float_t)(*((Char_t*)(ptr)));
      ptr+=ESizeBYTE;
      break;
    case EepicsSHORT:
      outbuff[n]=(Float_t)(*((Short_t*)(ptr)));
      ptr+=ESizeSHORT;
      break;
    case EepicsLONG:
      outbuff[n]=(Float_t)(*((Long_t*)(ptr)));
      ptr+=ESizeLONG;
      break;
    case EepicsFLOAT:
      outbuff[n]=(Float_t)(*((Float_t*)(ptr)));
      ptr+=ESizeFLOAT;
      break;
    case EepicsDOUBLE:
      outbuff[n]=(Float_t)(*((Double_t*)(ptr)));
      ptr+=ESizeDOUBLE;
      break;
    default:
      fprintf(stdout, "WARNING: Unknown epics data type: %hd for EPICS channel %s\n",*type, PVName);
      return databuffer;
      break;
    }
  }
  return databuffer;    //return pointer to the actual data array
}

//-----------------------------------------------------------------------------
TEPICSmodule::~TEPICSmodule( ){
#ifdef ACQUROOT_EPICS    //if compiled with EPICS support, clean up EPIC chann access related stuff

  if(fReadOnly) return;  //if in read only mode, nothing to clean up.

  // Clean up
  if(fTimer) {
    fTimer->Stop();
    delete fTimer;
  }
  delete fNelem;
  delete fChannelType; 
  delete fChannelAddr;
  delete fChannelData;
  for(int n=0;n<fNChannels;n++){delete fPVName[n];}
  delete fPVName;
  //delete fDataBuffer; //hmm - this delete causes a seg fault. Don't know why.  
#endif
}
ClassImp(TEPICSmodule)
