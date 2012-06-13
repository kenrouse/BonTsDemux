// TsDescrambler.cpp: CTsDescrambler クラスのインプリメンテーション
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
// CTsDescrambler 構築/消滅
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CTsDescrambler(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_dwInputPacketCount(0UL)
	, m_dwScramblePacketCount(0UL)
{
	// PATテーブルPIDマップ追加
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CTsDescrambler::OnPatUpdated, static_cast<PVOID>(this));
}

CTsDescrambler::~CTsDescrambler()
{
	CloseBcasCard();
}

void CTsDescrambler::Reset(void)
{
	// 内部状態を初期化する
	m_PidMapManager.UnmapAllTarget();

	// PATテーブルPIDマップ追加
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CTsDescrambler::OnPatUpdated, static_cast<PVOID>(this));

	// 統計データ初期化
	m_dwInputPacketCount = 0UL;
	m_dwScramblePacketCount = 0UL;

	// 下流デコーダを初期化する
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

	// 入力メディアデータは互換性がない
	if(!pTsPacket)return false;

	// 入力パケット数カウント
	if(m_dwInputPacketCount < 0xFFFFFFFFUL)m_dwInputPacketCount++;

	// PIDルーティング
	m_PidMapManager.StorePacket(pTsPacket);

	// パケット出力
	if(pTsPacket->IsScrambled()){
		// 復号漏れパケット数カウント
		if(m_dwScramblePacketCount < 0xFFFFFFFFUL)m_dwScramblePacketCount++;
	} else {

		// パケットを下流デコーダにデータを渡す
		OutputMedia(pMediaData);
	}

	return true;
}

const bool CTsDescrambler::OpenBcasCard(DWORD *pErrorCode)
{
	// カードリーダからB-CASカードを検索して開く
	const bool bReturn = m_BcasCard.OpenCard();
	
	// エラーコードセット
	if(pErrorCode)*pErrorCode = m_BcasCard.GetLastError();

	return bReturn;
}

void CTsDescrambler::CloseBcasCard(void)
{
	// B-CASカードを閉じる
	m_BcasCard.CloseCard();
}

const bool CTsDescrambler::GetBcasCardID(BYTE *pCardID)
{
	// カードID取得
	const BYTE *pBuff = m_BcasCard.GetBcasCardID();
	
	// バッファにコピー
	if(pCardID && pBuff)::CopyMemory(pCardID, pBuff, 6UL);
	
	return (pBuff)? true : false;
}

const DWORD CTsDescrambler::GetInputPacketCount(void) const
{
	// 入力パケット数を返す
	return m_dwInputPacketCount;
}

const DWORD CTsDescrambler::GetScramblePacketCount(void) const
{
	// 復号漏れパケット数を返す
	return m_dwScramblePacketCount;
}

void CALLBACK CTsDescrambler::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PATが更新された
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPatTable)::DebugBreak();
#endif

	// PMTテーブルPIDマップ追加
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		pMapManager->MapTarget(pPatTable->GetPmtPID(wIndex), new CPmtTable, CTsDescrambler::OnPmtUpdated, pParam);
		}
}

void CALLBACK CTsDescrambler::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMTが更新された
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPmtTable)::DebugBreak();
#endif

	// ECMのPIDマップ追加
	const WORD wEcmPID = pPmtTable->GetEcmPID();
	if(wEcmPID >= 0x1FFFU)return;

	// 既存のECM処理ターゲットを確認
	CEcmProcessor *pEcmProcessor = dynamic_cast<CEcmProcessor *>(pMapManager->GetMapTarget(wEcmPID));

	if(!pEcmProcessor){
		// ECM処理内部クラス新規マップ
		pEcmProcessor = new CEcmProcessor(&pThis->m_BcasCard);
		pMapManager->MapTarget(wEcmPID, pEcmProcessor);
		}
	
	// ESのPIDマップ追加
	for(WORD wIndex = 0U ; wIndex < pPmtTable->GetEsInfoNum() ; wIndex++){
		pMapManager->MapTarget(pPmtTable->GetEsPID(wIndex), new CEsProcessor(pEcmProcessor), NULL, pParam);
		}
}


//////////////////////////////////////////////////////////////////////
// CTsDescrambler::CEcmProcessor 構築/消滅
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CEcmProcessor::CEcmProcessor(CBcasCard *pBcasCard)
	: CDynamicReferenceable()
	, CPsiSingleTable(true)
	, m_pBcasCard(pBcasCard)
	, m_bLastEcmSucceed(true)
{
	// MULTI2デコーダにシステムキーと初期CBCをセット
	m_Multi2Decoder.Initialize(m_pBcasCard->GetSystemKey(), m_pBcasCard->GetInitialCbc());
}

void CTsDescrambler::CEcmProcessor::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	// 参照カウント追加
	AddRef();
}

void CTsDescrambler::CEcmProcessor::OnPidUnmapped(const WORD wPID)
{
	// 参照カウント開放
	ReleaseRef();
}

const bool CTsDescrambler::CEcmProcessor::DescramblePacket(CTsPacket *pTsPacket)
{
	// スクランブル解除
	if(m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(), (DWORD)pTsPacket->GetPayloadSize(), pTsPacket->m_Header.byTransportScramblingCtrl)){
		// トランスポートスクランブル制御再設定
		pTsPacket->SetAt(3UL, pTsPacket->GetAt(3UL) & 0x3FU);
		pTsPacket->m_Header.byTransportScramblingCtrl = 0U;
		return true;
		}
	
	return false;
}

const bool CTsDescrambler::CEcmProcessor::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	// ECMをB-CASカードに渡してキー取得
	const BYTE *pKsData = m_pBcasCard->GetKsFromEcm(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());

	// ECM処理失敗時は一度だけB-CASカードを再初期化する
	if(!pKsData && m_bLastEcmSucceed && (m_pBcasCard->GetLastError() != BCEC_ECMREFUSED)){
		if(m_pBcasCard->OpenCard()){
			pKsData = m_pBcasCard->GetKsFromEcm(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());
			}
		}

	// スクランブルキー更新
	m_Multi2Decoder.SetScrambleKey(pKsData);

	// ECM処理成功状態更新
	m_bLastEcmSucceed = (pKsData)? true : false;

	return true;
}


//////////////////////////////////////////////////////////////////////
// CTsDescrambler::CEsProcessor 構築/消滅
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CEsProcessor::CEsProcessor(CTsDescrambler::CEcmProcessor *pEcmProcessor)
	: CTsPidMapTarget()
	, m_pEcmProcessor(pEcmProcessor)
{
	// 参照カウント追加
	m_pEcmProcessor->AddRef();
}

CTsDescrambler::CEsProcessor::~CEsProcessor()
{
	// 参照カウント削除
	m_pEcmProcessor->ReleaseRef();
}

const bool CTsDescrambler::CEsProcessor::StorePacket(const CTsPacket *pPacket)
{
	// スクランブル解除
	m_pEcmProcessor->DescramblePacket(const_cast<CTsPacket *>(pPacket));

	return false;
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(const WORD wPID)
{
	// インスタンス開放
	delete this;
}
