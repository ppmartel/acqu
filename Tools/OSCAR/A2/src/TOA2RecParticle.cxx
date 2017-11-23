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


#include "TClass.h"
#include "TLorentzVector.h"
#include "TH1.h"
#include "TDatabasePDG.h"

#include "TOA2RecParticle.h"
#include "TOA2DetParticle.h"
#include "TOGlobals.h"


//______________________________________________________________________________
TOA2RecParticle::TOA2RecParticle()
    : TObject()
{
    // Empty constructor.
    
    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();
    
    // init members
    fP4 = 0;
    fNDetectedProducts = 0;
    fDetectedProducts = 0;
    fChiSquare = 0;
}

//______________________________________________________________________________
TOA2RecParticle::TOA2RecParticle(Int_t nPart)
    : TObject()
{
    // Constructor using 'nPart' particles as the number of detected products.
    
    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();
 
    // init members
    fP4 = new TLorentzVector();
    fNDetectedProducts = nPart;
    fDetectedProducts = new TOA2DetParticle*[fNDetectedProducts];
    for (Int_t i = 0; i < fNDetectedProducts; i++) fDetectedProducts[i] = 0;
    fChiSquare = 0;
}

//______________________________________________________________________________
TOA2RecParticle::TOA2RecParticle(const TOA2RecParticle& orig)
    : TObject(orig)
{
    // Copy constructor.
    
    // do not save bit mask and unique ID
    Class()->IgnoreTObjectStreamer();
 
    // init members
    fP4 = new TLorentzVector(*orig.fP4);
    fNDetectedProducts = orig.fNDetectedProducts;
    fDetectedProducts = new TOA2DetParticle*[fNDetectedProducts];
    for (Int_t i = 0; i < fNDetectedProducts; i++) 
    {
        if (orig.fDetectedProducts[i]) fDetectedProducts[i] = new TOA2DetParticle(*orig.fDetectedProducts[i]);
        else fDetectedProducts[i] = 0;
    }
    fChiSquare = orig.fChiSquare;
}

//______________________________________________________________________________
TOA2RecParticle::~TOA2RecParticle()
{
    // Destructor.
 
    if (fP4) delete fP4;
    if (fDetectedProducts) 
    {
        for (Int_t i = 0; i < fNDetectedProducts; i++) 
            if (fDetectedProducts[i]) delete fDetectedProducts[i];
        delete [] fDetectedProducts;
    }
}

//______________________________________________________________________________
Bool_t TOA2RecParticle::Reconstruct(Int_t nPart, TOA2DetParticle** partList)
{
    // Basic reconstruction of the particle from its detected decay products.
    // Get the mass of the detected decay products, calculate the 4-vectors
    // and combine them to this particle's 4-vector.
    //
    // Return kTRUE if the reconstruction was successful.
    
    // check the number of particles
    if (nPart != fNDetectedProducts)
    {
        Error("Reconstruct", "Number of particles has to be equal to %d!", fNDetectedProducts);
        return kFALSE;
    }
    else
    {
        // add particles as photons
        for (Int_t i = 0; i < fNDetectedProducts; i++) 
        {
            fDetectedProducts[i] = new TOA2DetParticle(*partList[i]);
            fDetectedProducts[i]->SetPDG_ID(kGamma);
        }

        // calculate the 4-vector
        Calculate4Vector();

        return kTRUE;
    }
}

//______________________________________________________________________________
void TOA2RecParticle::Calculate4Vector()
{
    // Basic method to calculate the 4-vector of the reconstructed particle
    // from the 4-vectors of the detected products.
    
    // clear particle 4-vector
    fP4->SetPxPyPzE(0., 0., 0., 0.);

    // loop over decay products
    for (Int_t i = 0; i < fNDetectedProducts; i++)
    {
        // determine mass of decay product
        Double_t mass;
        TParticlePDG* pdg_pa = TOGlobals::GetPDG_DB()->GetParticle(fDetectedProducts[i]->GetPDG_ID());
        
        // check if particle is in database
        if (pdg_pa) mass = pdg_pa->Mass();
        else
        {
            mass = 0.;
            Error("Calculate4Vector", "Particle with code %d was not found in the PDG database!", 
                  (Int_t)fDetectedProducts[i]->GetPDG_ID());
        }
        
        // build 4-vector of decay product
        TLorentzVector p4;
        fDetectedProducts[i]->Calculate4Vector(&p4, mass);

        // add 4-vector to total 4-vector
        *fP4 += p4;
    }
}

//______________________________________________________________________________
void TOA2RecParticle::CalculateConstrained4Vector(Double_t mass, TLorentzVector* outP4)
{
    // Calculate the 4-vector constrained to the mass 'mass' and save it in 'outP4.
    // If the particle was not reconstructed yet 0 is returned.
    
    if (!fP4->M()) 
    {
        Error("CalculateConstrained4Vector", "Cannot constrain 4-vector because particle mass is zero!");
        return;
    }

    Double_t scale = mass / fP4->M();
    outP4->SetPxPyPzE(fP4->Px()*scale, fP4->Py()*scale, fP4->Pz()*scale, fP4->E()*scale);
}

//______________________________________________________________________________
Double_t TOA2RecParticle::GetAverageTime(A2Detector_t detector) const
{
    // Return the average time of the decay products detected in the A2 detector
    // 'detector'. 
    //
    // If no decay products are set or no decay products are found int the 
    // specified detector 9999 is returned.

    Double_t avr_time = 0.;
    Int_t n = 0;

    // loop over decay products
    for (Int_t i = 0; i < fNDetectedProducts; i++)
    {
        // check detector
        if (fDetectedProducts[i]->GetDetector() == detector) 
        {
            avr_time += fDetectedProducts[i]->GetTime();
            n++;
        }
    }

    if (!n) return 9999;
    else return avr_time / (Double_t) n;
}

//______________________________________________________________________________
Int_t TOA2RecParticle::GetNDetectorHits(A2Detector_t detector) const
{
    // Return the number of detected decay products found in the detector 'detector'.

    Int_t n = 0;

    for (Int_t i = 0; i < fNDetectedProducts; i++)
    {
        if (fDetectedProducts[i]->GetDetector() == detector) n++;
    }
    
    return n;
}

//______________________________________________________________________________
Bool_t TOA2RecParticle::CheckCoincidence(Double_t limitLowCB, Double_t limitHighCB, TH1* hCB,
                                         Double_t limitLowTAPS, Double_t limitHighTAPS, TH1* hTAPS,
                                         Double_t limitLowCBTAPS, Double_t limitHighCBTAPS, TH1* hCBTAPS)
{
    // Check the coincidence of all detected particles. Use
    // - 'limitLowCB' / 'limitHighCB' as limits for coincident particles in CB
    // - 'limitLowTAPS' / 'limitHighTAPS' as limits for coincident particles in TAPS
    // - 'limitLowCBTAPS' / 'limitHighCBTAPS' as limits for the difference of the average 
    // times of particles in CB and TAPS, respectively.
    // If 'hCB', 'hTAPS' or 'hCBTAPS' are provided the time differences will be filled into these histograms.

    Bool_t coincOk = kTRUE;

    // check CB coincidence
    if (!CheckDetectorCoincidence(kCBDetector, limitLowCB, limitHighCB, hCB)) coincOk = kFALSE;

    // check TAPS coincidence
    if (!CheckDetectorCoincidence(kTAPSDetector, limitLowTAPS, limitHighTAPS, hTAPS)) coincOk = kFALSE;

    // check CB-TAPS coincidence if there is a hit in TAPS
    Int_t nTAPSHits = GetNDetectorHits(kTAPSDetector);
    if (nTAPSHits)
    {
        Double_t avrCBTime = GetAverageTime(kCBDetector);
        Double_t avrTAPSTime = GetAverageTime(kTAPSDetector);

        // the time values have to be added to remove the trigger time
        Double_t avrCBTAPSTime = avrCBTime + avrTAPSTime;
        hCBTAPS->Fill(avrCBTAPSTime);
        
        if (avrCBTAPSTime < limitLowCBTAPS || avrCBTAPSTime > limitHighCBTAPS) coincOk = kFALSE;
    }

    return coincOk;
}

//______________________________________________________________________________
Bool_t TOA2RecParticle::CheckDetectorCoincidence(A2Detector_t detector, Double_t limitLow, Double_t limitHigh, TH1* h)
{
    // Check if all combinations of particles detected in the detector 'detector' fulfill the coincidence
    // condition, i.e. their time difference lies between 'limitLow' and 'limitHigh'.
    // If 'h' is provided the time differences will be filled into this histogram.
    // 
    // Note: 
    // If no particles are found in the specified detector kTRUE is returned.

    Bool_t coincOk = kTRUE;

    // detected particle loop 1
    for (Int_t i = 0; i < fNDetectedProducts; i++)
    {
        // check the detector
        if (fDetectedProducts[i]->GetDetector() != detector) continue;

        // detected particle loop 2
        for (Int_t j = i+1; j < fNDetectedProducts; j++)
        {
            // check the detector
            if (fDetectedProducts[j]->GetDetector() != detector) continue;
    
            // calculate time difference
            Double_t timeDiff = fDetectedProducts[i]->GetTime() - fDetectedProducts[j]->GetTime();

            // fill histogram
            if (h) h->Fill(timeDiff);

            // check coincidence
            if (timeDiff < limitLow || timeDiff > limitHigh) coincOk = kFALSE;
        }
    }

    return coincOk;
}

//______________________________________________________________________________
Double_t TOA2RecParticle::GetTime() const
{
    // Return the time of the particle that is calculated by averaging the
    // times of the detected products. If there are products detected in TAPS
    // this time will be preferred because of its better resolution.
    
    // check if there are particles in TAPS
    if (GetNDetectorHits(kTAPSDetector)) return GetAverageTime(kTAPSDetector);
    else return GetAverageTime(kCBDetector);
}

//______________________________________________________________________________
TOA2DetParticle* TOA2RecParticle::GetDetectedProduct(Int_t index) const
{
    // Return the decay product with index 'index'. Return 0 if there is no
    // product with this index in the products list.

    // check index
    if (index < 0 || index >= fNDetectedProducts) 
    {
        Error("GetDetectedProduct", "Index %d out of bounds (size: %d)", index, fNDetectedProducts);
        return 0;
    }

    // return particle
    return fDetectedProducts[index];
}

//______________________________________________________________________________
void TOA2RecParticle::Reset()
{
    // Reset the content for a new reconstruction.

    // reset class members
    fP4->SetPxPyPzE(0, 0, 0, 0);
    for (Int_t i = 0; i < fNDetectedProducts; i++) 
    {
        if (fDetectedProducts)
        {
            if (fDetectedProducts[i]) 
            {
                delete fDetectedProducts[i];
                fDetectedProducts[i] = 0;
            }
        }
    }
    fChiSquare = 0;
}

//______________________________________________________________________________
void TOA2RecParticle::Print(Option_t* option) const
{
    // Print out the content of this class.

    printf("TOA2RecParticle content:\n");
    printf("4-vector               : %f %f %f %f %f\n", fP4->Px(), fP4->Py(),fP4->Pz(),fP4->E(), fP4->M());
    printf("chi square (reconstr.) : %f\n", fChiSquare);
    printf("detected products      : %d\n", fNDetectedProducts);
    for (Int_t i = 0; i < fNDetectedProducts; i++) 
    {
        printf("\n");
        printf("product %d\n", i+1);
        if (fDetectedProducts[i]) fDetectedProducts[i]->Print();
        else printf("undefined\n");
    }
}

//______________________________________________________________________________
TOA2RecParticle& TOA2RecParticle::operator=(const TOA2RecParticle& p)
{
    // Assignment operator.

    // check self assignment
    if (this != &p)
    {
        *fP4 = *p.fP4;
        fChiSquare = p.fChiSquare;
        
        // delete current particles
        for (Int_t i = 0; i < fNDetectedProducts; i++) 
            if (fDetectedProducts[i]) delete fDetectedProducts[i];
        delete [] fDetectedProducts;

        // set new particles
        fNDetectedProducts = p.fNDetectedProducts;
        fDetectedProducts = new TOA2DetParticle*[fNDetectedProducts];
        for (Int_t i = 0; i < fNDetectedProducts; i++) 
        {
            if (p.fDetectedProducts[i]) fDetectedProducts[i] = new TOA2DetParticle(*p.fDetectedProducts[i]);
            else fDetectedProducts[i] = 0;
        }
    }
    
    return *this;
}

//______________________________________________________________________________
TOA2RecParticle& TOA2RecParticle::operator+=(const TOA2RecParticle& p)
{
    // Compound assignment operator +=.
    
    *fP4 += *p.fP4;
    
    // backup old particle list
    TOA2DetParticle** old = fDetectedProducts;
    
    // create new particle list
    fDetectedProducts = new TOA2DetParticle*[fNDetectedProducts+p.fNDetectedProducts];
    for (Int_t i = 0; i < fNDetectedProducts; i++) fDetectedProducts[i] = old[i];
    
    // add new particles
    for (Int_t i = 0; i < p.fNDetectedProducts; i++) 
    {
        if (p.fDetectedProducts[i]) fDetectedProducts[fNDetectedProducts+i] = new TOA2DetParticle(*p.fDetectedProducts[i]);
        else fDetectedProducts[fNDetectedProducts+i] = 0;
    }

    // set new number of particles
    fNDetectedProducts += p.fNDetectedProducts;
    
    // destroy old list
    delete [] old;

    return *this;
}

//______________________________________________________________________________
const TOA2RecParticle TOA2RecParticle::operator+(const TOA2RecParticle& p) const
{
    // Binary Arithmetic Operators +.

     return TOA2RecParticle(*this) += p;
}
ClassImp(TOA2RecParticle)
