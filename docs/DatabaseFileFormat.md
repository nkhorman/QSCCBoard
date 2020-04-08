BRDATAx.DAT File Format
---

Copyright by Steve Clynes, 2019
Portions Copyright by Neal Horman, 2020

The following information was acumulated by means of reverse engineering, and is incomplete, and may be erronious.

Header format
--

	00	Sequence count lower byte
	01	Pick count
	02	Place count
	03	Chuck count
	04	Repeat Pick count
	05	Repeat Place count
	06	Extent count
	07	Filesize low byte
	08	Filesize high byte, add 0xa8
	09	00	?
	0A	00	?
	0B	00	? - seems like a byte value
	0C	00	? - seems like a byte value
	0D	00	?
	0E	00	?
	0F	00	Bit 0 = MSB of sequence count

Format records follow starting at offet 0x10
---

	0x10	Sequence start
	n	Pickup start		= Sequence start		+ (sequence count * 4)
	n	Place start			= Pickup start			+ (pickup count * 10)
	n	Chuck start			= Place start			+ (place count * 10)
	n	RepeatPickup start	= Chuck start			+ (chuck count * 10)
	n	RepeatPlace start	= RepeatPickup start	+ (repeat pickup count * 10)
	n	Extent start		= Chuck start			+ (repeat place count * 10)


Sequence format
--
	Offset 0	Command
		: 0 = Stop
		: 1 = Pickup		param1 = pickup index, param3 = inpection type - 223 = lead inspection
		: 2 = Place			param1 = place index, param2 - 10 = dispense only, 11 = dispense and place, 255 = place only
		: 3 = Chuck			param1 = nozzle index
		: 4 = Test			param1 = test index
		: 5 = Repeat		param1 = repeat index, param2 - 9 = test image, otherwise 255
		: 6 = Goto			param1 = sequence step index
		: 7 = Wait Address	param1 = address, param2 = port, param3 = operation type - 0 = off, 1 = on, 2 = toggle, 3 = pulse
		: 8 = Output Address - see Wait Address operation code 7 for param values
		: 9 = Home			param1 = axis number - 1 = X, 2 = Y, 3 = Z, 4 = T, 0 = All (X, Y, Z, and T)
		: 10 = If Address	- see Wait Address operation code 7 for param values
		: 11 = Run			Param1 = run index
		: 15 = Mod Code		Param1 = code number index, Param2 = New value for mod code
		: 16 = Transport	param1 = 0-> hold, 1-> pass
	Offset 1	Param1 - 255 if not used
	Offset 2	Param2 - 255 if not used
	Offset 3	Param3 - 255 if not used

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

LAE extent : describes component dimensions and laser align point & method
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

Pickup Repeat format : Used for waffle tray definitions
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


Place Repeat format
--
	Offset 0 - imageColSpanHi
	Offset 1 - imageColSpanLo
	Offset 2 - 0 - ? seems like a byte value
	Offset 3 - 0 - ? seems like a byte value
	Offset 4 - imageRowSpanHi
	Offset 5 - imageRowSpanLo
	Offset 6 - 0 - ? seems like a byte value
	Offset 7 - 0 - ? seems like a byte value
	Offset 8 - col
	Offset 9 - rows