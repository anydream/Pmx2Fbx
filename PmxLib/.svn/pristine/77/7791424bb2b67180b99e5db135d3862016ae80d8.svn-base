
#ifndef __BinReader_h__
#define __BinReader_h__

#include <stdint.h>
#include <string>
#include <vector>

class BinReader
{
public:
	typedef std::vector<uint8_t> Data;

public:
	BinReader(const void * pBuffer, size_t len)
		: m_pBuffer(pBuffer)
		, m_Length(len)
		, m_Offset(0)
	{
	}

	template <typename T>
	T Read()
	{
		if (m_Offset + sizeof(T) <= m_Length)
		{
			m_Offset += sizeof(T);
			return *(T*)((char*)m_pBuffer + m_Offset - sizeof(T));
		}
		return 0;
	}

	void ReadString(size_t len, std::string & str)
	{
		if (m_Offset + len <= m_Length)
		{
			str.assign((char*)m_pBuffer + m_Offset, len);
			m_Offset += len;
		}
	}

	void ReadString(std::string & str)
	{
		char ch;
		while (0 != (ch = Read<char>()))
		{
			str.push_back(ch);
		}
	}

	void ReadData(size_t len, Data & dat)
	{
		if (m_Offset + len <= m_Length)
		{
			char * pFirst = (char*)m_pBuffer + m_Offset;
			dat.assign(pFirst, pFirst + len);
			m_Offset += len;
		}
	}

	void Skip(size_t len)
	{
		m_Offset += len;
		if (m_Offset > m_Length)
			m_Offset = m_Length;
	}

	void Rewind()
	{
		m_Offset = 0;
	}

private:
	const void	*m_pBuffer;
	size_t		m_Length;
	size_t		m_Offset;
};

#endif