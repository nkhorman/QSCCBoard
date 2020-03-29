#ifndef _CBOM_H_

#include <string>
#include <map>
#include <functional>

#include "CBrdLoc.h"

class CBomPlace
{
public:
	CBomPlace() {};
	virtual ~CBomPlace() {};
protected:
	CBrdLoc mLoc;
};

class CBomPickup
{
public:
	CBomPickup() {};
	virtual ~CBomPickup() {};

	inline uint Num() const { return mNum; };
	inline void Num(uint v) { mNum = v; };

	inline uint Chuck() const { return mChuck; };
	inline void Chuck(uint v) { mChuck = v; };

	inline void Part(std::string const &v) { mParts.push_back(v); };
	// inline std::vector<std::string> const &Parts() const { return mParts; };
	inline void Parts(std::function<void(std::string const &)> fn)
	{
		if(fn)
			std::for_each(mParts.begin(), mParts.end(), [&](std::string const &item) { fn(item); });
	}

	std::string Parse(std::string const &str);
	std::string Export(uint &lastChuckNum, uint &lastItemNum, uint pickupNum) const;
protected:
	uint mNum; // pickup number
	uint mChuck;
	std::vector<std::string> mParts;
};

class CBom
{
public:
	CBom() {};
	virtual ~CBom() {};

	std::string ImportPickup(std::string fname);
	std::string ExportPickup(std::string fname);

	std::string ImportPlace(std::string fname);
protected:

	std::vector<CBomPickup> mPickup;
	std::map<std::string, CBomPlace> mPlace;
};
#endif