//--Author	Ken Livingston    23nd Sep 2012  Added support for EPICS
//--Rev 	
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TEPICSModule
// Basic class for reading EPICS evets


#ifndef __TEPICSmodule_h__
#define __TEPICSmodule_h__

#include "TDAQmodule.h"
class TTimer;

enum ETrigTypes{ EEPICS_TRIG_THRESH, EEPICS_TRIG_WINDOW, EEPICS_TRIG_AND, EEPICS_TRIG_OR};

class TEPICSmodule : public TDAQmodule {
 protected:
  Short_t  fEPICSID;      // EPICS event ID for the user 
  Int_t    fEpicsIndex;   // Counter for no of EPICS modules
  Char_t   fName[32];     // EPICS event Name
  Char_t  *fDataBuffer;   // Buffer to hold the epics data read in for the event
  Int_t    fBuffLen;      // Length of the data buffer
  Short_t  fNChannels;    // No of EPICS channels to be read
  Char_t **fPVName;       // Array of channel names
  Int_t  *fNelem;         // Array of no of elements for each channel
  Int_t   *fChannelType;  // Array of channel types
  void   **fChannelAddr;  // Array of the addresses of the channels info within the buffer for each channel
  void   **fChannelData;  // Array of the addresses of the data within the buffer for each channel
  Int_t    fPeriodMode;   // Timed or scaler counts
  Long_t   fPeriod;       // Period in ms or # scaler events, depending on fPeriodMode
  Long_t   fCounter;      // Count no of scaler events
  TTimer  *fTimer;        // To deal with timed periodic readout
  Bool_t   fVerbose;      
  Bool_t   fIsTimed;      // To flag if it's timed
  Bool_t   fIsTriggered;  // To flag if it's triggered
  Bool_t   fIsCounted;    // To flag if it's periodic
  Bool_t   fIsTimedOut;   // To flag when periodic timed readout is due
  Bool_t   fIsCountedOut; // To flag when periodic #scaler count readout is due
  Bool_t   fIsTriggeredOut; // To flag when periodic #scaler count readout is due
  Bool_t   fReadOnly;     // For reading epics buffers in the datastream
  Int_t    fNEpics;       // Count no of epics events
  Int_t    fNEpicsT;      // Count no of epics events timed
  Int_t    fNEpicsC;      // Count no of epics events scaler counted
  Int_t    fNEpicsTr;     // Count no of epics events triggered
  Short_t  fTrigADC;      // ADC index for trigger
  Short_t  fTrigLow;      // Lower value of trig range
  Short_t  fTrigHigh;     // Upper value of trig range
  Short_t  fTrigType;
  Char_t   fModeString[32]; // Description of the mode
  Short_t  fVal;            // value of adc which triggers an EPICS read in trig mode.
  
 public:

  TEPICSmodule(Char_t* name = NULL, Char_t * file = NULL, FILE* log = NULL, Char_t *line = NULL);  
  //Constructor for writing epics into AcuqDAQ data steam (and reading buffs if reqd)

  //functions for decoding EPICS buffer 
  virtual void DumpBuffer(Char_t *buffer = NULL, Char_t *outfile=NULL);      //Dump the epics events from a buffer
  virtual Int_t GetBufferInfo(Short_t*, Short_t *, Short_t*, Char_t *, Short_t *, Char_t *, time_t *); //Get info from and EPICS buffer header
  virtual void *GetChannel(Char_t *, Int_t *, Char_t *, Float_t *, Int_t *); //Get data for a channel in the buffer

  //functions related to writing EPICs channels intot EPICS buffers etc.
  virtual void SetConfig( Char_t*, Int_t );          // configure EPICS module
  virtual void PostInit();                           // initialisation
  virtual void WriteEPICS(void **);                  // Write the epics events from channel acces to the data buffer
  virtual Bool_t HandleTimer(TTimer *timer);         //Called for timed periodic mode to flag timed out.
  virtual void SetVerbose(Bool_t mode){fVerbose=mode;}
  virtual void Start();                              //Start counting for periodic readout
  virtual void Stop();                               //Stop timer
  virtual Bool_t IsTimedOut(){return fIsTimedOut;}   //Check if timer has run out
  virtual Bool_t IsCountedOut();                     //Check if no of scaler counts is reached
  virtual Bool_t IsTriggeredOut(Char_t* eventBuff,Int_t adcEnd);  //Check if passed trigger
  virtual Bool_t IsTimed(){return fIsTimed;}         //Check if timed periodic readout setup
  virtual Long_t Count(){return fCounter++;}         //Increment the periodic scaler counter
  virtual Bool_t IsCounted(){return fIsCounted;}     //Check if scaler counted periodic readout setup
  virtual Bool_t IsTriggered(){return fIsTriggered;} //Check if scaler counted periodic readout setup
  virtual void SetEpicsIndex(Int_t i){fEpicsIndex=i;} //To store the EPICS index of module

  virtual ~TEPICSmodule();

  ClassDef(TEPICSmodule,1)
};


inline void TEPICSmodule::Start()
{ 
  if(fPeriod){                                  //if doing periodic reads 
    if(fTimer){                                 //if a timer is set
      fTimer->Reset();                          //reset the timer
      //fTimer->Start(fPeriod, kFALSE);            //start in single shot mode
      fTimer->Start(fPeriod, kTRUE);            //start in single shot mode
      fIsTimedOut=kFALSE;                       //flag that it has not timed out
    }  
    else{                                       //if scaler event counting 
      fCounter=0;                               //zero the counter
      fIsCountedOut=kFALSE;                     //flag that it has not counted out
    }
  }
}
inline void TEPICSmodule::Stop()
{ 
  if(fPeriod){                                  //if doing periodic reads 
    if(fTimer){                                 //if a timer is set
      fTimer->Stop();                           //reset the timer
      fIsTimedOut=kFALSE;                       //flag that it has not timed out
    } 
  } 
}

inline Bool_t TEPICSmodule::IsCountedOut()
{ 
  if(fCounter < fPeriod) fIsCountedOut=kFALSE; //increment counter and return if not reached the period count
  else fIsCountedOut=kTRUE;
  return fIsCountedOut;
}

inline Bool_t TEPICSmodule::IsTriggeredOut(Char_t* eventBuff, Int_t adcEnd) 
{ 

  Int_t n=0;
  UShort_t *adc = (UShort_t*)(eventBuff);    //adc = ushort[evntH,evntL,lenH,lenL,adc1,val1,adc2,val2 ......]
  for(n=4;n<adcEnd;n+=2){                    //go from start to end of adc data
    if(adc[n]==fTrigADC) break;
  }
  if(adc[n]!= fTrigADC) return kFALSE;
  
  if((adc[n+1]==16384)||(adc[n+1]==-1)) return kFALSE;           //null adc


  fVal=adc[n+1];
  
  
  switch(fTrigType){
  case  EEPICS_TRIG_AND:
    if (fTrigHigh&&fVal) return kTRUE;
    break;
  case  EEPICS_TRIG_THRESH:
    if (fTrigLow<=fVal) return kTRUE;
    break;
  case  EEPICS_TRIG_WINDOW:
    if ((fTrigLow<=fVal)&&(fVal<=fTrigHigh)) return kTRUE;
    break;
  default:
    break;   
  }
  return kFALSE;
}

#endif
