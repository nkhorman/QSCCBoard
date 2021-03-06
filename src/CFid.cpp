
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
			uint y = std::atoi( Rotate90() ? cols[1].c_str() : cols[2].c_str() ); // board x -> machine y
			uint x = std::atoi( Rotate90() ? cols[2].c_str() : cols[1].c_str() ); // board y -> machine x
			// uint z = std::atoi(cols[3].c_str()); // board surface -> machine z

			CBrdLoc fiducial(x, y, 0, 0, Rotate90());
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

std::string CFiducial::ExportRef(uint num, CBrdLoc loc, std::map<std::string, uint> settings)
{
	std::ostringstream oss;
	bool bIsUsed = (loc.x() != 0 || loc.y() != 0);

	if(bIsUsed)
	{
		if(settings.size() == 0)
		{
			// system generated defaults, hardcoded here
			settings["vid.fore"] = 75;
			settings["vid.back"] = 90;
			settings["light1.level"] = 150;
			settings["light2.level"] = 150;
			settings["fid.dia"] = 58;
			settings["fid.search.x"] = 300;
			settings["fid.search.y"] = 300;
		}

		// PF11 Y +0.00000e+000 21.703 06.898 00.000 00.000 00 0.000 0.000 000 000 0.300 0.300 0.058 0.058 001 075 090 000 000 000 0001 96 0002 96

		// Specifying mils in floats... I don't understand this thinking
		oss << " " << (bIsUsed ? "Y" : "N") << " +0.00000e+000"
			<< " " << std::setfill('0') << std::setw(6) << std::setprecision(5) << ((float)loc.x() / 1000) //"21.703 06.898"
			<< " " << std::setfill('0') << std::setw(6) << std::setprecision(5) << ((float)loc.y() / 1000) //"21.703 06.898"
			<< " 00.000 00.000 00 0.000 0.000 000 000"
			// the std stream formatter is some fucked up shit! cause ... who wants trailing zeros ?
			// I just want precision to be 3... FUCK!
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << ((float)settings["fid.search.x"]/ 1000) // search window size
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << ((float)settings["fid.search.y"]/ 1000) // search window size
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << ((float)settings["fid.dia"]/ 1000) //" 0.058 0.058 "
			<< " " << std::setfill('0') << std::left << std::setw(5) << std::setprecision(5) << ((float)settings["fid.dia"]/ 1000)  //" 0.058 0.058 "
			<< " " << std::setfill('0') << std::right << std::setw(3) << num
			<< " " << std::setfill('0') << std::right << std::setw(3) << settings["vid.fore"] % 256 // videometer foreground
			<< " " << std::setfill('0') << std::right << std::setw(3) << settings["vid.back"] % 256 // videometer background
			<< " 000 000 000"
			<< " 0001 " << std::setfill('0') << std::setbase(16) << std::setw(2) << settings["light1.level"] % 256 // light 1, level in hex ?
			<< " 0002 " << std::setfill('0') << std::setbase(16) << std::setw(2) << settings["light2.level"] % 255 // light 2, level in hex ?
			;
	}
	else
		oss << " N +0.00000e+000 00.000 00.000 00.000 00.000 00 0.000 0.000 000 000 0.000 0.000 0.000 0.000 000 075 090 000 000 000 0000 00 0000 00";

	return oss.str();
}

std::string CFiducial::Export(std::string fname, bool bFidIsImage, std::map<std::string, uint> const &settings)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		uint headNum,j,count=0;
		std::ostringstream oss;
		std::vector<std::string> arFidEmpty;
		std::vector<std::string> arFid;

		// build some emptys
		for(j=0; j<6; j++)
			arFidEmpty.push_back(ExportRef(j+1, CBrdLoc(), settings));

		for(j=0; j<6; j++)
			ofs << "REJ" << (j+1) << arFidEmpty[j] << "\r\n";
		for(headNum=0; headNum<6; headNum++)
		{
			std::ostringstream pre;
			pre.str("");
			pre << "PF";
			pre << (headNum+1);
			if(headNum == 0 && !bFidIsImage)
			{
				j = 0;
				std::for_each(mFiducials.begin(), mFiducials.end(), [&](CBrdLoc const &item)
				{
					oss.str("");
					oss << pre.str() << (++j) << ExportRef(++count, item, settings);
					arFid.push_back(oss.str());
				});
			}
			else
			{
				for(j=0; j<3; j++,count++)
				{
					oss.str("");
					oss << pre.str() << (j+1) << arFidEmpty[j];
					arFid.push_back(oss.str());
				}
			}
		}
		for(headNum=0; headNum<6; headNum++)
		{
			std::ostringstream pre;
			pre.str("");
			pre << "IF";
			pre << (headNum+1);

			if(headNum == 0 && bFidIsImage)
			{
				j = 0;
				std::for_each(mFiducials.begin(), mFiducials.end(), [&](CBrdLoc const &item)
				{
					oss.str("");
					oss << pre.str() << (++j) << ExportRef(++count, item, settings);
					arFid.push_back(oss.str());
				});
			}
			else
			{
				for(j=0; j<3; j++,count++)
				{
					oss.str("");
					oss << pre.str() << (j+1) << arFidEmpty[j];
					arFid.push_back(oss.str());
				}
			}
		}
		std::for_each(arFid.begin(), arFid.end(), [&](std::string const &str)
			{ ofs << str << "\r\n"; });
		ofs.flush();
		ofs.clear();
	}
	else ossError << "Error - Fiducial - Unable to open file '" << fname << "'";

	return ossError.str();
}