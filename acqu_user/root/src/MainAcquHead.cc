
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>

#include <TAcquRoot.h>
#include <ARFile_t.h>

using namespace std;

// copy&paste from Acqu source files...

struct header_mk1_t {	       	// 1st part of header buffer
  Char_t fTime[26];		// run start time (ascii)
  Char_t fDescription[133];	// description of experiment
  Char_t fRunNote[133];		// particular run note
  Char_t fOutFile[40];		// output file
  UShort_t fRun;		// run number
  UShort_t fNslave;		// no. of slave VME crates
  UShort_t fNmodule;		// total no. modules
  UShort_t fNvme;	       	// no. VME modules
  UShort_t fNcamac;		// no. CAMAC modules
  UShort_t fNfastb;		// no. FASTBUS modules
  UShort_t fNspect;		// no. ADC's read out
  UShort_t fNscaler;		// no. scalers readout
  UShort_t fCIrq;	       	// no. CAMAC ADC module readouts
  UShort_t fCIrqS;		// no. CAMAC scaler module readouts 
  UShort_t fFIrq;	       	// no. FASTBUS ADC module readouts
  UShort_t fFIrqS;		// no. FASTBUS scaler module readouts
  UShort_t fRecLen;		// maximum buffer length = record len
};

struct header_mk2_t {	       	      // 1st part of header buffer
  UInt_t fMk2;                        // Mark as Mk2 data
  Char_t fTime[EMk2SizeTime];	      // run start time (ascii)
  Char_t fDescription[EMk2SizeDesc];  // description of experiment
  Char_t fRunNote[EMk2SizeComment];   // particular run note
  Char_t fOutFile[EMk2SizeFName];     // output file
  Int_t fRun;		              // run number
  Int_t fNModule;		      // total no. modules
  Int_t fNADCModule;	       	      // no. ADC modules
  Int_t fNScalerModule;	              // no. scaler modules
  Int_t fNADC;		              // no. ADC's read out
  Int_t fNScaler;	    	      // no. scalers readout
  Int_t fRecLen;		      // maximum buffer length = record len
};

// our personal data container of interesting header info
struct header_info_t {
  string Time;
  string Description;
  string RunNote;
  string OutFile;
  Int_t  RunNumber;
  Bool_t IsMaybeMk2; // check for the Mk2 marker
  Bool_t IsSane;     // check if fields have meaningful contents
}; 


string sanitize(const char* in) {
  string s(in);
  const string whitespace = " \t\f\v\n\r";
  int start = s.find_first_not_of(whitespace);
  int end = s.find_last_not_of(whitespace);
  if(start == -1 && end == -1)
    return string("");
  s.erase(0,start);
  s.erase((end - start) + 1);
  s.erase(remove(s.begin(), s.end(), '\n'), s.end());
  return s;
}

UInt_t* ReadFileBuffer(const string& filename) {
  
  // this is really ugly, but using ARFile_t way we can read 
  // also compressed files...
  
  const UInt_t recLen = 524288;
  TAcquRoot* ar = new TAcquRoot("AcquRoot", kTRUE);
  ar->SetLogFile("/dev/null"); // no log file creation...
  
  ARFile_t* datafile = new ARFile_t(filename.c_str(), O_RDONLY, 0, ar);
  char* filebuffer = new char[recLen];  
  
  UInt_t bytes = 0;
  if(datafile->GetPath() == -2) {
    bytes = fread(filebuffer,1,recLen,datafile->GetStart());
  }
  else {
    bytes = read(datafile->GetPath(),filebuffer,recLen);
  }
 
  if(bytes != recLen) {
    cerr << "Error: Cannot read " << recLen << " bytes from file: " << 
            strerror(errno) << endl; 
    exit(EXIT_FAILURE);
  }

  // we really don't care about memory management...AcquRoot does neither...
  
  return (UInt_t*)filebuffer + 1;  // advance to the beginning of the header struct already
}

header_info_t GetHeaderInfo_Mk1(UInt_t* filebuffer) {
  header_mk1_t* h = (header_mk1_t*)filebuffer;
  header_info_t info;
  info.Time = sanitize(h->fTime);
  info.Description = sanitize(h->fDescription);
  info.RunNote = sanitize(h->fRunNote);
  info.OutFile = sanitize(h->fOutFile);
  info.RunNumber = h->fRun;
  info.IsMaybeMk2 =  *filebuffer == EHeadBuff;
  info.IsSane  = true;
  // check a few conditions for saneness
  info.IsSane &= info.Time.length() == 24;
  info.IsSane &= info.OutFile.length() != 0;
  info.IsSane &= info.RunNumber >= 0; 
  return info;
}

header_info_t GetHeaderInfo_Mk2(UInt_t* filebuffer) {
  header_mk2_t* h = (header_mk2_t*)filebuffer;
  header_info_t info;
  info.Time = sanitize(h->fTime);
  info.Description = sanitize(h->fDescription);
  info.RunNote = sanitize(h->fRunNote);
  info.OutFile = sanitize(h->fOutFile);
  info.RunNumber = h->fRun;
  info.IsMaybeMk2 =  *filebuffer == EHeadBuff;
  info.IsSane  = true;
  // check a few conditions for saneness
  info.IsSane &= info.Time.length() == 24;
  info.IsSane &= info.OutFile.length() != 0;
  info.IsSane &= info.RunNumber >= 0; 
  return info;
}

void PrintHeaderInfo(const header_info_t& info) {
  cout << "Time: "        << info.Time  << endl;
  cout << "Description: " << info.Description << endl;
  cout << "RunNote: "     << info.RunNote << endl;
  cout << "OutFile: "     << info.OutFile << endl;
  cout << "RunNumber: "   << info.RunNumber  << endl;
  cout << "Mk2?: "         << info.IsMaybeMk2 << endl;
  cout << "Sane?: "        << info.IsSane << endl;
}

int main(int argc, char **argv)
{
  if(argc != 2) {
    cerr << "Please provide input file as argument" << endl;
    exit(EXIT_FAILURE);
  }
  string filename(argv[1]);
  // ensure the file is readable, otherwise the fake 
  // AcquRoot will exit...
  ifstream file(filename.c_str());
  if(!file.is_open()) {
    cerr << "Error: Cannot open file '" << filename << "': " << strerror(errno) << endl; 
    exit(EXIT_FAILURE);
  }
  cout << "# Working on " << filename << endl;
  
  UInt_t* filebuffer = ReadFileBuffer(filename);
  
  // heurestically guess Mk1 or Mk2
  header_info_t header_mk1 = GetHeaderInfo_Mk1(filebuffer);
  header_info_t header_mk2 = GetHeaderInfo_Mk2(filebuffer);
  
  
  if(header_mk1.IsSane && 
     !header_mk1.IsMaybeMk2 && // header_mk2.IsMaybeMk2 is identical...
     !header_mk2.IsSane) {
    // sane mk1, insane mk2 => clear mk1 signature
    PrintHeaderInfo(header_mk1);
  }
  else if(!header_mk1.IsSane && 
          header_mk1.IsMaybeMk2 && // header_mk2.IsMaybeMk2 is identical...
          header_mk2.IsSane)  {
    // insane mk1, sane mk2 => clear mk2 signature
    PrintHeaderInfo(header_mk2);
  }
  else {
    cerr << "Error: No clear Mk1/Mk2 signature found." << endl;
    cout << "===>> Printing Mk1 header info: " << endl;
    PrintHeaderInfo(header_mk1);
    cout << "===>> Printing Mk2 header info: " << endl;
    PrintHeaderInfo(header_mk2);    
    exit(EXIT_FAILURE);
  }
}

