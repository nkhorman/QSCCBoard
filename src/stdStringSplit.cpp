
#include "stdStringSplit.h"

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

std::string stringTrim(std::string str, std::string commentChars)
{
	std::string newstr(str);

	if(str.length())
	{
		// skip leading white space
		// truncate trailing comments
		size_t pos_l = str.find_first_not_of(" \t\r\n", 0);
		if(pos_l == std::string::npos)
			pos_l = 0;
		size_t pos_r = str.find_first_of(commentChars, pos_l);

		if(pos_l == 0 && pos_r == 0)
			newstr = "";
		else
		{
			if(pos_r == std::string::npos)
				pos_r = str.length();
			// truncate trailing white space
			pos_r = str.find_last_not_of(" \t\r\n", pos_r - 1) + 1;
			newstr = str.substr(pos_l, pos_r - pos_l);
		}
	}

	return newstr;
}

void addWhiteSpcs(std::string &str, size_t maxLength)
{
	if(str.size() > maxLength-1)
		str.erase(maxLength);
    for(size_t i=str.length(); i<maxLength; i++)
	    str += " ";
}
