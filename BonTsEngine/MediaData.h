// MediaData.h: CMediaData クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


class CMediaData
{
public:
	CMediaData();
	CMediaData(const CMediaData &Operand);
	CMediaData(const DWORD dwBuffSize);
	CMediaData(const BYTE *pData, const DWORD dwDataSize);
	CMediaData(const BYTE byFiller, const DWORD dwDataSize);

	virtual ~CMediaData();

	CMediaData & operator = (const CMediaData &Operand);

	virtual BYTE * GetData(void) const;
	virtual const DWORD GetSize(void) const;

	virtual void SetAt(const DWORD dwPos, const BYTE byData);
	virtual const BYTE GetAt(const DWORD dwPos) const;

	virtual const DWORD SetData(const BYTE *pData, const DWORD dwDataSize);
	virtual const DWORD AddData(const BYTE *pData, const DWORD dwDataSize);
	virtual const DWORD AddData(const CMediaData *pData);
	virtual const DWORD AddByte(const BYTE byData);
	virtual const DWORD TrimHead(const DWORD dwTrimSize = 1UL);
	virtual const DWORD TrimTail(const DWORD dwTrimSize = 1UL);

	virtual const DWORD GetBuffer(const DWORD dwGetSize);

	virtual const DWORD SetSize(const DWORD dwSetSize);
	virtual const DWORD SetSize(const DWORD dwSetSize, const BYTE byFiller);
	
	virtual void ClearSize(void);
	virtual void Delete(void);

protected:
	DWORD m_dwDataSize;
	DWORD m_dwBuffSize;
	BYTE *m_pData;
};
