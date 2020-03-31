
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "CBom.h"
#include "stdStringSplit.h"

// **
std::string CBomPickup::Parse(std::string const &str, std::function<void(std::string const &)> fn)
{
	std::ostringstream ossError;
	std::vector<std::string> cols;

	split(str, "	", [&](std::string const &item){ cols.push_back(item); });

	if(cols.size() == 3)
	{
		if(cols[0].size() && cols[1].size() && cols[2].size())
		{
			std::vector<std::string> arParts;
			std::string strParts(cols[2]);

			strParts.erase(std::remove(strParts.begin(), strParts.end(), ' '), strParts.end());
			split(cols[2],  ",", [&](std::string const &item) { arParts.push_back(item); if(fn) fn(item); });
			
			mNum = std::atoi(cols[0].c_str());
			mChuck = std::atoi(cols[1].c_str());
			mParts = arParts;
			if(mNum == 0)
				ossError << "Error - Pickup - invalid pickup number";
			else if(mChuck == 0)
				ossError << "Error - Pickup - invalid chuck number";
			else if(mParts.size() == 0)
				ossError << "Error - Pickup - no parts specified";
		}
		else ossError << "Error - Pickup - one or more empty columns";
	}
	else ossError << "Error - Pickup - invalid number of Tab separated columns";

	return ossError.str();
}

std::string CBomPickup::Export(uint &lastChuckNum, uint &lastItemNum, uint pickupNum) const
{
	std::ostringstream oss;

	if(lastChuckNum != mChuck)
	{
		oss << "chuck " << mChuck << std::endl;
		lastChuckNum = mChuck;
	}
	std::for_each(mParts.begin(), mParts.end(), [&](std::string const &str)
	{
		(void)str;
		oss << "pickup " << pickupNum << std::endl
			<< "place " << ++lastItemNum << std::endl
			;
	});

	return oss.str();
}

// **
std::string CBomPlace::Parse(std::string const &str)
{
	std::ostringstream ossError;
	std::vector<std::string> cols;

	split(str, "	", [&](std::string const &item){ cols.push_back(item); });

	if(cols.size() >= 4 && cols.size() <= 5)
	{
		if(cols[0].size() && cols[1].size() && cols[2].size() && cols[3].size())
		{
			mName = cols[0];

			// Extract board datum relative part place location and convert it to machine coordinates
			// Important note - this auto-rotates the board 90 degrees CW
			//	Board datum is presumed to be lower left corner = 0,0
			//	Machine datum is lower right corner = X=23460, y=6790 for a Quad IIc,115 workbox
			uint y = std::atoi(cols[1].c_str()); // board x -> machine y
			uint x = std::atoi(cols[2].c_str()); // board y -> machine x
			float t = 66.666666666666667 * std::atoi(cols[3].c_str()); // board rotation -> machine theta
			uint z = (cols.size() == 5 ? std::atoi(cols[4].c_str()) : 0); // board surface -> machine z

			mLoc = CBrdLoc(x, y, z, (uint)t);
			mLoc.Offset(mMachineHome); // this is the datum offset operation
		}
		else ossError << "Error - Place - one or more empty columns";
	}
	else ossError << "Error - Place - invalid number of Tab separated columns";

	return ossError.str();
}

std::string CBomPlace::Export() const
{
		// std::cout << __func__ << " " << __LINE__ << " - " << mLoc.Dump() << std::endl;
	return mLoc.Dump();
}

// **
std::string CBom::ImportPickup(std::string fname)
{
	std::ostringstream ossError;
	std::ifstream ifs(fname.c_str(), std::ifstream::in);
	std::vector<CBomPickup> ar;

	if(ifs.is_open())
	{
		uint lineNum = 0;
		uint partNum = 0;
		uint pickupNum = 0;

		while(!ifs.eof() && ossError.str().size() == 0)
		{
			std::string strLine;
			std::getline(ifs, strLine);

			lineNum ++;
			if(strLine.size())
			{
				pickupNum++;
				std::transform(strLine.begin(), strLine.end(), strLine.begin(),
					[](const unsigned char i){ return tolower(i); });
				CBomPickup pickup;
				ossError << pickup.Parse(strLine, [&](std::string const &partName)
				{
					mPartPlaceName.push_back(partName);
					mPartPickupPlaceNum[partName] = std::pair<uint,uint>(pickupNum, ++partNum);
				});
				if(ossError.str().size() == 0)
					ar.push_back(pickup);
			}
		}
		ifs.close();

		if(ossError.str().size())
			ossError << " on line " << lineNum;
		else if(ar.size())
			mPickup = ar;
		else ossError << "No pickups imported.";
		// std::cout << __func__ << " " << __LINE__ << std::endl;
	}
	else ossError << "Error - Pickup - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ExportPickup(std::string fname)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		uint lastChuckNum = 0;
		uint lastItemNum = 0;
		uint pickupNum = 1;
		std::for_each(mPickup.begin(), mPickup.end(), [&](CBomPickup const &item)
		{
			ofs << item.Export(lastChuckNum, lastItemNum, pickupNum++);
		});
		ofs << "chuck 0" << std::endl << "goto 0" << std::endl;
		ofs.flush();
		ofs.clear();
	}
	else ossError << "Error - Pickup - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ImportPlace(std::string fname, CBrdLoc const &home)
{
	std::ostringstream ossError;
	std::ifstream ifs(fname.c_str(), std::ifstream::in);
	std::map<std::string, CBomPlace> places;

	if(ifs.is_open())
	{
		uint lineNum = 0;

		while(!ifs.eof() && ossError.str().size() == 0)
		{
			std::string strLine;
			std::getline(ifs, strLine);

			lineNum ++;
			if(strLine.size())
			{
				std::transform(strLine.begin(), strLine.end(), strLine.begin(), [](const unsigned char i){ return tolower(i); });
				CBomPlace place(home);
				ossError << place.Parse(strLine);
				if(ossError.str().size() == 0)
					places[place.Name()] = place;
			}
		}

		if(ossError.str().size())
			ossError << " on line " << lineNum;
		else if(places.size())
			mPlace = places;
		else ossError << "No placements imported.";
		// std::cout << __func__ << " " << __LINE__ << std::endl;
		ifs.close();
	}
	else ossError << "Error - Place - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ExportPlace(std::string fname)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		// std::for_each(mPlace.begin(), mPlace.end(), [&](CBomPlace const &item)
		// 	{ ofs << item.Export() << std::endl; });

		std::for_each(mPartPlaceName.begin(), mPartPlaceName.end(), [&](std::string const &name)
		{
			// ofs << "# " << name << std::endl;
			ofs << mPlace[name].Export() << std::endl;
		});
		ofs.flush();
		ofs.clear();
	}
	else ossError << "Error - Place - Unable to open file '" << fname << "'";

	return ossError.str();
}
