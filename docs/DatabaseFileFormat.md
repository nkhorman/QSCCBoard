BRDATAx.DAT File Format
---

Copyright by Steve Clynes, 2019

Portions Copyright by Neal Horman, 2020
   

The following information was accumulated by means of reverse engineering, and is incomplete, and may be erroneous.

Values are decimal, unless expressly prefixed with 0x for hex values.

Header format
--

	Offset 0	Sequence count lower byte
	Offset 1	Pick count
	Offset 2	Place count
	Offset 3	Chuck count
	Offset 4	Repeat Pick count
	Offset 5	Repeat Place count
	Offset 6	Extent count
	Offset 7	Filesize low byte
	Offset 8	Filesize high byte, add 0xa8
	Offset 9	?
	Offset 10	?
	Offset 11	? - seems like a byte value
	Offset 12	? - seems like a byte value
	Offset 13	?
	Offset 14	?
	Offset 15	0 - Bit 0 = MSB of sequence count

Format records follow starting at offset 16
---

	Offset 16	Sequence start
	Offset n	Pickup start		= Sequence start		+ (sequence count * 4)
	Offset n	Place start			= Pickup start			+ (pickup count * 10)
	Offset n	Chuck start			= Place start			+ (place count * 10)
	Offset n	RepeatPickup start	= Chuck start			+ (chuck count * 10)
	Offset n	RepeatPlace start	= RepeatPickup start	+ (repeat pickup count * 10)
	Offset n	Extent start		= Chuck start			+ (repeat place count * 10)


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
	Offset 0	Extent Type - 6
	Offset 1	Extent B pointer (current extent + 1)
	Offset 2	Board width low byte
	Offset 3	Board width high byte
	Offset 4	Pin/edge registration : 0 = edge, 1 = pin
	Offset 5	BRSTRT
	Offset 6	BMAX
	Offset 7	BA/DECL low byte
	Offset 8	BA/DECL high byte
	Offset 9	0

	Extent B
	Offset 0	Extent type - 7
	Offset 1	00
	Offset 2	Gap
	Offset 3	DIS2PIN
	Offset 4	RSTRT
	Offset 5	RMAX
	Offset 6	RAC/DECL low byte
	Offset 7	RAC/DECL low byte
	Offset 8	0
	Offset 9	0

LAE extent : describes component dimensions and laser align point & method
-
	Offset 0	Extent type - 12
	Offset 1	0
	Offset 2	Width low byte
	Offset 3	[2:0] = Width high byte, [7:3] = width tol
	Offset 4	Length low byte
	Offset 5	[2:0] = Length high byte, [7:3] = length tol.
	Offset 6	Z measure point low byte
	Offset 7	[1:0] = Z measure high byte, [7:2] = Measure type
	Offset 8	[6:0] = pickup delay, [7] = VAC ver.
	Offset 9	0

Pickup Repeat extent : Used for waffle tray definitions
-
	Offset 0	Extent type - 1
	Offset 1	Next extent (possibly point to LAE?)
	Offset 2	Column step HIGH byte [3:0], current count [7:4] ?
	Offset 3	Column step LOW byte. NB high and low not same order as other numbers.
	Offset 4	Row step HIGH byte [3:0], current count [7:4] ?
	Offset 5	Row step LOW byte. NB high and low not same order as other numbers.
	Offset 6	Columns (X)	
	Offset 7	Rows (Y)
	Offset 8	255
	Offset 9	255


Place Repeat format
--
	Offset 0	imageColSpanHi
	Offset 1	imageColSpanLo
	Offset 2	? - seems like a byte value
	Offset 3	? - seems like a byte value
	Offset 4	imageRowSpanHi
	Offset 5	imageRowSpanLo
	Offset 6	? - seems like a byte value
	Offset 7	? - seems like a byte value
	Offset 8	col
	Offset 9	rows