#ifndef _CFID_H_
#define _CFID_H_

#include <vector>
#include "CBrdLoc.h"

class CFiducial
{
public:
	CFiducial() {};
	CFiducial(CBrdLoc home)
		: mMachineHome(home)
		{};
	virtual ~CFiducial() {};
	
	std::string Import(std::string fname);
	std::string Export(std::string fname);
protected:
	std::string Parse(std::string &line);
	std::string ExportRef(std::string prefix, uint num, CBrdLoc loc);

	std::string mName;
	std::vector<CBrdLoc> mFiducials;
	CBrdLoc mMachineHome;
};

#endif