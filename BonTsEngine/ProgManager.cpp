// ProgManager.cpp: CProgManager �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"
#include "ProgManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CProgManager �\�z/����
//////////////////////////////////////////////////////////////////////

CProgManager::CProgManager(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
{
	// �v���O�����f�[�^�x�[�X�C���X�^���X����
	m_pProgDatabase = new CProgDatabase(*this);
}

CProgManager::~CProgManager()
{
	// �v���O�����f�[�^�x�[�X�C���X�^���X�J��
	delete m_pProgDatabase;
}

void CProgManager::Reset()
{
	// �T�[�r�X���X�g���N���A
	m_ServiceList.clear();

	// �v���O�����f�[�^�x�[�X���Z�b�g
	m_pProgDatabase->Reset();

	// ���ʃf�R�[�_�����Z�b�g
	CMediaDecoder::Reset();
}

const DWORD CProgManager::GetInputNum() const
{
	return 1UL;
}

const DWORD CProgManager::GetOutputNum() const
{
	return 1UL;
}

const bool CProgManager::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;

	// PID���[�e�B���O
	m_PidMapManager.StorePacket(pTsPacket);

	// ���̃t�B���^�Ƀf�[�^��n��
	OutputMedia(pMediaData);

	return true;
}

const WORD CProgManager::GetServiceNum(void) const
{
	// �T�[�r�X����Ԃ�
	return m_ServiceList.size();
}

const bool CProgManager::GetServiceID(WORD *pwServiceID, const WORD wIndex) const
{
	// �T�[�r�XID���擾����
	if((wIndex < GetServiceNum()) && pwServiceID){
		*pwServiceID = m_ServiceList[wIndex].wServiceID;
		return true;		
		}
		
	return false;
}

const bool CProgManager::GetServiceEsPID(WORD *pwVideoPID, WORD *pwAudioPID, const WORD wIndex) const
{
	// ES��PID���擾����
	if((wIndex < GetServiceNum()) && (pwVideoPID || pwAudioPID)){
		if(pwVideoPID)*pwVideoPID = m_ServiceList[wIndex].wVideoEsPID;
		if(pwAudioPID)*pwAudioPID = m_ServiceList[wIndex].wAudioEsPID;
		return true;
		}

	return false;
}

const DWORD CProgManager::GetServiceName(LPTSTR lpszDst, const WORD wIndex) const
{
	// �T�[�r�X�����擾����
	if((wIndex < GetServiceNum()) && lpszDst){
		const WORD wNameLen = ::lstrlen(m_ServiceList[wIndex].szServiceName);
		if(wNameLen)::lstrcpy(lpszDst, m_ServiceList[wIndex].szServiceName);
		return wNameLen;		
		}
		
	return 0U;
}

void CProgManager::OnServiceListUpdated(void)
{
	// �T�[�r�X���X�g�N���A�A���T�C�Y
	m_ServiceList.clear();

	// �T�[�r�X���X�g�\�z
	for(WORD wIndex = 0U, wServiceNum = 0U ; wIndex < m_pProgDatabase->m_ServiceList.size() ; wIndex++){
		if(m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID != 0xFFFFU){
			// MPEG2�f���̂�(�����Z�O�A�f�[�^�����ȊO)
			m_ServiceList.resize(wServiceNum + 1);
			m_ServiceList[wServiceNum].wServiceID = m_pProgDatabase->m_ServiceList[wIndex].wServiceID;
			m_ServiceList[wServiceNum].wVideoEsPID = m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID;
			m_ServiceList[wServiceNum].wAudioEsPID = m_pProgDatabase->m_ServiceList[wIndex].wAudioEsPID;
			m_ServiceList[wServiceNum].szServiceName[0] = TEXT('\0');
			wServiceNum++;
			}
		}

	TRACE(TEXT("CProgManager::OnServiceListUpdated()\n"));

	SendDecoderEvent(EID_SERVICE_LIST_UPDATED);
}

void CProgManager::OnServiceInfoUpdated(void)
{
	// �T�[�r�X�����X�V����
	for(WORD wIndex = 0U, wServiceNum = 0U ; wIndex < GetServiceNum() ; wIndex++){
		const WORD wServiceIndex = m_pProgDatabase->GetServiceIndexByID(m_ServiceList[wIndex].wServiceID);

		if(wServiceIndex != 0xFFFFU){
			if(m_pProgDatabase->m_ServiceList[wIndex].szServiceName[0]){
				::lstrcpy(m_ServiceList[wIndex].szServiceName, m_pProgDatabase->m_ServiceList[wServiceIndex].szServiceName);
				}
			else{
				::wsprintf(m_ServiceList[wIndex].szServiceName, TEXT("�T�[�r�X%u"), wIndex + 1U);
				}
			}
		}

	TRACE(TEXT("CProgManager::OnServiceInfoUpdated()\n"));
	
	SendDecoderEvent(EID_SERVICE_INFO_UPDATED);
}


//////////////////////////////////////////////////////////////////////
// CProgDatabase �\�z/����
//////////////////////////////////////////////////////////////////////

CProgManager::CProgDatabase::CProgDatabase(CProgManager &ProgManager)
	: m_ProgManager(ProgManager)
	, m_PidMapManager(ProgManager.m_PidMapManager)
	, m_wTransportStreamID(0x0000U)
{
	Reset();
}

CProgManager::CProgDatabase::~CProgDatabase()
{
	UnmapTable();
}

void CProgManager::CProgDatabase::Reset(void)
{
	// �S�e�[�u���A���}�b�v
	UnmapTable();

	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CProgDatabase::OnPatUpdated, static_cast<PVOID>(this));
}

void CProgManager::CProgDatabase::UnmapTable(void)
{
	// �SPMT PID�A���}�b�v
	for(WORD wIndex = 0U ; wIndex < m_ServiceList.size() ; wIndex++){
		m_PidMapManager.UnmapTarget(m_ServiceList[wIndex].wPmtTablePID);
		}

	// �T�[�r�X���X�g�N���A
	m_ServiceList.clear();
	
	// �g�����X�|�[�g�X�g���[��ID������
	m_wTransportStreamID = 0xFFFFU;
	
	// PAT�e�[�u�����Z�b�g
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(m_PidMapManager.GetMapTarget(0x0000U));
	if(pPatTable)pPatTable->Reset();
}

const WORD CProgManager::CProgDatabase::GetServiceIndexByID(const WORD wServiceID)
{
	// �v���O����ID����T�[�r�X�C���f�b�N�X����������
	for(WORD wIndex = 0U ; wIndex < m_ServiceList.size() ; wIndex++){
		if(m_ServiceList[wIndex].wServiceID == wServiceID)return wIndex;
		}

	// �v���O����ID��������Ȃ�
	return 0xFFFFU;
}

void CALLBACK CProgManager::CProgDatabase::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PAT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);
		
	// �g�����X�|�[�g�X�g���[��ID�X�V
	pThis->m_wTransportStreamID = pPatTable->m_CurSection.GetTableIdExtension();

	// ��PMT��PID���A���}�b�v����
	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		pMapManager->UnmapTarget(pThis->m_ServiceList[wIndex].wPmtTablePID);
		}

	// �VPMT���X�g�A����
	pThis->m_ServiceList.resize(pPatTable->GetProgramNum());

	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		// �T�[�r�X���X�g�X�V
		pThis->m_ServiceList[wIndex].bIsUpdated = false;
		pThis->m_ServiceList[wIndex].wServiceID = pPatTable->GetProgramID(wIndex);
		pThis->m_ServiceList[wIndex].wPmtTablePID = pPatTable->GetPmtPID(wIndex);

		pThis->m_ServiceList[wIndex].wVideoEsPID = 0xFFFFU;
		pThis->m_ServiceList[wIndex].wAudioEsPID = 0xFFFFU;
		pThis->m_ServiceList[wIndex].byVideoComponentTag = 0xFFU;
		pThis->m_ServiceList[wIndex].byAudioComponentTag = 0xFFU;
		pThis->m_ServiceList[wIndex].byServiceType = 0xFFU;
		pThis->m_ServiceList[wIndex].byRunningStatus = 0xFFU;
		pThis->m_ServiceList[wIndex].bIsCaService = false;
		pThis->m_ServiceList[wIndex].szServiceName[0] = TEXT('\0');
		
		// PMT��PID���}�b�v
		pMapManager->MapTarget(pPatTable->GetPmtPID(wIndex), new CPmtTable, CProgDatabase::OnPmtUpdated, pParam);
		}
}

void CALLBACK CProgManager::CProgDatabase::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

	// �T�[�r�X�C���f�b�N�X������
	const WORD wServiceIndex = pThis->GetServiceIndexByID(pPmtTable->m_CurSection.GetTableIdExtension());
	if(wServiceIndex == 0xFFFFU)return;

	// �r�f�IES��PID���X�g�A
	pThis->m_ServiceList[wServiceIndex].wVideoEsPID = 0xFFFFU;
	
	for(WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++){
		// �uITU-T Rec. H.262|ISO/IEC 13818-2 Video or ISO/IEC 11172-2�v�̃X�g���[���^�C�v������
		if(pPmtTable->GetStreamTypeID(wEsIndex) == 0x02U){
			pThis->m_ServiceList[wServiceIndex].wVideoEsPID = pPmtTable->GetEsPID(wEsIndex);
			break;
			}		
		}

	// �I�[�f�B�IES��PID���X�g�A
	pThis->m_ServiceList[wServiceIndex].wAudioEsPID = 0xFFFFU;
	
	for(WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++){
		// �uISO/IEC 13818-7 Audio (ADTS Transport Syntax)�v�̃X�g���[���^�C�v������
		if(pPmtTable->GetStreamTypeID(wEsIndex) == 0x0FU){
			pThis->m_ServiceList[wServiceIndex].wAudioEsPID = pPmtTable->GetEsPID(wEsIndex);
			break;
			}
		}

	// �X�V�ς݃}�[�N
	pThis->m_ServiceList[wServiceIndex].bIsUpdated = true;
	
/*
	�����ǂɂ���Ă�PAT�Ɋ܂܂��T�[�r�X�S�Ă�PMT�𗬂��Ă��Ȃ��ꍇ������B
�@�@(���x���n���h�����Ăяo�����̂�h�~���邽�ߑS�Ă̏�񂪂�������i�K�ŌĂяo����������)

	// ����PMT�̍X�V�󋵂𒲂ׂ�
	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		if(!pThis->m_ServiceList[wIndex].bIsUpdated)return;
		}
*/

	// SDT�e�[�u�����ă}�b�v����
	pMapManager->MapTarget(0x0011U, new CSdtTable, CProgDatabase::OnSdtUpdated, pParam);
	
	// �C�x���g�n���h���Ăяo��
	pThis->m_ProgManager.OnServiceListUpdated();
}

void CALLBACK CProgManager::CProgDatabase::OnSdtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// SDT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CSdtTable *pSdtTable = dynamic_cast<CSdtTable *>(pMapTarget);

	for(WORD wSdtIndex = 0U ; wSdtIndex < pSdtTable->GetServiceNum() ; wSdtIndex++){
		// �T�[�r�XID������
		const WORD wServiceIndex = pThis->GetServiceIndexByID(pSdtTable->GetServiceID(wSdtIndex));
		if(wServiceIndex == 0xFFFFU)continue;

		// �T�[�r�X���X�V
		pThis->m_ServiceList[wServiceIndex].byRunningStatus = pSdtTable->GetRunningStatus(wSdtIndex);
		pThis->m_ServiceList[wServiceIndex].bIsCaService = pSdtTable->GetFreeCaMode(wSdtIndex);
		
		// �T�[�r�X���X�V
		pThis->m_ServiceList[wServiceIndex].szServiceName[0] = TEXT('\0');

		const CDescBlock *pDescBlock = pSdtTable->GetItemDesc(wSdtIndex);
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(pDescBlock->GetDescByTag(CServiceDesc::DESC_TAG));

		if(pServiceDesc){
			pServiceDesc->GetServiceName(pThis->m_ServiceList[wServiceIndex].szServiceName);
			pThis->m_ServiceList[wServiceIndex].byServiceType = pServiceDesc->GetServiceType();
			}
		}

	// �C�x���g�n���h���Ăяo��
	pThis->m_ProgManager.OnServiceInfoUpdated();
}
