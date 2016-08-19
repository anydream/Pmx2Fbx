
#ifndef __Utils_h__
#define __Utils_h__

#include <string>

std::wstring & mbs2wcs(unsigned int codePage, const std::string & strMB, std::wstring & tmpWstr);
std::string & wcs2mbs(unsigned int codePage, const std::wstring & strWCS, std::string & tmpStr);
std::wstring & Platform_Utf8To16(const std::string & strMB, std::wstring & tmpWstr);
std::string & Platform_Utf16To8(const std::wstring & strWCS, std::string & tmpStr);
std::string & Platform_SJIS2ACP_UTF8(const std::string & strUTF8, std::string & tmpStrAcpUTF8);

#endif