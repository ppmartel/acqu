#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TFile.h"
#include "TF1.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TMath.h"
#include "TChain.h"
#include "TProfile.h"
#include "TNtuple.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "TStyle.h"
#include "TLine.h"
#include <iomanip>



using namespace std;

void set_plot_style()
{
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}

void Fit(TH1* h, Double_t& outPos, Double_t& outFWHM)
{
    // Perform fit.
    
    Char_t tmp[256];

    Double_t x1 = 50;
    Double_t x2 = 300;

    TF1* func = new TF1("func", "gaus(0)+pol2(3)", x1, x2);
    func->SetParameters(h->GetMaximum(), 135, 8, 1, 1, 0.1, 0.1);
    func->SetLineColor(kBlue);
    func->SetParLimits(1, 128, 143);
    func->SetParLimits(2, 5, 20);
    h->Fit(func, "RBQO");
    
    // get position and FWHM
    Double_t fPi0Pos = func->GetParameter(1);
    outPos = fPi0Pos;
    outFWHM = 2.35*func->GetParameter(2);

    // indicator line
    TLine line;
    line.SetLineWidth(2);
    line.SetX1(fPi0Pos);
    line.SetX2(fPi0Pos);
    line.SetY1(0);
    line.SetY2(h->GetMaximum());

    TF1* fBG = new TF1("funcBG", "pol2", x1, x2);
    for (Int_t i = 0; i < 3; i++) fBG->SetParameter(i, func->GetParameter(3+i));
   // for (Int_t i = 0; i < 6; i++) fBG->SetParameter(i, func->GetParameter(3+i));
    fBG->SetLineColor(kRed);

	delete func;
	delete fBG;
}

// create a new histogram- IM(gg) and Eg as function of a run number
// For each run number: get for each detector element average peak position and average energy. Fill upp ntuples
// When all runs have been looped through, plot as function of detector nr, 
void IMggAll_vs_runnr()
{


	string path = "analysis/2014-10/root/";

	TString pathtohist = "CaLib_CB_IM";
	ostringstream s;
	
	ofstream prtofile;
	prtofile.open("CB_gain_vs_det_vs_runnr4.dat");
	ofstream prtofileav;
	prtofileav.open("IMggAll_vs_runnr.dat");


	int ifirst_runnr = 6055; 
	int ilast_runnr = 6920;
	int ibinlow = 0;
	int ibinhigh = 720;

// Creating 720 TH2F histograms which will contain the IM(gg) All versus detector nr.
	TH2F** detnr = new TH2F*[720];
	for (int i = 0; i < 720; i++)
	{
		detnr[i] = new TH2F( Form("detnr%03d", i), Form("detnr%03d", i), (ilast_runnr-ifirst_runnr + 20), ifirst_runnr-10, ilast_runnr+10, 400, 120., 150. );	
	}


	std::vector<int>	fileNumbers;
	for (int irunnr = ifirst_runnr; irunnr <= ilast_runnr; irunnr++)
	{
		s << path << "Hist_CBTaggTAPS_" << irunnr << ".root";
		TString file1 = s.str();
		TFile f( file1, "READ" );
		if(f.IsOpen())
		{
			fileNumbers.push_back(irunnr);
			f.Close();
		}
		s.str("");
	}




	std::vector<int>::iterator it = fileNumbers.begin();
	for( int irunnr = ifirst_runnr; irunnr <= ilast_runnr; irunnr++ )
	{	
		if( irunnr == *(it) &&  it != fileNumbers.end() )
  		{
  		
	  		prtofile << *(it) << "\t" ;
	  		prtofileav << *(it) << "\t" ;

			TH2F* summedHist = new TH2F("summedHist", "summedHist", 500, 0, 1000, 720, 0, 720);
			int count = 0;
			std::cout << *(it) << std::endl;
			for(int i=-2; i<3; i++)
			{
				if( (it+i) < fileNumbers.begin() ) continue;
				if( (it+i) >= fileNumbers.end() )	continue;
				
				count++;
				s << path << "Hist_CBTaggTAPS_" << *(it+i) << ".root";
				TString file1 = s.str();
				TFile f( file1, "READ" );
				if(f.IsOpen())
				{
					TH2F *rec1 = (TH2F*)f.Get(pathtohist);
					if(rec1)
						summedHist->Add(rec1);
					f.Close();
				}
				s.str("");
			}
				TH1F* prpi0all = (TH1F*)summedHist->ProjectionX( "IM(#gamma#gamma) (MeV)", 0, 720)->Clone();
				if( prpi0all->Integral() == 0 ) 
				{
					std::cout << "sth is wrong in run " << (*it) <<  std::endl;
				}	
				Double_t mean[2];
//				prpi0all->Rebin(2);
				Fit(prpi0all, mean[0], mean[1]);
				prtofileav << std::setprecision(5) << mean[0] << "\n" ;
				for( int ibn = ibinlow; ibn < ibinhigh; ibn++)
				{
					
					TH1F* prpi0av = (TH1F*)summedHist->ProjectionX( "IM(#gamma#gamma) (MeV)", ibn+1, ibn+1)->Clone();

				// insert the average over the full peak projection here
				if( prpi0av->Integral() == 0 ) 
				{

	//				std::cout << "sth is wrong in run " << (*it) << " element number " << ibn <<  std::endl;
					detnr[ibn]->Fill( *(it), 145.00 );
					prtofile << std::setprecision(5) << "1.0000" << "\t" ;
					
				}
				else
				{
					Double_t help[2];
//					prpi0av->Rebin(2);
					Fit(prpi0av, help[0], help[1]);
				
					if( (help[0] > 128.5) && (help[0] < 142.0) )
					{	

						detnr[ibn]->Fill( *(it), help[0] );
						prtofile << std::setprecision(5) <<	Double_t( (134.9766*mean[0])/(help[0]*help[0]) ) << "\t" ;					
					}
					else
					{
						detnr[ibn]->Fill( *(it), 145.00 );
						prtofile << std::setprecision(5) << "1.0000" << "\t" ;
					}

					if(prpi0av) delete prpi0av;
				}
								
			}

			prtofile << "\n" ;
			if(summedHist) delete summedHist;
			++it;
		}
		else
		{
			prtofile << irunnr << "\t" ;
	  		prtofileav << irunnr  << "\t" ;
	  		prtofileav << std::setprecision(5) << 134.9766 << "\n" ;
	  		for( int k = 0; k < 720 ; k++ )
	  		{
	  			prtofile << std::setprecision(5) << "1.0000" << "\t" ;
	  		}
	  		prtofile << "\n" ;
	  	}

	}

	prtofile.close();
	prtofileav.close();

	// Draws the histograms for all detector elements of floating average vs run number;  
	int nCanvas = 45;
	TCanvas** CCAC = new TCanvas*[nCanvas];
	for (int i = 0; i < nCanvas; i++)
	{
		CCAC[i] = new TCanvas(Form("CCAC%02d", i), Form("CCAC%02d", i), 200, 10, 1000, 700);
		CCAC[i]->SetFillColor(18);
		CCAC[i]->Divide(4,4);
		for( int k = 1; k <= 16; k++)
		{
			int inr = 720 - (k + 16*i);
		//	int inr = (k + 16*i);
			CCAC[i]->cd(17-k);
			gPad->SetGridx();
			gPad->SetGridy();
			detnr[inr]->Draw("BOX");
		}
		CCAC[i]->SaveAs(Form("iter4_%02d.eps", i));
	}
	for(int i = 0; i < nCanvas; i++)
		delete CCAC[i];
	delete CCAC;
	exit(0);
}