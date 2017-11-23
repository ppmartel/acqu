//--Author	JRM Annand   27th Sep 2004
//--Rev 	JRM Annand...
//--Rev 	JRM Annand...23rd Oct 2004 Integrate to apparatus
//--Rev 	JRM Annand...17th Mar 2005 Store particle types, SetMassP4
//--Rev 	JRM Annand...15th Apr 2005 SetMassP4 mass in MeV bugfix
//--Rev 	JRM Annand...20th Apr 2005 Remove local 4-mom array
//--Rev 	JRM Annand...22nd Jan 2007 4v0 update
//--Rev 	JRM Annand... 6th Feb 2008 P4 from TOF
//--Update	JRM Annand... 1st Sep 2009 delete[], zap fP4
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2ParticleID
// Particle attributes using PDG numbering scheme....
// As used in GEANT-4, but NOT GEANT-3 
//

#include "TA2ParticleID.h"
#include "TA2Analysis.h"
#include "CMakeConfig.h"

R__EXTERN TA2Analysis*  gAN;


//---------------------------------------------------------------------------
TA2ParticleID::TA2ParticleID( char* line ) :
  TA2System( "PDG", NULL )
{
  // Setup maximum number of particles to consider in reaction.
  // Optionally supply the PDG indices of much-used particle species.
  // ....masses etc. then stored in local arrays

  SetLogStream( gAN->GetLogStream() );        // text O/P to analysis log

  Int_t p[] = {0,0,0,0,0,0,0,0};        // particle types....all ROOTinos
  Int_t n = sscanf( line, "%d%d%d%d%d%d%d%d%d", &fMaxType,
		    p, p+1, p+2, p+3, p+4, p+5, p+6, p+7 );
  switch (n) {
  case 0:
    PrintError( line, "<PDG Particles to consider not correctly specified>" );
    return;
  case 1:
    // No particle types supplied ...default photons
    fMaxType = 1;
    p[0] = kGamma;
    fprintf(fLogStream,
	    " PDG default: particles all photons, PDG index %d\n", p[0]);
    break;
  default:
    if( fMaxType >= n ) fMaxType = n-1;
    // Log registered particles
    fprintf(fLogStream,
	    "PDG codes of saved particle species:%d %d %d %d %d %d %d %d\n",
	    p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
    break;
  }
  fPDG = new TDatabasePDG();
  char* dbname = BuildName(ENV_OR_CMAKE("ROOTSYS", CMAKE_ROOTSYS), "/etc/pdg_table.txt" );
  fPDG->ReadPDGTable(dbname);
  delete[] dbname;
  fPDGtype = new Int_t[fMaxType];
  fMass = new Double_t[fMaxType];
  fCharge = new Double_t[fMaxType];
  for(Int_t i=0; i<fMaxType; i++){
    fPDGtype[i] = p[i];
    fMass[i] = fPDG->GetParticle(p[i])->Mass();
    fCharge[i] =  fPDG->GetParticle(p[i])->Charge();
  }
  return;
}

//---------------------------------------------------------------------------
TA2ParticleID::~TA2ParticleID( )
{
  // Free memory allocated
  if( fPDGtype ) delete[] fPDGtype;
  if( fMass ) delete[] fMass;
  if( fCharge ) delete[] fCharge;
  if( fPDG ) delete fPDG;
}
ClassImp(TA2ParticleID)
