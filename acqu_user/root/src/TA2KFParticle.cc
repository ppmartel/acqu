#include "TA2KFParticle.h"
#include <iostream>

/////////////////////////////////////////////////////////////
//TCBKFParticle
TA2KFParticle::TA2KFParticle(TLorentzVector lv,Double_t sig_th,Double_t sig_ph,Double_t sig_E){

  fNvar=4;
  flv=lv;

  //Set Alpha
  fAlpha.ResizeTo(fNvar,1);
  SetAlpha();
  //Get V_Alpha
  fV_Alpha.ResizeTo(fNvar,fNvar);

  //Polar-Cart Transfomation matrix
  fT.ResizeTo(fNvar,fNvar-1);
  Polar2Cartesian(sig_th,sig_ph,sig_E);
  //end
}
TA2KFParticle::TA2KFParticle(TLorentzVector *lv){

  fNvar=4;
  flv=*lv;

  //Set Alpha
  fAlpha.ResizeTo(fNvar,1);
  SetAlpha();
  //Get V_Alpha
  fV_Alpha.ResizeTo(fNvar,fNvar);

  //Polar-Cart Transfomation matrix
  fT.ResizeTo(fNvar,fNvar-1);
  //  Polar2Cartesian(sig_th,sig_ph,sig_E);
  //end
}
TA2KFParticle TA2KFParticle::Add(TA2KFParticle p1){
  if(p1.GetNVar()!=fNvar) {std::cout<<"TA2KFParticle::Add can't add, different Nvar"<<std::endl; return p1;}
  TA2KFParticle p12(TLorentzVector(0,0,1,1),1,1,1);
  p12.SetVAlpha(p1.GetVAlpha()+fV_Alpha);
  p12.Set4Vector(p1.Get4Vector()+flv);
  return p12;
}
TA2KFParticle TA2KFParticle::Subtract(TA2KFParticle p1){
  if(p1.GetNVar()!=fNvar) {std::cout<<"TA2KFParticle::Add can't add, different Nvar"<<std::endl; return p1;}
  TA2KFParticle p12(TLorentzVector(0,0,1,1),1,1,1);
  p12.SetVAlpha(p1.GetVAlpha()+fV_Alpha);
  p12.Set4Vector(p1.Get4Vector()-flv);
  return p12;
}
void TA2KFParticle::SetAlpha(){
  fAlpha[0][0]=flv.X();
  fAlpha[1][0]=flv.Y();
  fAlpha[2][0]=flv.Z();
  fAlpha[3][0]=flv.T();
}
TLorentzVector TA2KFParticle::Get4Vector(){
  flv.SetX(fAlpha[0][0]);
  flv.SetY(fAlpha[1][0]);
  flv.SetZ(fAlpha[2][0]);
  flv.SetT(fAlpha[3][0]);
  return flv;
}
void TA2KFParticle::Polar2Cartesian(Double_t sig_th,Double_t sig_ph,Double_t sig_E){
  Double_t sth=sin(flv.Theta());
  //Double_t E=flv.T();
  //Double_t P=flv.P();

  GetT();
  TMatrixD TT=fT;
  TT.T();
  //printf("TT\n");
  //TT.Print();

  TMatrixD Vprime(fNvar-1,fNvar-1);
  Double_t sigma_th=sig_th/180*TMath::Pi();
  Double_t sigma_ph;
  if(sth==0) sigma_ph=0;
  //else sigma_ph=sig_ph/180*TMath::Pi()/sth;
  else sigma_ph=sig_ph/180*TMath::Pi();
  Double_t sigma_E=sig_E;//Kinetic energy//TMath::Power(E-M,0.75);
  //std::cout<<"sigE "<<sigma_E<<" sigth "<<sigma_th<<" sigph "<<sigma_ph<<std::endl;
  //Construct polar error matrix
  Vprime[0][0]=sigma_th*sigma_th;  Vprime[0][1]=0;                  Vprime[0][2]=0; 
  Vprime[1][0]=0;                  Vprime[1][1]=sigma_ph*sigma_ph;  Vprime[1][2]=0;
  Vprime[2][0]=0;                  Vprime[2][1]=0;                  Vprime[2][2]=sigma_E*sigma_E;  
  //printf("Vprime\n");
  //Vprime.Print();
  //Construct cartesian matrix
  fV_Alpha=fT*Vprime*TT;
  //printf("fV_Alpha\n");
  //fV_Alpha.Print();
}
TMatrixD TA2KFParticle::GetT(){
  Double_t sth=sin(flv.Theta());
  Double_t cth=cos(flv.Theta());
  Double_t sph=sin(flv.Phi());
  Double_t cph=cos(flv.Phi());
  Double_t E=flv.T();
  Double_t P=flv.P();

  //Construct Transformation matrix
  fT[0][0]=P*cth*cph;      fT[0][1]=-P*sth*sph;   fT[0][2]=E/P*sth*cph;
  fT[1][0]=P*cth*sph;      fT[1][1]=P*sth*cph;    fT[1][2]=E/P*sth*sph;
  fT[2][0]=-P*sth;         fT[2][1]=0;            fT[2][2]=E/P*cth;
  fT[3][0]=0;              fT[3][1]=0;            fT[3][2]=1;
  //printf("fT\n");
  //fT.Print();
  return fT;
}

ClassImp(TA2KFParticle);
