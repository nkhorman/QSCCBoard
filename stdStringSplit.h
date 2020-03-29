#ifndef _STDSTRINGSPLIT_H_

#include <string>
#include <functional>

void split(std::string str, std::string split, std::function<void(std::string const &)>fn);


#endif