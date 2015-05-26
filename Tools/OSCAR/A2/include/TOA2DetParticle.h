/*************************************************************************
 * Author: Dominik Werthmueller, 2008-2014
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TOA2DetParticle                                                      //
//                                                                      //
// Class for reconstructed particles in the A2 setup.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef OSCAR_TOA2DetParticle
#define OSCAR_TOA2DetParticle

#include "TOA2BaseDetParticle.h"

class TOA2DetParticle : public TOA2BaseDetParticle
{

protected:
    UInt_t* fClusterHits;                       //[fClusterSize] Cluster hit elements 
    Double_t* fClusterHitEnergies;              //[fClusterSize] Cluster hit energies
    Double_t* fClusterHitTimes;                 //[fClusterSize] Cluster hit times
    Int_t* fClusterHitTimeMultiplicities;    //[fClusterSize] Cluster hit time multiplicities    
    Double_t fErrorTheta;                       // error in theta angle [rad]
    Double_t fErrorPhi;                         // error in phi angle [rad]
    Double_t fErrorEnergy;                      // error in energy [MeV]

public:
    TOA2DetParticle() : TOA2BaseDetParticle(),
                        fClusterHits(0), fClusterHitEnergies(0), fClusterHitTimes(0), fClusterHitTimeMultiplicities(0),
                        fErrorTheta(0), fErrorPhi(0), fErrorEnergy(0) { }
    TOA2DetParticle(const TOA2DetParticle& orig);
    TOA2DetParticle(const TOA2BaseDetParticle& orig);
    virtual ~TOA2DetParticle();
 
    Double_t CalcChi2IMError(TOA2DetParticle* p);

    virtual void Print(Option_t* option = "") const;

    void SetClusterHits(Int_t size, UInt_t* hits);
    void SetClusterHitEnergies(Int_t size, Double_t* energies);
    void SetClusterHitTimes(Int_t size, Double_t* times);
    void SetClusterHitTimeMultiplicities(Int_t size, Int_t* mults);    
    void SetErrorTheta(Double_t e) { fErrorTheta = e; }
    void SetErrorPhi(Double_t e) { fErrorPhi = e; }
    void SetErrorEnergy(Double_t e) { fErrorEnergy = e; }

    UInt_t* GetClusterHits() const { return fClusterHits; }
    Double_t* GetClusterHitEnergies() const { return fClusterHitEnergies; }
    Double_t* GetClusterHitTimes() const { return fClusterHitTimes; }
    Int_t* GetClusterHitTimeMultiplicities() const { return fClusterHitTimeMultiplicities; }    
    Double_t GetErrorTheta() const { return fErrorTheta; }
    Double_t GetErrorPhi() const { return fErrorPhi; }
    Double_t GetErrorEnergy() const { return fErrorEnergy; }

    ClassDef(TOA2DetParticle, 1)  // Detected particles in the A2 setup
};

#endif

