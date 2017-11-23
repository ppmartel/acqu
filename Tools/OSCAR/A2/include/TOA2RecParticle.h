/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2RecParticle                                                      //
//                                                                      //
// Base class for reconstructed particles in the A2 setup.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2RecParticle
#define OSCAR_TOA2RecParticle

#include "TObject.h"

#include "TOA2Detector.h"

class TH1;
class TLorentzVector;
class TOA2DetParticle;

class TOA2RecParticle : public TObject
{

protected:
    TLorentzVector* fP4;                            //-> 4-vector
    Int_t fNDetectedProducts;                       // number of detected decay products
    TOA2DetParticle** fDetectedProducts;            //[fNDetectedProducts] pointer to detected decay product list
    Double_t fChiSquare;                            // chi square value of reconstruction

    virtual void Calculate4Vector();

public:
    TOA2RecParticle(); 
    TOA2RecParticle(Int_t nPart);
    TOA2RecParticle(const TOA2RecParticle& orig);
    virtual ~TOA2RecParticle();
    
    TLorentzVector* Get4Vector() const { return fP4; }
    Int_t GetNDetectedProducts() const { return fNDetectedProducts; }
    TOA2DetParticle** GetDetectedProducts() const { return fDetectedProducts; }
    TOA2DetParticle* GetDetectedProduct(Int_t index) const;
    Double_t GetChiSquare() const { return fChiSquare; }
 
    Double_t GetTime() const;
    Double_t GetAverageTime(A2Detector_t detector) const;
    Int_t GetNDetectorHits(A2Detector_t detector) const;
    Bool_t CheckCoincidence(Double_t limitLowCB, Double_t limitHighCB, TH1* hCB,
                            Double_t limitLowTAPS, Double_t limitHighTAPS, TH1* hTAPS,
                            Double_t limitLowCBTAPS, Double_t limitHighCBTAPS, TH1* hCBTAPS);
    Bool_t CheckDetectorCoincidence(A2Detector_t detector, Double_t limitLow, Double_t limitHigh, TH1* h = 0);
    void CalculateConstrained4Vector(Double_t mass, TLorentzVector* outP4);
    
    virtual Bool_t Reconstruct(Int_t nPart, TOA2DetParticle** partList);
    
    virtual void Reset();
    virtual void Print(Option_t* option = "") const;
    
    TOA2RecParticle& operator=(const TOA2RecParticle& p);
    TOA2RecParticle& operator+=(const TOA2RecParticle& p);
    const TOA2RecParticle operator+(const TOA2RecParticle& p) const;
     
    ClassDef(TOA2RecParticle, 1)  // Reconstructed particles in the A2 setup
};

#endif

