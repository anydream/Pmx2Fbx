
#include <windows.h>
#include "Utils.h"

std::wstring & mbs2wcs(unsigned int codePage, const std::string & strMB, std::wstring & tmpWstr)
{
	int len = MultiByteToWideChar(codePage, 0, strMB.c_str(), strMB.size(), 0, 0);
	tmpWstr.resize(len);
	MultiByteToWideChar(codePage, 0, strMB.c_str(), strMB.size(), &tmpWstr[0], len);
	return tmpWstr;
}

std::string & wcs2mbs(unsigned int codePage, const std::wstring & strWCS, std::string & tmpStr)
{
	int len = WideCharToMultiByte(codePage, 0, strWCS.c_str(), strWCS.size(), 0, 0, 0, 0);
	tmpStr.resize(len);
	WideCharToMultiByte(codePage, 0, strWCS.c_str(), strWCS.size(), &tmpStr[0], len, 0, 0);
	return tmpStr;
}

std::wstring & Platform_Utf8To16(const std::string & strMB, std::wstring & tmpWstr)
{
	return mbs2wcs(CP_UTF8, strMB, tmpWstr);
}

std::string & Platform_Utf16To8(const std::wstring & strWCS, std::string & tmpStr)
{
	return wcs2mbs(CP_UTF8, strWCS, tmpStr);
}

std::string & Platform_SJIS2ACP_UTF8(const std::string & strUTF8, std::string & tmpStrAcpUTF8)
{
	std::wstring tmpWstr;
	std::string tmpStr;
	mbs2wcs(CP_UTF8, strUTF8, tmpWstr);
	wcs2mbs(932, tmpWstr, tmpStr);
	mbs2wcs(CP_ACP, tmpStr, tmpWstr);
	wcs2mbs(CP_UTF8, tmpWstr, tmpStrAcpUTF8);
	return tmpStrAcpUTF8;
}