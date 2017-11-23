//--Author	JRM Annand    1st Dec 2004
//--Rev         JRM Annand    9th Oct 2008  Add ReadDecoded	
//--Rev         JRM Annand    9th Oct 2008  Generalise ReadDecoded	
//--Rev         JRM Annand   21st May 2009  Store MC incident particles	
//--Rev         JRM Annand   16th Feb 2010  MC Weighted or non-weighted energy
//--Rev         JRM Annand   27th Mar 2010  MC Time over threshold option
//--Rev         JRM Annand   30th May 2012  Check *fNPartMC not negative
//--Update      JRM Annand   10th Oct 2012  Time over threshold
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2GenericDetector
//
// A version of TA2Detector which can be instantiated
//

#include "TA2GenericDetector.h"


enum { EMCnpart, EMCntof, EMCtofi, EMCtofe, EMCtofew, EMCtoft, EMCtofte,
       EMCtofx, EMCtofy, EMCtofz, EMCvertex, EMCbeam, EMCdircos, EMCplab,
       EMCelab };
// for finding data branches in analysis of MC generated data
static const Map_t kMCBranchKeys[] = {
  {"npart",        EMCnpart},
  {"ntof",         EMCntof},
  {"tofi",         EMCtofi},
  {"tofe",         EMCtofe},
  {"tofew",        EMCtofew},
  {"toft",         EMCtoft},
  {"tofte",        EMCtofte},
  {"tofx",         EMCtofx},
  {"tofy",         EMCtofy},
  {"tofz",         EMCtofz},
  {"vertex",       EMCvertex},
  {"beam",         EMCbeam},
  {"dircos",       EMCdircos},
  {"plab",         EMCplab},
  {"elab",         EMCelab},
  {NULL,           -1}
};
//---------------------------------------------------------------------------
TA2GenericDetector::TA2GenericDetector( const char* name,TA2System* apparatus )
  :TA2Detector(name, apparatus)
{
  // Basic primary setup in TA2Detector
  fIminMC = fImaxMC = 0;
  fNHitsMC = fNPartMC = NULL;
  fHitsMC = NULL;
  fIndex2Element = NULL;
  fEnergyMC = NULL;
  fTimeMC = NULL;
  fX_MC = fY_MC = fZ_MC = NULL;
  fVertexMC = NULL;
  fBeamMC = NULL;
  fDirCosMC = NULL;
  fPlabMC = NULL;
  fElabMC = NULL;
  fP4MC = NULL;
  fP4Comb = NULL;
  fEMC = fPMC = fThMC = fPhiMC = NULL;
  fXMC = fYMC = fZMC = NULL;
  fXMCOR = fYMCOR = fZMCOR = NULL;
  fTimeOvThr = NULL;
  fNComb = 0;
  AddCmdList( kGenericDetectorKeys );                  // for SetConfig()
}

//---------------------------------------------------------------------------
TA2GenericDetector::~TA2GenericDetector()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  DeleteArrays();
}

//---------------------------------------------------------------------------
void TA2GenericDetector::DeleteArrays( )
{
  // Free up all allocated memory
  if( fIndex2Element ) delete fIndex2Element;
  if( fXMC ) delete fXMC;
  if( fYMC ) delete fYMC;
  if( fZMC ) delete fZMC;
  if( fXMCOR ) delete fXMCOR;
  if( fYMCOR ) delete fYMCOR;
  if( fZMCOR ) delete fZMCOR;
  if( fP4MC ) delete fP4MC;
  if( fPMC ) delete fPMC;
  if( fEMC ) delete fEMC;
  if( fThMC ) delete fThMC;
  if( fPhiMC ) delete fPhiMC;
  if( fTimeOvThr ) delete fTimeOvThr;
  TA2Detector::DeleteArrays();
}

//---------------------------------------------------------------------------
void TA2GenericDetector::SetConfig( char* line, int key )
{
  // Load basic detector parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kDetectorKeys array at the top of the source .cc file
  // The following are setup...
  //	1. # Elements in the detector, is energy, time, position info input
  //	2. Calibration parameters for each element
  //	3. Any post initialisation
  //	4. Histograms for display...should be done after post-initialisation

  Char_t brSpec[32];  // incident lab energies
  Int_t iComb[8];
  Char_t* ln;
  Char_t* ln1;
  Int_t i;
  switch( key ){
  case EGenericDetectorMC:
    // Setup where to look for MC generated data
    // flexible format
    ln1 = line;
    for(;;){
      i = sscanf( ln1,"%s", brSpec );
      if( !i ) break;
      ln = strstr(ln1,brSpec);          // advance linear point to next
      ln1 = strchr(ln,' ');
      i = Map2Key(brSpec,kMCBranchKeys);
      if( i == -1 ){
	i = sscanf(brSpec,"%d",&fIminMC); // not valid string, integer?
	continue;
      }
      Int_t ib = GetBranchIndex(brSpec);
      if( ib != -1 ){
	switch( i ){
	default:
	  break;
	case EMCnpart:
	  fNPartMC = (Int_t*)(fEvent[ib]);
	  break;
	case EMCntof:
	  fNHitsMC = (Int_t*)(fEvent[ib]);
	  break;
	case EMCtofi:
	  fHitsMC = (Int_t*)(fEvent[ib]);
	  break;
	case EMCtofe:       // unweighted energy
	case EMCtofew:      // weighted energy
	  fEnergyMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCtoft:
	case EMCtofte:
	  fTimeMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCtofx:
	  fX_MC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCtofy:
	  fY_MC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCtofz:
	  fZ_MC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCvertex:
	  fVertexMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCbeam:
	  fBeamMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCdircos:
	  fDirCosMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCplab:
	  fPlabMC = (Float_t*)(fEvent[ib]);
	  break;
	case EMCelab:
	  fElabMC = (Float_t*)(fEvent[ib]);
	  break;
	}
      }
      if( !ln1 ) break;
    }
    break;
  case EGenericDetectorComb:
    i = sscanf(line,"%d%d%d%d%d%d%d%d",
	       iComb,iComb+1,iComb+2,iComb+3,
	       iComb+4,iComb+5,iComb+6,iComb+7);
    fNComb = i;
    fIComb = new Int_t[i];
    for(Int_t j=0; j<fNComb; j++) fIComb[j] = iComb[j];
    break;
  case EGenericDetectorToThr:
    if( !fTimeOvThr ){
      fTimeOvThr = new Double_t[fNelement];
      for(UInt_t i=0; i<fNelement; i++ ) fTimeOvThr[i] = (Double_t)ENullADC;
    }
    sscanf(line,"%d",&i);
    if( (UInt_t)i >= fNelem )
      PrintError(line,"<Time over thresh - element out of range>");
    fElement[i]->SetToThr(line, this);
    break;
  default:
    // Standard detector stuff
    TA2Detector::SetConfig( line, key );
    break;
  }
  return;
}

//---------------------------------------------------------------------------
void TA2GenericDetector::SaveDecoded( )
{
  // Save decoded info to Root Tree file
}

//---------------------------------------------------------------------------
void TA2GenericDetector::PostInit( )
{
  // If a Monte Carlo up arraysrun is being analysed set
  if( fNHitsMC ){
    // hit position storage allocation
    if( fX_MC ){
      fXMC = new Double_t[fNelement+1];
      fXMCOR = new Double_t[fNelement+1];
    }
    if( fY_MC ){
      fYMC = new Double_t[fNelement+1];
      fYMCOR = new Double_t[fNelement+1];
    }
    if( fZ_MC ){
      fZMC = new Double_t[fNelement+1];
      fZMCOR = new Double_t[fNelement+1];
    }
    fHitPos = new TVector3[fNelement+1];
    if( !fImaxMC ) fImaxMC = fNelement + fIminMC;
    // cross reference MC indices with detector elements
    if( fImaxMC ){
      fIndex2Element = new Int_t[fImaxMC];
      // Initialise NULL
      for( Int_t i=0; i<fImaxMC; i++ ) fIndex2Element[i] = ENullADC;
      for( UInt_t i=0; i<fNelement; i++ ){
	Int_t j = fElement[i]->GetIadc();
	// Write value only if NULL (this is for >1 "PMT" per element)
	if( fIndex2Element[j] == ENullADC ) fIndex2Element[j] = i;
      }
    }
  }
  if( fNPartMC ){
    Int_t np = *fNPartMC;
    if( (np <= 0) || (np > EGenericDetectorNPartMC) )
      np = EGenericDetectorNPartMC;
    if( fNComb ) np++;                  // for combination eg 2 or 6 photons
    fP4MC = new TLorentzVector[np];
    if( fNComb ) fP4Comb = fP4MC+np-1; // for combination eg 2 or 6 photons
    fEMC = new Double_t[np];
    fPMC = new Double_t[np];
    fThMC = new Double_t[np];
    fPhiMC = new Double_t[np];
  }
  TA2Detector::PostInit();
}

//---------------------------------------------------------------------------
Int_t TA2GenericDetector::GetBranchIndex( Char_t* brName )
{
  // Search for branch name and return corresponding index.
  // Index used to point to the appropriate section of fEvent array
  // Return -1 if error
  Int_t nbr = gAR->GetNbranch();
  Int_t i;
  for( i=0; i<nbr; i++ ){
    Char_t* brN = gAR->GetBranchName(i);
    if( !brN ) continue;
    if( !strcmp( brName, brN ) ) break;
  }
  if( i == nbr ){
    PrintError(brName,"<Root branch of this name not registered>");
    return -1;
  }
  return i;
}

//-----------------------------------------------------------------------------
void TA2GenericDetector::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  TA2DataManager::LoadVariable("XMC",          fXMC,             EDSingleX);
  TA2DataManager::LoadVariable("YMC",          fYMC,             EDSingleX);
  TA2DataManager::LoadVariable("ZMC",          fZMC,             EDSingleX);
  TA2DataManager::LoadVariable("EMC",          fEMC,             EDSingleX);
  TA2DataManager::LoadVariable("PMC",          fPMC,             EDSingleX);
  TA2DataManager::LoadVariable("ThMC",         fThMC,            EDSingleX);
  TA2DataManager::LoadVariable("PhiMC",        fPhiMC,           EDSingleX);
  TA2DataManager::LoadVariable("XMCOR",        fXMCOR,           EDMultiX);
  TA2DataManager::LoadVariable("YMCOR",        fYMCOR,           EDMultiX);
  TA2DataManager::LoadVariable("ZMCOR",        fZMCOR,           EDMultiX);
  if( fTimeOvThr ){
    TA2DataManager::LoadVariable("TimeOvThr",  fTimeOvThr,       EDSingleX);
  }
  TA2Detector::LoadVariable();
}

ClassImp(TA2GenericDetector)
