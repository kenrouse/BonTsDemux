// TsPacketParser.cpp: CTsPacketParser �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsPacketParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define TS_HEADSYNCBYTE		(0x47U)


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////


CTsPacketParser::CTsPacketParser(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_bOutputNullPacket(false)
	, m_dwInputPacketCount(0UL)
	, m_dwOutputPacketCount(0UL)
	, m_dwErrorPacketCount(0UL)
{
	// �p�P�b�g�A�����J�E���^������������
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);
}

CTsPacketParser::~CTsPacketParser()
{

}

void CTsPacketParser::Reset(void)
{
	// �p�P�b�g�J�E���^���N���A����
	m_dwInputPacketCount =	0UL;
	m_dwOutputPacketCount = 0UL;
	m_dwErrorPacketCount =	0UL;

	// �p�P�b�g�A�����J�E���^������������
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);

	// ��Ԃ����Z�b�g����
	m_TsPacket.ClearSize();
	
	CMediaDecoder::Reset();
}

const DWORD CTsPacketParser::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CTsPacketParser::GetOutputNum(void) const
{
	return 1UL;
}

const bool CTsPacketParser::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;
	if(!pMediaData)return false;

	// TS�p�P�b�g����������
	SyncPacket(pMediaData->GetData(), pMediaData->GetSize());

	return true;
}

void CTsPacketParser::SetOutputNullPacket(const bool bEnable)
{
	// NULL�p�P�b�g�̏o�͗L����ݒ肷��
	m_bOutputNullPacket = (bEnable)? true : false;
}

const DWORD CTsPacketParser::GetInputPacketCount(void) const
{
	// ���̓p�P�b�g����Ԃ�
	return m_dwInputPacketCount;
}

const DWORD CTsPacketParser::GetOutputPacketCount(void) const
{
	// �o�̓p�P�b�g����Ԃ�
	return m_dwOutputPacketCount;
}

const DWORD CTsPacketParser::GetErrorPacketCount(void) const
{
	// �G���[�p�P�b�g����Ԃ�
	return m_dwErrorPacketCount;
}

void inline CTsPacketParser::SyncPacket(const BYTE *pData, const DWORD dwSize)
{
	// �����̕��@�͊��S�ł͂Ȃ��A���������ꂽ�ꍇ�ɑO��Ăяo�����̃f�[�^�܂ł����̂ڂ��Ă͍ē����͂ł��Ȃ�
	DWORD dwCurSize = 0UL;
	DWORD dwCurPos = 0UL;

	while(dwCurPos < dwSize){
		dwCurSize = m_TsPacket.GetSize();

		if(!dwCurSize){
			// �����o�C�g�҂���
			for( ; dwCurPos < dwSize ; dwCurPos++){
				if(pData[dwCurPos] == TS_HEADSYNCBYTE){
					// �����o�C�g����
					m_TsPacket.AddByte(TS_HEADSYNCBYTE);
					dwCurPos++;
					break;
					}				
				}
			
			continue;
			}
		else  if(dwCurSize == TS_PACKETSIZE){
			// �p�P�b�g�T�C�Y���f�[�^���������

			if(pData[dwCurPos] == TS_HEADSYNCBYTE){
				// ���̃f�[�^�͓����o�C�g
				ParsePacket();
				}
			else{
				// �����G���[
				m_TsPacket.ClearSize();				
				
				// �ʒu�����ɖ߂�
				if(dwCurPos >= (TS_PACKETSIZE - 1UL))dwCurPos -= (TS_PACKETSIZE - 1UL);
				else dwCurPos = 0UL;
				}

			continue;
			}
		else{
			// �f�[�^�҂�
			if((dwSize - dwCurPos) >= (TS_PACKETSIZE - dwCurSize)){
				m_TsPacket.AddData(&pData[dwCurPos], TS_PACKETSIZE - dwCurSize);
				dwCurPos += (TS_PACKETSIZE - dwCurSize);
				}	
			else{
				m_TsPacket.AddData(&pData[dwCurPos], dwSize - dwCurPos);
				dwCurPos += (dwSize - dwCurPos);
				}			
			
			continue;
			}
		}
}

void inline CTsPacketParser::ParsePacket(void)
{
	// �p�P�b�g����͂���
	m_TsPacket.ParseHeader();

	// �p�P�b�g���`�F�b�N����
	if(m_TsPacket.CheckPacket(&m_abyContCounter[m_TsPacket.GetPID()])){
		// ���̓J�E���g�C���N�������g
		if(m_dwInputPacketCount < 0xFFFFFFFFUL)m_dwInputPacketCount++;

		// ���̃f�R�[�_�Ƀf�[�^��n��
		if(m_bOutputNullPacket || (m_TsPacket.GetPID() != 0x1FFFU)){
			
			// �o�̓J�E���g�C���N�������g
			if(m_dwOutputPacketCount < 0xFFFFFFFFUL)m_dwOutputPacketCount++;
			
			OutputMedia(&m_TsPacket);
			}
		}
	else{
		// �G���[�J�E���g�C���N�������g
		if(m_dwErrorPacketCount < 0xFFFFFFFFUL)m_dwErrorPacketCount++;
		}

	// �T�C�Y���N���A�����̃X�g�A�ɔ�����
	m_TsPacket.ClearSize();
}
