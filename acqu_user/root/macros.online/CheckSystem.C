void CheckSystem() {
	// ensure that CheckGUI is compiled
	gROOT->ProcessLine(".x CheckGUI.C+");

        gROOT->ProcessLine(".x extra_check.C");
}
