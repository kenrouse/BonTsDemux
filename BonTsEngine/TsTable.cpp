// TsTable.cpp: TSテーブルラッパークラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CPsiTableクラスの構築/消滅
//////////////////////////////////////////////////////////////////////
/*
CPsiTable::CPsiTable()
{
	Reset();
}

CPsiTable::CPsiTable(const CPsiTable &Operand)
{
	*this = Operand;
}

CPsiTable & CPsiTable::operator = (const CPsiTable &Operand)
{
	m_TableArray = Operand.m_TableArray;

	return *this;
}

const bool CPsiTable::StoreSection(const CPsiSection *pSection, bool *pbUpdate)
{
	bool bUpdate = false;

	// セクションのサイズをチェック
	if(!pSection->GetPayloadSize()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return false;
		}

	// カレントネクストインジケータチェック(本来は別バンクに貯めるべき？)
	if(!pSection->IsCurrentNext()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return true;
		}

	// テーブルID拡張を検索する
	DWORD dwIndex;

	for(dwIndex = 0UL ; dwIndex < m_TableArray.size() ; dwIndex++){
		if(m_TableArray[dwIndex].wTableIdExtension == pSection->GetTableIdExtension())break;
		}

	if(dwIndex >= m_TableArray.size()){
		// テーブルID拡張が見つからない、テーブル追加
		m_TableArray.resize(dwIndex + 1);
		m_TableArray[dwIndex].wTableIdExtension = pSection->GetTableIdExtension();
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].byVersionNo != pSection->GetVersionNo()){
		// バージョンが不一致、テーブルが更新された
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.clear();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].wSectionNum != ((WORD)pSection->GetLastSectionNumber() + 1U)){
		// セクション数が変化した(例外？)
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		}

	// セクションデータを更新する
	m_TableArray[dwIndex].SectionArray[pSection->GetSectionNumber()].SetData(pSection->GetPayloadData(), pSection->GetPayloadSize());

	// 更新情報設定
	if(pbUpdate)*pbUpdate = bUpdate;

	return true;
}

const WORD CPsiTable::GetExtensionNum(void) const
{
	// テーブルの数を返す
	return m_TableArray.size();
}

const bool CPsiTable::GetExtension(const WORD wIndex, WORD *pwExtension) const
{
	if(wIndex >= GetExtensionNum())return false;

	// テーブルID拡張を返す
	*pwExtension = m_TableArray[wIndex].wTableIdExtension;

	return true;
}

const bool CPsiTable::GetSectionNum(const WORD wIndex, WORD *pwSectionNum) const
{
	if(wIndex >= GetExtensionNum())return false;

	// テーブルID拡張を返す
	*pwSectionNum = m_TableArray[wIndex].wSectionNum;

	return true;
}

const CMediaData * CPsiTable::GetSectionData(const WORD wIndex, const BYTE bySectionNo) const
{
	if(wIndex >= GetExtensionNum())return NULL;
	if(bySectionNo > m_TableArray[wIndex].wSectionNum)return NULL;

	// セクションデータを返す
	return &m_TableArray[wIndex].SectionArray[bySectionNo];
}

void CPsiTable::Reset(void)
{
	// 全てのテーブルを削除する
	m_TableArray.clear();
}
*/

/////////////////////////////////////////////////////////////////////////////
// PSIシングルテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPsiSingleTable::CPsiSingleTable(const bool bTargetSectionExt)
	: CTsPidMapTarget()
	, m_bTargetSectionExt(bTargetSectionExt)
	, m_bTableUpdated(false)
{
	// ターゲットセクション、コールバック設定
	m_PsiSectionParser.SetRecvCallback(bTargetSectionExt, CPsiSingleTable::StoreSection, static_cast<PVOID>(this));
}

CPsiSingleTable::CPsiSingleTable(const CPsiSingleTable &Operand)
{
	// コピーコンストラクタ
	*this = Operand;
}

CPsiSingleTable::~CPsiSingleTable()
{

}

CPsiSingleTable & CPsiSingleTable::operator = (const CPsiSingleTable &Operand)
{
	// インスタンスのコピー
	m_CurSection = Operand.m_CurSection;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	m_bTargetSectionExt = Operand.m_bTargetSectionExt;
	m_bTableUpdated = Operand.m_bTableUpdated;
	
	// コールバック再設定
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiSingleTable::StoreSection, static_cast<PVOID>(this));

	return *this;
}

const bool CPsiSingleTable::StorePacket(const CTsPacket *pPacket)
{
	if(!pPacket)return false;
	
	m_bTableUpdated = false;
	
	// パケットストア
	m_PsiSectionParser.StorePacket(pPacket);
	
	return m_bTableUpdated;
}

void CPsiSingleTable::OnPidUnmapped(const WORD wPID)
{
	// インスタンスを開放する
	delete this;
}

void CPsiSingleTable::Reset(void)
{
	// 状態初期化
	m_PsiSectionParser.Reset();
	m_CurSection.Reset();
	
	m_bTableUpdated = false;
}

const DWORD CPsiSingleTable::GetCrcErrorCount(void) const
{
	// CRCエラーカウントを取得する
	return m_PsiSectionParser.GetCrcErrorCount();
}

const bool CPsiSingleTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	// デフォルトの実装では何もしない
	return true;
}

void CPsiSingleTable::OnPsiSection(const CPsiSection *pSection)
{
	// セクションのフィルタリングを行う場合は派生クラスでオーバーライドする
	// デフォルトの実装ではセクションペイロード更新時に仮想関数に通知する
	if(!(*pSection == m_CurSection)){
		// セクションが更新された
		if(OnTableUpdate(pSection, &m_CurSection)){
			// セクションストア
			m_CurSection = *pSection;
			m_bTableUpdated = true;
			}
		}
}

void CALLBACK CPsiSingleTable::StoreSection(const CPsiSection *pSection, const PVOID pParam)
{
	CPsiSingleTable *pThis = static_cast<CPsiSingleTable *>(pParam);

	// 仮想関数呼び出し
	pThis->OnPsiSection(pSection);
}


/////////////////////////////////////////////////////////////////////////////
// PSIテーブルセット抽象化クラス
/////////////////////////////////////////////////////////////////////////////
/*
CPsiTableSuite::CPsiTableSuite()
	: m_bTargetSectionExt(true)
{
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	Reset();
}

CPsiTableSuite::CPsiTableSuite(const CPsiTableSuite &Operand)
{
	*this = Operand;
}

CPsiTableSuite & CPsiTableSuite::operator = (const CPsiTableSuite &Operand)
{
	m_TableSet = Operand.m_TableSet;
	m_bTargetSectionExt = Operand.m_bTargetSectionExt;
	m_bTableUpdated = Operand.m_bTableUpdated;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	return *this;
}

const bool CPsiTableSuite::StorePacket(const CTsPacket *pPacket)
{
	m_bTableUpdated = false;
	
	// PSIセクションをテーブルに追加する
	m_PsiSectionParser.StorePacket(pPacket);

	return m_bTableUpdated;
}

void CPsiTableSuite::SetTargetSectionExt(const bool bTargetExt)
{
	// 処理対象とするセクションの種類を設定する(拡張 or 標準)
	m_bTargetSectionExt = bTargetExt;
	m_PsiSectionParser.SetRecvCallback(bTargetExt, CPsiTableSuite::StoreSection, this);
}

const bool CPsiTableSuite::AddTable(const BYTE byTableID)
{
	const WORD wNum = m_TableSet.size();

	// 既存のIDをチェック
	if(GetIndexByID(byTableID) < wNum)return false;

	// 最後尾に要素を追加
	m_TableSet.resize(wNum + 1U);

	// IDをセット
	m_TableSet[wNum].byTableID = byTableID;

	return true;
}

const WORD CPsiTableSuite::GetIndexByID(const BYTE byTableID)
{
	// 既存のテーブルIDを検索する
	for(WORD wIndex = 0U ; wIndex < m_TableSet.size() ; wIndex++){
		if(m_TableSet[wIndex].byTableID == byTableID)return wIndex;
		}

	// エラー時は常にテーブルの最大数より大きいインデックスを返す
	return 0x0100U;
}

const CPsiTable * CPsiTableSuite::GetTable(const WORD wIndex) const
{
	// テーブルを返す
	return (wIndex < m_TableSet.size())? &m_TableSet[wIndex].PsiTable : NULL;
}

const CMediaData * CPsiTableSuite::GetSectionData(const WORD wIndex, const WORD wSubIndex, const BYTE bySectionNo) const
{
	// セクションデータを返す
	const CPsiTable *pPsiTable = GetTable(wIndex);

	// テーブルが見つからない
	if(!pPsiTable)return NULL;

	// データを返す
	return pPsiTable->GetSectionData(wSubIndex, bySectionNo);
}

void CPsiTableSuite::Reset(void)
{
	m_bTableUpdated = false;

	m_PsiSectionParser.Reset();
	m_TableSet.clear();
}

const DWORD CPsiTableSuite::GetCrcErrorCount(void) const
{
	return m_PsiSectionParser.GetCrcErrorCount();
}

void CALLBACK CPsiTableSuite::StoreSection(const CPsiSection *pSection, const PVOID pParam)
{
	CPsiTableSuite *pThis = static_cast<CPsiTableSuite *>(pParam);

	// 対象外のセクションは処理しない
	if(pSection->IsExtendedSection() != pThis->m_bTargetSectionExt)return;

	// テーブルIDを検索
	const WORD wIndex = pThis->GetIndexByID(pSection->GetTableID());

	// テーブルIDが見つからない
	if(wIndex >= pThis->m_TableSet.size())return;

	// テーブルにストアする
	bool bUpdate = false;
	pThis->m_TableSet[wIndex].PsiTable.StoreSection(pSection, &bUpdate);

	if(bUpdate)pThis->m_bTableUpdated = true;
}
*/

/////////////////////////////////////////////////////////////////////////////
// PATテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPatTable::CPatTable()
	: CPsiSingleTable()
{
	Reset();
}

CPatTable::CPatTable(const CPatTable &Operand)
{
	*this = Operand;
}

CPatTable & CPatTable::operator = (const CPatTable &Operand)
{
	CPsiSingleTable::operator = (Operand);

	m_NitPIDArray = Operand.m_NitPIDArray;
	m_PmtPIDArray = Operand.m_PmtPIDArray;
	
	return *this;
}

const WORD CPatTable::GetNitPID(const WORD wIndex) const
{
	// NITのPIDを返す
	return (wIndex < m_NitPIDArray.size())? m_NitPIDArray[wIndex].wPID : 0xFFFFU;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetNitNum(void) const
{
	// NITの数を返す
	return m_NitPIDArray.size();
}

const WORD CPatTable::GetPmtPID(const WORD wIndex) const
{
	// PMTのPIDを返す
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wPID : 0xFFFFU;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetProgramID(const WORD wIndex) const
{
	// Program Number IDを返す
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wProgramID : 0x0000U;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetProgramNum(void) const
{
	// PMTの数を返す
	return m_PmtPIDArray.size();
}

const bool CPatTable::IsPmtTablePID(const WORD wPID) const
{
	// PMTのPIDかどうかを返す
	for(WORD wIndex = 0U ; wIndex < m_PmtPIDArray.size() ; wIndex++){
		if(wPID == m_PmtPIDArray[wIndex].wPID)return true;
		}

	return false;
}

void CPatTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();
}

const bool CPatTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if(wDataSize % 4U)return false;						// テーブルのサイズが不正
	if(pCurSection->GetTableID() != 0x00U)return false;	// テーブルIDが不正

	// PIDをクリアする
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	TAG_PATITEM PatItem;

	// テーブルを解析する

	TRACE(TEXT("\n------- PAT Table -------\nTS ID = %04X\n"), pCurSection->GetTableIdExtension());

	for(WORD wPos = 0 ; wPos < wDataSize ; wPos += 4U, pHexData += 4){
		PatItem.wProgramID	= ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];				// +1,2
		PatItem.wPID		= ((WORD)(pHexData[2] & 0x1FU) << 8) | (WORD)pHexData[3];	// +3,4

		if(!PatItem.wProgramID){
			// NITのPID
			TRACE(TEXT("NIT #%u [ID:%04X][PID:%04X]\n"), m_NitPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_NitPIDArray.push_back(PatItem);
			}
		else{
			// PMTのPID
			TRACE(TEXT("PMT #%u [ID:%04X][PID:%04X]\n"), m_PmtPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_PmtPIDArray.push_back(PatItem);
			}
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// PMTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPmtTable::CPmtTable()
	: CPsiSingleTable()
{
	Reset();
}

CPmtTable::CPmtTable(const CPmtTable &Operand)
{
	*this = Operand;
}

CPmtTable & CPmtTable::operator = (const CPmtTable &Operand)
{
	CPsiSingleTable::operator = (Operand);
	m_wPcrPID = Operand.m_wPcrPID;
	m_TableDescBlock = Operand.m_TableDescBlock;
	m_EsInfoArray = Operand.m_EsInfoArray;
	
	return *this;
}

const WORD CPmtTable::GetPcrPID(void) const
{
	// PCR_PID を返す
	return m_wPcrPID;
}

const CDescBlock * CPmtTable::GetTableDesc(void) const
{
	// テーブルの記述子ブロックを返す
	return &m_TableDescBlock;
}

const WORD CPmtTable::GetEcmPID(void) const
{
	// ECMのPIDを返す
	const CCaMethodDesc *pCaMethodDesc = dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG));

	return (pCaMethodDesc)? pCaMethodDesc->GetCaPID() : 0xFFFFU;
}

const WORD CPmtTable::GetEsInfoNum(void) const
{
	// ES情報の数を返す
	return m_EsInfoArray.size();
}

const BYTE CPmtTable::GetStreamTypeID(const WORD wIndex) const
{
	// Stream Type ID を返す
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].byStreamTypeID : 0x00U;	// 0x00は未定義のID
}

const WORD CPmtTable::GetEsPID(const WORD wIndex) const
{
	// Elementary Stream PID を返す
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].wEsPID : 0xFFFFU;			// 0xFFFFは未定義のPID
}

const CDescBlock * CPmtTable::GetItemDesc(const WORD wIndex) const
{
	// アイテムの記述子ブロックを返す
	return (wIndex < m_EsInfoArray.size())? &m_EsInfoArray[wIndex].DescBlock : NULL;
}

void CPmtTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_wPcrPID = 0xFFFFU;
	m_TableDescBlock.Reset();
	m_EsInfoArray.clear();
}

const bool CPmtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if(pCurSection->GetTableID() != 0x02U)return false;	// テーブルIDが不正

	// 状態をクリアする
	m_wPcrPID = 0xFFFFU;
	m_EsInfoArray.clear();

	// テーブルを解析する
	WORD wDescLen = ((WORD)(pHexData[2] & 0x0FU) << 8) | (WORD)pHexData[3];
	m_wPcrPID = ((WORD)(pHexData[0] & 0x1FU) << 8) | (WORD)pHexData[1];				// +0,1

	// 記述子ブロック
	m_TableDescBlock.ParseBlock(&pHexData[4], wDescLen);

	// ストリーム情報の開始位置を計算
	WORD wPos = wDescLen + 4U;
	TAG_PMTITEM PmtItem;

	TRACE(TEXT("\n------- PMT Table -------\nProgram Number ID = %04X\nPCR PID = %04X\nECM PID = %04X\n"), pCurSection->GetTableIdExtension(), m_wPcrPID, (m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))? dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))->GetCaPID() : 0xFFFFU);
	
	// ストリーム情報を解析
	while(wPos < wDataSize){
		PmtItem.byStreamTypeID = pHexData[wPos + 0];													// +0
		PmtItem.wEsPID = ((WORD)(pHexData[wPos + 1] & 0x1FU) << 8) | (WORD)pHexData[wPos + 2];			// +1,2	
		wDescLen = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];				// +3,4

		// 記述子ブロック
		PmtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wDescLen);	

		TRACE(TEXT("[%u] Stream Type ID = %02X  PID = %04X\n"), m_EsInfoArray.size(), PmtItem.byStreamTypeID, PmtItem.wEsPID);		

		// テーブルに追加する
		m_EsInfoArray.push_back(PmtItem);
		wPos += (wDescLen + 5U);
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// SDTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CSdtTable::CSdtTable()
	: CPsiSingleTable()
{

}

CSdtTable::CSdtTable(const CSdtTable &Operand)
{
	*this = Operand;
}

CSdtTable & CSdtTable::operator = (const CSdtTable &Operand)
{
	CPsiSingleTable::operator = (Operand);

	return *this;
}

const WORD CSdtTable::GetServiceNum(void) const
{
	// サービス数を返す
	return m_ServiceInfoArray.size();
}

const WORD CSdtTable::GetServiceIndexByID(const WORD wServiceID)
{
	// サービスIDからインデックスを返す
	for(WORD wIndex = 0U ; wIndex < GetServiceNum() ; wIndex++){
		if(m_ServiceInfoArray[wIndex].wServiceID == wServiceID){
			return wIndex;
			}		
		}
	
	// サービスIDが見つからない
	return 0xFFFFU;
}

const WORD CSdtTable::GetServiceID(const WORD wIndex) const
{
	// 	サービスIDを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].wServiceID : 0xFFFFU;
}

const BYTE CSdtTable::GetRunningStatus(const WORD wIndex) const
{
	// Running Statusを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].byRunningStatus : 0xFFU;
}

const bool CSdtTable::GetFreeCaMode(const WORD wIndex) const
{
	// Free CA Modeを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].bFreeCaMode : false;
}

const CDescBlock * CSdtTable::GetItemDesc(const WORD wIndex) const
{
	// アイテムの記述子ブロックを返す
	return (wIndex < m_ServiceInfoArray.size())? &m_ServiceInfoArray[wIndex].DescBlock : NULL;
}

void CSdtTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();
	
	m_ServiceInfoArray.clear();
}

void CSdtTable::OnPsiSection(const CPsiSection *pSection)
{
	if(pSection->GetTableID() != 0x42U)return;
	
	// Actual Streamだけを処理する
	CPsiSingleTable::OnPsiSection(pSection);
}

const bool CSdtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	// 状態をクリアする
	m_ServiceInfoArray.clear();

	TRACE(TEXT("\n------- SDT Table -------\nTransport Stream ID = %04X\nOriginal Network ID = %02X%02X\n"), pCurSection->GetTableIdExtension(), pHexData[0], pHexData[1]);

	// テーブルを解析する
	for(WORD wPos = 3U ; wPos < wDataSize ; ){
		TAG_SDTITEM SdtItem;
		SdtItem.wServiceID		= ((WORD)pHexData[wPos + 0] << 8) | (WORD)pHexData[wPos + 1];
		SdtItem.byRunningStatus	= pHexData[wPos + 3] >> 5;
		SdtItem.bFreeCaMode		= (pHexData[wPos + 3] & 0x10U)? true : false;
		
		// Service Descriptor
		const WORD wLength = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];

		// 記述子ブロック
		SdtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wLength);

		// デバッグ用ここから
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(SdtItem.DescBlock.GetDescByTag(CServiceDesc::DESC_TAG));
		if(pServiceDesc){
			TCHAR szServiceName[1024] = {TEXT('\0')};
			pServiceDesc->GetServiceName(szServiceName);
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  Service Type = %02X  Service Name = %s\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode, pServiceDesc->GetServiceType(), szServiceName);		
			}
		else{
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  ※サービス記述子なし\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode);		
			}
		// ここまで

		// テーブル追加
		m_ServiceInfoArray.push_back(SdtItem);

		// 位置更新	
		wPos += (wLength + 5U);
		}

	return true;
}
