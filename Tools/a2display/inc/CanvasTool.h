#ifndef CANVASTOOL_H
#define CANVASTOOL_H

#include "TCanvas.h"
#include <string>
#include "Rtypes.h"

class CanvasTool: public TCanvas { 
  
private:
    Double_t fillvalue;

public:
    CanvasTool( const std::string& name="", const std::string& title=""):
        TCanvas(name.c_str(), title.c_str()), fillvalue(1.0) {}

    virtual ~CanvasTool() {}

    virtual void HandleInput(EEventType event, Int_t px, Int_t py);

    virtual void SetFillValue( const Double_t value); //*MENU*
    
    ClassDef(CanvasTool, 1)
};

#endif
