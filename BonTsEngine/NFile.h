// NFile.h: CNFile クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(_NFILE_H_)
#define _NFILE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNFile  
{
public:
	CNFile();
	virtual ~CNFile();

	const BOOL Open(LPCTSTR lpszName, BYTE bFlags);
	void Close(void);

	const BOOL Read(BYTE *pBuff, const DWORD dwLen);
	const BOOL Read(BYTE *pBuff, const DWORD dwLen, const LONGLONG llPos);

	const BOOL Write(const BYTE *pBuff, const DWORD dwLen);
	const BOOL Write(const BYTE *pBuff, const DWORD dwLen, const LONGLONG llPos);

	const LONGLONG GetSize(void);
	const BOOL Seek(const LONGLONG llPos);

	LPCTSTR GetErrorMessage(void);

	enum {CNF_READ = 0x01, CNF_WRITE = 0x02, CNF_NEW = 0x04, CNF_SHAREREAD = 0x08, CNF_SHAREWRITE = 0x10};

	HANDLE m_hFile;
};

#endif // !defined(_NFILE_H_)
