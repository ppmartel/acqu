//--Author	JRM Annand   30th Sep 2003  Read MC data
//--Rev 	JRM Annand...15th Sep 2003  Generalise methods
//--Rev 	JRM Annand....9th Mar 2004  New OR variables
//--Rev 	JRM Annand....9th Mar 2005  protected instead of private vars
//--Rev 	JRM Annand...13th Jul 2005  split offs, time OR
//--Rev 	JRM Annand...25th Jul 2005  SetConfig hierarchy
//--Rev 	JRM Annand...20th Oct 2005  Iterative neighbour find (TAPS)
//--Rev 	JRM Annand... 9th Dec 2005  Change ambigous split-off thresh
//--Rev 	JRM Annand... 6th Feb 2006  Bug in SplitSearch
//--Rev 	JRM Annand...21st Apr 2006  Command-key enum to .h
//--Rev 	JRM Annand...22nd Jan 2007  4v0 update
//--Rev 	JRM Annand...12th May 2007  Central-frac and radius
//--Rev 	JRM Annand...18th Jan 2009  TMath::Sort (Root v5.22)
//--Update	JRM Annand   17th Sep 2011  log energy weighting
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2ClusterDetector
//
// Decoding and calibration methods for EM calorimeters or similar systems
// where a shower or showers of secondary particles fire a cluster or
// clusters neighbouring calorimeter elements,
// e.g. Crystal Ball NaI(Tl) array
//

#define NICE_EVENT 106

#include "TA2ClusterDetector.h"
#include "HitClusterTrad_t.h"
#include "HitClusterTAPS_t.h"
#include "HitClusterUCLA_t.h"
#include "TMarker.h"

// Command-line key words which determine what to read in
static const Map_t kClustDetKeys[] = {
  {"Max-Cluster:",          EClustDetMaxCluster},
  {"Next-Neighbour:",       EClustDetNeighbour},
  {"All-Neighbour:",        EClustDetAllNeighbour},
  {"Split-Off:",            EClustDetSplitOff},
  {"Iterate-Neighbours:",   EClustDetIterate},
  {"Energy-Weight:",        EClustEnergyWeight},
  {"Cluster-Algorithm:",    EClustAlgo},
  {NULL,          -1}
};

#include <sstream>
#include <TA2Analysis.h>

//---------------------------------------------------------------------------
TA2ClusterDetector::TA2ClusterDetector( const char* name,
          TA2System* apparatus )
  :TA2Detector(name, apparatus)
{
  // Do not allocate any "new" memory here...Root will wipe
  // Set private variables to zero/false/undefined state
  // Add further commands to the default list given by TA2Detector
  AddCmdList( kClustDetKeys );
  fCluster = NULL;
  fClustHit = NULL;
  fIsSplit = NULL;
  fTempHits = NULL;
  fTempHits2 = NULL;
  fTryHits = NULL;
  fNCluster = fNSplit = fNSplitMerged = fMaxCluster = 0;
  fClustSizeFactor = 1;
  fNClustHitOR = NULL;
  fTheta = fPhi = fClEnergyOR = fClTimeOR = fClCentFracOR = fClRadiusOR = NULL;
  fClEthresh = fEthresh = fEthreshSplit = 0.0;
  fMaxThetaSplitOff = fMinPosDiff  = fMaxPosDiff = 0.0;
  fEWgt = 0.0;
  fLEWgt = 0.0;
  fSplitAngle = NULL;
  fISplit = fIJSplit = NULL;
  fMaxSplitPerm = 0;
  fIsIterate = kFALSE;
  fClustAlgoType = EClustAlgoEmpty;

  fDispClusterEnable = kFALSE; // config stuff missing...
  // will be set by child class
  fDispClusterHitsAll    = NULL;
  fDispClusterHitsSingle = NULL;  
  fDispClusterHitsEnergy = NULL;

  fIsPos = ETrue;          // override standard detector, must have position
}


//---------------------------------------------------------------------------
TA2ClusterDetector::~TA2ClusterDetector()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  // Start with common arrays of TA2Detector class
  DeleteClusterArrays();
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::DeleteClusterArrays( )
{
  // Free up allocated memory
  DeleteArrays();
  for( UInt_t i=0; i<fMaxCluster; i++ ){
    if( fCluster[i] ) delete fCluster[i];
  }
  delete fCluster;
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::SetConfig( char* line, int key )
{
  // Load Ladder parameters from file or command line
  // Keywords which specify a type of command can be found in
  // the kLaddKeys array at the top of the source .cc file
  // The following are setup...
  //	1. # X-triggers, #elements in ladder array, global TDC offset
  //	2. X-trigger parameters if any, TDC, TDC calib, TDC cut
  //	3. Ladder parameters, TDC, calib, cut window, Eg calib, Scaler
  //	4. Any post initialisation
  //	5. Histograms...should be done after post-initialisation

  // Cluster specific configuration
  switch( key ){
  case EClustDetMaxCluster:
    // Max number of clusters
    if( sscanf( line, "%d%lf", &fMaxCluster, &fClEthresh ) < 1 ){
      PrintError(line,"<Parse # clusters>");
      break;
    }
    fEthresh = fClEthresh;
    fCluster = new HitCluster_t*[fNelement+1];
    fClustHit = new UInt_t[fMaxCluster+1];
    fTempHits = new UInt_t[fNelement+1];
    fTempHits2 = new UInt_t[fNelement+1];
    fTryHits = new UInt_t[fNelement+1];
    fNClustHitOR = new UInt_t[fNelement+1];
    fTheta = new Double_t[fNelement+1];
    fPhi      = new Double_t[fNelement+1];
    fClEnergyOR  = new Double_t[fNelement+1];
    fClTimeOR  = new Double_t[fNelement+1];
    fClCentFracOR  = new Double_t[fNelement+1];
    fClRadiusOR  = new Double_t[fNelement+1];
    fNCluster = 0;
    break;
  case EClustDetIterate:
    // Enable iterative cluster-member determination
    // Must be done before any next-neighbour setup
    if( fNCluster ){
      PrintError(line,"<Enable iteration before cluster-neighbour setup>");
      break;
    }
    if( sscanf( line, "%d%lf%lf",
    &fClustSizeFactor, &fMinPosDiff, &fMaxPosDiff ) < 3 ){
      PrintError(line,"<Cluster-iterate parse>");
      break;
    }
    PrintMessage(" Iterative cluster neighbour seek enabled\n");
    fIsIterate = kTRUE;
    break;
  case EClustDetNeighbour:
    // Nearest neighbout input
    if( fNCluster < fNelement )
    {
      switch (fClustAlgoType)
      {
        case EClustAlgoTrad:
          fCluster[fNCluster] = new HitClusterTrad_t(line,fNCluster,fClustSizeFactor,fEWgt,fLEWgt);
          break;
        case EClustAlgoTAPS:
          fCluster[fNCluster] = new HitClusterTAPS_t(line,fNCluster);
          break;
        case EClustAlgoUCLA:
          fCluster[fNCluster] = new HitClusterUCLA_t(line,fNCluster,fClustSizeFactor);
          break;
        default:
          PrintError("Unknown or unconfigured cluster algorithm!");
          break;
      }
      fNCluster++;
    }
    break;
  case EClustDetAllNeighbour:
    // All nearest neighbours (for diagnostics only)
    for(fNCluster=0; fNCluster < fNelement; fNCluster++)
    {
      switch (fClustAlgoType)
      {
        case EClustAlgoTrad:
          fCluster[fNCluster] = new HitClusterTrad_t(line,fNCluster,fClustSizeFactor,fEWgt,fLEWgt);
          break;
        case EClustAlgoTAPS:
          fCluster[fNCluster] = new HitClusterTAPS_t(line,fNCluster);
          break;
        case EClustAlgoUCLA:
          fCluster[fNCluster] = new HitClusterUCLA_t(line,fNCluster,fClustSizeFactor);
          break;
        default:
          PrintError("Unknown or unconfigured cluster algorithm!");
          break;
      }
    }
    break;
  case EClustDetSplitOff:
    // Enable split-off search
    if( sscanf( line, "%lf%lf", &fEthreshSplit, &fMaxThetaSplitOff ) < 1 ){
      PrintError(line,"<Parse split off threshold>");
      break;
    }
    fEthresh = fEthreshSplit;          // reset generic threshold
    for(UInt_t i=2; i<fMaxCluster; i++) fMaxSplitPerm += i;
    fSplitAngle = new Double_t[fMaxSplitPerm];
    fISplit = new Int_t[fMaxSplitPerm];
    fIJSplit = new Int_t[fMaxSplitPerm];
    fIsSplit = new Bool_t[fMaxCluster];
    break;
  case EClustEnergyWeight:
    // Set energy-weighting factor for position determination
    // default = sqrt(E)
    if( sscanf( line, "%lf%d", &fEWgt, &fLEWgt ) < 1 ){
      PrintError(line,"<Parse energy weighting factor>");
    }
    break;
  case EClustAlgo:
  {
    // select cluster algorithm
    TString s(line);
    s.ReplaceAll(" ", "");
    if (s.EqualTo("Trad")) fClustAlgoType = EClustAlgoTrad;
    else if (s.EqualTo("TAPS")) fClustAlgoType = EClustAlgoTAPS;
    else if (s.EqualTo("UCLA")) fClustAlgoType = EClustAlgoUCLA;
    break;
  }
  default:
    // Command not found...try standard detector
    TA2Detector::SetConfig( line, key );
    break;;
  }
  return;
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // Start with alignment offsets
  // Create space for various output arrays
  TA2Detector::PostInit();

  // log type of cluster algorithm
  switch (fClustAlgoType)
  {
    case EClustAlgoTrad:
      PrintMessage("Cluster algorithm: Traditional\n");
      break;
    case EClustAlgoTAPS:
      PrintMessage("Cluster algorithm: TAPS\n");
      break;
    case EClustAlgoUCLA:
      PrintMessage("Cluster algorithm: UCLA\n");
      break;
    default:
      PrintError("Unknown or unconfigured cluster algorithm!");
      break;
  }
}

//-----------------------------------------------------------------------------
void TA2ClusterDetector::LoadVariable( )
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

  //  TA2DataManager::LoadVariable("ClEnergy",  &fClEnergy,   EDSingleX);
  //  TA2DataManager::LoadVariable("ClTimeOR",  f???,         EDSingleX);
  //  TA2DataManager::LoadVariable("ClHits",    fClHits,           EDSingleX);
  //  TA2DataManager::LoadVariable("ClMulti",   fClMulti,         EDMultiX);
  TA2DataManager::LoadVariable("ClNhits",       &fNCluster,      EISingleX);
  TA2DataManager::LoadVariable("ClNSplit",      &fNSplit,        EISingleX);
  TA2DataManager::LoadVariable("ClNSplitMerged",&fNSplitMerged,  EISingleX);
  TA2DataManager::LoadVariable("ClTotHits",     fClustHit,       EIMultiX);
  TA2DataManager::LoadVariable("ClTheta",       fTheta,          EDMultiX);
  TA2DataManager::LoadVariable("ClPhi",         fPhi,            EDMultiX);
  TA2DataManager::LoadVariable("ClNhitsOR",     fNClustHitOR,    EIMultiX);
  TA2DataManager::LoadVariable("ClEnergyOR",    fClEnergyOR,     EDMultiX);
  TA2DataManager::LoadVariable("ClTimeOR",      fClTimeOR,       EDMultiX);
  TA2DataManager::LoadVariable("ClCentFracOR",  fClCentFracOR,   EDMultiX);
  TA2DataManager::LoadVariable("ClRadiusOR",    fClRadiusOR,     EDMultiX);
  TA2Detector::LoadVariable();
}

//-----------------------------------------------------------------------------
void TA2ClusterDetector::ParseDisplay( char* line )
{
  // Input private histogram spec to general purpose parse
  // and setup routine

  // List of local cluster histograms
  const Map_t kClustDetHist[] = {
    {"ClEnergy",         EClustDetEnergy},
    {"ClTime",           EClustDetTime},
    {"ClCentFrac",       EClustDetCentFrac},
    {"ClRadius",         EClustDetRadius},
    {"ClHits",           EClustDetHits},
    {"ClMulti",          EClustDetMulti},
    {NULL,                  -1}
  };

  UInt_t i,j,k,l,chan;
  Double_t low,high;
  Char_t name[EMaxName];
  Char_t histline[EMaxName];

  // Do the rather specialist cluster display setup.
  // This doesn't fit too well with the Name2Variable_t model as
  // the cluster info is stored in separate cluster classes
  // If not specialist cluster display call standard detector display

  if( sscanf(line, "%s%s", histline, name) != 2 ){
    PrintError(line,"<Cluster display - bad format>");
    return;
  }
  i = Map2Key( name, kClustDetHist );
  switch( i ){
  case EClustDetEnergy:
  case EClustDetHits:
  case EClustDetTime:
  case EClustDetMulti:
    // Cluster spectra
    // cluster  chan bins, range low--high, elements j--k,
    if( sscanf(line, "%*s%*s%d%lf%lf%d%d", &chan,&low,&high,&j,&k) != 5 ){
      PrintError(line,"<Cluster display - bad format>");
      return;
    }
    for( l=j; l<=k; l++ ){
      if( l >= fNCluster ){
  PrintError(line,"<Cluster display - element outwith range>");
  return;
      }
      sprintf( histline, "%s%d %d  %lf %lf",name,l,chan,low,high );
      switch( i ){
      case EClustDetEnergy:
  Setup1D( histline, fCluster[l]->GetEnergyPtr() );
  break;
      case EClustDetHits:
  Setup1D( histline, fCluster[l]->GetHits(), EHistMultiX );
  break;
      case EClustDetTime:
  Setup1D( histline, fCluster[l]->GetTimePtr() );
  break;
      case EClustDetMulti:
  Setup1D( histline, fCluster[l]->GetNhitsPtr() );
  break;
      }
    }
    break;
  default:
    // Try the standard display if not specialist cluster stuff
    TA2HistManager::ParseDisplay(line);
    break;
  }

  fIsDisplay = ETrue;
  return;
}


//-----------------------------------------------------------------------------
void TA2ClusterDetector::DisplayClusters() {
  if(!fDispClusterEnable)
    return;

  if(gAN->GetNEvent()<NICE_EVENT) {
    return;
  }
  else if(gAN->GetNEvent()>NICE_EVENT) {
    usleep(5e5);
    return;
  }
  // clear histograms
  fDispClusterHitsEnergy->GetListOfFunctions()->Clear();
  for(UInt_t i=0;i<fNelement;i++) {
    fDispClusterHitsAll->SetElement(i,0);    
    fDispClusterHitsEnergy->SetElement(i,0);     
    for(int i=0;i<MAX_DISP_CLUSTERS;i++) {
      fDispClusterHitsSingle[i]->SetElement(i,0);           
    }
  }
  
  for(UInt_t i=0;i<fNhits;i++) {
    fDispClusterHitsEnergy->SetElement(fHits[i],fEnergy[fHits[i]]);
  }
  
  for(UInt_t i=0;i<fNCluster;i++) {
    HitCluster_t* cl = fCluster[fClustHit[i]];
    UInt_t* hits = cl->GetHits();
    Double_t* energies = cl->GetEnergies();
    UInt_t nHits = cl->GetNhits();
    TVector3* pos = cl->GetMeanPosition();
    fDispClusterHitsEnergy->GetListOfFunctions()->Add(new TMarker(pos->X(), pos->Y(), 8));
    for(UInt_t j=0;j<nHits;j++) {
      Double_t val = fDispClusterHitsAll->GetElement(hits[j]);
      val += 1<<i;
      if(j==0)
        val += 0.1*(i+1);
      fDispClusterHitsAll->SetElement(hits[j],val);
      if(i>=MAX_DISP_CLUSTERS)
        continue;
      //Double_t energy = energies[j]<0.1 ? 0 : energies[j];
      fDispClusterHitsSingle[i]->SetElement(hits[j],energies[j]);      
      /*cout << "i="<<i<<" j="<<j<<" energy="<<energies[j]
              <<" hit=" << hits[j]
              <<endl;*/
    }
  }
  std::stringstream ss;
  ss << fDispClusterHitsAll->GetName();
  ss << " Event=" << gAN->GetNEvent();
  ss << " Clusters=" << fNCluster;
  fDispClusterHitsAll->SetTitle(ss.str().c_str());
  //usleep(5e5);
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::DecodeCluster( )
{
  switch (fClustAlgoType)
  {
    case EClustAlgoTrad:
      DecodeClusterTrad();
      break;
    case EClustAlgoTAPS:
      DecodeClusterTrad();
      break;
    case EClustAlgoUCLA:
      DecodeClusterUCLA();
      break;
    default:
      PrintError("Unknown or unconfigured cluster algorithm!");
      break;
  }
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::DecodeClusterTrad( )
{
  // Determine clusters of hits
  // Search around peak energies absorbed in individual crystals
  // Make copy of hits array as the copy will be altered

  memcpy( fTempHits, fHits, sizeof(UInt_t)*fNhits );  // temp copy
  //  fNCluster = 0;
  Double_t maxenergy;
  UInt_t i,j,k,kmax,jmax;
  // Find hit with maximum energy
  for( i=0; i<fMaxCluster;  ){
    maxenergy = 0;
    for( j=0; j<fNhits; j++ ){
      if( (k = fTempHits[j]) == ENullHit ) continue;
      if( maxenergy < fEnergy[k] ){
  maxenergy = fEnergy[k];
  kmax = k;
  jmax = j;
      }
    }
    if( maxenergy == 0 ) break;              // no more isolated hits
    if( kmax < fNelement ){
      fCluster[kmax]->ClusterDetermine( this ); // determine the cluster
      if( fCluster[kmax]->GetEnergy() >= fEthresh ){
  fClustHit[i] = kmax;
  fTheta[i] = fCluster[kmax]->GetTheta();
  fPhi[i] = fCluster[kmax]->GetPhi();
  fNClustHitOR[i] = fCluster[kmax]->GetNhits();
  fClEnergyOR[i] = fCluster[kmax]->GetEnergy();
  fClTimeOR[i] = fCluster[kmax]->GetTime();
  fClCentFracOR[i] = fCluster[kmax]->GetCentralFrac();
  fClRadiusOR[i] = fCluster[kmax]->GetRadius();
  i++;
      }
    }
    // If you reach here then there is an error in the decode
    // possible bad detector ID
    else fTempHits[jmax] = ENullHit;
  }
  fNCluster = i;                   // save # clusters
  // Now search for possible split offs if this is enabled
  if( fMaxSplitPerm ) SplitSearch();
  fClustHit[fNCluster] = EBufferEnd;
  fTheta[fNCluster] = EBufferEnd;
  fPhi[fNCluster] = EBufferEnd;
  fNClustHitOR[fNCluster] = EBufferEnd;
  fClEnergyOR[fNCluster] = EBufferEnd;
  fClTimeOR[fNCluster] = EBufferEnd;
  fClCentFracOR[fNCluster] = EBufferEnd;
  fClRadiusOR[fNCluster] = EBufferEnd;
}

//---------------------------------------------------------------------------
void TA2ClusterDetector::DecodeClusterUCLA()
{
  // Determine clusters of hits
  // Search around peak energies absorbed in individual crystals
  // Make copy of hits array as the copy will be altered

/*   std::cout<<"Beginning DecodeClusterUCLA"<<std::endl; */
  memcpy( fTempHits, fHits, sizeof(UInt_t)*fNhits );  // temp copy

  const Double_t fEthcrs_CB = 4., fEthcls_CB = 12.;
  const Double_t fEthcrs_TAPS = 4., fEthcls_TAPS = 12.;
  const Double_t fEthcls2_CB = 50., fEthcls2_TAPS = 50.;
  const Double_t opangl = 32., difmax = 24.;
  Double_t fEthcrs, fEthclsi, fEthcls, fEthcls2, thet, phi, Ecl, Ecli, oang;

  Double_t maxenergy;
  UInt_t i, j, k, m, ntaken, nc;
  UInt_t kmax = 0;
  UInt_t jmax = 0;
  //UInt_t nwid=0,
  //UInt_t indwid[15]={15*0},
  UInt_t nomit=0;
  TVector3 vcl, vcln, vdif;

  /*static Int_t ifirst = 0;
  char hiname[100];

  if(ifirst==0)
  {
    ifirst = 1;
    for(i=0; i<720; i++)
    {
      sprintf(hiname,"h_energyor_timeor_%d",i);
      htmp[i] = new TH2F(hiname,hiname,200,-50.,150.,500,0.,500.);
    }
  }
  */
  fNCluster = 0;
  if(fNhits>250 || fNhits<1) goto OUT;
/*   std::cout<<"here in DecodeClusterUCLA"<<std::endl; */
  if(fNelement == 720)
  {
    fEthcrs=fEthcrs_CB; fEthcls=fEthcls_CB; fEthcls2=fEthcls2_CB;
    /*
    for(j=0; j<fNhits; j++)
    {
      k = fTempHits[j];
      if(k>=0 && k < 720) htmp[k]->Fill(fTime[k], fEnergy[k]);
    }
    */
  }
  else
  {
    fEthcrs=fEthcrs_TAPS; fEthcls=fEthcls_TAPS; fEthcls2=fEthcls2_TAPS;
  }
  // Find hit with maximum energy

  for(m=0; m<fNhits; m++) fTryHits[m] = fTempHits[m];

  for(i=0; i<fMaxCluster; )
  {
    maxenergy = 0;
    for(j=0; j<fNhits; j++)
    {
      if((k = fTryHits[j])  == ENullHit) continue;
      if((k = fTempHits[j]) == ENullHit) continue;
      if((maxenergy < fEnergy[k]) && (fEnergy[k] > fEthcrs))
      {
	maxenergy = fEnergy[k];
	kmax = k;
	jmax = j;
      }
    }
    if(maxenergy==0) break;              // no more isolated hits
    if(kmax < fNelement)
    {
      for(m=0; m<fNhits; m++) fTempHits2[m] = fTempHits[m];
      //std::cout<<"here2 in DecodeClusterUCLA"<<std::endl;
      fCluster[kmax]->ClusterDetermine(this); // determine the cluster
      Ecl = fCluster[kmax]->GetEnergy();
      if(Ecl>=fEthcls)
      {
        if ( Ecl < fEthcls2 && fNCluster>0 )
        {
          for (j=0; j<fNCluster; j++)
          {
	    Ecli = fClEnergyOR[j];
	    fEthclsi = 25.+ 25. * Ecli / 1000.;
            thet = fTheta[j] * TMath::DegToRad();
            phi  = fPhi[j] * TMath::DegToRad();
	    vcl.SetMagThetaPhi ( 146., thet, phi );
            thet = fCluster[kmax]->GetTheta() * TMath::DegToRad();
            phi  = fCluster[kmax]->GetPhi() * TMath::DegToRad();
	    vcln.SetMagThetaPhi( 146., thet, phi );
	    if ( fNelement==720 )
            {
	      oang = vcl.Angle(vcln)*TMath::RadToDeg();
	      if ( oang < opangl && Ecl < fEthclsi )
              {
		nomit++;
		fTryHits[jmax] = ENullHit;
		goto NEXTCR;
	      }
	    }
	    else
            {
	      vdif = vcl - vcln;
	      if((vdif.Mag() < difmax)  && (Ecl < fEthclsi))
              {
		nomit++;
		fTryHits[jmax] = ENullHit;
		goto NEXTCR;
	      }
	    }
	  }
	}
        for(m=0; m<fNhits; m++) fTempHits[m] = fTempHits2[m];
	fClustHit[i] = kmax;
	fTheta[i] = fCluster[kmax]->GetTheta();
	fPhi[i] = fCluster[kmax]->GetPhi();
	fNClustHitOR[i] = fCluster[kmax]->GetNhits();
	fClEnergyOR[i] = Ecl;
	fClRadiusOR[i] = ((HitClusterUCLA_t*)fCluster[kmax])->ClusterRadiusUCLA(this);
	i++;
	fNCluster = i;
      }
      else fTryHits[jmax] = ENullHit;
    }
    // If you reach here then there is an error in the decode
    // possible bad detector ID
    else fTryHits[jmax] = ENullHit;
  NEXTCR: continue;
  }
 OUT:
  fClustHit[fNCluster] = EBufferEnd;
  fTheta[fNCluster] = EBufferEnd;
  fPhi[fNCluster] = EBufferEnd;
  fNClustHitOR[fNCluster] = EBufferEnd;
  fClEnergyOR[fNCluster] = EBufferEnd;
  fClRadiusOR[fNCluster] = EBufferEnd;

  if(fNCluster==0) return;

  ntaken=0;
  for(m=0; m<fNhits; m++)
  {
    fTempHits2[m] = fTempHits[m];
    if(fTempHits2[m]==ENullHit) ntaken++;
  }
  if(ntaken==fNhits) return;

  for(j=0; j<fNCluster; j++)
  {
    kmax = fClustHit[j];
    if(((HitClusterUCLA_t*)fCluster[kmax])->ClusterDetermine2(this)) // the wider cluster
    {
      fTheta[j] = fCluster[kmax]->GetTheta();
      fPhi[j] = fCluster[kmax]->GetPhi();
      fNClustHitOR[j] = fCluster[kmax]->GetNhits();
      fClEnergyOR[j] = fCluster[kmax]->GetEnergy();
      fClRadiusOR[j] = ((HitClusterUCLA_t*)fCluster[kmax])->ClusterRadiusUCLA(this);
    }
  }

  if(nomit==0) return;
  ntaken = 0;
  for(m=0; m<fNhits; m++)
  {
    if(fTempHits2[m]==ENullHit) ntaken++;
    fTryHits[m] = fTempHits[m] = fTempHits2[m];
  }
  if(ntaken==fNhits) return;

  nc = fNCluster;
  for( i=nc; i<fMaxCluster; )
  {
    maxenergy = 0;
    for( j=0; j<fNhits; j++ )
     {
      if((k = fTryHits[j])==ENullHit) continue;
      if((k = fTempHits[j])==ENullHit) continue;
      if((maxenergy < fEnergy[k]) && (fEnergy[k] > fEthcrs))
      {
	maxenergy = fEnergy[k];
	kmax = k;
	jmax = j;
      }
    }
    if(maxenergy==0) break;              // no more isolated hits
    if(kmax < fNelement)
    {
      for(m=0;m<fNhits;m++) fTempHits2[m] = fTempHits[m];
      fCluster[kmax]->ClusterDetermine(this); // determine the cluster
      Ecl = fCluster[kmax]->GetEnergy();
      if(Ecl>=fEthcls)
      {
        for(m=0;m<fNhits;m++) fTempHits[m] = fTempHits2[m];
	fClustHit[i] = kmax;
	fTheta[i] = fCluster[kmax]->GetTheta();
	fPhi[i] = fCluster[kmax]->GetPhi();
	fNClustHitOR[i] = fCluster[kmax]->GetNhits();
	fClEnergyOR[i] = Ecl;
	fClRadiusOR[i] = ((HitClusterUCLA_t*)fCluster[kmax])->ClusterRadiusUCLA(this);
	i++;
	fNCluster = i;
      }
      else fTryHits[jmax] = ENullHit;
    }
    // If you reach here then there is an error in the decode
    // possible bad detector ID
    else fTryHits[jmax] = ENullHit;
  }
  fClustHit[fNCluster] = EBufferEnd;
  fTheta[fNCluster] = EBufferEnd;
  fPhi[fNCluster] = EBufferEnd;
  fNClustHitOR[fNCluster] = EBufferEnd;
  fClEnergyOR[fNCluster] = EBufferEnd;
  fClRadiusOR[fNCluster] = EBufferEnd;

  if(nc==fNCluster) return;
  ntaken = 0;
  for(m=0; m<fNhits; m++) if(fTempHits2[m]==ENullHit) ntaken++;
  if(ntaken==fNhits) return;

  for (j=nc; j<fNCluster; j++ )
  {
    kmax = fClustHit[j];
    if(((HitClusterUCLA_t*)fCluster[kmax])->ClusterDetermine2(this))  // the wider cluster
    {
      fTheta[j] = fCluster[kmax]->GetTheta();
      fPhi[j] = fCluster[kmax]->GetPhi();
      fNClustHitOR[j] = fCluster[kmax]->GetNhits();
      fClEnergyOR[j] = fCluster[kmax]->GetEnergy();
      fClRadiusOR[j] = ((HitClusterUCLA_t*)fCluster[kmax])->ClusterRadiusUCLA(this);
    }
  }
}


ClassImp(TA2ClusterDetector)
