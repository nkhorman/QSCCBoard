#ifndef _STDSTRINGSPLIT_H_

#include <string>
#include <functional>

void split(std::string str, std::string split, std::function<void(std::string const &)>fn);
std::string stringTrim(std::string str, std::string commentChars);
void addWhiteSpcs(std::string &str, size_t maxLength);

#endif