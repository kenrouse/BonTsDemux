// ProgManager.cpp: CProgManager クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"
#include "ProgManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CProgManager 構築/消滅
//////////////////////////////////////////////////////////////////////

CProgManager::CProgManager(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
{
	// プログラムデータベースインスタンス生成
	m_pProgDatabase = new CProgDatabase(*this);
}

CProgManager::~CProgManager()
{
	// プログラムデータベースインスタンス開放
	delete m_pProgDatabase;
}

void CProgManager::Reset()
{
	// サービスリストをクリア
	m_ServiceList.clear();

	// プログラムデータベースリセット
	m_pProgDatabase->Reset();

	// 下位デコーダをリセット
	CMediaDecoder::Reset();
}

const DWORD CProgManager::GetInputNum() const
{
	return 1UL;
}

const DWORD CProgManager::GetOutputNum() const
{
	return 1UL;
}

const bool CProgManager::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// 入力メディアデータは互換性がない
	if(!pTsPacket)return false;

	// PIDルーティング
	m_PidMapManager.StorePacket(pTsPacket);

	// 次のフィルタにデータを渡す
	OutputMedia(pMediaData);

	return true;
}

const WORD CProgManager::GetServiceNum(void) const
{
	// サービス数を返す
	return m_ServiceList.size();
}

const bool CProgManager::GetServiceID(WORD *pwServiceID, const WORD wIndex) const
{
	// サービスIDを取得する
	if((wIndex < GetServiceNum()) && pwServiceID){
		*pwServiceID = m_ServiceList[wIndex].wServiceID;
		return true;		
		}
		
	return false;
}

const bool CProgManager::GetServiceEsPID(WORD *pwVideoPID, WORD *pwAudioPID, const WORD wIndex) const
{
	// ESのPIDを取得する
	if((wIndex < GetServiceNum()) && (pwVideoPID || pwAudioPID)){
		if(pwVideoPID)*pwVideoPID = m_ServiceList[wIndex].wVideoEsPID;
		if(pwAudioPID)*pwAudioPID = m_ServiceList[wIndex].wAudioEsPID;
		return true;
		}

	return false;
}

const DWORD CProgManager::GetServiceName(LPTSTR lpszDst, const WORD wIndex) const
{
	// サービス名を取得する
	if((wIndex < GetServiceNum()) && lpszDst){
		const WORD wNameLen = ::lstrlen(m_ServiceList[wIndex].szServiceName);
		if(wNameLen)::lstrcpy(lpszDst, m_ServiceList[wIndex].szServiceName);
		return wNameLen;		
		}
		
	return 0U;
}

void CProgManager::OnServiceListUpdated(void)
{
	// サービスリストクリア、リサイズ
	m_ServiceList.clear();

	// サービスリスト構築
	for(WORD wIndex = 0U, wServiceNum = 0U ; wIndex < m_pProgDatabase->m_ServiceList.size() ; wIndex++){
		if(m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID != 0xFFFFU){
			// MPEG2映像のみ(ワンセグ、データ放送以外)
			m_ServiceList.resize(wServiceNum + 1);
			m_ServiceList[wServiceNum].wServiceID = m_pProgDatabase->m_ServiceList[wIndex].wServiceID;
			m_ServiceList[wServiceNum].wVideoEsPID = m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID;
			m_ServiceList[wServiceNum].wAudioEsPID = m_pProgDatabase->m_ServiceList[wIndex].wAudioEsPID;
			m_ServiceList[wServiceNum].szServiceName[0] = TEXT('\0');
			wServiceNum++;
			}
		}

	TRACE(TEXT("CProgManager::OnServiceListUpdated()\n"));

	SendDecoderEvent(EID_SERVICE_LIST_UPDATED);
}

void CProgManager::OnServiceInfoUpdated(void)
{
	// サービス名を更新する
	for(WORD wIndex = 0U, wServiceNum = 0U ; wIndex < GetServiceNum() ; wIndex++){
		const WORD wServiceIndex = m_pProgDatabase->GetServiceIndexByID(m_ServiceList[wIndex].wServiceID);

		if(wServiceIndex != 0xFFFFU){
			if(m_pProgDatabase->m_ServiceList[wIndex].szServiceName[0]){
				::lstrcpy(m_ServiceList[wIndex].szServiceName, m_pProgDatabase->m_ServiceList[wServiceIndex].szServiceName);
				}
			else{
				::wsprintf(m_ServiceList[wIndex].szServiceName, TEXT("サービス%u"), wIndex + 1U);
				}
			}
		}

	TRACE(TEXT("CProgManager::OnServiceInfoUpdated()\n"));
	
	SendDecoderEvent(EID_SERVICE_INFO_UPDATED);
}


//////////////////////////////////////////////////////////////////////
// CProgDatabase 構築/消滅
//////////////////////////////////////////////////////////////////////

CProgManager::CProgDatabase::CProgDatabase(CProgManager &ProgManager)
	: m_ProgManager(ProgManager)
	, m_PidMapManager(ProgManager.m_PidMapManager)
	, m_wTransportStreamID(0x0000U)
{
	Reset();
}

CProgManager::CProgDatabase::~CProgDatabase()
{
	UnmapTable();
}

void CProgManager::CProgDatabase::Reset(void)
{
	// 全テーブルアンマップ
	UnmapTable();

	// PATテーブルPIDマップ追加
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CProgDatabase::OnPatUpdated, static_cast<PVOID>(this));
}

void CProgManager::CProgDatabase::UnmapTable(void)
{
	// 全PMT PIDアンマップ
	for(WORD wIndex = 0U ; wIndex < m_ServiceList.size() ; wIndex++){
		m_PidMapManager.UnmapTarget(m_ServiceList[wIndex].wPmtTablePID);
		}

	// サービスリストクリア
	m_ServiceList.clear();
	
	// トランスポートストリームID初期化
	m_wTransportStreamID = 0xFFFFU;
	
	// PATテーブルリセット
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(m_PidMapManager.GetMapTarget(0x0000U));
	if(pPatTable)pPatTable->Reset();
}

const WORD CProgManager::CProgDatabase::GetServiceIndexByID(const WORD wServiceID)
{
	// プログラムIDからサービスインデックスを検索する
	for(WORD wIndex = 0U ; wIndex < m_ServiceList.size() ; wIndex++){
		if(m_ServiceList[wIndex].wServiceID == wServiceID)return wIndex;
		}

	// プログラムIDが見つからない
	return 0xFFFFU;
}

void CALLBACK CProgManager::CProgDatabase::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PATが更新された
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);
		
	// トランスポートストリームID更新
	pThis->m_wTransportStreamID = pPatTable->m_CurSection.GetTableIdExtension();

	// 現PMTのPIDをアンマップする
	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		pMapManager->UnmapTarget(pThis->m_ServiceList[wIndex].wPmtTablePID);
		}

	// 新PMTをストアする
	pThis->m_ServiceList.resize(pPatTable->GetProgramNum());

	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		// サービスリスト更新
		pThis->m_ServiceList[wIndex].bIsUpdated = false;
		pThis->m_ServiceList[wIndex].wServiceID = pPatTable->GetProgramID(wIndex);
		pThis->m_ServiceList[wIndex].wPmtTablePID = pPatTable->GetPmtPID(wIndex);

		pThis->m_ServiceList[wIndex].wVideoEsPID = 0xFFFFU;
		pThis->m_ServiceList[wIndex].wAudioEsPID = 0xFFFFU;
		pThis->m_ServiceList[wIndex].byVideoComponentTag = 0xFFU;
		pThis->m_ServiceList[wIndex].byAudioComponentTag = 0xFFU;
		pThis->m_ServiceList[wIndex].byServiceType = 0xFFU;
		pThis->m_ServiceList[wIndex].byRunningStatus = 0xFFU;
		pThis->m_ServiceList[wIndex].bIsCaService = false;
		pThis->m_ServiceList[wIndex].szServiceName[0] = TEXT('\0');
		
		// PMTのPIDをマップ
		pMapManager->MapTarget(pPatTable->GetPmtPID(wIndex), new CPmtTable, CProgDatabase::OnPmtUpdated, pParam);
		}
}

void CALLBACK CProgManager::CProgDatabase::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMTが更新された
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

	// サービスインデックスを検索
	const WORD wServiceIndex = pThis->GetServiceIndexByID(pPmtTable->m_CurSection.GetTableIdExtension());
	if(wServiceIndex == 0xFFFFU)return;

	// ビデオESのPIDをストア
	pThis->m_ServiceList[wServiceIndex].wVideoEsPID = 0xFFFFU;
	
	for(WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++){
		// 「ITU-T Rec. H.262|ISO/IEC 13818-2 Video or ISO/IEC 11172-2」のストリームタイプを検索
		if(pPmtTable->GetStreamTypeID(wEsIndex) == 0x02U){
			pThis->m_ServiceList[wServiceIndex].wVideoEsPID = pPmtTable->GetEsPID(wEsIndex);
			break;
			}		
		}

	// オーディオESのPIDをストア
	pThis->m_ServiceList[wServiceIndex].wAudioEsPID = 0xFFFFU;
	
	for(WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++){
		// 「ISO/IEC 13818-7 Audio (ADTS Transport Syntax)」のストリームタイプを検索
		if(pPmtTable->GetStreamTypeID(wEsIndex) == 0x0FU){
			pThis->m_ServiceList[wServiceIndex].wAudioEsPID = pPmtTable->GetEsPID(wEsIndex);
			break;
			}
		}

	// 更新済みマーク
	pThis->m_ServiceList[wServiceIndex].bIsUpdated = true;
	
/*
	放送局によってはPATに含まれるサービス全てのPMTを流していない場合がある。
　　(何度もハンドラが呼び出されるのを防止するため全ての情報がそろった段階で呼び出したかった)

	// 他のPMTの更新状況を調べる
	for(WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++){
		if(!pThis->m_ServiceList[wIndex].bIsUpdated)return;
		}
*/

	// SDTテーブルを再マップする
	pMapManager->MapTarget(0x0011U, new CSdtTable, CProgDatabase::OnSdtUpdated, pParam);
	
	// イベントハンドラ呼び出し
	pThis->m_ProgManager.OnServiceListUpdated();
}

void CALLBACK CProgManager::CProgDatabase::OnSdtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// SDTが更新された
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CSdtTable *pSdtTable = dynamic_cast<CSdtTable *>(pMapTarget);

	for(WORD wSdtIndex = 0U ; wSdtIndex < pSdtTable->GetServiceNum() ; wSdtIndex++){
		// サービスIDを検索
		const WORD wServiceIndex = pThis->GetServiceIndexByID(pSdtTable->GetServiceID(wSdtIndex));
		if(wServiceIndex == 0xFFFFU)continue;

		// サービス情報更新
		pThis->m_ServiceList[wServiceIndex].byRunningStatus = pSdtTable->GetRunningStatus(wSdtIndex);
		pThis->m_ServiceList[wServiceIndex].bIsCaService = pSdtTable->GetFreeCaMode(wSdtIndex);
		
		// サービス名更新
		pThis->m_ServiceList[wServiceIndex].szServiceName[0] = TEXT('\0');

		const CDescBlock *pDescBlock = pSdtTable->GetItemDesc(wSdtIndex);
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(pDescBlock->GetDescByTag(CServiceDesc::DESC_TAG));

		if(pServiceDesc){
			pServiceDesc->GetServiceName(pThis->m_ServiceList[wServiceIndex].szServiceName);
			pThis->m_ServiceList[wServiceIndex].byServiceType = pServiceDesc->GetServiceType();
			}
		}

	// イベントハンドラ呼び出し
	pThis->m_ProgManager.OnServiceInfoUpdated();
}
