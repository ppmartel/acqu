void CheckLinPol(TCanvas *canvLinPol) {
  if(canvLinPol==NULL) {
    return;
  }
 
  //gStyle->SetOptFit(1);
  //gStyle->SetOptStat(0);
  canvLinPol->Range(-79.0074,-0.373242,79.3233,0.359577);
  canvLinPol->SetFillColor(10);
  canvLinPol->SetFillStyle(4000);
  canvLinPol->SetBorderSize(0);
  canvLinPol->SetFrameFillColor(10);
  canvLinPol->SetFrameFillStyle(0);
  canvLinPol->SetBottomMargin(0.12);
  canvLinPol->SetTopMargin(0.03);
  //canvLinPol->SetStatFormat("5.3g");

  TA2LinearPolEpics* pLP = (TA2LinearPolEpics*)(gAN->GetChild("LinPol"));

  char* hnames[] = {"LinPol_Incoherent",			  
		    "LinPol_Coherent",			  
		    "LinPol_CoherentPara",
		    "LinPol_CoherentPerp",
		    "LinPol_Enhancement",
		    "LinPol_EnhancementPara",
		    "LinPol_EnhancementPerp",
		    "LinPol_CohEdge",
		    "LinPol_CohEdgePara",
		    "LinPol_CohEdgePerp",
		    "LinPol_GatedIncoherent",
		    "LinPol_GatedCoherent",
		    "LinPol_GatedCoherentPara",
		    "LinPol_GatedCoherentPerp",
		    "LinPol_GatedCurrEnh",
		    "LinPol_GatedCurrEnhPara",
		    "LinPol_GatedCurrEnhPerp",
		    "LinPol_CohEdgeGated",
		    "LinPol_CohEdgeGatedPara",
		    "LinPol_CohEdgeGatedPerp",
		    "LinPol_PolTableEnh",
		    "LinPol_PolTablePol"};

  TLatex *texPARA= new TLatex(0.63,0.85,"PARA");
  texPARA->SetNDC();
  texPARA->SetTextColor(2);
  TLatex *texPERP= new TLatex(0.63,0.8,"PERP");
  texPERP->SetNDC();
  texPERP->SetTextColor(4);
  TLatex *texAMO= new TLatex(0.63,0.75,"AMO");
  texAMO->SetNDC();
  texAMO->SetTextColor(3);
  TLatex *texFree= new TLatex(0.45,0.85,"Free");
  texFree->SetNDC();
  TLine *LFree = new TLine(0.38,0.87,0.44,0.87);
  LFree->SetNDC();
  LFree->SetLineStyle(1);
  TLatex *texGated= new TLatex(0.45,0.8,"Gated");
  texGated->SetNDC();
  texGated->SetLineStyle(2);
  TLine *LGated = new TLine(0.38,0.82,0.44,0.82);
  LGated->SetNDC();
  LGated->SetLineStyle(2);

  //canvLinPol->SetFillStyle(4000);
  canvLinPol->Divide(3,3,0.01,0.01);
  TH1F* his;
  
  //Para or perp?
  Int_t mode=pLP->GetPolPlane();
  //cout<<"Current mode "<<mode<<endl;
  
  //////////////////////////////////////////////////////
  /////////LinPol_Incoherent
  canvLinPol->cd(1);
  his=(TH1F*)gROOT->Get(hnames[0]);
  his->SetTitle("TA2LinearPolEpics::Incoherent");
  his->SetStats(false);
  his->SetFillColor(kBlue+2);
  his->Draw("hist");

  /////////LinPol_Coherent/Para/Perp
  canvLinPol->cd(2);
  if(mode==0)
    {
      his=(TH1F*)gROOT->Get(hnames[2]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetTitle("TA2LinearPolEpics::Coherent");
      his->SetFillColor(2);
      his->Draw();
      texPARA->Draw("hist");
    }
  else if (mode==1)
    {
      his=(TH1F*)gROOT->Get(hnames[3]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetTitle("TA2LinearPolEpics::Coherent");
      his->SetFillColor(4);
      his->Draw();
      texPERP->Draw("hist");
    }
  else if (mode==2)
    {
      his=(TH1F*)gROOT->Get(hnames[1]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetTitle("TA2LinearPolEpics::Coherent");
      his->SetFillColor(3);
      his->Draw("hist");
      texAMO->Draw();
    }
	
  /////////LinPol_Enhancement/Para/Perp
  canvLinPol->cd(3);
  if(mode==0)
    {
      his=(TH1F*)gROOT->Get(hnames[5]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetTitle("TA2LinearPolEpics::Enhancement");
      his->SetFillColor(2);
      his->SetMinimum(50);
      his->Draw("hist");
      texPARA->Draw();
    }
  else if (mode==1)
    {
      his=(TH1F*)gROOT->Get(hnames[6]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetTitle("TA2LinearPolEpics::Enhancement");
      his->SetFillColor(4);
      his->SetMinimum(50);
      his->Draw("hist");
      texPERP->Draw();
    }
  else if (mode==2)
    {
      his=(TH1F*)gROOT->Get(hnames[4]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetTitle("TA2LinearPolEpics::Enhancement");
      his->SetFillColor(3);
      his->SetMinimum(50);
      his->Draw("hist");
      texAMO->Draw();
    }
  
  /////////LinPol_GatedInc
   canvLinPol->cd(4);
  his=(TH1F*)gROOT->Get(hnames[10]);
  his->SetStats(false);
  his->SetTitle("TA2LinearPolEpics::GatedIncoherent");
  his->SetFillColor(kBlue+2);
  his->Draw("hist");

  /////////LinPol_GatedCoh/Para/Perp
  canvLinPol->cd(5);
  if(mode==0)
    {
      his=(TH1F*)gROOT->Get(hnames[12]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetTitle("TA2LinearPolEpics::GatedCoherent");
      his->SetFillColor(2);
      his->Draw("hist");
      texPARA->Draw();
    }
  else if (mode==1)
    {
      his=(TH1F*)gROOT->Get(hnames[13]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetTitle("TA2LinearPolEpics::GatedCoherent");
      his->SetFillColor(4);
      his->Draw("hist");
      texPERP->Draw();
    }
  else if (mode==2)
    {
      his=(TH1F*)gROOT->Get(hnames[11]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetTitle("TA2LinearPolEpics::GatedCoherent");
      his->SetFillColor(3);
      his->Draw("hist");
      texAMO->Draw();
    }
	
  /////////LinPol_GatedEnh/Para/Perp
  canvLinPol->cd(6);
  if(mode==0)
    {
      his=(TH1F*)gROOT->Get(hnames[15]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetLineStyle(1);
      his->SetTitle("TA2LinearPolEpics::GatedEnhancement");
      his->SetFillColor(2);
      his->SetMinimum(50);
      his->SetMaximum(350);
      his->Draw("hist");
      texPARA->Draw();
    }
  else if (mode==1)
    {
      his=(TH1F*)gROOT->Get(hnames[16]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetLineStyle(1);
      his->SetTitle("TA2LinearPolEpics::GatedEnhancement");
      his->SetFillColor(4);
      his->SetMinimum(50);
      his->SetMaximum(350);
      his->Draw("hist");
      texPERP->Draw();
    }
  else if (mode==2)
    {
      his=(TH1F*)gROOT->Get(hnames[14]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetLineStyle(1);
      his->SetTitle("TA2LinearPolEpics::GatedEnhancement");
      his->SetFillColor(3);
      his->SetMinimum(50);
      his->SetMaximum(350);
      his->Draw("hist");
      texAMO->Draw();
    }

  /////////LinPol_CohEdge
  canvLinPol->cd(7);
  if(mode==0)
    {
      his=(TH1F*)gROOT->Get(hnames[8]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetLineStyle(1);
      his->SetMarkerStyle(21);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(2);
      his->SetTitle("TA2LinearPolEpics::CohEdge");
      his->Draw("pe1");
      his->SetMarkerStyle(25);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(2);
      his=(TH1F*)gROOT->Get(hnames[18]);
      his->SetStats(false);
      his->SetLineColor(2);
      his->SetLineStyle(2);
      
      his->Draw("SAME,pe1");

      LFree->Draw();
      LGated->Draw();
      texGated->Draw();
      texFree->Draw();
      texPARA->Draw();
    }
  else if (mode==1)
    {
      his=(TH1F*)gROOT->Get(hnames[9]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetLineStyle(1);
      his->SetMarkerStyle(21);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(4);
      his->SetTitle("TA2LinearPolEpics::CohEdge");
      his->Draw("pe1");

      his=(TH1F*)gROOT->Get(hnames[19]);
      his->SetStats(false);
      his->SetLineColor(4);
      his->SetLineStyle(2);
      his->SetMarkerStyle(25);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(4);
      his->Draw("SAME,pe1");

      LFree->Draw();
      LGated->Draw();
      texGated->Draw();
      texFree->Draw();
      texPERP->Draw();
    }
  else if (mode==2)
    {
      his=(TH1F*)gROOT->Get(hnames[7]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetLineStyle(1);
      his->SetMarkerStyle(21);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(3);
      his->SetTitle("TA2LinearPolEpics::CohEdge");
      his->Draw("pe1");

      his=(TH1F*)gROOT->Get(hnames[17]);
      his->SetStats(false);
      his->SetLineColor(3);
      his->SetLineStyle(2);
      his->SetMarkerStyle(25);
      his->SetMarkerSize(0.5);
      his->SetMarkerColor(3);
      his->Draw("SAME,pe1");

      LFree->Draw();
      LGated->Draw();
      texGated->Draw();
      texFree->Draw();
      texAMO->Draw();
    }
  
  /////////LinPol_PolTableEnh
  canvLinPol->cd(8);
  his=(TH1F*)gROOT->Get(hnames[20]);
  his->SetStats(false);
  his->SetTitle("TA2LinearPolEpics::PolTableEnh");
  his->SetFillColor(kBlue+2);
  his->Draw();
	
  /////////LinPol_PolTablePol
  canvLinPol->cd(9);
  his=(TH1F*)gROOT->Get(hnames[21]);
  his->SetStats(false);
  his->SetTitle("TA2LinearPolEpics::PolTablePol");
  his->SetFillColor(kBlue+2);
  his->Draw();
	
}
