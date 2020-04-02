
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

#include "CFid.h"
#include "stdStringSplit.h"

std::string CFiducial::Parse(std::string &line)
{
	std::ostringstream ossError;
	std::vector<std::string> cols;

	split(line, "	", [&](std::string const &item){ cols.push_back(item); });

	if(cols.size() >= 3)
	{
		if(cols[0].size() && cols[1].size() && cols[2].size())
		{
			mName = cols[0];

			// Extract board datum relative part place location and convert it to machine coordinates
			// Important note - this auto-rotates the board 90 degrees CW
			//	Board datum is presumed to be lower left corner = 0,0
			//	Machine datum is lower right corner = X=23460, y=6790 for a Quad IIc,115 workbox
			uint y = std::atoi(cols[1].c_str()); // board x -> machine y
			uint x = std::atoi(cols[2].c_str()); // board y -> machine x
			// uint z = std::atoi(cols[3].c_str()); // board surface -> machine z

			CBrdLoc fiducial(x, y, 0, 0);
			fiducial.Offset(mMachineHome); // this is the datum offset operation
			mFiducials.push_back(fiducial);
		}
		else ossError << "Error - Fiducial - one or more empty columns";
	}
	else ossError << "Error - Fiducial - invalid number of Tab separated columns";

	return ossError.str();
}

std::string CFiducial::Import(std::string fname)
{
	std::ostringstream ossError;
	std::ifstream ifs(fname.c_str(), std::ifstream::in);

	if(ifs.is_open())
	{
		uint lineNum = 0;

		while(!ifs.eof() && ossError.str().size() == 0)
		{
			std::string strLine;
			std::getline(ifs, strLine);

			strLine = stringTrim(strLine, "#");
			lineNum ++;

			if(strLine.size())
				ossError << Parse(strLine);
		}
		ifs.close();

		if(ossError.str().size())
			ossError << " on line " << lineNum;
		else if(mFiducials.size() == 0)
			ossError << "No fiducials imported.";
	}
	else ossError << "Error - Fiducial - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CFiducial::ExportRef(std::string prefix, uint num, CBrdLoc loc)
{
	std::ostringstream oss;
	bool bIsUsed = (loc.x() != 0 || loc.y() != 0);

	if(bIsUsed)
	{
		// PF11 Y +0.00000e+000 21.703 06.898 00.000 00.000 00 0.000 0.000 000 000 0.300 0.300 0.058 0.058 001 075 090 000 000 000 0001 96 0002 96
		float fdia = (float)60 / 1000; // fiducial diameter

		// CC is fucking stupid to specify mils in floats !
		oss << prefix << num << " " << (bIsUsed ? "Y" : "N") << " +0.00000e+000"
			<< " " << std::setfill('0') << std::setw(6) << std::setprecision(5) << ((float)loc.x() / 1000) //"21.703 06.898"
			<< " " << std::setfill('0') << std::setw(6) << std::setprecision(5) << ((float)loc.y() / 1000) //"21.703 06.898"
			<< " 00.000 00.000 00 0.000 0.000 000 000 0.300 0.300"
			// the std stream formatter is some fucked up shit! cause ... who wants trailing zerros ?
			// I just want precision to be 3... FUCK!
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << fdia //" 0.058 0.058 "
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << fdia //" 0.058 0.058 "
			<< " " << std::setfill('0') << std::right << std::setw(3) << num
			<< " 075 090 000 000 000 0001 96 0002 96";
	}
	else
		oss << prefix << num << " N +0.00000e+000 00.000 00.000 00.000 00.000 00 0.000 0.000 000 000 0.000 0.000 0.000 0.000 000 075 090 000 000 000 0000 00 0000 00";

	return oss.str();
}

std::string CFiducial::Export(std::string fname)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		uint i,j;

		for(i=0; i<6; i++)
			ofs << ExportRef("REJ", i+1, CBrdLoc()) << "\r\n"; // CRLF cause... DOS
		for(i=0; i<6; i++)
		{
			std::ostringstream pre;
			pre.str("");
			pre << "PF";
			if(i == 0)
			{
				pre << (i+1);
				j = 0;
				std::for_each(mFiducials.begin(), mFiducials.end(), [&](CBrdLoc const &item)
				{
					ofs << ExportRef(pre.str(), ++j, item) << "\r\n"; // CRLF cause... DOS
				});
			}
			else
			{
				pre << (i+1);
				for(j=0; j<3; j++)
					ofs << ExportRef(pre.str(), j+1, CBrdLoc()) << "\r\n"; // CRLF cause... DOS
			}
		}
		for(i=0; i<6; i++)
		{
			std::ostringstream pre;
			pre.str();
			pre << "IF";
			pre << (i+1);
			for(j=0; j<3; j++)
				ofs << ExportRef(pre.str(), j+1, CBrdLoc()) << "\r\n"; // CRLF cause... DOS
		}
		ofs.flush();
		ofs.clear();
	}
	else ossError << "Error - Fiducial - Unable to open file '" << fname << "'";

	return ossError.str();
}