#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <fstream>
// #include <ostream>
// #include <istream>
#include <string>
#include <vector>

#include "CBoard.h"
#include "CAppArg.h"
#include "stdStringSplit.h"
#include "CBom.h"
#include "CFid.h"

class CBoardEx : public CBoard
{
public:
    CBoardEx()
        // : mbModeJson(false)
        {};
    virtual ~CBoardEx() {};

    // inline void ModeJson(bool v = true) { mbModeJson = v; };
    std::string ImportExport(bool bImport, std::vector<std::string> const &args);

    std::string SectionDump(std::string strSectionName);

protected:
	void StreamRead(std::ifstream &ifs
		, std::ostringstream &ossError
		, std::function<bool(std::string const &, std::ostringstream &)> fn
		);
	std::string ImportSequence(std::ifstream &ifs);
	std::string ImportPlace(std::ifstream &ifs);
	std::string ImportPickup(std::ifstream &ifs);
	std::string ImportChuck(std::ifstream &ifs);
	std::string ImportExtent(std::ifstream &ifs);
	std::string ImportPickupExtent(std::ifstream &ifs);

    std::string Import(std::vector<std::string> const &args);
    std::string Export(std::vector<std::string> const &args);

    // bool mbModeJson;
};

void CBoardEx::StreamRead(std::ifstream &ifs
	, std::ostringstream &ossError
	, std::function<bool(std::string const &, std::ostringstream &)> fn
	)
{
	uint lineNum = 0;
	bool bAgain = true;

	while(bAgain && !ifs.eof() && ossError.str().size() == 0)
	{
		std::string strLine;
		std::getline(ifs, strLine);

		strLine = stringTrim(strLine, "#");
		lineNum ++;
		if(strLine.size())
		{
			std::transform(strLine.begin(), strLine.end(), strLine.begin(), [](const unsigned char i){ return tolower(i); });
			bAgain = fn(strLine, ossError);
			if(ossError.str().size() > 0)
				ossError << " on line " << lineNum;
		}
	}
}

std::string CBoardEx::ImportSequence(std::ifstream &ifs)
{
	std::vector<CBrdSeq> ar;
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError)
	{
		CBrdSeq seq;
		ossError << seq.Parse(strLine);
		if(ossError.str().size() == 0)
			ar.push_back(seq);

		return true;
	});

	if(ossError.str().size() == 0 && ar.size())
		mSeq = ar;
	else ossError << "No sequences imported.";

	return ossError.str();
}

std::string CBoardEx::ImportPlace(std::ifstream &ifs)
{
	std::vector<CBrdPPC> ar;
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError)
	{
		CBrdPPC ppc;
		ossError << ppc.Parse(strLine);
		if(ossError.str().size() == 0)
			ar.push_back(ppc);

		return true;
	});

	if(ossError.str().size() == 0 && ar.size())
		mPlace = ar;
	else ossError << "No placements imported.";

	return ossError.str();
}

std::string CBoardEx::ImportPickup(std::ifstream &ifs)
{
	std::vector<CBrdPPC> ar;
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError)
	{
		CBrdPPC ppc;
		ossError << ppc.Parse(strLine);
		if(ossError.str().size() == 0)
			ar.push_back(ppc);

		return true;
	});

	if(ossError.str().size() == 0 && ar.size())
		mPickup = ar;
	else ossError << "No pickups imported.";

	return ossError.str();
}

std::string CBoardEx::ImportChuck(std::ifstream &ifs)
{
	std::vector<CBrdPPC> ar;
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError)
	{
		CBrdPPC ppc;
		ossError << ppc.Parse(strLine);
		if(ossError.str().size() == 0)
			ar.push_back(ppc);

		return true;
	});

	if(ossError.str().size() == 0 && ar.size())
		mChuck = ar;
	else ossError << "No chucks imported.";

	return ossError.str();
}

std::string CBoardEx::ImportExtent(std::ifstream &ifs)
{
	std::vector<CBrdExtent> ar;
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError)
	{
		CBrdExtent extent;

		ossError << extent.Parse(strLine);
		if(ossError.str().size() == 0)
		{
			uint num = extent.Num();

			if(num == 0)
				num = ar.size()+1;

			if(num && ar.size() < num)
			{
				uint oldNum = ar.size();
				CBrdExtent ext;

				ext.Lae(CBrdExtentLAE());

				ar.resize(num);
				for(uint i=oldNum; i<num; i++)
				{
					ext.Num(i+1);
					ar[i] = CBrdExtent(ext);
				}
			}
			if(num)
				ar[num-1] = extent;
		}

		return true;
	});

	if(ossError.str().size() == 0 && ar.size())
		mExtent = ar;
	else ossError << "No extents imported.";

	return ossError.str();
}

std::string CBoardEx::ImportPickupExtent(std::ifstream &ifs)
{
	std::ostringstream ossError;

	StreamRead(ifs, ossError, [&](std::string const &strLine, std::ostringstream &ossError) mutable
	{
		std::vector<std::string> fields;
		std::string str(strLine);

		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
		split(str, ",", [&](std::string const &item){ fields.push_back(item); });

		std::map<std::string, std::string> fieldMap;
		for(uint i=0,q=fields.size(); i<q; i++)
		{
			std::vector<std::string> ar;
			split(fields[i], ":", [&](std::string const &item) { ar.push_back(item); });
			fieldMap[ar[0]] = ar[1];
		}

		if(fieldMap.find("pickup") != fieldMap.end() && fieldMap.find("extent") != fieldMap.end())
		{
			uint numPickup = std::atoi(fieldMap["pickup"].c_str());
			uint numExtent = std::atoi(fieldMap["extent"].c_str());

			if(numPickup && numExtent && numPickup < mPickup.size())
				mPickup[numPickup-1].Extent(numExtent);
			else ossError << "Error - PickupExtent - invalid pickup or extent";
		}
		else ossError << "Error - PickupExtent - missing fields";

		return ossError.str().size() == 0;
	});

	return ossError.str();
}

std::string CBoardEx::Import(std::vector<std::string> const &args)
{
	std::ostringstream ossError;
	if(args.size() == 2)
	{
		std::string sectionName = args[0];
		std::string fname = args[1];
		std::ifstream ifstr(fname.c_str(), std::ifstream::in);

		if(ifstr.is_open())
		{
			if(sectionName == "sequence")
				ossError << ImportSequence(ifstr);
			else if(sectionName == "place")
				ossError << ImportPlace(ifstr);
			else if(sectionName == "pickup")
				ossError << ImportPickup(ifstr);
			else if(sectionName == "chuck")
				ossError << ImportChuck(ifstr);
			else if(sectionName == "extent")
				ossError << ImportExtent(ifstr);
			else if(sectionName == "pickupextents")
				ossError << ImportPickupExtent(ifstr);
			else
				ossError << "Error - Import - Invalid section name. Try;"
					// " sequence, pickup, place, chuck, repeat pickup, repeat place, or extent"
					" sequence, pickup, or place"
					;

			if(ossError.str().size() == 0)
				Update();
			ifstr.close();
		}
		else ossError << "Error - Import - Unable to open file '" << fname << "'";
	}
	else ossError << "Error - Import - invalid number of arguments";

	return ossError.str();
}

std::string CBoardEx::Export(std::vector<std::string> const &args)
{
	std::ostringstream ossError;

	if(args.size() == 2)
	{
		std::ostringstream oss;
		std::string sectionName = args[0];
		std::string fname = args[1];

		std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), [](const unsigned char i){ return tolower(i); });

		if(sectionName == "sequence")
			for(uint i=0,q=mSeq.size(); i<q; i++) { oss << mSeq[i].Dump() << std::endl; }
		else if(sectionName == "pickup")
			for(uint i=0,q=mPickup.size(); i<q; i++) { oss << mPickup[i].Dump() << std::endl; }
		else if(sectionName == "place")
			for(uint i=0,q=mPlace.size(); i<q; i++) { oss << mPlace[i].Dump() << std::endl; }
		else if(sectionName == "chuck")
			for(uint i=0,q=mChuck.size(); i<q; i++) { oss << mChuck[i].Dump() << std::endl; }
		else if(sectionName == "repeat pickup")
			for(uint i=0,q=mRepeatPickup.size(); i<q; i++) { oss << mRepeatPickup[i].Dump() << std::endl; }
		else if(sectionName == "repeat place")
			for(uint i=0,q=mRepeatPlace.size(); i<q; i++) { oss << mRepeatPlace[i].Dump() << std::endl; }
		else if(sectionName == "extent")
			for(uint i=0,q=mExtent.size(); i<q; i++) { oss << mExtent[i].Dump() << std::endl; }
		else
			ossError << "Error - Export - Invalid section name. Try;"
				" sequence, pickup, place, chuck, repeat pickup, repeat place, or extent"
				;

		if(oss.str().size())
		{
			std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

			if(ofs.is_open())
			{
				ofs << oss.str();
				ofs.flush();
				ofs.clear();
			}
		}
	}

	return ossError.str();
}

std::string CBoardEx::ImportExport(bool bImport, std::vector<std::string> const &args)
{
	std::ostringstream ossError;

    std::for_each(args.begin(), args.end(), [&](std::string const &arg)
    {
        std::vector<std::string> strOpArgs;
        split(arg, "=", [&](std::string const &str) { strOpArgs.push_back(str); });

        if(strOpArgs.size() == 2)
        {
            if(bImport)
                ossError << Import(strOpArgs);
            else
                ossError << Export(strOpArgs);
        }
		else ossError << "Error - " << (bImport ? "Import" : "Export")
			<< " - Invalid number of arguments. Try [section name]=[file name]";
    });

	return ossError.str();
}

std::string CBoardEx::SectionDump(std::string sectionName)
{
    std::ostringstream oss;
	std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), [](const unsigned char i){ return tolower(i); });

    if(sectionName == "all" || sectionName == "header")
    	oss << "header - " << mInfo.Dump() << std::endl;
    if(sectionName == "all" || sectionName == "sequence")
        for(uint i=0,q=mSeq.size(); i<q; i++) { oss << "sequence " << i+1 << " - " << mSeq[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "pickup")
        for(uint i=0,q=mPickup.size(); i<q; i++) { oss << "pickup " << i+1 << " - " << mPickup[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "place")
        for(uint i=0,q=mPlace.size(); i<q; i++) { oss << "place " << i+1 << " - " << mPlace[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "chuck")
        for(uint i=0,q=mChuck.size(); i<q; i++) { oss << "chuck " << i+1 << " - " << mChuck[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "repeat pickup")
        for(uint i=0,q=mRepeatPickup.size(); i<q; i++) { oss << "repeat pickup " << i+1 << " - " << mRepeatPickup[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "repeat place")
        for(uint i=0,q=mRepeatPlace.size(); i<q; i++) { oss << "repeat place " << i+1 << " - " << mRepeatPlace[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "extent")
        for(uint i=0,q=mExtent.size(); i<q; i++) { oss << "extent " << i+1 << " - " << mExtent[i].Dump() << std::endl; }

    return oss.str();
}

int main(int argc, char **argv)
{
    CAppArg cliargs(argc, argv);
    CBoardEx board;
	bool bRotate90 = true;

    cliargs.forEach([&](std::string const &key, std::string const &val)
    {
		std::string action = key;
	    std::vector<std::string> vals;
		std::transform(action.begin(), action.end(), action.begin(), [](const unsigned char i){ return tolower(i); });
		split(val, "     ", [&](std::string const &v) { vals.push_back(v); });

		std::map<std::string, std::string> mapArgs;
		std::for_each(vals.begin(), vals.end(), [&](std::string const &v)
		{
			if(v.find("=") != std::string::npos)
			{
				std::vector<std::string> ar;
				split(v, "=", [&](std::string const &item){ ar.push_back(item); });
				if(ar.size())
				{
					std::string k = ar[0];
					std::transform(k.begin(), k.end(), k.begin(), [](const unsigned char i){ return tolower(i); });
					mapArgs[k] = ar[1];
				}
			}
		});

        // std::cout << "key: " << key << ", val: " << val << std::endl;
		if(action == "read")
        {
            std::ifstream ifstr(val.c_str(), std::ifstream::in | std::ifstream::binary);

            if(ifstr.is_open())
            {
                ifstr >> board;
                ifstr.close();
            }
        }
        else if(action == "write")
        {
            std::ofstream ofs(val.c_str(), std::ofstream::out | std::ofstream::binary);

            if(ofs.is_open())
            {
                ofs << board;
                ofs.flush();
                ofs.clear();
            }
        }
        else if(action == "export" || action == "import")
        {

            std::string strError = board.ImportExport(action == "import", vals);
			if(strError.size())
				std::cerr << strError << std::endl;
        }
        else if(action == "dump")
        {
            std::cout << board.SectionDump(val);
        }
		else if(action == "bom")
		{
			CBom bom;

			std::ostringstream ossError;

			if(mapArgs.find("rotate0") != mapArgs.end())
			{
				bom.Rotate90(false);
				bRotate90 = false;
			}
			if(mapArgs.find("rotate90") != mapArgs.end())
			{
				bom.Rotate90(true);
				bRotate90 = true;
			}

			if(mapArgs.find("chuck") != mapArgs.end())
				ossError << bom.ImportChuck(mapArgs["chuck"]);
			if(mapArgs.find("pickup") != mapArgs.end())
				ossError << bom.ImportPickup(mapArgs["pickup"]);
			if(ossError.str().size() == 0 && mapArgs.find("place") != mapArgs.end())
			{
				uint width = (mapArgs.find("width") != mapArgs.end()
					? std::atoi(mapArgs["width"].c_str())
					: 0);
				CBrdLoc machineHome;
				if(mapArgs.find("home") != mapArgs.end())
				{
					// expecting... "x:1,y:2,z:3", split them at ',' and then parse them
					std::vector<std::string> arHome;
					split(mapArgs["home"], ",", [&](std::string const &item){ arHome.push_back(item); });
					std::for_each(arHome.begin(), arHome.end(), [&](std::string const &item)
						{ machineHome.Parse(item); });
				}
				if(bom.Rotate90() && width && machineHome.y())
					machineHome.y(machineHome.y() + width);
				ossError << bom.ImportPlace(mapArgs["place"], machineHome);
			}
			if(ossError.str().size() == 0 && mapArgs.find("sequence") != mapArgs.end())
				ossError << bom.ExportSequence(
					mapArgs["sequence"], mapArgs["pickuprefout"]
					, mapArgs["sequencepre"]
					, mapArgs["sequencepost"]
				);
			if(ossError.str().size() == 0 && mapArgs.find("placeout") != mapArgs.end())
				ossError << bom.ExportPlace(mapArgs["placeout"], mapArgs["placerefout"]);

			if(ossError.str().size() == 0 && mapArgs.find("extentout") != mapArgs.end())
				ossError << bom.ExportPickupExtent(mapArgs["extentout"]);

			if(ossError.str().size())
				std::cerr << ossError.str() << std::endl;
		}
		else if(action == "fid")
		{
			if(mapArgs.find("rotate0") != mapArgs.end())
				bRotate90 = false;
			if(mapArgs.find("rotate90") != mapArgs.end())
				bRotate90 = true;

			std::ostringstream ossError;
			uint width = (mapArgs.find("width") != mapArgs.end()
				? std::atoi(mapArgs["width"].c_str())
				: 0);
			CBrdLoc machineHome;

			if(mapArgs.find("home") != mapArgs.end())
			{
				// expecting... "x:1,y:2,z:3", split them at ',' and then parse them
				std::vector<std::string> arHome;
				split(mapArgs["home"], ",", [&](std::string const &item){ arHome.push_back(item); });
				std::for_each(arHome.begin(), arHome.end(), [&](std::string const &item)
					{ machineHome.Parse(item); });
			}
			if(bRotate90 && width && machineHome.y())
				machineHome.y(machineHome.y() + width);

			CFiducial fid(machineHome);
			fid.Rotate90(bRotate90);

			if(mapArgs.find("in") != mapArgs.end())
				ossError << fid.Import(mapArgs["in"]);
			if(ossError.str().size() == 0 && mapArgs.find("out") != mapArgs.end())
				ossError << fid.Export(mapArgs["out"]);

			if(ossError.str().c_str())
				std::cerr << ossError.str() << std::endl;
		}
        // else if(action == "json")
        //     board.ModeJson();
    });

    return 0;
}