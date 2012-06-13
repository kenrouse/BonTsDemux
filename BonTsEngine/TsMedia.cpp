// TsMedia.cpp: TS���f�B�A���b�p�[�N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsMedia.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CPesPacket�N���X�̍\�z/����
/////////////////////////////////////////////////////////////////////////////

CPesPacket::CPesPacket()
	: CMediaData()
{
	Reset();
}

CPesPacket::CPesPacket(const DWORD dwBuffSize)
	: CMediaData(dwBuffSize)
{
	Reset();
}

CPesPacket::CPesPacket(const CPesPacket &Operand)
	: CMediaData()
{
	Reset();

	*this = Operand;
}

CPesPacket & CPesPacket::operator = (const CPesPacket &Operand)
{
	// �C���X�^���X�̃R�s�[
	CMediaData::operator = (Operand);
	m_Header = Operand.m_Header;

	return *this;
}

const bool CPesPacket::ParseHeader(void)
{
	if(m_dwDataSize < 9UL)return false;														// PES_header_data_length�܂ł�6�o�C�g
	else if(m_pData[0] != 0x00U || m_pData[1] != 0x00U || m_pData[2] != 0x01U)return false;	// packet_start_code_prefix�ُ�
	else if((m_pData[6] & 0xC0U) != 0x80U)return false;										// �Œ�r�b�g�ُ�

	// �w�b�_���
	m_Header.byStreamID					= m_pData[3];										// +3 bit7-0
	m_Header.wPacketLength				= ((WORD)m_pData[4] << 8) | (WORD)m_pData[5];		// +4, +5
	m_Header.byScramblingCtrl			= (m_pData[6] & 0x30U) >> 4;						// +6 bit5-4
	m_Header.bPriority					= (m_pData[6] & 0x08U)? true : false;				// +6 bit3
	m_Header.bDataAlignmentIndicator	= (m_pData[6] & 0x04U)? true : false;				// +6 bit2
	m_Header.bCopyright					= (m_pData[6] & 0x02U)? true : false;				// +6 bit1
	m_Header.bOriginalOrCopy			= (m_pData[6] & 0x01U)? true : false;				// +6 bit0
	m_Header.byPtsDtsFlags				= (m_pData[7] & 0xC0U) >> 6;						// +7 bit7-6
	m_Header.bEscrFlag					= (m_pData[7] & 0x20U)? true : false;				// +7 bit5
	m_Header.bEsRateFlag				= (m_pData[7] & 0x10U)? true : false;				// +7 bit4
	m_Header.bDsmTrickModeFlag			= (m_pData[7] & 0x08U)? true : false;				// +7 bit3
	m_Header.bAdditionalCopyInfoFlag	= (m_pData[7] & 0x04U)? true : false;				// +7 bit2
	m_Header.bCrcFlag					= (m_pData[7] & 0x02U)? true : false;				// +7 bit1
	m_Header.bExtensionFlag				= (m_pData[7] & 0x01U)? true : false;				// +7 bit0
	m_Header.byHeaderDataLength			= m_pData[8];										// +8 bit7-0

	// �w�b�_�̃t�H�[�}�b�g�K�������`�F�b�N����
	if(m_Header.byScramblingCtrl != 0U)return false;	// Not scrambled �̂ݑΉ�
	else if(m_Header.byPtsDtsFlags == 1U)return false;	// ����`�̃t���O

	return true;
}

void CPesPacket::Reset(void)
{
	// �f�[�^���N���A����
	ClearSize();	
	::ZeroMemory(&m_Header, sizeof(m_Header));
}

const BYTE CPesPacket::GetStreamID(void) const
{
	// Stream ID��Ԃ�
	return m_Header.byStreamID;
}

const WORD CPesPacket::GetPacketLength(void) const
{
	// PES Packet Length��Ԃ�
	return m_Header.wPacketLength;
}

const BYTE CPesPacket::GetScramblingCtrl(void) const
{	// PES Scrambling Control��Ԃ�
	return m_Header.byScramblingCtrl;
}

const bool CPesPacket::IsPriority(void) const
{	// PES Priority��Ԃ�
	return m_Header.bPriority;
}

const bool CPesPacket::IsDataAlignmentIndicator(void) const
{
	// Data Alignment Indicator��Ԃ�
	return m_Header.bDataAlignmentIndicator;
}

const bool CPesPacket::IsCopyright(void) const
{
	// Copyright��Ԃ�
	return m_Header.bCopyright;
}

const bool CPesPacket::IsOriginalOrCopy(void) const
{
	// Original or Copy��Ԃ�
	return m_Header.bOriginalOrCopy;
}

const BYTE CPesPacket::GetPtsDtsFlags(void) const
{
	// PTS DTS Flags��Ԃ�
	return m_Header.byPtsDtsFlags;
}

const bool CPesPacket::IsEscrFlag(void) const
{
	// ESCR Flag��Ԃ�
	return m_Header.bEscrFlag;
}

const bool CPesPacket::IsEsRateFlag(void) const
{
	// ES Rate Flag��Ԃ�
	return m_Header.bEsRateFlag;
}

const bool CPesPacket::IsDsmTrickModeFlag(void) const
{
	// DSM Trick Mode Flag��Ԃ�
	return m_Header.bDsmTrickModeFlag;
}

const bool CPesPacket::IsAdditionalCopyInfoFlag(void) const
{
	// Additional Copy Info Flag��Ԃ�
	return m_Header.bAdditionalCopyInfoFlag;
}

const bool CPesPacket::IsCrcFlag(void) const
{
	// PES CRC Flag��Ԃ�
	return m_Header.bCrcFlag;
}

const bool CPesPacket::IsExtensionFlag(void) const
{
	// PES Extension Flag��Ԃ�
	return m_Header.bExtensionFlag;
}

const BYTE CPesPacket::GetHeaderDataLength(void) const
{
	// PES Header Data Length��Ԃ�
	return m_Header.byHeaderDataLength;
}

const LONGLONG CPesPacket::GetPtsCount(void)const
{
	// PTS(Presentation Time Stamp)��Ԃ�
	if(m_Header.byPtsDtsFlags){
		return HexToTimeStamp(&m_pData[9]);
		}
	
	// �G���[(PTS���Ȃ�)
	return -1LL;
}

const WORD CPesPacket::GetPacketCrc(void) const
{
	// PES Packet CRC��Ԃ�
	DWORD dwCrcPos = 9UL;
	
	// �ʒu���v�Z
	if(m_Header.byPtsDtsFlags == 2U)dwCrcPos += 5UL;
	if(m_Header.byPtsDtsFlags == 3U)dwCrcPos += 10UL;
	if(m_Header.bEscrFlag)dwCrcPos += 6UL;
	if(m_Header.bEsRateFlag)dwCrcPos += 3UL;
	if(m_Header.bDsmTrickModeFlag)dwCrcPos += 1UL;
	if(m_Header.bAdditionalCopyInfoFlag)dwCrcPos += 1UL;

	if(m_dwDataSize < (dwCrcPos + 2UL))return 0x0000U;

	return ((WORD)m_pData[dwCrcPos] << 8) | (WORD)m_pData[dwCrcPos + 1];
}

BYTE * CPesPacket::GetPayloadData(void) const
{
	// �y�C���[�h�|�C���^��Ԃ�
	const DWORD dwPayloadPos = m_Header.byHeaderDataLength + 9UL;

	return (m_dwDataSize >= (dwPayloadPos + 1UL))? &m_pData[dwPayloadPos] : NULL;
}

const DWORD CPesPacket::GetPayloadSize(void) const
{
	// �y�C���[�h�T�C�Y��Ԃ�(���ۂ̕ێ����Ă�@���p�P�b�g����菭�Ȃ��Ȃ邱�Ƃ�����)
	const DWORD dwHeaderSize = m_Header.byHeaderDataLength + 9UL;

	return (m_dwDataSize > dwHeaderSize)? (m_dwDataSize - dwHeaderSize) : 0UL;
}

inline const LONGLONG CPesPacket::HexToTimeStamp(const BYTE *pHexData)
{
	// 33bit 90KHz �^�C���X�^���v����͂���
	LONGLONG llCurPtsCount = 0LL;
	llCurPtsCount |= (LONGLONG)(pHexData[0] & 0x0EU) << 29;
	llCurPtsCount |= (LONGLONG)pHexData[1] << 22;
	llCurPtsCount |= (LONGLONG)(pHexData[2] & 0xFEU) << 14;
	llCurPtsCount |= (LONGLONG)pHexData[3] << 7;
	llCurPtsCount |= (LONGLONG)pHexData[4] >> 1;

	return llCurPtsCount;
}


//////////////////////////////////////////////////////////////////////
// CPesParser�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////

CPesParser::CPesParser(IPacketHandler *pPacketHandler)
	: m_pPacketHandler(pPacketHandler)
	, m_PesPacket(0x10005UL)
	, m_bIsStoring(false)
	, m_wStoreCrc(0x0000U)
	, m_dwStoreSize(0UL)
{

}

CPesParser::CPesParser(const CPesParser &Operand)
{
	*this = Operand;
}

CPesParser & CPesParser::operator = (const CPesParser &Operand)
{
	// �C���X�^���X�̃R�s�[
	m_pPacketHandler = Operand.m_pPacketHandler;
	m_PesPacket = Operand.m_PesPacket;
	m_bIsStoring = Operand.m_bIsStoring;
	m_wStoreCrc = Operand.m_wStoreCrc;

	return *this;
}

const bool CPesParser::StorePacket(const CTsPacket *pPacket)
{
	const BYTE *pData = pPacket->GetPayloadData();
	const BYTE bySize = pPacket->GetPayloadSize();
	if(!bySize || !pData)return false;

	bool bTrigger = false;
	BYTE byPos = 0U;

	if(pPacket->m_Header.bPayloadUnitStartIndicator){
		// �w�b�_�擪 + [�y�C���[�h�f��]

		// PES�p�P�b�g���E�Ȃ��̃X�g�A����������
		if(m_bIsStoring && !m_PesPacket.GetPacketLength()){
			OnPesPacket(&m_PesPacket);
			}

		m_bIsStoring = false;
		bTrigger = true;
		m_PesPacket.ClearSize();
			
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}
	else{
		// [�w�b�_�f��] + �y�C���[�h + [�X�^�b�t�B���O�o�C�g]
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}

	return bTrigger;
}

void CPesParser::Reset(void)
{
	// ��Ԃ�����������
	m_PesPacket.Reset();
	m_bIsStoring = false;
	m_dwStoreSize = 0UL;
}

void CPesParser::OnPesPacket(const CPesPacket *pPacket) const
{
	// �n���h���Ăяo��
	if(m_pPacketHandler)m_pPacketHandler->OnPesPacket(this, pPacket);
}

const BYTE CPesParser::StoreHeader(const BYTE *pPayload, const BYTE byRemain)
{
	// �w�b�_����͂��ăZ�N�V�����̃X�g�A���J�n����
	if(m_bIsStoring)return 0U;

	const BYTE byHeaderRemain = 9U - (BYTE)m_PesPacket.GetSize();

	if(byRemain >= byHeaderRemain){
		// �w�b�_�X�g�A�����A�w�b�_����͂��ăy�C���[�h�̃X�g�A���J�n����
		m_PesPacket.AddData(pPayload, byHeaderRemain);
		if(m_PesPacket.ParseHeader()){
			// �w�b�_�t�H�[�}�b�gOK
			m_dwStoreSize = m_PesPacket.GetPacketLength();
			if(m_dwStoreSize)m_dwStoreSize += 6UL;
			m_bIsStoring = true;
			return byHeaderRemain;
			}
		else{
			// �w�b�_�G���[
			m_PesPacket.Reset();
			return byRemain;
			}
		}
	else{
		// �w�b�_�X�g�A�������A���̃f�[�^��҂�
		m_PesPacket.AddData(pPayload, byRemain);
		return byRemain;
		}
}

const BYTE CPesParser::StorePayload(const BYTE *pPayload, const BYTE byRemain)
{
	// �Z�N�V�����̃X�g�A����������
	if(!m_bIsStoring)return 0U;
	
	const DWORD dwStoreRemain = m_dwStoreSize - m_PesPacket.GetSize();

	if(m_dwStoreSize && (dwStoreRemain <= (DWORD)byRemain)){
		// �X�g�A����
		m_PesPacket.AddData(pPayload, dwStoreRemain);
				
		// CRC����A�R�[���o�b�N�ɃZ�N�V������n��
		OnPesPacket(&m_PesPacket);
		
		// ��Ԃ����������A���̃Z�N�V������M�ɔ�����
		m_PesPacket.Reset();
		m_bIsStoring = false;

		return (BYTE)dwStoreRemain;
		}
	else{
		// �X�g�A�������A���̃y�C���[�h��҂�
		m_PesPacket.AddData(pPayload, byRemain);
		return byRemain;
		}
}

const WORD CPesParser::CalcCrc(const BYTE *pData, const WORD wDataSize, WORD wCurCrc)
{
	// CRC16�v�Z(ISO/IEC 13818-1 ����)
	static const WORD CrcTable[256] = {
		0x0000U, 0x8005U, 0x800FU, 0x000AU, 0x801BU, 0x001EU, 0x0014U, 0x8011U, 0x8033U, 0x0036U, 0x003CU, 0x8039U, 0x0028U, 0x802DU, 0x8027U, 0x0022U,
		0x8063U, 0x0066U, 0x006CU, 0x8069U, 0x0078U, 0x807DU, 0x8077U, 0x0072U, 0x0050U, 0x8055U, 0x805FU, 0x005AU, 0x804BU, 0x004EU, 0x0044U, 0x8041U,
		0x80C3U, 0x00C6U, 0x00CCU, 0x80C9U, 0x00D8U, 0x80DDU, 0x80D7U, 0x00D2U, 0x00F0U, 0x80F5U, 0x80FFU, 0x00FAU, 0x80EBU, 0x00EEU, 0x00E4U, 0x80E1U,
		0x00A0U, 0x80A5U, 0x80AFU, 0x00AAU, 0x80BBU, 0x00BEU, 0x00B4U, 0x80B1U, 0x8093U, 0x0096U, 0x009CU, 0x8099U, 0x0088U, 0x808DU, 0x8087U, 0x0082U,
		0x8183U, 0x0186U, 0x018CU, 0x8189U, 0x0198U, 0x819DU, 0x8197U, 0x0192U, 0x01B0U, 0x81B5U, 0x81BFU, 0x01BAU, 0x81ABU, 0x01AEU, 0x01A4U, 0x81A1U,
		0x01E0U, 0x81E5U, 0x81EFU, 0x01EAU, 0x81FBU, 0x01FEU, 0x01F4U, 0x81F1U, 0x81D3U, 0x01D6U, 0x01DCU, 0x81D9U, 0x01C8U, 0x81CDU, 0x81C7U, 0x01C2U,
		0x0140U, 0x8145U, 0x814FU, 0x014AU, 0x815BU, 0x015EU, 0x0154U, 0x8151U, 0x8173U, 0x0176U, 0x017CU, 0x8179U, 0x0168U, 0x816DU, 0x8167U, 0x0162U,
		0x8123U, 0x0126U, 0x012CU, 0x8129U, 0x0138U, 0x813DU, 0x8137U, 0x0132U, 0x0110U, 0x8115U, 0x811FU, 0x011AU, 0x810BU, 0x010EU, 0x0104U, 0x8101U,
		0x8303U, 0x0306U, 0x030CU, 0x8309U, 0x0318U, 0x831DU, 0x8317U, 0x0312U, 0x0330U, 0x8335U, 0x833FU, 0x033AU, 0x832BU, 0x032EU, 0x0324U, 0x8321U,
		0x0360U, 0x8365U, 0x836FU, 0x036AU, 0x837BU, 0x037EU, 0x0374U, 0x8371U, 0x8353U, 0x0356U, 0x035CU, 0x8359U, 0x0348U, 0x834DU, 0x8347U, 0x0342U,
		0x03C0U, 0x83C5U, 0x83CFU, 0x03CAU, 0x83DBU, 0x03DEU, 0x03D4U, 0x83D1U, 0x83F3U, 0x03F6U, 0x03FCU, 0x83F9U, 0x03E8U, 0x83EDU, 0x83E7U, 0x03E2U,
		0x83A3U, 0x03A6U, 0x03ACU, 0x83A9U, 0x03B8U, 0x83BDU, 0x83B7U, 0x03B2U, 0x0390U, 0x8395U, 0x839FU, 0x039AU, 0x838BU, 0x038EU, 0x0384U, 0x8381U,
		0x0280U, 0x8285U, 0x828FU, 0x028AU, 0x829BU, 0x029EU, 0x0294U, 0x8291U, 0x82B3U, 0x02B6U, 0x02BCU, 0x82B9U, 0x02A8U, 0x82ADU, 0x82A7U, 0x02A2U,
		0x82E3U, 0x02E6U, 0x02ECU, 0x82E9U, 0x02F8U, 0x82FDU, 0x82F7U, 0x02F2U, 0x02D0U, 0x82D5U, 0x82DFU, 0x02DAU, 0x82CBU, 0x02CEU, 0x02C4U, 0x82C1U,
		0x8243U, 0x0246U, 0x024CU, 0x8249U, 0x0258U, 0x825DU, 0x8257U, 0x0252U, 0x0270U, 0x8275U, 0x827FU, 0x027AU, 0x826BU, 0x026EU, 0x0264U, 0x8261U,
		0x0220U, 0x8225U, 0x822FU, 0x022AU, 0x823BU, 0x023EU, 0x0234U, 0x8231U, 0x8213U, 0x0216U, 0x021CU, 0x8219U, 0x0208U, 0x820DU, 0x8207U, 0x0202U
		};		

	for(WORD wPos = 0 ; wPos < wDataSize ; wPos++){
		wCurCrc = (wCurCrc << 8) ^ CrcTable[ (wCurCrc >> 8) ^ pData[wPos] ];
		}

	return wCurCrc;
}

//2010.05.07 fuji �I������
void CPesParser::Close(void ){
	
	if (m_PesPacket.GetSize() > 0) 
		OnPesPacket(&m_PesPacket);

}

//////////////////////////////////////////////////////////////////////
// CAdtsFrame�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////

CAdtsFrame::CAdtsFrame()
	: CMediaData()
{
	Reset();
}

CAdtsFrame::CAdtsFrame(const CAdtsFrame &Operand)
{
	Reset();

	*this = Operand;
}

CAdtsFrame & CAdtsFrame::operator = (const CAdtsFrame &Operand)
{
	// �C���X�^���X�̃R�s�[
	CMediaData::operator = (Operand);
	m_Header = Operand.m_Header;

	return *this;
}

const bool CAdtsFrame::ParseHeader(void)
{
	// adts_fixed_header()
	if(m_dwDataSize < 7UL)return false;									// ADTS�w�b�_��7�o�C�g
	else if(m_pData[0] != 0xFFU || m_pData[1] != 0xF8U)return false;	// Syncword�AID�Alayer�Aprotection_absent�ُ�@��CRC�Ȃ��͔�Ή�
	
	m_Header.byProfile				= (m_pData[2] & 0xC0U) >> 6;									// +2 bit7-6
	m_Header.bySamplingFreqIndex	= (m_pData[2] & 0x3CU) >> 2;									// +2 bit5-2
	m_Header.bPrivateBit			= (m_pData[2] & 0x02U)? true : false;							// +2 bit1
	m_Header.byChannelConfig		= ((m_pData[2] & 0x01U) << 2) | ((m_pData[3] & 0xC0U) >> 6);	// +3 bit0, +4 bit7-6
	m_Header.bOriginalCopy			= (m_pData[3] & 0x20U)? true : false;							// +3 bit5
	m_Header.bHome					= (m_pData[3] & 0x10U)? true : false;							// +3 bit4

	// adts_variable_header()
	m_Header.bCopyrightIdBit		= (m_pData[3] & 0x08U)? true : false;							// +3 bit3
	m_Header.bCopyrightIdStart		= (m_pData[3] & 0x04U)? true : false;							// +3 bit2
	m_Header.wFrameLength			= ((WORD)(m_pData[3] & 0x03U) << 11) | ((WORD)m_pData[4] << 3) | ((WORD)(m_pData[5] & 0xE0U) >> 5);
	m_Header.wBufferFullness		= ((WORD)(m_pData[5] & 0x1FU) << 6) | ((WORD)(m_pData[6] & 0xFCU) >> 2);
	m_Header.byRawDataBlockNum		= m_pData[6] & 0x03U;

	// �t�H�[�}�b�g�K�����`�F�b�N
	if(m_Header.byProfile == 3U)return false;							// ����`�̃v���t�@�C��
	else if(m_Header.bySamplingFreqIndex > 0x0BU)return false;			// ����`�̃T���v�����O���g��
	else if(m_Header.wFrameLength < 2U)return false;					// �f�[�^�Ȃ��̏ꍇ���Œ�CRC�̃T�C�Y���K�v
	else if(m_Header.byRawDataBlockNum)return false;					// �{�N���X�͒P���Raw Data Block�ɂ����Ή����Ȃ�

	return true;
}

void CAdtsFrame::Reset(void)
{
	// �f�[�^���N���A����
	ClearSize();	
	::ZeroMemory(&m_Header, sizeof(m_Header));
}

const BYTE CAdtsFrame::GetProfile(void) const
{
	// Profile ��Ԃ�
	return m_Header.byProfile;
}

const BYTE CAdtsFrame::GetSamplingFreqIndex(void) const
{
	// Sampling Frequency Index ��Ԃ�
	return m_Header.bySamplingFreqIndex;
}

const bool CAdtsFrame::IsPrivateBit(void) const
{
	// Private Bit ��Ԃ�
	return m_Header.bPrivateBit;
}

const BYTE CAdtsFrame::GetChannelConfig(void) const
{
	// Channel Configuration ��Ԃ�
	return m_Header.byChannelConfig;
}

const bool CAdtsFrame::IsOriginalCopy(void) const
{
	// Original/Copy ��Ԃ�
	return m_Header.bOriginalCopy;
}

const bool CAdtsFrame::IsHome(void) const
{
	// Home ��Ԃ�
	return m_Header.bHome;
}

const bool CAdtsFrame::IsCopyrightIdBit(void) const
{
	// Copyright Identification Bit ��Ԃ�
	return m_Header.bCopyrightIdBit;
}

const bool CAdtsFrame::IsCopyrightIdStart(void) const
{
	// Copyright Identification Start ��Ԃ�
	return m_Header.bCopyrightIdStart;
}

const WORD CAdtsFrame::GetFrameLength(void) const
{
	// Frame Length ��Ԃ�
	return m_Header.wFrameLength;
}

const WORD CAdtsFrame::GetBufferFullness(void) const
{
	// ADTS Buffer Fullness ��Ԃ�
	return m_Header.wBufferFullness;
}

const BYTE CAdtsFrame::GetRawDataBlockNum(void) const
{
	// Number of Raw Data Blocks in Frame ��Ԃ�
	return m_Header.byRawDataBlockNum;
}


//////////////////////////////////////////////////////////////////////
// CAdtsParser�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////

CAdtsParser::CAdtsParser(IFrameHandler *pFrameHandler)
	: m_pFrameHandler(pFrameHandler)
{
	// ADTS�t���[���ő咷�̃o�b�t�@�m��
	m_AdtsFrame.GetBuffer(0x2000UL);

	Reset();
}

CAdtsParser::CAdtsParser(const CAdtsParser &Operand)
{
	*this = Operand;
}

CAdtsParser & CAdtsParser::operator = (const CAdtsParser &Operand)
{
	// �C���X�^���X�̃R�s�[
	m_pFrameHandler = Operand.m_pFrameHandler;
	m_AdtsFrame = Operand.m_AdtsFrame;
	m_bIsStoring = Operand.m_bIsStoring;
	m_wStoreCrc = Operand.m_wStoreCrc;

	return *this;
}

const bool CAdtsParser::StorePacket(const CPesPacket *pPacket)
{
	return StoreEs(pPacket->GetPayloadData(), pPacket->GetPayloadSize());
}

const bool CAdtsParser::StoreEs(const BYTE *pData, const DWORD dwSize)
{
	bool bTrigger = false;
	DWORD dwPos = 0UL;

	if(!dwSize || !dwSize)return bTrigger;

	while(dwPos < dwSize){
		if(!m_bIsStoring){
			// �w�b�_����������
			m_bIsStoring = SyncFrame(pData[dwPos++]);
			if(m_bIsStoring)bTrigger = true;
			}
		else{
			// �f�[�^���X�g�A����
			const DWORD dwStoreRemain = m_AdtsFrame.GetFrameLength() - (WORD)m_AdtsFrame.GetSize();
			const DWORD dwDataRemain = dwSize - dwPos;
			
			if(dwStoreRemain <= dwDataRemain){
				// �X�g�A����
				m_AdtsFrame.AddData(&pData[dwPos], dwStoreRemain);
				dwPos += dwStoreRemain;
				m_bIsStoring = false;
				
				// �{���Ȃ炱����CRC�`�F�b�N�����ׂ�
				// �`�F�b�N�Ώۗ̈悪�ςŕ��G�Ȃ̂ŕۗ��A�N���������܂���...

				// �t���[���o��
				OnAdtsFrame(&m_AdtsFrame);
				
				// ���̃t���[�����������邽�߃��Z�b�g
				m_AdtsFrame.ClearSize();
				}
			else{
				// �X�g�A�������A���̃y�C���[�h��҂�
				m_AdtsFrame.AddData(&pData[dwPos], dwDataRemain);
				dwPos += dwDataRemain;
				}
			}		
		}

	return bTrigger;
}

void CAdtsParser::Reset(void)
{
	// ��Ԃ�����������
	m_bIsStoring = false;
	m_AdtsFrame.Reset();
}

//2010.05.07 fuji �I������
void CAdtsParser::Close(void)
{
	if(m_AdtsFrame.GetSize() > 0) 
	{	
		OnAdtsFrame(&m_AdtsFrame);
		m_AdtsFrame.Reset();			
	}
}

void CAdtsParser::OnPesPacket(const CPesParser *pPesParser, const CPesPacket *pPacket)
{
	// CPesParser::IPacketHandler�C���^�t�F�[�X�̎���
	StorePacket(pPacket);
}

void CAdtsParser::OnAdtsFrame(const CAdtsFrame *pFrame) const
{
	// �n���h���Ăяo��
	if(m_pFrameHandler)m_pFrameHandler->OnAdtsFrame(this, pFrame);
}

inline const bool CAdtsParser::SyncFrame(const BYTE byData)
{
	switch(m_AdtsFrame.GetSize()){
		case 0UL :
			// syncword(8bit)
			if(byData == 0xFFU)m_AdtsFrame.AddByte(byData);
			break;

		case 1UL :
			// syncword(4bit), ID, layer, protection_absent	��CRC�t���̃t���[���̂ݑΉ�
			if(byData == 0xF8U)m_AdtsFrame.AddByte(byData);
			else m_AdtsFrame.ClearSize();
			break;

		case 2UL :
		case 3UL :
		case 4UL :
		case 5UL :
			// adts_fixed_header() - adts_variable_header()
			m_AdtsFrame.AddByte(byData);
			break;

		case 6UL :
			// �w�b�_���S�Ă������
			m_AdtsFrame.AddByte(byData);

			// �w�b�_����͂���
			if(m_AdtsFrame.ParseHeader())return true;
			else m_AdtsFrame.ClearSize();
			break;
		
		default:
			// ��O
			m_AdtsFrame.ClearSize();
			break;
		}

	return false;
}

const WORD CAdtsParser::CalcCrc(const BYTE *pData, const WORD wDataSize, WORD wCurCrc)
{
	// CRC16�v�Z(ISO/IEC 11172-3 ����)
	static const WORD CrcTable[256] = {
		0x0000U, 0x8005U, 0x800FU, 0x000AU, 0x801BU, 0x001EU, 0x0014U, 0x8011U, 0x8033U, 0x0036U, 0x003CU, 0x8039U, 0x0028U, 0x802DU, 0x8027U, 0x0022U,
		0x8063U, 0x0066U, 0x006CU, 0x8069U, 0x0078U, 0x807DU, 0x8077U, 0x0072U, 0x0050U, 0x8055U, 0x805FU, 0x005AU, 0x804BU, 0x004EU, 0x0044U, 0x8041U,
		0x80C3U, 0x00C6U, 0x00CCU, 0x80C9U, 0x00D8U, 0x80DDU, 0x80D7U, 0x00D2U, 0x00F0U, 0x80F5U, 0x80FFU, 0x00FAU, 0x80EBU, 0x00EEU, 0x00E4U, 0x80E1U,
		0x00A0U, 0x80A5U, 0x80AFU, 0x00AAU, 0x80BBU, 0x00BEU, 0x00B4U, 0x80B1U, 0x8093U, 0x0096U, 0x009CU, 0x8099U, 0x0088U, 0x808DU, 0x8087U, 0x0082U,
		0x8183U, 0x0186U, 0x018CU, 0x8189U, 0x0198U, 0x819DU, 0x8197U, 0x0192U, 0x01B0U, 0x81B5U, 0x81BFU, 0x01BAU, 0x81ABU, 0x01AEU, 0x01A4U, 0x81A1U,
		0x01E0U, 0x81E5U, 0x81EFU, 0x01EAU, 0x81FBU, 0x01FEU, 0x01F4U, 0x81F1U, 0x81D3U, 0x01D6U, 0x01DCU, 0x81D9U, 0x01C8U, 0x81CDU, 0x81C7U, 0x01C2U,
		0x0140U, 0x8145U, 0x814FU, 0x014AU, 0x815BU, 0x015EU, 0x0154U, 0x8151U, 0x8173U, 0x0176U, 0x017CU, 0x8179U, 0x0168U, 0x816DU, 0x8167U, 0x0162U,
		0x8123U, 0x0126U, 0x012CU, 0x8129U, 0x0138U, 0x813DU, 0x8137U, 0x0132U, 0x0110U, 0x8115U, 0x811FU, 0x011AU, 0x810BU, 0x010EU, 0x0104U, 0x8101U,
		0x8303U, 0x0306U, 0x030CU, 0x8309U, 0x0318U, 0x831DU, 0x8317U, 0x0312U, 0x0330U, 0x8335U, 0x833FU, 0x033AU, 0x832BU, 0x032EU, 0x0324U, 0x8321U,
		0x0360U, 0x8365U, 0x836FU, 0x036AU, 0x837BU, 0x037EU, 0x0374U, 0x8371U, 0x8353U, 0x0356U, 0x035CU, 0x8359U, 0x0348U, 0x834DU, 0x8347U, 0x0342U,
		0x03C0U, 0x83C5U, 0x83CFU, 0x03CAU, 0x83DBU, 0x03DEU, 0x03D4U, 0x83D1U, 0x83F3U, 0x03F6U, 0x03FCU, 0x83F9U, 0x03E8U, 0x83EDU, 0x83E7U, 0x03E2U,
		0x83A3U, 0x03A6U, 0x03ACU, 0x83A9U, 0x03B8U, 0x83BDU, 0x83B7U, 0x03B2U, 0x0390U, 0x8395U, 0x839FU, 0x039AU, 0x838BU, 0x038EU, 0x0384U, 0x8381U,
		0x0280U, 0x8285U, 0x828FU, 0x028AU, 0x829BU, 0x029EU, 0x0294U, 0x8291U, 0x82B3U, 0x02B6U, 0x02BCU, 0x82B9U, 0x02A8U, 0x82ADU, 0x82A7U, 0x02A2U,
		0x82E3U, 0x02E6U, 0x02ECU, 0x82E9U, 0x02F8U, 0x82FDU, 0x82F7U, 0x02F2U, 0x02D0U, 0x82D5U, 0x82DFU, 0x02DAU, 0x82CBU, 0x02CEU, 0x02C4U, 0x82C1U,
		0x8243U, 0x0246U, 0x024CU, 0x8249U, 0x0258U, 0x825DU, 0x8257U, 0x0252U, 0x0270U, 0x8275U, 0x827FU, 0x027AU, 0x826BU, 0x026EU, 0x0264U, 0x8261U,
		0x0220U, 0x8225U, 0x822FU, 0x022AU, 0x823BU, 0x023EU, 0x0234U, 0x8231U, 0x8213U, 0x0216U, 0x021CU, 0x8219U, 0x0208U, 0x820DU, 0x8207U, 0x0202U
		};		

	for(WORD wPos = 0U ; wPos < wDataSize ; wPos++){
		wCurCrc = (wCurCrc << 8) ^ CrcTable[ (wCurCrc >> 8) ^ pData[wPos] ];
		}

	return wCurCrc;
}


//////////////////////////////////////////////////////////////////////
// CMpeg2Sequence�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////

CMpeg2Sequence::CMpeg2Sequence()
	: CMediaData()
{
	Reset();
}

CMpeg2Sequence::CMpeg2Sequence(const CMpeg2Sequence &Operand)
{
	Reset();

	*this = Operand;
}

CMpeg2Sequence & CMpeg2Sequence::operator = (const CMpeg2Sequence &Operand)
{
	// �C���X�^���X�̃R�s�[
	CMediaData::operator = (Operand);
	m_Header = Operand.m_Header;

	return *this;
}

const bool CMpeg2Sequence::ParseHeader(void)
{
	// �����ł�Start Code Prifix��Start Code�����`�F�b�N���Ȃ��B(�V�[�P���X�̓����݂̂�ړI�Ƃ���)

	// next_start_code()
	if(m_dwDataSize < 12UL)return false;
	else if(m_pData[0] || m_pData[1] || m_pData[2] != 0x01U || m_pData[3] != 0xB3U)return false;					// +0,+1,+2,+3
	
	m_Header.wHorizontalSize			= ((WORD)m_pData[4] << 4) | ((WORD)(m_pData[5] & 0xF0U) >> 4);				// +4,+5 bit7-4
	m_Header.wVerticalSize				= ((WORD)(m_pData[5] & 0x0FU) << 8) | (WORD)m_pData[6];						// +5 bit3-0, +6
	m_Header.byAspectRatioInfo			= (m_pData[7] & 0xF0U) >> 4;												// +7 bit7-4
	m_Header.byFrameRateCode			= m_pData[7] & 0x0FU;														// +7 bit3-0
	m_Header.dwBitRate					= ((DWORD)m_pData[8] << 10) | ((DWORD)m_pData[9] << 2) | ((DWORD)(m_pData[10] & 0xC0U) >> 6);	// +8, +9, +10 bit7-6
	m_Header.bMarkerBit					= (m_pData[10] & 0x20U)? true : false;										// +10 bit5
	m_Header.wVbvBufferSize				= ((WORD)(m_pData[10] & 0x1FU) << 5) | ((WORD)(m_pData[11] & 0xF8U) >> 3);	// +10 bit4-0, +11 bit7-3
	m_Header.bConstrainedParamFlag		= (m_pData[11] & 0x04U)? true : false;										// +11 bit2
	m_Header.bLoadIntraQuantiserMatrix	= (m_pData[11] & 0x02U)? true : false;										// +11 bit1

	// �t�H�[�}�b�g�K�����`�F�b�N
	if(!m_Header.byAspectRatioInfo || m_Header.byAspectRatioInfo > 4U)return false;		// �A�X�y�N�g�䂪�ُ�
	else if(!m_Header.byFrameRateCode || m_Header.byFrameRateCode > 8U)return false;	// �t���[�����[�g���ُ�
	else if(!m_Header.bMarkerBit)return false;											// �}�[�J�[�r�b�g���ُ�
	else if(m_Header.bConstrainedParamFlag)return false;								// Constrained Parameters Flag ���ُ�

	return true;
}

void CMpeg2Sequence::Reset(void)
{
	// �f�[�^���N���A����
	ClearSize();	
	::ZeroMemory(&m_Header, sizeof(m_Header));
}

const WORD CMpeg2Sequence::GetHorizontalSize(void) const
{
	// Horizontal Size Value ��Ԃ�
	return m_Header.wHorizontalSize;
}

const WORD CMpeg2Sequence::GetVerticalSize(void) const
{
	// Vertical Size Value ��Ԃ�
	return m_Header.wVerticalSize;
}

const BYTE CMpeg2Sequence::GetAspectRatioInfo(void) const
{
	// Aspect Ratio Information ��Ԃ�
	return m_Header.byAspectRatioInfo;
}

const BYTE CMpeg2Sequence::GetFrameRateCode(void) const
{
	// Frame Rate Code ��Ԃ�
	return m_Header.byFrameRateCode;
}

const DWORD CMpeg2Sequence::GetBitRate(void) const
{
	// Bit Rate Value ��Ԃ�
	return m_Header.dwBitRate;
}

const bool CMpeg2Sequence::IsMarkerBit(void) const
{
	// Marker Bit ��Ԃ�
	return m_Header.bMarkerBit;
}

const WORD CMpeg2Sequence::GetVbvBufferSize(void) const
{
	// VBV Buffer Size Value ��Ԃ�
	return m_Header.wVbvBufferSize;
}

const bool CMpeg2Sequence::IsConstrainedParamFlag(void) const
{
	// Constrained Parameters Flag ��Ԃ�
	return m_Header.bConstrainedParamFlag;
}

const bool CMpeg2Sequence::IsLoadIntraQuantiserMatrix(void) const
{
	// Load Intra Quantiser Matrix ��Ԃ�
	return m_Header.bLoadIntraQuantiserMatrix;
}


//////////////////////////////////////////////////////////////////////
// CMpeg2Parser�N���X�̍\�z/����
//////////////////////////////////////////////////////////////////////

CMpeg2Parser::CMpeg2Parser(ISequenceHandler *pSequenceHandler)
	: m_pSequenceHandler(pSequenceHandler)
{
	Reset();
	m_dwTotalFrame = 0;
	m_dwFrameCount = 0;
}

CMpeg2Parser::CMpeg2Parser(const CMpeg2Parser &Operand)
{
	*this = Operand;
}

CMpeg2Parser & CMpeg2Parser::operator = (const CMpeg2Parser &Operand)
{
	// �C���X�^���X�̃R�s�[
	m_pSequenceHandler = Operand.m_pSequenceHandler;
	m_Mpeg2Sequence = Operand.m_Mpeg2Sequence;
	m_bIsStoring = Operand.m_bIsStoring;
	m_dwSyncState = Operand.m_dwSyncState;

	return *this;
}

const bool CMpeg2Parser::StorePacket(const CPesPacket *pPacket)
{
	const BYTE *pData = pPacket->GetPayloadData();
	const DWORD dwSize = pPacket->GetPayloadSize();
	static const BYTE StartCode[] = {0x00U, 0x00U, 0x01U, 0xB3U};

	bool bTrigger = false;
	DWORD dwPos = 0UL, dwStart;

	while(dwPos < dwSize){
		// �X�^�[�g�R�[�h����������
		dwStart = FindStartCode(&pData[dwPos], dwSize - dwPos);
	
		if(dwStart < (dwSize - dwPos)){
			dwStart++;
			
			if(m_Mpeg2Sequence.GetSize() >= 4UL){

				// �X�^�[�g�R�[�h�̒f�Ђ���菜��
				if(dwStart < 4)m_Mpeg2Sequence.TrimTail(4UL - dwStart);

				{	// Frame���J�E���g
					DWORD i;
					BYTE* data = m_Mpeg2Sequence.GetData();
					DWORD len = m_Mpeg2Sequence.GetSize() - 4;

					for(i = 0 ; i < len ; i ++){
						if(data[i] == 0x00 && data[i+1] == 0x00 && data[i+2] == 0x01 && data[i+3] == 0x00 ){
							m_dwTotalFrame++;
						}
					}
				}

				// �V�[�P���X���o�͂���
				if(m_Mpeg2Sequence.ParseHeader())OnMpeg2Sequence(&m_Mpeg2Sequence);
				}
				
			// �X�^�[�g�R�[�h���Z�b�g����		
			m_Mpeg2Sequence.SetData(StartCode, 4UL);


			bTrigger = true;
			}
		else  if(m_Mpeg2Sequence.GetSize() >= 4UL){
			// �V�[�P���X�X�g�A
			if(m_Mpeg2Sequence.AddData(&pData[dwPos], dwSize - dwPos) >= 0x1000000UL){
				// ��O(�V�[�P���X��16MB�𒴂���)
				m_Mpeg2Sequence.ClearSize();
				}
			}

		// �|�W�V�����X�V
		dwPos += dwStart;
		}

	return bTrigger;
}

void CMpeg2Parser::UpdateTotalFrame(void)
{
//	m_dwTotalFrame += m_dwFrameCount;
//	m_dwFrameCount = 0;
}


void CMpeg2Parser::Reset(void)
{
	// ��Ԃ�����������
	m_bIsStoring = false;
	m_dwSyncState = 0xFFFFFFFFUL;

	m_Mpeg2Sequence.Reset();
}

void CMpeg2Parser::OnPesPacket(const CPesParser *pPesParser, const CPesPacket *pPacket)
{
	// CPesParser::IPacketHandler�C���^�t�F�[�X�̎���
	StorePacket(pPacket);
}

void CMpeg2Parser::OnMpeg2Sequence(const CMpeg2Sequence *pSequence) const
{
	// �n���h���Ăяo��
	if(m_pSequenceHandler)m_pSequenceHandler->OnMpeg2Sequence(this, pSequence);
}

inline const DWORD CMpeg2Parser::FindStartCode(const BYTE *pData, const DWORD dwDataSize)
{
	// Sequence Header Code (0x000001B3) ����������
	DWORD dwPos;

	for(dwPos = 0UL ; dwPos < dwDataSize ; dwPos++){
		m_dwSyncState <<= 8;
		m_dwSyncState |= (DWORD)pData[dwPos];

		if(m_dwSyncState == 0x000001B3UL){
			// �X�^�[�g�R�[�h�����A�V�t�g���W�X�^������������
			m_dwSyncState = 0xFFFFFFFFUL;
			break;
			}
		}

	return dwPos;
}
//2010.05.07 fuji �I������
void CMpeg2Parser::Close(void){

	//Close���ɂ��ׂēf���o���B
	if(m_Mpeg2Sequence.ParseHeader())OnMpeg2Sequence(&m_Mpeg2Sequence);

}