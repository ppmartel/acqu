//--Author	JRM Annand    9th Jan 2003
//--Rev		JRM Annand...26th Feb 2003...1st production version
//--rev 	JRM Annand...28th Feb 2003...Separate from TA2System
//--Rev 	JRM Annand...15th Oct 2003...Upgrade Setup1D methods
//--Rev 	JRM Annand... 4th Feb 2004...General display parse, data-cuts
//--Rev 	JRM Annand...30th Mar 2004...2D names scaler plots
//--Rev 	JRM Annand...27th Jun 2004...Rate/multiscaler stuff
//--Rev 	JRM Annand... 5th Oct 2004...DecodeSaved()
//--Rev 	JRM Annand...13th Apr 2005...Cuts,Weights,3D hist,Tidy code
//--Rev 	JRM Annand...10th May 2005...'.' separator cut, weight names
//--Rev 	JRM Annand....6th Jul 2006...ZeroAll() also children
//--Update	JRM Annand....8th Nov 2008...Non-const channel widths
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2HistManager
//
// Contains methods for...
// Setup, filling and plotting of histograms
// All Acqu-Root analysis classes requiring histograms inherit from this one.
// It cannot itself be instantiated...pure virtual SetConfig().

#include "TA2HistManager.h"
#include "TFile.h"                           // ROOT file
#include "ARFile_t.h"


//-----------------------------------------------------------------------------
TA2HistManager::TA2HistManager(const Char_t* name, const Map_t* commlist,
			       const Map_t* histlist ):
  TA2DataManager( name, commlist )
{
  // Import string-to-key maps:
  // commlist, the list of key strings for command-line control,
  // histlist the list of valid histogram names for any display
  // from the calling constructor of the derived class
  // Initialise local variables OFF/not defined...default display OFF

  fHistList = histlist;
  f1Dhist = NULL;
  f2Dhist = NULL;
  f3Dhist = NULL;
  fHistCuts = NULL;
  fN1Dhist = fN2Dhist = fN3Dhist = 0;
  fIsDisplay = EFalse;
}

//-----------------------------------------------------------------------------
TA2HistManager::~TA2HistManager()
{
  // Delete histograms in 1D and 2D lists and then delete the lists
  if( f1Dhist ){
    f1Dhist->Delete();
    delete f1Dhist;
  }
  if( f2Dhist ){
    f2Dhist->Delete();
    delete f2Dhist;
  }
  if( f3Dhist ){
    f2Dhist->Delete();
    delete f3Dhist;
  }
}

//-----------------------------------------------------------------------------
void TA2HistManager::ParseDisplay( const Char_t* line )
{
  // General-purpose histogram display setup
  // Pull out histogram information from "Display:" command line
  // Check initialisation completed first and do it if not.
  // After working out what's desired, from the input line
  // the generic Setup1D(), Setup2D() or Setup3D() functions are called.

  Int_t k;
  Double_t lx,ly,lz,hx,hy,hz;           // low/high limits of X/Y/Z axes
  Int_t cx,cy,cz;                       // # bins in X/Y/Z axes	
  Int_t imin, imax;                     // min/max array elements to hist
  Int_t ix,iy,iz;                       // x,y,z single array element offset
  Char_t histspec[EMaxName];		// contains 1/2/3D spec.
  Char_t varX[EMaxName];	       	// X-variable name
  Char_t varY[EMaxName];	       	// Y-variable name
  Char_t varZ[EMaxName];	       	// Y-variable name
  Char_t varC[EMaxName];	       	// Cut-variable name
  Char_t varW[EMaxName];	        // Accumulate weight variable name
  Char_t cutname[EMaxName];		// Cut name
  Char_t wgtname[EMaxName];		// Weight variable name
  Char_t chName[EMaxName];               // Name of x channel limits file
  Double_t* xBuff;                      // x channel limits (non-const width)
  Double_t* yBuff;                      // x channel limits (non-const width)
  Char_t type[4];                         // 1D,2D,3D
  Int_t typeX,typeY,typeXY;             // single/multi types
  Bool_t isIntX, isIntY, isIntZ;        // integer variables
  TA2Cut* cut = NULL;                   // no cut default
  Stat_t* wgt = NULL;;                  // weight variable default none

  if( !fIsInit ){
    PrintMessage("Running PostInitialise before display setup\n\n");
    PostInitialise();
  }
  // Read the input line and work out the basics
  // Look for a match of the histogram X name from the command line
  // with the previously loaded name-variable association
  Int_t nscan = sscanf(line, "%s%s",type, varX);
  if( !GetN2V(varX) ){
    PrintError(line,"<Unknown X-variable for histogram>");
    return;
  }
  typeX = GetVarType(varX);
  isIntX = IsInt(varX);
  *cutname = 0;                         // ensure null string
  *wgtname = 0;                         // ditto
  *varW = 0;                            // ditto
  *varC = 0;                            // ditto
  imin = imax = -1;                     // default no range of indices
  xBuff = yBuff = NULL;                 // default no variable bin width

  // If its a 1D histogram get further variables
  // #chan, axis-low, axis-high, range of indices, cut-name, weight-variable
  // If axis-low, axis-high are not read correctly try
  // #chan, axis-file, range of indices, cut-name, weight-variable
  // for variable channel width
  if( !strcmp(type,"1D") ){  
    nscan = sscanf(line, "%*s%*s%d%lf%lf%d%d%s%s",
		   &cx,&lx,&hx,&imin,&imax,cutname,wgtname);
    // try variable bin width
    if( nscan < 3 ){
      nscan = sscanf(line, "%*s%*s%d%s%d%d%s%s",
		     &cx,chName,&imin,&imax,cutname,wgtname);
      // nothing worked...ignore 1D spectrum request
      if( nscan < 2 ){
	PrintError(line,"<1D spectrum line format>");
	return;
      }
      ReadChanLim(cx, &xBuff, chName);
    }
    // check for histogram cut, add '.' separator if it exists
    if( (cut = fHistCuts->FindCut(cutname)) ) sprintf(varC,".%s",cutname);
    // check for weight variable, add '.' separator if it exists
    if( (wgt = GetVar<Stat_t>(wgtname)) ) sprintf(varW,".%s",wgtname);
    // setup of 1D plot(s)...imin=-1 turns off any indices
    for( k=imin; k<=imax; k++ ){
      if(imin >= 0)
	sprintf(histspec,"%s%d%s%s %d %lf %lf",varX,k,varC,varW,cx,lx,hx);
      else{
	sprintf(histspec,"%s%s%s %d %lf %lf",varX,varC,varW,cx,lx,hx);
	k = 0;
      }
      if( isIntX )
	Setup1D( histspec, GetVar<Int_t>(varX)+k, typeX, xBuff, cut, wgt );
      else
	Setup1D( histspec, GetVar<Double_t>(varX)+k, typeX, xBuff, cut, wgt );
    }
  }
  // If its a 2D histogram get further variables
  // #X-chan, X-axis-low, X-axis-high, X-index
  // Y-variable #Y-chan, Y-axis-low, Y-axis-high, Y-index
  // cut-name, weight-variable
  // Variable bin width options added 4v3 EITHER x OR y axis has variable
  // bin widths. BOTH is not allowed
  else if( !strcmp(type,"2D") ){
    if( (nscan = sscanf(line, "%*s%*s%d%lf%lf%d%s%d%lf%lf%d%s%s",
			&cx,&lx,&hx,&ix,varY,&cy,&ly,&hy,&iy,cutname,wgtname))
	< 9 ){
      // Check variable y bin width
      nscan = sscanf(line, "%*s%*s%d%lf%lf%d%s%d%s%d%s%s",
		     &cx,&lx,&hx,&ix,varY,&cy,chName,&iy,cutname,wgtname);
      if( nscan >= 8 ) ReadChanLim(cy, &yBuff, chName);
      // Wasn't variable y so check for variable x bin width
      else{
	nscan = sscanf(line, "%*s%*s%d%s%d%s%d%lf%lf%d%s%s",
		       &cx,chName,&ix,varY,&cy,&ly,&hy,&iy,cutname,wgtname);
	// Nothing worked give up and ignore 2D plot request
	if( nscan < 8 ){
	  PrintError(line,"<2D spectrum line format>");
	  return;
	}
	ReadChanLim(cx, &xBuff, chName);
      }
    }
    // Check for histogram cut, add '.' separator if it exists
    if( (cut = fHistCuts->FindCut(cutname)) ) sprintf(varC,".%s",cutname);
    // check for weight variable, add '.' separator if it exists
    if( (wgt = GetVar<Stat_t>(wgtname)) ) sprintf(varW,".%s",wgtname);
    // check Y variable
    if( !GetN2V(varY) ){
      PrintError(line,"<Unknown Y-variable for 2D histogram>");
      return;
    }
    typeY = GetVarType(varY);
    isIntY = IsInt(varY);
    // Concatenate the Y_v_X name, if array element (ix, iy = -1) add array
    // index to X or Y name
    if( (ix < 0) && (iy < 0) ){
      ix = iy = 0;               // X, Y variables scaler
      sprintf(histspec,"%s_v_%s%s%s %d %lf %lf %d %lf %lf",
	      varY, varX, varC, varW, cx,lx,hx,cy,ly,hy);
    }
    else if( ix < 0 ){
      ix = 0;
      sprintf(histspec,"%s%d_v_%s%s%s %d %lf %lf %d %lf %lf",
	      varY, iy, varX, varC, varW, cx,lx,hx,cy,ly,hy);
    }
    else if( iy < 0 ){
      iy = 0;
      sprintf(histspec,"%s_v_%s%d%s%s %d %lf %lf %d %lf %lf",
	      varY, varX, ix, varC, varW, cx,lx,hx,cy,ly,hy);
    }
    else sprintf(histspec,"%s%d_v_%s%d%s%s %d %lf %lf %d %lf %lf",
		 varY, iy, varX, ix, varC, varW, cx,lx,hx,cy,ly,hy);

    // determine the single/multiple-value  X-Y combination
    // MultiX-SingleY, MultiY-SingleX, SingleX-SingleY
    if( (typeX == EMultiX) && (typeY == EMultiX ) ) typeXY = EMultiXY;
    else if( typeX == EMultiX ) typeXY = typeX;
    else if( typeY == EMultiY ) typeXY = typeY;
    else typeXY = ESingleX;
    // Create via the Setup2D method...work out variable types for compiler
    if( isIntX && isIntY )
      Setup2D( histspec, GetVar<Int_t>(varX)+ix,
	       GetVar<Int_t>(varY)+iy,typeXY, xBuff, yBuff, cut, wgt );
    else if( isIntX && !isIntY )
      Setup2D( histspec, GetVar<Int_t>(varX)+ix,
	       GetVar<Double_t>(varY)+iy,typeXY, xBuff, yBuff, cut, wgt );
    else if( !isIntX && isIntY )
      Setup2D( histspec, GetVar<Double_t>(varX)+ix,
	       GetVar<Int_t>(varY)+iy,typeXY, xBuff, yBuff, cut, wgt );
    else
      Setup2D( histspec, GetVar<Double_t>(varX)+ix,
	       GetVar<Double_t>(varY)+iy,typeXY, xBuff, yBuff, cut, wgt );
  }
  // If its a 3D histogram get further variables
  // #X-chan, X-axis-low, X-axis-high, X-index
  // Y-variable #Y-chan, Y-axis-low, Y-axis-high, Y-index
  // Z-variable #Z-chan, Z-axis-low, Z-axis-high, Z-index
  // cut-name, weight-variable
  else if( !strcmp(type,"3D") ){
    // 3D plot X-Y-Z scaler variable or single array element
    if( (nscan = sscanf(line, "%*s%*s%d%lf%lf%d%s%d%lf%lf%d%s%d%lf%lf%d%s%s",
			&cx,&lx,&hx,&ix,varY,&cy,&ly,&hy,&iy,
			varZ,&cz,&lz,&hz,&iz,cutname,varW)) < 14 ){
      PrintError(line,"<3D spectrum line format>");
      return;
    }
    // check for histogram fill condition, add '.' separator if it exists
    if( (cut = fHistCuts->FindCut(cutname)) ) sprintf(varC,".%s",cutname);
    // check for weight variable, add '.' separator if it exists
    if( (wgt = GetVar<Stat_t>(wgtname)) ) sprintf(varW,".%s",wgtname);
    // check Y and Z variables
    if( (!GetN2V(varY)) || (!GetN2V(varZ)) ){
      PrintError(line,"<Unknown Y or Z-variable for 3D histogram>");
      return;
    }
    // check variable types (single/multi valued)
    if( (GetVarType(varY) != typeX) || (GetVarType(varZ) != typeX) ){
      PrintError(line,"<3D histogram...X/Y/Z types must be identical>");
      return;
    }
    // Check for indexing of variables....so that the indices can
    // be incorporated in the name of the histogram
    if( ix < 0 ){
      if( iy < 0 ){
	if( iz < 0 )                                // no indices
	  sprintf(histspec,"%s_v_%s_v_%s%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,varY,varX,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
	else                                        // z index only
	  sprintf(histspec,
		  "%s%d_v_%s_v_%s%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,iz,varY,varX,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
      }
      else{
	if( iz < 0 )                                // y index only
	  sprintf(histspec,
		  "%s_v_%s%d_v_%s%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,varY,iy,varX,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
	else                                        // y and z indices
	  sprintf(histspec,
		  "%s%d_v_%s%d_v_%s%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,iz,varY,iy,varX,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
      }
    }
    else{
      if( iy < 0 ){
	if( iz < 0 )                                // x index only
	  sprintf(histspec,
		  "%s_v_%s_v_%s%d%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,varY,varX,ix,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
	else                                        // x and z indices
	  sprintf(histspec,
		  "%s%d_v_%s_v_%s%d%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,iz,varY,varX,ix,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
      }
      else{
	if( iz < 0 )                                // x and y indices
	  sprintf(histspec,
		  "%s_v_%s%d_v_%s%d%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,varY,iy,varX,ix,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
	else                                        // x, y and z indices
	  sprintf(histspec,
		  "%s%d_v_%s%d_v_%s%d%s%s %d %lf %lf %d %lf %lf %d %lf %lf",
		  varZ,iz,varY,iy,varX,ix,varC,varW,cx,lx,hx,cy,ly,hy,cz,lz,hz);
      }
    }
    // Create 3D histogram X/Y/Z = Double
    isIntY = IsInt( varY ); isIntZ = IsInt( varZ );
    if( isIntX ){                                   // integer X
      if( isIntY ){                                 // integer Y
	if(isIntZ) Setup3D( histspec, GetVar<Int_t>(varX)+ix, 
			    GetVar<Int_t>(varY)+iy, GetVar<Int_t>(varZ)+iz,
			    typeX, NULL,NULL,NULL, cut, wgt );
	else Setup3D( histspec, GetVar<Int_t>(varX)+ix, 
		      GetVar<Int_t>(varY)+iy, GetVar<Double_t>(varZ)+iz,
		      typeX, NULL,NULL,NULL, cut, wgt );
      }
      else{                                        // double y
	if(isIntZ) Setup3D( histspec, GetVar<Int_t>(varX)+ix, 
			    GetVar<Double_t>(varY)+iy, GetVar<Int_t>(varZ)+iz,
			    typeX, NULL,NULL,NULL, cut, wgt );
	else Setup3D( histspec, GetVar<Int_t>(varX)+ix, 
		      GetVar<Double_t>(varY)+iy, GetVar<Double_t>(varZ)+iz,
		      typeX, NULL,NULL,NULL, cut, wgt );
      }
    }
    else{                                          // double X
      if( isIntY ){                                // integer Y
	if(isIntZ) Setup3D( histspec, GetVar<Double_t>(varX)+ix, 
			    GetVar<Int_t>(varY)+iy, GetVar<Int_t>(varZ)+iz,
			    typeX, NULL,NULL,NULL, cut, wgt );
	else Setup3D( histspec, GetVar<Double_t>(varX)+ix, 
		      GetVar<Int_t>(varY)+iy, GetVar<Double_t>(varZ)+iz,
		      typeX, NULL,NULL,NULL, cut, wgt );
      }
      else{                                       // double Y
	if(isIntZ) Setup3D( histspec, GetVar<Double_t>(varX)+ix, 
			    GetVar<Double_t>(varY)+iy, GetVar<Int_t>(varZ)+iz,
			    typeX, NULL,NULL,NULL, cut, wgt );
	else Setup3D( histspec, GetVar<Double_t>(varX)+ix, 
		      GetVar<Double_t>(varY)+iy, GetVar<Double_t>(varZ)+iz,
		      typeX, NULL,NULL,NULL, cut, wgt );
      }
    }
  }
  else PrintError(line,"<No dimension specification for histogram>");
  return;
}

//-----------------------------------------------------------------------------
void TA2HistManager::Hist( Char_t* name, Int_t mode, Axis_t xl, Axis_t xh,
		      Axis_t yl, Axis_t yh)
{
  // Draw 1 or 2D histogram with axis-range specification
  // Optional drawing range setup by parameters xl,xh x-axis, yl,yh y-axis
  // Default xl,xh,yl,yh = 0....use full axis range
  // Histogram is not drawn if any error

  if( !fIsDisplay ) goto HistErr;

  // 1D histogram
  if( mode == EHist1D ){
    if( !f1Dhist ) goto HistErr;                    // list if 1D histograms
    TH1F* h1 = (TH1F*)(f1Dhist->FindObject(name));  // is it on the list
    if( !h1 ) goto HistErr;                         // not found
    if( xl ){                                       // optional set x-axis
      TAxis* xaxis =  h1->GetXaxis();
      xaxis->SetRangeUser(xl,xh);
    }
    h1->Draw();
  }
  // 2D histogram
  else if( mode == EHist2D ){
    if( !f2Dhist ) goto HistErr;                    // list of 2D histograms
    TH2F* h2 = (TH2F*)(f2Dhist->FindObject(name));  // is it on the list
    if( !h2 ) goto HistErr;                         // not found
    if( xl ){                                       // optional set x-axis
      TAxis* xaxis =  h2->GetXaxis();
      xaxis->SetRangeUser(xl,xh);
    }
    if( yl ){                                       // optional set y-axis
      TAxis* yaxis =  h2->GetYaxis();
      yaxis->SetRangeUser(yl,yh);
    }
    h2->Draw();
  }
  return;

  // General error message, return without doing anything
 HistErr: printf(" %d-D histogram %s not enabled for this %s\n",
		  mode, name, this->ClassName() );
  return;
}

//-----------------------------------------------------------------------------
void TA2HistManager::FillAllHist()
{
  // Step through list of child analysis entities
  // and call the child histogram filler method for each
  TIter nextchild(fChildren);
  TA2HistManager* child;
  while( ( child = (TA2HistManager*)nextchild() ) )
    child->FillAllHist();
}

//-----------------------------------------------------------------------------
void TA2HistManager::ZeroAll()
{
  // If display is enabled...
  // Step through lists of 1D/2D/3D histograms
  // Reset each histogram found

  // check there is something to display
  if( fIsDisplay ){
    TH1* h;                                      // all hists inherit from TH1
    // 1D histogram list
    if( f1Dhist ){                               // if there is a 1D list
      TIter nexthist( f1Dhist );                 // list iterator
      while( (h = (TH1*)nexthist()) ) h->Reset();// zero all on list
    }
    // 2D histogram list
    if( f2Dhist ){                               // if there is a 2D list
      TIter nexthist( f2Dhist );                 // list iterator
      while( (h = (TH1*)nexthist()) ) h->Reset();// zero all on list
    }
    // 3D histogram list
    if( f3Dhist ){                               // if there is a 2D list
      TIter nexthist( f3Dhist );                 // list iterator
      while( (h = (TH1*)nexthist()) ) h->Reset();// zero all on list
    }
  }
  // Now zero histograms of child classes
  TIter nextchild(fChildren);
  TA2HistManager* child;
  while( ( child = (TA2HistManager*)nextchild() ) )
    child->ZeroAll();
}

//-----------------------------------------------------------------------------
void TA2HistManager::ZeroHist( Char_t* name )
{
  // Reset 1/2/3D histogram.
  // Find histogram by name

  // check there is something to display
  if( !fIsDisplay ){
    printf(" Sorry no histograms enabled for this %s\n", this->ClassName() );
    return;
  }
  TH1* h = NULL;                                    // all hists inherit TH1
  // Try 1D histogram
  if( f1Dhist ){
    h = (TH1*)(f1Dhist->FindObject(name));          // is it on the 1D list
    if( h ){  h->Reset(); return; }
  }
  // 2D histogram
  if( f2Dhist ){
    h = (TH1*)(f2Dhist->FindObject(name));          // is it on the 2D list
    if( h ){  h->Reset(); return; }
  }
  // 3D histogram
  if( f3Dhist ){
    h = (TH1*)(f3Dhist->FindObject(name));          // is it on the 3D list
    if( h ){  h->Reset(); return; }
  }
  // Histogram not found...print message
  printf("Histogram %s not found for class %s\n", name, this->ClassName() );
  return;
}

//-----------------------------------------------------------------------------
void TA2HistManager::SaveHist( )
{
  // Save all defined histograms for this instance of TA2HistManager
  // to a ROOT file. The file name is Hist_<classname>.root, where <classname>
  // is the name of the CLASS (derived from TA2HistManager) for which the
  // histograms are defined

  // check there is something to display
  if( !fIsDisplay ){
    printf(" Sorry no histograms enabled for this %s\n", this->ClassName() );
    return;
  }
  // construct file name and open
  Char_t* fname = BuildName("ARHist_",(Char_t*)this->ClassName(),".root");
  TFile f(fname, "RECREATE");
  if( f1Dhist ) f1Dhist->Write();        // 1D hist
  if( f2Dhist ) f2Dhist->Write();        // 2D hist
  if( f3Dhist ) f3Dhist->Write();        // 3D hist
  // list what's in file
  printf("Histograms stored in file %s\n", fname);
  f.ls(); f.Close(); delete fname;
  return;
}

//-----------------------------------------------------------------------------
void TA2HistManager::ListHist()
{
  // List the names of all histograms initialised for the calling class
  // check there is something to display
  if( !fIsDisplay ){
    printf(" Sorry no histograms enabled for this %s\n", this->ClassName() );
    return;
  }
  TH1* h;
  printf("Histograms initialised for class %s\n", this->ClassName());
  if( f1Dhist ){                               // if there is a 1D list
    TIter nexthist( f1Dhist );                 // list iterator
    while( (h = (TH1*)nexthist()) ) printf(" 1D histogram: %s\n",h->GetName());
  }
  if( f2Dhist ){                               // if there is a 2D list
    TIter nexthist( f2Dhist );                 // list iterator
    while( (h = (TH1*)nexthist()) ) printf(" 2D histogram: %s\n",h->GetName());
  }
  if( f3Dhist ){                               // if there is a 3D list
    TIter nexthist( f3Dhist );                 // list iterator
    while( (h = (TH1*)nexthist()) ) printf(" 3D histogram: %s\n",h->GetName());
  }
}

//-----------------------------------------------------------------------------
void TA2HistManager::ReadChanLim(Int_t nChan, Double_t** xBuff, Char_t* xname)
{
  // Read channel limits for non-constant channel width histogram
  // from file into buffer pointed to by xBuff. The file must have at least
  // nChan+1 lines of info which are not comment lines (# == comment)
  //
  Char_t* line;
  Double_t* xb = new Double_t[nChan + 1];
  *xBuff = xb;
  Char_t* xdname = BuildName("data/",xname);
  ARFile_t* xfile = new ARFile_t( xdname, "r", this );
  Int_t i = 0;
  Int_t j;
  while( (line = xfile->ReadLine()) ){
    if( sscanf( line, "%d%lf", &j, xb+i ) != 2 )
      PrintError(line,"<Channel-limits file format error>",EErrFatal);
    if( i != j )
      PrintError(line,"<Channel-limits file out of synch>",EErrFatal);
    i++;
    if( i > nChan )break; 
  }
  delete xdname;
  delete xfile;
  if( i < nChan )
    PrintError(xname,"<Insufficient channel boundaries supplied>", EErrFatal);
  return;
}

ClassImp(TA2HistManager)
