// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCFileManager                                                        //
//                                                                      //
// Histogram building class.                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCFileManager.h"



//______________________________________________________________________________
TCFileManager::TCFileManager(const Char_t* data, const Char_t* calibration, 
                             Int_t nSet, Int_t* set, const Char_t* filePat)
{
    // Constructor using the calibration data 'data', the calibration identifier
    // 'calibration' and the 'nSet' sets in the array 'set'.
    // If filePat is non-zero use this file pattern instead of the one written
    // in the configuration file.

    // init members
    fCalibData = data;
    fCalibration = calibration;
    fNset = nSet;
    fSet = new Int_t[fNset];
    for (Int_t i = 0; i < fNset; i++) fSet[i] = set[i];
    fFiles = new TList();
    fFiles->SetOwner(kTRUE);

    // read input file pattern
    if (filePat) fInputFilePatt = filePat;
    else
    {
        if (TString* f = TCReadConfig::GetReader()->GetConfig("File.Input.Rootfiles"))
        {
            fInputFilePatt = *f;

            // check file pattern
            if (!fInputFilePatt.Contains("RUN"))
            {
                Error("TCFileManager", "Error in file pattern configuration!");
                return;
            }
        }
        else
        {
            Error("TCFileManager", "Could not load input file pattern from configuration!");
            return;
        }
    }

    // build the list of files
    BuildFileList();
}

//______________________________________________________________________________
TCFileManager::~TCFileManager()
{
    // Destructor.

    if (fFiles) delete fFiles;
    if (fSet) delete [] fSet;
}

//______________________________________________________________________________
void TCFileManager::BuildFileList()
{
    // Build the list of files belonging to the runsets.

    // check if MC files should be read
    if (TCReadConfig::GetReader()->GetConfigInt("File.Input.MC"))
    {
        const char* path = TCReadConfig::GetReader()->GetConfig("File.MC.Directory")->Data();
        const int run_number = TCReadConfig::GetReader()->GetConfigInt("File.MC.RunNumber");
        const char ext[8] = ".root";
        std::list<std::string> files;

        list_files(path, files);
        if (files.empty()) {
            fprintf(stderr, "Directory '%s' doesn't contain any files!\n", path);
            exit(EXIT_FAILURE);
        }

        // filter the list for root files
        filter_list(files, ext);
        if (files.empty()) {
            fprintf(stderr, "Directory '%s' doesn't contain any %s-files!\n", ext, path);
            exit(EXIT_FAILURE);
        }

        for (std::list<std::string>::iterator it = files.begin(); it != files.end(); ++it)
            printf("%s\n", *it);

        exit(EXIT_SUCCESS);

        // user information
        Info("BuildFileList", "Adding MC files");

        // loop over all collected MC files
        unsigned int i = 0;  // file counter for terminal output
        for (std::list<std::string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            // open the file
            TFile* f = TFile::Open((*it).c_str());

            // check for nonexisting or bad file
            if (!f || f->IsZombie())
            {
                Warning("BuildFileList", "Could not open file '%s'", *it);
                continue;
            }

            // add good file to list
            fFiles->Add(f);

            // user information
            Info("BuildFileList", "%03d : added file '%s'", ++i, f->GetName());
        }
    }
    else  // else loop over run sets
    {
        for (Int_t i = 0; i < fNset; i++)
        {
            // get the list of runs for this set
            Int_t nRun;
            Int_t* runs = TCMySQLManager::GetManager()->GetRunsOfSet(fCalibData.Data(), fCalibration.Data(), fSet[i], &nRun);

            // user information
            Info("BuildFileList", "Adding runs of set %d", fSet[i]);

            // loop over runs
            for (Int_t j = 0; j < nRun; j++)
            {
                // construct file name
                TString filename(fInputFilePatt);
                filename.ReplaceAll("RUN", TString::Format("%d", runs[j]));

                // open the file
                TFile* f = TFile::Open(filename.Data());

                // check nonexisting file
                if (!f)
                {
                    Warning("BuildFileList", "Could not open file '%s'", filename.Data());
                    continue;
                }

                // check bad file
                if (f->IsZombie())
                {
                    Warning("BuildFileList", "Could not open file '%s'", filename.Data());
                    continue;
                }

                // add good file to list
                fFiles->Add(f);

                // user information
                Info("BuildFileList", "%03d : added file '%s'", j, f->GetName());
            }

            // clean-up
            delete runs;
        }
    }
}

//______________________________________________________________________________
TH1* TCFileManager::GetHistogram(const Char_t* name)
{
    // Get the summed-up histogram with name 'name'.
    // NOTE: the histogram has to be destroyed by the caller.

    TH1* hOut = 0;

    // check if there are some runs
    if (!fFiles->GetSize())
    {
        Error("GetHistogram", "ROOT file list is empty!");
        return 0;
    }

    // do not keep histograms in memory
    TH1::AddDirectory(kFALSE);

    // loop over files
    TIter next(fFiles);
    TFile* f;
    Bool_t first = kTRUE;
    while ((f = (TFile*)next()))
    {
        // get histogram
        TH1* h = (TH1*) f->Get(name);

        // check if histogram is there
        if (h)
        {
            // correct destroying
            h->ResetBit(kMustCleanup);

            // check if object is really a histogram
            if (h->InheritsFrom("TH1"))
            {
                // check if it is the first one
                if (first)
                {
                    hOut = (TH1*) h->Clone();
                    first = kFALSE;
                }
                else hOut->Add(h);
            }
            else
            {
                Error("GetHistogram", "Object '%s' found in file '%s' is not a histogram!",
                                      name, f->GetName());
            }

            // clean-up
            delete h;
        }
        else
        {
            Warning("GetHistogram", "Histogram '%s' was not found in file '%s'",
                                    name, f->GetName());
        }
    } // loop over files

    return hOut;
}

//______________________________________________________________________________

/**
 * The following methods are used to recursively read in root files
 * for a given directory in the case of MC files
 */

// join to path segements to a full path
const char* TCFileManager::join_path(const char* path1,
                                     const char* path2,
                                     const char* path_sep = "/")
{
	char* path = new char[128];
	strcpy(path, path1);
	strcat(path, path_sep);
	strcat(path, path2);

	return path;
}

// recursively get all files from a given directory path
void TCFileManager::list_files(const char* path, std::list<std::string>& file_list)
{
	DIR *dir;
	struct dirent *ent;

	if (dir = opendir(path)) {
		while (ent = readdir(dir))  // loop over dir as long as ent gets a pointer assigned
			if (ent->d_type == DT_DIR)  // is directory?
				if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
					continue;  // skip . and .. dirs
				else
					list_files(join_path(path, ent->d_name), file_list);
			else
				file_list.push_back(join_path(path, ent->d_name));

		closedir(dir);
	} else {
		fprintf(stderr, "The directory '%s' could not be read!\n", std::string(ent->d_name));
		exit(EXIT_FAILURE);
	}

	return;
}

// remove all entries from the list which don't contain the given pattern
void TCFileManager::filter_list(std::list<std::string>& list, const char* pattern)
{
	std::list<std::string>::iterator it = list.begin();
	while (it != list.end())
		if (!strstr((*it).c_str(), pattern))
			it = list.erase(it);
		else
			it++;
}

ClassImp(TCFileManager)
