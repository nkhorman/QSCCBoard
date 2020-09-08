
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
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

std::string CBomPickup::Export(uint &lastChuckNum, std::map<std::string, std::pair<uint, uint>> &placeMap, bool bImageRepeat) const
{
	std::ostringstream oss;

	if(lastChuckNum != mChuck)
	{
		if(bImageRepeat && lastChuckNum !=  0)
			oss << "repeat 0" << std::endl;
		oss << "chuck " << mChuck << std::endl;
		if(bImageRepeat)
			oss << "repeat 1" << std::endl;
		lastChuckNum = mChuck;
	}
	std::for_each(mParts.begin(), mParts.end(), [&](std::string const &str)
	{
		std::string lowerPartName = str;
		std::transform(lowerPartName.begin(), lowerPartName.end(), lowerPartName.begin(),
			[](const unsigned char i){ return tolower(i); });
		uint placeNum = placeMap[lowerPartName].second;

		oss << "pickup " << mNum << std::endl
			<< "place " << placeNum << std::endl
			;
	});

	return oss.str();
}

std::string CBomPickup::ExportExtent() const
{
	std::ostringstream oss;
	uint tol = 20;
	float tolFl = 1.0 + ((float)(tol-5) / 100);
	uint height = 40; // height percentage
	float heightFl = (float)(height) / 100;

	oss << "extent " << mNum << " - LAE -"
		<< " Length: " << (uint)(mSize.x() * tolFl) << ", Tol: " << tol
		<< ", Width: " << (uint)(mSize.y() * tolFl) << ", Tol: " << tol
		<< ", Height: " << (uint)(mSize.z() * heightFl)
		<< ", MeasureType: 1"
		<< std::endl
		;

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
			uint y = std::atoi( Rotate90() ? cols[1].c_str() : cols[2].c_str() ); // board x -> machine y
			uint x = std::atoi( Rotate90() ? cols[2].c_str() : cols[1].c_str() ); // board y -> machine x
			uint rot = std::atoi(cols[3].c_str());
			if(!Rotate90())
			{
				if(rot == 0)
					rot = 360;
				rot -= 90;
			}
			float t = 66.666666666666667 * rot; // board rotation -> machine theta
			uint z = (cols.size() == 5 ? std::atoi(cols[4].c_str()) : 0); // board surface -> machine z

			mLoc = CBrdLoc(x, y, z, (uint)t, Rotate90());
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

static bool cbp_compare(const CBomPickup &lhs, const CBomPickup &rhs)
{
	// different chuck number
	if(lhs.Chuck() < rhs.Chuck()) return true;

	CBrdLoc lhss = lhs.Size();
	CBrdLoc rhss = rhs.Size();

	// smaller height
	uint lhsz = lhss.z();
	uint rhsz = rhss.z();
	if(lhsz && rhsz && lhsz < rhsz) return true;

	// smaller surface area
	uint lhsa = lhss.x() * lhss.y();
	uint rhsa = rhss.x() * rhss.y();
	if(lhsa && rhsa && lhsa < rhsa) return true;

	return false; // lhs is < rhs
}

std::string CBom::ExportSequence(
	std::string fname, std::string fnameRef
	, std::string fnamePre, std::string fnamePost
	, bool bImageRepeat
	)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);
	std::ofstream ofsRef(fnameRef.c_str(), std::ofstream::out | std::ofstream::trunc);
	std::ifstream ifsPre(fnamePre.c_str(), std::ofstream::in);
	std::ifstream ifsPost(fnamePost.c_str(), std::ofstream::in);

	if(ofs.is_open())
	{
		uint lastChuckNum = 0;
		std::list<CBomPickup> cbpl;

		std::for_each(mPickup.begin(), mPickup.end(), [&](CBomPickup const &item) { cbpl.push_back(item); });
		cbpl.sort(cbp_compare);

		if(ifsPre.is_open())
		{
			ofs << ifsPre.rdbuf();
			ofs.flush();
			ifsPre.close();
		}
		std::map<std::string, std::pair<uint, uint>> partPickupPlaceNumCopy = mPartPickupPlaceNum;

		std::for_each(cbpl.begin(), cbpl.end(), [&](CBomPickup const &item)
		{
			ofs << item.Export(lastChuckNum, partPickupPlaceNumCopy, bImageRepeat);
			if(ofsRef.is_open())
			{
				std::string pickupDescription = item.Description();
				addWhiteSpcs(pickupDescription, 25);
				ofsRef << std::setfill('0') << std::setw(3) << item.Num() << " "
					<< pickupDescription << "\r\n"; // dos file, needs CR
			}
		});

		if(ifsPost.is_open())
		{
			ofs << ifsPost.rdbuf();
			ofs.flush();
			ifsPost.close();
		}
		if(bImageRepeat)
			ofs << "repeat 0" << std::endl;
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

std::string CBom::ExportPickupExtent(std::string fname)
{
	std::ostringstream ossError;
	std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);
	
	if(ofs.is_open())
	{
		std::for_each(mPickup.begin(), mPickup.end(), [&](CBomPickup const &item)
		{
			ofs << item.ExportExtent();
		});

		ofs.flush();
		ofs.clear();
	}
	else ossError << "Error - PickupExtent - Unable to open file '" << fname << "'";

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
				place.Rotate90(mbRotate90);
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

			// Compute the placement z
			// based on;
			//	"board height", which is the floor
			//	part height
			// Note: The greater (further from 0) the numerical number, the closer to the work surface you are
			// This should be "floor - part height"
			uint zBoardHeight = place.Loc().z(); // this is presently already set to "Home:z"
			uint zPartHeight = pickup.Size().z();
			uint z = zBoardHeight - zPartHeight;

			place.Loc().z(z);
			ofs << "# " << name
				<< " pickup: " << pickup.Dump()
				<< " bh: " << zBoardHeight
				<< " ph: " << zPartHeight
				//<< " rotate: " << (place.Rotate90() ? "90" : "0")
				<< std::endl;
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
