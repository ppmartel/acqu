/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOMCParticle                                                         //
//                                                                      //
// Class representing a particle in the MC simulation.                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOMCParticle
#define OSCAR_TOMCParticle

#include "TObject.h"

class TParticlePDG;
class TF1;
class TVector3;
class TLorentzVector;

class TOMCParticle : public TObject
{

private:
    TParticlePDG* fPDG;                         // PDG particle properties
    TLorentzVector* fP4;                        // 4-vector
    Bool_t fStable;                             // stable flag for decay calculation
    Int_t fNDecay;                              // number of decay particles
    TOMCParticle** fDecay;                      //[fNDecay] list of decay particles (this class is owner)
    TF1* fMassDistr;                            // particle mass distribution (e.g. for resonances) [MeV]
    TVector3* fVertex;                          // vertex of particle
    Bool_t fHasVertex;                          // flag for existence of vertex

public:
    TOMCParticle() : TObject(),
                     fPDG(0), fP4(0), fStable(kTRUE), 
                     fNDecay(0), fDecay(0), fMassDistr(0),
                     fVertex(0), fHasVertex(kFALSE) { }
    TOMCParticle(TParticlePDG* inPDG, Bool_t initDecay = kTRUE);
    TOMCParticle(const Char_t* inName, Bool_t initDecay = kTRUE);
    TOMCParticle(Int_t inID, Bool_t isGeant3_ID, Bool_t initDecay = kTRUE);
    virtual ~TOMCParticle();

    Bool_t CalculateDecay();
    void CalculateVertex(TVector3* v);
    void SetHasVertex(Bool_t b) { fHasVertex = b; }
    Bool_t HasVertex() const { return fHasVertex; }

    TOMCParticle* AddDecayParticle(const Char_t* inPartName);
    void SetGeant3_ID(Int_t inID);
    void SetP4(TLorentzVector* inP4);
    void SetP4(Double_t inPx, Double_t inPy, Double_t inPz, Double_t inE);
    void SetMassDistribution(TF1* inDistr) { fMassDistr = inDistr; }
    void SetVertex(Double_t x, Double_t y, Double_t z);

    Int_t GetPDG_ID() const;
    Int_t GetGeant3_ID() const;
    const Char_t* GetName() const;
    Double_t GetPDGMassGeV() const;
    TLorentzVector* Get4Vector() const { return fP4; }
    Int_t GetTotalNDecayParticles() const;
    Int_t GetNDecayParticles() const { return fNDecay; }
    TOMCParticle* GetDecayParticle(Int_t index) const;
    TF1* GetMassDistribution() const { return fMassDistr; }
    TVector3* GetVertex() const { return fVertex; }
    static Double_t GetLifetime(Int_t pdgID);

    void PrintDecayChain(Char_t* outString = 0, Int_t level = 0);

    ClassDef(TOMCParticle, 0)  // MC particle class
};

#endif

