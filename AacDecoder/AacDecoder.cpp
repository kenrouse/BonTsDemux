// AacDecoder.cpp: CAacDecoder クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AacDecoder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
#ifdef _DEBUG
#pragma comment(lib, "LibFaadd.lib")
#else
#pragma comment(lib, "LibFaad.lib")
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CAacDecoder::CAacDecoder()
	: m_OnLpcmFrameProc(NULL)
	, m_hDecoder(NULL)
	, m_InitRequest(false)
	, m_byLastChannelConfig(0U)
{
	ZeroMemory(&m_LastFrameInfo,sizeof(m_LastFrameInfo));
}

CAacDecoder::~CAacDecoder()
{
	CloseDecoder();
}

const bool CAacDecoder::OpenDecoder(ONLPCMFRAMEPROC pOnLpcmFrameProc, PVOID pParam)
{
	CloseDecoder();

	// FAAD2オープン
	if(!(m_hDecoder = faacDecOpen()))return false;

	// デフォルト設定取得
	faacDecConfigurationPtr pDecodeConfig = faacDecGetCurrentConfiguration(m_hDecoder);
	
	if(!pDecodeConfig){
		CloseDecoder();
		return false;
		}

	// デコーダ設定
	pDecodeConfig->defSampleRate = 48000UL;
	pDecodeConfig->outputFormat = FAAD_FMT_16BIT;

	if(!faacDecSetConfiguration(m_hDecoder, pDecodeConfig)){
		CloseDecoder();
		return false;
		}
	
	m_InitRequest = true;
	m_byLastChannelConfig = 0xFFU;

	// コールバック情報保存
	m_OnLpcmFrameProc = pOnLpcmFrameProc;
	m_pParam = pParam;

	return true;
}

void CAacDecoder::CloseDecoder()
{
	m_OnLpcmFrameProc = NULL;

	// FAAD2クローズ
	if(m_hDecoder){
		faacDecClose(m_hDecoder);
		m_hDecoder = NULL;
		}
}

const bool CAacDecoder::ResetDecoder(void)
{
	if(!m_hDecoder)return false;
	
	// FAAD2クローズ
	faacDecClose(m_hDecoder);

	// FAAD2オープン
	if(!(m_hDecoder = faacDecOpen()))return false;

	// デフォルト設定取得
	faacDecConfigurationPtr pDecodeConfig = faacDecGetCurrentConfiguration(m_hDecoder);
	
	if(!pDecodeConfig){
		CloseDecoder();
		return false;
		}

	// デコーダ設定
	pDecodeConfig->defSampleRate = 48000UL;
	pDecodeConfig->outputFormat = FAAD_FMT_16BIT;

	if(!faacDecSetConfiguration(m_hDecoder, pDecodeConfig)){
		CloseDecoder();
		return false;
		}
	
	m_InitRequest = true;
	m_byLastChannelConfig = 0xFFU;

	return true;
}

const bool CAacDecoder::Decode(const CAdtsFrame *pFrame)
{
	if(!m_hDecoder)return false;

	// デコード
	DWORD dwSamples = 0UL;
	BYTE byChannels = 0U;
	BYTE retry = 1;
	
	// チャンネル設定解析
	if(pFrame->GetChannelConfig() != m_byLastChannelConfig){
		// チャンネル設定が変化した、デコーダリセット
		ResetDecoder();
		m_byLastChannelConfig = pFrame->GetChannelConfig();
	}	

	// 初回フレーム解析
	if(m_InitRequest){
		if(faacDecInit(m_hDecoder, pFrame->GetData(), pFrame->GetSize(), &dwSamples, &byChannels) < 0){
			return false;
		}
		
		m_InitRequest = false;
	}
	
	// デコード
	faacDecFrameInfo FrameInfo;
	::ZeroMemory(&FrameInfo, sizeof(FrameInfo));
RETRY:
	BYTE *pPcmBuffer = (BYTE *)faacDecDecode(m_hDecoder, &FrameInfo, pFrame->GetData(), pFrame->GetSize());
//TRACE1("Frame : %d\n", m_total_frame );
	if((!FrameInfo.error) && (FrameInfo.samples > 0L)){
		// L-PCMコールバックに通知
		m_LastFrameInfo = FrameInfo;
		if(m_OnLpcmFrameProc)m_OnLpcmFrameProc(pPcmBuffer, FrameInfo.samples / (DWORD)FrameInfo.channels, FrameInfo.channels, m_pParam);
	}else{
		// エラー発生
		if(retry--) goto RETRY;
			char* err = faacDecGetErrorMessage(FrameInfo.error);
		TRACE1("FAAD2 Error:%s\n",err);
			m_byLastChannelConfig = 0xFF;
			m_InitRequest = true;

		if(m_LastFrameInfo.channels && pFrame->GetSize()){
			TRACE1("Add 0 to Wave. Frame num:%d\n",m_LastFrameInfo.samples / (DWORD)m_LastFrameInfo.channels);
			if(m_OnLpcmFrameProc)m_OnLpcmFrameProc(NULL, m_LastFrameInfo.samples / (DWORD)m_LastFrameInfo.channels, m_LastFrameInfo.channels, m_pParam);
		} else {
		}
		}

	return true;
}
