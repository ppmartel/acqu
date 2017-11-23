//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Update	JRM Annand...19th Feb 2004...User code
//--Description
//                *** Acqu++ <-> Root ***
// Apparatus clas to monitor linear polarisation
//
// TA2LinearPolEpics
//
// User coded version of Linearly polarised photon beam monitoring apparatus

#ifndef __TA2LinearPolEpics_h__
#define __TA2LinearPolEpics_h__

#include "TA2Apparatus.h"  
#include "TA2Tagger.h"
#include "TEPICSmodule.h"
#include "TA2Ladder.h"

enum { ETablePara	=0,	//for pol lookup table
       ETablePerp	=1,
       ETableAmo	=2,
};

//Some enumerators and names
enum {      
  THETA,  // [0] theta      main angle responsible for coherent edge cutoffs
  SIGMA,  // [1] sigma      smearing of theta
  THETAR, // [2] thetar     relative angle resonsible for colli cutoffs
  SIGMAR, // [3] sigmar     smearing of colli cutoff angle
  E0MEV,  // [4] beam energy
  NVEC,   // [5] nvec       no of vectors contributing
  IVEC,   // [6] ivec[]     array of intensities of vectors up to nvec.
};  

enum { THETASTEPS = 201 };  //No of steps in the Gaussian smearing of the coherent edge.

class TA2LinearPolEpics : public TA2Apparatus {
 protected:
  TA2System *fAnalysis;
  Int_t    fInitLevel;		//To allow multiple passes at Init

  Int_t    fTaggerChannels;	// no of tagger scalers
  Int_t    fNormChannel;	// Channel to use for normalisation
  Double_t fNormEnergy;		// Photon energy for normalisation for  "enhancement"
  Double_t *fEnergyCalib;	// Photon energy map in increasing photon energies
  Double_t *fLadderPhotonEnergy;	// Photon energy map in ladder element order;
  Double_t  *fEnergyBins;
  Double_t fBeamEnergy;		// Photon beam energy
  UInt_t  *fScalerCurr;	        // ptr to ladder current scaler buffer
  char 	   fTaggerName[64];	// Name of tagger object (in case more than 1 tagger defined)
  char 	   fLadderName[64];	// Name of ladder object within tagger (ditto)
  Double_t *fIncSpectrum;	// Array to hold Inc ref. spectrum
  Double_t *fIncGatedSpectrum;  // Array to hold Inc gated ref. spectrum
  Double_t *fCohSpectrum;	// Array to hold Coh spectrum
  Double_t *fEnhSpectrum;	// Array to hold Enhancement spectrum
  Double_t *fRandSubtraction;   // Array to hold Random Subtraction spectrum
  Double_t *fAccPromptSpec;     // Array to hold accumulated prompt spectrum
  Double_t *fAccRandSpec;       // Array to hold accumulated random spectrum
  Double_t *fGatedCurrEnhSpec;  // Array to hold gated buffered enhancement spectrum
  Bool_t   *fBadScalerChan;	// Hold map of bad channels
  Bool_t   *fBadGatedScalerChan;// Hold map of bad channels of gated spectrum
  Bool_t   fHaveIncScaler;	// Flag we have ref from scalers.
  Bool_t   fHaveIncGatedScaler; // Flag we have ref from gated scalers.
  Bool_t   fHaveTaggerApp;     	// Flag if tagger apparatus
  Bool_t   fHaveLadderDet;     	// Flag if ladder detector loaded
  Bool_t   fDoingScalers;	// Flag if we're handling scalers
  Char_t   fCurrentRunFileName[128]; // Hold the name of the current run file
  Bool_t   fIsNewFile;		// To when a new file is opened
  Int_t    fRunNo;
  Double_t *fPolArray;          // Hold the polarizations for all the hits in the ladder.
  Double_t *fPolArrayM;         // Hold the polarizations for all the hits in the ladder, including multihit.
  Int_t   *fLadderHits;         // pointer to ladder hits array
  Double_t **fAccScaler;        // keep a few consecutive scaler buffers for summing up
  Double_t **fAccPromptScaler;
  Double_t **fAccRandScaler;
  Int_t    fNScalerBuffers;
  Int_t    fScalerEvent;
  
  Double_t fEdge;		// for position of coherent edge chans
  Double_t fEdgeError;
  Double_t fEdgeGated;          // for position of gated coherent edge chans
  Double_t fEdgeGatedError;
  Double_t fEdgeSetting;	// for nominal edge setting
  Double_t fEdgeRange;	        // how much it can drift from the nominal setting 
  Double_t fLastEdge;		// for position of coherent edge chans
  Int_t    fPlane;              // for plane
  Int_t    fRadiator;           // radiator
  Char_t   fPlaneString[10];    // mostly epics (but para or perp to force if epics not in data)
  Char_t   fEdgeString[10];     // mostly epics (but fit to force if epics not in data)
  Int_t    fForcePlane;         // override the plane setting from EPICS and force to para or perp
  Int_t    fForceFit;           // override the edge setting from EPICS and use fit
  Double_t fEdgeMin;            // range in which to try and fit coherent edge
  Double_t fEdgeMax;
  Int_t    fDiamondID;          //id of rads in epics
  Int_t    fAmoID;
  Int_t    fIsDiamond;
  Int_t    fIsEpics;

  Int_t    fPolTableNRanges[2];       //pol table run ranges
  Int_t    fPolTableTypes[50][2];     //pol table run ranges
  Int_t    fPolTableRangesMin[50][2];
  Int_t    fPolTableRangesMax[50][2];
  Double_t fPolTableParams[50][2][11];
  Int_t    fPolRangeIndex[2];
  Int_t    fLastPolRangeIndex[2];
  Double_t *fCurrentPolTable;
  Double_t *fCurrentPolTable_TC;
  Double_t *fCurrentEnhTable;  
  Double_t *fCurrentEnhTable_TC;
  
  Int_t    fNRunRanges;
  Int_t    fRunRangeMin[100];
  Int_t    fRunRangeMax[100];
  Int_t    fRunPlanes[100];
  Char_t   fRunRefFiles[100][80];
  Int_t    fRunRangeIndex;
  Int_t    fLastRunRangeIndex;
  

  Double_t polTable[2][352];           //where its [plane][edge][E_id][field]
  
  //flags for lines from setup file;
  Bool_t fHaveApp;     
  Bool_t fHaveScaler; 
  Bool_t fHaveCbrem;  
  Bool_t fHavePVNames;
  Bool_t fHaveRads;   
  Bool_t fHaveFit;    
  Bool_t fHavePolTable;    


  // ------------------- epics general: for any epics handling  -------------------
  TEPICSmodule *fEpics;                                   // Epics module
  Int_t fEpicsInit;                                       // flag EPICsS initiaised
  Int_t fNEpics;                                          // No of different epics event types
  Char_t **fEpicsBuff;                                    // Buffers for the EPICS data
  Bool_t *fEpicsIndex;                                    // Flag which EPICS indices were read in EPICS event.
  Int_t fEpicsCount;                                      // Count EPICS events
  Int_t fEpicsNElem;                                      // no of elements in last channel read
  Int_t fEpicsType ;                                      // data type of last channel read
  void *fEpicsChannelBuffer;                              // buffer of last channel data

  // --------------- epics coherent brem ------------------------------------------
  // In the simplest form of readout, all values are made into floats.
  
  Int_t   fLinPolEpicsInit;                              // flag LinPolEpics initiaised
  Int_t   fA2LinPolEpicsIndex;                           // index of epics buffer with lin pol data
  
  Float_t fA2LinPolCohEdgeEpics;                         // variables to be read from epics
  Float_t fA2LinPolCohRadiatorEpics;
  Float_t fA2LinPolCohPlaneEpics;
  Float_t fA2GoniAxis1Epics;
  Float_t fA2GoniAxis2Epics;
  Float_t fA2GoniAxis3Epics;
  Float_t fA2GoniAxis4Epics;
  Float_t fA2GoniAxis5Epics;

  Int_t   fNEpicsVariables;
  Float_t *fLinPolEpicsVariables[8];                     //array of addresses of variables to be read
  Char_t  fLinPolEpicsPVNames[8][32];                    //names of variables to be read

  UShort_t fPolPlane;		// Polarisation plane (Para / Perp)
  Double_t fDInc;		//dummy variables to set up 1D and 2D scaler hists
  Double_t fDGatedInc;
  Double_t fDCoh;
  Double_t fDEnh;
  Double_t fDCohPara;
  Double_t fDEnhPara;
  Double_t fDCohPerp;
  Double_t fDEnhPerp;
  
  Double_t fDGatedCoherent;
  Double_t fDGatedCoherentPara;
  Double_t fDGatedCoherentPerp;
  Double_t fDGatedCurrEnh;
  Double_t fDGatedCurrEnhPara;
  Double_t fDGatedCurrEnhPerp;
  
  Double_t fDEdge;		// for position history  of coherent edge MeV
  Double_t fDEdgeGated;
  Double_t fDEdgePerp;
  Double_t fDEdgePara;
  Double_t fDEdgeGatedPara;
  Double_t fDEdgeGatedPerp;
  Double_t fDEdgeDistPerp;		// for position distribution  of coherent edge MeV
  Double_t fDEdgeDistPara;
  Double_t fDEdgeEpics;		// for position history  of coherent edge MeV
  Double_t fDEdgePerpEpics;
  Double_t fDEdgeParaEpics;
  Double_t fDEdgeDistPerpEpics;		// for position distribution  of coherent edge MeV
  Double_t fDEdgeDistParaEpics;
  Double_t fDPolTableEnh;
  Double_t fDPolTablePol;
  Double_t fDPolMean;
  Double_t fDPolCount;
  Double_t fBeforeEdge;
  Double_t fAfterEdge;
  Int_t    fBeforeEdgeBin;
  Int_t    fAfterEdgeBin;
  Double_t fPolMin;
  Double_t fDeadband;           // Energy range for deadband on  coherent peak
  
  
  Double_t fDScanStep;
  UInt_t   fScalerCount;		//running counter of scaler events
  UInt_t   fScalerCountGated;           //Running counter for gated scaler reads

  TA2Tagger  *fTagger;		        //Tagger and Ladder classes
  TA2Ladder  *fLadder;

  Bool_t fIsPolLookupTable;		//Flag if pol table loaded
  Char_t fPolLookupTableName[100];	//Name of run table
  Int_t  fNPolLookupEdges[2];		//number of different edge positions in lookup table [para,perp]
  Int_t  fNPolLookupEnergies;		//number of photon  energies in lookup table
  Double_t *fPolLookupEdges[2];		//array of all lookup edge positions [para,perp]
  Double_t *fPolLookupEnergies;		//array of all lookup photon energies
  Double_t **fPolLookupPolarisations[2]; //pol lookup arrays for each edge position [para,perp]
  Double_t *fPolTableCurrent;		//interpolated table for current plane and edge position
  Double_t *fPolTableCurrentLad;       	//interpolated table for current plane and edge position ladder
 
  TH1F *fHInc;				//ptrs to the hists if they're defined
  TH1F *fHGatedInc;
  TH1F *fHCoh;
  TH1F *fHEnh;
  TH1F *fHCohPara;
  TH1F *fHEnhPara;
  TH1F *fHCohPerp;
  TH1F *fHEnhPerp;
  TH1F *fHEdge;
  TH1F *fHEdgeGated;
  TH1F *fHEdgePerp;
  TH1F *fHEdgePara;
  TH1F *fHEdgeGatedPara;
  TH1F *fHEdgeGatedPerp;
  TH1F *fHEdgeDistPara;
  TH1F *fHEdgeDistPerp;
  TH1F *fHEdgeEpics;
  TH1F *fHEdgePerpEpics;
  TH1F *fHEdgeParaEpics;
  TH1F *fHEdgeDistParaEpics;
  TH1F *fHEdgeDistPerpEpics;
  TH1F *fHPolTableEnh;
  TH1F *fHPolTablePol;
  TH1F *fHPolMean;
  TH1F *fHPolCount;
  TH1F *fHistE;
  TH1F *fHistP;
  TH1F *fWeightHist;
  TH2F *fThetaPol;
  TH2F *fThetaItot;

  TH1F *fHGatedCoherent;
  TH1F *fHGatedCoherentPara;
  TH1F *fHGatedCoherentPerp;
  TH1F *fHGatedCurrEnh;
  TH1F *fHGatedCurrEnhPara;
  TH1F *fHGatedCurrEnhPerp;

  ///////////////////////////////////////////////////////
  /////////////////Variables for fit/////////////////////
  Double_t fLOWFIT; //how far below the peak in MeV to start fitting
  Double_t fFitedge;
  Double_t fbestPar[10];
  Double_t fbestChisq;
  Double_t fFitMinBin;
  Double_t fFitMaxBin;
  Bool_t fFitInitialised;
  TH1F *fFitEnhData;
  TH1F *fFitEnh;
  TH1F *fFitPol;
  
 public:
  TA2LinearPolEpics( const char*, TA2System* ); 	// pass ptr to analyser
  virtual 	~TA2LinearPolEpics();
  virtual void 	PostInitialise();      		// some setup after parms read in
  virtual void 	ParseMisc(char *);		// read parameters in the setup file
  virtual 	TA2DataManager* CreateChild( const char*, Int_t ){return NULL;};
  virtual void 	LoadVariable( );         	// display setup
  virtual void 	Reconstruct( );			//the main event reconstruction function
  Int_t  	LoadAmoRef(Char_t *refFilename);//load the amorphoes reference scalers 
  Int_t		GetPolPlane(){return fPlane;};  //para,perp or unknown
  Double_t	GetEdge(){return fEdge;};       //coherent edge
  Double_t      GetEdgeGated(){return fEdgeGated;};
  Double_t	GetEdgeSetting(){return fEdgeSetting;};  //coherent edge
  Double_t	*GetPolTable(){return fCurrentPolTable;};       //get pol for given photon energy
  Double_t	*GetEnhTable(){return fCurrentEnhTable;};       //get pol for given photon energy
  Double_t	*GetPolTable_TC(){return fCurrentPolTable_TC;};       //get pol for Tagger channels
  Double_t	*GetEnhTable_TC(){return fCurrentEnhTable_TC;};       //get pol for Tagger Channels
  Double_t	GetPolDegree(Double_t energy);	//get pol for given photon energy
  Int_t 	GetNormChannel(){return fNormChannel;};		//return the normalisation channel
  Double_t 	*GetEnergyCalib(){return fEnergyCalib;};	//get the energy calibration table
  Double_t 	*GetPolArray(){return fPolArray;};	//array of polarizations for all ladder hits.
  
 private:
  Int_t		CreateCurrentPolTable();	//create one for the current edge position
  void          enhFromParams();
  void          enhFromParams(Double_t *par, Double_t edge); //called by fit function
  void          SetPolTable();
  Double_t 	*FillPolArray();	         //fill and return array of polarizations for all ladder hits.
  void          FitInit(const TH1F *histD);
  void          FitEnhancement(const TH1F *histD, const double scalingN, const int nVec=2);
  void          parFromHuman(Double_t edgeMeV = 750.0, Double_t spreadMeV = 20.0, Double_t colliDist_m = 2.5, Double_t colliRad_mm = 3.0, Int_t nVec = 2, Double_t *par=NULL);
  Double_t      efit(const Double_t *parms); //The main customized fitting function which gets called by MINUIT
  
  // Root needs this line for incorporation in dictionary
  ClassDef(TA2LinearPolEpics,1)
};


//-----------------------------------------------------------------------------
#endif
