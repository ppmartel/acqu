#include "TH2CB.h"
#include "TH2TAPS.h"
#include "TCanvas.h"

#include <vector>
#include "TRint.h"

int main(int argc, char** argv) {

    // Create the application object. Pass the command line arguments through.
    TRint app( "AppTemplate", &argc, argv);

    std::vector<double> v(720);

    for(size_t i=0;i<v.size();++i) {
        v.at(i)=i;
    }

    new TCanvas("c_cb","Crystal Ball");
    TH2CB* cb = new TH2CB("cb","Crystal Ball");
    cb->FillElements(v);
    cb->Draw("text col");

    new TCanvas("c_taps","TAPS");
    TH2TAPS* taps = new TH2TAPS("taps","TAPS");
    taps->FillElementNumbers();
    taps->Draw("text col");

    app.Run();

    return 0;
}
