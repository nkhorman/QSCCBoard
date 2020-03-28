#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <fstream>
// #include <ostream>
// #include <istream>
#include <string>

#include "CBoard.h"

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

void test3(char const *fnameIn, char const *fnameOut)
{
	std::ifstream ifs(fnameIn, std::ifstream::in | std::ifstream::binary);

	if(ifs.is_open())
	{
		CBoard board;

		ifs >> board;

		//std::cout << board.Dump();

		ifs.close();

		std::ofstream ofs(fnameOut, std::ofstream::out | std::ofstream::binary);
		if(ofs.is_open())
		{
			ofs << board;
			ofs.flush();
			ofs.clear();
		}
		
	}
}

int main(int argc, char **argv)
{
	if(argc == 2)
		test2(argv[1]);
	else if(argc == 3)
		test3(argv[1], argv[2]);

	return 0;
}
