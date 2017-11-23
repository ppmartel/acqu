#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>      // std::setprecision
#include <vector>
#include <stdlib.h>
#include <iomanip>      // std::setprecision

int main()
{
//g++ mergecorr.C -o merge --std=c++11


// This part is to get a list of all the run numbers in the gain correction files, i.e. the first element in each line
	std::ifstream file1("/home/adlarson/data2014.07/macros/gaincorr/CB_gain_vs_det_vs_runnrit3.dat");		// path to first gain correction file 
	std::ifstream file2("/home/adlarson/data2014.07/macros/gaincorr/gain_vs_det_vs_runnr_merged2.dat");		// path to second gain correction file 
	std::string   line;
	int irun;
	const unsigned int start = 4952;
	const unsigned int end = 5906;
	double gain;
	std::vector<int> f1;
	std::vector<int> f2;

	while(std::getline(file1, line))
	{
		std::stringstream ss;
		ss << line;
		irun = atoi(ss.str().c_str());  // atoi only reads everything from a string until a non-integer type occurs
		f1.push_back(irun);

	}
	while(std::getline(file2, line))
	{
		std::stringstream ss;
		ss << line;
		irun = atoi(ss.str().c_str());
		f2.push_back(irun);
	}



	auto it1 = f1.begin();
	auto it2 = f2.begin();
	std::ifstream in1("/home/adlarson/data2014.07/macros/gaincorr/CB_gain_vs_det_vs_runnrit3.dat");		// path to first gain correction fil
	std::ifstream in2("/home/adlarson/data2014.07/macros/gaincorr/gain_vs_det_vs_runnr_merged2.dat");	// path to second gain correction file 
	std::string line1, line2;
	double gain1, gain2;
	std::string buffer1, buffer2;

	std::ofstream out;
	out.open("/home/adlarson/data2014.07/macros/gaincorr/gain_vs_det_vs_runnr_merged3.dat");

	
	if (*it1 < start || *it2 < start) {
		fprintf(stderr, "ERROR: start value %d is greater than run number %d|%d in file 1|2\n", start, *it1, *it2);
		return 1;
	}

	for (unsigned int i = start; i <= end; i++) {
		if (*it1 == i && *it2 == i) 
		{
			if (std::getline(in1, line1) && std::getline(in2, line2)) {
				out << i << '\t';
				std::stringstream ss1, ss2;

				
				// now we have the whole lines corresponding to runnumber i in lineX
				ss1 << line1;
				ss2 << line2;
				std::getline(ss1, buffer1, '\t');
				std::getline(ss2, buffer2, '\t');
				while (std::getline(ss1, buffer1, '\t') && std::getline(ss2, buffer2, '\t')) {
					out << std::fixed << std::setprecision(5) << std::stod(buffer1)*std::stod(buffer2) << '\t';
				}
				out << '\n';
				++it1;
				++it2;
			} else
				fprintf(stderr, "WARNING: no line to runnumber %d existing!\nThis should not happen!", i);
		} 
		else if (*it1 == i) 
		{
				out << i << '\t';
				std::stringstream ss1;
				// now we have the whole lines corresponding to runnumber i in lineX
				ss1 << line1;
				std::getline(ss1, buffer1, '\t');
				//int j = 1;
				while (std::getline(ss1, buffer1, '\t')) {
					out << std::fixed << std::setprecision(5) << std::stod(buffer1) << '\t';
				}
				out << '\n';
				++it1;
		}
		else if (*it2 == i) 
		{
				out << i << '\t';
				std::stringstream ss2;
				// now we have the whole lines corresponding to runnumber i in lineX
				ss2 << line2;
				std::getline(ss2, buffer2, '\t');
				while (std::getline(ss2, buffer2, '\t')) {
					out << std::fixed << std::setprecision(5) << std::stod(buffer2) << '\t';
				}
				out << '\n';
				++it2;
		} 
		else 
		{
			out << i << '\t';
			for(int k = 0; k < 720; k++)
			{
				out << std::fixed << std::setprecision(5) << "1.00000" << '\t';	
			}
			out << '\n';
		}
	}

	out.close();

	return 0;

}