// TsDescrambler.cpp: CTsDescrambler �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsDescrambler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CTsDescrambler �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CTsDescrambler(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_dwInputPacketCount(0UL)
	, m_dwScramblePacketCount(0UL)
{
	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CTsDescrambler::OnPatUpdated, static_cast<PVOID>(this));
}

CTsDescrambler::~CTsDescrambler()
{
	CloseBcasCard();
}

void CTsDescrambler::Reset(void)
{
	// ������Ԃ�����������
	m_PidMapManager.UnmapAllTarget();

	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CTsDescrambler::OnPatUpdated, static_cast<PVOID>(this));

	// ���v�f�[�^������
	m_dwInputPacketCount = 0UL;
	m_dwScramblePacketCount = 0UL;

	// �����f�R�[�_������������
	CMediaDecoder::Reset();
}

const DWORD CTsDescrambler::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CTsDescrambler::GetOutputNum(void) const
{
	return 1UL;
}

const bool CTsDescrambler::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;

	// ���̓p�P�b�g���J�E���g
	if(m_dwInputPacketCount < 0xFFFFFFFFUL)m_dwInputPacketCount++;

	// PID���[�e�B���O
	m_PidMapManager.StorePacket(pTsPacket);

	// �p�P�b�g�o��
	if(pTsPacket->IsScrambled()){
		// �����R��p�P�b�g���J�E���g
		if(m_dwScramblePacketCount < 0xFFFFFFFFUL)m_dwScramblePacketCount++;
	} else {

		// �p�P�b�g�������f�R�[�_�Ƀf�[�^��n��
		OutputMedia(pMediaData);
	}

	return true;
}

const bool CTsDescrambler::OpenBcasCard(DWORD *pErrorCode)
{
	// �J�[�h���[�_����B-CAS�J�[�h���������ĊJ��
	const bool bReturn = m_BcasCard.OpenCard();
	
	// �G���[�R�[�h�Z�b�g
	if(pErrorCode)*pErrorCode = m_BcasCard.GetLastError();

	return bReturn;
}

void CTsDescrambler::CloseBcasCard(void)
{
	// B-CAS�J�[�h�����
	m_BcasCard.CloseCard();
}

const bool CTsDescrambler::GetBcasCardID(BYTE *pCardID)
{
	// �J�[�hID�擾
	const BYTE *pBuff = m_BcasCard.GetBcasCardID();
	
	// �o�b�t�@�ɃR�s�[
	if(pCardID && pBuff)::CopyMemory(pCardID, pBuff, 6UL);
	
	return (pBuff)? true : false;
}

const DWORD CTsDescrambler::GetInputPacketCount(void) const
{
	// ���̓p�P�b�g����Ԃ�
	return m_dwInputPacketCount;
}

const DWORD CTsDescrambler::GetScramblePacketCount(void) const
{
	// �����R��p�P�b�g����Ԃ�
	return m_dwScramblePacketCount;
}

void CALLBACK CTsDescrambler::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PAT���X�V���ꂽ
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPatTable)::DebugBreak();
#endif

	// PMT�e�[�u��PID�}�b�v�ǉ�
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		pMapManager->MapTarget(pPatTable->GetPmtPID(wIndex), new CPmtTable, CTsDescrambler::OnPmtUpdated, pParam);
		}
}

void CALLBACK CTsDescrambler::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMT���X�V���ꂽ
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPmtTable)::DebugBreak();
#endif

	// ECM��PID�}�b�v�ǉ�
	const WORD wEcmPID = pPmtTable->GetEcmPID();
	if(wEcmPID >= 0x1FFFU)return;

	// ������ECM�����^�[�Q�b�g���m�F
	CEcmProcessor *pEcmProcessor = dynamic_cast<CEcmProcessor *>(pMapManager->GetMapTarget(wEcmPID));

	if(!pEcmProcessor){
		// ECM���������N���X�V�K�}�b�v
		pEcmProcessor = new CEcmProcessor(&pThis->m_BcasCard);
		pMapManager->MapTarget(wEcmPID, pEcmProcessor);
		}
	
	// ES��PID�}�b�v�ǉ�
	for(WORD wIndex = 0U ; wIndex < pPmtTable->GetEsInfoNum() ; wIndex++){
		pMapManager->MapTarget(pPmtTable->GetEsPID(wIndex), new CEsProcessor(pEcmProcessor), NULL, pParam);
		}
}


//////////////////////////////////////////////////////////////////////
// CTsDescrambler::CEcmProcessor �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CEcmProcessor::CEcmProcessor(CBcasCard *pBcasCard)
	: CDynamicReferenceable()
	, CPsiSingleTable(true)
	, m_pBcasCard(pBcasCard)
	, m_bLastEcmSucceed(true)
{
	// MULTI2�f�R�[�_�ɃV�X�e���L�[�Ə���CBC���Z�b�g
	m_Multi2Decoder.Initialize(m_pBcasCard->GetSystemKey(), m_pBcasCard->GetInitialCbc());
}

void CTsDescrambler::CEcmProcessor::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	// �Q�ƃJ�E���g�ǉ�
	AddRef();
}

void CTsDescrambler::CEcmProcessor::OnPidUnmapped(const WORD wPID)
{
	// �Q�ƃJ�E���g�J��
	ReleaseRef();
}

const bool CTsDescrambler::CEcmProcessor::DescramblePacket(CTsPacket *pTsPacket)
{
	// �X�N�����u������
	if(m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(), (DWORD)pTsPacket->GetPayloadSize(), pTsPacket->m_Header.byTransportScramblingCtrl)){
		// �g�����X�|�[�g�X�N�����u������Đݒ�
		pTsPacket->SetAt(3UL, pTsPacket->GetAt(3UL) & 0x3FU);
		pTsPacket->m_Header.byTransportScramblingCtrl = 0U;
		return true;
		}
	
	return false;
}

const bool CTsDescrambler::CEcmProcessor::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	// ECM��B-CAS�J�[�h�ɓn���ăL�[�擾
	const BYTE *pKsData = m_pBcasCard->GetKsFromEcm(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());

	// ECM�������s���͈�x����B-CAS�J�[�h���ď���������
	if(!pKsData && m_bLastEcmSucceed && (m_pBcasCard->GetLastError() != BCEC_ECMREFUSED)){
		if(m_pBcasCard->OpenCard()){
			pKsData = m_pBcasCard->GetKsFromEcm(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());
			}
		}

	// �X�N�����u���L�[�X�V
	m_Multi2Decoder.SetScrambleKey(pKsData);

	// ECM����������ԍX�V
	m_bLastEcmSucceed = (pKsData)? true : false;

	return true;
}


//////////////////////////////////////////////////////////////////////
// CTsDescrambler::CEsProcessor �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CEsProcessor::CEsProcessor(CTsDescrambler::CEcmProcessor *pEcmProcessor)
	: CTsPidMapTarget()
	, m_pEcmProcessor(pEcmProcessor)
{
	// �Q�ƃJ�E���g�ǉ�
	m_pEcmProcessor->AddRef();
}

CTsDescrambler::CEsProcessor::~CEsProcessor()
{
	// �Q�ƃJ�E���g�폜
	m_pEcmProcessor->ReleaseRef();
}

const bool CTsDescrambler::CEsProcessor::StorePacket(const CTsPacket *pPacket)
{
	// �X�N�����u������
	m_pEcmProcessor->DescramblePacket(const_cast<CTsPacket *>(pPacket));

	return false;
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(const WORD wPID)
{
	// �C���X�^���X�J��
	delete this;
}
