// Example for the self-updating PollingHistogram class.
//

void Polling() 
{

    int loaded = gSystem->Load("../build/liba2display.so");
    if(loaded != 0) {
        cerr << "Could not load liba2display.so\nRun from inside example folder." << endl;
    }

    TCanvas* canvas = new TCanvas();
    canvas->Divide(2,1);

    // data soruce histogram
    TH1D* Hits = new TH1D("NaI_Hits","NaI Hits",720,0,2);
    canvas->cd(1);
    Hits->Draw();

    // fill the data source with random data every second
    TTimer* data_maker = new TTimer("NaI_Hits->FillRandom(\"gaus\",10)",1000);
    data_maker->Start();

    // set up a polling CB histogram that automatically pulls data from the data source histogram
    PollingHistogram<TH2CB>* cb = new PollingHistogram<TH2CB>("NaI_Hits","NaI_update","Self Updating from NaI_Hits every second. Click Canvas to see changes.");
    
    canvas->cd(2);
    cb->Draw("col");
    
    cb->EnableAutoUpdate(1000);

    //Minor problem: The user has to click the canvas to see changes. The Canvas does not know that the histogram has changed.

}
