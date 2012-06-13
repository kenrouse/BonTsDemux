// TsDescriptor.h: �L�q�q���b�p�[�N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "MediaData.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// �L�q�q�̊��N���X
/////////////////////////////////////////////////////////////////////////////

class CBaseDesc
{
public:
	CBaseDesc();
	CBaseDesc(const CBaseDesc &Operand);
	virtual ~CBaseDesc();
	CBaseDesc & operator = (const CBaseDesc &Operand);

	virtual void CopyDesc(const CBaseDesc *pOperand);
	virtual const bool ParseDesc(const BYTE *pHexData, const WORD wDataLength);

	virtual const bool IsValid(void) const;
	virtual const BYTE GetTag(void) const;
	virtual const BYTE GetLength(void) const;

	virtual void Reset(void);

protected:
	virtual const bool StoreContents(const BYTE *pPayload);

	BYTE m_byDescTag;	// �L�q�q�^�O
	BYTE m_byDescLen;	// �L�q�q��
	bool m_bIsValid;	// ��͌���
};


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access Method �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CCaMethodDesc : public CBaseDesc
{
public:
	enum {DESC_TAG = 0x09U};

	CCaMethodDesc();
	CCaMethodDesc(const CCaMethodDesc &Operand);
	CCaMethodDesc & operator = (const CCaMethodDesc &Operand);
	
// CBaseDesc
	virtual void CopyDesc(const CBaseDesc *pOperand);
	virtual void Reset(void);

// CCaMethodDesc
	const WORD GetCaMethodID(void) const;
	const WORD GetCaPID(void) const;
	const CMediaData * GetPrivateData(void) const;

protected:
	virtual const bool StoreContents(const BYTE *pPayload);

	WORD m_wCaMethodID;			// Conditional Access Method ID
	WORD m_wCaPID;				// Conditional Access PID
	CMediaData m_PrivateData;	// Private Data
};


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CServiceDesc : public CBaseDesc
{
public:
	enum {DESC_TAG = 0x48U};

	CServiceDesc();
	CServiceDesc(const CServiceDesc &Operand);
	CServiceDesc & operator = (const CServiceDesc &Operand);
	
// CBaseDesc
	virtual void CopyDesc(const CBaseDesc *pOperand);
	virtual void Reset(void);

// CServiceDesc
	const BYTE GetServiceType(void) const;
	const DWORD GetProviderName(LPTSTR lpszDst) const;
	const DWORD GetServiceName(LPTSTR lpszDst) const;

protected:
	virtual const bool StoreContents(const BYTE *pPayload);
	
	BYTE m_byServiceType;			// Service Type
	TCHAR m_szProviderName[256];	// Service Provider Name
	TCHAR m_szServiceName[256];		// Service Name
};


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CStreamIdDesc : public CBaseDesc
{
public:
	enum {DESC_TAG = 0x52U};

	CStreamIdDesc();
	CStreamIdDesc(const CStreamIdDesc &Operand);
	CStreamIdDesc & operator = (const CStreamIdDesc &Operand);
	
// CBaseDesc
	virtual void CopyDesc(const CBaseDesc *pOperand);
	virtual void Reset(void);

// CStreamIdDesc
	const BYTE GetComponentTag(void) const;

protected:
	virtual const bool StoreContents(const BYTE *pPayload);

	BYTE m_byComponentTag;		// Component Tag
};


/////////////////////////////////////////////////////////////////////////////
// �L�q�q�u���b�N���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CDescBlock
{
public:
	CDescBlock();
	CDescBlock(const CDescBlock &Operand);
	~CDescBlock();
	CDescBlock & operator = (const CDescBlock &Operand);

	const WORD ParseBlock(const BYTE *pHexData, const WORD wDataLength);
	const CBaseDesc * ParseBlock(const BYTE *pHexData, const WORD wDataLength, const BYTE byTag);

	virtual void Reset(void);

	const WORD GetDescNum(void) const;
	const CBaseDesc * GetDescByIndex(const WORD wIndex = 0U) const;
	const CBaseDesc * GetDescByTag(const BYTE byTag) const;

protected:
	CBaseDesc * ParseDesc(const BYTE *pHexData, const WORD wDataLength);
	static CBaseDesc * CreateDescInstance(const BYTE byTag);

	vector<CBaseDesc *> m_DescArray;
};
