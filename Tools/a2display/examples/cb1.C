void cb1() 
{

    int loaded = gSystem->Load("../build/liba2display.so");
    if(loaded != 0) {
        cerr << "Could not load liba2display.so\nRun from inside example folder." << endl;
    }

    TH2CB* cb = new TH2CB("cb", "Crystal Ball");

    cb->FillElementNumbers();

    cb->Draw("text col");
}
