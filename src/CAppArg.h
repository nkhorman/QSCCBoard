/**
 *
 * @author	Initial coding and development - Neal Horman - http://www.wanlink.com
 *
 * @copyright	Copyright (c) 2018 by Neal Horman. All Rights Reserved.
 * @copyright	THIS IS UNPUBLISED PROPIETARY SOURCE CODE OF Neal Horman.
 * @copyright	The copyright notice above does not evidence any actual or
 * @copyright	intended publication of the information in this file or
 * @copyright	associated files.
 *
 * @license	Use of this code, in any form, requires explictly granted terms in
 * @license	writting from the author and copyright holder, Neal Horman.
 * @license	Any use not expressly provided for in the granted terms, is expressly
 * @license	declined, and prohibited.
 * @license	In addition to any granted terms, this license statement, the
 * @license	copyright notice, and the author attribution above, may not be
 * @license	altered or removed.
 *
 * @file
 * @brief	CAppArg - program command line argument proccessor
 *
 * $Date$
 * $Revision$
 */

#ifndef _CCAPPARG_H_
#define _CCAPPARG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <map>
#include <sstream>

class CAppArg
{
public:
	CAppArg() {};
	CAppArg(int argc, char **argv) { Parse(argc, argv); };
	virtual ~CAppArg(){};

	// all keys are forced to lower case
	void Parse(int argc, char **argv)
	{
		std::string key;
		std::string val;

		mArgs.clear();
		for(int i=1; i<argc; i++)
		{
			char *arg = argv[i];

			// single char key/val args. ie '-a123' or '-a 123'
			if(arg[0] == '-' && arg[1] != '-' && arg[1])
			{
				if(key.length())
					mArgs.push_back(std::pair<std::string, std::string>(key, val));

				key = std::string(&arg[1]);
				std::transform(key.begin(), key.end(), key.begin(), [](const unsigned char i){ return tolower(i); });
				// '-a123' ?
				if(arg[2])
				{
					key.erase(1, std::string::npos);
					val = std::string(&arg[2]);
					mArgs.push_back(std::pair<std::string, std::string>(key,val));
					key.erase();
				}
				val.erase();
			}
			// long key/val args. ie '--abc 123' or '--abc=123'
			else if(arg[0] == '-' && arg[1] == '-' && arg[2])
			{
				if(key.length())
					mArgs.push_back(std::pair<std::string, std::string>(key, val));
				key = (char *)(&arg[2]);
				std::transform(key.begin(), key.end(), key.begin(), [](const unsigned char i){ return tolower(i); });
				val.erase();

				// key=value arg pairs
				size_t sepPos = key.find('=');
				if(sepPos != std::string::npos)
				{
					val = key.substr(sepPos+1);
					key.erase(sepPos, std::string::npos);
					mArgs.push_back(std::pair<std::string, std::string>(key, val));
					val.erase();
					key.erase();
				}
			}
			// collect multi-word vals
			else if(key.length())
			{
				if(val.length())
					val += " ";
				val += arg;
			}
			// un-keyed words
			else
				mArgs.push_back(std::pair<std::string, std::string>(std::string(""), std::string(arg)));
		}

		if(key.length())
			mArgs.push_back(std::pair<std::string, std::string>(key, val));
	}

	std::string Dump() const
	{
		std::ostringstream oss;

		std::for_each(mArgs.begin(), mArgs.end(), [&](std::pair<std::string const &, std::string const &> pair)
		{
			oss << "key: '" << pair.first << "' val: '" << pair.second << "'" << std::endl;
		});

		return oss.str();
	}

	std::string Find(std::string const &needle) const
	{
		std::string val;
		std::string str(needle);

		// force the needle to lower case
		std::transform(str.begin(), str.end(), str.begin(), [](const unsigned char i){ return tolower(i); });
		std::vector<std::pair<std::string, std::string>>::const_iterator it =
			std::find_if(mArgs.begin(), mArgs.end(), [&](std::pair<std::string const &, std::string const &> pair)
			{
				return pair.first == str;
			});

		if(it != mArgs.end())
			val = it->second;

		return val;
	}

	void forEach(std::function<void(std::string const &, std::string const &)> fn) const
	{
		if(fn)
		{
			std::for_each(mArgs.begin(), mArgs.end(), [&fn](std::pair<std::string, std::string> pair)
			{
				fn(pair.first, pair.second);
			});
		}
	}

	// PITA - find any cli arg that matches a dst key, and if it's length > 0 set the value in the dst map
	// keys are forced to lower for comparison purposes
	void Extract(std::map<std::string, std::string> &dst)
	{
		std::map<std::string, std::string> tmp = dst;
		forEach([&](std::string const &kkey, std::string const &kval)
		{
			std::string keyArg(kkey);
			std::transform(keyArg.begin(), keyArg.end(), keyArg.begin(), [](const unsigned char i){ return tolower(i); });
			std::for_each(tmp.begin(), tmp.end(), [&](std::pair<std::string, std::string> pair)
			{
				std::string keyConfig(pair.first);
				std::transform(keyConfig.begin(), keyConfig.end(), keyConfig.begin(), [](const unsigned char i){ return tolower(i); });

				if(keyArg == keyConfig && kval.length())
					dst[pair.first] = kval;
			});
		});
	}

protected:
	// Store arg key/val pairs, and un-keyed words in order of occurance
	std::vector<std::pair<std::string, std::string>> mArgs;
};

#ifdef _UNITEST_CLIARG_

// ./test this is a test --hello 1 2 3 4 --abc -1 -2 -4b -z --def --akey=someval --another= some shit -q '--test=a b c' -b 5

int main(int argc, char **argv)
{
	CAppArg args(argc, argv);

	std::cout << args.Dump();
	std::cout << "find akey: " << args.Find("akey") << std::endl;

	return 0;
}
#endif
#endif
