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
#include <vector>

#include "CBoard.h"
#include "CAppArg.h"


// Split a std::string by the chars in "split", and call "fn" for each of those "strings" 
// The "str" need not be terminated with "split" chars 
void split(std::string str, std::string split, std::function<void(std::string const &)>fn)
{
    for(size_t r=str.find_first_of(split); r != std::string::npos;)
    {
        if(fn)
            fn(str.substr(0, r));
        r = str.find_first_not_of(split, r);
        str.erase(0, r);
        r = str.find_first_of(split);
    }

    if(str.length() && fn)
        fn(str);
}

class CBoardEx : public CBoard
{
public:
    CBoardEx()
        : mbModeJson(false)
        {};
    virtual ~CBoardEx() {};

    inline void ModeJson(bool v = true) { mbModeJson = v; };
    void ImportExport(bool bImport, std::vector<std::string> const &args);

    std::string SectionDump(std::string strSectionName);

protected:
    void Import(std::vector<std::string> const &args);
    void Export(std::vector<std::string> const &args);

    bool mbModeJson;
};

void CBoardEx::Import(std::vector<std::string> const &args)
{

}

void CBoardEx::Export(std::vector<std::string> const &args)
{
	if(args.size() == 2)
	{
		std::ostringstream oss;
		std::string sectionName = args[0];
		std::string fname = args[1];

		std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), [](const unsigned char i){ return tolower(i); });

		if(sectionName == "sequence")
			for(uint i=0,q=mSeq.size(); i<q; i++) { oss << mSeq[i].Dump() << std::endl; }
		else if(sectionName == "pickup")
			for(uint i=0,q=mPickup.size(); i<q; i++) { oss << mPickup[i].Dump() << std::endl; }
		else if(sectionName == "place")
			for(uint i=0,q=mPlace.size(); i<q; i++) { oss << mPlace[i].Dump() << std::endl; }
		else if(sectionName == "chuck")
			for(uint i=0,q=mChuck.size(); i<q; i++) { oss << mChuck[i].Dump() << std::endl; }
		else if(sectionName == "repeat pickup")
			for(uint i=0,q=mRepeatPickup.size(); i<q; i++) { oss << mRepeatPickup[i].Dump() << std::endl; }
		else if(sectionName == "repeat place")
			for(uint i=0,q=mRepeatPlace.size(); i<q; i++) { oss << mRepeatPlace[i].Dump() << std::endl; }
		else if(sectionName == "extent")
			for(uint i=0,q=mExtent.size(); i<q; i++) { oss << mExtent[i].Dump() << std::endl; }
		else
			std::cerr << "Error - Export - Invalid secion name. Try;"
				" sequence, pickup, place, chuck, repeat pickup, repeat place, or extent"
				<< std::endl;

		if(oss.str().size())
		{
			std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::trunc);

			if(ofs.is_open())
			{
				ofs << oss.str();
				ofs.flush();
				ofs.clear();
			}
		}
	}
}

void CBoardEx::ImportExport(bool bImport, std::vector<std::string> const &args)
{
    std::for_each(args.begin(), args.end(), [&](std::string const &arg) mutable
    {
        std::vector<std::string> strOpArgs;
        split(arg, "=", [&](std::string const &str) mutable { strOpArgs.push_back(str); });

        if(strOpArgs.size() == 2)
        {
            if(bImport)
                Import(strOpArgs);
            else
                Export(strOpArgs);
        }
		else std::cerr << "Error - " << (bImport ? "Import" : "Export")
			<< " - Invalid number of arguments. Try [section name]=[file name]" << std::endl;
    });
}

std::string CBoardEx::SectionDump(std::string sectionName)
{
    std::ostringstream oss;
	std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), [](const unsigned char i){ return tolower(i); });

    if(sectionName == "all" || sectionName == "header")
    	oss << "header - " << mInfo.Dump() << std::endl;
    if(sectionName == "all" || sectionName == "sequence")
        for(uint i=0,q=mSeq.size(); i<q; i++) { oss << "sequence " << i+1 << " - " << mSeq[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "pickup")
        for(uint i=0,q=mPickup.size(); i<q; i++) { oss << "pickup " << i+1 << " - " << mPickup[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "place")
        for(uint i=0,q=mPlace.size(); i<q; i++) { oss << "place " << i+1 << " - " << mPlace[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "chuck")
        for(uint i=0,q=mChuck.size(); i<q; i++) { oss << "chuck " << i+1 << " - " << mChuck[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "repeat pickup")
        for(uint i=0,q=mRepeatPickup.size(); i<q; i++) { oss << "repeat pickup " << i+1 << " - " << mRepeatPickup[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "repeat place")
        for(uint i=0,q=mRepeatPlace.size(); i<q; i++) { oss << "repeat place " << i+1 << " - " << mRepeatPlace[i].Dump() << std::endl; }
    if(sectionName == "all" || sectionName == "extent")
        for(uint i=0,q=mExtent.size(); i<q; i++) { oss << "extent " << i+1 << " - " << mExtent[i].Dump() << std::endl; }

    return oss.str();
}

int main(int argc, char **argv)
{
    CAppArg args(argc, argv);

    //std::cout << args.Dump() << std::endl;

    std::string strCmd;
    std::vector<std::string> strCmdArgs;
    CBoardEx board;
    args.forEach([&](std::string const &key, std::string const &val) mutable
    {
		std::string action = key;
		std::transform(action.begin(), action.end(), action.begin(), [](const unsigned char i){ return tolower(i); });
        // std::cout << "key: " << key << ", val: " << val << std::endl;
        if(action == "read")
        {
            std::ifstream ifstr(val.c_str(), std::ifstream::in | std::ifstream::binary);

            if(ifstr.is_open())
            {
                ifstr >> board;
                ifstr.close();
            }
        }
        else if(action == "write")
        {
            std::ofstream ofs(val.c_str(), std::ofstream::out | std::ofstream::binary);

            if(ofs.is_open())
            {
                ofs << board;
                ofs.flush();
                ofs.clear();
            }
        }
        else if(action == "export" || action == "import")
        {
            strCmd = key;
            split(val, "     ", [&](std::string const &valarg) mutable { strCmdArgs.push_back(valarg); });

            board.ImportExport(action == "import", strCmdArgs);
        }
        else if(action == "dump")
        {
            std::cout << board.SectionDump(val);
        }
        else if(action == "json")
            board.ModeJson();
    });

    return 0;
}