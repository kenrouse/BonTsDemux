// NCachedFile.h: CNCachedFile クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "NFile.h"


#define DEFBUFFSIZE		0x00400000UL	// 4MB


class CNCachedFile : public CNFile  
{
public:
	CNCachedFile();
	virtual ~CNCachedFile();

	const bool Open(LPCTSTR lpszName, const BYTE bFlags, const DWORD dwBuffSize = DEFBUFFSIZE);
	void Close(void);

	const bool Read(BYTE *pBuff, const DWORD dwLen);
	const bool Read(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	const bool Write(const BYTE *pBuff, const DWORD dwLen);
	const bool Write(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	const ULONGLONG GetPos(void) const;
	const bool Seek(const ULONGLONG llPos);

	const bool Flush(void);

protected:
	BOOL m_bIsWritable;

	BYTE *m_pBuff;
	ULONGLONG m_llDataPos;
	ULONGLONG m_llDataSize;
	DWORD m_dwBuffSize;

	ULONGLONG m_llCurPos;
};
