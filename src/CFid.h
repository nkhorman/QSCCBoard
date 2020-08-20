#ifndef _CFID_H_
#define _CFID_H_

#include <vector>
#include <map>
#include "CBrdLoc.h"

class CFiducial
{
public:
	CFiducial() {};
	CFiducial(CBrdLoc home)
		: mMachineHome(home)
		, mbRotate90(true)
		{};
	virtual ~CFiducial() {};
	
	std::string Import(std::string fname);
	std::string Export(std::string fname);

	inline void Rotate90(bool v) { mbRotate90 = v; };
	inline bool Rotate90() const { return mbRotate90; };
protected:
	std::string Parse(std::string &line);
	std::string ExportRef(std::string prefix, uint num, CBrdLoc loc, std::map<std::string, uint> lighting);

	std::string mName;
	std::vector<CBrdLoc> mFiducials;
	CBrdLoc mMachineHome;
	bool mbRotate90;
};

#endif