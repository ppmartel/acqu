//////////////////////////////////////////////////////////////////
//TA2CBKinematicFitter
//////////////////////////////////////////////////////////////////

#include "TA2CBKinematicFitter.h"
#include "TDecompLU.h"
#include <iostream>


//-----------------------------------------------------------------------------
TA2CBKinematicFitter::TA2CBKinematicFitter(Int_t npart, Int_t ncon, Int_t unk){

  fNvar=4;
  fNpart=npart;
  fNpar = fNpart*fNvar;
  fNcon=ncon;
  fNparti=0;
  fNpari=0;
  fNconi=0;
  fNiter=0;
  fNunKnown=unk;

  fmAlpha0.ResizeTo(fNpar,1);
  fmAlpha.ResizeTo(fNpar,1);
  fmV_Alpha0.ResizeTo(fNpar,fNpar);
  fmV_Alpha.ResizeTo(fNpar,fNpar);
  fmD.ResizeTo(fNcon,fNpar);
  fmd.ResizeTo(fNcon,1);
  fmlamda.ResizeTo(fNcon,1);
  fmV_D.ResizeTo(fNcon,fNcon);
  fT.ResizeTo(fNpar,fNpar);

  fPtot.SetXYZT(0,0,0,0);

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::ResetMatrices(){

  fmAlpha0.Zero();
  fmAlpha.Zero();
  fmV_Alpha0.Zero();
  fmV_Alpha.Zero();
  fmD.Zero();
  fmd.Zero();
  fmlamda.Zero();
  fmV_D.Zero();

}

//-----------------------------------------------------------------------------
Int_t TA2CBKinematicFitter::Solve(){

  //Solve according to algorithm of Paul Avery:
  //Applied Fitting Theory VI, Formulas for Kinematic Fitting
  //see www.phys.ufl.edu/~avery/fitting.html

  if(fNpart!=fNparti){
    std::cout<<"TA2CBKinematicFitter::Solve() Added wrong number of particles. KinFit not completed"<<std::endl;
    return -1;
  }

  TMatrixD mDT=fmD;
  mDT.T();
  TMatrixD mV_Dinv=fmD*fmV_Alpha0*mDT;
  fmV_D=mV_Dinv;
  TDecompLU lu(fmV_D);
  if(!lu.Decompose()){
    std::cout<<"TA2CBKinematicFitter::Solve() Cannot invert. KinFit not completed"<<std::endl;
    return -1;
  } 
  fmV_D.Invert();
 
  //Double_t det;
  //TMatrixD Einheit = fmV_D*mV_Dinv;
  //if(fNpart == 7) Einheit.Print();

  //Derive langrian multipliers
  fmlamda=fmV_D*fmd;
  //New parameters
  fmAlpha=fmAlpha0-fmV_Alpha0*mDT*fmlamda;
  //New Covariant matrix
  fmV_Alpha=fmV_Alpha0-fmV_Alpha0*mDT*fmV_D*fmD*fmV_Alpha0;
  //chi2
  TMatrixD mlamdaT=fmlamda;
  mlamdaT.T();
  TMatrixD mchi2=mlamdaT*fmd;
  fchi2=mchi2[0][0];
  fNiter++;

  return 1;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddInvMassConstraint(Double_t Minv){

  // d matrix (evaluate constraint eqn.)
  fmd[fNconi][0]=fPtot.M2()-Minv*Minv;

  // D matrix (derivitives of constraint eqn)
  for(Int_t i=0; i<fNpart; i++){
    //[Cons Number][Var Number] 
    fmD[fNconi][0+i*fNvar]=-2*fPtot.X();
    fmD[fNconi][1+i*fNvar]=-2*fPtot.Y();
    fmD[fNconi][2+i*fNvar]=-2*fPtot.Z();
    fmD[fNconi][3+i*fNvar]= 2*fPtot.T();
  }

  //increment constraint counter
  fNconi++;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddSubInvMassConstraint(Int_t Np, Int_t pid[], Double_t Minv){

  //Add invariant mass constraint to subset of particles
  //Np is number of subs particles
  //pid[] contains the particle number i.e the order when they were added

  if(Np>fNpart){
    std::cout<<"TA2CBKinematicFitter::AddSubInvMassConstraint too many particles!"<<std::endl;
    return;
  }

  //Add up the particle 4 vectors
  TLorentzVector ptot(0.0,0.0,0.0,0.0);
  for(Int_t i=0; i<Np; i++){
    ptot+=GetInitialParticle(pid[i]).Get4Vector();
  }

  //d matrix (evaluate constraint eqn.)
  fmd[fNconi][0]=ptot.M2()-Minv*Minv;

  //D matrix (derivitives of constraint eqn)
  for(Int_t i=0;i<Np;i++){
    //[Cons Number][Var Number] 
    fmD[fNconi][0+pid[i]*fNvar]=-2*ptot.X();
    fmD[fNconi][1+pid[i]*fNvar]=-2*ptot.Y();
    fmD[fNconi][2+pid[i]*fNvar]=-2*ptot.Z();
    fmD[fNconi][3+pid[i]*fNvar]= 2*ptot.T();
  }

  //increment constraint counter
  fNconi++;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddTotEnergyConstraint(Double_t Etot){

  //d matrix (evaluate constraint eqn.)
  fmd[fNconi][0]=fPtot.E()-Etot;

  //D matrix (derivitives of constraint eqn)
  for(Int_t i=0; i<fNpart; i++){
    //[Cons Number][Var Number] 
    fmD[fNconi][0+i*fNvar]=0;
    fmD[fNconi][1+i*fNvar]=0;
    fmD[fNconi][2+i*fNvar]=0;
    fmD[fNconi][3+i*fNvar]=1;
  }

  //increment constraint counter
  fNconi++;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddTotMomentumConstraint(TVector3 mom){

  //d matrix (evaluate constraint eqn.)
  fmd[fNconi+0][0]=fPtot.X()-mom.X();
  fmd[fNconi+1][0]=fPtot.Y()-mom.Y();
  fmd[fNconi+2][0]=fPtot.Z()-mom.Z();

  //D matrix (derivitives of constraint eqn)
  Double_t D[3][4]={{1,0,0,0},{0,1,0,0},{0,0,1,0}};
  for(Int_t i=0; i<fNpart; i++){
    for(Int_t j=0;j<3;j++){
      //[Cons Number][Var Number] 
      fmD[fNconi+j][0+i*fNvar]=D[j][0];
      fmD[fNconi+j][1+i*fNvar]=D[j][1];
      fmD[fNconi+j][2+i*fNvar]=D[j][2];
      fmD[fNconi+j][3+i*fNvar]=D[j][3];
    }
  }
  fNconi+=3;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddSubMissMassConstraint(TLorentzVector Mom, Int_t Np, Int_t pid[], Double_t MissMass){

  // Add missing mass constraint to subset of particles
  // Np is number of subset particles
  // pid[] contains the particle number i.e the order when they were added

  if(Np>fNpart){
    std::cout<<"TA2CBKinematicFitter::AddSubMissMassConstraint too many particles!"<<std::endl;
    return;
  }

  //Add up the particle 4 vectors
  TLorentzVector Ptot(0.0,0.0,0.0,0.0);
  for(Int_t i=0; i<Np; i++){
    Ptot += GetInitialParticle(pid[i]).Get4Vector();
  }

  //d matrix (evaluate constraint eqn.)
  fmd[fNconi][0]=(Mom-Ptot).M2()-MissMass*MissMass;

  //D matrix (derivitives of constraint eqn)
  for(Int_t i=0 ;i<Np ;i++)
    {//[Cons Number][Var Number] 
    fmD[fNconi][0+pid[i]*fNvar]=-2*(Ptot-Mom).X();
    fmD[fNconi][1+pid[i]*fNvar]=-2*(Ptot-Mom).Y();
    fmD[fNconi][2+pid[i]*fNvar]=-2*(Ptot-Mom).Z();
    fmD[fNconi][3+pid[i]*fNvar]= 2*(Ptot-Mom).T();
  }
    //increment constraint counter
  fNconi++;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddPosKFParticle(TA2KFParticle kfp){

  if(fNparti>fNpart){
    std::cout<<"TA2CBKinematicFitter::AddPosKFParticle already at max particles"<<std::endl;
    return;
  }

  //Add parameters to Alpha0
  fmAlpha0.SetSub(fNpari,0,kfp.GetAlpha());
  //Add error matrix to V_Alpha0
  fmV_Alpha0.SetSub(fNpari,fNpari,kfp.GetVAlpha());
  //Add transformation matrice to fT
  fT.SetSub(fNpari,fNpari,kfp.GetT());
  //ADD Lorentz Vector !!
  if(fNparti) fPtot=fPtot+kfp.Get4Vector();
  else fPtot=kfp.Get4Vector();

  //increment counters
  fNpari+=fNvar;
  fNparti++;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::AddNegKFParticle(TA2KFParticle kfp){

  if(fNparti>fNpart){
    std::cout<<"TA2CBKinematicFitter::AddPosKFParticle already at max particles"<<std::endl;
    return;
  }

  //Add parameters to Alpha0
  fmAlpha0.SetSub(fNpari,0,kfp.GetAlpha());
  //Add error matrix to V_Alpha0
  fmV_Alpha0.SetSub(fNpari,fNpari,kfp.GetVAlpha());
  //Add transformation matrice to fT
  fT.SetSub(fNpari,fNpari,kfp.GetT());
  //SUBTRACT Lorentz Vector !!
  if(fNparti) fPtot=fPtot-kfp.Get4Vector();
  else fPtot=kfp.Get4Vector();

  //increment counters
  fNpari+=fNvar;
  fNparti++;

}

//-----------------------------------------------------------------------------
TA2KFParticle TA2CBKinematicFitter::GetTotalFitParticle(){

  TA2KFParticle kfp;
  TMatrixD mtot(fNvar,1);

  //loop over the sub matrices in alpha and add to total
  for(Int_t i=0; i<fNpart; i++){
    mtot+=fmAlpha.GetSub(i*fNvar,(i+1)*fNvar-1,0,0);
  }

  //Set 4 vector, automatically sets alpha.
  kfp.Set4Vector(TLorentzVector(mtot[0][0],mtot[1][0],mtot[2][0],mtot[3][0]));
  // Add the error matrixes
  TMatrixD mV_tot(fNvar,fNvar);

  for(Int_t i=0; i<fNpart; i++){
    mV_tot+=fmV_Alpha.GetSub(i*fNvar,(i+1)*fNvar-1,i*fNvar,(i+1)*fNvar-1);
  }

  kfp.SetVAlpha(mV_tot);

  return kfp;

}

//-----------------------------------------------------------------------------
TA2KFParticle TA2CBKinematicFitter::GetParticle(Int_t ip){

  //Return the fitted particle that was added ith
  if(ip>fNpari){
    std::cout<<"TA2CBKinematicFitter::GetParticle particle not in fit"<<std::endl;
    return TA2KFParticle();
  }

  TA2KFParticle kfp;
  TMatrixD mi(fNvar,1);

  mi=fmAlpha.GetSub(ip*fNvar,(ip+1)*fNvar-1,0,0);
  kfp.Set4Vector(TLorentzVector(mi[0][0],mi[1][0],mi[2][0],mi[3][0]));
  TMatrixD mVi(fNvar,fNvar);
  mVi=fmV_Alpha.GetSub(ip*fNvar,(ip+1)*fNvar-1,ip*fNvar,(ip+1)*fNvar-1);
  kfp.SetVAlpha(mVi);
  return kfp;

}

//-----------------------------------------------------------------------------
TA2KFParticle TA2CBKinematicFitter::GetInitialParticle(Int_t ip){

  //Return the unfitted particle that was added ith
  if(ip>fNpari){
    std::cout<<"TA2CBKinematicFitter::GetInitialParticle particle not in fit"<<std::endl;
    return TA2KFParticle();
  }

  TA2KFParticle kfp;
  TMatrixD mi(fNvar,1);

  mi=fmAlpha0.GetSub(ip*fNvar,(ip+1)*fNvar-1,0,0);
  kfp.Set4Vector(TLorentzVector(mi[0][0],mi[1][0],mi[2][0],mi[3][0]));
  TMatrixD mVi(fNvar,fNvar);
  mVi=fmV_Alpha0.GetSub(ip*fNvar,(ip+1)*fNvar-1,ip*fNvar,(ip+1)*fNvar-1);
  kfp.SetVAlpha(mVi);
  return kfp;

}

//-----------------------------------------------------------------------------
void TA2CBKinematicFitter::Debug(){

  std::cout<<"Alpha0 "<<std::endl;
  fmAlpha0.Print();
  std::cout<<"Alpha "<<std::endl;
  fmAlpha.Print();
  std::cout<<"V_Alpha0 "<<std::endl;
  fmV_Alpha0.Print();
  std::cout<<"V_Alpha "<<std::endl;
  fmV_Alpha.Print();
  std::cout<<"d "<<std::endl;
  fmd.Print();
  std::cout<<"D "<<std::endl;
  fmD.Print();
  std::cout<<"V_D "<<std::endl;
  fmV_D.Print();
  std::cout<<"lamda "<<std::endl;
  fmlamda.Print();
  std::cout<<"T "<<std::endl;
  fT.Print();

  //Check D*deltaAlpha+d=0
  TMatrixD mdelAlpha=fmAlpha-fmAlpha0;
  TMatrix mCheck1=fmD*mdelAlpha + fmd;
  std::cout<<"delAlpha"<<std::endl;
  mdelAlpha.Print();
  std::cout<<"Check1 "<<std::endl;
  mCheck1.Print();

}
ClassImp(TA2CBKinematicFitter);
