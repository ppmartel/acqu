// Example for the self-updating PollingHistogram class.
//

void detector_display() 
{

    int loaded = gSystem->Load("/home/a2cb/a2display/build/liba2display.so");
    if(loaded != 0) {
        cerr << "Could not load liba2display.so\nRun from inside example folder or add extend your LD_LIBRARY_PATH environment variable to include it's location." << endl;
    }

    TCanvas* canvas = new TCanvas("c_hits","Detector Hits");
    canvas->Divide(2,1);

    PollingHistogram<TH2CB>* cb = new PollingHistogram<TH2CB>("NaI_Hits","NaI Hits","Self Updating from NaI_Hits every second. Click Canvas to see changes.");
   cb->EnableAutoUpdate(1000);
   canvas->cd(1);
   cb->Draw("colz");
   
   PollingHistogram<TH2TAPS>* taps = new PollingHistogram<TH2TAPS>("BaF2PWO_Hits","BaF_{2}+PbWO_{4} Hits","Self Updating from BaF2PWO_Hits every second. Click Canvas to see changes.");
   taps->EnableAutoUpdate(1000);
   canvas->cd(2);
   taps->Draw("colz");  

    //Minor problem: The user has to click the canvas to see changes. The Canvas does not know that the histogram has changed.

}
