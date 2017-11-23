//--Author	JRM Annand   23rd Feb 2006
//--Rev      
//--Update      
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCHe4Phase
//
// 4He(gamma,n) kinematics
// 2,3 body breakup
//

#include "TMCHe4Phase.h"
#include "TFile.h"

static Double_t Mhe3;
static Double_t Mhe4;
static Double_t C;


enum { EHe4Egamma };
static const Map_t kHe4Keys[] = {
  {"E-gamma:",         EHe4Egamma},
  {NULL,               -1}
};
//-----------------------------------------------------------------------------
TMCHe4Phase::TMCHe4Phase( const Char_t* name, Int_t seed )
  :TA2System( name, kHe4Keys )
{
  // Calculate masses 3He and 4He in GeV
  // A * AMU - 2*M_e   in GeV
  // Zero protected variables

  Mhe3 = Mamu * 3.016030 - (2*Me);
  Mhe4 = Mamu * 4.002602 - (2*Me);
  fRand = new TRandom( seed );
  fTh_n = fT_nr = fEbeam = fEb = fdEb = 0.0;
  fThDet = NULL;
  fdThDet = 0.0;
  fArcMi = fArcPl = fXpl = fXmi = fYpl = fYmi = fGrad = NULL;
  fRfp = fRb = fRt = fRdet = fDfp = 0.0;
  fNdet = fNevent = 0;
  fIsHist = kFALSE;
  hEbeam = NULL;
  hDalitz = NULL;
  hTm2 = hTm3 = hDfp = hTm23 = NULL;
}

//---------------------------------------------------------------------------
TMCHe4Phase::~TMCHe4Phase()
{
  // Free up allocated memory
}

//-----------------------------------------------------------------------------
void TMCHe4Phase::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus

  switch (key){
  case EHe4Egamma:
    // Photon energy
    if( sscanf(line,"%lf%lf",&fEb, &fdEb) != 2 ){
      PrintError(line,"<Parse photon energy>");
      return;
    }
    break;
  default:
    // default main apparatus SetConfig()
    //    TA2System::SetConfig( line, key );
    break;
  }
}

//-----------------------------------------------------------------------------
void TMCHe4Phase::PostInit( )
{
  // Default initialise
  if( !fNdet ){
    fNdet = 8;
    fThDet = new Double_t[fNdet];
    for( Int_t i=0; i<fNdet; i++ ) fThDet[i] = TMath::DegToRad()*(30.1 + i*15);
    fdThDet = TMath::DegToRad() * 6.71;
    fIsHist = kTRUE;
    fRfp = 2.55;                 // 2.6 m flight path
    fRb = 0.01;                  // 1cm beam radius
    fRt = 0.045;                 // 4.5cm target radius
    fRdet = 0.10;                // 0.1 m neutron detector thickness
    fdTfp = 0.72;                // sigma uncertainty electronic time pickoff
    //
    fEb = 0.0246;                // beam energy (GeV)
    //    fEb = 0.0267;                // beam energy (GeV)
    //    fEb = 0.0288;                // beam energy (GeV)
    //    fEb = 0.0311;                // beam energy (GeV)
    //    fEb = 0.0346;                // beam energy (GeV)
    //    fEb = 0.0364;                // beam energy (GeV)
    //    fEb = 0.0388;                // beam energy (GeV)
    //    fEb = 0.0406;                // beam energy (GeV)
    //    fEb = 0.0514;                // beam energy (GeV)
    //    fEb = 0.0536;                // beam energy (GeV)
    //    fEb = 0.0560;                // beam energy (GeV)
    //    fEb = 0.0584;                // beam energy (GeV)
    //    fEb = 0.0636;                // beam energy (GeV)
    //    fEb = 0.0685;                // beam energy (GeV)
    fdEb = 0.00125;              // 1/2 beam bin width
    fNevent = 100000;
  }
  else fThDet = new Double_t[fNdet];

  fArcMi =  new Double_t[fNdet];
  fArcPl =  new Double_t[fNdet];
  fXpl =  new Double_t[fNdet];
  fXmi =  new Double_t[fNdet];
  fYpl =  new Double_t[fNdet];
  fYmi =  new Double_t[fNdet];
  fGrad =  new Double_t[fNdet];

  for( Int_t i=0; i<fNdet; i++ ){
    Double_t ThMi = (fThDet[i] - fdThDet);
    Double_t ThPl = (fThDet[i] + fdThDet);
    fArcMi[i] = fRfp*ThMi;
    fArcPl[i] = fRfp*ThPl;
    fXmi[i] = fRfp * TMath::Sin( ThMi ) ;
    fXpl[i] = fRfp * TMath::Sin( ThPl ) ;
    fYmi[i] = fRfp * TMath::Cos( ThMi ) ;
    fYpl[i] = fRfp * TMath::Cos( ThPl ) ;
    fGrad[i] = (fYpl[i] - fYmi[i])/(fXpl[i] - fXmi[i]);
  }
  // Photon beam incident on target at rest in lab.
  fTarget.SetPxPyPzE(0.0, 0.0, 0.0, Mhe4);
  // Final-state particle masses for phase-space generator
  fM2[0] = Mhe3; fM2[1] = Mn;
  fM3[0] = Md;   fM3[1] = Mp; fM3[2] = Mn;
  C = TMath::C() * 1.0e-9;   // C in m/ns
  // Quick check that masses are correct
  fQ2 = 1000.0*(Mhe4 - Mhe3 - Mn);
  fQ3 = 1000.0*(Mhe4 - Md - Mp - Mn);
  printf("Q 2-Body = %f  , Q 3-Body = %f   \n",fQ2,fQ3);
  //
  // Histograms
  if(fIsHist){
    Char_t hname[128];
    hEbeam = new TH1F( "Ebeam","h12", 500,0.0,100.0 );
    hDalitz = new TH2F("Dalitz","h12", 200,7.8,8.3, 200,7.8,8.3);
    hTm2 = new TH1F*[fNdet];
    hTm3 = new TH1F*[fNdet];
    hTm23 = new TH1F*[fNdet];
    hT2 = new TH1F*[fNdet];
    hT2B = new TH1F*[fNdet];
    hT3 = new TH1F*[fNdet];
    hT23 = new TH1F*[fNdet];
    hDfp =  new TH1F*[fNdet];
    for( Int_t i=0; i<fNdet; i++ ){
      sprintf( hname,"%s%d", "tm2_",i );
      hTm2[i] = new TH1F( hname, "2-body", 1000, 0.0, 200 );
      sprintf( hname,"%s%d", "tm3_",i );
      hTm3[i] = new TH1F( hname, "3-body", 1000, 0.0, 200 );
      sprintf( hname,"%s%d", "tm23_",i );
      hTm23[i] = new TH1F( hname, "2&3-body", 1000, 0.0, 200 );
      sprintf( hname,"%s%d", "T2B_",i );
      hT2B[i] = new TH1F( hname, "2-body", 500, 0.0, 100 );
      sprintf( hname,"%s%d", "T2_",i );
      hT2[i] = new TH1F( hname, "2-body", 500, 0.0, 100 );
      sprintf( hname,"%s%d", "T3_",i );
      hT3[i] = new TH1F( hname, "3-body", 500, 0.0, 100 );
      sprintf( hname,"%s%d", "T23_",i );
      hT23[i] = new TH1F( hname, "2&3-body", 500, 0.0, 100 );
      sprintf( hname,"%s%d", "Dfp_",i );
      hDfp[i] = new TH1F( hname, "neutron flight path", 500, 0.0, 5.0 );
    }
  }
  return;
}

//-----------------------------------------------------------------------------
void TMCHe4Phase::FlightPath(TLorentzVector* P )
{
  // Flight Path target to Detector
  //
  Int_t i;
  Double_t x0 = fRand->Uniform(-fRb,fRb);
  Double_t y0 = fRand->Uniform(-fRt,fRt);
  Double_t beta, gamma;  
  //  TVector2 uv(P->X(),P->Y());
  //  uv = uv.Unit();
  //  Double_t u = uv.X();
  //  Double_t v = uv.Y();
  Double_t u = TMath::Sin( fTh_n * TMath::DegToRad() );
  Double_t v = TMath::Cos( fTh_n * TMath::DegToRad() );
  Double_t b = x0*u + y0*v;
  Double_t c = x0*x0 + y0*y0 -fRfp*fRfp;
  Double_t d = -b + TMath::Sqrt( b*b - c );
  // Double_t x1 = x0 + u*d;
  Double_t y1 = y0 + v*d;
  Double_t a1 = TMath::ACos( y1/fRfp ) * fRfp;
  for( i=0; i<fNdet; i++ ){
    if( (fArcMi[i] < a1) && (fArcPl[i] > a1) ) break;
  }
  if( i == fNdet ) { fIsHit = kFALSE; return; }

  fIdet = i;
  d = (fYmi[i] - fGrad[i]*fXmi[i] + fGrad[i]*x0 - y0)/(v - fGrad[i]*u);
  Double_t yPr = fYmi[i] + fRdet*TMath::Cos( fThDet[i] );
  Double_t xPr = fXmi[i] + fRdet*TMath::Sin( fThDet[i] );
  Double_t dPr = (yPr - fGrad[i]*xPr + fGrad[i]*x0 - y0)/(v - fGrad[i]*u);
  fDfp = fRand->Uniform(d,dPr);
  Double_t phi = fRand->Uniform(0.0, fdThDet);
  fDfp = fDfp/TMath::Cos(phi);
  fTfp = fDfp/(C*fPn->Beta());
  fTfp = fRand->Gaus( fTfp, fdTfp );
  beta = fDfp/(fTfp * C);
  gamma = 1./TMath::Sqrt(1. - beta*beta);
  Double_t E_nr = Mn * gamma;
  fT_nr = 1000. * (E_nr - Mn);
  fIsHit = kTRUE;
  if( fIsHist ){
    hDfp[i]->Fill(fDfp);
    hTm23[i]->Fill(fTfp);
    hT23[i]->Fill(fT_nr);
  }
  return;
}

//-----------------------------------------------------------------------------
void TMCHe4Phase::Generate()
{
// gamma + 4He -> X  2-3 body final-state separation
// 3He,n and d,p,n final state kinematics samples from phase space.
// ROOT class TGenPhaseSpace used.
//
//   Double_t C = TMath::C() * 1.0e-9;   // C in m/ns
//   Double_t T_n, T_nr;                 // neutron kinetic energy
//   Double_t beta, gamma;               // velocity

   for (Int_t n=0;n<fNevent;n++) {
     GenTotal();                      // beam + target 4 momentum
     if(fEbeam > fQ2){
       Gen2Body();
       FlightPath(fPn);
       if( fIsHit && fIsHist ){
	 hTm2[fIdet]->Fill(fTfp);
	 hT2[fIdet]->Fill(fT_nr);
	 hT2B[fIdet]->Fill(fT_n);
	 //    tm2[ith]->Fill( ftr );
	 //    tm23[ith]->Fill( ftr );
	 //    h23_13.Fill( ftr, Theta_n );
	 //    h2_14.Fill( T_nr, Theta_n );
	 //    h23_14.Fill( T_nr, Theta_n );
	 //    h2_13.Fill( ftr, Theta_n );
       }
     }
     //     continue;
     if( fEbeam > fQ3 ){
       Gen3Body();
       FlightPath(fPn);
       if( fIsHit && fIsHist ){
	 hTm3[fIdet]->Fill(fTfp);
	 hT3[fIdet]->Fill(fT_nr);
	 //       h3_13.Fill( ftr, Theta_n );
	 //       h23_13.Fill( ftr, Theta_n );
	 //       h3_14.Fill( T_nr, Theta_n );
	 //       h23_14.Fill( T_nr, Theta_n );
	 //       tm3[ith]->Fill( ftr );
	 //       tm23[ith]->Fill( ftr );
       }
     }
   }

   TFile f("He4PhaseHist.root","recreate");
   gROOT->GetList()->Write();
   f.Close();
   return;
}

ClassImp(TMCHe4Phase)
