void CheckSystem() {
	// ensure that CheckGUI is compiled
	gROOT->ProcessLine(".x CheckGUI.C+");

        //gROOT->ProcessLine(".x extra_check_EPT-10-2014.C");
        gROOT->ProcessLine(".x extra_check_G-E-05-2015.C");
}
