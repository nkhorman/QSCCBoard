QSCCBoard
---------

This is a utility to generate and or modify the binary "Board" (BRDATAx.DAT) files used by the DOS based Quad Systems Central Controller computer for use in controlling the Quad IIc, IIIc, and IVc Pick and Place machines.

The "CC" program as it is known, also uses a few ASCII based files in addtion to the .DAT file to maintain information about compoments, pickup nozzels (known as chucks), fiducial locations, etc.

Along with modifying the .DAT file, QSCCBoard has been extended to also help create the some of those other support data files.

The format of binary .DAT file, was reverse engineered, initially by Steve Clynes, who graciously provided the information to my self, after which I continued exploring and documenting the .DAT file, and the other ASCII support files.

My goal was to be able to extract as much information as possible from my CAD tools (FreePCB1, and TinyCad) and generate as much configration data for the CC, as possible, without manual input and or translation.

The process of extracting the CAD info into a format for QSCCBoard is not totally automated. Presently, there is some manual formatting using a spread sheet, however, I'm working towards that end.

I hope the Quad Pick and Place community will find this useful.

For licensing terms, please see License.md

Neal Horman