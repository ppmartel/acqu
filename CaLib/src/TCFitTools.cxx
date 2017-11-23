
#include "TCFitTools.h"


Double_t		TCUtils::PedFinder(const TH1D* hist)
{
	TSpectrum	spec;
	
	int	nPeaks	= spec.Search(hist,2,"nodraw");
	//printf("nPeaks: %d\n",nPeaks);
	
	Double_t	lowPos		= spec.GetPositionX()[0];
	//int		lowIndex	= 0;
	
	for(int i=1; i<nPeaks; i++)
	{
		//printf("peak %d: %lf\n", i, spec.GetPositionX()[i]);
		if(spec.GetPositionX()[i]<lowPos)
		{
			lowPos		= spec.GetPositionX()[i];
			//lowIndex	= i;
		}
	}
	//printf("result:  %lf\n", lowPos);
	return lowPos;
}

