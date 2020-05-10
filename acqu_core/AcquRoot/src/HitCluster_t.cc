//--Author	JRM Annand   30th Aug 2003
//--Rev 	A.Starostin..24th Jan 2004  add theta,phi
//--Rev 	JRM Annand   30th Jan 2004  mod neighbour determination
//--Rev 	JRM Annand   21st Oct 2004  cluster time
//--Rev 	JRM Annand    9th Mar 2005  protected instead of private vars
//--Rev 	JRM Annand   14th Apr 2005  energy fraction central crystal
//--Rev 	JRM Annand    6th Jun 2005  up to 48 nearest neighbours
//--Rev 	JRM Annand   13th Jul 2005  add Merge function
//--Rev 	JRM Annand   19th Oct 2005  up to 64 nearest neighbours
//--Rev 	JRM Annand   12th May 2007  mean radius
//--Rev 	D Glazier    24th Aug 2007  Add IsTime check
//--Rev 	JRM Annand   20th Nov 2009  Adjustable energy weighting
//--Update	JRM Annand   17th Sep 2011  log energy weighting
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// HitCluster_t
// Specimen for hit cluster determination in a segmented calorimeter
// Mean cluster position obtained from the sqrt(E) weighted sum
//
//---------------------------------------------------------------------------

#include "HitCluster_t.h"
#include "TA2ClusterDetector.h"

//---------------------------------------------------------------------------
HitCluster_t::HitCluster_t( Char_t* line, UInt_t index, Int_t sizefactor)
{
  // store input parameters
  // # inner nearest neighbours (outer calculated from total read)
  // coordinates of center of front face of central element
  // List of nearest neighbours inner & outer
  UInt_t hit[64];
  fIndex = index;
  UInt_t n = 
    sscanf( line, 
	    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
	    &fNNearNeighbour,
	    hit,   hit+1, hit+2, hit+3, hit+4, hit+5, hit+6, hit+7,
	    hit+8, hit+9, hit+10,hit+11,hit+12,hit+13,hit+14,hit+15,
	    hit+16,hit+17,hit+18,hit+19,hit+20,hit+21,hit+22,hit+23,
 	    hit+24,hit+25,hit+26,hit+27,hit+28,hit+29,hit+30,hit+31,
	    hit+32,hit+33,hit+34,hit+35,hit+36,hit+37,hit+38,hit+39,
	    hit+40,hit+41,hit+42,hit+43,hit+44,hit+45,hit+46,hit+47, 
	    hit+48,hit+49,hit+50,hit+51,hit+52,hit+53,hit+54,hit+55,
	    hit+56,hit+57,hit+58,hit+59,hit+60,hit+61,hit+62,hit+63 );

  // Consistency check...1st hit must be the index
  if( (n != (fNNearNeighbour + 1)) || (index != *hit) ){
    printf(" Error in nearest neighbour input at line:\n %s\n", line );
    exit(1);
  }
  n -= 2;                         // # neighbours around central element
  fNNeighbour = n;
  fNeighbour = new UInt_t[n];
  fMaxHits = (n+1) * sizefactor;
  fHits = new UInt_t[ fMaxHits ];
  fEnergies = new Double_t[ fMaxHits ];
  fTimes = new Double_t[ fMaxHits ];  
  fHits[0] = ENullHit;
  fNhits = 0;
  fEnergy = (Double_t)ENullHit;
  for( UInt_t i=0; i<n; i++ ) fNeighbour[i] = hit[i+1];
  fMeanPosition = new TVector3(0.0, 0.0, 0.0);
  fTime = 0.0;                   // in case its not defined
}

//---------------------------------------------------------------------------
HitCluster_t::~HitCluster_t( )
{
  // delete arrays
  if( fHits ) delete fHits;
  if( fNeighbour ) delete fNeighbour;
  if( fMeanPosition ) delete fMeanPosition;
}

//---------------------------------------------------------------------------
void HitCluster_t::BuildElementary(TA2ClusterDetector* det)
{
    // Build the elementary cluster consisting only of the central element.

    fNhits = 1;
    fHits[0] = fIndex;
    fEnergies[0] = det->GetEnergy()[fIndex];
    fTimes[0] = det->GetTime()[fIndex];
    fHits[fNhits] = EBufferEnd;

    *fMeanPosition = *(det->GetPosition()[fIndex]);
    fEnergy = fEnergies[0];
    fCentralFrac = 1;
    fSqrtEtot = 0;
    fTime = fTimes[0];
    fTheta = TMath::RadToDeg() * fMeanPosition->Theta();
    fPhi = TMath::RadToDeg() * fMeanPosition->Phi();
    fRadius = 0;
}

