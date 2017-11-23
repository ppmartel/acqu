// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadACQU                                                           //
//                                                                      //
// Read raw ACQU MK1 files.                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCREADACQU_H
#define TCREADACQU_H

#include "TList.h"
#include "TError.h"
#include "TSystem.h"
#include "TSystemDirectory.h"


// Mk1 format header field sizes
enum EMk1HeaderSize
{
    kMk1Marker    =   4,
    kMk1SizeTime  =  26, 
    kMk1SizeDesc  = 133, 
    kMk1SizeRNote = 133, 
    kMk1SizeFName =  40  
};


// Mk2 format header field sizes
enum EMk2HeaderSize
{ 
    kMk2Marker    =   8,
    kMk2SizeTime  =  32, 
    kMk2SizeDesc  = 256, 
    kMk2SizeRNote = 256, 
    kMk2SizeFName = 128
};


// type of raw files
enum ERawFileType
{
    kFileBad,
    kFileUnComp,
    kFileGZ,
    kFileXZ
};
typedef ERawFileType RawFileType_t;


// types of raw file formats
enum ERawFileFormat
{
    kRawUnknown,
    kRawMk1,
    kRawMk2
};
typedef ERawFileFormat RawFileFormat_t;


class TCACQUFile : public TObject
{

private:
    RawFileFormat_t fFormat;                // raw file format
    Char_t fTime[kMk2SizeTime];             // run start time (ascii)
    Char_t fDescription[kMk2SizeDesc];      // description of experiment
    Char_t fRunNote[kMk2SizeRNote];         // particular run note
    Char_t fOutFile[kMk2SizeFName];         // output file
    UShort_t fRun;                          // run number
    Long64_t fSize;                         // file size in bytes
    Char_t fFileName[256];                  // actual filename
    
    RawFileType_t CheckFileType(const Char_t* file)
    {
        Char_t header[6];
        const unsigned char hGZ[3] = { 0x1f, 0x8b, 0x08 };
        const unsigned char hXZ[6] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00 };
        
        // open the file
        FILE* f = fopen(file, "r");
        
        // check file opening
        if (f)
        {
            // read 6 bytes
            if(fread(header, 1, 6, f));
            
            // close the file
            fclose(f);
 
            // check headers
            if (!memcmp(header, hGZ, 3)) return kFileGZ;
            else if (!memcmp(header, hXZ, 6)) return kFileXZ;
            else return kFileUnComp;
        } 
        else return kFileBad;
    }

    RawFileFormat_t CheckFileFormat(const Char_t* hdr)
    {
        // Mk2
        Int_t n = 0;
        for (Int_t i = 0; i < 8; i++)
            if (hdr[i] == 0x10) n++;
        if (n == 8) return kRawMk2;

        // Mk1
        n = 0;
        for (Int_t i = 0; i < 4; i++)
            if (hdr[i] == 0x10) n++;
        if (n == 4) return kRawMk1;
        
        // other cases
        return kRawUnknown;
    }
    
    void RemoveControlChars(Char_t* string)
    {            
        TString s(string);
        s.Remove(TString::kBoth, '\n');
        s.Remove(TString::kBoth, '\t');
        s.Remove(TString::kBoth, ' ');
        strcpy(string, s.Data());
    }
 
public:
    TCACQUFile() : TObject()
    {
        fFormat = kRawUnknown;
        fTime[0] = '\0';
        fDescription[0] = '\0';
        fRunNote[0] = '\0';
        fOutFile[0] = '\0';
        fRun = 0;
        fSize = 0;
        fFileName[0] = '\0';
    }
    virtual ~TCACQUFile() { }
   
    void ReadFile(const Char_t* path, const Char_t* fname)
    {
        Char_t tmp[280];
        
        // set full file name
        Char_t filename[256];
        sprintf(filename, "%s/%s", path, fname);

        // set actual file name
        strcpy(fFileName, fname);

        // identify file type
        RawFileType_t ftype = CheckFileType(filename);
        
        // try to open the file
        FILE* file = 0;
        if (ftype == kFileUnComp) file = fopen(filename, "r");
        else if (ftype == kFileGZ)
        {
            sprintf(tmp, "zcat %s 2> /dev/null", filename);
            file = popen(tmp, "r");
        }
        else if (ftype == kFileXZ)
        {
            sprintf(tmp, "xzcat %s", filename);
            file = popen(tmp, "r");
        }

        // check if file was opened
        if (!file)
        {
            Error("Read", "Could not open '%s'", filename);
            return;
        }
        
        // read complete header
        Int_t maxHdrLength = kMk2Marker + kMk2SizeTime + kMk2SizeDesc + 
                             kMk2SizeRNote + kMk2SizeFName +
                             sizeof(UShort_t);
        Char_t header[maxHdrLength];
        if(fread(header, 1, sizeof(header), file));
        
        // close file
        if (ftype == kFileUnComp) fclose(file);
        else if (ftype == kFileGZ) pclose(file);
        else if (ftype == kFileXZ) pclose(file);
        
        // identify raw file format
        fFormat = CheckFileFormat(header);
        
        // parse header
        if (fFormat == kRawMk1 || fFormat == kRawMk2)
        {
            // configure parsing
            Int_t sMarker = fFormat == kRawMk1 ? (Int_t)kMk1Marker : (Int_t)kMk2Marker;
            Int_t sTime = fFormat == kRawMk1 ? (Int_t)kMk1SizeTime : (Int_t)kMk2SizeTime;
            Int_t sDesc = fFormat == kRawMk1 ? (Int_t)kMk1SizeDesc : (Int_t)kMk2SizeDesc;
            Int_t sRNote = fFormat == kRawMk1 ? (Int_t)kMk1SizeRNote : (Int_t)kMk2SizeRNote;
            Int_t sFName = fFormat == kRawMk1 ? (Int_t)kMk1SizeFName : (Int_t)kMk2SizeFName;
            
            // start parsing
            Char_t* pos = header;

            // skip header
            pos += sMarker;

            // read time
            memcpy(fTime, pos, sTime);
            pos += sTime;
            RemoveControlChars(fTime);
            
            // read description
            memcpy(fDescription, pos, sDesc);
            pos += sDesc;
            RemoveControlChars(fDescription);
            
            // read run note
            memcpy(fRunNote, pos, sRNote);
            pos += sRNote;
            RemoveControlChars(fRunNote);

            // read output file
            memcpy(fOutFile, pos, sFName);
            pos += sFName;
            RemoveControlChars(fOutFile);
        
            // read run number
            memcpy(&fRun, pos, sizeof(UShort_t));
        }

        // set file size
        FileStat_t fileinfo;
        gSystem->GetPathInfo(filename, fileinfo);
        fSize = fileinfo.fSize;
    }
  
    void Print()
    {
        Char_t format[16];
        if (fFormat == kRawMk1) strcpy(format, "Mk1");
        else if (fFormat == kRawMk2) strcpy(format, "Mk2");
        else strcpy(format, "unknown");

        printf("ACQU File Information\n");
        printf("Format        : %s\n", format);
        printf("Time          : %s\n", fTime);
        printf("Description   : %s\n", fDescription);
        printf("Run note      : %s\n", fRunNote);
        printf("Output file   : %s\n", fOutFile);
        printf("Run number    : %d\n", fRun);
        printf("Size in bytes : %lld\n", fSize);
        printf("File name     : %s\n", fFileName);
        printf("\n");
    }

    void PrintListing() { printf("%s\t%s\t%s\t%s\t%lld\n", fFileName, fTime, fDescription, fRunNote, fSize); }

    Bool_t IsGoodDataFile()
    {
        if (fFormat == kRawMk1) return kTRUE;
        else if (fFormat == kRawMk2) return kTRUE;
        else return kFALSE;
    }

    const Char_t* GetTime() const { return fTime; }
    const Char_t* GetDescription() const { return fDescription; }
    const Char_t* GetRunNote() const { return fRunNote; }
    const Char_t* GetOutFile() const { return fOutFile; }
    UShort_t GetRun() const { return fRun; }
    Long64_t GetSize() const { return fSize; }
    const Char_t* GetFileName() const { return fFileName; }
    
    ClassDef(TCACQUFile, 0) // ACQU file class
};


class TCReadACQU
{

private:
    Char_t* fPath;                  // path of files
    TList* fFiles;                  // list of files

    void ReadFiles();

public:
    TCReadACQU() : fPath(0), fFiles(0) { }
    TCReadACQU(const Char_t* path);
    virtual ~TCReadACQU();
    
    TList* GetFiles() const { return fFiles; }
    Int_t GetNFiles() const { return fFiles ? fFiles->GetSize() : 0; }
    TCACQUFile* GetFile(Int_t n) const { return fFiles ? (TCACQUFile*)fFiles->At(n) : 0; }

    ClassDef(TCReadACQU, 0) // ACQU raw file reader
};

#endif

