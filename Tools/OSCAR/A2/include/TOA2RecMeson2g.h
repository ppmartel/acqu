/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2RecMeson2g                                                       //
//                                                                      //
// Class for the reconstruction of mesons in the A2 setup using a two   //
// photon decay.                                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2RecMeson2g
#define OSCAR_TOA2RecMeson2g

#include "TOA2RecParticle.h"

class TOA2RecMeson2g : public TOA2RecParticle
{

private:
    TOA2DetParticle* fNeutronCand;               //-> neutron candidate
    Double_t fMass;                              // mass of reconstructed meson

    void FindBestMeson(Int_t n, TOA2DetParticle** list, Int_t* id_1, Int_t* id_2);

public:
    TOA2RecMeson2g() : TOA2RecParticle(), 
                       fNeutronCand(0), fMass(0) { }
    TOA2RecMeson2g(Int_t nPart, Double_t mass);
    virtual ~TOA2RecMeson2g();
    
    TOA2DetParticle* GetNeutronCandidate() const { return fNeutronCand; }
    virtual Bool_t Reconstruct(Int_t nPart, TOA2DetParticle** partList);
    virtual void Reset();

    ClassDef(TOA2RecMeson2g, 1)  // Reconstructed mesons->2g in the A2 setup
};

#endif

