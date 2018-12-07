#include "TA2GoAT.h"


TA2GoAT::TA2GoAT(const char* Name, TA2Analysis* Analysis) : TA2AccessSQL(Name, Analysis),
							    file(0),
							    treeTracks(0),
							    treeTagger(0),
							    treeLinPol(0),
							    treePairSpec(0),
							    treeTrigger(0),
							    treeDetectorHits(0),
							    treeVertex(0),
							    treeScalers(0),
							    treeMoeller(0),
							    treeSetupParameters(0),
							    nParticles(0),
							    clusterEnergy(0),
							    theta(0),
							    phi(0),
							    time(0),
							    clusterSize(0),
							    centralCrystal(0),
							    centralVeto(0),
							    detectors(0),
							    vetoEnergy(0),
							    MWPC0Energy(0),
							    MWPC1Energy(0),
							    shortEnergy(0),
							    pseudoVertexX(0),
							    pseudoVertexY(0),
							    pseudoVertexZ(0),
							    nVertex(0),
							    vertexX(0),
							    vertexY(0),
							    vertexZ(0),
							    nChamberHitsin1(0),
							    nChamberHitsin2(0),
							    MWPC0PosX(0),
							    MWPC1PosX(0),
							    MWPC0PosY(0),
							    MWPC1PosY(0),
							    MWPC0PosZ(0),
							    MWPC1PosZ(0),
							    nTagged(0),
							    taggedEnergy(0),
							    taggedChannel(0),
							    taggedTime(0),
							    saveTaggedEnergy(0),
							    plane(0),
							    edge(0),
							    edgeSetting(0),
							    nNaIADCs(0),
							    NaIADCs(0),
							    NaIADCsRaw(0),
							    NaIADCsHit(0),
							    nNaITDCs(0),
							    NaITDCs(0),
							    NaITDCsRaw(0),
							    NaITDCsHit(0),
							    nNaIHits(0),
							    NaIHits(0),
							    NaICluster(0),
							    NaIEnergy(0),
							    NaITime(0),
							    nPIDADCs(0),
							    PIDADCs(0),
							    PIDADCsRaw(0),
							    PIDADCsHit(0),
							    nPIDTDCs(0),
							    PIDTDCs(0),
							    PIDTDCsRaw(0),
							    PIDTDCsHit(0),
							    nPIDHits(0),
							    PIDHits(0),
							    PIDEnergy(0),
							    PIDTime(0),
							    nMWPCHits(0),
							    MWPCHits(0),
							    nBaF2ADCs(0),
							    BaF2ADCs(0),
							    BaF2ADCsRaw(0),
							    BaF2ADCsHit(0),
							    nBaF2TDCs(0),
							    BaF2TDCs(0),
							    BaF2TDCsRaw(0),
							    BaF2TDCsHit(0),
							    nBaF2Hits(0),
							    BaF2Hits(0),
							    BaF2Cluster(0),
							    BaF2Energy(0),
							    BaF2Time(0),
							    nVetoADCs(0),
							    VetoADCs(0),
							    VetoADCsRaw(0),
							    VetoADCsHit(0),
							    nVetoTDCs(0),
							    VetoTDCs(0),
							    VetoTDCsRaw(0),
							    VetoTDCsHit(0),
							    nVetoHits(0),
							    VetoHits(0),
							    VetoEnergy(0),
							    VetoTime(0),
							    energySum(0),
							    multiplicity(0),
							    nTriggerPattern(0),
							    triggerPattern(0),
							    nHelicityBits(0),
							    helicity(0),
							    helicityInverted(0),
							    helicityADC(0),
							    nErrors(0),
							    errorModuleID(0),
							    errorModuleIndex(0),
							    errorCode(0),
							    nChannels(0),
							    eventNumber(0),
							    eventID(0),
							    moellerRead(0),
							    moellerPairs(0),
							    MCEventID(0),
							    MCRndID(0)
{
  strcpy(outputFolder,"");
  strcpy(inputName,"");
  strcpy(fileName,"Acqu");

  AddCmdList(RootTreeConfigKeys);
}


TA2GoAT::~TA2GoAT()
{
  if(treeTracks)
    delete treeTracks;
  if(treeTagger)
    delete treeTagger;
  if(treeLinPol)
    delete treeLinPol;
  if(treePairSpec)
    delete treePairSpec;
  if(treeTrigger)
    delete treeTrigger;
  if(treeDetectorHits)
    delete treeDetectorHits;
  if(treeVertex)
    delete treeVertex;
  if(treeScalers)
    delete treeScalers;
  if(treeMoeller)
    delete treeMoeller;
  if(treeSetupParameters)
    delete treeSetupParameters;
  if(file)
    delete file;
}

void    TA2GoAT::LoadVariable()
{
  // Including histogram output for testing purposes (quick check of variables)
  TA2AccessSQL::LoadVariable();

  TA2DataManager::LoadVariable("nParticles", 	  &nParticles,   EISingleX);
  TA2DataManager::LoadVariable("clusterEnergy", clusterEnergy, EDMultiX);
  TA2DataManager::LoadVariable("theta", 		  theta,         EDMultiX);
  TA2DataManager::LoadVariable("phi", 		  phi,           EDMultiX);
  TA2DataManager::LoadVariable("time", 		  time,          EDMultiX);

  TA2DataManager::LoadVariable("nTagged", 	  &nTagged,	     EISingleX);
  TA2DataManager::LoadVariable("taggedEnergy",  taggedEnergy,  EDMultiX);
  TA2DataManager::LoadVariable("taggedChannel", taggedChannel, EIMultiX);
  TA2DataManager::LoadVariable("taggedTime", 	  taggedTime,    EDMultiX);

  TA2DataManager::LoadVariable("vetoEnergy",    vetoEnergy,    EDMultiX);
  TA2DataManager::LoadVariable("MWPC0Energy",   MWPC0Energy,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC1Energy",   MWPC1Energy,   EDMultiX);

  TA2DataManager::LoadVariable("vertexX",       vertexX,       EDMultiX);
  TA2DataManager::LoadVariable("vertexY",       vertexY,       EDMultiX);
  TA2DataManager::LoadVariable("vertexZ",       vertexZ,       EDMultiX);

  TA2DataManager::LoadVariable("energySum",     &energySum,    EDSingleX);

  TA2DataManager::LoadVariable("MWPC0PosX",   MWPC0PosX,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC1PosX",   MWPC1PosX,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC0PosY",   MWPC0PosY,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC1PosY",   MWPC1PosY,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC0PosZ",   MWPC0PosZ,   EDMultiX);
  TA2DataManager::LoadVariable("MWPC1PosZ",   MWPC1PosZ,   EDMultiX);



  return;
    
}

void    TA2GoAT::SetConfig(Char_t* line, Int_t key)
{
  switch(key)
    {
    case EG_OUTPUT_FOLDER:
      strcpy(outputFolder,line);
      while(outputFolder[strlen(outputFolder)-1]=='\n' 
	    || outputFolder[strlen(outputFolder)-1]=='\r')
	outputFolder[strlen(outputFolder)-1]='\0';
      return;
    case EG_INPUT_NAME:
      strcpy(inputName,line);
      while(inputName[strlen(inputName)-1]=='\n' 
	    || inputName[strlen(inputName)-1]=='\r')
	inputName[strlen(inputName)-1]='\0';
      return;
    case EG_FILE_NAME:
      strcpy(fileName,line);
      while(fileName[strlen(fileName)-1]=='\n' 
	    || fileName[strlen(fileName)-1]=='\r')
	fileName[strlen(fileName)-1]='\0';
      return;
    case EG_BEAM_HELICITY:
      nHelicityBits = sscanf(line, "%i%s%s%s%s%s%s%s%s", &helicityADC, helicityBits[0], helicityBits[1], helicityBits[2], helicityBits[3], helicityBits[4], helicityBits[5], helicityBits[6], helicityBits[7]);
      nHelicityBits--;
      if(nHelicityBits < 2) Error("SetConfig", "Not enough information to construct beam helicity!");
      else
	{
	  printf("Helicity");
	  for(Int_t i=0; i<nHelicityBits; i++)
	    {
	      helicityInhibit[i] = false;
	      if(!strcmp(helicityBits[i],"I") || !strcmp(helicityBits[i],"i")) helicityInhibit[i] = true;
	      else if(!strcmp(helicityBits[i],"L") || !strcmp(helicityBits[i],"l")) helicityBeam[i] = false;
	      else if(!strcmp(helicityBits[i],"H") || !strcmp(helicityBits[i],"h")) helicityBeam[i] = true;
	      printf(" - %s %i %i",helicityBits[i], helicityInhibit[i], helicityBeam[i]);
	    }
	  printf("\n");
	}
      return;
    case EG_TAGGED_ENERGY:
      sscanf(line, "%i", &saveTaggedEnergy);
      return;
    case EG_SAVE_CHANNEL:
      if(sscanf(line, "%s%s", channelIndex[nChannels], channelName[nChannels]) == 1) strcpy(channelName[nChannels],channelIndex[nChannels]);
      nChannels++;
      return;
    default:
      TA2AccessSQL::SetConfig(line, key);
    }
}

void    TA2GoAT::PostInit()
{
  clusterEnergy    = new Double_t[TA2GoAT_MAX_PARTICLE];
  theta            = new Double_t[TA2GoAT_MAX_PARTICLE];
  phi              = new Double_t[TA2GoAT_MAX_PARTICLE];
  time             = new Double_t[TA2GoAT_MAX_PARTICLE];
  clusterSize      = new Int_t[TA2GoAT_MAX_PARTICLE];
  centralCrystal   = new Int_t[TA2GoAT_MAX_PARTICLE];
  centralVeto      = new Int_t[TA2GoAT_MAX_PARTICLE];

  detectors        = new Int_t[TA2GoAT_MAX_PARTICLE];
  vetoEnergy       = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC0Energy      = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC1Energy      = new Double_t[TA2GoAT_MAX_PARTICLE];
  shortEnergy      = new Double_t[TA2GoAT_MAX_PARTICLE];

  pseudoVertexX    = new Double_t[TA2GoAT_MAX_PARTICLE];
  pseudoVertexY    = new Double_t[TA2GoAT_MAX_PARTICLE];
  pseudoVertexZ    = new Double_t[TA2GoAT_MAX_PARTICLE];

  MWPC0PosX         = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC1PosX         = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC0PosY         = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC1PosY         = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC0PosZ         = new Double_t[TA2GoAT_MAX_PARTICLE];
  MWPC1PosZ         = new Double_t[TA2GoAT_MAX_PARTICLE];

  vertexX          = new Double_t[TA2GoAT_MAX_PARTICLE];
  vertexY          = new Double_t[TA2GoAT_MAX_PARTICLE];
  vertexZ          = new Double_t[TA2GoAT_MAX_PARTICLE];

  taggedChannel    = new Int_t[TA2GoAT_MAX_TAGGER];
  taggedTime       = new Double_t[TA2GoAT_MAX_TAGGER];
  taggedEnergy     = new Double_t[TA2GoAT_MAX_TAGGER];

  NaIADCs          = new Int_t[TA2GoAT_MAX_HITS];
  NaIADCsRaw       = new Int_t[720];
  NaIADCsHit       = new Bool_t[TA2GoAT_MAX_HITS];
  NaITDCs          = new Int_t[TA2GoAT_MAX_HITS];
  NaITDCsRaw       = new Int_t[720];
  NaITDCsHit       = new Bool_t[TA2GoAT_MAX_HITS];
  NaIHits	         = new Int_t[TA2GoAT_MAX_HITS];
  NaICluster       = new Int_t[TA2GoAT_MAX_HITS];
  NaIEnergy        = new Double_t[TA2GoAT_MAX_HITS];
  NaITime          = new Double_t[TA2GoAT_MAX_HITS];
  PIDADCs          = new Int_t[TA2GoAT_MAX_HITS];
  PIDADCsRaw       = new Int_t[24];
  PIDADCsHit       = new Bool_t[TA2GoAT_MAX_HITS];
  PIDTDCs          = new Int_t[TA2GoAT_MAX_HITS];
  PIDTDCsRaw       = new Int_t[24];
  PIDTDCsHit       = new Bool_t[TA2GoAT_MAX_HITS];
  PIDHits	         = new Int_t[TA2GoAT_MAX_HITS];
  PIDEnergy        = new Double_t[TA2GoAT_MAX_HITS];
  PIDTime          = new Double_t[TA2GoAT_MAX_HITS];
  MWPCHits		 = new Int_t[TA2GoAT_MAX_HITS];
  BaF2ADCs         = new Int_t[TA2GoAT_MAX_HITS];
  BaF2ADCsRaw      = new Int_t[438];
  BaF2ADCsHit      = new Bool_t[TA2GoAT_MAX_HITS];
  BaF2TDCs         = new Int_t[TA2GoAT_MAX_HITS];
  BaF2TDCsRaw      = new Int_t[438];
  BaF2TDCsHit      = new Bool_t[TA2GoAT_MAX_HITS];
  BaF2Hits	     = new Int_t[TA2GoAT_MAX_HITS];
  BaF2Cluster      = new Int_t[TA2GoAT_MAX_HITS];
  BaF2Energy       = new Double_t[TA2GoAT_MAX_HITS];
  BaF2Time         = new Double_t[TA2GoAT_MAX_HITS];
  VetoADCs         = new Int_t[TA2GoAT_MAX_HITS];
  VetoADCsRaw      = new Int_t[438];
  VetoADCsHit      = new Bool_t[TA2GoAT_MAX_HITS];
  VetoTDCs         = new Int_t[TA2GoAT_MAX_HITS];
  VetoTDCsRaw      = new Int_t[438];
  VetoTDCsHit      = new Bool_t[TA2GoAT_MAX_HITS];
  VetoHits         = new Int_t[TA2GoAT_MAX_HITS];
  VetoEnergy       = new Double_t[TA2GoAT_MAX_HITS];
  VetoTime         = new Double_t[TA2GoAT_MAX_HITS];

  triggerPattern   = new Int_t[32];

  errorModuleID 	 = new Int_t[TA2GoAT_MAX_ERROR];
  errorModuleIndex = new Int_t[TA2GoAT_MAX_ERROR];
  errorCode        = new Int_t[TA2GoAT_MAX_ERROR];

  // Default SQL-physics initialisation
  TA2AccessSQL::PostInit();

  printf("---------\n");
  printf("Init Tree\n");
  printf("---------\n");
    
  // Append input filename to output tree name.
  TString fullName;
  if(gAR->GetProcessType() == EMCProcess) fullName = gAR->GetTreeFileList(0);        
  else  fullName = gAR->GetFileName();
		
  while(fullName.Contains("/")) fullName.Remove(0,1+fullName.Index("/"));
  fullName.ReplaceAll(".dat",".root");
  if(strlen(inputName) && fullName.BeginsWith(inputName)) fullName.Remove(0,strlen(inputName));
  else fullName.Prepend("_");
  fullName.Prepend(fileName);
  if((gAR->IsBatch() || !strcmp(outputFolder,"")) && (gAR->GetTreeDir())) strcpy(outputFolder, gAR->GetTreeDir());
  if((strlen(outputFolder)>0) && strcmp(outputFolder+strlen(outputFolder)-1,"/")) strcat(outputFolder, "/");
  fullName.Prepend(outputFolder);
  printf("Root file saved to %s\n", fullName.Data());  

  file		        = new TFile(fullName.Data(),     "RECREATE");
  treeTracks          = new TTree("tracks",            "tracks");
  treeTagger          = new TTree("tagger",            "tagger");
  treeTrigger	        = new TTree("trigger",           "trigger");
  treeDetectorHits    = new TTree("detectorHits",      "detectorHits");
  treeSetupParameters = new TTree("setupParameters",   "setupParameters");

  treeTracks->Branch("nTracks", &nParticles, "nTracks/I");
  treeTracks->Branch("clusterEnergy", clusterEnergy, "clusterEnergy[nTracks]/D");
  treeTracks->Branch("theta", theta, "theta[nTracks]/D");
  treeTracks->Branch("phi", phi, "phi[nTracks]/D");
  treeTracks->Branch("time", time, "time[nTracks]/D");
  treeTracks->Branch("clusterSize", clusterSize, "clusterSize[nTracks]/I");
  treeTracks->Branch("centralCrystal", centralCrystal, "centralCrystal[nTracks]/I");
  treeTracks->Branch("centralVeto", centralVeto, "centralVeto[nTracks]/I");
  treeTracks->Branch("detectors", detectors, "detectors[nTracks]/I");
  treeTracks->Branch("vetoEnergy", vetoEnergy, "vetoEnergy[nTracks]/D");
  treeTracks->Branch("MWPC0Energy", MWPC0Energy, "MWPC0Energy[nTracks]/D");
  treeTracks->Branch("MWPC1Energy", MWPC1Energy, "MWPC1Energy[nTracks]/D");
  treeTracks->Branch("shortEnergy", shortEnergy, "shortEnergy[nTracks]/D");
  treeTracks->Branch("pseudoVertexX", pseudoVertexX, "pseudoVertexX[nTracks]/D");
  treeTracks->Branch("pseudoVertexY", pseudoVertexY, "pseudoVertexY[nTracks]/D");
  treeTracks->Branch("pseudoVertexZ", pseudoVertexZ, "pseudoVertexZ[nTracks]/D");

  treeTracks->Branch("MWPC0PosX",MWPC0PosX, "MWPC0PosX[nTracks]/D");
  treeTracks->Branch("MWPC1PosX",MWPC1PosX, "MWPC1PosX[nTracks]/D");
  treeTracks->Branch("MWPC0PosY",MWPC0PosY, "MWPC0PosY[nTracks]/D");
  treeTracks->Branch("MWPC1PosY",MWPC1PosY, "MWPC1PosY[nTracks]/D");
  treeTracks->Branch("MWPC0PosZ",MWPC0PosZ, "MWPC0PosZ[nTracks]/D");
  treeTracks->Branch("MWPC1PosZ",MWPC1PosZ, "MWPC1PosZ[nTracks]/D");


  treeTagger->Branch("nTagged", &nTagged,"nTagged/I");
  treeTagger->Branch("taggedChannel", taggedChannel, "taggedChannel[nTagged]/I");
  treeTagger->Branch("taggedTime", taggedTime, "taggedTime[nTagged]/D");
  if(saveTaggedEnergy)
    {
      treeTagger->Branch("taggedEnergy", taggedEnergy, "taggedEnergy[nTagged]/D");
      printf("User requested tagged photon energies be exported to the tagger tree\n");
    }

  treeTrigger->Branch("energySum", &energySum, "energySum/D");
  treeTrigger->Branch("multiplicity", &multiplicity, "multiplicity/I");
  if(nHelicityBits > 1) treeTrigger->Branch("helicity", &helicity, "helicity/O");
  treeTrigger->Branch("nErrors", &nErrors, "nErrors/I");
  treeTrigger->Branch("errorModuleID", errorModuleID, "errorModuleID[nErrors]/I");
  treeTrigger->Branch("errorModuleIndex", errorModuleIndex, "errorModuleIndex[nErrors]/I");
  treeTrigger->Branch("errorCode", errorCode, "errorCode[nErrors]/I");
  treeTrigger->Branch("nTriggerPattern", &nTriggerPattern, "nTriggerPattern/I");
  treeTrigger->Branch("triggerPattern", triggerPattern, "triggerPattern[nTriggerPattern]/I");
	
  if(fNaI)
    {
      if (fNaI->IsRawHits())
        {
	  treeDetectorHits->Branch("nNaIADCs", &nNaIADCs, "nNaIADCs/I");
	  treeDetectorHits->Branch("NaIADCs", NaIADCs, "NaIADCs[nNaIADCs]/I");
	  treeDetectorHits->Branch("NaIADCsRaw", NaIADCsRaw, "NaIADCsRaw[720]/I");
	  treeDetectorHits->Branch("NaIADCsHit", NaIADCsHit, "NaIADCsHit[nNaIADCs]/O");
	  treeDetectorHits->Branch("nNaITDCs", &nNaITDCs, "nNaITDCs/I");
	  treeDetectorHits->Branch("NaITDCs", NaITDCs, "NaITDCs[nNaITDCs]/I");
	  treeDetectorHits->Branch("NaITDCsRaw", NaITDCsRaw, "NaITDCsRaw[720]/I");
	  treeDetectorHits->Branch("NaITDCsHit", NaITDCsHit, "NaITDCsHit[nNaITDCs]/O");
        }
      treeDetectorHits->Branch("nNaIHits", &nNaIHits, "nNaIHits/I");
      treeDetectorHits->Branch("NaIHits", NaIHits, "NaIHits[nNaIHits]/I");
      treeDetectorHits->Branch("NaICluster", NaICluster, "NaICluster[nNaIHits]/I");
      if(fNaI->IsEnergy()) treeDetectorHits->Branch("NaIEnergy", NaIEnergy, "NaIEnergy[nNaIHits]/D");
      if(fNaI->IsTime()) treeDetectorHits->Branch("NaITime", NaITime, "NaITime[nNaIHits]/D");
    }
  if(fPID)
    {
      if (fPID->IsRawHits())
        {
	  treeDetectorHits->Branch("nPIDADCs", &nPIDADCs, "nPIDADCs/I");
	  treeDetectorHits->Branch("PIDADCs", PIDADCs, "PIDADCs[nPIDADCs]/I");
	  treeDetectorHits->Branch("PIDADCsRaw", PIDADCsRaw, "PIDADCsRaw[24]/I");
	  treeDetectorHits->Branch("PIDADCsHit", PIDADCsHit, "PIDADCsHit[nPIDADCs]/O");
	  treeDetectorHits->Branch("nPIDTDCs", &nPIDTDCs, "nPIDTDCs/I");
	  treeDetectorHits->Branch("PIDTDCs", PIDTDCs, "PIDTDCs[nPIDTDCs]/I");
	  treeDetectorHits->Branch("PIDTDCsRaw", PIDTDCsRaw, "PIDTDCsRaw[24]/I");
	  treeDetectorHits->Branch("PIDTDCsHit", PIDTDCsHit, "PIDTDCsHit[nPIDTDCs]/O");
        }
      treeDetectorHits->Branch("nPIDHits", &nPIDHits, "nPIDHits/I");
      treeDetectorHits->Branch("PIDHits", PIDHits, "PIDHits[nPIDHits]/I");
      if(fPID->IsEnergy()) treeDetectorHits->Branch("PIDEnergy", PIDEnergy, "PIDEnergy[nPIDHits]/D");
      if(fPID->IsTime()) treeDetectorHits->Branch("PIDTime", PIDTime, "PIDTime[nPIDHits]/D");
    }
  if(fMWPC)
    {
      treeDetectorHits->Branch("nMWPCHits", &nMWPCHits, "nMWPCHits/I");
      treeDetectorHits->Branch("MWPCHits", MWPCHits, "MWPCHits[nMWPCHits]/I");
    }
  if(fBaF2PWO)
    {
      if (fBaF2PWO->IsRawHits())
        {
	  treeDetectorHits->Branch("nBaF2ADCs", &nBaF2ADCs, "nBaF2ADCs/I");
	  treeDetectorHits->Branch("BaF2ADCs", BaF2ADCs, "BaF2ADCs[nBaF2ADCs]/I");
	  treeDetectorHits->Branch("BaF2ADCsRaw", BaF2ADCsRaw, "BaF2ADCsRaw[438]/I");
	  treeDetectorHits->Branch("BaF2ADCsHit", BaF2ADCsHit, "BaF2ADCsHit[nBaF2ADCs]/O");
	  treeDetectorHits->Branch("nBaF2TDCs", &nBaF2TDCs, "nBaF2TDCs/I");
	  treeDetectorHits->Branch("BaF2TDCs", BaF2TDCs, "BaF2TDCs[nBaF2TDCs]/I");
	  treeDetectorHits->Branch("BaF2TDCsRaw", BaF2TDCsRaw, "BaF2TDCsRaw[438]/I");
	  treeDetectorHits->Branch("BaF2TDCsHit", BaF2TDCsHit, "BaF2TDCsHit[nBaF2TDCs]/O");
        }
      treeDetectorHits->Branch("nBaF2Hits", &nBaF2Hits, "nBaF2Hits/I");
      treeDetectorHits->Branch("BaF2Hits", BaF2Hits, "BaF2Hits[nBaF2Hits]/I");
      treeDetectorHits->Branch("BaF2Cluster", BaF2Cluster, "BaF2Cluster[nBaF2Hits]/I");
      if(fBaF2PWO->IsEnergy()) treeDetectorHits->Branch("BaF2Energy", BaF2Energy, "BaF2Energy[nBaF2Hits]/D");
      if(fBaF2PWO->IsTime()) treeDetectorHits->Branch("BaF2Time", BaF2Time, "BaF2Time[nBaF2Hits]/D");
    }
  if(fVeto)
    {
      if (fVeto->IsRawHits())
        {
	  treeDetectorHits->Branch("nVetoADCs", &nVetoADCs, "nVetoADCs/I");
	  treeDetectorHits->Branch("VetoADCs", VetoADCs, "VetoADCs[nVetoADCs]/I");
	  treeDetectorHits->Branch("VetoADCsRaw", VetoADCsRaw, "VetoADCsRaw[438]/I");
	  treeDetectorHits->Branch("VetoADCsHit", VetoADCsHit, "VetoADCsHit[nVetoADCs]/O");
	  treeDetectorHits->Branch("nVetoTDCs", &nVetoTDCs, "nVetoTDCs/I");
	  treeDetectorHits->Branch("VetoTDCs", VetoTDCs, "VetoTDCs[nVetoTDCs]/I");
	  treeDetectorHits->Branch("VetoTDCsRaw", VetoTDCsRaw, "VetoTDCsRaw[438]/I");
	  treeDetectorHits->Branch("VetoTDCsHit", VetoTDCsHit, "VetoTDCsHit[nVetoTDCs]/O");
        }
      treeDetectorHits->Branch("nVetoHits", &nVetoHits, "nVetoHits/I");
      treeDetectorHits->Branch("VetoHits", VetoHits, "VetoHits[nVetoHits]/I");
      if(fVeto->IsEnergy()) treeDetectorHits->Branch("VetoEnergy", VetoEnergy, "VetoEnergy[nVetoHits]/D");
      if(fVeto->IsTime()) treeDetectorHits->Branch("VetoTime", VetoTime, "VetoTime[nVetoHits]/D");
    }

  for(Int_t i=0; i<nChannels; i++) treeDetectorHits->Branch(channelName[i],&channelValue[i],Form("%s/I",channelName[i]));

  if(fNaI && fMWPC)
    {
      treeVertex = new TTree("vertex", "vertex");
      treeVertex->Branch("nVertex", &nVertex, "nVertex/I");
      treeVertex->Branch("vertexX", vertexX, "vertexX[nVertex]/D");
      treeVertex->Branch("vertexY", vertexY, "vertexY[nVertex]/D");
      treeVertex->Branch("vertexZ", vertexZ, "vertexZ[nVertex]/D");
    }


  // Recording start time of file

  Int_t timestamp = gAR->GetFileTimeEpoch();
  treeSetupParameters->Branch("TimeStamp", &timestamp, "TimeStamp/I");

  // Adding Tagger information to parameters tree

  Int_t nTagger;
  Double_t TaggerGlobalOffset;
  const Double_t* ChToE;
  Double_t BeamE;

  Double_t* TaggerTDCLoThr;
  Double_t* TaggerTDCHiThr;
  Double_t* TaggerTDCOffset;
  Double_t* TaggerElectronEnergy;
  Double_t* TaggerPhotonEnergy;
  Double_t *TaggerEnergyWidth;

  if(fTagger)
    {
      nTagger = fLadder->GetNelem();
      TaggerGlobalOffset = fLadder->GetTimeOffset();
      ChToE = fLadder->GetECalibration();
      BeamE = fTagger->GetBeamEnergy();

      TaggerTDCLoThr = new Double_t[nTagger];
      TaggerTDCHiThr = new Double_t[nTagger];
      TaggerTDCOffset = new Double_t[nTagger];
      TaggerElectronEnergy = new Double_t[nTagger];
      TaggerPhotonEnergy = new Double_t[nTagger];

      for(UInt_t i=0; i<fLadder->GetNelem(); i++)
        {
	  TaggerTDCLoThr[i] = fLadder->GetElement(i)->GetTimeLowThr();
	  TaggerTDCHiThr[i] = fLadder->GetElement(i)->GetTimeHighThr();
	  TaggerTDCOffset[i] = fLadder->GetElement(i)->GetT0();
	  TaggerElectronEnergy[i] = ChToE[i];
	  TaggerPhotonEnergy[i] = BeamE - ChToE[i];
        }
      TaggerEnergyWidth = fLadder->GetEWidth();

      treeSetupParameters->Branch("nTagger", &nTagger, "nTagger/I");
      treeSetupParameters->Branch("TaggerGlobalOffset", &TaggerGlobalOffset, "TaggerGlobalOffset/D");
      treeSetupParameters->Branch("TaggerTDCLoThr", TaggerTDCLoThr, "TaggerTDCLoThr[nTagger]/D");
      treeSetupParameters->Branch("TaggerTDCHiThr", TaggerTDCHiThr, "TaggerTDCHiThr[nTagger]/D");
      treeSetupParameters->Branch("TaggerTDCOffset", TaggerTDCOffset, "TaggerTDCOffset[nTagger]/D");
      treeSetupParameters->Branch("TaggerElectronEnergy", TaggerElectronEnergy, "TaggerElectronEnergy[nTagger]/D");
      treeSetupParameters->Branch("TaggerPhotonEnergy", TaggerPhotonEnergy, "TaggerPhotonEnergy[nTagger]/D");
      treeSetupParameters->Branch("TaggerEnergyWidth", TaggerEnergyWidth, "TaggerEnergyWidth[nTagger]/D");
    }

  // Store Scalers for non-MC process
  if (gAR->GetProcessType() != EMCProcess) 
    {
      treeScalers = new TTree("scalers", "scalers");
      treeScalers->Branch("eventNumber", &eventNumber, "eventNumber/I");
      treeScalers->Branch("eventID", &eventID, "eventID/I");
      printf("GetMaxScaler: %d\n", GetMaxScaler());
      Char_t str[256];
      sprintf(str, "scalers[%d]/i", GetMaxScaler());
      treeScalers->Branch("scalers", fScaler, str);

      // Store Lin Pol if class is active
      if(fLinPol)
	{
	  treeLinPol = new TTree("linPol", "linPol");		
	  treeLinPol->Branch("plane", &plane, "plane/I");
	  treeLinPol->Branch("edge", &edge, "edge/D");
	  treeLinPol->Branch("edgeSetting", &edgeSetting, "edgeSetting/D");
	  treeLinPol->Branch("polarizationTable", fLinPol->GetPolTable_TC(), TString::Format("polarizationTable[%i]/D", nTagger));
	  treeLinPol->Branch("enhancementTable", fLinPol->GetEnhTable_TC(), TString::Format("enhancementTable[%i]/D", nTagger));
	}

      // Store Pair Spec if class is active
      if(fPairSpec)
	{
	  treePairSpec = new TTree("pairSpec", "pairSpec");
	  treePairSpec->Branch("scalerOpen", fPairSpec->GetScalerOpen(), TString::Format("scalerOpen[%i]/D",nTagger));
	  treePairSpec->Branch("scalerGated", fPairSpec->GetScalerGated(), TString::Format("scalerGated[%i]/D",nTagger));
	  treePairSpec->Branch("scalerGatedDly", fPairSpec->GetScalerGatedDly(), TString::Format("scalerGatedDly[%i]/D",nTagger));
	}

      if(fMoeller)
        {
	  treeMoeller = new TTree("moeller", "moeller");
	  treeMoeller->Branch("eventNumber", &eventNumber, "eventNumber/I");
	  moellerRead = false;
	  moellerPairs = new UInt_t*[fMoeller->GetNPairs()];
	  Char_t moellerName[256];
	  Char_t moellerType[256];
	  for(UShort_t i=0; i<(fMoeller->GetNVuproms()); i++)
            {
	      for(UShort_t j=0; j<(fMoeller->GetNLeftChannels()); j++)
                {
		  for(UShort_t k=0; k<(fMoeller->GetNPairsPerCh()); k++)
                    {
		      for(UShort_t hel=0;hel<2;hel++)
                        {
			  UShort_t idx = (i*2*(fMoeller->GetNLeftChannels())*(fMoeller->GetNPairsPerCh()));
			  idx += (j*2*(fMoeller->GetNPairsPerCh()));
			  idx += (k*2);
			  idx += hel;
			  sprintf(moellerName, "pair_V%d_L%d_R%d_H%d", i, j, k, hel);
			  sprintf(moellerType, "pair_V%d_L%d_R%d_H%d[256]/i", i, j, k, hel);
			  moellerPairs[idx] = new UInt_t[fMoeller->GetNBins()];
			  treeMoeller->Branch(moellerName, moellerPairs[idx], moellerType);

			  for(UInt_t bin=0; bin<fMoeller->GetNBins(); bin++) moellerPairs[idx][bin] = 0;
                        }
                    }
                }
            }
        }
    }

  if (gAR->GetProcessType() == EMCProcess)
    {
      // Store MC event id for MC process
      if(EI_mc_evt_id < gAR->GetNbranch()) treeTrigger->Branch("mc_evt_id", &MCEventID, "mc_event_id/L");
      if(EI_mc_rnd_id < gAR->GetNbranch()) treeTrigger->Branch("mc_rnd_id", &MCRndID, "mc_rnd_id/L");
    }

  // Adding NaI information to parameters tree

  Int_t nNaI;
  Double_t NaIGlobalOffset;
  Double_t NaIGlobalScale;
  Int_t NaIMaxClusters;
  Double_t NaIClusterThr;

  Double_t* NaIADCLoThr;
  Double_t* NaIADCHiThr;
  Double_t* NaIADCGain;
  Double_t* NaITDCLoThr;
  Double_t* NaITDCHiThr;
  Double_t* NaITDCOffset;

  if(fNaI)
    {
      nNaI = fNaI->GetNelem();
      NaIGlobalOffset = fNaI->GetTimeOffset();
      NaIGlobalScale = fNaI->GetEnergyScale();
      NaIMaxClusters = (Int_t)fNaI->GetMaxCluster();
      NaIClusterThr = fNaI->GetClusterThreshold();

      NaIADCLoThr = new Double_t[nNaI];
      NaIADCHiThr = new Double_t[nNaI];
      NaIADCGain = new Double_t[nNaI];
      NaITDCLoThr = new Double_t[nNaI];
      NaITDCHiThr = new Double_t[nNaI];
      NaITDCOffset = new Double_t[nNaI];

      for(UInt_t i=0; i<fNaI->GetNelem(); i++)
        {
	  NaIADCLoThr[i] = fNaI->GetElement(i)->GetEnergyLowThr();
	  NaIADCHiThr[i] = fNaI->GetElement(i)->GetEnergyHighThr();
	  NaIADCGain[i] = fNaI->GetElement(i)->GetA1();
	  NaITDCLoThr[i] = fNaI->GetElement(i)->GetTimeLowThr();
	  NaITDCHiThr[i] = fNaI->GetElement(i)->GetTimeHighThr();
	  NaITDCOffset[i] = fNaI->GetElement(i)->GetT0();
        }

      treeSetupParameters->Branch("nNaI", &nNaI, "nNaI/I");
      treeSetupParameters->Branch("NaIGlobalOffset", &NaIGlobalOffset, "NaIGlobalOffset/D");
      treeSetupParameters->Branch("NaIGlobalScale", &NaIGlobalScale, "NaIGlobalScale/D");
      treeSetupParameters->Branch("NaIMaxClusters", &NaIMaxClusters, "NaIMaxClusters/I");
      treeSetupParameters->Branch("NaIClusterThr", &NaIClusterThr, "NaIClusterThr/D");
      treeSetupParameters->Branch("NaIADCLoThr", NaIADCLoThr, "NaIADCLoThr[nNaI]/D");
      treeSetupParameters->Branch("NaIADCHiThr", NaIADCHiThr, "NaIADCHiThr[nNaI]/D");
      treeSetupParameters->Branch("NaIADCGain", NaIADCGain, "NaIADCGain[nNaI]/D");
      treeSetupParameters->Branch("NaITDCLoThr", NaITDCLoThr, "NaITDCLoThr[nNaI]/D");
      treeSetupParameters->Branch("NaITDCHiThr", NaITDCHiThr, "NaITDCHiThr[nNaI]/D");
      treeSetupParameters->Branch("NaITDCOffset", NaITDCOffset, "NaITDCOffset[nNaI]/D");
    }

  // Adding PID information to parameters tree

  Int_t nPID;
  Double_t PIDGlobalOffset;

  Double_t* PIDADCLoThr;
  Double_t* PIDADCHiThr;
  Double_t* PIDADCPedestal;
  Double_t* PIDADCGain;
  Double_t* PIDTDCLoThr;
  Double_t* PIDTDCHiThr;
  Double_t* PIDTDCOffset;
  Double_t* PIDPhi;

  if(fPID)
    {
      nPID = fPID->GetNelem();
      PIDGlobalOffset = fPID->GetTimeOffset();

      PIDADCLoThr = new Double_t[nPID];
      PIDADCHiThr = new Double_t[nPID];
      PIDADCPedestal = new Double_t[nPID];
      PIDADCGain = new Double_t[nPID];
      PIDTDCLoThr = new Double_t[nPID];
      PIDTDCHiThr = new Double_t[nPID];
      PIDTDCOffset = new Double_t[nPID];
      PIDPhi = new Double_t[nPID];

      for(UInt_t i=0; i<fPID->GetNelem(); i++)
        {
	  PIDADCLoThr[i] = fPID->GetElement(i)->GetEnergyLowThr();
	  PIDADCHiThr[i] = fPID->GetElement(i)->GetEnergyHighThr();
	  PIDADCPedestal[i] = fPID->GetElement(i)->GetA0();
	  PIDADCGain[i] = fPID->GetElement(i)->GetA1();
	  PIDTDCLoThr[i] = fPID->GetElement(i)->GetTimeLowThr();
	  PIDTDCHiThr[i] = fPID->GetElement(i)->GetTimeHighThr();
	  PIDTDCOffset[i] = fPID->GetElement(i)->GetT0();
	  PIDPhi[i] = fPID->GetPosition(i)->Z();
        }

      treeSetupParameters->Branch("nPID", &nPID, "nPID/I");
      treeSetupParameters->Branch("PIDGlobalOffset", &PIDGlobalOffset, "PIDGlobalOffset/D");
      treeSetupParameters->Branch("PIDADCLoThr", PIDADCLoThr, "PIDADCLoThr[nPID]/D");
      treeSetupParameters->Branch("PIDADCHiThr", PIDADCHiThr, "PIDADCHiThr[nPID]/D");
      treeSetupParameters->Branch("PIDADCPedestal", PIDADCPedestal, "PIDADCPedestal[nPID]/D");
      treeSetupParameters->Branch("PIDADCGain", PIDADCGain, "PIDADCGain[nPID]/D");
      treeSetupParameters->Branch("PIDTDCLoThr", PIDTDCLoThr, "PIDTDCLoThr[nPID]/D");
      treeSetupParameters->Branch("PIDTDCHiThr", PIDTDCHiThr, "PIDTDCHiThr[nPID]/D");
      treeSetupParameters->Branch("PIDTDCOffset", PIDTDCOffset, "PIDTDCOffset[nPID]/D");
      treeSetupParameters->Branch("PIDPhi", PIDPhi, "PIDPhi[nPID]/D");
    }

  // Adding BaF2 information to parameters tree

  Int_t nBaF2;
  Double_t BaF2GlobalOffset;
  Double_t BaF2GlobalScale;
  Double_t BaF2Distance;
  Int_t BaF2MaxClusters;
  Double_t BaF2ClusterThr;

  Double_t* BaF2ADCLoThr;
  Double_t* BaF2ADCHiThr;
  Double_t* BaF2ADCPedestal;
  Double_t* BaF2ADCGain;
  Double_t* BaF2TDCLoThr;
  Double_t* BaF2TDCHiThr;
  Double_t* BaF2TDCOffset;
  Double_t* BaF2TDCGain;

  if(fBaF2PWO)
    {
      nBaF2 = fBaF2PWO->GetNelem();
      BaF2GlobalOffset = fBaF2PWO->GetTimeOffset();
      BaF2GlobalScale = fBaF2PWO->GetEnergyScale();
      BaF2Distance = fBaF2PWO->GetPosition(0)->Z();
      BaF2MaxClusters = (Int_t)fBaF2PWO->GetMaxCluster();
      BaF2ClusterThr = fBaF2PWO->GetClusterThreshold();

      BaF2ADCLoThr = new Double_t[nBaF2];
      BaF2ADCHiThr = new Double_t[nBaF2];
      BaF2ADCPedestal = new Double_t[nBaF2];
      BaF2ADCGain = new Double_t[nBaF2];
      BaF2TDCLoThr = new Double_t[nBaF2];
      BaF2TDCHiThr = new Double_t[nBaF2];
      BaF2TDCOffset = new Double_t[nBaF2];
      BaF2TDCGain = new Double_t[nBaF2];

      for(UInt_t i=0; i<fBaF2PWO->GetNelem(); i++)
        {
	  BaF2ADCLoThr[i] = fBaF2PWO->GetElement(i)->GetEnergyLowThr();
	  BaF2ADCHiThr[i] = fBaF2PWO->GetElement(i)->GetEnergyHighThr();
	  BaF2ADCPedestal[i] = fBaF2PWO->GetElement(i)->GetA0();
	  BaF2ADCGain[i] = fBaF2PWO->GetElement(i)->GetA1();
	  BaF2TDCLoThr[i] = fBaF2PWO->GetElement(i)->GetTimeLowThr();
	  BaF2TDCHiThr[i] = fBaF2PWO->GetElement(i)->GetTimeHighThr();
	  BaF2TDCOffset[i] = fBaF2PWO->GetElement(i)->GetT0();
	  BaF2TDCGain[i] = fBaF2PWO->GetElement(i)->GetT1();
        }

      treeSetupParameters->Branch("nBaF2", &nBaF2, "nBaF2/I");
      treeSetupParameters->Branch("BaF2GlobalOffset", &BaF2GlobalOffset, "BaF2GlobalOffset/D");
      treeSetupParameters->Branch("BaF2GlobalScale", &BaF2GlobalScale, "BaF2GlobalScale/D");
      treeSetupParameters->Branch("BaF2Distance", &BaF2Distance, "BaF2Distance/D");
      treeSetupParameters->Branch("BaF2MaxClusters", &BaF2MaxClusters, "BaF2MaxClusters/I");
      treeSetupParameters->Branch("BaF2ClusterThr", &BaF2ClusterThr, "BaF2ClusterThr/D");
      treeSetupParameters->Branch("BaF2ADCLoThr", BaF2ADCLoThr, "BaF2ADCLoThr[nBaF2]/D");
      treeSetupParameters->Branch("BaF2ADCHiThr", BaF2ADCHiThr, "BaF2ADCHiThr[nBaF2]/D");
      treeSetupParameters->Branch("BaF2ADCPedestal", BaF2ADCPedestal, "BaF2ADCPedestal[nBaF2]/D");
      treeSetupParameters->Branch("BaF2ADCGain", BaF2ADCGain, "BaF2ADCGain[nBaF2]/D");
      treeSetupParameters->Branch("BaF2TDCLoThr", BaF2TDCLoThr, "BaF2TDCLoThr[nBaF2]/D");
      treeSetupParameters->Branch("BaF2TDCHiThr", BaF2TDCHiThr, "BaF2TDCHiThr[nBaF2]/D");
      treeSetupParameters->Branch("BaF2TDCOffset", BaF2TDCOffset, "BaF2TDCOffset[nBaF2]/D");
      treeSetupParameters->Branch("BaF2TDCGain", BaF2TDCGain, "BaF2TDCGain[nBaF2]/D");
    }

  // Adding Veto information to parameters tree

  Int_t nVeto;
  Double_t VetoGlobalOffset;
  Double_t VetoDistance;

  Double_t* VetoADCLoThr;
  Double_t* VetoADCHiThr;
  Double_t* VetoADCPedestal;
  Double_t* VetoADCGain;
  Double_t* VetoTDCLoThr;
  Double_t* VetoTDCHiThr;
  Double_t* VetoTDCOffset;

  if(fVeto)
    {
      nVeto = fVeto->GetNelem();
      VetoGlobalOffset = fVeto->GetTimeOffset();
      VetoDistance = fVeto->GetPosition(0)->Z();

      VetoADCLoThr = new Double_t[nVeto];
      VetoADCHiThr = new Double_t[nVeto];
      VetoADCPedestal = new Double_t[nVeto];
      VetoADCGain = new Double_t[nVeto];
      VetoTDCLoThr = new Double_t[nVeto];
      VetoTDCHiThr = new Double_t[nVeto];
      VetoTDCOffset = new Double_t[nVeto];

      for(UInt_t i=0; i<fVeto->GetNelem(); i++)
        {
	  VetoADCLoThr[i] = fVeto->GetElement(i)->GetEnergyLowThr();
	  VetoADCHiThr[i] = fVeto->GetElement(i)->GetEnergyHighThr();
	  VetoADCPedestal[i] = fVeto->GetElement(i)->GetA0();
	  VetoADCGain[i] = fVeto->GetElement(i)->GetA1();
	  VetoTDCLoThr[i] = fVeto->GetElement(i)->GetTimeLowThr();
	  VetoTDCHiThr[i] = fVeto->GetElement(i)->GetTimeHighThr();
	  VetoTDCOffset[i] = fVeto->GetElement(i)->GetT0();
        }

      treeSetupParameters->Branch("nVeto", &nVeto, "nVeto/I");
      treeSetupParameters->Branch("VetoGlobalOffset", &VetoGlobalOffset, "VetoGlobalOffset/D");
      treeSetupParameters->Branch("VetoDistance", &VetoDistance, "VetoDistance/D");
      treeSetupParameters->Branch("VetoADCLoThr", VetoADCLoThr, "VetoADCLoThr[nVeto]/D");
      treeSetupParameters->Branch("VetoADCHiThr", VetoADCHiThr, "VetoADCHiThr[nVeto]/D");
      treeSetupParameters->Branch("VetoADCPedestal", VetoADCPedestal, "VetoADCPedestal[nVeto]/D");
      treeSetupParameters->Branch("VetoADCGain", VetoADCGain, "VetoADCGain[nVeto]/D");
      treeSetupParameters->Branch("VetoTDCLoThr", VetoTDCLoThr, "VetoTDCLoThr[nVeto]/D");
      treeSetupParameters->Branch("VetoTDCHiThr", VetoTDCHiThr, "VetoTDCHiThr[nVeto]/D");
      treeSetupParameters->Branch("VetoTDCOffset", VetoTDCOffset, "VetoTDCOffset[nVeto]/D");
    }

  treeSetupParameters->Fill();

  // Define Histograms which will be saved to root tree
  DefineHistograms();

  gROOT->cd();
	
  eventNumber	= 0;

  printf("---------\n");
  printf("Running\n");
  printf("---------\n");	

}

void    TA2GoAT::Reconstruct()
{
  // Fill standard data check histograms
  DataCheckHistograms();

  if(gAR->GetProcessType() == EMCProcess ) { 
    if(EI_mc_evt_id < gAR->GetNbranch()) MCEventID = *(Long64_t*) (fEvent[EI_mc_evt_id]);
    if(EI_mc_rnd_id < gAR->GetNbranch()) MCRndID   = *(Long64_t*) (fEvent[EI_mc_rnd_id]);
  }

  // Output scaler info on scaler read events
  if((gAR->IsScalerRead()) && (gAR->GetProcessType() != EMCProcess))
    {
      eventID	= gAN->GetNDAQEvent();
      if(treeScalers) treeScalers->Fill();
		
      if(fLinPol)
	{
	  plane 	= fLinPol->GetPolPlane();
	  edge 	= fLinPol->GetEdge();
	  edgeSetting = fLinPol->GetEdgeSetting();
	  if(treeLinPol) treeLinPol->Fill();
	}

      if(treePairSpec) treePairSpec -> Fill();

      if(fMoeller)
        {
	  if(fMoeller->IsReadoutStarted())
            {
	      moellerRead = true;
	      //printf("Moeller read started - Event %d\n",eventNumber);
            }
        }
    }

  if(fMoeller)
    {
      if(moellerRead && !fMoeller->IsReadoutStarted())
        {
	  for(Int_t i=0; i<fMoeller->GetNPairs(); i++)
            {
	      for(UInt_t j=0; j<fMoeller->GetNBins(); j++)
                {
		  moellerPairs[i][j] = fMoeller->GetTDCValue(i,j);
                }
            }
	  treeMoeller->Fill();
	  moellerRead = false;
	  //printf("Moeller read finish  - Event %d\n",eventNumber);
        }
    }

  nTagged = 0;
  if(fTagger && fLadder)
    {
      // Get the conversion of tagger channel to photon energy
      Double_t electron_E = fTagger->GetBeamEnergy();
      const Double_t* ChToE = fLadder->GetECalibration();
      Int_t fNmult = 1;
      if ( gAR->IsOnline() ) fNmult = fLadder->GetNMultihit();

      if ( fNmult <= 1 )
	{
	  // Collect Tagger Hits without Multihits
	  nTagged	= fLadder->GetNhitsAll();
	  for(Int_t i=0; i<nTagged; i++)
	    {
	      taggedChannel[i] = (fLadder->GetHitsAll())[i];
	      taggedTime[i] = (fLadder->GetTimeAll())[i];
	      taggedEnergy[i] = electron_E - ChToE[taggedChannel[i]];
	    }
	}
		
      else
	{
	  // Collect Tagger Hits with Multihits
	  for(UInt_t m=0; m<(UInt_t)fNmult; m++)
	    {
	      for(UInt_t i=0; i<fLadder->GetNhitsM(m); i++)
		{
		  taggedChannel[nTagged+i] 	= (fLadder->GetHitsM(m))[i];
		  taggedTime[nTagged+i]	= (fLadder->GetTimeORM(m))[i];
		  taggedEnergy[nTagged+i] = electron_E - ChToE[taggedChannel[nTagged+i]];
		}
	      nTagged	+= fLadder->GetNhitsM(m);
	    }
	}
    }
	
  // Gather particle information
  Int_t nCB = 0;
  if(fCB) nCB	= fCB->GetNParticle();

  Int_t nTAPS = 0;
  if(fTAPS) nTAPS = fTAPS->GetNParticle();

  nParticles = nCB + nTAPS;
  TA2Particle part;

  if(fMWPC){
    nChamberHitsin1 = fMWPC->GetNinters(0); 
    Chamber1Hits = fMWPC->GetInters(0); 
    Chamber2Hits = fMWPC->GetInters(1);
    nChamberHitsin2 = fMWPC->GetNinters(1);
  }

  for(Int_t i=0; i<nParticles; i++)
    {
      if(i < nCB) part = fCB->GetParticles(i);
      else part = fTAPS->GetParticles(i-nCB);

      part.SetParticleID(kRootino); // Set mass to 0 (rootino)
      part.SetMass(0.0);

      // Reset a bunch of inconsistant "no-value" markers
      if(TMath::Abs(part.GetT()) >= TA2GoAT_NULL) clusterEnergy[i] = 0.0;
      else clusterEnergy[i] = part.GetT();

      if(TMath::Abs(part.GetTime()) >= TA2GoAT_NULL) time[i] = 0.0;
      else time[i] = part.GetTime();

      if(TMath::Abs(part.GetVetoEnergy()) >= TA2GoAT_NULL) vetoEnergy[i] = 0.0;
      else vetoEnergy[i]	= part.GetVetoEnergy();

      if(TMath::Abs(part.GetEnergyMwpc0()) >= TA2GoAT_NULL) MWPC0Energy[i] = 0.0;
      else MWPC0Energy[i] = part.GetEnergyMwpc0();

      if(TMath::Abs(part.GetEnergyMwpc1()) >= TA2GoAT_NULL) MWPC1Energy[i] = 0.0;
      else MWPC1Energy[i] = part.GetEnergyMwpc1();

      if(TMath::Abs(part.GetPSAShort()) >= TA2GoAT_NULL) shortEnergy[i] = 0.0;
      else shortEnergy[i] = part.GetPSAShort();

      if(part.GetClusterSize() == ENullHit) clusterSize[i] = 0;
      else clusterSize[i] = part.GetClusterSize();

      if(part.GetCentralIndex() == ENullHit) centralCrystal[i] = -1;
      else centralCrystal[i] = part.GetCentralIndex();

      if(part.GetVetoIndex() == ENullHit) centralVeto[i] = -1;
      else centralVeto[i]	= part.GetVetoIndex();


      MWPC0PosX[i]=0;
      MWPC1PosX[i]=0;
      MWPC0PosY[i]=0;
      MWPC1PosY[i]=0;
      MWPC0PosZ[i]=0;
      MWPC1PosZ[i]=0;

      if(fMWPC){
	if(part.GetTrackIntersect(0)>=0){
	  MWPC0PosX[i]=(*(Chamber1Hits[part.GetTrackIntersect(0)].GetPosition())).X();
	  MWPC0PosY[i]=(*(Chamber1Hits[part.GetTrackIntersect(0)].GetPosition())).Y();
	  MWPC0PosZ[i]=(*(Chamber1Hits[part.GetTrackIntersect(0)].GetPosition())).Z();
	}
	if(part.GetTrackIntersect(1)>=0){
	  MWPC1PosX[i]=(*(Chamber2Hits[part.GetTrackIntersect(1)].GetPosition())).X();
	  MWPC1PosY[i]=(*(Chamber2Hits[part.GetTrackIntersect(1)].GetPosition())).Y();
	  MWPC1PosZ[i]=(*(Chamber2Hits[part.GetTrackIntersect(1)].GetPosition())).Z();
	}
      }
      // Store other values which don't have this "no-value" option
      detectors[i]	= part.GetDetectors();
      theta[i]		= part.GetThetaDg();
      phi[i]			= part.GetPhiDg();
      pseudoVertexX[i] = part.GetPsVertex().X();
      pseudoVertexY[i] = part.GetPsVertex().Y();
      pseudoVertexZ[i] = part.GetPsVertex().Z();

    }

  UInt_t *clhits;
  HitCluster_t *cl;
  UInt_t *hits;
  Int_t clindex[720];
  Int_t *rhits;

  // Get Detector Hits
  if(fNaI)
    {
      for(Int_t i=0; i<720; i++)
	{
	  clindex[i] = -1;
	}
      clhits = fNaI->GetClustHit();
      for(UInt_t i=0; i<fNaI->GetNCluster(); i++)
	{
	  cl = fNaI->GetCluster(clhits[i]);
	  hits = cl->GetHits();
	  for(UInt_t j=0; j<(cl->GetNhits()); j++)
	    {
	      clindex[hits[j]] = i;
	    }
	}
			
      if(fNaI->IsRawHits())
        {
	  nNaIADCs = fNaI->GetNADChits();
	  rhits = fNaI->GetRawEnergyHits();
	  for(Int_t i=0; i<720; i++) NaIADCsRaw[i] = 0;
	  for(Int_t i=0; i<nNaIADCs; i++)
            {
	      NaIADCs[i] = rhits[i];
	      NaIADCsRaw[NaIADCs[i]] = (Int_t)fNaI->GetElement(NaIADCs[i])->GetRawADCValue();
	      NaIADCsHit[i] = fNaI->GetElement(NaIADCs[i])->CheckEnergy();
            }
	  nNaITDCs = fNaI->GetNTDChits();
	  rhits = fNaI->GetRawTimeHits();
	  for(Int_t i=0; i<720; i++) NaITDCsRaw[i] = 0;
	  for(Int_t i=0; i<nNaITDCs; i++)
            {
	      NaITDCs[i] = rhits[i];
	      NaITDCsRaw[NaITDCs[i]] = (Int_t)fNaI->GetElement(NaITDCs[i])->GetRawTDCValue();
	      NaITDCsHit[i] = fNaI->GetElement(NaITDCs[i])->CheckTime();
            }
        }
      nNaIHits = fNaI->GetNhits();
      for(Int_t i=0; i<nNaIHits; i++)
	{
	  NaIHits[i] = fNaI->GetHits(i);
	  NaICluster[i] = clindex[NaIHits[i]];
	  if(fNaI->IsEnergy()) NaIEnergy[i] = fNaI->GetEnergy(NaIHits[i]);
	  if(fNaI->IsTime()) NaITime[i] = fNaI->GetTime(NaIHits[i]);
        }
    }

  if(fPID)
    {
      if(fPID->IsRawHits())
        {
	  nPIDADCs = fPID->GetNADChits();
	  rhits = fPID->GetRawEnergyHits();
	  for(Int_t i=0; i<24; i++) PIDADCsRaw[i] = 0;
	  for(Int_t i=0; i<nPIDADCs; i++)
            {
	      PIDADCs[i] = rhits[i];
	      PIDADCsRaw[PIDADCs[i]] = (Int_t)fPID->GetElement(PIDADCs[i])->GetRawADCValue();
	      PIDADCsHit[i] = fPID->GetElement(PIDADCs[i])->CheckEnergy();
            }
	  nPIDTDCs = fPID->GetNTDChits();
	  rhits = fPID->GetRawTimeHits();
	  for(Int_t i=0; i<24; i++) PIDTDCsRaw[i] = 0;
	  for(Int_t i=0; i<nPIDTDCs; i++)
            {
	      PIDTDCs[i] = rhits[i];
	      PIDTDCsRaw[PIDTDCs[i]] = (Int_t)fPID->GetElement(PIDTDCs[i])->GetRawTDCValue();
	      PIDTDCsHit[i] = fPID->GetElement(PIDTDCs[i])->CheckTime();
            }
        }
      nPIDHits = fPID->GetNhits();
      for(Int_t i=0; i<nPIDHits; i++)
        {
	  PIDHits[i] = fPID->GetHits(i);
	  if(fPID->IsEnergy()) PIDEnergy[i] = fPID->GetEnergy(PIDHits[i]);
	  if(fPID->IsTime()) PIDTime[i] = fPID->GetTime(PIDHits[i]);
        }
    }

  if(fMWPC)
    {
      nMWPCHits = fMWPC->GetNhits();
      for(Int_t i=0; i<nMWPCHits; i++)
        { MWPCHits[i] = fMWPC->GetHits(i); }
    }

  if(fBaF2PWO)
    {
      for(Int_t i=0; i<438; i++)
	{
	  clindex[i] = -1;
	}
      clhits = fBaF2PWO->GetClustHit();
      for(UInt_t i=0; i<fBaF2PWO->GetNCluster(); i++)
	{
	  cl = fBaF2PWO->GetCluster(clhits[i]);
	  hits = cl->GetHits();
	  for(UInt_t j=0; j<(cl->GetNhits()); j++)
	    {
	      clindex[hits[j]] = i;
	    }
	}
			
      if(fBaF2PWO->IsRawHits())
        {
	  nBaF2ADCs = fBaF2PWO->GetNADChits();
	  rhits = fBaF2PWO->GetRawEnergyHits();
	  for(Int_t i=0; i<438; i++) BaF2ADCsRaw[i] = 0;
	  for(Int_t i=0; i<nBaF2ADCs; i++)
            {
	      BaF2ADCs[i] = rhits[i];
	      BaF2ADCsRaw[BaF2ADCs[i]] = (Int_t)fBaF2PWO->GetElement(BaF2ADCs[i])->GetRawADCValue();
	      BaF2ADCsHit[i] = fBaF2PWO->GetElement(BaF2ADCs[i])->CheckEnergy();
            }
	  nBaF2TDCs = fBaF2PWO->GetNTDChits();
	  rhits = fBaF2PWO->GetRawTimeHits();
	  for(Int_t i=0; i<438; i++) BaF2TDCsRaw[i] = 0;
	  for(Int_t i=0; i<nBaF2TDCs; i++)
            {
	      BaF2TDCs[i] = rhits[i];
	      BaF2TDCsRaw[BaF2TDCs[i]] = (Int_t)fBaF2PWO->GetElement(BaF2TDCs[i])->GetRawTDCValue();
	      BaF2TDCsHit[i] = fBaF2PWO->GetElement(BaF2TDCs[i])->CheckTime();
            }
        }
      nBaF2Hits = fBaF2PWO->GetNhits();
      for(Int_t i=0; i<nBaF2Hits; i++)
	{
	  BaF2Hits[i] = fBaF2PWO->GetHits(i);
	  BaF2Cluster[i] = clindex[fBaF2PWO->GetHits(i)];
	  if(fBaF2PWO->IsEnergy()) BaF2Energy[i] = fBaF2PWO->GetLGEnergy(fBaF2PWO->GetHits(i));
	  if(fBaF2PWO->IsTime()) BaF2Time[i] = fBaF2PWO->GetTime(fBaF2PWO->GetHits(i));
        }
    }

  if(fVeto)
    {
      if(fVeto->IsRawHits())
        {
	  nVetoADCs = fVeto->GetNADChits();
	  rhits = fVeto->GetRawEnergyHits();
	  for(Int_t i=0; i<438; i++) VetoADCsRaw[i] = 0;
	  for(Int_t i=0; i<nVetoADCs; i++)
            {
	      VetoADCs[i] = rhits[i];
	      VetoADCsRaw[VetoADCs[i]] = (Int_t)fVeto->GetElement(VetoADCs[i])->GetRawADCValue();
	      VetoADCsHit[i] = fVeto->GetElement(VetoADCs[i])->CheckEnergy();
            }
	  nVetoTDCs = fVeto->GetNTDChits();
	  rhits = fVeto->GetRawTimeHits();
	  for(Int_t i=0; i<438; i++) VetoTDCsRaw[i] = 0;
	  for(Int_t i=0; i<nVetoTDCs; i++)
            {
	      VetoTDCs[i] = rhits[i];
	      VetoTDCsRaw[VetoTDCs[i]] = (Int_t)fVeto->GetElement(VetoTDCs[i])->GetRawTDCValue();
	      VetoTDCsHit[i] = fVeto->GetElement(VetoTDCs[i])->CheckTime();
            }
        }
      nVetoHits = fVeto->GetNhits();
      for(Int_t i=0; i<nVetoHits; i++)
        {
	  VetoHits[i] = fVeto->GetHits(i);
	  if(fVeto->IsEnergy()) VetoEnergy[i] = fVeto->GetEnergy(VetoHits[i]);
	  if(fVeto->IsTime()) VetoTime[i] = fVeto->GetTime(VetoHits[i]);
        }
    }

  for(Int_t i=0; i<nChannels; i++)
    {
      TString index = channelIndex[i];
      if(index.Contains("M"))
        {
	  TString mult = channelIndex[i];
	  index.Remove(index.First("M"));
	  mult.Remove(0,mult.First("M")+1);
	  channelValue[i] = fMulti[index.Atoi()]->GetHit(mult.Atoi());
        }
      else channelValue[i] = fADC[index.Atoi()];
    }

  // Get two track vertex information from CentralApparatus
  if(fNaI && fMWPC)
    {
      nVertex = fCB->GetNvertexes();
      const TVector3 *vertex = fCB->GetVertexes();
      for(Int_t i=0; i<nVertex; i++)
        {
	  vertexX[i] = vertex[i].X();
	  vertexY[i] = vertex[i].Y();
	  vertexZ[i] = vertex[i].Z();
        }
    }
	
  // Get Trigger information
  TriggerReconstruction();

  nErrors = gAR->GetHardError();
  ReadErrorMk2_t *ErrorBlock = gAR->GetHardwareError();
  ReadErrorMk2_t *Error;
  for(Int_t i=0; i<nErrors; i++)
    {
      Error = ErrorBlock + i;
      errorModuleID[i] = Error->fModID;
      errorModuleIndex[i] = Error->fModIndex;
      errorCode[i] = Error->fErrCode;
    }

  if(nHelicityBits > 1)
    {
      Bool_t helicityBit;
      helicity = true;
      helicityInverted = true;
      for(Int_t i=0; i<nHelicityBits; i++)
	{
	  helicityBit = (fADC[helicityADC] & 1<<i);
	  if(helicityInhibit[i] && helicityBit)
	    {
	      errorCode[nErrors] = 9;
	      nErrors++;
	      break;
	    }
	  else if(helicityInhibit[i]) continue;
	  helicity = (helicity && (helicityBeam[i] == helicityBit));
	  helicityInverted = (helicityInverted && (helicityBeam[i] != helicityBit));
	  if(helicity == helicityInverted)
	    {
	      errorCode[nErrors] = 10;
	      nErrors++;
	      break;
	    }
	}
    } 

  if(fMulti[400])
    {
      Int_t eventIDcheck = fMulti[400]->GetHit(0);
      for(Int_t i=1; i<23; i++)
	{
	  if(eventIDcheck != fMulti[400]->GetHit(i))
	    {
	      errorCode[nErrors] = 11;
	      nErrors++;
	      break;
	    }
	}
    }


  //Apply EndBuffer
  clusterEnergy[nParticles] = EBufferEnd;
  theta[nParticles]         = EBufferEnd;
  phi[nParticles]           = EBufferEnd;
  time[nParticles]          = EBufferEnd;
  clusterSize[nParticles]   = EBufferEnd;
  centralCrystal[nParticles]= EBufferEnd;
  centralVeto[nParticles]   = EBufferEnd;
  detectors[nParticles] 	  = EBufferEnd;
  vetoEnergy[nParticles] 	  = EBufferEnd;
  MWPC0Energy[nParticles]   = EBufferEnd;
  MWPC1Energy[nParticles]   = EBufferEnd;
  shortEnergy[nParticles]   = EBufferEnd;
  pseudoVertexX[nParticles] = EBufferEnd;
  pseudoVertexY[nParticles] = EBufferEnd;
  pseudoVertexZ[nParticles] = EBufferEnd;

  vertexX[nVertex]          = EBufferEnd;
  vertexY[nVertex]          = EBufferEnd;
  vertexZ[nVertex]          = EBufferEnd;

  taggedChannel[nTagged] 	  = EBufferEnd;
  taggedTime[nTagged] 	  = EBufferEnd;
  taggedEnergy[nTagged] 	  = EBufferEnd;

  NaIADCs[nNaIADCs] 	      = EBufferEnd;
  NaIADCsHit[nNaIADCs]      = EBufferEnd;
  NaITDCs[nNaITDCs] 	      = EBufferEnd;
  NaITDCsHit[nNaITDCs]      = EBufferEnd;
  NaIHits[nNaIHits] 	      = EBufferEnd;
  NaICluster[nNaIHits] 	  = EBufferEnd;
  NaIEnergy[nNaIHits] 	  = EBufferEnd;
  NaITime[nNaIHits]         = EBufferEnd;
  PIDADCs[nPIDADCs] 	      = EBufferEnd;
  PIDADCsHit[nPIDADCs]      = EBufferEnd;
  PIDTDCs[nPIDTDCs] 	      = EBufferEnd;
  PIDTDCsHit[nPIDTDCs]      = EBufferEnd;
  PIDHits[nPIDHits] 	      = EBufferEnd;
  PIDEnergy[nPIDHits] 	  = EBufferEnd;
  PIDTime[nPIDHits]         = EBufferEnd;
  MWPCHits[nMWPCHits] 	  = EBufferEnd;
  BaF2ADCs[nBaF2ADCs]       = EBufferEnd;
  BaF2ADCsHit[nBaF2ADCs]    = EBufferEnd;
  BaF2TDCs[nBaF2TDCs] 	  = EBufferEnd;
  BaF2TDCsHit[nBaF2TDCs]    = EBufferEnd;
  BaF2Hits[nBaF2Hits] 	  = EBufferEnd;
  BaF2Cluster[nBaF2Hits] 	  = EBufferEnd;
  BaF2Energy[nBaF2Hits] 	  = EBufferEnd;
  BaF2Time[nBaF2Hits]       = EBufferEnd;
  VetoADCs[nVetoADCs] 	  = EBufferEnd;
  VetoADCsHit[nVetoADCs]    = EBufferEnd;
  VetoTDCs[nVetoTDCs] 	  = EBufferEnd;
  VetoTDCsHit[nVetoTDCs]    = EBufferEnd;
  VetoHits[nVetoHits] 	  = EBufferEnd;
  VetoEnergy[nVetoHits] 	  = EBufferEnd;
  VetoTime[nVetoHits]       = EBufferEnd;

  errorModuleID[nErrors] 	  = EBufferEnd;
  errorModuleIndex[nErrors] = EBufferEnd;
  errorCode[nErrors] 	      = EBufferEnd;

  //Fill Trees
  if(treeTracks) 	treeTracks->Fill();
  if(treeTagger)			treeTagger->Fill();
  if(treeTrigger)  		treeTrigger->Fill();
  if(treeDetectorHits)	treeDetectorHits->Fill();
  if(treeVertex)          treeVertex->Fill();

  //increment event number
  eventNumber++;	
}

void	TA2GoAT::DefineHistograms()
{
  // Define new data check histograms
  Check_CBdE_E		= new TH2F("Check_CBdE_E", "dE_E (all CB clusters compared to PID hits)", 	400, 0, 400, 100, 0, 10);
  Check_CBPhiCorr 	= new TH2F("Check_CBPhiCorr","PID-NaI phi correlation (all CB clusters compared to PID hits)", 24,  0,  24, 180, -180, 180);

  Check_CBdE_E_1PID 	 = new TH2F("Check_CBdE_E_1PID", "dE_E (all CB clusters compared to single PID hits)", 	400, 0, 400, 100, 0, 10);
  Check_CBPhiCorr_1PID = new TH2F("Check_CBPhiCorr_1PID","PID-NaI phi correlation (all CB clusters compared to single PID hits)", 24,  0,  24, 180, -180, 180);

  Check_CBdE_E_pi0 	= new TH2F("Check_CBdE_E_pi0", 		"dE_E after pi0 identification", 	400, 0, 400, 100, 0, 10);
  Check_CBPhiCorr_pi0 = new TH2F("Check_CBPhiCorr_pi0",	"PID-NaI phi correlation after pi0 identification", 24,  0,  24, 180, -180, 180);

  Check_TAPSdE_E		= new TH2F("Check_TAPSdE_E", "dE_E (all TAPS clusters compared to Veto hits)", 	400, 0, 400, 100, 0, 10);
  Check_TAPSPhiCorr 	= new TH2F("Check_TAPSPhiCorr","TAPS-Veto phi correlation (all TAPS clusters compared to Veto hits)", 438,  0,  438, 180, -180, 180);

  Check_TAPSdE_E_1Veto	= new TH2F("Check_TAPSdE_E_1Veto", "dE_E (all TAPS clusters compared to single Veto hits)", 	400, 0, 400, 100, 0, 10);
  Check_TAPSPhiCorr_1Veto = new TH2F("Check_TAPSPhiCorr_1Veto","TAPS-Veto phi correlation (all TAPS clusters compared to single Veto hits)", 438,  0,  438, 180, -180, 180);

  Check_CBHits 		= new TH2F("Check_CBHits", 		"CB Hits by event number", 		10000,  0,  10000000, 720, 0, 720);	
  Check_CBADCHits 	= new TH2F("Check_CBADCHits", 	"CB ADC Hits by event number", 	10000,  0,  10000000, 720, 0, 720);
  Check_CBTDCHits 	= new TH2F("Check_CBTDCHits",	"CB TDC Hits by event number", 	10000,  0,  10000000, 720, 0, 720);

  Check_PIDHits 		= new TH2F("Check_PIDHits", 	"PID Hits by event number", 	10000,  0,  10000000, 24, 0, 24);	
  Check_PIDADCHits 	= new TH2F("Check_PIDADCHits", 	"PID ADC Hits by event number", 10000,  0,  10000000, 24, 0, 24);
  Check_PIDTDCHits 	= new TH2F("Check_PIDTDCHits",	"PID TDC Hits by event number", 10000,  0,  10000000, 24, 0, 24);

  Check_TAPSHits 		= new TH2F("Check_TAPSHits", 	"TAPS Hits by event number", 	10000,  0,  10000000, 438, 0, 438);	
  Check_TAPSADCHits 	= new TH2F("Check_TAPSADCHits", "TAPS ADC Hits by event number",10000,  0,  10000000, 438, 0, 438);
  Check_TAPSTDCHits 	= new TH2F("Check_TAPSTDCHits", "TAPS TDC Hits by event number",10000,  0,  10000000, 438, 0, 438);

  Check_VetoHits 		= new TH2F("Check_VetoHits", 	"Veto Hits by event number", 	10000,  0,  10000000, 438, 0, 438);	
  Check_VetoADCHits 	= new TH2F("Check_VetoADCHits", "Veto ADC Hits by event number",10000,  0,  10000000, 438, 0, 438);
  Check_VetoTDCHits 	= new TH2F("Check_VetoTDCHits", "Veto TDC Hits by event number",10000,  0,  10000000, 438, 0, 438);
	
}

void	TA2GoAT::WriteHistograms()
{
  // Write check histograms
  file->mkdir("CheckCB");
  file->cd("CheckCB");
  Check_CBdE_E->Write();
  Check_CBPhiCorr->Write();	
	
  Check_CBdE_E_1PID->Write();
  Check_CBPhiCorr_1PID->Write();

  Check_CBdE_E_pi0->Write();
  Check_CBPhiCorr_pi0->Write();

  file->cd();
  file->mkdir("CheckTAPS");
  file->cd("CheckTAPS");
  Check_TAPSdE_E->Write();
  Check_TAPSPhiCorr->Write();	
	
  Check_TAPSdE_E_1Veto->Write();
  Check_TAPSPhiCorr_1Veto->Write();
	
  file->cd();
  file->mkdir("CheckHitPatterns");
  file->cd("CheckHitPatterns");	
  // Zoom in on all the plots with eventNumber on x-Axis
  Check_CBHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_CBHits->Write();
	
  Check_CBADCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_CBADCHits->Write();
	
  Check_CBTDCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_CBTDCHits->Write();

  Check_PIDHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_PIDHits->Write();
	
  Check_PIDADCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_PIDADCHits->Write();
	
  Check_PIDTDCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_PIDTDCHits->Write();

  Check_TAPSHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_TAPSHits->Write();
	
  Check_TAPSADCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_TAPSADCHits->Write();
	
  Check_TAPSTDCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_TAPSTDCHits->Write();	
	
  Check_VetoHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_VetoHits->Write();
	
  Check_VetoADCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_VetoADCHits->Write();
	
  Check_VetoTDCHits->GetXaxis()->SetRangeUser(0,eventNumber);
  Check_VetoTDCHits->Write();		

  file->cd();
}

void 	TA2GoAT::DataCheckHistograms()
{
	
  if((fCB) && (fPID))
    {
      // Find the charged particle (if possible) and fill dE_E for this
      // Fill the charged particle phi against the PID hit
      for(Int_t i=0; i<fCB->GetNParticle(); i++)
	{
	  TA2Particle part = fCB->GetParticles(i);

	  if (part.GetVetoEnergy() < 0)  continue;
	  if (part.GetVetoEnergy() > 10) continue;			

	  for(UInt_t j=0; j<fPID->GetNhits(); j++)
	    {
	      Check_CBdE_E->Fill(part.GetT(),part.GetVetoEnergy());
	      Check_CBPhiCorr->Fill(fPID->GetHits(j),part.GetPhiDg());
	    }

	  if(fPID->GetNhits() == 1)
	    {	
	      Check_CBdE_E_1PID->Fill(part.GetT(),part.GetVetoEnergy());
	      Check_CBPhiCorr_1PID->Fill(fPID->GetHits(0),part.GetPhiDg());	
	    }
	}		
		
      // Do basic pi0 analysis on events with 3 NaI cluster and 1 PID hit
      // Fill dE_E for charged particle if pi0 reconstructed
      // Fill associated NaI cluster phi against single PID element		
      if( (fCB->GetNParticle() == 3) && (fPID->GetNhits() == 1) )	
	{
	  // Get 3 particles, check for a pi0 from possible pairs
	  TA2Particle part1 = fCB->GetParticles(0);
	  TA2Particle part2 = fCB->GetParticles(1);
	  TA2Particle part3 = fCB->GetParticles(2);
			
	  TLorentzVector p4_12 = part1.GetP4() + part2.GetP4();
	  TLorentzVector p4_13 = part1.GetP4() + part3.GetP4();
	  TLorentzVector p4_23 = part2.GetP4() + part3.GetP4();
			
	  // Calculated IM width/20 MeV (acts as a rough IM cut)
	  Double_t IM_12 = TMath::Abs(135 - p4_12.M())/20;
	  Double_t IM_13 = TMath::Abs(135 - p4_13.M())/20;
	  Double_t IM_23 = TMath::Abs(135 - p4_23.M())/20;
			
	  if((IM_12 <= 1.0) && (part3.GetVetoEnergy() > 0) && (part3.GetVetoEnergy() < 10))
	    {
	      Check_CBdE_E_pi0->Fill(part3.GetT(),part3.GetVetoEnergy());
	      Check_CBPhiCorr_pi0->Fill(fPID->GetHits(0),part3.GetPhiDg());				
	    }
	  if((IM_13 <= 1.0) && (part2.GetVetoEnergy() > 0) && (part2.GetVetoEnergy() < 10))
	    {
	      Check_CBdE_E_pi0->Fill(part2.GetT(),part2.GetVetoEnergy());
	      Check_CBPhiCorr_pi0->Fill(fPID->GetHits(0),part2.GetPhiDg());				
	    }			
	  if((IM_23 <= 1.0) && (part1.GetVetoEnergy() > 0) && (part1.GetVetoEnergy() < 10))
	    {
	      Check_CBdE_E_pi0->Fill(part1.GetT(),part1.GetVetoEnergy());
	      Check_CBPhiCorr_pi0->Fill(fPID->GetHits(0),part1.GetPhiDg());				
	    }			
			
	}
    }

  if((fTAPS) && (fVeto))
    {
      // Find the charged particle (if possible) and fill dE_E for this
      // Fill the charged particle phi against the Veto hit
      for(Int_t i=0; i<fTAPS->GetNParticle(); i++)
	{
	  TA2Particle part = fTAPS->GetParticles(i);

	  if (part.GetVetoEnergy() < 0)  continue;
	  if (part.GetVetoEnergy() > 10) continue;			

	  for(UInt_t j=0; j<fVeto->GetNhits(); j++)
	    {
	      Check_TAPSdE_E->Fill(part.GetT(),part.GetVetoEnergy());
	      Check_TAPSPhiCorr->Fill(fVeto->GetHits(j),part.GetPhiDg());
	    }

	  if(fVeto->GetNhits() == 1)
	    {	
	      Check_TAPSdE_E_1Veto->Fill(part.GetT(),part.GetVetoEnergy());
	      Check_TAPSPhiCorr_1Veto->Fill(fVeto->GetHits(0),part.GetPhiDg());	
	    }
	}
    }
	
  // Fill CB stability histograms
  if(fNaI)
    {
      for(UInt_t i=0; i<fNaI->GetNhits(); i++)   
	{ Check_CBHits->Fill(eventNumber,fNaI->GetHits(i)); }
			
      if (fNaI->IsRawHits())
	{
	  for (UInt_t i=0; i< fNaI->GetNADChits(); i++)
	    { Check_CBADCHits->Fill(eventNumber,fNaI->GetRawEnergyHits()[i]); }	
				
	  for (UInt_t i=0; i< fNaI->GetNTDChits(); i++)
	    { Check_CBTDCHits->Fill(eventNumber,fNaI->GetRawTimeHits()[i]);	}	
							
	}
    }
	
  // Fill PID stability histograms
  if(fPID)
    {
      for(UInt_t i=0; i<fPID->GetNhits(); i++)   
	{ Check_PIDHits->Fill(eventNumber,fPID->GetHits(i)); }
			
      if (fPID->IsRawHits())
	{
	  for (UInt_t i=0; i< fPID->GetNADChits(); i++)
	    { Check_PIDADCHits->Fill(eventNumber,fPID->GetRawEnergyHits()[i]); }	
				
	  for (UInt_t i=0; i< fPID->GetNTDChits(); i++)
	    { Check_PIDTDCHits->Fill(eventNumber,fPID->GetRawTimeHits()[i]);	}	
							
	}
    }	
	
  // Fill TAPS stability histograms
  if(fBaF2PWO)
    {
      for(UInt_t i=0; i<fBaF2PWO->GetNhits(); i++)   
	{ Check_TAPSHits->Fill(eventNumber,fBaF2PWO->GetHits(i)); }
			
      if (fBaF2PWO->IsRawHits())
	{
	  if (!(fADC[0] & 1<<15)) // Ignore TAPS Pulser reads
	    {			
	      for (UInt_t i=0; i< fBaF2PWO->GetNADChits(); i++)
		{ Check_TAPSADCHits->Fill(eventNumber,fBaF2PWO->GetRawEnergyHits()[i]); }	
					
	      for (UInt_t i=0; i< fBaF2PWO->GetNTDChits(); i++)
		{ Check_TAPSTDCHits->Fill(eventNumber,fBaF2PWO->GetRawTimeHits()[i]);	}	
	    }
	}
    }	
	
  // Fill PID stability histograms
  if(fVeto)
    {
      for(UInt_t i=0; i<fVeto->GetNhits(); i++)   
	{ Check_VetoHits->Fill(eventNumber,fVeto->GetHits(i)); }
			
      if (fVeto->IsRawHits())
	{
	  if (!(fADC[0] & 1<<15)) // Ingore TAPS Pulser reads
	    {
	      for (UInt_t i=0; i< fVeto->GetNADChits(); i++)
		{ Check_VetoADCHits->Fill(eventNumber,fVeto->GetRawEnergyHits()[i]); }	

	      for (UInt_t i=0; i< fVeto->GetNTDChits(); i++)
		{ Check_VetoTDCHits->Fill(eventNumber,fVeto->GetRawTimeHits()[i]);	}	
	    }			
	}
    }
			
}

void    TA2GoAT::Finish()
{
  printf("------------------\n");
  printf("Write Tree to file\n");
  printf("------------------\n");
	
  file->cd();
	
  if(treeTracks)
    {
      treeTracks->Write();	// Write
      delete treeTracks; 	// Close and delete in memory
    }
  if(treeTagger) 
    {
      treeTagger->Write();	// Write	
      delete treeTagger; 	// Close and delete in memory
    }	
  if(treeLinPol) 
    {
      treeLinPol->Write();	// Write	
      delete treeLinPol; 		// Close and delete in memory
    }
  if(treePairSpec) 
    {
      treePairSpec->Write();	// Write	
      delete treePairSpec; 		// Close and delete in memory
    }	
  if(treeTrigger) 
    {
      treeTrigger->Write();	// Write	
      delete treeTrigger; 	// Close and delete in memory
    }		
  if(treeDetectorHits) 
    {
      treeDetectorHits->Write();// Write	
      delete treeDetectorHits;  // Close and delete in memory
    }
  if(treeVertex)
    {
      treeVertex->Write();    // Write
      delete treeVertex;      // Close and delete in memory
    }
  if(treeScalers)
    {
      treeScalers->Write();	// Write
      delete treeScalers; 	// Close and delete in memory
    }
  if(treeMoeller)
    {
      treeMoeller->Write();	// Write
      delete treeMoeller; 	// Close and delete in memory
    }
  if(treeSetupParameters)
    {
      treeSetupParameters->Write();	// Write
      delete treeSetupParameters; 	// Close and delete in memory
    }

  WriteHistograms();
	
  if(file) 
    delete file;		// Close and delete in memory

	
  TA2AccessSQL::Finish();
}

void    TA2GoAT::ParseMisc(char* line)
{
  TA2AccessSQL::ParseMisc(line);
}

void 	TA2GoAT::TriggerReconstruction()
{
  if (fNaI) energySum = fNaI->GetTotalEnergy();
  if(gAR->GetProcessType() == EMCProcess) TriggerMC();
  else TriggerHW();

	
}

void 	TA2GoAT::TriggerMC() 
{
  // really rough, just the basic idea 
  // A good example is done in TA2BasePhysics but requires some extra work
  // Also need some flag for new and old 
  // Set some basic discriminator thresh for now
  Double_t DiscTh = 5.0; 
  multiplicity = 0;
		
  if(fNaI) 
    {
      for (Int_t i = 0; i < 45; i++) 
	{ 
	  Bool_t flag = kFALSE;
	  for (Int_t j = 0; j < 16; j++) 
	    {
	      if ((fNaI->GetEnergyAll(i*16 + j)) >= DiscTh) flag = kTRUE;
	    }
	  if (flag == kTRUE) multiplicity++;
	}
    }
		
  if (fBaF2PWO) {
    for (Int_t i = 0; i < 6; i++) { 
      Bool_t flag = kFALSE;
      for (Int_t j = 12; j < 71; j++) {  
	// really add some check of how many crystals are used, skip PbWO4s
	if ((fBaF2PWO->GetEnergyAll(i*71 + j)) >= DiscTh) flag = kTRUE;
      }
      if (flag == kTRUE) multiplicity++;
    }
  }
		
  // True Hardware multiplicities store only M2, M3, M4+
  //  Reduce MC multiplicity to reflect this limitation
  if (multiplicity == 1) multiplicity = 0;
  if (multiplicity > 4) multiplicity = 4;
}
	
void 	TA2GoAT::TriggerHW() 
{
  nTriggerPattern = 0;
  for (Int_t i= 0; i < 16; i++)
    {
      if (fADC[0] & 1<<i) 
	{ 
	  triggerPattern[nTriggerPattern] = i;
	  nTriggerPattern++;
	}
      if (fADC[1] & 1<<i) 
	{ 
	  triggerPattern[nTriggerPattern] = i+16;
	  nTriggerPattern++;
	}
    }
	
  multiplicity = 0;
	
  if (fADC[0] & 1<<11) multiplicity+=2;
  if (fADC[0] & 1<<12) multiplicity++;
  if (fADC[0] & 1<<13) multiplicity++;
 	
}

ClassImp(TA2GoAT)
