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


#include "TLorentzVector.h"
#include "TRandom.h"
#include "TGenPhaseSpace.h"
#include "TF1.h"
#include "TDatabasePDG.h"

#include "TOMCParticle.h"
#include "TOGlobals.h"


//______________________________________________________________________________
TOMCParticle::TOMCParticle(TParticlePDG* inPDG, Bool_t initDecay)
    : TObject()
{
    // Constructor using the PDG particle definition 'inPDG'.
    // Set the random number generator seed if 'initDecay' is kTRUE (default).

    fPDG = inPDG;
    fP4 = new TLorentzVector(0., 0., 0., 0.);
    fStable = kTRUE;
    fNDecay = 0;
    fDecay = 0;
    fMassDistr = 0;
    fVertex = new TVector3();
    fHasVertex = kFALSE;

    // set machine clock based random seed to the standart
    // ROOT random number generator (used by TGenPhaseSpace)
    if (initDecay) gRandom->SetSeed(0);
}

//______________________________________________________________________________
TOMCParticle::TOMCParticle(const Char_t* inName, Bool_t initDecay)
    : TObject()
{
    // Constructor using the particle name 'inName'.
    // Set the random number generator seed if 'initDecay' is kTRUE (default).

    fPDG = TOGlobals::GetPDG_DB()->GetParticle(inName);
    fP4 = new TLorentzVector(0., 0., 0., 0.);
    fStable = kTRUE;
    fNDecay = 0;
    fDecay = 0;
    fMassDistr = 0;
    fVertex = new TVector3();
    fHasVertex = kFALSE;
    
    // set machine clock based random seed to the standart
    // ROOT random number generator (used by TGenPhaseSpace)
    if (initDecay) gRandom->SetSeed(0);
}

//______________________________________________________________________________
TOMCParticle::TOMCParticle(Int_t inID, Bool_t isGeant3_ID, Bool_t initDecay)
    : TObject()
{
    // Constructor using the PDG or the GEANT3 particle ID 'inID'.
    // Set the random number generator seed if 'initDecay' is kTRUE (default).

    if (isGeant3_ID) fPDG = TOGlobals::GetPDG_DB()->GetParticle(TOGlobals::GetPDG_DB()->ConvertGeant3ToPdg(inID));
    else fPDG = TOGlobals::GetPDG_DB()->GetParticle(inID);
    fP4 = new TLorentzVector(0., 0., 0., 0.);
    fStable = kTRUE;
    fNDecay = 0;
    fDecay = 0;
    fMassDistr = 0;
    fVertex = new TVector3();
    fHasVertex = kFALSE;

    // set machine clock based random seed to the standart
    // ROOT random number generator (used by TGenPhaseSpace)
    if (initDecay) gRandom->SetSeed(0);
}

//______________________________________________________________________________
TOMCParticle::~TOMCParticle()
{
    // Destructor.
 
    if (fP4) delete fP4;
    if (fNDecay) 
    {
        for (Int_t i = 0; i < fNDecay; i++) delete fDecay[i];
        delete [] fDecay;
    }
    if (fVertex) delete fVertex;
}

//______________________________________________________________________________
void TOMCParticle::SetGeant3_ID(Int_t inID)
{
    // Set the PDG ID of this particle using the GEANT3 ID 'inID'.
    
    // set ID
    fPDG = TOGlobals::GetPDG_DB()->GetParticle(TOGlobals::GetPDG_DB()->ConvertGeant3ToPdg(inID));
}

//______________________________________________________________________________
Int_t TOMCParticle::GetGeant3_ID() const 
{
    // Return the GEANT3 particle ID of this particle.
    
    // handle self-defined particles
    if (fPDG->PdgCode() == TOGlobals::kDeuteronID) return fPDG->TrackingCode();
    else return TOGlobals::GetPDG_DB()->ConvertPdgToGeant3(fPDG->PdgCode()); 
}

//______________________________________________________________________________
Int_t TOMCParticle::GetPDG_ID() const
{
    // Return the PID ID of this particle.

    return fPDG->PdgCode();
}

//______________________________________________________________________________
const Char_t* TOMCParticle::GetName() const
{
    // Return the name of this particle.

    return fPDG->GetName();
}

//______________________________________________________________________________
Double_t TOMCParticle::GetPDGMassGeV() const
{
    // Return the PDG mass of this particle.

    return fPDG->Mass();
}
 
//______________________________________________________________________________
void TOMCParticle::SetP4(TLorentzVector* inP4)
{
    // Set the 4-vector of this particle to 'inP4'.

    if (fP4) *fP4 = *inP4;
}

//______________________________________________________________________________
void TOMCParticle::SetP4(Double_t inPx, Double_t inPy, Double_t inPz, Double_t inE)
{
    // Set the 4-vector of this particle to 'inE', 'inPx', 'inPy', 'inPz'.

    if (fP4) fP4->SetPxPyPzE(inPx, inPy, inPz, inE);
}

//______________________________________________________________________________
void TOMCParticle::SetVertex(Double_t x, Double_t y, Double_t z)
{
    // Set the vertex of this particle to 'x', 'y', 'z'.

    fVertex->SetXYZ(x, y, z);
}

//______________________________________________________________________________
TOMCParticle* TOMCParticle::AddDecayParticle(const Char_t* inPartName)
{
    // Add the particle with the name 'inPartName' to the list of decay
    // particles.
    // A pointer to the added particle is returned.
    
    // translate some shortcut particle names
    Char_t partName[256];
    if (!strcmp(inPartName, "p")) strcpy(partName, "proton");
    else if (!strcmp(inPartName, "n")) strcpy(partName, "neutron");
    else if (!strcmp(inPartName, "g")) strcpy(partName, "gamma");
    else strcpy(partName, inPartName);
    
    // get particle from PDG database
    TParticlePDG* pdg_pa = TOGlobals::GetPDG_DB()->GetParticle(partName);
    
    // check if the particle exists
    if (!pdg_pa) 
    {
        Error("AddDecayParticle", "Particle '%s' was not found in the PDG database!", inPartName);
        return 0;
    }
    
    // create the particle
    TOMCParticle* part = new TOMCParticle(partName);
  
    // mark particle as unstable to trigger the decay calculation
    fStable = kFALSE;
    
    // set individual vertex depending on mother particle
    if (GetLifetime(GetPDG_ID()) > 0) part->SetHasVertex(kTRUE);

    // enable vertex if mother particle has own vertex itself
    if (fHasVertex) part->SetHasVertex(kTRUE);

    // backup the existing decay products
    TOMCParticle** backup = fDecay;

    // create new decay particle array
    fDecay = new TOMCParticle*[fNDecay + 1];
    
    // copy backuped particles
    for (Int_t i = 0; i < fNDecay; i++) fDecay[i] = backup[i];

    // add new particle
    fDecay[fNDecay] = part;
   
    // increment number of decay particles
    fNDecay++;
    
    // delete the backup
    if (backup) delete [] backup;
 
    // return added particle
    return part;
}

//______________________________________________________________________________
Int_t TOMCParticle::GetTotalNDecayParticles() const
{
    // Returns the total number of particles of this particle's decay chain
    // (recursive method).

    // recursive search through decay chain
    Int_t npart = 0;
    for (Int_t i = 0; i < fNDecay; i++)
    {
        // get decay particle and count it
        TOMCParticle* p = GetDecayParticle(i);
        npart++;

        // count decay particles of this decay particle
        npart += p->GetTotalNDecayParticles();
    }
    
    return npart;
}

//______________________________________________________________________________
Bool_t TOMCParticle::CalculateDecay()
{
    // Calculate a phase-space decay of this particle whole decay chain
    // (recursive method). Return kFALSE if there was an error during the
    // calculation, e.g. insufficient center-of-mass energy for a decay.
    //
    // ATTENTION: All input energies are assumed to be in GeV and all output energies
    //            will be set in GeV!

    // no decay if particle is marked as stable
    if (fStable) return kTRUE;

    // set masses of decay particles
    // choose random mass from distribution if the particle is a Rootino
    // and a mass distribution was specified.
    Double_t masses[fNDecay];
    for (Int_t i = 0; i < fNDecay; i++) 
    {
        // get decay particle
        TOMCParticle* decayPart = GetDecayParticle(i);

        // take mass from distribution if set [GeV]
        if (decayPart->GetMassDistribution()) 
        {
            masses[i] = 0.001 * decayPart->GetMassDistribution()->GetRandom();
        }
        else 
        {
            // check if particle is known by PDG   
            if (decayPart->GetPDG_ID()) 
            {
                masses[i] = decayPart->GetPDGMassGeV();
            }
            else
            {
                Error("CalculateDecay", "No mass found for decay particle! Rootino without mass distribution?");
                return kFALSE;
            }
        }
    }

    // calculate decay
    TGenPhaseSpace decayGen;
    if (decayGen.SetDecay(*fP4, fNDecay, masses))
    {
        // generate decay
        decayGen.Generate();

        // set 4-vectors of decay products
        for (Int_t i = 0; i < fNDecay; i++) GetDecayParticle(i)->SetP4(decayGen.GetDecay(i));
    }
    else 
    {
        // decay could not be calculated (forbidden by kinematics)
        return kFALSE;
    }

    // calculate decay of each decay product
    for (Int_t i = 0; i < fNDecay; i++) 
    {
        // abort decay chain calculation if there was an error
        if (!GetDecayParticle(i)->CalculateDecay()) return kFALSE;
    }

    // calculation is ok here
    return kTRUE;
}

//______________________________________________________________________________
void TOMCParticle::CalculateVertex(TVector3* v)
{
    // Calculate the vertex (if enabled) of this particle and all of its decay 
    // particles (recursive method) based on the preceding vertex 'v'.
    
    // new vertex
    TVector3 vNew;

    // get particle lifetime
    Double_t tau = GetLifetime(GetPDG_ID());

    // check if the particle has some defined lifetime
    if (tau > 0)
    {
        // calculate lifetime in the lab frame (inspired by PLUTO)
        Double_t t = -fP4->Gamma() * tau * TMath::Log(gRandom->Rndm());
        
        // calculate new vertex [cm]
        vNew = *v + 100. * TMath::C() * t * fP4->BoostVector();
    }
    else
    {
        // keep current vertex
        vNew = *v;
    }

    // save current vertex if enabled
    if (fHasVertex) *fVertex = *v;

    // calculate vertex of each decay product
    for (Int_t i = 0; i < fNDecay; i++) 
        GetDecayParticle(i)->CalculateVertex(&vNew);
}

//______________________________________________________________________________
Double_t TOMCParticle::GetLifetime(Int_t pdgID)
{
    // Return the proper lifetime in seconds of the particle with PDG ID 'pdgID'.
    // Return 0 if the particle lifetime is not supported.

    switch (pdgID)
    {
        case 310:
            return TOGlobals::kKS0Lifetime;
        case 321:
            return TOGlobals::kKChargedLifetime;
        case -321:
            return TOGlobals::kKChargedLifetime;
        case 3122:
            return TOGlobals::kLambdaLifetime;
        case 3212:
            return TOGlobals::kSigma0Lifetime;
        case 3222:
            return TOGlobals::kSigmaPLifetime;
        case 3112:
            return TOGlobals::kSigmaMLifetime;
        default:
            return 0;
    }
}

//______________________________________________________________________________
TOMCParticle* TOMCParticle::GetDecayParticle(Int_t index) const
{
    // Return the decay particle with index 'index'. Return 0 if no such 
    // particle could be found.

    // check index range
    if (index < 0 || index >= fNDecay)
    {
        Error("GetDecayParticle", "Decay particle index %d out of bounds (number of decay particles: %d)", index, fNDecay);
        return 0;
    }
    else
    {
        return fDecay[index];
    }
}

//______________________________________________________________________________
void TOMCParticle::PrintDecayChain(Char_t* outString, Int_t level)
{
    // Print the total decay chain of this particle to 'outString' or
    // to stdout if 'outString' is zero.
    // 'level' is used to keep track of the decay hierarchy 
    // (recursive method).
    
    Char_t dchain[256] = "";
    Char_t name[256];
    Bool_t toStdout = kFALSE;

    // create local string if outString is zero
    // (for stdout output)
    if (!outString) 
    {
        outString = dchain;
        toStdout = kTRUE;
    }

    // add space if needed
    if (level != 0 ) 
    {
        if (outString[strlen(outString)-1] != '[')
            strcat(outString, " ");
    }

    // add this particle, set some shortcuts for longer particle names
    if (!strcmp(GetName(), "gamma")) strcpy(name, "g");
    else if (!strcmp(GetName(), "proton")) strcpy(name, "p");
    else if (!strcmp(GetName(), "neutron")) strcpy(name, "n");
    else strcpy(name, GetName());
    strcat(outString, name);
    
    // leave if there are no further decay products
    if (!fNDecay) return;
    
    // open decay chain 
    strcat(outString, "[");

    // recursive search through decay chain
    for (Int_t i = 0; i < fNDecay; i++)
    {
        // call this method of the sub-decay products
        GetDecayParticle(i)->PrintDecayChain(outString, level+1);
    }
    
    // close decay chain
    strcat(outString, "]");

    // print everything to stdout if initial outString was zero
    if (level == 0 && toStdout) printf("%s\n", outString);
}
ClassImp(TOMCParticle)
