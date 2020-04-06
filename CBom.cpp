
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include "CBom.h"
#include "stdStringSplit.h"

// **
std::string CBomPickup::Parse(std::string const &str, std::function<void(std::string const &)> fn)
{
	std::ostringstream ossError;
	std::vector<std::string> cols;

	split(str, "	", [&](std::string const &item){ cols.push_back(item); });

	if(cols.size() >= 7)
	{
		if(cols[0].size() && cols[1].size() && cols[6].size())
		{
			std::vector<std::string> arParts;
			std::string strParts(cols[6]);

			strParts.erase(std::remove(strParts.begin(), strParts.end(), ' '), strParts.end());
			split(strParts,  ",", [&](std::string const &item) { arParts.push_back(item); if(fn) fn(item); });
			
			mNum = std::atoi(cols[0].c_str());
			mChuck = std::atoi(cols[1].c_str());
			uint x = std::atoi(cols[2].c_str());
			uint y = std::atoi(cols[3].c_str());
			uint z = std::atoi(cols[4].c_str());
			uint t = std::atoi(cols[5].c_str());
			mSize = CBrdLoc(x,y,z,t);
			mDescription = (cols.size() == 8 ? cols[7] : "");
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

std::string CBomPickup::Dump()
{
	std::ostringstream oss;

	oss
		<< "Num: " << mNum
		<< ", Chuck: " << mChuck
		<< " Size: " << mSize.Dump()
		<< " Description: " << mDescription;

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
// # chuck	name	id/od	height	x	y	z	t
// 1	xf	30/18	565	5020	5729	1283	0
// 2	xg	40/28	565	5019	5038	1259	0
// 3	xh	55/39	565	5022	4222	1248	0
// 4	bas	245/75	560	6555	5724	1247	0
// 5	aa	161/95	564	6559	5035	1259	0
// 6	vn	428	565	6557	4227	1258	0

std::string CBomChuck::Parse(std::string const &str)
{
	std::ostringstream ossError;
	std::vector<std::string> cols;

	split(str, "	", [&](std::string const &item){ cols.push_back(item); });

	if(cols.size() == 8)
	{
		if(
			cols[0].size() && cols[1].size() && cols[2].size() && cols[3].size()
			&& cols[4].size() && cols[5].size() && cols[6].size() && cols[7].size()
		)
		{
			mNum = std::atoi(cols[0].c_str());
			mName = cols[1];
			mIdOd = cols[2];
			mHeight = std::atoi(cols[3].c_str());

			mLoc = CBrdLoc(
				std::atoi(cols[4].c_str())
				, std::atoi(cols[5].c_str())
				, std::atoi(cols[6].c_str())
				, std::atoi(cols[7].c_str())
				);
		}
		else ossError << "Error - Chuck - one or more empty columns";
	}
	else ossError << "Error - Chuck - invalid number of Tab separated columns";

	return ossError.str();
}

std::string CBomChuck::Export() const
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

			strLine = stringTrim(strLine, "#");
			lineNum ++;

			if(strLine.size())
			{
				pickupNum++;
				CBomPickup pickup;
				ossError << pickup.Parse(strLine, [&](std::string const &partName)
				{
					std::string lowerPartName = partName;
					std::transform(lowerPartName.begin(), lowerPartName.end(), lowerPartName.begin(),
						[](const unsigned char i){ return tolower(i); });
					mPartPlaceName.push_back(lowerPartName);
					mPartPickupPlaceNum[lowerPartName] = std::pair<uint,uint>(pickupNum, ++partNum);
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
	}
	else ossError << "Error - Pickup - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ExportPickup(std::string fname, std::string fnameRef)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);
	std::ofstream ofsRef(fnameRef.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		uint lastChuckNum = 0;
		uint lastItemNum = 0;
		uint pickupNum = 1;
		std::for_each(mPickup.begin(), mPickup.end(), [&](CBomPickup const &item)
		{
			ofs << item.Export(lastChuckNum, lastItemNum, pickupNum++);
			if(ofsRef.is_open())
			{
				std::string pickupDescription = item.Description();
				addWhiteSpcs(pickupDescription, 25);
				ofsRef << std::setfill('0') << std::setw(3) << item.Num() << " "
					<< pickupDescription << "\r\n"; // dos file, needs CR
			}
		});
		ofs << "chuck 0" << std::endl << "goto 0" << std::endl;
		ofs.flush();
		ofs.clear();
		if(ofsRef.is_open())
		{
			ofsRef.flush();
			ofsRef.close();
		}
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

			strLine = stringTrim(strLine, "#");
			lineNum ++;

			if(strLine.size())
			{
				// std::transform(strLine.begin(), strLine.end(), strLine.begin(), [](const unsigned char i){ return tolower(i); });
				CBomPlace place(home);
				ossError << place.Parse(strLine);
				if(ossError.str().size() == 0)
				{
					std::string name = place.Name();
					std::transform(name.begin(), name.end(), name.begin(), [](const unsigned char i){ return tolower(i); });
					places[name] = place;
				}
			}
		}

		if(ossError.str().size())
			ossError << " on line " << lineNum;
		else if(places.size())
			mPlace = places;
		else ossError << "No placements imported.";
		ifs.close();
	}
	else ossError << "Error - Place - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ExportPlace(std::string fname, std::string fnameRef)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);
	std::ofstream ofsRef(fnameRef.c_str(), std::ofstream::out | std::ofstream::trunc);

	if(ofs.is_open())
	{
		// std::cout << __func__ << ": " << __LINE__ << std::endl;
		std::for_each(mPartPlaceName.begin(), mPartPlaceName.end(), [&](std::string const &name)
		{
			uint pickupNum = mPartPickupPlaceNum[name].first;
			CBomPickup pickup = mPickup[pickupNum-1];
			CBomPlace place = mPlace[name];
			CBomChuck chuck = (mChuck.size() > pickup.Chuck()-1 ? mChuck[pickup.Chuck()-1] : CBomChuck());

			// Compute the placement z
			// based on;
			//	"board height", which is the floor
			//	part height
			//	chuck height
			// Note: The lower (closer to 0) the numerical number, the closer to the work surface you are
			// This should be "floor + chuck height + part height"
			uint zBoardHeight = place.Loc().z(); // this is presently already set to "Home:z"
			uint zChuckHeight = chuck.Height();
			uint zPartHeight = pickup.Size().z();
			uint z = zBoardHeight + zChuckHeight + zPartHeight;

			place.Loc().z(z);
			ofs << "# " << name << " pickup: " << pickup.Dump() << " bh: " << zBoardHeight << " ch: " << zChuckHeight << " ph: " << zPartHeight << std::endl;
			ofs << place.Export() << std::endl;
			if(ofsRef.is_open())
			{
				uint placeNum = mPartPickupPlaceNum[name].second;
				std::string pickupDescription = pickup.Description();
				std::string placeDescription = mPlace[name].Name();

				addWhiteSpcs(pickupDescription, 25);
				addWhiteSpcs(placeDescription, 10);
				ofsRef << std::setfill('0') << std::setw(3) << placeNum
					<< " " << pickupDescription << placeDescription
					<< "\r\n"; // dos file, needs CR
			}
		});
		ofs.flush();
		ofs.close();
		if(ofsRef.is_open())
		{
			ofsRef.flush();
			ofsRef.close();
		}
	}
	else ossError << "Error - Place - Unable to open file '" << fname << "'";

	return ossError.str();
}

std::string CBom::ImportChuck(std::string fname)
{
	std::ostringstream ossError;
	std::ifstream ifs(fname.c_str(), std::ifstream::in);
	std::vector<CBomChuck> ar;

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
			{
				// std::transform(strLine.begin(), strLine.end(), strLine.begin(), [](const unsigned char i){ return tolower(i); });
				CBomChuck chuck;
				ossError << chuck.Parse(strLine);
				if(ossError.str().size() == 0)
					ar.push_back(chuck);
			}
		}
		ifs.close();

		if(ossError.str().size())
			ossError << " on line " << lineNum;
		else if(ar.size())
			mChuck = ar;
		else ossError << "No chucks imported.";
		ifs.close();
	}
	else ossError << "Error - Chuck - Unable to open file '" << fname << "'";

	return ossError.str();
}
