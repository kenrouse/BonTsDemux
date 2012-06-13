// NFile.h: CNFile クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


class CNFile  
{
public:
	enum {CNF_READ = 0x01U, CNF_WRITE = 0x02U, CNF_NEW = 0x04U, CNF_SHAREREAD = 0x08U, CNF_SHAREWRITE = 0x10U, CNF_SHAREDELETE = 0x20U};

	CNFile();
	virtual ~CNFile();

	const bool Open(LPCTSTR lpszName, const BYTE bFlags);
	void Close(void);

	const bool Read(BYTE *pBuff, const DWORD dwLen);
	const bool Read(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	const bool Write(const BYTE *pBuff, const DWORD dwLen);
	const bool Write(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	const ULONGLONG GetSize(void) const;
	const ULONGLONG GetPos(void) const;
	const bool Seek(const ULONGLONG llPos);

	LPCTSTR GetErrorMessage(void) const;

protected:
	HANDLE m_hFile;
};
