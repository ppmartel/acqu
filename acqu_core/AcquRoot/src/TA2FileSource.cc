//--Author	JRM Annand	12th Jan 2004   Convert Acqu++ -> AcquRoot
//--Rev 	JRM Annand
//--Rev 	JRM Annand      22nd Jan 2007   4v0 update
//--Update	JRM Annand       7th Jan 2013   Implement max number of buffers
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2FileSource
// Specify data stream from disk or tape file for AcquRoot analysis
//
//---------------------------------------------------------------------------

#include "TA2Control.h"
#include "TA2FileSource.h"
#include "ARFile_t.h"
#include "Semaphore_t.h"


TA2FileSource::TA2FileSource( char* name, Int_t irecl,
			      Int_t save, Int_t rsize, Int_t swap ) :
  TA2DataSource( name, save, rsize )
{
  // Save data record lengths
  fInRecLen = irecl;
  fIsSwap = swap;
  fFileName = NULL;
}

TA2FileSource::~TA2FileSource()
{
  // delete local storage
  if( fFileName ){
    for( Int_t i=0; i<fNfile; i++ ){
      if( fFileName[i] ) delete fFileName[i];
    }
    delete fFileName;
  }
}

//---------------------------------------------------------------------------
void TA2FileSource::Process()
{
  // Read data from tape or disk and write to local buffer
  // List of files to read specified in fFileName array
  
  ARFile_t* datafile;                              // file I/O handler

  PrintMessage("Starting data file read\n\n");
  // Loop round files in list
  for(Int_t ifile=0; ifile<fNfile; ifile++){
    fNrecord = 0;                                  // zero record number
    datafile = new ARFile_t(fFileName[ifile], O_RDONLY, 0, this);
    fInPath = datafile->GetPath();
    UInt_t stopRec = fStopList[ifile];
    // Continue reading buffers until file completely read or an
    // incomplete buffer read encountered
    for(;;){
      // Check if current sub-buffer is empty, if not wait until it is
      fBuffer->WaitEmpty();
      if (fInPath == -2) fNbyte = fread(fBuffer->GetStore(),1,fInRecLen,datafile->GetStart());
      else fNbyte = read(fInPath,fBuffer->GetStore(),fInRecLen);
      if( fNbyte != fInRecLen ) break;
      fNrecord++;
      if( fNrecord > stopRec || (gAR->UseDirectIO() && fNrecord >= 1) ){
	fBuffer->SetHeader(EEndBuff);
	fBuffer->FNext();
	break;
      }
      // Flag current buffer full, move to next, signal to server if waiting
      fBuffer->FNext();   
    }
    
    // Notify reading of file complete
    fprintf( fLogStream, "TA2FileSource: read %d records from file %s\n",
	     fNrecord,fFileName[ifile] );
    // close data file
    delete datafile;
  }
  while( fBuffer->IsFull() ) sleep(1);
  // ensure shutdown of Server
  UInt_t* buff = (UInt_t*)fBuffer->GetStore();
  *buff = EKillBuff;
  fBuffer->FNext();
  PrintMessage("Completed list of data files for analysis.\n\n");
}

//---------------------------------------------------------------------------
void TA2FileSource::InputList(char* name, UInt_t start, UInt_t stop)
{
  // Include file name in list of input files to analyse

  if( !fFileName ){
    fFileName = new char*[EMaxInputFiles];
    fStartList = new UInt_t[EMaxInputFiles];
    fStopList = new UInt_t[EMaxInputFiles];
    fNfile = 0;
  }
  fFileName[fNfile] = new char[strlen(name)+1];
  strcpy(fFileName[fNfile], name);
  fStartList[fNfile] = start; // this is actually not used by Process...
  if( !stop ) stop = 0xffffffff;
  fStopList[fNfile] = stop;
  fNfile++;
}

void TA2FileSource::GetFileInfo(Int_t i, std::string& filename, UInt_t& start, UInt_t& stop) {
  filename.assign(fFileName[i]);
  start = fStartList[i];
  stop = fStopList[i];
}

ClassImp(TA2FileSource)
