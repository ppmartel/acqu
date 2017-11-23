{

    int loaded = gSystem->Load("../build/liba2display.so");
    if(loaded != 0) {
        cerr << "Could not load liba2display.so\nRun from inside example folder." << endl;
    }


    std::vector<double> v(672);

    for(int i=0;i<v.size();++i) {
        v.at(i)=i;
    }

    TH2CB* cb = new TH2CB();
    cb->FillHitpattern672(v);
    cb->Draw("text col");
    
}
