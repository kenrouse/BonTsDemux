// TsDescriptor.h: 記述子ラッパークラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsEncode.h"
#include "TsDescriptor.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// 記述子の基底クラス
/////////////////////////////////////////////////////////////////////////////

CBaseDesc::CBaseDesc()
{
	Reset();
}

CBaseDesc::CBaseDesc(const CBaseDesc &Operand)
{
	// コピーコンストラクタ
	*this = Operand;
}

CBaseDesc::~CBaseDesc()
{

}

CBaseDesc & CBaseDesc::operator = (const CBaseDesc &Operand)
{
	// 代入演算子
	CopyDesc(&Operand);

	return *this;
}

void CBaseDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// インスタンスのコピー
	m_byDescTag = pOperand->m_byDescTag;
	m_byDescLen = pOperand->m_byDescLen;
	m_bIsValid = pOperand->m_bIsValid;
}

const bool CBaseDesc::ParseDesc(const BYTE *pHexData, const WORD wDataLength)
{
	Reset();
	
	// 共通フォーマットをチェック
	if(!pHexData)return false;										// データが空
	else if(wDataLength < 2U)return false;							// データが最低記述子サイズ未満
	else if(wDataLength < (WORD)(pHexData[1] + 2U))return false;	// データが記述子のサイズよりも小さい

	m_byDescTag = pHexData[0];
	m_byDescLen = pHexData[1];

	// ペイロード解析
	if(StoreContents(&pHexData[2])){
		m_bIsValid = true;
		}
	
	return m_bIsValid;
}

const bool CBaseDesc::IsValid(void) const
{
	// データが有効(解析済)かどうかを返す
	return m_bIsValid;
}

const BYTE CBaseDesc::GetTag(void) const
{
	// 記述子タグを返す
	return m_byDescTag;
}

const BYTE CBaseDesc::GetLength(void) const
{
	// 記述子長を返す
	return m_byDescLen;
}

void CBaseDesc::Reset(void)
{
	// 状態をクリアする
	m_byDescTag = 0x00U;
	m_byDescLen = 0U;
	m_bIsValid = false;
}

const bool CBaseDesc::StoreContents(const BYTE *pPayload)
{
	// デフォルトの実装では何もしない
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access 記述子抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CCaMethodDesc::CCaMethodDesc()
	: CBaseDesc()
{
	Reset();
}

CCaMethodDesc::CCaMethodDesc(const CCaMethodDesc &Operand)
{
	*this = Operand;
}

CCaMethodDesc & CCaMethodDesc::operator = (const CCaMethodDesc &Operand)
{
	CopyDesc(&Operand);

	return *this;
}

void CCaMethodDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// インスタンスのコピー
	CBaseDesc::CopyDesc(pOperand);

	const CCaMethodDesc *pSrcDesc = dynamic_cast<const CCaMethodDesc *>(pOperand);
	
	if(pSrcDesc){
		m_wCaMethodID = pSrcDesc->m_wCaMethodID;
		m_wCaPID = pSrcDesc->m_wCaPID;
		m_PrivateData = pSrcDesc->m_PrivateData;
		}
}

void CCaMethodDesc::Reset(void)
{
	CBaseDesc::Reset();

	m_wCaMethodID = 0x0000U;		// Conditional Access Method ID
	m_wCaPID = 0xFFFFU;				// Conditional Access PID
	m_PrivateData.ClearSize();		// Private Data
}

const WORD CCaMethodDesc::GetCaMethodID(void) const
{
	// Conditional Access Method ID を返す
	return m_wCaMethodID;
}

const WORD CCaMethodDesc::GetCaPID(void) const
{
	// Conditional Access PID
	return m_wCaPID;
}

const CMediaData * CCaMethodDesc::GetPrivateData(void) const
{
	// Private Data を返す
	return &m_PrivateData;
}

const bool CCaMethodDesc::StoreContents(const BYTE *pPayload)
{
	// フォーマットをチェック
	if(m_byDescTag != 0x09U)return false;								// タグが不正
	else if(m_byDescLen < 4U)return false;								// CAメソッド記述子の最小サイズは4
	else if((pPayload[2] & 0xE0U) != 0xE0U)return false;				// 固定ビットが不正

	// 記述子を解析
	m_wCaMethodID = (WORD)pPayload[0] << 8 | (WORD)pPayload[1];			// +0,1	Conditional Access Method ID
	m_wCaPID = (WORD)(pPayload[2] & 0x1FU) << 8 | (WORD)pPayload[3];	// +2,3	Conditional Access PID
	m_PrivateData.SetData(&pPayload[4], m_byDescLen - 4U);				// +4-	Private Data

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service 記述子抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CServiceDesc::CServiceDesc()
	: CBaseDesc()
{
	Reset();
}

CServiceDesc::CServiceDesc(const CServiceDesc &Operand)
{
	*this = Operand;
}

CServiceDesc & CServiceDesc::operator = (const CServiceDesc &Operand)
{
	CopyDesc(&Operand);
	
	return *this;
}

void CServiceDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// インスタンスのコピー
	CBaseDesc::CopyDesc(pOperand);

	const CServiceDesc *pSrcDesc = dynamic_cast<const CServiceDesc *>(pOperand);
	
	if(pSrcDesc){
		m_byServiceType = pSrcDesc->m_byServiceType;
		::lstrcpy(m_szProviderName, pSrcDesc->m_szProviderName);
		::lstrcpy(m_szServiceName, pSrcDesc->m_szServiceName);
		}
}

void CServiceDesc::Reset(void)
{
	CBaseDesc::Reset();
	
	m_byServiceType = 0x00U;			// Service Type
	m_szProviderName[0] = TEXT('\0');	// Service Provider Name
	m_szServiceName[0] = TEXT('\0');	// Service Name
}

const BYTE CServiceDesc::GetServiceType(void) const
{
	// Service Typeを返す
	return m_byServiceType;
}

const DWORD CServiceDesc::GetProviderName(LPTSTR lpszDst) const
{
	// Service Provider Nameを返す
	if(lpszDst)::lstrcpy(lpszDst, m_szProviderName);

	return ::lstrlen(m_szProviderName);
}

const DWORD CServiceDesc::GetServiceName(LPTSTR lpszDst) const
{
	// Service Provider Nameを返す
	if(lpszDst)::lstrcpy(lpszDst, m_szServiceName);

	return ::lstrlen(m_szServiceName);
}

const bool CServiceDesc::StoreContents(const BYTE *pPayload)
{
	// フォーマットをチェック
	if(m_byDescTag != 0x48U)return false;		// タグが不正
	else if(m_byDescLen < 3U)return false;		// サービス記述子のサイズは最低3

	// 記述子を解析
	m_byServiceType = pPayload[0];				// +0	Service Type
	
	BYTE byPos = 1U;
	
	// Provider Name
	if(pPayload[byPos + 0]){
		CAribString::AribToString(m_szProviderName, &pPayload[byPos + 1], pPayload[byPos + 0]);
		byPos += pPayload[byPos + 0];
		}
	
	byPos++;

	// Service Name
	if(pPayload[byPos + 0]){
		CAribString::AribToString(m_szServiceName, &pPayload[byPos + 1], pPayload[byPos + 0]);
		byPos += pPayload[byPos + 0];
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier 記述子抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CStreamIdDesc::CStreamIdDesc()
	: CBaseDesc()
{
	Reset();
}

CStreamIdDesc::CStreamIdDesc(const CStreamIdDesc &Operand)
{
	*this = Operand;
}

CStreamIdDesc & CStreamIdDesc::operator = (const CStreamIdDesc &Operand)
{
	CopyDesc(&Operand);

	return *this;
}

void CStreamIdDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// インスタンスのコピー
	CBaseDesc::CopyDesc(pOperand);

	const CStreamIdDesc *pSrcDesc = dynamic_cast<const CStreamIdDesc *>(pOperand);
	
	if(pSrcDesc){
		m_byComponentTag = pSrcDesc->m_byComponentTag;
		}
}

void CStreamIdDesc::Reset(void)
{
	CBaseDesc::Reset();

	m_byComponentTag = 0x00U;	// Component Tag
}

const BYTE CStreamIdDesc::GetComponentTag(void) const
{
	// Component Tag を返す
	return m_byComponentTag;
}

const bool CStreamIdDesc::StoreContents(const BYTE *pPayload)
{
	// フォーマットをチェック
	if(m_byDescTag != 0x52U)return false;		// タグが不正
	else if(m_byDescLen != 1U)return false;		// ストリームID記述子のサイズは常に1

	// 記述子を解析
	m_byComponentTag = pPayload[0];				// +0	Component Tag

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// 記述子ブロック抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CDescBlock::CDescBlock()
{

}

CDescBlock::CDescBlock(const CDescBlock &Operand)
{
	*this = Operand;
}

CDescBlock::~CDescBlock()
{
	Reset();
}

CDescBlock & CDescBlock::operator = (const CDescBlock &Operand)
{
	// インスタンスのコピー
	Reset();
	m_DescArray.resize(Operand.m_DescArray.size());
	
	for(WORD wIndex = 0UL ; wIndex < m_DescArray.size() ; wIndex++){
		m_DescArray[wIndex] = CreateDescInstance(Operand.m_DescArray[wIndex]->GetTag());
		m_DescArray[wIndex]->CopyDesc(Operand.m_DescArray[wIndex]);
		}
	
	return *this;
}

const WORD CDescBlock::ParseBlock(const BYTE *pHexData, const WORD wDataLength)
{
	if(!pHexData || (wDataLength < 2U))return 0U;

	// 状態をクリア
	Reset();

	// 指定されたブロックに含まれる記述子を解析する
	WORD wPos = 0UL;
	CBaseDesc *pNewDesc = NULL;
	
	while(wPos < wDataLength){
		// ブロックを解析する
		if(!(pNewDesc = ParseDesc(&pHexData[wPos], wDataLength - wPos)))break;

		// リストに追加する
		m_DescArray.push_back(pNewDesc);

		// 位置更新
		wPos += (pNewDesc->GetLength() + 2U);
		}

	return m_DescArray.size();
}

const CBaseDesc * CDescBlock::ParseBlock(const BYTE *pHexData, const WORD wDataLength, const BYTE byTag)
{
	// 指定されたブロックに含まれる記述子を解析して指定されたタグの記述子を返す
	return (ParseBlock(pHexData, wDataLength))? GetDescByTag(byTag) : NULL;
}

void CDescBlock::Reset(void)
{
	// 全てのインスタンスを開放する
	for(WORD wIndex = 0U ; wIndex < m_DescArray.size() ; wIndex++){
		delete m_DescArray[wIndex];
		}
		
	m_DescArray.clear();
}

const WORD CDescBlock::GetDescNum(void) const
{
	// 記述子の数を返す
	return m_DescArray.size();
}

const CBaseDesc * CDescBlock::GetDescByIndex(const WORD wIndex) const
{
	// インデックスで指定した記述子を返す
	return (wIndex < m_DescArray.size())? m_DescArray[wIndex] : NULL;
}

const CBaseDesc * CDescBlock::GetDescByTag(const BYTE byTag) const
{
	// 指定したタグに一致する記述子を返す
	for(WORD wIndex = 0U ; wIndex < m_DescArray.size() ; wIndex++){
		if(m_DescArray[wIndex]->GetTag() == byTag)return m_DescArray[wIndex];
		}

	return NULL;
}

CBaseDesc * CDescBlock::ParseDesc(const BYTE *pHexData, const WORD wDataLength)
{
	if(!pHexData || (wDataLength < 2U))return NULL;

	// タグに対応したインスタンスを生成する
	CBaseDesc *pNewDesc = CreateDescInstance(pHexData[0]);

	// メモリ不足
	if(!pNewDesc)return NULL;

	// 記述子を解析する
	if(!pNewDesc->ParseDesc(pHexData, wDataLength)){
		// エラーあり
		delete pNewDesc;
		return NULL;
		}

	return pNewDesc;
}

CBaseDesc * CDescBlock::CreateDescInstance(const BYTE byTag)
{
	// タグに対応したインスタンスを生成する
	switch(byTag){
		case CCaMethodDesc::DESC_TAG	: return new CCaMethodDesc;
		case CServiceDesc::DESC_TAG		: return new CServiceDesc;
		case CStreamIdDesc::DESC_TAG	: return new CStreamIdDesc;
		default							: return new CBaseDesc;
		}
}
