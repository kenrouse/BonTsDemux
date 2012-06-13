// TsPacketParser.h: CTsPacketParser クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsStream.h"


/////////////////////////////////////////////////////////////////////////////
// TSパケット抽出デコーダ(バイナリデータからTSパケットを抽出する)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData	TSパケットを含むバイナリデータ
// Output	#0	: CTsPacket		TSパケット
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
