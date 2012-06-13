// TsStream.cpp: TSストリームラッパークラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsStream.h"

#include <mmsystem.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#pragma comment(lib, "winmm.lib")


//////////////////////////////////////////////////////////////////////
// CTsPacketクラスの構築/消滅
//////////////////////////////////////////////////////////////////////

CTsPacket::CTsPacket()
	: CMediaData(TS_PACKETSIZE)
{
	// 空のパケットを生成する
	::ZeroMemory(&m_Header, sizeof(m_Header));
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
}

CTsPacket::CTsPacket(const BYTE *pHexData)
	: CMediaData(pHexData, TS_PACKETSIZE)
{
	// バイナリデータからパケットを生成する
	ParseHeader();
}

CTsPacket::CTsPacket(const CTsPacket &Operand)
{
	*this = Operand;
}
	
CTsPacket & CTsPacket::operator = (const CTsPacket &Operand)
{
	// インスタンスのコピー
	CMediaData::operator = (Operand);

	m_Header = Operand.m_Header;
	m_AdaptationField = Operand.m_AdaptationField;

	return *this;
}

void CTsPacket::ParseHeader(void)
{
	// TSパケットヘッダ解析
	m_Header.bySyncByte					= m_pData[0];							// +0
	m_Header.bTransportErrorIndicator	= (m_pData[1] & 0x80U)? true : false;	// +1 bit7
	m_Header.bPayloadUnitStartIndicator	= (m_pData[1] & 0x40U)? true : false;	// +1 bit6
	m_Header.TransportPriority			= (m_pData[1] & 0x20U)? true : false;	// +1 bit5
	m_Header.wPID = ((WORD)(m_pData[1] & 0x1F) << 8) | (WORD)m_pData[2];		// +1 bit4-0, +2
	m_Header.byTransportScramblingCtrl	= (m_pData[3] & 0xC0U) >> 6;			// +3 bit7-6
	m_Header.byAdaptationFieldCtrl		= (m_pData[3] & 0x30U) >> 4;			// +3 bit5-4
	m_Header.byContinuityCounter		= m_pData[3] & 0x0FU;					// +3 bit3-0
	
	// アダプテーションフィールド解析
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
	
	if(m_Header.byAdaptationFieldCtrl & 0x02){
		// アダプテーションフィールドあり
		if(m_AdaptationField.byAdaptationFieldLength = m_pData[4]){								// +4
			// フィールド長以降あり
			m_AdaptationField.bDiscontinuityIndicator	= (m_pData[5] & 0x80U)? true : false;	// +5 bit7
			m_AdaptationField.bRamdomAccessIndicator	= (m_pData[5] & 0x40U)? true : false;	// +5 bit6
			m_AdaptationField.bEsPriorityIndicator		= (m_pData[5] & 0x20U)? true : false;	// +5 bit5
			m_AdaptationField.bPcrFlag					= (m_pData[5] & 0x10U)? true : false;	// +5 bit4
			m_AdaptationField.bOpcrFlag					= (m_pData[5] & 0x08U)? true : false;	// +5 bit3
			m_AdaptationField.bSplicingPointFlag		= (m_pData[5] & 0x04U)? true : false;	// +5 bit2
			m_AdaptationField.bTransportPrivateDataFlag	= (m_pData[5] & 0x02U)? true : false;	// +5 bit1
			m_AdaptationField.bAdaptationFieldExtFlag	= (m_pData[5] & 0x01U)? true : false;	// +5 bit0
			
			if(m_pData[4] > 1U){
				m_AdaptationField.pOptionData			= &m_pData[6];
				m_AdaptationField.byOptionSize			= m_pData[4] - 1U;
				}
			}
		}
}

const bool CTsPacket::CheckPacket(BYTE *pContinuityCounter) const
{
	bool bValid = true;

	// パケットのフォーマット適合性をチェックする
	if(m_Header.bySyncByte != 0x47U)bValid = false;									// 同期バイト不正
//	else if(m_Header.bTransportErrorIndicator)bValid = false;						// ビット誤りあり		// bTransportErrorIndicatorを無視する
	else if(m_Header.wPID >= 0x0002U && m_Header.wPID <= 0x000FU)bValid = false;	// 未定義PID範囲
	else if(m_Header.byTransportScramblingCtrl == 0x01)bValid = false;				// 未定義スクランブル制御値
	else if(m_Header.byAdaptationFieldCtrl == 0x00)bValid = false;					// 未定義アダプテーションフィールド制御値
	else if(m_Header.byAdaptationFieldCtrl == 0x02U && m_AdaptationField.byAdaptationFieldLength > 183U)bValid = false;	// アダプテーションフィールド長異常
	else if(m_Header.byAdaptationFieldCtrl == 0x03U && m_AdaptationField.byAdaptationFieldLength > 182U)bValid = false;	// アダプテーションフィールド長異常

	if(!pContinuityCounter || m_Header.wPID == 0x1FFFU)return bValid;

	// 連続性チェック
	const BYTE byNewCounter = (m_Header.byAdaptationFieldCtrl & 0x01U)? m_Header.byContinuityCounter : 0x10U;

	if(!m_AdaptationField.bDiscontinuityIndicator){
		if(*pContinuityCounter < 0x10U && byNewCounter < 0x10U){
			if(((*pContinuityCounter + 1U) & 0x0FU) != byNewCounter){
				bValid = false;
				}
			}
		}

	// カウンタ更新
	*pContinuityCounter = byNewCounter;

	return bValid;
}

BYTE * CTsPacket::GetPayloadData(void) const
{
	// ペイロードポインタを返す
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// ペイロードのみ
			return &m_pData[4];
		
		case 3U :	// アダプテーションフィールド、ペイロードあり
			return &m_pData[m_AdaptationField.byAdaptationFieldLength + 5U];
		
		default :	// アダプテーションフィールドのみ or 例外
			return NULL;
		}
}

const BYTE CTsPacket::GetPayloadSize(void) const
{
	// ペイロードサイズを返す
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// ペイロードのみ
			return (TS_PACKETSIZE - 4U);
		
		case 3U :	// アダプテーションフィールド、ペイロードあり
			return (TS_PACKETSIZE - m_AdaptationField.byAdaptationFieldLength - 5U);
		
		default :	// アダプテーションフィールドのみ or 例外
			return 0U;
		}
}

const WORD CTsPacket::GetPID(void) const
{
	// PIDを返す
	return m_Header.wPID;
}

const bool CTsPacket::HaveAdaptationField(void) const
{
	// アダプテーションフィールドの有無を返す
	return (m_Header.byAdaptationFieldCtrl & 0x02U)? true : false;
}

const bool CTsPacket::HavePayload(void) const
{
	// ペイロードの有無を返す
	return (m_Header.byAdaptationFieldCtrl & 0x01U)? true : false;
}

const bool CTsPacket::IsScrambled(void) const
{
	// スクランブル有無を返す
	return (m_Header.byTransportScramblingCtrl & 0x02U)? true : false;
}


//////////////////////////////////////////////////////////////////////
// CPsiSectionクラスの構築/消滅
//////////////////////////////////////////////////////////////////////

CPsiSection::CPsiSection()
	: CMediaData()
{
	Reset();
}

CPsiSection::CPsiSection(const DWORD dwBuffSize)
	: CMediaData(dwBuffSize)
{
	Reset();
}

CPsiSection::CPsiSection(const CPsiSection &Operand)
{
	Reset();

	*this = Operand;
}

CPsiSection & CPsiSection::operator = (const CPsiSection &Operand)
{
	// インスタンスのコピー
	CMediaData::operator = (Operand);
	m_Header = Operand.m_Header;

	return *this;
}

const bool CPsiSection::operator == (const CPsiSection &Operand) const
{
	// セクションの内容を比較する
	if(GetPayloadSize() != Operand.GetPayloadSize()){
		// サイズが異なる
		return false;
		}

	const BYTE *pSrcData = GetPayloadData();
	const BYTE *pDstData = Operand.GetPayloadData();
	
	if((pSrcData == pDstData) && !pSrcData){
		// いずれもNULL
		return true;
		}
	
	if(!pSrcData || !pDstData){
		// 一方だけNULL
		return false;
		}
	
	// バイナリ比較
	for(DWORD dwPos = 0 ; dwPos < GetPayloadSize() ; dwPos++){
		if(pSrcData[dwPos] != pDstData[dwPos])return false;
		}
		
	// 一致する
	return true;
}

const bool CPsiSection::ParseHeader(const bool bIsExtended)
{
	const DWORD dwHeaderSize = (bIsExtended)? 8UL : 3UL;

	// ヘッダサイズチェック
	if(m_dwDataSize < dwHeaderSize)return false;

	// 標準形式ヘッダ解析
	m_Header.byTableID					= m_pData[0];									// +0
	m_Header.bSectionSyntaxIndicator	= (m_pData[1] & 0x80U)? true : false;			// +1 bit7
	m_Header.bPrivateIndicator			= (m_pData[1] & 0x40U)? true : false;			// +1 bit6
	m_Header.wSectionLength = ((WORD)(m_pData[1] & 0x0FU) << 8) | (WORD)m_pData[2];		// +1 bit5-0, +2

	if(m_Header.bSectionSyntaxIndicator && bIsExtended){
		// 拡張形式のヘッダ解析
		m_Header.wTableIdExtension		= (WORD)m_pData[3] << 8 | (WORD)m_pData[4];		// +3, +4
		m_Header.byVersionNo			= (m_pData[5] & 0x3EU) >> 1;					// +5 bit5-1
		m_Header.bCurrentNextIndicator	= (m_pData[5] & 0x01U)? true : false;			// +5 bit0
		m_Header.bySectionNumber		= m_pData[6];									// +6
		m_Header.byLastSectionNumber	= m_pData[7];									// +7
		}

	// ヘッダのフォーマット適合性をチェックする
	if((m_pData[1] & 0x30U) != 0x30U)return false;										// 固定ビット異常
	else if(m_Header.wSectionLength > 4093U)return false;								// セクション長異常
	else if(m_Header.bSectionSyntaxIndicator){
		// 拡張形式のエラーチェック
		if(!bIsExtended)return false;													// 目的のヘッダではない
		else if((m_pData[5] & 0xC0U) != 0xC0U)return false;								// 固定ビット異常
		else if(m_Header.bySectionNumber > m_Header.byLastSectionNumber)return false;	// セクション番号異常
		else if(m_Header.wSectionLength < 9U)return false;								// セクション長異常
		}
	else{
		// 標準形式のエラーチェック	
		if(bIsExtended)return false;													// 目的のヘッダではない
		else if(m_Header.wSectionLength < 4U)return false;								// セクション長異常
		}

	return true;
}

void CPsiSection::Reset(void)
{
	// データをクリアする
	ClearSize();	
	::ZeroMemory(&m_Header, sizeof(m_Header));
}

BYTE * CPsiSection::GetPayloadData(void) const
{
	// ペイロードポインタを返す
	const DWORD dwHeaderSize = (m_Header.bSectionSyntaxIndicator)? 8UL : 3UL;

	return (m_dwDataSize > dwHeaderSize)? &m_pData[dwHeaderSize] : NULL;
}

const WORD CPsiSection::GetPayloadSize(void) const
{
	// ペイロードサイズを返す(実際に保持してる　※セクション長より少なくなることもある)
	const DWORD dwHeaderSize = (m_Header.bSectionSyntaxIndicator)? 8UL : 3UL;

	if(m_dwDataSize <= dwHeaderSize)return 0U;
	else if(m_Header.bSectionSyntaxIndicator){
		// 拡張セクション
		return (m_dwDataSize >= (m_Header.wSectionLength + 3UL))? (m_Header.wSectionLength - 9U) : ((WORD)m_dwDataSize - 8U);
		}
	else{
		// 標準セクション
		return (m_dwDataSize >= (m_Header.wSectionLength + 3UL))? m_Header.wSectionLength : ((WORD)m_dwDataSize - 3U);
		}
}

const BYTE CPsiSection::GetTableID(void) const
{
	// テーブルIDを返す
	return m_Header.byTableID;
}

const bool CPsiSection::IsExtendedSection(void) const
{
	// セクションシンタックスインジケータを返す
	return m_Header.bSectionSyntaxIndicator;
}

const bool CPsiSection::IsPrivate(void) const
{
	// プライベートインジケータを返す
	return m_Header.bPrivateIndicator;
}

const WORD CPsiSection::GetSectionLength(void) const
{
	// セクション長を返す
	return m_Header.wSectionLength;
}

const WORD CPsiSection::GetTableIdExtension(void) const
{
	// テーブルID拡張を返す
	return m_Header.wTableIdExtension;
}

const BYTE CPsiSection::GetVersionNo(void) const
{
	// バージョン番号を返す
	return m_Header.byVersionNo;
}

const bool CPsiSection::IsCurrentNext(void) const
{
	// カレントネクストインジケータを返す
	return m_Header.bCurrentNextIndicator;
}

const BYTE CPsiSection::GetSectionNumber(void) const
{
	// セクション番号を返す
	return m_Header.bySectionNumber;
}

const BYTE CPsiSection::GetLastSectionNumber(void) const
{
	// ラストセクション番号を返す
	return m_Header.byLastSectionNumber;
}


/////////////////////////////////////////////////////////////////////////////
// TS PIDマップ対象クラス
/////////////////////////////////////////////////////////////////////////////

void CTsPidMapTarget::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	// マッピングされた
}

void CTsPidMapTarget::OnPidUnmapped(const WORD wPID)
{
	// マップ解除された
}


/////////////////////////////////////////////////////////////////////////////
// TS PIDマップ管理クラス
/////////////////////////////////////////////////////////////////////////////

CTsPidMapManager::CTsPidMapManager()
	: m_wMapCount(0U)
{
	::ZeroMemory(m_PidMap, sizeof(m_PidMap));
}

CTsPidMapManager::~CTsPidMapManager()
{
	UnmapAllTarget();
}

const bool CTsPidMapManager::StorePacket(const CTsPacket *pPacket)
{
	const WORD wPID = pPacket->GetPID();
	
	if(wPID > 0x1FFFU)return false;					// PID範囲外
	
	if(!m_PidMap[wPID].pMapTarget)return false;		// PIDマップターゲットなし

	if(m_PidMap[wPID].pMapTarget->StorePacket(pPacket)){
		// ターゲットの更新があったときはコールバックを呼び出す
		
		if(m_PidMap[wPID].pMapCallback){
			m_PidMap[wPID].pMapCallback(wPID, m_PidMap[wPID].pMapTarget, this, m_PidMap[wPID].pMapParam);
			}
		}

	return true;
}

const bool CTsPidMapManager::MapTarget(const WORD wPID, CTsPidMapTarget *pMapTarget, const PIDMAPHANDLERFUNC pMapCallback, const PVOID pMapParam)
{
	if((wPID > 0x1FFFU) || (!pMapTarget))return false;

	// 現在のターゲットをアンマップ
	UnmapTarget(wPID);

	// 新しいターゲットをマップ
	m_PidMap[wPID].pMapTarget = pMapTarget;
	m_PidMap[wPID].pMapCallback = pMapCallback;
	m_PidMap[wPID].pMapParam = pMapParam;
	m_wMapCount++;
	
	pMapTarget->OnPidMapped(wPID, pMapParam);

	return true;
}

const bool CTsPidMapManager::UnmapTarget(const WORD wPID)
{
	if(wPID > 0x1FFFU)return false;

	if(!m_PidMap[wPID].pMapTarget)return false;

	// 現在のターゲットをアンマップ
	CTsPidMapTarget *pTarget = m_PidMap[wPID].pMapTarget;
	::ZeroMemory(&m_PidMap[wPID], sizeof(m_PidMap[wPID]));
	m_wMapCount--;

	pTarget->OnPidUnmapped(wPID);

	return true;
}

void CTsPidMapManager::UnmapAllTarget(void)
{
	// 全ターゲットをマップ
	for(WORD wPID = 0x0000U ; wPID <= 0x1FFFU ; wPID++){
		UnmapTarget(wPID);
		}
}

CTsPidMapTarget * CTsPidMapManager::GetMapTarget(const WORD wPID) const
{
	// マップされているターゲットを返す
	return (wPID <= 0x1FFFU)? m_PidMap[wPID].pMapTarget : NULL;
}

const WORD CTsPidMapManager::GetMapCount(void) const
{
	// マップされているPIDの総数を返す
	return m_wMapCount;
}


/////////////////////////////////////////////////////////////////////////////
// PSIセクション抽出クラス
/////////////////////////////////////////////////////////////////////////////

CPsiSectionParser::CPsiSectionParser()
	: m_PsiSection(0x10002UL)		// PSIセクション最大サイズのバッファ確保
	, m_pfnSectionRecvFunc(NULL)
	, m_pCallbackParam(NULL)
	, m_bTargetExt(true)
	, m_bIsStoring(false)
	, m_dwCrcErrorCount(0UL)
{

}

CPsiSectionParser::CPsiSectionParser(const CPsiSectionParser &Operand)
{
	*this = Operand;
}

CPsiSectionParser & CPsiSectionParser::operator = (const CPsiSectionParser &Operand)
{
	// インスタンスのコピー
	m_pfnSectionRecvFunc = Operand.m_pfnSectionRecvFunc;
	m_pCallbackParam = Operand.m_pCallbackParam;
	m_bTargetExt = Operand.m_bTargetExt;
	m_PsiSection = Operand.m_PsiSection;
	m_bIsStoring = Operand.m_bIsStoring;
	m_dwStoreCrc = Operand.m_dwStoreCrc;
	m_wStoreSize = Operand.m_wStoreSize;
	m_dwCrcErrorCount = Operand.m_dwCrcErrorCount;

	return *this;
}

void CPsiSectionParser::SetRecvCallback(const bool bTargetExt, const SECTIONRECVFUNC pCallback, const PVOID pParam)
{
	// 処理対象セクションの種類(拡張 or 標準)、セクションを受け取るコールバック、パラメータを設定する
	m_bTargetExt = bTargetExt;
	m_pfnSectionRecvFunc = pCallback;
	m_pCallbackParam = pParam;
}

void CPsiSectionParser::StorePacket(const CTsPacket *pPacket)
{
	const BYTE *pData = pPacket->GetPayloadData();
	const BYTE bySize = pPacket->GetPayloadSize();
	if(!bySize || !pData)return;

	const BYTE byUnitStartPos = (pPacket->m_Header.bPayloadUnitStartIndicator)? (pData[0] + 1U) : 0U;

	if(byUnitStartPos){
		// [ヘッダ断片 | ペイロード断片] + [スタッフィングバイト] + ヘッダ先頭 + [ヘッダ断片] + [ペイロード断片] + [スタッフィングバイト]
		BYTE byPos = 1U;
		
		if(byUnitStartPos > 1U){
			// ユニット開始位置が先頭ではない場合(断片がある場合)
			byPos += StoreHeader(&pData[byPos], bySize - byPos);
			byPos += StorePayload(&pData[byPos], bySize - byPos);
			}
		
		// ユニット開始位置から新規セクションのストアを開始する
		m_bIsStoring = false;
		m_PsiSection.ClearSize();

		byPos = byUnitStartPos;
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}
	else{
		// [ヘッダ断片] + ペイロード + [スタッフィングバイト]
		BYTE byPos = 0U;
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}

	return;
}

void CPsiSectionParser::Reset(void)
{
	// 状態を初期化する
	m_bIsStoring = false;
	m_wStoreSize = 0U;
	m_dwCrcErrorCount = 0UL;
	m_PsiSection.Reset();
}

const DWORD CPsiSectionParser::GetCrcErrorCount(void) const
{
	// CRCエラー回数を返す
	return m_dwCrcErrorCount;
}

const BYTE CPsiSectionParser::StoreHeader(const BYTE *pPayload, const BYTE byRemain)
{
	// ヘッダを解析してセクションのストアを開始する
	if(m_bIsStoring)return 0U;

	const BYTE byHeaderSize = (m_bTargetExt)? 8U : 3U;
	const BYTE byHeaderRemain = byHeaderSize - (BYTE)m_PsiSection.GetSize();

	if(byRemain >= byHeaderRemain){
		// ヘッダストア完了、ヘッダを解析してペイロードのストアを開始する
		m_PsiSection.AddData(pPayload, byHeaderRemain);
		if(m_PsiSection.ParseHeader(m_bTargetExt)){
			// ヘッダフォーマットOK、ヘッダのみのCRCを計算する
			m_wStoreSize = m_PsiSection.GetSectionLength() + 3U;
			m_dwStoreCrc = CalcCrc(pPayload, byHeaderSize);
			m_bIsStoring = true;
			return byHeaderRemain;
			}
		else{
			// ヘッダエラー
			m_PsiSection.Reset();
			return byRemain;
			}
		}
	else{
		// ヘッダストア未完了、次のデータを待つ
		m_PsiSection.AddData(pPayload, byRemain);
		return byRemain;
		}
}

const BYTE CPsiSectionParser::StorePayload(const BYTE *pPayload, const BYTE byRemain)
{
	// セクションのストアを完了する
	if(!m_bIsStoring)return 0U;
	
	const WORD wStoreRemain = m_wStoreSize - (WORD)m_PsiSection.GetSize();

	if(wStoreRemain <= (WORD)byRemain){
		// ストア完了
		m_PsiSection.AddData(pPayload, wStoreRemain);
				
		if(!CalcCrc(pPayload, wStoreRemain, m_dwStoreCrc)){
			// CRC正常、コールバックにセクションを渡す
			if(m_pfnSectionRecvFunc)m_pfnSectionRecvFunc(&m_PsiSection, m_pCallbackParam);
			//TRACE(TEXT("[%02X] PSI Stored: %lu / %lu\n"), m_PsiSection.GetTableID(), m_PsiSection.GetSize(), (DWORD)m_wStoreSize);
			}
		else{
			// CRC異常
			if(m_dwCrcErrorCount < 0xFFFFFFFFUL)m_dwCrcErrorCount++;
			//TRACE(TEXT("[%02X] PSI CRC Error: %lu / %lu\n"), m_PsiSection.GetTableID(), m_PsiSection.GetSize(), (DWORD)m_wStoreSize);
			}
		
		// 状態を初期化し、次のセクション受信に備える
		m_PsiSection.Reset();
		m_bIsStoring = false;

		return (BYTE)wStoreRemain;
		}
	else{
		// ストア未完了、次のペイロードを待つ
		m_PsiSection.AddData(pPayload, byRemain);
		m_dwStoreCrc = CalcCrc(pPayload, byRemain, m_dwStoreCrc);
		return byRemain;
		}
}

const DWORD CPsiSectionParser::CalcCrc(const BYTE *pData, const WORD wDataSize, DWORD dwCurCrc)
{
	// CRC32計算(「Mpeg2-TSのストリームからデータ放送情報を抽出するテスト」からコードを流用、計算を分割できるように拡張)
	static const DWORD CrcTable[256] = {
		0x00000000UL, 0x04C11DB7UL, 0x09823B6EUL, 0x0D4326D9UL,	0x130476DCUL, 0x17C56B6BUL, 0x1A864DB2UL, 0x1E475005UL,	0x2608EDB8UL, 0x22C9F00FUL, 0x2F8AD6D6UL, 0x2B4BCB61UL,	0x350C9B64UL, 0x31CD86D3UL, 0x3C8EA00AUL, 0x384FBDBDUL,
		0x4C11DB70UL, 0x48D0C6C7UL, 0x4593E01EUL, 0x4152FDA9UL,	0x5F15ADACUL, 0x5BD4B01BUL, 0x569796C2UL, 0x52568B75UL,	0x6A1936C8UL, 0x6ED82B7FUL, 0x639B0DA6UL, 0x675A1011UL,	0x791D4014UL, 0x7DDC5DA3UL, 0x709F7B7AUL, 0x745E66CDUL,
		0x9823B6E0UL, 0x9CE2AB57UL, 0x91A18D8EUL, 0x95609039UL,	0x8B27C03CUL, 0x8FE6DD8BUL, 0x82A5FB52UL, 0x8664E6E5UL,	0xBE2B5B58UL, 0xBAEA46EFUL, 0xB7A96036UL, 0xB3687D81UL,	0xAD2F2D84UL, 0xA9EE3033UL, 0xA4AD16EAUL, 0xA06C0B5DUL,
		0xD4326D90UL, 0xD0F37027UL, 0xDDB056FEUL, 0xD9714B49UL,	0xC7361B4CUL, 0xC3F706FBUL, 0xCEB42022UL, 0xCA753D95UL,	0xF23A8028UL, 0xF6FB9D9FUL, 0xFBB8BB46UL, 0xFF79A6F1UL,	0xE13EF6F4UL, 0xE5FFEB43UL, 0xE8BCCD9AUL, 0xEC7DD02DUL,
		0x34867077UL, 0x30476DC0UL, 0x3D044B19UL, 0x39C556AEUL,	0x278206ABUL, 0x23431B1CUL, 0x2E003DC5UL, 0x2AC12072UL,	0x128E9DCFUL, 0x164F8078UL, 0x1B0CA6A1UL, 0x1FCDBB16UL,	0x018AEB13UL, 0x054BF6A4UL, 0x0808D07DUL, 0x0CC9CDCAUL,
		0x7897AB07UL, 0x7C56B6B0UL, 0x71159069UL, 0x75D48DDEUL,	0x6B93DDDBUL, 0x6F52C06CUL, 0x6211E6B5UL, 0x66D0FB02UL,	0x5E9F46BFUL, 0x5A5E5B08UL, 0x571D7DD1UL, 0x53DC6066UL,	0x4D9B3063UL, 0x495A2DD4UL, 0x44190B0DUL, 0x40D816BAUL,
		0xACA5C697UL, 0xA864DB20UL, 0xA527FDF9UL, 0xA1E6E04EUL,	0xBFA1B04BUL, 0xBB60ADFCUL, 0xB6238B25UL, 0xB2E29692UL,	0x8AAD2B2FUL, 0x8E6C3698UL, 0x832F1041UL, 0x87EE0DF6UL,	0x99A95DF3UL, 0x9D684044UL, 0x902B669DUL, 0x94EA7B2AUL,
		0xE0B41DE7UL, 0xE4750050UL, 0xE9362689UL, 0xEDF73B3EUL,	0xF3B06B3BUL, 0xF771768CUL, 0xFA325055UL, 0xFEF34DE2UL,	0xC6BCF05FUL, 0xC27DEDE8UL, 0xCF3ECB31UL, 0xCBFFD686UL,	0xD5B88683UL, 0xD1799B34UL, 0xDC3ABDEDUL, 0xD8FBA05AUL,
		0x690CE0EEUL, 0x6DCDFD59UL, 0x608EDB80UL, 0x644FC637UL,	0x7A089632UL, 0x7EC98B85UL, 0x738AAD5CUL, 0x774BB0EBUL,	0x4F040D56UL, 0x4BC510E1UL, 0x46863638UL, 0x42472B8FUL,	0x5C007B8AUL, 0x58C1663DUL, 0x558240E4UL, 0x51435D53UL,
		0x251D3B9EUL, 0x21DC2629UL, 0x2C9F00F0UL, 0x285E1D47UL,	0x36194D42UL, 0x32D850F5UL, 0x3F9B762CUL, 0x3B5A6B9BUL,	0x0315D626UL, 0x07D4CB91UL, 0x0A97ED48UL, 0x0E56F0FFUL,	0x1011A0FAUL, 0x14D0BD4DUL, 0x19939B94UL, 0x1D528623UL,
		0xF12F560EUL, 0xF5EE4BB9UL, 0xF8AD6D60UL, 0xFC6C70D7UL,	0xE22B20D2UL, 0xE6EA3D65UL, 0xEBA91BBCUL, 0xEF68060BUL,	0xD727BBB6UL, 0xD3E6A601UL, 0xDEA580D8UL, 0xDA649D6FUL,	0xC423CD6AUL, 0xC0E2D0DDUL, 0xCDA1F604UL, 0xC960EBB3UL,
		0xBD3E8D7EUL, 0xB9FF90C9UL, 0xB4BCB610UL, 0xB07DABA7UL,	0xAE3AFBA2UL, 0xAAFBE615UL, 0xA7B8C0CCUL, 0xA379DD7BUL,	0x9B3660C6UL, 0x9FF77D71UL, 0x92B45BA8UL, 0x9675461FUL,	0x8832161AUL, 0x8CF30BADUL, 0x81B02D74UL, 0x857130C3UL,
		0x5D8A9099UL, 0x594B8D2EUL, 0x5408ABF7UL, 0x50C9B640UL,	0x4E8EE645UL, 0x4A4FFBF2UL, 0x470CDD2BUL, 0x43CDC09CUL,	0x7B827D21UL, 0x7F436096UL, 0x7200464FUL, 0x76C15BF8UL,	0x68860BFDUL, 0x6C47164AUL, 0x61043093UL, 0x65C52D24UL,
		0x119B4BE9UL, 0x155A565EUL, 0x18197087UL, 0x1CD86D30UL,	0x029F3D35UL, 0x065E2082UL, 0x0B1D065BUL, 0x0FDC1BECUL,	0x3793A651UL, 0x3352BBE6UL, 0x3E119D3FUL, 0x3AD08088UL,	0x2497D08DUL, 0x2056CD3AUL, 0x2D15EBE3UL, 0x29D4F654UL,
		0xC5A92679UL, 0xC1683BCEUL, 0xCC2B1D17UL, 0xC8EA00A0UL,	0xD6AD50A5UL, 0xD26C4D12UL, 0xDF2F6BCBUL, 0xDBEE767CUL,	0xE3A1CBC1UL, 0xE760D676UL, 0xEA23F0AFUL, 0xEEE2ED18UL,	0xF0A5BD1DUL, 0xF464A0AAUL, 0xF9278673UL, 0xFDE69BC4UL,
		0x89B8FD09UL, 0x8D79E0BEUL, 0x803AC667UL, 0x84FBDBD0UL,	0x9ABC8BD5UL, 0x9E7D9662UL, 0x933EB0BBUL, 0x97FFAD0CUL,	0xAFB010B1UL, 0xAB710D06UL, 0xA6322BDFUL, 0xA2F33668UL,	0xBCB4666DUL, 0xB8757BDAUL, 0xB5365D03UL, 0xB1F740B4UL
		};		

	for(WORD wPos = 0U ; wPos < wDataSize ; wPos++){
		dwCurCrc = (dwCurCrc << 8) ^ CrcTable[ (dwCurCrc >> 24) ^ pData[wPos] ];
		}

	return dwCurCrc;
}


/////////////////////////////////////////////////////////////////////////////
// PCR抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CTsClockRef::CTsClockRef()
{
	InitPcrPll(0LL);
}

CTsClockRef::CTsClockRef(const CTsClockRef &Operand)
{
	*this = Operand;
}

CTsClockRef & CTsClockRef::operator = (const CTsClockRef &Operand)
{
	// インスタンスのコピー
	m_llHrcUnitFreq = Operand.m_llHrcUnitFreq;
	m_llHrcLastTime = Operand.m_llHrcLastTime;
	m_llCurPcrCount = Operand.m_llCurPcrCount;
	m_lfPllFeedBack = Operand.m_lfPllFeedBack;	

	return *this;
}

const bool CTsClockRef::StorePacket(const CTsPacket *pPacket, const WORD wPcrPID)
{
	if(pPacket->GetPID() != wPcrPID)return false;
	if(!pPacket->m_AdaptationField.bPcrFlag)return false;

	// 33bit 90KHz PCRを計算
	const LONGLONG llCurPcrCount = GetPcrFromHex(pPacket->m_AdaptationField.pOptionData);

	if(llCurPcrCount < 0LL){
		// PCRなし(エラー)
		TRACE(TEXT("PCRなし(エラー)\n"));
		return true;
		}
	else if(!m_llCurPcrCount){
		// PCR PLL初期化
		InitPcrPll(llCurPcrCount);
		TRACE(TEXT("PLL初期化\n"));
		}
	else if(pPacket->m_AdaptationField.bDiscontinuityIndicator){
		// PCR PLL再同期
		SyncPcrPll(llCurPcrCount);
		TRACE(TEXT("PLL再同期\n"));
		}
	else{
		// PCR PLL処理
		ProcPcrPll(llCurPcrCount);
		}

	return true;
}

void CTsClockRef::Reset(void)
{
	InitPcrPll(0LL);
}

const LONGLONG CTsClockRef::GetGlobalPcr(void) const
{
	// 現在時刻からグローバルPCRを取得する
	LONGLONG llHrcCurTime;
	::QueryPerformanceCounter((LARGE_INTEGER *)&llHrcCurTime);

	// 最後に更新されたPCR + 更新からの経過時間 = 現在のPCR
	return (m_llGlobalPcrCount + (LONGLONG)(((double)(llHrcCurTime - m_llHrcLastTime) * 90000.0) / (double)m_llHrcUnitFreq));
}

const LONGLONG CTsClockRef::GetCurrentPcr(void) const
{
	// 現在時刻からPCRを取得する
	LONGLONG llHrcCurTime;
	::QueryPerformanceCounter((LARGE_INTEGER *)&llHrcCurTime);

	// 最後に更新されたPCR + 更新からの経過時間 = 現在のPCR
	return (m_llCurPcrCount + (LONGLONG)(((double)(llHrcCurTime - m_llHrcLastTime) * 90000.0) / (double)m_llHrcUnitFreq));
}

const LONGLONG CTsClockRef::PtsToGlobalPcr(const LONGLONG llPts) const
{
	// PTSをPCRに変換する
	if(llPts > m_llCurPcrCount){
		// グローバルPCRにPCRのオフセットを加算した値を返す
		return m_llGlobalPcrCount + (llPts - m_llCurPcrCount);
		}
	else{
		// 既に時刻を過ぎている(エラー or 大幅な処理遅れ)
		LONGLONG llHrcCurTime;
		::QueryPerformanceCounter((LARGE_INTEGER *)&llHrcCurTime);

		// 最後に更新されたPCR + 更新からの経過時間 = 現在のPCR
		return (m_llGlobalPcrCount + (LONGLONG)(((double)(llHrcCurTime - m_llHrcLastTime) * 90000.0) / (double)m_llHrcUnitFreq));		
		}
}

void CTsClockRef::InitPcrPll(const LONGLONG llCurPcr)
{
	// PLLを初期化する
	::QueryPerformanceFrequency((LARGE_INTEGER *)&m_llHrcUnitFreq);
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_llHrcLastTime);

	m_llCurPcrCount = llCurPcr;
	m_lfPllFeedBack = 0.0;

	m_llGlobalPcrCount = 0LL;
	m_llBasePcrCount = llCurPcr;
}

void CTsClockRef::ProcPcrPll(const LONGLONG llCurPcr)
{
	// PLLを処理する
	ULONGLONG llHrcCurTime;
	::QueryPerformanceCounter((LARGE_INTEGER *)&llHrcCurTime);

	// ローカルPCRを計算する(高分解能タイマによる実周期の積分値)
	m_llCurPcrCount += (LONGLONG)(((double)(llHrcCurTime - m_llHrcLastTime) * 90000.0) / (double)m_llHrcUnitFreq);	// 50ms
	m_llHrcLastTime = llHrcCurTime;
	
	// ローカルPCRとストリームPCRの位相差にLPFを施す(フィードバックゲイン = -40dB、時定数 = 約5.5s)
	m_lfPllFeedBack = m_lfPllFeedBack * 0.99 + (double)(m_llCurPcrCount - llCurPcr) * 0.01;
	
	// ローカルPCRに位相差をフィードバックする
	m_llCurPcrCount -= (LONGLONG)m_lfPllFeedBack;
	
	// グローバルPCRを更新する
	m_llGlobalPcrCount = m_llCurPcrCount - m_llBasePcrCount;
}

void CTsClockRef::SyncPcrPll(const LONGLONG llCurPcr)
{
	// PCR不連続発生時にPLLを再同期する
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_llHrcLastTime);

	// PLL初期値設定
	m_llCurPcrCount = llCurPcr;
	m_lfPllFeedBack = 0.0;
	
	// グローバルPCRの基点再設定
	m_llBasePcrCount = llCurPcr;
}

inline const LONGLONG CTsClockRef::GetPcrFromHex(const BYTE *pPcrData)
{
	// PCRを解析する(42bit 27MHz)
	LONGLONG llCurPcrCount = 0LL;
	llCurPcrCount |= (LONGLONG)pPcrData[0] << 34;
	llCurPcrCount |= (LONGLONG)pPcrData[1] << 26;
	llCurPcrCount |= (LONGLONG)pPcrData[2] << 18;
	llCurPcrCount |= (LONGLONG)pPcrData[3] << 10;
	llCurPcrCount |= (LONGLONG)(pPcrData[4] & 0x80U) << 2;
	llCurPcrCount |= (LONGLONG)(pPcrData[4] & 0x01U) << 8;
	llCurPcrCount |= (LONGLONG)pPcrData[5];

	// 33bit 90KHzにシフトする(42bitでは演算誤差が蓄積して使い物にならない)
	return llCurPcrCount >> 9;
}
