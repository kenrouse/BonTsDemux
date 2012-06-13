// MediaDecoder.cpp: CMediaDecoder クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaDecoder.h"
#include "MediaData.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CMediaDecoder 構築/消滅
//////////////////////////////////////////////////////////////////////

CMediaDecoder::CMediaDecoder(CDecoderHandler *pDecoderHandler)
	: m_pDecoderHandler(pDecoderHandler)
{
	// 出力フィルタ配列をクリアする
	::ZeroMemory(m_aOutputDecoder, sizeof(m_aOutputDecoder));
}

CMediaDecoder::~CMediaDecoder()
{

}

void CMediaDecoder::Reset()
{
	// 次のフィルタをリセットする
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			m_aOutputDecoder[dwOutputIndex].pDecoder->Reset();
			}
		}
}

const bool CMediaDecoder::SetOutputDecoder(CMediaDecoder *pDecoder, const DWORD dwOutputIndex, const DWORD dwInputIndex)
{
	// 出力フィルタをセットする
	if(dwOutputIndex < GetOutputNum()){
		m_aOutputDecoder[dwOutputIndex].pDecoder = pDecoder;
		m_aOutputDecoder[dwOutputIndex].dwInputIndex = dwInputIndex;
		return true;
		}

	return false;
}

const bool CMediaDecoder::OutputMedia(CMediaData *pMediaData, const DWORD dwOutptIndex)
{
	// デフォルトの出力処理

	// 次のフィルタにデータを渡す
	if(dwOutptIndex < GetOutputNum()){
		if(m_aOutputDecoder[dwOutptIndex].pDecoder){
			return m_aOutputDecoder[dwOutptIndex].pDecoder->InputMedia(pMediaData, m_aOutputDecoder[dwOutptIndex].dwInputIndex);
			}
		}

	return false;
}

const DWORD CMediaDecoder::SendDecoderEvent(const DWORD dwEventID, PVOID pParam)
{
	// イベントを通知する
		return m_pDecoderHandler->OnDecoderEvent(this, dwEventID, pParam);
}


//////////////////////////////////////////////////////////////////////
// CDecoderHandler 構築/消滅
//////////////////////////////////////////////////////////////////////

const DWORD CDecoderHandler::OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	// デフォルトの処理
	return 0UL;
}
