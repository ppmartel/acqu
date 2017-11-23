#include "CanvasTool.h"
#include "TH2.h"

using namespace std;

void CanvasTool::HandleInput(EEventType event, Int_t px, Int_t py)
{
    if( event == kButton1Down || event == kButton1Shift ) {

        TH2* h2 = dynamic_cast<TH2*>(fClickSelected);
        if(h2) {
            Double_t x,y;
            AbsPixeltoXY(px,py,x,y);

            if( event == kButton1Shift )
                h2->Fill(x, y, -fillvalue);
            else
                h2->Fill(x, y, fillvalue);

            Modified();
            Update();
        }
    }

    TCanvas::HandleInput( event, px, py);
}

void CanvasTool::SetFillValue(const Double_t value)
{
    fillvalue = value;
}
