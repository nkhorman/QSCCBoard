
#include <iostream>
#include <sstream>
#include <string>

#include "CBoard.h"

// for_each thoughts - https://www.fluentcpp.com/2018/03/30/is-stdfor_each-obsolete/

std::istream &operator>>(std::istream &stream, CBrdInfo &hdr)
{
	brd_hdr_t brdHdr;

	stream.read((char *)&brdHdr, sizeof(brdHdr));
	hdr.insert(brdHdr);

	return stream;
};

std::ostream &operator<<(std::ostream &stream, const CBrdInfo &hdr)
{
	brd_hdr_t brdHdr = hdr.extract();

	stream.write((char const *)&brdHdr, sizeof(brdHdr));

	return stream;
};

void CBrdInfo::insert(brd_hdr_t hdr)
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
	mFileSize = (hdr.size_file_lo | ((hdr.size_file_hi - 0xa8) << 8));
	// preserve these across read/write operations
	mRes1[0] = hdr.res1[0];
	mRes1[1] = hdr.res1[1];
	mRes1[2] = hdr.res1[2];
	mRes1[3] = hdr.res1[3];
	mRes1[4] = hdr.res1[4];
	mRes1[5] = hdr.res1[5];
};

brd_hdr_t CBrdInfo::extract() const
{
	brd_hdr_t hdr = {0};

	hdr.seq_lo = mCountSequence & 0xff;
	hdr.seq_hi = (mCountSequence & 0x0100) >> 8;
	hdr.count_pick = mCountPick & 0xff;
	hdr.count_place = mCountPlace & 0xff;
	hdr.count_chuck = mCountChuck & 0xff;
	hdr.count_repeat_pickup = mCountRepeatPick & 0xff;
	hdr.count_repeat_place = mCountRepeatPlace & 0xff;
	hdr.count_extent = mCountExtent & 0xff;
	hdr.size_file_lo = mFileSize & 0xff;
	hdr.size_file_hi = ((mFileSize & 0xff00) >> 8) + 0xa8;
	// preserve these across read/write operations
	hdr.res1[0] = mRes1[0];
	hdr.res1[1] = mRes1[1];
	hdr.res1[2] = mRes1[2];
	hdr.res1[3] = mRes1[3];
	hdr.res1[4] = mRes1[4];
	hdr.res1[5] = mRes1[5];

	return hdr;
}

std::string CBrdInfo::Dump() const
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


// **
std::istream &operator>>(std::istream &stream, CBrdSeq &seq)
{
	brd_fmt_seq_t brdSeq;

	stream.read((char *)&brdSeq, sizeof(brdSeq));
	seq.insert(brdSeq);

	return stream;
};

std::ostream &operator<<(std::ostream &stream, const CBrdSeq &seq)
{
	brd_fmt_seq_t brdSeq = seq.extract();

	stream.write((char const *)&brdSeq, sizeof(brdSeq));

	return stream;
};

void CBrdSeq::insert(brd_fmt_seq_t v)
{
		mCmd = v.cmd;
		mParam.clear();
		mParam.push_back(v.param[0]);
		mParam.push_back(v.param[1]);
		mParam.push_back(v.param[2]);
};

brd_fmt_seq_t CBrdSeq::extract() const
{
	brd_fmt_seq_t seq = {0};

	seq.cmd = mCmd & 0xff;
	seq.param[0] = mParam[0] & 0xff;
	seq.param[1] = mParam[1] & 0xff;
	seq.param[2] = mParam[2] & 0xff;

	return seq;
}

std::string CBrdSeq::Dump() const
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

// **
std::string CBrdLoc::Dump() const
{
	std::ostringstream oss;

	oss << "X: " << mX << ", Y: " << mY << ", Z: " << mZ << ", T: " << mT;

	return oss.str();
}

// **
std::istream &operator>>(std::istream &stream, CBrdPPC &ppc)
{
	brd_fmt_pickplacechuck_t brdPpc;

	stream.read((char *)&brdPpc, sizeof(brdPpc));
	ppc.insert(brdPpc);

	return stream;
};

std::ostream &operator<<(std::ostream &stream, const CBrdPPC &ppc)
{
	brd_fmt_pickplacechuck_t brdPpc = ppc.extract();

	stream.write((char const *)&brdPpc, sizeof(brdPpc));

	return stream;
};

void CBrdPPC::insert(brd_fmt_pickplacechuck_t v)
{
	mAltIndex = v.altIndex;
	mExtent = v.extent;
	mLoc = CBrdLoc(v.loc.x, v.loc.y, v.loc.z, v.loc.t);
};

brd_fmt_pickplacechuck_t CBrdPPC::extract() const
{
	brd_fmt_pickplacechuck_t brdPpc = {0};

	brdPpc.altIndex = mAltIndex;
	brdPpc.extent = mExtent;
	brdPpc.loc.x = mLoc.x() & 0xffff;
	brdPpc.loc.y = mLoc.y() & 0xffff;
	brdPpc.loc.z = mLoc.z() & 0xffff;
	brdPpc.loc.t = mLoc.t() & 0xffff;

	return brdPpc;
}

std::string CBrdPPC::Dump() const
{
	std::ostringstream oss;

	if(mAltIndex)
		oss << "AltIndex: " << mAltIndex << ", ";
	if(mExtent)
		oss << "Extent: " << mExtent << ", ";
	oss << mLoc.Dump();

	return oss.str();
}

// **
std::string CBrdMeas::Dump() const
{
	std::ostringstream oss;

	oss << mVal;
	if(mTol)
		oss << ", Tol: " << mTol;

	return oss.str();
}

// **
std::string CBrdExtentLAE::Dump() const
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

// **
std::string CBrdExtentRepeatPickup::Dump() const
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

// **
std::istream &operator>>(std::istream &stream, CBrdExtent &extent)
{
	brd_fmt_extent_t brdExtent;

	stream.read((char *)&brdExtent, sizeof(brdExtent));
	extent.insert(brdExtent);

	return stream;
};

std::ostream &operator<<(std::ostream &stream, const CBrdExtent &extent)
{
	brd_fmt_extent_t brdExtent = extent.extract();

	stream.write((char const *)&brdExtent, sizeof(brdExtent));

	return stream;
};

void CBrdExtent::insert(brd_fmt_extent_t v)
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

brd_fmt_extent_t CBrdExtent::extract() const
{
	brd_fmt_extent_t brdExtent = {0};

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

std::string CBrdExtent::Dump() const
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

// **
std::istream &operator>>(std::istream &stream, CBrdRepeatPlace &repl)
{
	brd_place_repeat_t brdPlRe;

	stream.read((char *)&brdPlRe, sizeof(brdPlRe));
	repl.insert(brdPlRe);

	return stream;
};

std::ostream &operator<<(std::ostream &stream, const CBrdRepeatPlace &repl)
{
	brd_place_repeat_t brdPlRe = repl.extract();

	stream.write((char const *)&brdPlRe, sizeof(brdPlRe));

	return stream;
};

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

void CBrdRepeatPlace::insert(brd_place_repeat_t v)
{
	// uint8_t *pbuf = (uint8_t *)&v;
	// std::cout
	// 	<< "repeat detail in - "
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
	mRes1 = v.res1;
	mImageSpanCol = v.imageColSpan;
	mRes2 = v.res2;
	mRow = v.row;
	mCol = v.col;
};

brd_place_repeat_t CBrdRepeatPlace::extract() const
{
	brd_place_repeat_t brdPlRe = {0};

	brdPlRe.imageRowSpan = mImageSpanRow;
	brdPlRe.res1 = mRes1;
	brdPlRe.imageColSpan = mImageSpanCol;
	brdPlRe.res2 = mRes2;
	brdPlRe.row = mRow;
	brdPlRe.col = mCol;

	// uint8_t *pbuf = (uint8_t *)&brdPlRe;
	// std::cout
	// 	<< "repeat detail out - "
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

	return brdPlRe;
}

std::string CBrdRepeatPlace::Dump() const
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

// **
std::istream &operator>>(std::istream &stream, CBoard &brd)
{
	CBrdInfo info;

	stream >> info;
	brd.Info(info);
	for(uint i=0,q=info.mCountSequence; i<q; i++) { CBrdSeq seq; stream >> seq; brd.Seq(seq); }
	for(uint i=0,q=info.mCountPick; i<q; i++) { CBrdPPC ppc; stream >> ppc; brd.Pickup(ppc); }
	for(uint i=0,q=info.mCountPlace; i<q; i++) { CBrdPPC ppc; stream >> ppc; brd.Place(ppc); }
	for(uint i=0,q=info.mCountChuck; i<q; i++) { CBrdPPC ppc; stream >> ppc; brd.Chuck(ppc); }
	for(uint i=0,q=info.mCountRepeatPick; i<q; i++) { CBrdRepeatPlace brp; stream >> brp; brd.RepeatPickup(brp); }
	for(uint i=0,q=info.mCountRepeatPlace; i<q; i++) { CBrdRepeatPlace brp; stream >> brp; brd.RepeatPlace(brp); }
	for(uint i=0,q=info.mCountExtent; i<q; i++) { CBrdExtent lae; stream >> lae; brd.Extent(lae); }

	return stream;
};

std::ostream &operator<<(std::ostream &stream, CBoard &brd)
{
	brd.Update();
	stream << brd.Info();
	std::for_each(brd.Seq().begin(), brd.Seq().end(), [&](CBrdSeq const &item) mutable { stream << item; });
	std::for_each(brd.Pickup().begin(), brd.Pickup().end(), [&](CBrdPPC const &item) mutable { stream << item; });
	std::for_each(brd.Place().begin(), brd.Place().end(), [&](CBrdPPC const &item) mutable { stream << item; });
	std::for_each(brd.Chuck().begin(), brd.Chuck().end(), [&](CBrdPPC const &item) mutable { stream << item; });
	std::for_each(brd.RepeatPickup().begin(), brd.RepeatPickup().end(), [&](CBrdRepeatPlace const &item) mutable { stream << item; });
	std::for_each(brd.RepeatPlace().begin(), brd.RepeatPlace().end(), [&](CBrdRepeatPlace const &item) mutable { stream << item; });
	std::for_each(brd.Extent().begin(), brd.Extent().end(), [&](CBrdExtent const &item) mutable { stream << item; });

	return stream;
};

void CBoard::Update()
{
	mInfo.mCountSequence = mSeq.size();
	mInfo.mCountPick = mPickup.size();
	mInfo.mCountPlace = mPlace.size();
	mInfo.mCountChuck = mChuck.size();
	mInfo.mCountRepeatPick = mRepeatPickup.size();
	mInfo.mCountRepeatPlace = mRepeatPlace.size();
	mInfo.mCountExtent = mExtent.size();

	FilesizeCalculate();
}

void CBoard::FilesizeCalculate()
{
	// uint filesizeOld = mInfo.mFileSize;
	uint filesizeNew =
		mInfo.FileSize()
		+ (mInfo.mCountSequence * CBrdSeq::FileSize())
		+ (mInfo.mCountPick * CBrdPPC::FileSize())
		+ (mInfo.mCountPlace * CBrdPPC::FileSize())
		+ (mInfo.mCountChuck * CBrdPPC::FileSize())
		+ (mInfo.mCountRepeatPick * CBrdRepeatPlace::FileSize())
		+ (mInfo.mCountRepeatPlace * CBrdRepeatPlace::FileSize())
		+ (mInfo.mCountExtent * CBrdExtent::FileSize())
		;
	mInfo.mFileSize = filesizeNew;
}	

std::string CBoard::Dump() const
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
