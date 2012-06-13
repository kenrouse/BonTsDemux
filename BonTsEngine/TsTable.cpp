// TsTable.cpp: TS�e�[�u�����b�p�[�N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CPsiTable�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////
/*
CPsiTable::CPsiTable()
{
	Reset();
}

CPsiTable::CPsiTable(const CPsiTable &Operand)
{
	*this = Operand;
}

CPsiTable & CPsiTable::operator = (const CPsiTable &Operand)
{
	m_TableArray = Operand.m_TableArray;

	return *this;
}

const bool CPsiTable::StoreSection(const CPsiSection *pSection, bool *pbUpdate)
{
	bool bUpdate = false;

	// �Z�N�V�����̃T�C�Y���`�F�b�N
	if(!pSection->GetPayloadSize()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return false;
		}

	// �J�����g�l�N�X�g�C���W�P�[�^�`�F�b�N(�{���͕ʃo���N�ɒ��߂�ׂ��H)
	if(!pSection->IsCurrentNext()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return true;
		}

	// �e�[�u��ID�g������������
	DWORD dwIndex;

	for(dwIndex = 0UL ; dwIndex < m_TableArray.size() ; dwIndex++){
		if(m_TableArray[dwIndex].wTableIdExtension == pSection->GetTableIdExtension())break;
		}

	if(dwIndex >= m_TableArray.size()){
		// �e�[�u��ID�g����������Ȃ��A�e�[�u���ǉ�
		m_TableArray.resize(dwIndex + 1);
		m_TableArray[dwIndex].wTableIdExtension = pSection->GetTableIdExtension();
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].byVersionNo != pSection->GetVersionNo()){
		// �o�[�W�������s��v�A�e�[�u�����X�V���ꂽ
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.clear();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].wSectionNum != ((WORD)pSection->GetLastSectionNumber() + 1U)){
		// �Z�N�V���������ω�����(��O�H)
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		}

	// �Z�N�V�����f�[�^���X�V����
	m_TableArray[dwIndex].SectionArray[pSection->GetSectionNumber()].SetData(pSection->GetPayloadData(), pSection->GetPayloadSize());

	// �X�V���ݒ�
	if(pbUpdate)*pbUpdate = bUpdate;

	return true;
}

const WORD CPsiTable::GetExtensionNum(void) const
{
	// �e�[�u���̐���Ԃ�
	return m_TableArray.size();
}

const bool CPsiTable::GetExtension(const WORD wIndex, WORD *pwExtension) const
{
	if(wIndex >= GetExtensionNum())return false;

	// �e�[�u��ID�g����Ԃ�
	*pwExtension = m_TableArray[wIndex].wTableIdExtension;

	return true;
}

const bool CPsiTable::GetSectionNum(const WORD wIndex, WORD *pwSectionNum) const
{
	if(wIndex >= GetExtensionNum())return false;

	// �e�[�u��ID�g����Ԃ�
	*pwSectionNum = m_TableArray[wIndex].wSectionNum;

	return true;
}

const CMediaData * CPsiTable::GetSectionData(const WORD wIndex, const BYTE bySectionNo) const
{
	if(wIndex >= GetExtensionNum())return NULL;
	if(bySectionNo > m_TableArray[wIndex].wSectionNum)return NULL;

	// �Z�N�V�����f�[�^��Ԃ�
	return &m_TableArray[wIndex].SectionArray[bySectionNo];
}

void CPsiTable::Reset(void)
{
	// �S�Ẵe�[�u�����폜����
	m_TableArray.clear();
}
*/

/////////////////////////////////////////////////////////////////////////////
// PSI�V���O���e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CPsiSingleTable::CPsiSingleTable(const bool bTargetSectionExt)
	: CTsPidMapTarget()
	, m_bTargetSectionExt(bTargetSectionExt)
	, m_bTableUpdated(false)
{
	// �^�[�Q�b�g�Z�N�V�����A�R�[���o�b�N�ݒ�
	m_PsiSectionParser.SetRecvCallback(bTargetSectionExt, CPsiSingleTable::StoreSection, static_cast<PVOID>(this));
}

CPsiSingleTable::CPsiSingleTable(const CPsiSingleTable &Operand)
{
	// �R�s�[�R���X�g���N�^
	*this = Operand;
}

CPsiSingleTable::~CPsiSingleTable()
{

}

CPsiSingleTable & CPsiSingleTable::operator = (const CPsiSingleTable &Operand)
{
	// �C���X�^���X�̃R�s�[
	m_CurSection = Operand.m_CurSection;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	m_bTargetSectionExt = Operand.m_bTargetSectionExt;
	m_bTableUpdated = Operand.m_bTableUpdated;
	
	// �R�[���o�b�N�Đݒ�
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiSingleTable::StoreSection, static_cast<PVOID>(this));

	return *this;
}

const bool CPsiSingleTable::StorePacket(const CTsPacket *pPacket)
{
	if(!pPacket)return false;
	
	m_bTableUpdated = false;
	
	// �p�P�b�g�X�g�A
	m_PsiSectionParser.StorePacket(pPacket);
	
	return m_bTableUpdated;
}

void CPsiSingleTable::OnPidUnmapped(const WORD wPID)
{
	// �C���X�^���X���J������
	delete this;
}

void CPsiSingleTable::Reset(void)
{
	// ��ԏ�����
	m_PsiSectionParser.Reset();
	m_CurSection.Reset();
	
	m_bTableUpdated = false;
}

const DWORD CPsiSingleTable::GetCrcErrorCount(void) const
{
	// CRC�G���[�J�E���g���擾����
	return m_PsiSectionParser.GetCrcErrorCount();
}

const bool CPsiSingleTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}

void CPsiSingleTable::OnPsiSection(const CPsiSection *pSection)
{
	// �Z�N�V�����̃t�B���^�����O���s���ꍇ�͔h���N���X�ŃI�[�o�[���C�h����
	// �f�t�H���g�̎����ł̓Z�N�V�����y�C���[�h�X�V���ɉ��z�֐��ɒʒm����
	if(!(*pSection == m_CurSection)){
		// �Z�N�V�������X�V���ꂽ
		if(OnTableUpdate(pSection, &m_CurSection)){
			// �Z�N�V�����X�g�A
			m_CurSection = *pSection;
			m_bTableUpdated = true;
			}
		}
}

void CALLBACK CPsiSingleTable::StoreSection(const CPsiSection *pSection, const PVOID pParam)
{
	CPsiSingleTable *pThis = static_cast<CPsiSingleTable *>(pParam);

	// ���z�֐��Ăяo��
	pThis->OnPsiSection(pSection);
}


/////////////////////////////////////////////////////////////////////////////
// PSI�e�[�u���Z�b�g���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////
/*
CPsiTableSuite::CPsiTableSuite()
	: m_bTargetSectionExt(true)
{
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	Reset();
}

CPsiTableSuite::CPsiTableSuite(const CPsiTableSuite &Operand)
{
	*this = Operand;
}

CPsiTableSuite & CPsiTableSuite::operator = (const CPsiTableSuite &Operand)
{
	m_TableSet = Operand.m_TableSet;
	m_bTargetSectionExt = Operand.m_bTargetSectionExt;
	m_bTableUpdated = Operand.m_bTableUpdated;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	return *this;
}

const bool CPsiTableSuite::StorePacket(const CTsPacket *pPacket)
{
	m_bTableUpdated = false;
	
	// PSI�Z�N�V�������e�[�u���ɒǉ�����
	m_PsiSectionParser.StorePacket(pPacket);

	return m_bTableUpdated;
}

void CPsiTableSuite::SetTargetSectionExt(const bool bTargetExt)
{
	// �����ΏۂƂ���Z�N�V�����̎�ނ�ݒ肷��(�g�� or �W��)
	m_bTargetSectionExt = bTargetExt;
	m_PsiSectionParser.SetRecvCallback(bTargetExt, CPsiTableSuite::StoreSection, this);
}

const bool CPsiTableSuite::AddTable(const BYTE byTableID)
{
	const WORD wNum = m_TableSet.size();

	// ������ID���`�F�b�N
	if(GetIndexByID(byTableID) < wNum)return false;

	// �Ō���ɗv�f��ǉ�
	m_TableSet.resize(wNum + 1U);

	// ID���Z�b�g
	m_TableSet[wNum].byTableID = byTableID;

	return true;
}

const WORD CPsiTableSuite::GetIndexByID(const BYTE byTableID)
{
	// �����̃e�[�u��ID����������
	for(WORD wIndex = 0U ; wIndex < m_TableSet.size() ; wIndex++){
		if(m_TableSet[wIndex].byTableID == byTableID)return wIndex;
		}

	// �G���[���͏�Ƀe�[�u���̍ő吔���傫���C���f�b�N�X��Ԃ�
	return 0x0100U;
}

const CPsiTable * CPsiTableSuite::GetTable(const WORD wIndex) const
{
	// �e�[�u����Ԃ�
	return (wIndex < m_TableSet.size())? &m_TableSet[wIndex].PsiTable : NULL;
}

const CMediaData * CPsiTableSuite::GetSectionData(const WORD wIndex, const WORD wSubIndex, const BYTE bySectionNo) const
{
	// �Z�N�V�����f�[�^��Ԃ�
	const CPsiTable *pPsiTable = GetTable(wIndex);

	// �e�[�u����������Ȃ�
	if(!pPsiTable)return NULL;

	// �f�[�^��Ԃ�
	return pPsiTable->GetSectionData(wSubIndex, bySectionNo);
}

void CPsiTableSuite::Reset(void)
{
	m_bTableUpdated = false;

	m_PsiSectionParser.Reset();
	m_TableSet.clear();
}

const DWORD CPsiTableSuite::GetCrcErrorCount(void) const
{
	return m_PsiSectionParser.GetCrcErrorCount();
}

void CALLBACK CPsiTableSuite::StoreSection(const CPsiSection *pSection, const PVOID pParam)
{
	CPsiTableSuite *pThis = static_cast<CPsiTableSuite *>(pParam);

	// �ΏۊO�̃Z�N�V�����͏������Ȃ�
	if(pSection->IsExtendedSection() != pThis->m_bTargetSectionExt)return;

	// �e�[�u��ID������
	const WORD wIndex = pThis->GetIndexByID(pSection->GetTableID());

	// �e�[�u��ID��������Ȃ�
	if(wIndex >= pThis->m_TableSet.size())return;

	// �e�[�u���ɃX�g�A����
	bool bUpdate = false;
	pThis->m_TableSet[wIndex].PsiTable.StoreSection(pSection, &bUpdate);

	if(bUpdate)pThis->m_bTableUpdated = true;
}
*/

/////////////////////////////////////////////////////////////////////////////
// PAT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CPatTable::CPatTable()
	: CPsiSingleTable()
{
	Reset();
}

CPatTable::CPatTable(const CPatTable &Operand)
{
	*this = Operand;
}

CPatTable & CPatTable::operator = (const CPatTable &Operand)
{
	CPsiSingleTable::operator = (Operand);

	m_NitPIDArray = Operand.m_NitPIDArray;
	m_PmtPIDArray = Operand.m_PmtPIDArray;
	
	return *this;
}

const WORD CPatTable::GetNitPID(const WORD wIndex) const
{
	// NIT��PID��Ԃ�
	return (wIndex < m_NitPIDArray.size())? m_NitPIDArray[wIndex].wPID : 0xFFFFU;	// 0xFFFF�͖���`��PID
}

const WORD CPatTable::GetNitNum(void) const
{
	// NIT�̐���Ԃ�
	return m_NitPIDArray.size();
}

const WORD CPatTable::GetPmtPID(const WORD wIndex) const
{
	// PMT��PID��Ԃ�
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wPID : 0xFFFFU;	// 0xFFFF�͖���`��PID
}

const WORD CPatTable::GetProgramID(const WORD wIndex) const
{
	// Program Number ID��Ԃ�
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wProgramID : 0x0000U;	// 0xFFFF�͖���`��PID
}

const WORD CPatTable::GetProgramNum(void) const
{
	// PMT�̐���Ԃ�
	return m_PmtPIDArray.size();
}

const bool CPatTable::IsPmtTablePID(const WORD wPID) const
{
	// PMT��PID���ǂ�����Ԃ�
	for(WORD wIndex = 0U ; wIndex < m_PmtPIDArray.size() ; wIndex++){
		if(wPID == m_PmtPIDArray[wIndex].wPID)return true;
		}

	return false;
}

void CPatTable::Reset(void)
{
	// ��Ԃ��N���A����
	CPsiSingleTable::Reset();

	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();
}

const bool CPatTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if(wDataSize % 4U)return false;						// �e�[�u���̃T�C�Y���s��
	if(pCurSection->GetTableID() != 0x00U)return false;	// �e�[�u��ID���s��

	// PID���N���A����
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	TAG_PATITEM PatItem;

	// �e�[�u������͂���

	TRACE(TEXT("\n------- PAT Table -------\nTS ID = %04X\n"), pCurSection->GetTableIdExtension());

	for(WORD wPos = 0 ; wPos < wDataSize ; wPos += 4U, pHexData += 4){
		PatItem.wProgramID	= ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];				// +1,2
		PatItem.wPID		= ((WORD)(pHexData[2] & 0x1FU) << 8) | (WORD)pHexData[3];	// +3,4

		if(!PatItem.wProgramID){
			// NIT��PID
			TRACE(TEXT("NIT #%u [ID:%04X][PID:%04X]\n"), m_NitPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_NitPIDArray.push_back(PatItem);
			}
		else{
			// PMT��PID
			TRACE(TEXT("PMT #%u [ID:%04X][PID:%04X]\n"), m_PmtPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_PmtPIDArray.push_back(PatItem);
			}
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// PMT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CPmtTable::CPmtTable()
	: CPsiSingleTable()
{
	Reset();
}

CPmtTable::CPmtTable(const CPmtTable &Operand)
{
	*this = Operand;
}

CPmtTable & CPmtTable::operator = (const CPmtTable &Operand)
{
	CPsiSingleTable::operator = (Operand);
	m_wPcrPID = Operand.m_wPcrPID;
	m_TableDescBlock = Operand.m_TableDescBlock;
	m_EsInfoArray = Operand.m_EsInfoArray;
	
	return *this;
}

const WORD CPmtTable::GetPcrPID(void) const
{
	// PCR_PID ��Ԃ�
	return m_wPcrPID;
}

const CDescBlock * CPmtTable::GetTableDesc(void) const
{
	// �e�[�u���̋L�q�q�u���b�N��Ԃ�
	return &m_TableDescBlock;
}

const WORD CPmtTable::GetEcmPID(void) const
{
	// ECM��PID��Ԃ�
	const CCaMethodDesc *pCaMethodDesc = dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG));

	return (pCaMethodDesc)? pCaMethodDesc->GetCaPID() : 0xFFFFU;
}

const WORD CPmtTable::GetEsInfoNum(void) const
{
	// ES���̐���Ԃ�
	return m_EsInfoArray.size();
}

const BYTE CPmtTable::GetStreamTypeID(const WORD wIndex) const
{
	// Stream Type ID ��Ԃ�
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].byStreamTypeID : 0x00U;	// 0x00�͖���`��ID
}

const WORD CPmtTable::GetEsPID(const WORD wIndex) const
{
	// Elementary Stream PID ��Ԃ�
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].wEsPID : 0xFFFFU;			// 0xFFFF�͖���`��PID
}

const CDescBlock * CPmtTable::GetItemDesc(const WORD wIndex) const
{
	// �A�C�e���̋L�q�q�u���b�N��Ԃ�
	return (wIndex < m_EsInfoArray.size())? &m_EsInfoArray[wIndex].DescBlock : NULL;
}

void CPmtTable::Reset(void)
{
	// ��Ԃ��N���A����
	CPsiSingleTable::Reset();

	m_wPcrPID = 0xFFFFU;
	m_TableDescBlock.Reset();
	m_EsInfoArray.clear();
}

const bool CPmtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if(pCurSection->GetTableID() != 0x02U)return false;	// �e�[�u��ID���s��

	// ��Ԃ��N���A����
	m_wPcrPID = 0xFFFFU;
	m_EsInfoArray.clear();

	// �e�[�u������͂���
	WORD wDescLen = ((WORD)(pHexData[2] & 0x0FU) << 8) | (WORD)pHexData[3];
	m_wPcrPID = ((WORD)(pHexData[0] & 0x1FU) << 8) | (WORD)pHexData[1];				// +0,1

	// �L�q�q�u���b�N
	m_TableDescBlock.ParseBlock(&pHexData[4], wDescLen);

	// �X�g���[�����̊J�n�ʒu���v�Z
	WORD wPos = wDescLen + 4U;
	TAG_PMTITEM PmtItem;

	TRACE(TEXT("\n------- PMT Table -------\nProgram Number ID = %04X\nPCR PID = %04X\nECM PID = %04X\n"), pCurSection->GetTableIdExtension(), m_wPcrPID, (m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))? dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))->GetCaPID() : 0xFFFFU);
	
	// �X�g���[���������
	while(wPos < wDataSize){
		PmtItem.byStreamTypeID = pHexData[wPos + 0];													// +0
		PmtItem.wEsPID = ((WORD)(pHexData[wPos + 1] & 0x1FU) << 8) | (WORD)pHexData[wPos + 2];			// +1,2	
		wDescLen = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];				// +3,4

		// �L�q�q�u���b�N
		PmtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wDescLen);	

		TRACE(TEXT("[%u] Stream Type ID = %02X  PID = %04X\n"), m_EsInfoArray.size(), PmtItem.byStreamTypeID, PmtItem.wEsPID);		

		// �e�[�u���ɒǉ�����
		m_EsInfoArray.push_back(PmtItem);
		wPos += (wDescLen + 5U);
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// SDT�e�[�u�����ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CSdtTable::CSdtTable()
	: CPsiSingleTable()
{

}

CSdtTable::CSdtTable(const CSdtTable &Operand)
{
	*this = Operand;
}

CSdtTable & CSdtTable::operator = (const CSdtTable &Operand)
{
	CPsiSingleTable::operator = (Operand);

	return *this;
}

const WORD CSdtTable::GetServiceNum(void) const
{
	// �T�[�r�X����Ԃ�
	return m_ServiceInfoArray.size();
}

const WORD CSdtTable::GetServiceIndexByID(const WORD wServiceID)
{
	// �T�[�r�XID����C���f�b�N�X��Ԃ�
	for(WORD wIndex = 0U ; wIndex < GetServiceNum() ; wIndex++){
		if(m_ServiceInfoArray[wIndex].wServiceID == wServiceID){
			return wIndex;
			}		
		}
	
	// �T�[�r�XID��������Ȃ�
	return 0xFFFFU;
}

const WORD CSdtTable::GetServiceID(const WORD wIndex) const
{
	// 	�T�[�r�XID��Ԃ�
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].wServiceID : 0xFFFFU;
}

const BYTE CSdtTable::GetRunningStatus(const WORD wIndex) const
{
	// Running Status��Ԃ�
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].byRunningStatus : 0xFFU;
}

const bool CSdtTable::GetFreeCaMode(const WORD wIndex) const
{
	// Free CA Mode��Ԃ�
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].bFreeCaMode : false;
}

const CDescBlock * CSdtTable::GetItemDesc(const WORD wIndex) const
{
	// �A�C�e���̋L�q�q�u���b�N��Ԃ�
	return (wIndex < m_ServiceInfoArray.size())? &m_ServiceInfoArray[wIndex].DescBlock : NULL;
}

void CSdtTable::Reset(void)
{
	// ��Ԃ��N���A����
	CPsiSingleTable::Reset();
	
	m_ServiceInfoArray.clear();
}

void CSdtTable::OnPsiSection(const CPsiSection *pSection)
{
	if(pSection->GetTableID() != 0x42U)return;
	
	// Actual Stream��������������
	CPsiSingleTable::OnPsiSection(pSection);
}

const bool CSdtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	// ��Ԃ��N���A����
	m_ServiceInfoArray.clear();

	TRACE(TEXT("\n------- SDT Table -------\nTransport Stream ID = %04X\nOriginal Network ID = %02X%02X\n"), pCurSection->GetTableIdExtension(), pHexData[0], pHexData[1]);

	// �e�[�u������͂���
	for(WORD wPos = 3U ; wPos < wDataSize ; ){
		TAG_SDTITEM SdtItem;
		SdtItem.wServiceID		= ((WORD)pHexData[wPos + 0] << 8) | (WORD)pHexData[wPos + 1];
		SdtItem.byRunningStatus	= pHexData[wPos + 3] >> 5;
		SdtItem.bFreeCaMode		= (pHexData[wPos + 3] & 0x10U)? true : false;
		
		// Service Descriptor
		const WORD wLength = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];

		// �L�q�q�u���b�N
		SdtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wLength);

		// �f�o�b�O�p��������
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(SdtItem.DescBlock.GetDescByTag(CServiceDesc::DESC_TAG));
		if(pServiceDesc){
			TCHAR szServiceName[1024] = {TEXT('\0')};
			pServiceDesc->GetServiceName(szServiceName);
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  Service Type = %02X  Service Name = %s\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode, pServiceDesc->GetServiceType(), szServiceName);		
			}
		else{
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  ���T�[�r�X�L�q�q�Ȃ�\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode);		
			}
		// �����܂�

		// �e�[�u���ǉ�
		m_ServiceInfoArray.push_back(SdtItem);

		// �ʒu�X�V	
		wPos += (wLength + 5U);
		}

	return true;
}
