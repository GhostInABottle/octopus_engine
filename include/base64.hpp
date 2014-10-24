#ifndef HPP_BASE64
#define HPP_BASE64

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif
