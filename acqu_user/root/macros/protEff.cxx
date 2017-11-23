#include <iostream>
#include <sstream>
#include <string>
#define PI 3.14159265359
#define D2R 3.14159265359/180
#define piMass 134.277
#define protMass 938.977


Double_t sqr(Double_t IN)
	{
	return IN*IN;
	}

Double_t fractError(Double_t first,Double_t second,Double_t eFirst,Double_t eSecond)
	{
	Double_t temp1 = sqr(eFirst/first) + sqr(eSecond/second);
	return sqrt(temp1);
	}

Double_t shouldBe(Double_t Pi0Th,Double_t Pi0En,Double_t ProtEn)
	{
	Double_t temp =Pi0En*sin(Pi0Th*D2R)/ProtEn;
	cout << temp << endl;
	return asin(temp)/D2R;
	}

Double_t whereShouldBe(Double_t phIn,Double_t piEn,Double_t piTh)
	{
	TLorentzVector photIn,pionOut,protIn,protOut;
	photIn.SetPxPyPzE(0,0,phIn,phIn);
	protIn.SetPxPyPzE(0,0,0,protMass);
	Double_t temp = piTh*D2R;
	pionOut.SetPxPyPzE(0,piEn*sin(temp),piEn*cos(temp),sqrt(sqr(piEn)+sqr(piMass)));
	protOut = photIn+protIn-pionOut;
	return protOut.Theta()/D2R;
	}

void protEff()
	{

	//LOAD TAGGER	
	cout << "Loading Tagger Channels!\n";
	Double_t tEnergy[352];
	ifstream myFile("tagent855.dat");
	string line,nEnergy,chanStr;
	Int_t nChan;
	if (myFile.is_open())
		{
		while (myFile.good())
			{
			getline(myFile,line);
			ostringstream conv;

			for(int a=0;a<3;a++)
				{
				conv << line[a];
				}
			chanStr = conv.str();
			istringstream(chanStr) >> nChan;
			ostringstream conv2;
			for (int b=0;b<5;b++)
				{
			   if (!(line[b+4]==' '))
					{
					conv2 << line[b+4];
					}
				}
			nEnergy = conv2.str();
			istringstream(nEnergy) >> tEnergy[nChan];
			if (nChan==351)
				{break;}
			}
		cout << "\nTagger channels loaded!!!!\n\n";
		}
	else
		{
		cout << "Something went very wrong!!! GOOD LUCK!\n";
		}
	myFile.close();


	//SET THE FILE FOR ANALYSIS AND TREE BRANCHES

	TFile *funFile = new TFile("/work0/jcrowe/goodStuff/TA2Pi0ComptonMC_Pi0250.root");
	//TFile *funFile = new TFile("/work0/jcrowe/goodStuff/TA2Pi0Compton2008.root");
	TTree *funTree = (TTree*)funFile->Get("Pi0ComptonTree");

	Int_t NPi0,NProton,NHits=0,NMiss=0;
	Double_t MMPi0Rand[1000],MMPi0Prompt[1000],TaggEnergy[1000],ProtTheta[1000],ProtPhi[1000],ProtEnergy[1000],Pi0Theta[1000],Pi0Phi[1000],Pi0Energy[1000];
	Int_t TaggerChannel[1000],NPi0Prompt,NPi0Random;
	Double_t temp,thresh=10.00;

	//SOME HISTOGRAMS

	TH1F *whereHit = new TH1F("whereHit","Hit Angle",80,0,80);
	TH1F *whereMiss = new TH1F("whereShould","Missed Angle",80,0,80);
	TH1F *whereDenom = new TH1F("whereDenom","Total",80,0,80);
	TH1F *openAngle = new TH1F("Angle Difference","Open Angle",50,0,50);
	TH1F *MMassPrompt = new TH1F("Missing mass Prompt","Missing Mass",200,800,1200);
	TH1F *MMassRandom = new TH1F("Missing Mass 2","Pion & Proton Detected",200,800,1200);
	TH1F *MMassReal = new TH1F("Missing Mass 3","Open Angle Cut",200,800,1200);

	funTree->SetBranchAddress("TaggerChannel",TaggerChannel);
	funTree->SetBranchAddress("NPi0",&NPi0);
	funTree->SetBranchAddress("NProton",&NProton);
	funTree->SetBranchAddress("ProtonTheta",&ProtTheta);
	funTree->SetBranchAddress("ProtonEnergy",&ProtEnergy);
	funTree->SetBranchAddress("Pi0Theta",&Pi0Theta);
	funTree->SetBranchAddress("Pi0Energy",&Pi0Energy);
	funTree->SetBranchAddress("Pi0Phi",&Pi0Phi);
	funTree->SetBranchAddress("ProtonPhi",&ProtPhi);
	funTree->SetBranchAddress("MissingMassRandomPi0",&MMPi0Rand);
	funTree->SetBranchAddress("MissingMassPromptPi0",&MMPi0Prompt);

	Int_t nEvents = (Int_t)funTree->GetEntries();

	for (Int_t a;a<nEvents;a++)
		{		
		funTree->GetEvent(a);
		
		if (NPi0>0)
			{
			MMassPrompt->Fill(MMPi0Prompt[0]);
			if (NProton>0)
				{
				temp = whereShouldBe(Pi0Theta[0],Pi0Energy[0],tEnergy[TaggerChannel[0]]);
				openAngle->Fill(abs(temp-ProtTheta[0]));
				if ((temp-ProtTheta[0])<10)
					{
					whereHit->Fill(ProtTheta[0]);
					}
				}
			else
				{
				temp = whereShouldBe(Pi0Theta[0],Pi0Energy[0],tEnergy[TaggerChannel[0]]);
				whereShould->Fill(temp);
				}
			}		
		}

	TH1F *ratio = (TH1F*)whereHit->Clone("ratio");
	whereDenom->Add(whereHit);
	whereDenom->Add(whereShould);
	ratio->Divide(whereDenom);
	ratio->SetTitle("found/expected");
	
	Int_t NHit,NDen;
	Double_t EHit,EDen,ERatio;
	for (int b=0;b<80;b++)
		{
		NHit = whereHit->GetBinContent(b);
		NDen = whereDenom->GetBinContent(b);
		EHit = whereHit->GetBinError(b);
		EDen = whereDenom->GetBinError(b);
				
		ERatio = ratio->GetBinContent(b);
		if (!(NHit==0)&&!(NDen==0))
			{
			ERatio = ERatio*fractError(NHit,NDen,EHit,EDen);
			cout << ERatio << "   ";
			ratio->SetBinError(b,ERatio);
			}
		else
			{
			ERatio = 0;
			}
		if ((b%5)==0)
			{cout << endl;}
		}

	TCanvas *funCanvas = new TCanvas("funCanvas","Awesome",1);
	funCanvas->Divide(3,2);
	funCanvas->cd(1);
		whereHit->Draw("E");
	funCanvas->cd(2);
		whereMiss->Draw("E");
	funCanvas->cd(3);
		ratio->Draw("E");
	funCanvas->cd(4);
		openAngle->Draw("hist");
	funCanvas->cd(5);
		MMassPrompt->Draw("hist");
	}

