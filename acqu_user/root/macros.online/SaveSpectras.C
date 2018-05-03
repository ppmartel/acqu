void SaveOpenSpectras() {
	char *spectra_names[] =
		{"CB-Spectra",
		 "TAPS-BaF2-Spectra",
		 "TAPS-PbWO4-Spectra",
		 "TAPS-Veto-Spectra",
		 "Tagger-Spectra",
		 "EPT-Spectra",
		 "PID-Spectra",
		 "MWPC-Spectra",
		 "Scaler-Spectra", // aka trigger spectra...
		 "Livetimes-Spectra",
		 "Physics-Spectra",
		 0};

	// clean up directory...
	printf("\nDelete all PNG images on daq-master:/home/a2cb/OnlineSpectra\n");
	stringstream CmdStr;
	CmdStr << "rm /home/a2cb/acqu/OnlineSpectra/*.png";
	system(CmdStr.str().c_str());

	// prepare elog command, note the blank space at every line!
	stringstream elog_cmd;
	elog_cmd << "echo Run " << gAR->GetRunNumber() << " Online Spectra | ";
	elog_cmd << "elog -h elog.office.a2.kph -u a2online a2messung ";
	elog_cmd << "-l 'Main Logbook 2018' -a Experiment='2018-05_Pi0_TFF' ";
	elog_cmd << "-a Author='PLEASE FILL IN' -a Type=Routine ";
	elog_cmd << "-a Subject='Online Spectra Run " << gAR->GetRunNumber() << "' ";
	
	// Save Spectra
	UInt_t i = 0;
	char spectra_title[50];
	char spectra_filename[100];
	
	while (spectra_names[i]){
		sprintf(spectra_title, "%s", spectra_names[i]);
		sprintf(spectra_filename, "/home/a2cb/acqu/OnlineSpectra/%02i_%s.png", i, spectra_names[i]);
		gSystem->ProcessEvents();

		if (gROOT->FindObject(spectra_title)) {

			TImage *img = TImage::Create();
			img->FromPad((TVirtualPad*)gROOT->FindObject(spectra_title));
			img->WriteImage(spectra_filename);
			delete img;
			elog_cmd << "-f " << spectra_filename << " ";
			printf("Spectra saved: %s\n", spectra_title);
		} else {
			printf("Spectra not found: %s\n", spectra_title);
		}
		i++;
	}


	printf("\nPosting all PNG images to the Elog\n");
        cout << elog_cmd.str() << endl;
	system(elog_cmd.str().c_str());
	printf("\nPLEASE EDIT THE ELOG ENTRY TO MAKE IT COMPLETE!\n\n");
}
