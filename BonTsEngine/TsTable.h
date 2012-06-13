// TsTable.h: TS�e�[�u�����b�p�[�N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "MediaData.h"
#include "TsStream.h"
#include "TsDescriptor.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// PSI�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////
/*
class CPsiTable		// �ۗ��F ���ۂ̃��[�X�P�[�X�𔻒f������Ŏd�l�����߂�K�v����
{
public:
	CPsiTable();
	CPsiTable(const CPsiTable &Operand);

	CPsiTable & operator = (const CPsiTable &Operand);

	const bool StoreSection(const CPsiSection *pSection, bool *pbUpdate = NULL);

	const WORD GetExtensionNum(void) const;
	const bool GetExtension(const WORD wIndex, WORD *pwExtension) const;
	const bool GetSectionNum(const WORD wIndex, WORD *pwSectionNum) const;
	const CMediaData * GetSectionData(const WORD wIndex = 0U, const BYTE bySectionNo = 0U) const;

	void Reset(void);

protected:
	struct TAG_TABLEITEM
	{
		WORD wTableIdExtension;				// �e�[�u��ID�g��
		WORD wSectionNum;					// �Z�N�V������
		BYTE byVersionNo;					// �o�[�W�����ԍ�
		vector<CMediaData> SectionArray;	// �Z�N�V�����f�[�^
	};

	vector<TAG_TABLEITEM> m_TableArray;		// �e�[�u��
};
*/


/////////////////////////////////////////////////////////////////////////////
// PSI�V���O���e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CPsiSingleTable : public CTsPidMapTarget
{
public:
	CPsiSingleTable(const bool bTargetSectionExt = true);
	CPsiSingleTable(const CPsiSingleTable &Operand);
	virtual ~CPsiSingleTable();

	CPsiSingleTable & operator = (const CPsiSingleTable &Operand);

// CTsPidMapTarget
	virtual const bool StorePacket(const CTsPacket *pPacket);
	virtual void OnPidUnmapped(const WORD wPID);

// CPsiSingleTable
	void Reset(void);

	const DWORD GetCrcErrorCount(void) const;

	CPsiSection m_CurSection;

protected:
	virtual void OnPsiSection(const CPsiSection *pSection);
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

private:
	static void CALLBACK StoreSection(const CPsiSection *pSection, const PVOID pParam);

	CPsiSectionParser m_PsiSectionParser;
	bool m_bTargetSectionExt;
	bool m_bTableUpdated;
};


/////////////////////////////////////////////////////////////////////////////
// PSI�e�[�u���Z�b�g���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////
/*
class CPsiTableSuite	// �ۗ��F ���ۂ̃��[�X�P�[�X�𔻒f������Ŏd�l�����߂�K�v����
{
public:
	CPsiTableSuite();
	CPsiTableSuite(const CPsiTableSuite &Operand);

	CPsiTableSuite & operator = (const CPsiTableSuite &Operand);

	const bool StorePacket(const CTsPacket *pPacket);

	void SetTargetSectionExt(const bool bTargetExt);
	const bool AddTable(const BYTE byTableID);

	const WORD GetIndexByID(const BYTE byTableID);
	const CPsiTable * GetTable(const WORD wIndex = 0U) const;
	const CMediaData * GetSectionData(const WORD wIndex = 0U, const WORD wSubIndex = 0U, const BYTE bySectionNo = 0U) const;

	const DWORD GetCrcErrorCount(void) const;
	void Reset(void);

protected:
	static void CALLBACK StoreSection(const CPsiSection *pSection, const PVOID pParam);

	struct TAG_TABLESET
	{
		BYTE byTableID;						// �e�[�u��ID
		CPsiTable PsiTable;					// �e�[�u��
	};

	vector<TAG_TABLESET> m_TableSet;		// �e�[�u���Z�b�g

	bool m_bTargetSectionExt;
	bool m_bTableUpdated;

private:
	CPsiSectionParser m_PsiSectionParser;
};
*/

/////////////////////////////////////////////////////////////////////////////
// PAT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CPatTable : public CPsiSingleTable
{
public:
	CPatTable();
	CPatTable(const CPatTable &Operand);

	CPatTable & operator = (const CPatTable &Operand);

// CPatTable
	const WORD GetNitPID(const WORD wIndex = 0U) const;
	const WORD GetNitNum(void) const;

	const WORD GetPmtPID(const WORD wIndex = 0U) const;
	const WORD GetProgramID(const WORD wIndex = 0U) const;
	const WORD GetProgramNum(void) const;

	const bool IsPmtTablePID(const WORD wPID) const;

	void Reset(void);

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_PATITEM
	{
		WORD wProgramID;	// �����ԑg�ԍ�ID
		WORD wPID;			// NIT or PMT��PID
	};

	vector<TAG_PATITEM> m_NitPIDArray;
	vector<TAG_PATITEM> m_PmtPIDArray;
};


/////////////////////////////////////////////////////////////////////////////
// PMT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CPmtTable : public CPsiSingleTable
{
public:
	CPmtTable();
	CPmtTable(const CPmtTable &Operand);

	CPmtTable & operator = (const CPmtTable &Operand);

// CPmtTable
	const WORD GetPcrPID(void) const;
	const CDescBlock * GetTableDesc(void) const;
	const WORD GetEcmPID(void) const;

	const WORD GetEsInfoNum(void) const;
	const BYTE GetStreamTypeID(const WORD wIndex) const;
	const WORD GetEsPID(const WORD wIndex) const;
	const CDescBlock * GetItemDesc(const WORD wIndex) const;

	void Reset(void);

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_PMTITEM
	{
		BYTE byStreamTypeID;			// Stream Type ID
		WORD wEsPID;					// Elementary Stream PID
		CDescBlock DescBlock;			// Stream ID Descriptor ��
	};

	vector<TAG_PMTITEM> m_EsInfoArray;

	WORD m_wPcrPID;						// PCR_PID
	CDescBlock m_TableDescBlock;		// Conditional Access Method Descriptor ��
};


/////////////////////////////////////////////////////////////////////////////
// SDT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

class CSdtTable : public CPsiSingleTable
{
public:
	CSdtTable();
	CSdtTable(const CSdtTable &Operand);

	CSdtTable & operator = (const CSdtTable &Operand);

// CSdtTable
	const WORD GetServiceNum(void) const;
	const WORD GetServiceIndexByID(const WORD wServiceID);
	const WORD GetServiceID(const WORD wIndex) const;
	const BYTE GetRunningStatus(const WORD wIndex) const;
	const bool GetFreeCaMode(const WORD wIndex) const;
	const CDescBlock * GetItemDesc(const WORD wIndex) const;

	void Reset(void);

protected:
	virtual void OnPsiSection(const CPsiSection *pSection);
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_SDTITEM
	{
		WORD wServiceID;				// Service ID
		BYTE byRunningStatus;			// Running Status
		bool bFreeCaMode;				// Free CA Mode(true: CA / false: Free)
		CDescBlock DescBlock;			// Service Descriptor ��
	};

	vector<TAG_SDTITEM> m_ServiceInfoArray;
};




