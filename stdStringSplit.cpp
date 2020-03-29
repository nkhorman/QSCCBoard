
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
