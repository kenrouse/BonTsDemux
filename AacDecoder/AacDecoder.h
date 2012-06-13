// AacDecoder.h: CAacDecoder クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "Faad.h"
#include "TsMedia.h"


// FAAD2 AACデコーダラッパークラス 
//
// "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
//


class CAacDecoder  
{
public:
	// L-PCMフレーム取得コールバック関数型
	typedef void (CALLBACK * ONLPCMFRAMEPROC)(const BYTE *pData, const DWORD dwSamples, const BYTE byChannel, PVOID pParam);

	CAacDecoder();
	virtual ~CAacDecoder();

	const bool OpenDecoder(ONLPCMFRAMEPROC pOnLpcmFrameProc, PVOID pParam = NULL);
	void CloseDecoder(void);

	const bool ResetDecoder(void);

	const bool Decode(const CAdtsFrame *pFrame);

private:
	
	faacDecHandle m_hDecoder;

	bool m_InitRequest;
	BYTE m_byLastChannelConfig;

	ONLPCMFRAMEPROC m_OnLpcmFrameProc;
	PVOID m_pParam;

	faacDecFrameInfo m_LastFrameInfo;

};
