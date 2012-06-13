// BcasCard.cpp: CBcasCard �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <memory>
#include "BcasCardM.h"
#include "EcmDat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#pragma comment(lib, "WinScard.lib")


using std::auto_ptr;


CBcasCardM::CBcasCardM()
	: m_hBcasCard(NULL)
	, m_bIsEstablish(false)
	, m_dwLastError(BCEC_NOERROR)
{
	// ������ԏ�����
	::ZeroMemory(&m_BcasCardInfo, sizeof(m_BcasCardInfo));
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	m_bIsEstablish = true;
		
	// �J�[�h���[�_��
	EnumCardReader();

}

CBcasCardM::~CBcasCardM()
{
	CloseCard();

}

const DWORD CBcasCardM::GetCardReaderNum(void) const
{
	// �J�[�h���[�_�[����Ԃ�
	return 1;
}

LPCTSTR CBcasCardM::GetCardReaderName(const DWORD dwIndex) const
{
	// �J�[�h���[�_�[����Ԃ�
	return (LPCTSTR)"Hitachi/Maxell M-500U/M-520U 0";
}

const bool CBcasCardM::OpenCard(LPCTSTR lpszReader)
{
	// ���\�[�X�}�l�[�W���R���e�L�X�g�̊m��
	if(!m_bIsEstablish){
		m_dwLastError = BCEC_NOTESTABLISHED;
		return false;
		}
	
	// ��U�N���[�Y����
	CloseCard();


	if(lpszReader){
		// �w�肳�ꂽ�J�[�h���[�_�ɑ΂��ăI�[�v�������݂�
		DWORD dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
		m_hBcasCard = 1;
		}
	else{
		// �S�ẴJ�[�h���[�_�ɑ΂��ăI�[�v�������݂�
		DWORD dwIndex = 0;
	
		while(GetCardReaderName(dwIndex)){
			if(OpenCard(GetCardReaderName(dwIndex++)))return true;			
			}
		
		return false;
		}

	// �J�[�h������
	if(!InitialSetting())return false;

	m_dwLastError = BCEC_NOERROR;

	return true;
}

void CBcasCardM::CloseCard(void)
{
	// �J�[�h���N���[�Y����
}

const DWORD CBcasCardM::GetLastError(void) const
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}

const bool CBcasCardM::EnumCardReader(void)
{

	m_dwLastError = BCEC_NOERROR;

	return true;
}

const bool CBcasCardM::TransmitCommand(const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize)
{

	return true;
}


const BYTE system_key[]={0x36,0x31,0x04,0x66,0x4B,0x17,0xEA,0x5C,0x32,0xDF,0x9C,0xF5,0xC4,0xC3,0x6C,0x1B,
						0xEC,0x99,0x39,0x21,0x68,0x9D,0x4B,0xB7,0xB7,0x4E,0x40,0x84,0x0D,0x2E,0x7D,0x98,};
const BYTE init_cbc[]={	0xFE,0x27,0x19,0x99,0x19,0x69,0x09,0x11,};

const BYTE card_id[] = {0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};

const bool CBcasCardM::InitialSetting(void)
{


	// ���X�|���X���
	::CopyMemory(m_BcasCardInfo.BcasCardID, card_id, 6);	// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey, system_key, 32);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, init_cbc, 8);	// +48	Descrambler CBC initial value

	TRACE0("BcasCardID : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.BcasCardID,6);
	TRACE0("SystemKey : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.SystemKey,32);
	TRACE0("InitialCbc : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.InitialCbc,8);

	// ECM�X�e�[�^�X������
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}

const BYTE * CBcasCardM::GetBcasCardID(void)
{
	// Card ID ��Ԃ�
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.BcasCardID;
}

const BYTE * CBcasCardM::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value ��Ԃ�
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.InitialCbc;
}

const BYTE * CBcasCardM::GetSystemKey(void)
{
	// Descrambling System Key ��Ԃ�
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.SystemKey;
}


CEcmDat* m_pEcm;		// �Ƃ肠�����B�B

const BYTE * CBcasCardM::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE EcmReceiveCmd[] = {0x90, 0x34, 0x00, 0x00};

	// �uECM Receive Command�v����������
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}

	// ECM�T�C�Y���`�F�b�N
	if(!pEcmData || (dwEcmSize < 30) || (dwEcmSize > 256)){
		m_dwLastError = BCEC_BADARGUMENT;
		return NULL;
		}

	// �L���b�V�����`�F�b�N����
	if(!StoreEcmData(pEcmData, dwEcmSize)){
		// ECM������̏ꍇ�̓L���b�V���ς�Ks��Ԃ�
		m_dwLastError = BCEC_NOERROR;
		return m_EcmStatus.KsData;
		}

	// �o�b�t�@����
	DWORD dwRecvSize = 0;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h�\�z
	::CopyMemory(SendData, EcmReceiveCmd, sizeof(EcmReceiveCmd));				// CLA, INS, P1, P2
	SendData[sizeof(EcmReceiveCmd)] = (BYTE)dwEcmSize;							// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(EcmReceiveCmd) + 1], pEcmData, dwEcmSize);	// ECM
	SendData[sizeof(EcmReceiveCmd) + dwEcmSize + 1] = 0x00;						// RESPONSE DATA LENGTH

	m_pEcm->GetKsKey(SendData,sizeof(EcmReceiveCmd) + dwEcmSize + 2,RecvData);
	dwRecvSize = 25;

	// �T�C�Y�`�F�b�N
	if(dwRecvSize != 25){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwLastError = BCEC_TRANSMITERROR;
		return NULL;
		}	
	
	// ���X�|���X���
	::CopyMemory(m_EcmStatus.KsData, &RecvData[6], sizeof(m_EcmStatus.KsData));

	// ���^�[���R�[�h���
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		// Purchased: Viewing
		case 0x0200 :	// Payment-deferred PPV
		case 0x0400 :	// Prepaid PPV
		case 0x0800 :	// Tier
			m_dwLastError = BCEC_NOERROR;
			return m_EcmStatus.KsData;
		
		// ��L�ȊO(�����s��)
		default :
			m_dwLastError = BCEC_ECMREFUSED;
			return NULL;
		}

}

const bool CBcasCardM::StoreEcmData(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	bool bUpdate = false;
	
	// ECM�f�[�^��r
	if(m_EcmStatus.dwLastEcmSize != dwEcmSize){
		// �T�C�Y���ω�����
		bUpdate = true;
		}
	else{
		// �T�C�Y�������ꍇ�̓f�[�^���`�F�b�N����
		for(DWORD dwPos = 0 ; dwPos < dwEcmSize ; dwPos++){
			if(pEcmData[dwPos] != m_EcmStatus.LastEcmData[dwPos]){
				// �f�[�^���s��v
				bUpdate = true;
				break;
				}			
			}
		}

	// ECM�f�[�^��ۑ�����
	if(bUpdate){
		m_EcmStatus.dwLastEcmSize = dwEcmSize;
		::CopyMemory(m_EcmStatus.LastEcmData, pEcmData, dwEcmSize);
		}

	return bUpdate;
}
