// TsDemuxer.cpp: CTsDemuxer クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsDemuxer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTsDemuxer::CTsDemuxer(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_VideoPesParser(this)
	, m_AudioPesParser(this)
	, m_Mpeg2Parser(this)
	, m_AdtsParser(this)
	, m_wVideoPID(0x1FFFU)
	, m_wAudioPID(0x1FFFU)
	, m_bLipSyncEnable(false)
	, m_bWaitingForVideo(true)
{
	m_FirstAudioPts = 0;
	m_NowAudioPts = 0;
	m_PrevAudioPts = 0;
	
	m_NowVideoPts = 0;
	m_PrevVideoPts = 0;
	m_FirstVideoPts = 0;
	
}

CTsDemuxer::~CTsDemuxer()
{

}

void CTsDemuxer::Reset()
{
	m_VideoPesParser.Reset();
	m_AudioPesParser.Reset();

	m_Mpeg2Parser.Reset();
	m_AdtsParser.Reset();

	m_bWaitingForVideo = true;

	m_FirstAudioPts = 0;
	m_NowAudioPts = 0;
	m_PrevAudioPts = 0;
	
	m_NowVideoPts = 0;
	m_PrevVideoPts = 0;
	m_FirstVideoPts = 0;

	// 下位デコーダをリセット
	CMediaDecoder::Reset();
}

const DWORD CTsDemuxer::GetInputNum() const
{
	return 1UL;
}

const DWORD CTsDemuxer::GetOutputNum() const
{
	return 2UL;
}

const bool CTsDemuxer::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// 入力メディアデータは互換性がない
	if(!pTsPacket)return false;

	// 映像PESパケット抽出
	if(pTsPacket->GetPID() == m_wVideoPID){
		m_VideoPesParser.StorePacket(pTsPacket);
		return true;
		}

	// 音声PESパケット抽出
	if(pTsPacket->GetPID() == m_wAudioPID){
		m_AudioPesParser.StorePacket(pTsPacket);
		return true;
		}

	return true;
}

const bool CTsDemuxer::SetVideoPID(const WORD wPID)
{
	// ビデオエレメンタリーストリームのPIDを設定する
	if(m_wVideoPID != wPID){
		m_wVideoPID = wPID;
		
		m_VideoPesParser.Reset();
		m_Mpeg2Parser.Reset();	
		m_bWaitingForVideo = true;
		
		return true;
		}

	return false;
}

const bool CTsDemuxer::SetAudioPID(const WORD wPID)
{
	// オーディオエレメンタリーストリームのPIDを設定する
	if(m_wAudioPID != wPID){
		m_wAudioPID = wPID;
		
		m_AudioPesParser.Reset();
		m_AdtsParser.Reset();
		
		return true;
		}
	
	return false;
}

void CTsDemuxer::EnableLipSync(const bool bEnable)
{
	if(!m_bLipSyncEnable && bEnable){
		// 再同期開始
		m_bWaitingForVideo = true;
		}

	// 設定保存
	m_bLipSyncEnable = (bEnable)? true : false;
}

LONGLONG CTsDemuxer::GetTotalAudioPts(void)
{
	return (m_NowAudioPts - m_FirstAudioPts);
}

LONGLONG CTsDemuxer::GetLastVideoPts(void)
{
	return (m_NowVideoPts - m_PrevVideoPts);
}

void CTsDemuxer::OnPesPacket(const CPesParser *pPesParser, const CPesPacket *pPacket)
{
	if(pPesParser == &m_VideoPesParser){
		// ビデオPESパケット受信

		if(m_Mpeg2Parser.StorePacket(pPacket)){

			m_PrevVideoPts = m_NowVideoPts;
			m_NowVideoPts = pPacket->GetPtsCount();

			if(!m_bWaitingForVideo){
				SendDecoderEvent(EID_SERVICE_M2V_COMMIT);	// 以前のPESパケットの処理完了通知
			}

			m_Mpeg2Parser.UpdateTotalFrame();			// 送信済みフレーム合計数の更新

			if(m_bWaitingForVideo){
				m_FirstVideoPts = pPacket->GetPtsCount();		
			}
			m_bWaitingForVideo = false;
		}

	} else if(pPesParser == &m_AudioPesParser) {
		// オーディオPESパケット受信
		if(m_bWaitingForVideo == false){
			if(m_FirstAudioPts==0){
				m_FirstAudioPts = pPacket->GetPtsCount();
				SendDecoderEvent(EID_SERVICE_FIRST_AAC);	// 以前のPESパケットの処理完了通知
			} else{
				m_PrevAudioPts = m_NowAudioPts;
				m_NowAudioPts = pPacket->GetPtsCount();

			}
			SendDecoderEvent(EID_SERVICE_AAC_COMMIT);	// 以前のPESパケットの処理完了通知
			m_AdtsParser.StorePacket(pPacket);
		}
	}
}

void CTsDemuxer::OnMpeg2Sequence(const CMpeg2Parser *pMpeg2Parser, const CMpeg2Sequence *pSequence)
{
	// MPEG2シーケンス受信、下位デコーダにデータを渡す
	OutputMedia(const_cast<CMpeg2Sequence *>(pSequence), OUTPUT_VIDEO);
}

void CTsDemuxer::OnAdtsFrame(const CAdtsParser *pAdtsParser, const CAdtsFrame *pFrame)
{
	// 最初のMPEG2シーケンスのトリガを待つ
//	if(m_bLipSyncEnable && m_bWaitingForVideo)return;
	// ADTSフレーム受信、下位デコーダにデータを渡す
	OutputMedia(const_cast<CAdtsFrame *>(pFrame), OUTPUT_AUDIO);
}

//2010.05.07 fuji 終了処理の追加
void CTsDemuxer::Close()
{
	//書き出されていない部分をはき出す
	m_VideoPesParser.Close();
	m_AudioPesParser.Close();
	m_Mpeg2Parser.Close();
	m_AdtsParser.Close();

}
