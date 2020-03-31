#ifndef _CBRDLOC_H_
#define _CBRDLOC_H_

// #include <stdint.h>

#include <string>
// #include <cstdint>

class CBrdLoc
{
public:
	CBrdLoc()
		: mX(0)
		, mY(0)
		, mZ(0)
		, mT(0)
		{};
	CBrdLoc(uint x, uint y, uint z, uint t)
		: mX(x)
		, mY(y)
		, mZ(z)
		, mT(t)
		{};
	virtual ~CBrdLoc() {};

	void Parse(std::string const &k, std::string const &v);
	void Parse(std::string const &kvp);

	inline uint x() const { return mX; };
	inline void x(uint v) { mX = v; };

	inline uint y() const { return mY; };
	inline void y(uint v) { mY = v; };

	inline uint z() const { return mZ; };
	inline void z(uint v) { mZ = v; };

	inline uint t() const { return mT; };
	inline void t(uint v) { mT = v; };

	void Offset(CBrdLoc const &offset);

	std::string Dump() const;
protected:
	uint mX;
	uint mY;
	uint mZ;
	uint mT;
};

#endif