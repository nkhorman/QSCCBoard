#ifndef _CBOARD_H_

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

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
	uint8_t size_file_lo;
	uint8_t size_file_hi; // add 0xa8
	uint8_t res1[6];
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
// Offset 0 - 183 - imageColSpanHi
// Offset 1 - 7 - imageColSpanLo
// Offset 2 - 0 - ? seems like a byte value
// Offset 3 - 0 - ? seems like a byte value
// Offset 4 - 0 - imageRowSpanHi
// Offset 5 - 0 - imageRowSpanLo
// Offset 6 - 0 - ? seems like a byte value
// Offset 7 - 0 - ? seems like a byte value
// Offset 8 - 3 - col
// Offset 9 - 1 - rows

typedef struct __attribute__((packed)) _brd_place_repeat_t
{
	uint16_t imageColSpan;
	uint16_t res1; // stored as 16 bit value for convenience
	uint16_t imageRowSpan;
	uint16_t res2; // stored as 16 bit value for convenience
	uint8_t col;
	uint8_t row;
}brd_place_repeat_t;

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
#include <fstream>
#include <vector>

#include "CBrdLoc.h"

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

	static uint FileSize() { return sizeof(brd_hdr_t); };

	friend std::istream &operator>>(std::istream &stream, CBrdInfo &hdr);
	friend std::ostream &operator<<(std::ostream &stream, const CBrdInfo &hdr);

	void insert(brd_hdr_t hdr);

	brd_hdr_t extract() const;

	uint mCountSequence;
	uint mCountPick;
	uint mCountPlace;
	uint mCountChuck;
	uint mCountRepeatPick;
	uint mCountRepeatPlace;
	uint mCountExtent;
	uint mFileSize;
	uint8_t mRes1[6];

	std::string Dump() const;
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

	static uint FileSize() { return sizeof(brd_fmt_seq_t); };

	friend std::istream &operator>>(std::istream &stream, CBrdSeq &seq);
	friend std::ostream &operator<<(std::ostream &stream, const CBrdSeq &seq);

	void insert(brd_fmt_seq_t v);

	brd_fmt_seq_t extract() const;

	inline uint Cmd() const { return mCmd; };
	inline void Cmd(uint v) { mCmd = v; };

	inline std::vector<uint> Param() const { return mParam; };
	inline void Param(std::vector<uint> v) { mParam = v; };
	
	std::string Dump() const;

	std::string Parse(std::string str);

protected:
	uint mCmd;
	std::vector<uint> mParam;
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

	static uint FileSize() { return sizeof(brd_fmt_pickplacechuck_t); };

	friend std::istream &operator>>(std::istream &stream, CBrdPPC &ppc);
	friend std::ostream &operator<<(std::ostream &stream, const CBrdPPC &ppc);

	void insert(brd_fmt_pickplacechuck_t v);

	brd_fmt_pickplacechuck_t extract() const;

	inline uint AltIndex() const { return mAltIndex; };
	inline uint Extent() const { return mExtent; };
	inline CBrdLoc const &Loc() const { return mLoc; };

	std::string Dump() const;
	std::string Parse(std::string str);

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

	std::string Dump() const;
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
	
    // these are left here so that maintenance is obvious
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
		brd_fmt_extent_lae_t lae = {0};

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

	std::string Dump() const;
	
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
		, mRes1(0)
		{};
	CBrdExtentRepeatPickup(brd_fmt_extent_pickup_repeat_t v)
		: mNext(v.extent)
		, mColumnCount(v.columnStepHiCount >> 3)
		, mColumnStep(v.columnStepLo | ((v.columnStepHiCount & 0x07) << 8))
		, mRowCount(v.rowStepHiCount >> 3)
		, mRowStep(v.rowStepLo | ((v.rowStepHiCount & 0x07) << 8))
		, mColumns(v.columns)
		, mRows(v.rows)
		, mRes1(v.res1)
		{};
	
    // these are left here so that maintenance is obvious
    void insert(brd_fmt_extent_pickup_repeat_t v)
	{
		mNext = v.extent;
		mColumnCount = v.columnStepHiCount >> 3;
		mColumnStep = v.columnStepLo | ((v.columnStepHiCount & 0x07) << 8);
		mRowCount = v.rowStepHiCount >> 3;
		mRowStep = v.rowStepLo | ((v.rowStepHiCount & 0x07) << 8);
		mColumns = v.columns;
		mRows = v.rows;
		mRes1 = v.res1;
	};

	brd_fmt_extent_pickup_repeat_t extract() const
	{
		brd_fmt_extent_pickup_repeat_t brdPure = {0};

		brdPure.extent = mNext & 0xff;
		brdPure.columnStepHiCount = ((mColumnCount & 0x7f) << 3) | ((mColumnStep & 0x3f00) >> 8);
		brdPure.columnStepLo = mColumnStep & 0xff;
		brdPure.rowStepHiCount = ((mRowCount & 0x7f) << 3) | ((mRowStep & 0x3f00) >> 8);
		brdPure.rowStepLo = mRowStep & 0xff;
		brdPure.columns = mColumns & 0xff;
		brdPure.rows = mRows & 0xff;
		brdPure.res1 = mRes1;

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

	std::string Dump() const;
	
protected:
	uint mNext;
	uint mColumnCount;
	uint mColumnStep;
	uint mRowCount;
	uint mRowStep;
	uint mColumns;
	uint mRows;
	uint mRes1;
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

	static uint FileSize() { return sizeof(brd_fmt_extent_t); };

	friend std::istream &operator>>(std::istream &stream, CBrdExtent &extent);
	friend std::ostream &operator<<(std::ostream &stream, const CBrdExtent &extent);

	void insert(brd_fmt_extent_t v);
	brd_fmt_extent_t extract() const;

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

	std::string Dump() const;
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
		: mImageSpanCol(0)
		, mRes1(0)
		, mImageSpanRow(0)
		, mRes2(0)
		, mCol(0)
		, mRow(0)
		{};
	CBrdRepeatPlace(brd_place_repeat_t brdPlRe)
		{
			insert(brdPlRe);
		};
	virtual ~CBrdRepeatPlace() {};

	static uint FileSize() { return sizeof(brd_place_repeat_t); };

	friend std::istream &operator>>(std::istream &stream, CBrdRepeatPlace &repl);
	friend std::ostream &operator<<(std::ostream &stream, const CBrdRepeatPlace &repl);

	void insert(brd_place_repeat_t v);
	brd_place_repeat_t extract() const;

	std::string Dump() const;

protected:
	uint mImageSpanCol;
	uint mRes1; // stored as 16 bit value for convenience
	uint mImageSpanRow;
	uint mRes2; // stored as 16 bit value for convenience
	uint mCol;
	uint mRow;
};


class CBoard
{
public:
	CBoard() {}
	virtual ~CBoard() {};

	friend std::istream &operator>>(std::istream &stream, CBoard &brd);

	friend std::ostream &operator<<(std::ostream &stream, CBoard &brd);

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

	inline void RepeatPickup(CBrdRepeatPlace v) { mRepeatPickup.push_back(v); };
	inline std::vector<CBrdRepeatPlace> const &RepeatPickup() const { return mRepeatPickup; };

	inline void RepeatPlace(CBrdRepeatPlace v) { mRepeatPlace.push_back(v); };
	inline std::vector<CBrdRepeatPlace> const &RepeatPlace() const { return mRepeatPlace; };

	inline void Extent(CBrdExtent v) { mExtent.push_back(v); };
	inline std::vector<CBrdExtent> const &Extent() const { return mExtent; };

	std::string Dump() const;

	void Update();

protected:
	void FilesizeCalculate();	

	CBrdInfo mInfo;
	std::vector<CBrdSeq> mSeq;
	std::vector<CBrdPPC> mPickup;
	std::vector<CBrdPPC> mPlace;
	std::vector<CBrdPPC> mChuck;
	std::vector<CBrdRepeatPlace> mRepeatPickup;
	std::vector<CBrdRepeatPlace> mRepeatPlace;
	std::vector<CBrdExtent> mExtent;
};

#endif

#endif // _CBOARD_H_