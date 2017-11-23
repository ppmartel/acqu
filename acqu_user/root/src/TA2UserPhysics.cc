//--Author	JRM Annand   18th Feb 2004   Use def physics
//--Rev
//--Update
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2UserPhysics
//
// User-defined physics class
//

#include "TA2UserPhysics.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2Calorimeter.h"
#include "TAcquFile.h"
#include "TA2Tagger.h"


//-----------------------------------------------------------------------------
TA2UserPhysics::TA2UserPhysics( const char* name, TA2Analysis* analysis )
  :TA2Physics( name, analysis )
{
  // Initialise UserPhysics variables here
}


//-----------------------------------------------------------------------------
TA2UserPhysics::~TA2UserPhysics()
{
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
}

//---------------------------------------------------------------------------
void TA2UserPhysics::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // default Cut setup
  fNbeam = fNpermutation[0];
  fP4miss = new TLorentzVector[fNbeam];
  fMmiss = new Double_t[fNbeam+1];       // +1 for buffer end marker
  fDpnMmiss = new Double_t[fNbeam+1];
  fDpnAngleCM = new Double_t[fNbeam+1];
  fThetaDiffCM = new Double_t[fNbeam+1];
  fThetaPCM = new Double_t[fNbeam+1];
  fThetaNCM = new Double_t[fNbeam+1];
  fPmiss = new Double_t[fNbeam+1];
  fMinv = new Double_t[fNbeam+1];
  TA2Physics::PostInit();
  fCB = (TA2Calorimeter*)((TA2Analysis*)fParent)->GetChild("CB");
  fTAGG = (TA2Tagger*)((TA2Analysis*)fParent)->GetChild("TAGG");
  fTAPS = (TA2Calorimeter*)((TA2Analysis*)fParent)->GetChild("TAPS");

  // Anythings else here
}

//-----------------------------------------------------------------------------
void TA2UserPhysics::LoadVariable( )
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

  TA2Physics::LoadVariable();
  TA2DataManager::LoadVariable("DpnAngle",  &fDpnAngle,       EDSingleX);
  TA2DataManager::LoadVariable("DpnAngleCM",fDpnAngleCM,      EDMultiX);
  TA2DataManager::LoadVariable("PhiDiff",   &fPhiDiff,        EDSingleX);
  TA2DataManager::LoadVariable("ThetaDiff", &fThetaDiff,      EDSingleX);
  TA2DataManager::LoadVariable("ThetaDiffCM",fThetaDiffCM,    EDMultiX);
  TA2DataManager::LoadVariable("ThetaP",    &fThetaP,         EDSingleX);
  TA2DataManager::LoadVariable("ThetaN",    &fThetaN,         EDSingleX);
  TA2DataManager::LoadVariable("ThetaPCM",  fThetaPCM,        EDMultiX);
  TA2DataManager::LoadVariable("ThetaNCM",  fThetaNCM,        EDMultiX);
  TA2DataManager::LoadVariable("DpnMinv",   &fDpnMinv,        EDSingleX);
  TA2DataManager::LoadVariable("DpnMmiss",  fDpnMmiss,        EDMultiX);
  TA2DataManager::LoadVariable("CBTAPS2PhotonMinv",  &fCBTAPS2PhotonMinv,        EDSingleX);
  return;
}

//-----------------------------------------------------------------------------
void TA2UserPhysics::Reconstruct()
{
  // Use 4-momenta and other info from apparati to reconstruct
  // reaction process
  Int_t ntagg = fTAGG->GetNParticle();
  Int_t ncb = fCB->GetNParticle();
  fP4beam = fTAGG->GetP4();
  TLorentzVector* p4cb = fCB->GetP4();
  TLorentzVector* p4taps;
  TLorentzVector p4Dp, p4Dn, p4DpCM, p4DnCM, p4Dtot, p4Dmiss;
  TVector3 pDp, pDn, boost;
  Int_t i;
  if( ncb == 1 ){
    if( fTAPS ){
      if( fTAPS->GetNParticle() == 1 ){
	p4taps = fTAPS->GetP4();
	p4Dtot = p4taps[0] + p4cb[0];
	fCBTAPS2PhotonMinv = p4Dtot.M();
      }
    }
  }      
  else if( ncb == 2 ){
    Double_t Tp,Tn;
    p4Dp = p4cb[0];
    p4Dn = p4cb[1];
    Tp = p4Dp.E();
    Tn = p4Dn.E();
    //    printf("CB: %lf %lf %lf %lf...   %lf %lf %lf %lf\n",
    //	   pDp.X(),pDp.Y(),pDp.Z(),p4cb[0].T(),
    //	   pDn.X(),pDn.Y(),pDn.Z(),p4cb[1].T());
    p4Dp.SetE( (Tp + 938.271) );
    p4Dn.SetE( (Tn + 939.565) );
    pDp = p4Dp.Vect();
    pDp = pDp.Unit() * TMath::Sqrt( p4Dp.M2() - 938.271*938.271 );
    pDn = p4Dn.Vect();
    pDn = pDn.Unit() * TMath::Sqrt( p4Dn.M2() - 939.565*939.565 );
    p4Dp.SetVect( pDp );
    p4Dn.SetVect( pDn );

    p4Dtot = p4Dp + p4Dn;
    fDpnMinv = p4Dtot.M();
    fDpnAngle = p4Dp.Angle(p4Dn.Vect()) * 57.29578;
    fPhiDiff = (p4Dp.Phi() - p4Dn.Phi()) * 57.29578;
    fThetaP = p4Dp.Theta() * 57.29578;
    fThetaN = p4Dn.Theta() * 57.29578;
    fThetaDiff = fThetaP - fThetaN;
    for(i=0; i<ntagg; i++ ){
      if( i >= fNbeam ) break;
      fP4input = fP4target[1] + fP4beam[i];
      //      printf("Input: %lf %lf %lf %lf\n",
      //	     fP4input.X(),fP4input.Y(),fP4input.Z(),fP4input.T());
      boost = -fP4input.BoostVector();
      //      bx = boost.X(); by = boost.Y(); bz = boost.Z();
      p4DpCM = p4Dp;
      p4DpCM.Boost(boost);
      //      printf("Dp: %lf %lf %lf %lf...   %lf %lf %lf %lf\n",
      //	     p4Dp.X(),p4Dp.Y(),p4Dp.Z(),p4Dp.T(),
      //	     p4DpCM.X(),p4DpCM.Y(),p4DpCM.Z(),p4DpCM.T());
      p4DnCM = p4Dn;
      p4DnCM.Boost(boost);
      fP4miss[i] = fP4input - p4Dtot;
      fDpnMmiss[i] = fP4miss[i].M();
      fDpnAngleCM[i] = p4DpCM.Angle(p4DnCM.Vect()) * 57.29578;
      
      //      fDpnAngle[i] = pDp.Angle(pDn) * 57.29578;
      fThetaPCM[i] = p4DpCM.Theta() * 57.29578;
      fThetaNCM[i] = p4DnCM.Theta() * 57.29578;
      fThetaDiffCM[i] = fThetaPCM[i] - fThetaNCM[i];
      
    }
    fDpnMmiss[i] = EBufferEnd;    
    fDpnAngleCM[i] = EBufferEnd;    
    fThetaPCM[i] = EBufferEnd;
    fThetaNCM[i] = EBufferEnd;
    fThetaDiffCM[i] = EBufferEnd;
  }
  else{ 
    fDpnMinv = 0;
    fDpnMmiss[0] = EBufferEnd;
    fDpnAngle = 10000;
    fDpnAngleCM[0] = EBufferEnd;
    fPhiDiff = 10000;
    fThetaDiff = 10000;
    fThetaDiffCM[0] = EBufferEnd;
    fThetaPCM[0] = EBufferEnd;
    fThetaNCM[0] = EBufferEnd;
  }

    
  //  TLorentzVector p4CBtot = fCB->GetP4tot();
  //  TLorentzVector target = 
  for(i=0; i<ntagg; i++ ){
    if( i >= fNbeam ) break;
    fP4input = fP4target[0] + fP4beam[i];
    fP4miss[i] = fP4input - fCB->GetP4tot();
    fMmiss[i] = fP4miss[i].M();
  }
  fMmiss[i] = EBufferEnd;
}
ClassImp(TA2UserPhysics)
