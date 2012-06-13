// TsPacketParser.h: CTsPacketParser �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsStream.h"


/////////////////////////////////////////////////////////////////////////////
// TS�p�P�b�g���o�f�R�[�_(�o�C�i���f�[�^����TS�p�P�b�g�𒊏o����)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData	TS�p�P�b�g���܂ރo�C�i���f�[�^
// Output	#0	: CTsPacket		TS�p�P�b�g
/////////////////////////////////////////////////////////////////////////////

class CTsPacketParser : public CMediaDecoder  
{
public:
	CTsPacketParser(CDecoderHandler *pDecoderHandler);
	virtual ~CTsPacketParser();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CTsPacketParser
	void SetOutputNullPacket(const bool bEnable = true);

	const DWORD GetInputPacketCount(void) const;
	const DWORD GetOutputPacketCount(void) const;
	const DWORD GetErrorPacketCount(void) const;

private:
	void inline SyncPacket(const BYTE *pData, const DWORD dwSize);
	void inline ParsePacket(void);

	CTsPacket m_TsPacket;

	bool m_bOutputNullPacket;

	DWORD m_dwInputPacketCount;
	DWORD m_dwOutputPacketCount;
	DWORD m_dwErrorPacketCount;
	BYTE m_abyContCounter[0x1FFF];
};
