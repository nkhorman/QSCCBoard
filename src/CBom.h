#ifndef _CBOM_H_

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>

#include "CBrdLoc.h"

class CBomPickup
{
public:
	CBomPickup() {};
	virtual ~CBomPickup() {};

	inline uint Num() const { return mNum; };
	inline void Num(uint v) { mNum = v; };

	inline uint Chuck() const { return mChuck; };
	inline void Chuck(uint v) { mChuck = v; };

	inline std::string Description() const { return mDescription; };
	inline void Description(std::string v) { mDescription = v; };

	inline void Part(std::string const &v) { mParts.push_back(v); };
	// inline std::vector<std::string> const &Parts() const { return mParts; };
	inline void Parts(std::function<void(std::string const &)> fn)
	{
		if(fn)
			std::for_each(mParts.begin(), mParts.end(), [&](std::string const &item) { fn(item); });
	}

	std::string Parse(std::string const &str, std::function<void(std::string const &)> fn);
	std::string Export(uint &lastChuckNum, std::map<std::string, std::pair<uint, uint>> &placeMap, bool bImageRepeat) const;
	std::string ExportExtent() const;

	std::string Dump() const;

	inline CBrdLoc const &Size() const { return mSize; };
protected:
	uint mNum; // pickup number
	uint mChuck;
	CBrdLoc mSize;
	std::string mDescription;
	std::vector<std::string> mParts;
};

class CBomPlace
{
public:
	CBomPlace() {};
	CBomPlace(CBrdLoc home)
		: mMachineHome(home)
		, mbRotate90(true)
		{};
	virtual ~CBomPlace() {};

	inline std::string Name() const { return mName; };
	inline void Name(std::string v) { mName = v; };

	std::string Parse(std::string const &str);
	std::string Export() const;

	inline CBrdLoc &Loc() { return mLoc; };

	inline bool Rotate90() const { return mbRotate90; };
	inline void Rotate90(bool v) { mbRotate90 = v; };

protected:
	std::string mName;
	CBrdLoc mLoc;
	CBrdLoc mMachineHome;
	bool mbRotate90;
};

class CBomChuck
{
public:
	CBomChuck()
		: mNum(0)
		, mHeight(0)
		{};
	virtual ~CBomChuck() {};

	inline uint Num() const { return mNum; };
	inline void Num(uint v) { mNum = v; };

	inline std::string Name() const { return mName; };
	inline void Name(std::string v) { mName = v; };

	inline uint Height() const { return mHeight; };
	inline void Height(uint v) { mHeight = v; };

	inline CBrdLoc const &Loc() const { return mLoc; };
	inline void Loc(CBrdLoc v) { mLoc = v; };

	std::string Parse(std::string const &str);
	std::string Export() const;
protected:
	uint mNum;
	std::string mName;
	std::string mIdOd;
	uint mHeight;
	CBrdLoc mLoc;
};

class CBom
{
public:
	CBom()
		: mbRotate90(true)
		{};
	virtual ~CBom() {};

	std::string ImportPickup(std::string fname);
	std::string ExportSequence(std::string fname, std::string fnameRef, std::string fnamePre, std::string fnamePost, bool bImageRepeat, bool bSortSizeThenChuck = true);
	std::string ExportPickupExtent(std::string fname);

	std::string ImportPlace(std::string fname, CBrdLoc const &home);
	std::string ExportPlace(std::string fname, std::string fnameRef);

	std::string ImportChuck(std::string fname);

	inline void Rotate90(bool v) { mbRotate90 = v; };
	inline bool Rotate90() const { return mbRotate90; };

protected:

	std::vector<CBomPickup> mPickup;
	// std::map<std::string, uint>mPartPlaceNum;
	std::vector<std::string> mPartPlaceName;
	std::map<std::string, std::pair<uint, uint>> mPartPickupPlaceNum;
	std::map<std::string, CBomPlace> mPlace;
	std::vector<CBomChuck> mChuck;
	bool mbRotate90;
};
#endif