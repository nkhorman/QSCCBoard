#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <string>
#include <cstdint>
#include <vector>

// for_each thoughts - https://www.fluentcpp.com/2018/03/30/is-stdfor_each-obsolete/


// 00	Sequence count lower byte
// 01	Pick count
// 02	Place count
// 03	Chuck count
// 04	Repeat Pickup count
// 05	Repeat Place count
// 06	Extent count
// 07	Filesize low byte
// 08	Filesize high byte, add 0xa8
// 09	00	?
// 0A	00	?
// 0B	00	?
// 0C	00	?
// 0D	00	?
// 0E	00	?
// 0F	00	Bit 0 = MSB of sequence count
// 10	Sequence start
// pickup start = Sequence start + (sequence count * 4)
// place start  = Pickup start   + (pickup count * 10)
// Chuck start  = Place start    + (place count * 10)
// RepeatPickup start = Chuck start + (chuck count * 10)
// RepeatPlace start = RepeatPickup start + (repeat pickup count * 10)
// Extent start = RepeatPlace start    + (repeat place count * 10)


typedef struct __attribute__((packed)) _brd_hdr_t
{
	uint8_t seq_lo;
	uint8_t count_pick;
	uint8_t count_place;
	uint8_t count_chuck;
	uint8_t count_repeat_pickup;
	uint8_t count_repeat_place;
	uint8_t count_extent;
	uint8_t size_file_low;
	uint8_t size_file_hi; // add 0xa8
	uint8_t res2[6];
	uint8_t seq_hi;
} brd_hdr_t;

// Sequence format
// Offset 0	Command : 0x01 = pickup		parameter = pickup index
// 		: 0x02 = place		parameter = place index
// 		: 0x03 = chuck (nozzle)	parameter = nozzle index
// 		: 0x10 = transport	parameter = 0-> hold, 1-> pass
// Offset 1	Parameter
// Offset 2	0xff
// Offset 3	0xff

typedef struct __attribute__((packed)) _brd_fmt_seq_t
{
	uint8_t cmd;
	uint8_t param[3];
}brd_fmt_seq_t;

typedef struct __attribute__((packed)) _brd_loc_t
{
	uint16_t x;
	uint16_t y;
	uint16_t t;
	uint16_t z;
} brd_loc_t;

// Pickup format
// Offset 0	Alternate pickup index
// Offset 1	Extent (used for additional information such as laser align data etc...)
// Offset 2	X low byte
// Offset 3	X high byte
// Offset 4	Y low byte
// Offset 5	Y high byte
// Offset 6	T low byte
// Offset 7	T high byte
// Offset 8	Z low byte
// Offset 9	Z high byte

// Place format
// Offset 0	Alternate pickup index
// Offset 1	Extent (00)
// Offset 2	X low byte
// Offset 3	X high byte
// Offset 4	Y low byte
// Offset 5	Y high byte
// Offset 6	T low byte
// Offset 7	T high byte
// Offset 8	Z low byte
// Offset 9	Z high byte

// Chuck format
// Offset 0	(00)
// Offset 1	Extent (00)
// Offset 2	X low byte
// Offset 3	X high byte
// Offset 4	Y low byte
// Offset 5	Y high byte
// Offset 6	T low byte
// Offset 7	T high byte
// Offset 8	Z low byte
// Offset 9	Z high byte

typedef struct __attribute__((packed)) _brd_fmt_pickplacechuck_t
{
	uint8_t altIndex;
	uint8_t extent; 
	brd_loc_t loc;
}brd_fmt_pickplacechuck_t;

// Extent format
// Transport extent (board information)
// Extent A
// Offset 0	Command (06)
// Offset 1	Extent B pointer (current + 1)
// Offset 2	Board width low byte
// Offset 3	Board width high byte
// Offset 4	Pin/edge registration : 0 = edge, 1 = pin
// Offset 5	BRSTRT (E0)
// Offset 6	BMAX (75)
// Offset 7	BA/DECL low byte (64)
// Offset 8	BA/DECL high byte (01)
// Offset 9	00

typedef struct __attribute__((packed)) _brd_fmt_extent_transport_a_t
{
	uint8_t extentBPtr; // (current + 1)
	uint16_t boardWidth;
	uint8_t pinEdgeRegistration; // 0=edge, 1=pin
	uint8_t brstrt; // E0
	uint8_t bmax; // 75
	uint8_t baDeclLo; // 64
	uint8_t baDecHi; // 01
	uint8_t res1; // 0
}brd_fmt_extent_transport_a_t;

// Extent B
// Offset 0	Command (07)
// Offset 1	00
// Offset 2	Gap (50)
// Offset 3	DIS2PIN (36)
// Offset 4	RSTRT (E0)
// Offset 5	RMAX (5A)
// Offset 6	RAC/DECL low byte (C8)
// Offset 7	RAC/DECL low byte (01)
// Offset 8	00
// Offset 9	00

typedef struct __attribute__((packed)) _brd_fmt_extent_transport_b_t
{
	uint8_t res1; // 0
	uint8_t gap; // 50
	uint8_t distance2pin; // 36
	uint8_t rStrt; // e0
	uint8_t rMax; // 5a
	uint16_t racDecl; // c801
	uint16_t res2;
}brd_fmt_extent_transport_b_t;

// LAE : describes component dimensions and laser align point & method
// Offset 0	Command (0C)
// Offset 1	00
// Offset 2	Width low byte
// Offset 3	[2:0] = Width high byte, [7:3] = width tol
// Offset 4	Length low byte
// Offset 5	[2:0] = Length high byte, [7:3] = length tol.
// Offset 6	Z measure point low byte
// Offset 7	[1:0] = Z measure high byte, [7:2] = Measure type
// Offset 8	[6:0] = pickup delay, [7] = VAC ver.
// Offset 9	00

typedef struct __attribute__((packed)) _brd_fmt_extent_lae_t
{
	uint8_t extent;
	uint8_t widthLo;
	uint8_t widthTolHi;
	uint8_t lengthLo;
	uint8_t lengthTolHi;
	uint8_t zLo;
	uint8_t zTypeHi;
	uint8_t vacVerPickupDelay;
	uint8_t res2;
}brd_fmt_extent_lae_t;


// Pickup repeat : Used for waffle tray definitions
// Offset 0	Command (01)
// Offset 1	Next extent (possibly point to LAE?)
// Offset 2	Column step HIGH byte [3:0], current count [7:4] ?
// Offset 3	Column step LOW byte. NB high and low not same order as other numbers.
// Offset 4	Row step HIGH byte [3:0], current count [7:4] ?
// Offset 5	Row step LOW byte. NB high and low not same order as other numbers.
// Offset 6	Columns (X)	
// Offset 7	Rows (Y)
// Offset 8	FF
// Offset 9	FF

typedef struct __attribute__((packed)) _brd_fmt_extent_pickup_repeat_t
{
	uint8_t extent;
	uint8_t columnStepHiCount;
	uint8_t columnStepLo;
	uint8_t rowStepHiCount;
	uint8_t rowStepLo;
	uint8_t columns;
	uint8_t rows;
	uint16_t res1;
}brd_fmt_extent_pickup_repeat_t;

typedef struct __attribute__((packed)) _brd_fmt_extent_t
{
	uint8_t cmd;
	union
	{
		brd_fmt_extent_pickup_repeat_t pure; // cmd 1
		brd_fmt_extent_transport_a_t transport_a; // cmd 6
		brd_fmt_extent_transport_b_t transport_b; // cmd 7
		brd_fmt_extent_lae_t lae; // cmd 12
	};
}brd_fmt_extent_t;

// Repeat - Place
// Offset 0 - 183
// Offset 1 - 7
// Offset 2 - 0
// Offset 3 - 0
// Offset 4 - 0
// Offset 5 - 0
// Offset 6 - 0
// Offset 7 - 0
// Offset 8 - 3 - col
// Offset 9 - 1 - rows

typedef struct __attribute__((packed)) _brd_place_repeat_t
{
	uint16_t imageColSpan;
	uint8_t res1[2];
	uint16_t imageRowSpan;
	uint8_t res2[2];
	uint8_t col;
	uint8_t row;
}brd_place_repeat_t;


class CBrdInfo
{
public:
	CBrdInfo()
		: mCountSequence(0)
		, mCountPick(0)
		, mCountPlace(0)
		, mCountChuck(0)
		, mCountRepeatPick(0)
		, mCountRepeatPlace(0)
		, mCountExtent(0)
		, mFileSize(0)
		{};
	CBrdInfo(brd_hdr_t hdr)
		: mCountSequence(0)
		, mCountPick(0)
		, mCountPlace(0)
		, mCountChuck(0)
		, mCountRepeatPick(0)
		, mCountRepeatPlace(0)
		, mCountExtent(0)
		, mFileSize(0)
		{
			insert(hdr);
		};

	friend std::istream &operator>>(std::istream &stream, CBrdInfo &hdr)
	{
		brd_hdr_t brdHdr;

		stream.read((char *)&brdHdr, sizeof(brdHdr));
		hdr.insert(brdHdr);

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBrdInfo &hdr)
	{
		brd_hdr_t brdHdr = hdr.extract();

		stream.write((char const *)&brdHdr, sizeof(brdHdr));

		return stream;
	};

	void insert(brd_hdr_t hdr)
	{
		// uint8_t *buf = (uint8_t *)&hdr;
		// std::cout
		// 	<< "header - "
		// 	// << "0: " << (uint)buf[0] << ", "
		// 	// << "1: " << (uint)buf[1] << ", "
		// 	// << "2: " << (uint)buf[2] << ", "
		// 	// << "3: " << (uint)buf[3] << ", "
		// 	// << "Repeat Pick count: " << (uint)buf[4] << ", "
		// 	// << "Repeat Place count: " << (uint)buf[5] << ", "
		// 	// << "Extent Count: " << (uint)buf[6] << ", "
		// 	// << "7: " << (uint)buf[7] << ", "
		// 	// << "8: " << (uint)buf[8] << ", "
		// 	<< "9: " << (uint)buf[9] << ", "
		// 	<< "10: " << (uint)buf[10] << ", "
		// 	<< "11: " << (uint)buf[11] << ", "
		// 	<< "12: " << (uint)buf[12] << ", "
		// 	<< "13: " << (uint)buf[13] << ", "
		// 	<< "14: " << (uint)buf[14] << ", "
		// 	<< "15: " << (uint)buf[15]
		// 	<< std::endl
		// 	;
		mCountSequence = (hdr.seq_lo | ((hdr.seq_hi & 0x01) << 8));
		mCountPick = hdr.count_pick;
		mCountPlace = hdr.count_place;
		mCountChuck = hdr.count_chuck;
		mCountRepeatPick = hdr.count_repeat_pickup;
		mCountRepeatPlace = hdr.count_repeat_place;
		mCountExtent = hdr.count_extent;
		mFileSize = (hdr.size_file_low | ((hdr.size_file_hi - 0xa8) << 8));
	};

	brd_hdr_t extract() const
	{
		brd_hdr_t hdr;

		hdr.seq_lo = mCountSequence & 0xff;
		hdr.seq_hi = (mCountSequence & 0x0100) >> 8;
		hdr.count_pick = mCountPick & 0xff;
		hdr.count_place = mCountPlace & 0xff;
		hdr.count_chuck = mCountChuck & 0xff;
		hdr.count_repeat_pickup = mCountRepeatPick & 0xff;
		hdr.count_repeat_place = mCountRepeatPlace & 0xff;
		hdr.count_extent = mCountExtent & 0xff;
		hdr.size_file_low = mFileSize & 0xff;
		hdr.size_file_hi = ((mFileSize & 0xff00) >> 8) + 0xa8;

		return hdr;
	}

	uint mCountSequence;
	uint mCountPick;
	uint mCountPlace;
	uint mCountChuck;
	uint mCountRepeatPick;
	uint mCountRepeatPlace;
	uint mCountExtent;
	uint mFileSize;

	std::string Dump() const
	{
		std::ostringstream oss;

		oss
			<< "Filesize: " << mFileSize << ", "
			<< "Sequence: " << mCountSequence << ", "
			<< "Pick: " << mCountPick << ", "
			<< "Place: " << mCountPlace << ", "
			<< "Chuck: " << mCountChuck << ", "
			<< "Repeat Pickup: " << mCountRepeatPick << ", "
			<< "Repeat Place: " << mCountRepeatPlace << ", "
			<< "Extent: " << mCountExtent
			;

		return oss.str();
	}
};

class CBrdSeq
{
public:
	CBrdSeq()
		: mCmd(0)
		, mParam(0)
		{};
	CBrdSeq(brd_fmt_seq_t seq)
		: mCmd(0)
		{
			insert(seq);
		};
	virtual ~CBrdSeq() {};

	friend std::istream &operator>>(std::istream &stream, CBrdSeq &seq)
	{
		brd_fmt_seq_t brdSeq;

		stream.read((char *)&brdSeq, sizeof(brdSeq));
		seq.insert(brdSeq);

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBrdSeq &seq)
	{
		brd_fmt_seq_t brdSeq = seq.extract();

		stream.write((char const *)&brdSeq, sizeof(brdSeq));

		return stream;
	};

	void insert(brd_fmt_seq_t v)
	{
			mCmd = v.cmd;
			mParam.clear();
			mParam.push_back(v.param[0]);
			mParam.push_back(v.param[1]);
			mParam.push_back(v.param[2]);
	};

	brd_fmt_seq_t extract() const
	{
		brd_fmt_seq_t seq;

		seq.cmd = mCmd & 0xff;
		seq.param[0] = mParam[0] & 0xff;
		seq.param[1] = mParam[1] & 0xff;
		seq.param[2] = mParam[2] & 0xff;

		return seq;
	}

	inline uint Cmd() const { return mCmd; };
	inline void Cmd(uint v) { mCmd = v; };

	inline std::vector<uint> Param() const { return mParam; };
	inline void Param(std::vector<uint> v) { mParam = v; };
	
	std::string Dump() const
	{
		std::ostringstream oss;

		switch(Cmd())
		{
			case 0: oss << "stop"; break;
			case 1: oss << "pickup " << Param()[0] << (Param()[2] == 223 ? " lead inspection" : ""); break;
			case 2:
				oss << "place " << Param()[0];
				switch(Param()[1])
				{
					case 10: oss << " dispense only"; break;
					case 11: oss << " dispense and place"; break;
					// case 255: oss << " place only"; break;
				}
				break;
			case 3: oss << "chuck " << Param()[0]; break;
			case 4: oss << "test " << Param()[0]; break;
			case 5: oss << "repeat " << Param()[0] << (Param()[1] == 9 ? " image" : ""); break;
			case 6: oss << "goto " << Param()[0]; break;
			case 7:
				oss << "wait address " << Param()[0] << " port " << Param()[1];
				switch(Param()[2])
				{
					case 3: oss << " pulse"; break;
					case 2: oss << " toggle"; break;
					case 1: oss << " on"; break;
					case 0: oss << " off"; break;
				}
				break;
			case 8:
				oss << "output address " << Param()[0] << " port " << Param()[1];
				switch(Param()[2])
				{
					case 3: oss << " pulse"; break;
					case 2: oss << " toggle"; break;
					case 1: oss << " on"; break;
					case 0: oss << " off"; break;
				}
				break;
			case 9:
				oss << "home ";
				switch(Param()[0])
				{
					case 4: oss << "t"; break;
					case 3: oss << "z"; break;
					case 2: oss << "y"; break;
					case 1: oss << "x"; break;
					case 0: oss << "all"; break;
				}
				break;
			case 10:
				oss << "if address " << Param()[0] << " port " << Param()[1];
				switch(Param()[2])
				{
					case 3: oss << " pulse"; break;
					case 2: oss << " toggle"; break;
					case 1: oss << " on"; break;
					case 0: oss << " off"; break;
				}
				break;
			case 11: oss << "run " << Param()[0]; break;
			case 15: oss << "mod code " << Param()[0] << " = " << Param()[1]; break;
			case 16:
				oss << "transport";
				switch(Param()[0])
				{
					case 0: oss << "hold"; break;
					case 1: oss << "pass"; break;
					default: oss << "unknown, param: " << Param()[0]; break;
				}
				break;
			case 17: oss << "fetch tray - elevator " << Param()[0] << " tray " << Param()[1]; break;
			default:
				oss << "cmd: " << Cmd() << "/uknown"
				<< ", param1: " << Param()[0]
				<< ", param2: " << Param()[1]
				<< ", param3: " << Param()[2]
				;
				break;
		}

		return oss.str();
	}

protected:
	uint mCmd;
	std::vector<uint> mParam;
};

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

	inline uint x() const { return mX; };
	inline uint y() const { return mY; };
	inline uint z() const { return mZ; };
	inline uint t() const { return mT; };

	std::string Dump() const
	{
		std::ostringstream oss;

		oss << "X: " << mX << ", Y: " << mY << ", Z: " << mZ << ", T: " << mT;

		return oss.str();
	}
protected:
	uint mX;
	uint mY;
	uint mZ;
	uint mT;
};

class CBrdPPC
{
public:
	CBrdPPC()
		: mAltIndex(0)
		, mExtent(0)
		{};
	CBrdPPC(brd_fmt_pickplacechuck_t ppc)
		: mAltIndex(ppc.altIndex)
		, mExtent(ppc.extent)
		, mLoc(CBrdLoc(ppc.loc.x, ppc.loc.y, ppc.loc.z, ppc.loc.t))
		{};
	virtual ~CBrdPPC() {};
	
	friend std::istream &operator>>(std::istream &stream, CBrdPPC &ppc)
	{
		brd_fmt_pickplacechuck_t brdPpc;

		stream.read((char *)&brdPpc, sizeof(brdPpc));
		ppc.insert(brdPpc);

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBrdPPC &ppc)
	{
		brd_fmt_pickplacechuck_t brdPpc = ppc.extract();

		stream.write((char const *)&brdPpc, sizeof(brdPpc));

		return stream;
	};

	void insert(brd_fmt_pickplacechuck_t v)
	{
		mAltIndex = v.altIndex;
		mExtent = v.extent;
		mLoc = CBrdLoc(v.loc.x, v.loc.y, v.loc.z, v.loc.t);
	};

	brd_fmt_pickplacechuck_t extract() const
	{
		brd_fmt_pickplacechuck_t brdPpc;

		brdPpc.altIndex = mAltIndex;
		brdPpc.extent = mExtent;
		brdPpc.loc.x = mLoc.x() & 0xffff;
		brdPpc.loc.y = mLoc.y() & 0xffff;
		brdPpc.loc.z = mLoc.z() & 0xffff;
		brdPpc.loc.t = mLoc.t() & 0xffff;

		return brdPpc;
	}

	inline uint AltIndex() const { return mAltIndex; };
	inline uint Extent() const { return mExtent; };
	inline CBrdLoc const &Loc() const { return mLoc; };

	std::string Dump() const
	{
		std::ostringstream oss;

		if(mAltIndex)
			oss << "AltIndex: " << mAltIndex << ", ";
		if(mExtent)
			oss << "Extent: " << mExtent << ", ";
		oss << mLoc.Dump();

		return oss.str();
	}

protected:
	uint mAltIndex;
	uint mExtent;
	CBrdLoc mLoc;
};

class CBrdMeas
{
public:
	CBrdMeas()
		: mVal(0)
		, mTol(0)
		{};
	CBrdMeas(uint val, uint tol)
		: mVal(val)
		, mTol(tol)
		{};
	virtual ~CBrdMeas() {};

	inline void Val(uint v) { mVal = v; };
	inline uint Val() const { return mVal; };

	inline void Tol(uint v) { mTol = v; };
	inline uint Tol() const { return mTol; };

	std::string Dump() const
	{
		std::ostringstream oss;

		oss << mVal;
		if(mTol)
			oss << ", Tol: " << mTol;

		return oss.str();
	}
protected:
	uint mVal;
	uint mTol;
};

class CBrdExtentLAE
{
public:
	CBrdExtentLAE()
		: mNext(0)
		, mMeasureType(0)
		, mPickupDelay(0)
		, mbVacVerify(false)
		{};
	CBrdExtentLAE(brd_fmt_extent_lae_t v)
		: mNext(v.extent)
		, mWidth(CBrdMeas(v.widthLo | ((v.widthTolHi & 0x07) << 8), v.widthTolHi >> 3))
		, mLength(CBrdMeas(v.lengthLo | ((v.lengthTolHi & 0x07) << 8), v.lengthTolHi >> 3))
		, mHeight(CBrdMeas(v.zLo | ((v.zTypeHi&0x03) << 8), 0))
		, mMeasureType(v.zTypeHi >> 2)
		, mPickupDelay(v.vacVerPickupDelay & 0x7f)
		, mbVacVerify((v.vacVerPickupDelay & 0x80) != 0)
		{};
	virtual ~CBrdExtentLAE() {};
	
	void insert(brd_fmt_extent_lae_t v)
	{
		mNext = v.extent;
		mWidth = CBrdMeas(v.widthLo | ((v.widthTolHi & 0x07) << 8), v.widthTolHi >> 3);
		mLength = CBrdMeas(v.lengthLo | ((v.lengthTolHi & 0x07) << 8), v.lengthTolHi >> 3);
		mHeight = CBrdMeas(v.zLo | ((v.zTypeHi&0x03) << 8), 0);
		mMeasureType = v.zTypeHi >> 2;
		mPickupDelay = v.vacVerPickupDelay & 0x7f;
		mbVacVerify = ((v.vacVerPickupDelay & 0x80) != 0);
	};

	brd_fmt_extent_lae_t extract() const
	{
		brd_fmt_extent_lae_t lae;

		lae.extent = mNext & 0xff;
		lae.widthLo = mWidth.Val() & 0xff;
		lae.widthTolHi = ((mWidth.Val() & 0x0700) >> 8) | ((mWidth.Tol() & 0x7f) << 3);
		lae.lengthLo = mLength.Val() & 0xff;
		lae.lengthTolHi = ((mLength.Val() & 0x0700) >> 8) | ((mLength.Tol() & 0x7f) << 3);
		lae.zLo = mHeight.Val() & 0xff;
		lae.zTypeHi = ((mHeight.Val() & 0x0300) >> 8) | ((mMeasureType & 0x3f) << 2);
		lae.vacVerPickupDelay = (mPickupDelay & 0x7f) | (mbVacVerify ? 0x80 : 0);

		return lae;
	}

	inline void Next(uint v) { mNext = v; };
	inline void Width(CBrdMeas v) { mWidth = v; };
	inline void Length(CBrdMeas v) { mLength = v; };
	inline void Height(CBrdMeas v) { mHeight = v; };

	inline void MeasureType(uint v) { mMeasureType = v; };
	inline void PickupDelay(uint v) { mPickupDelay = v; };
	inline void VacVerify(bool v) { mbVacVerify = v; };

	std::string Dump() const
	{
		std::ostringstream oss;

		oss
			<< "Length: " << mLength.Dump() << ", "
			<< "Width: " << mWidth.Dump() << ", "
			<< "Height: " << mHeight.Dump() << ", "
			;
		switch(mMeasureType)
		{
			default: oss << "MeasureType: " << mMeasureType << ", "; break;
		}

		oss << "PickupDelay: " << mPickupDelay << ", "
			<< "VacVerify: " << mbVacVerify
			;

		if(mNext > 0)
			oss << ", Next: " << mNext;

		return oss.str();
	}
	
protected:
	uint mNext;
	CBrdMeas mWidth;
	CBrdMeas mLength;
	CBrdMeas mHeight;
	uint mMeasureType;
	uint mPickupDelay;
	bool mbVacVerify;
};

class CBrdExtentRepeatPickup
{
public:
	CBrdExtentRepeatPickup()
		: mNext(0)
		, mColumnCount(0)
		, mColumnStep(0)
		, mRowCount(0)
		, mRowStep(0)
		, mColumns(0)
		, mRows(0)
		{};
	CBrdExtentRepeatPickup(brd_fmt_extent_pickup_repeat_t v)
		: mNext(v.extent)
		, mColumnCount(v.columnStepHiCount >> 3)
		, mColumnStep(v.columnStepLo | ((v.columnStepHiCount & 0x07) << 8))
		, mRowCount(v.rowStepHiCount >> 3)
		, mRowStep(v.rowStepLo | ((v.rowStepHiCount & 0x07) << 8))
		, mColumns(v.columns)
		, mRows(v.rows)
		{};
	
	void insert(brd_fmt_extent_pickup_repeat_t v)
	{
		mNext = v.extent;
		mColumnCount = v.columnStepHiCount >> 3;
		mColumnStep = v.columnStepLo | ((v.columnStepHiCount & 0x07) << 8);
		mRowCount = v.rowStepHiCount >> 3;
		mRowStep = v.rowStepLo | ((v.rowStepHiCount & 0x07) << 8);
		mColumns = v.columns;
		mRows = v.rows;
	};

	brd_fmt_extent_pickup_repeat_t extract() const
	{
		brd_fmt_extent_pickup_repeat_t brdPure;

		brdPure.extent = mNext & 0xff;
		brdPure.columnStepHiCount = ((mColumnCount & 0x7f) << 3) | ((mColumnStep & 0x3f) >> 8);
		brdPure.columnStepLo = mColumnStep & 0xff;
		brdPure.rowStepHiCount = ((mRowCount & 0x7f) << 3) | ((mRowStep & 0x3f) >> 8);
		brdPure.rowStepLo = mRowStep & 0xff;
		brdPure.columns = mColumns & 0xff;
		brdPure.rows = mRows & 0xff;

		return brdPure;
	}

	virtual ~CBrdExtentRepeatPickup() {};

	inline uint Next() const { return mNext; };
	inline void Next(uint v) { mNext = v; };
	
	inline uint ColumnCount() const { return mColumnCount; };
	inline void ColumnCount(uint v) { mColumnCount = v; };
	
	inline uint ColumnStep() const { return mColumnStep; };
	inline void ColumnStep(uint v) { mColumnStep = v; };
	
	inline uint RowCount() const { return mRowCount; };
	inline void RowCount(uint v) { mRowCount = v; };
	
	inline uint RowStep() const { return mRowStep; };
	inline void RowStep(uint v) { mRowStep = v; };
	
	inline uint Columns() const { return mColumns; };
	inline void Columns(uint v) { mColumns = v; };
	
	inline uint Rows() const { return mRows; };
	inline void Rows(uint v) { mRows = v; };

	std::string Dump() const
	{
		std::ostringstream oss;

		oss
			<< "Columns: " << mColumns << ", "
			<< "Rows: " << mRows << ", "
			<< "ColumnStep: " << mColumnStep << ", "
			<< "RowStep: " << mRowStep << ", "
			<< "ColumnCount: " << mColumnCount << ", "
			<< "RowCount: " << mRowCount
			;

		if(mNext > 0)
			oss << ", Next: " << mNext;

		return oss.str();
	}
	
protected:
	uint mNext;
	uint mColumnCount;
	uint mColumnStep;
	uint mRowCount;
	uint mRowStep;
	uint mColumns;
	uint mRows;
};

class CBrdExtent
{
public:
	CBrdExtent()
		: mCmd(0)
		{};
	CBrdExtent(brd_fmt_extent_t brdExtent)
		: mCmd(0)
		{
			insert(brdExtent);
		};
	virtual ~CBrdExtent() {};
	
	friend std::istream &operator>>(std::istream &stream, CBrdExtent &extent)
	{
		brd_fmt_extent_t brdExtent;

		stream.read((char *)&brdExtent, sizeof(brdExtent));
		extent.insert(brdExtent);

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBrdExtent &extent)
	{
		brd_fmt_extent_t brdExtent = extent.extract();

		stream.write((char const *)&brdExtent, sizeof(brdExtent));

		return stream;
	};

	void insert(brd_fmt_extent_t v)
	{
		Cmd(v.cmd);

		switch(v.cmd)
		{
			case 12: Lae(CBrdExtentLAE(v.lae)); break;
			case 1: Repeat(CBrdExtentRepeatPickup(v.pure)); break;
			// case 6: TransportA(CBrdExtentTransportA(v.transport_a)); break;
			// case 7: TransportB(CBrdExtentTransportB(v.transport_b)); break;
		}
	};

	brd_fmt_extent_t extract() const
	{
		brd_fmt_extent_t brdExtent;

		brdExtent.cmd = Cmd() & 0xff;

		switch(brdExtent.cmd)
		{
			case 12: brdExtent.lae = Lae().extract(); break;
			case 1: brdExtent.pure = Repeat().extract(); break;
			// case 6: brdExtent.transport_a = TransportA().extract(); break;
			// case 7: brdExtent.transport_b = TransportB().extract(); break;
		}

		return brdExtent;
	}

	inline void Cmd(uint v) { mCmd = v; };
	inline uint Cmd() const { return mCmd; };

	inline void Lae(CBrdExtentLAE v) { mLae = v; };
	CBrdExtentLAE const &Lae() const { return mLae; };

	inline void Repeat(CBrdExtentRepeatPickup v) { mRepeat = v; };
	inline CBrdExtentRepeatPickup const &Repeat() const { return mRepeat; };

	// inline void TransportA(CBrdExtentTransportA v) { mTransportA = v; };
	// inline CBrdExtentTransportA const &TransportA() const { return mTransportA; };

	// inline void TransportB(CBrdExtentTransportB v) { mTransportB = v; };
	// inline CBrdExtentTransportB const &TransportB() const { return mTransportB; };

	std::string Dump() const
	{
		std::ostringstream oss;

		switch(mCmd)
		{
			case 12:
				oss << "LAE - " << mLae.Dump();
				break;
			case 1:
				oss << "Repeat Pickup - " << mRepeat.Dump();
				break;
			case 6:
				oss << "Transport A";
				break;
			case 7:
				oss << "Transport B";
				break;
			default: oss << "cmd: " << mCmd;
		}

		return oss.str();
	}
protected:
	uint mCmd;
	CBrdExtentLAE mLae;
	CBrdExtentRepeatPickup mRepeat;
	// CBrdExtentTransportA mTransportA;
	// CBrdExtentTransportB mTransportB;
};

class CBrdRepeatPlace
{
public:
	CBrdRepeatPlace()
		: mCol(0)
		, mRow(0)
		, mImageSpanCol(0)
		, mImageSpanRow(0)
		{};
	CBrdRepeatPlace(brd_place_repeat_t brdPlRe)
		{
			insert(brdPlRe);
		};
	virtual ~CBrdRepeatPlace() {};
	
	friend std::istream &operator>>(std::istream &stream, CBrdRepeatPlace &repl)
	{
		brd_place_repeat_t brdPlRe;

		stream.read((char *)&brdPlRe, sizeof(brdPlRe));
		repl.insert(brdPlRe);

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBrdRepeatPlace &repl)
	{
		brd_place_repeat_t brdPlRe = repl.extract();

		stream.write((char const *)&brdPlRe, sizeof(brdPlRe));

		return stream;
	};

	void insert(brd_place_repeat_t v)
	{
		// uint8_t *pbuf = (uint8_t *)&buf;
		// std::cout
		// 	<< "repeat detail - "
		// 	<< "0: " << (uint)pbuf[0] << ", "
		// 	<< "1: " << (uint)pbuf[1] << ", "
		// 	<< "2: " << (uint)pbuf[2] << ", "
		// 	<< "3: " << (uint)pbuf[3] << ", "
		// 	<< "4: " << (uint)pbuf[4] << ", "
		// 	<< "5: " << (uint)pbuf[5] << ", "
		// 	<< "6: " << (uint)pbuf[6] << ", "
		// 	<< "7: " << (uint)pbuf[7] << ", "
		// 	<< "8: " << (uint)pbuf[8] << ", "
		// 	<< "9: " << (uint)pbuf[9]
		// 	<< std::endl
		// 	;
		mImageSpanRow = v.imageRowSpan;
		mImageSpanCol = v.imageColSpan;
		mRow = v.row;
		mCol = v.col;
	};

	brd_place_repeat_t extract() const
	{
		brd_place_repeat_t brdPlRe;

		brdPlRe.imageRowSpan = mImageSpanRow;
		brdPlRe.imageColSpan = mImageSpanCol;
		brdPlRe.row = mRow;
		brdPlRe.col = mCol;

		return brdPlRe;
	}

	std::string Dump() const
	{
		std::ostringstream oss;

		oss
			<< "Col: " << mCol << ", "
			<< "Row: " << mRow << ", "
			<< "ImageSpanCol: " << mImageSpanCol << ", "
			<< "ImageSpanRow: " << mImageSpanRow
			;

		return oss.str();
	}

protected:
	uint mCol;
	uint mRow;
	uint mImageSpanCol;
	uint mImageSpanRow;
};


class CBoard
{
public:
	CBoard() {}
	virtual ~CBoard() {};

	friend std::istream &operator>>(std::istream &stream, CBoard &brd)
	{
		// read
		CBrdInfo info;

		stream >> info;
		brd.Info(info);

		CBrdSeq seq;
		for(uint i=0,q=info.mCountSequence; i<q; i++) { stream >> seq; brd.Seq(seq); }

		CBrdPPC ppc;
		for(uint i=0,q=info.mCountPick; i<q; i++) { stream >> ppc; brd.Pickup(ppc); }
		for(uint i=0,q=info.mCountPlace; i<q; i++) { stream >> ppc; brd.Place(ppc); }
		for(uint i=0,q=info.mCountChuck; i<q; i++) { stream >> ppc; brd.Chuck(ppc); }

		CBrdRepeatPlace brp;
		for(uint i=0,q=info.mCountRepeatPick; i<q; i++) { stream >> brp; brd.RepeatPickup(brp); }
		for(uint i=0,q=info.mCountRepeatPlace; i<q; i++) { stream >> brp; brd.RepeatPlace(brp); }

		CBrdExtent lae;
		for(uint i=0,q=info.mCountExtent; i<q; i++) { stream >> lae; brd.Extent(lae); }

		return stream;
	};

	friend std::ostream &operator<<(std::ostream &stream, const CBoard &brd)
	{
		// write
		return stream;
	};

	inline void Info(CBrdInfo info) { mInfo = info; };
	inline CBrdInfo const &Info() const { return mInfo; };
	
	inline void Seq(CBrdSeq seq) { mSeq.push_back(seq); };
	inline std::vector<CBrdSeq> const &Seq() const { return mSeq; };

	inline void Pickup(CBrdPPC v) { mPickup.push_back(v); };
	inline std::vector<CBrdPPC> const &Pickup() const { return mPickup; };

	inline void Place(CBrdPPC v) { mPlace.push_back(v); };
	inline std::vector<CBrdPPC> const &Place() const { return mPlace; };

	inline void Chuck(CBrdPPC v) { mChuck.push_back(v); };
	inline std::vector<CBrdPPC> const &Chuck() const { return mChuck; };

	inline void Extent(CBrdExtent v) { mExtent.push_back(v); };
	inline std::vector<CBrdExtent> const &Extent() const { return mExtent; };

	inline void RepeatPickup(CBrdRepeatPlace v) { mRepeatPickup.push_back(v); };
	inline std::vector<CBrdRepeatPlace> const &RepeatPickup() const { return mRepeatPickup; };

	inline void RepeatPlace(CBrdRepeatPlace v) { mRepeatPlace.push_back(v); };
	inline std::vector<CBrdRepeatPlace> const &RepeatPlace() const { return mRepeatPlace; };
	
	std::string Dump() const
	{
		std::ostringstream oss;

		oss << "header - " << mInfo.Dump() << std::endl;
		for(uint i=0,q=mSeq.size(); i<q; i++) { oss << "sequence " << i+1 << " - " << mSeq[i].Dump() << std::endl; }
		for(uint i=0,q=mPickup.size(); i<q; i++) { oss << "pickup " << i+1 << " - " << mPickup[i].Dump() << std::endl; }
		for(uint i=0,q=mPlace.size(); i<q; i++) { oss << "place " << i+1 << " - " << mPlace[i].Dump() << std::endl; }
		for(uint i=0,q=mChuck.size(); i<q; i++) { oss << "chuck " << i+1 << " - " << mChuck[i].Dump() << std::endl; }
		for(uint i=0,q=mRepeatPickup.size(); i<q; i++) { oss << "repeat pickup " << i+1 << " - " << mRepeatPickup[i].Dump() << std::endl; }
		for(uint i=0,q=mRepeatPlace.size(); i<q; i++) { oss << "repeat place " << i+1 << " - " << mRepeatPlace[i].Dump() << std::endl; }
		for(uint i=0,q=mExtent.size(); i<q; i++) { oss << "extent " << i+1 << " - " << mExtent[i].Dump() << std::endl; }

		return oss.str();
	}

protected:
	CBrdInfo mInfo;
	std::vector<CBrdSeq> mSeq;
	std::vector<CBrdPPC> mPickup;
	std::vector<CBrdPPC> mPlace;
	std::vector<CBrdPPC> mChuck;
	std::vector<CBrdRepeatPlace> mRepeatPickup;
	std::vector<CBrdRepeatPlace> mRepeatPlace;
	std::vector<CBrdExtent> mExtent;
};


void test2(char const *fname)
{
	std::ifstream ifstr(fname, std::ifstream::in | std::ifstream::binary);

	if(ifstr.is_open())
	{
		CBoard board;

		ifstr >> board;

		std::cout << board.Dump();

		ifstr.close();
	}
}


// ---------------------------------------------

/*
enum
{
	SM_HDR,
	SM_SEQUENCE,
	SM_PICKUP,
	SM_PLACE,
	SM_CHUCK,
	SM_REPEAT_PICKUP,
	SM_REPEAT_PLACE,
	SM_EXTENT,
	SM_FINAL,
	SM_ERROR,
	SM_EOF,
	SM_END
};

void test1(char const *fname)
{
	FILE *fin = fopen(fname, "r");

	if(fin != NULL)
	{
		int stateMach = SM_HDR;
		char *pSmError = NULL;
		CBoard board;

		while(stateMach != SM_END)
		{
			if(feof(fin))
				stateMach = SM_EOF;
			switch(stateMach)
			{
				case SM_HDR: // header
					{
						brd_hdr_t hdr;

						if(fread(&hdr, sizeof(hdr), 1, fin) == 1)
						{
							CBrdInfo info(hdr);

							board.Info(info);
							std::cout << "header - " << info.Dump() << std::endl;
							stateMach = SM_SEQUENCE;
						}
						else stateMach = SM_ERROR;
					}
					break;

				case SM_SEQUENCE:
					if(board.Info().mCountSequence)
					{
						brd_fmt_seq_t brdSeq;

						if(fread(&brdSeq, sizeof(brdSeq), 1, fin) == 1)
						{
							CBrdSeq seq(brdSeq);
							board.Seq(seq);
							std::cout << "sequence " << board.Seq().size() << " - " << seq.Dump() << std::endl;
							if(board.Seq().size() >= board.Info().mCountSequence)
								stateMach = SM_PICKUP;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_PICKUP;
					break;

				case SM_PICKUP:
					if(board.Info().mCountPick)
					{
						brd_fmt_pickplacechuck_t brdPpc;

						if(fread(&brdPpc, sizeof(brdPpc), 1, fin) == 1)
						{
							CBrdPPC ppc(brdPpc);
							board.Pickup(ppc);
							std::cout << "pickup " << board.Pickup().size() << " - " << ppc.Dump() << std::endl;
							if(board.Pickup().size() >= board.Info().mCountPick)
								stateMach = SM_PLACE;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_PLACE;
					break;

				case SM_PLACE:
					if(board.Info().mCountPlace)
					{
						brd_fmt_pickplacechuck_t brdPpc;

						if(fread(&brdPpc, sizeof(brdPpc), 1, fin) == 1)
						{
							CBrdPPC ppc(brdPpc);
							board.Place(ppc);
							std::cout << "place " << board.Place().size() << " - " << ppc.Dump() << std::endl;
							if(board.Place().size() >= board.Info().mCountPlace)
								stateMach = SM_CHUCK;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_CHUCK;
					break;

				case SM_CHUCK:
					if(board.Info().mCountChuck)
					{
						brd_fmt_pickplacechuck_t brdPpc;

						if(fread(&brdPpc, sizeof(brdPpc), 1, fin) == 1)
						{
							CBrdPPC ppc(brdPpc);
							board.Chuck(ppc);
							std::cout << "chuck " << board.Chuck().size() << " - " << ppc.Dump() << std::endl;
							if(board.Chuck().size() >= board.Info().mCountChuck)
								stateMach = SM_REPEAT_PICKUP;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_REPEAT_PICKUP;
					break;

				case SM_REPEAT_PICKUP:
					if(board.Info().mCountRepeatPick)
					{
						brd_place_repeat_t buf;

						if(fread(&buf, sizeof(buf), 1, fin) == 1)
						{
							CBrdRepeatPlace detail(buf);
							board.RepeatPickup(detail);
							std::cout << "repeat pickup " << board.RepeatPickup().size() << " - " << detail.Dump() << std::endl;
							if(board.RepeatPickup().size() >= board.Info().mCountRepeatPick)
								stateMach = SM_REPEAT_PLACE;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_REPEAT_PLACE;
					break;

				case SM_REPEAT_PLACE:
					if(board.Info().mCountRepeatPlace)
					{
						brd_place_repeat_t buf;

						if(fread(&buf, sizeof(buf), 1, fin) == 1)
						{
							CBrdRepeatPlace detail(buf);
							board.RepeatPlace(detail);
							std::cout << "repeat place " << board.RepeatPlace().size() << " - " << detail.Dump() << std::endl;
							if(board.RepeatPlace().size() >= board.Info().mCountRepeatPlace)
								stateMach = SM_EXTENT;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_EXTENT;
					break;

				case SM_EXTENT:
					if(board.Info().mCountExtent)
					{
						brd_fmt_extent_t brdExtent;

						if(fread(&brdExtent, sizeof(brdExtent), 1, fin) == 1)
						{
							CBrdExtent extent(brdExtent);

							board.Extent(extent);
							std::cout << "extent " << board.Extent().size() << " - " << extent.Dump() << std::endl;
							if(board.Extent().size() >= board.Info().mCountExtent)
								stateMach = SM_FINAL;
						}
						else stateMach = SM_ERROR;
					}
					else stateMach = SM_FINAL;
					break;

				case SM_ERROR:
					std::cout << "error" << std::endl;
					stateMach = SM_END;
					break;

				case SM_FINAL:
					std::cout << "final" << std::endl;
					stateMach = SM_END;
					break;

				case SM_EOF:
					std::cout << "unexpected EOF" << std::endl;
					stateMach = SM_END;
					break;
			}
		}
		fclose(fin);

		if(stateMach != SM_END)
			std::cout << "pre-mature exit - steate: " << stateMach << std::endl;
	}
}
*/

int main(int argc, char **argv)
{
	if(argc > 1)
		test2(argv[1]);

	return 0;
}
