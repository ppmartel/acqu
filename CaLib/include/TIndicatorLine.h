#ifndef TINDICATORLINE_H
#define TINDICATORLINE_H

#include "TLine.h"

class TIndicatorLine: public TLine {
    ClassDef(TIndicatorLine, 1)

public:
    TIndicatorLine();
    virtual ~TIndicatorLine() {}

    // move both points the same way in x
    virtual void SetX1(Double_t x1) {fX1=x1;fX2=x1;}
    virtual void SetX2(Double_t x2) {fX2=x2;fX1=x2;}

    // ignore all y movements
    virtual void SetY1(Double_t y1) {}
    virtual void SetY2(Double_t y2) {}

    // set y positions
    virtual void SetupY(Double_t y1, Double_t y2) {fY1 = y1; fY2=y2; }

    virtual void SetHorizontal(Bool_t set) {}
    virtual void SetVertical(Bool_t set) {}

};

#endif
