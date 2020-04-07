BRDATAx.DAT File Format
---

Header format
--

	00	Sequence count lower byte
	01	Pick count
	02	Place count
	03	Chuck count
	04	00	?
	05	00	?
	06	Extent count
	07	Filesize low byte
	08	Filesize high byte, add 0xa8
	09	00	?
	0A	00	?
	0B	00	?
	0C	00	?
	0D	00	?
	0E	00	?
	0F	00	Bit 0 = MSB of sequence count
	10	Sequence start
	pickup start = Sequence start + (sequence count * 4)
	place start  = Pickup start   + (pickup count * 10)
	Chuck start  = Place start    + (place count * 10)
	Extent start = Chuck start    + (chuck count * 10)




Sequence format
--
	Offset 0	Command : 0x01 = pickup		parameter = pickup index
		: 0x02 = place		parameter = place index
		: 0x03 = chuck (nozzle)	parameter = nozzle index
		: 0x10 = transport	parameter = 0-> hold, 1-> pass
	Offset 1	Parameter
	Offset 2	0xff
	Offset 3	0xff

Pickup format
--
	Offset 0	Alternate pickup index
	Offset 1	Extent (used for additional information such as laser align data etc...)
	Offset 2	X low byte
	Offset 3	X high byte
	Offset 4	Y low byte
	Offset 5	Y high byte
	Offset 6	T low byte
	Offset 7	T high byte
	Offset 8	Z low byte
	Offset 9	Z high byte

Place format
--
	Offset 0	Alternate pickup index
	Offset 1	Extent (00)
	Offset 2	X low byte
	Offset 3	X high byte
	Offset 4	Y low byte
	Offset 5	Y high byte
	Offset 6	T low byte
	Offset 7	T high byte
	Offset 8	Z low byte
	Offset 9	Z high byte

Chuck format
--
	Offset 0	(00)
	Offset 1	Extent (00)
	Offset 2	X low byte
	Offset 3	X high byte
	Offset 4	Y low byte
	Offset 5	Y high byte
	Offset 6	T low byte
	Offset 7	T high byte
	Offset 8	Z low byte
	Offset 9	Z high byte

Extent format
--
Transport extent (board information)
-
	Extent A
	Offset 0	Command (06)
	Offset 1	Extent B pointer (current + 1)
	Offset 2	Board width low byte
	Offset 3	Board width high byte
	Offset 4	Pin/edge registration : 0 = edge, 1 = pin
	Offset 5	BRSTRT (E0)
	Offset 6	BMAX (75)
	Offset 7	BA/DECL low byte (64)
	Offset 8	BA/DECL high byte (01)
	Offset 9	00

	Extent B
	Offset 0	Command (07)
	Offset 1	00
	Offset 2	Gap (50)
	Offset 3	DIS2PIN (36)
	Offset 4	RSTRT (E0)
	Offset 5	RMAX (5A)
	Offset 6	RAC/DECL low byte (C8)
	Offset 7	RAC/DECL low byte (01)
	Offset 8	00
	Offset 9	00

LAE : describes component dimensions and laser align point & method
-
	Offset 0	Command (0C)
	Offset 1	00
	Offset 2	Width low byte
	Offset 3	[2:0] = Width high byte, [7:3] = width tol
	Offset 4	Length low byte
	Offset 5	[2:0] = Length high byte, [7:3] = length tol.
	Offset 6	Z measure point low byte
	Offset 7	[1:0] = Z measure high byte, [7:2] = Measure type
	Offset 8	[6:0] = pickup delay, [7] = VAC ver.
	Offset 9	00

Pickup repeat : Used for waffle tray definitions
-
	Offset 0	Command (01)
	Offset 1	Next extent (possibly point to LAE?)
	Offset 2	Column step HIGH byte [3:0], current count [7:4] ?
	Offset 3	Column step LOW byte. NB high and low not same order as other numbers.
	Offset 4	Row step HIGH byte [3:0], current count [7:4] ?
	Offset 5	Row step LOW byte. NB high and low not same order as other numbers.
	Offset 6	Columns (X)	
	Offset 7	Rows (Y)
	Offset 8	FF
	Offset 9	FF
