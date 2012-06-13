// MediaData.cpp: CMediaData クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

// stl::vector<BYTE>で書き直した方がいいかもしれない

#define	MINBUFSIZE	256UL		// 最小バッファサイズ
#define MINADDSIZE	256UL		// 最小追加確保サイズ


CMediaData::CMediaData()
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// 空のバッファを生成する

}

CMediaData::CMediaData(const CMediaData &Operand)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// コピーコンストラクタ
	*this = Operand;
}

CMediaData::CMediaData(const DWORD dwBuffSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// バッファサイズを指定してバッファを生成する
	GetBuffer(dwBuffSize);
}

CMediaData::CMediaData(const BYTE *pData, const DWORD dwDataSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// データ初期値を指定してバッファを生成する
	SetData(pData, dwDataSize);
}

CMediaData::CMediaData(const BYTE byFiller, const DWORD dwDataSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// フィルデータを指定してバッファを生成する
	SetSize(dwDataSize, byFiller);
}

CMediaData::~CMediaData()
{
	if(m_pData)delete [] m_pData;		
}

CMediaData & CMediaData::operator = (const CMediaData &Operand)
{
	// バッファサイズの情報まではコピーしない
	SetData(Operand.m_pData, Operand.m_dwDataSize);

	return *this;
}

BYTE * CMediaData::GetData() const
{
	// バッファポインタを取得する
	return (m_dwDataSize)? m_pData : NULL;
}

const DWORD CMediaData::GetSize() const
{
	// データサイズを取得する
	return m_dwDataSize;
}

void CMediaData::SetAt(const DWORD dwPos, const BYTE byData)
{
	// 1バイトセットする
	if(dwPos < m_dwDataSize)m_pData[dwPos] = byData;
}

const BYTE CMediaData::GetAt(const DWORD dwPos) const
{
	// 1バイト取得する
	return (dwPos < m_dwDataSize)? m_pData[dwPos] : 0x00U;
}

const DWORD CMediaData::SetData(const BYTE *pData, const DWORD dwDataSize)
{
	if(dwDataSize){
		// バッファ確保
		GetBuffer(dwDataSize);

		// データセット
		::CopyMemory(m_pData, pData, dwDataSize);
		}

	// サイズセット
	m_dwDataSize = dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const BYTE *pData, const DWORD dwDataSize)
{
	if(!dwDataSize)return m_dwDataSize;

	// バッファ確保
	GetBuffer(m_dwDataSize + dwDataSize);
	
	// データ追加
	::CopyMemory(&m_pData[m_dwDataSize], pData, dwDataSize);

	// サイズセット
	m_dwDataSize += dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const CMediaData *pData)
{
	return AddData(pData->m_pData, pData->m_dwDataSize);
}

const DWORD CMediaData::AddByte(const BYTE byData)
{
	// バッファ確保
	GetBuffer(m_dwDataSize + 1UL);
	
	// データ追加
	m_pData[m_dwDataSize] = byData;

	// サイズ更新
	m_dwDataSize++;

	return m_dwDataSize;
}

const DWORD CMediaData::TrimHead(const DWORD dwTrimSize)
{
	// データ先頭を切り詰める
	if(!m_dwDataSize || !dwTrimSize){
		// 何もしない
		}
	else if(dwTrimSize >= m_dwDataSize){
		// 全体を切り詰める
		m_dwDataSize = 0UL;		
		}
	else{
		// データを移動する
		::MoveMemory(m_pData, m_pData + dwTrimSize, m_dwDataSize - dwTrimSize);
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::TrimTail(const DWORD dwTrimSize)
{
	// データ末尾を切り詰める
	if(!m_dwDataSize || !dwTrimSize){
		// 何もしない
		}
	else if(dwTrimSize >= m_dwDataSize){
		// 全体を切り詰める
		m_dwDataSize = 0UL;		
		}
	else{
		// データ末尾を切り詰める
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::GetBuffer(const DWORD dwGetSize)
{
	if(dwGetSize <= m_dwBuffSize)return m_dwBuffSize;

	// 少なくとも指定サイズを格納できるバッファを確保する
	if(!m_pData){
		// バッファ確保まだ
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		m_pData = new BYTE [m_dwBuffSize];
		}
	else if(dwGetSize > m_dwBuffSize){
		// 要求サイズはバッファサイズを超える
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		if(m_dwBuffSize < (m_dwDataSize * 2UL))m_dwBuffSize = m_dwDataSize * 2UL;

		BYTE *pNewBuffer = new BYTE [m_dwBuffSize];

		// データコピー
		if(m_dwDataSize){
			::CopyMemory(pNewBuffer, m_pData, m_dwDataSize);
			}
		
		// 旧バッファ開放
		delete [] m_pData;

		// バッファ差し替え
		m_pData = pNewBuffer;
		}

	return m_dwBuffSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize)
{
	if(dwSetSize){
		// バッファ確保
		GetBuffer(dwSetSize);
		}

	// サイズセット
	m_dwDataSize = dwSetSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize, const BYTE byFiller)
{
	// サイズセット
	SetSize(dwSetSize);
	
	// データセット
	if(dwSetSize){
		::FillMemory(m_pData, dwSetSize, byFiller);
		}
		
	return m_dwDataSize;
}

void CMediaData::ClearSize(void)
{
	// データサイズをクリアする
	m_dwDataSize = 0UL;
}

void CMediaData::Delete()
{
	// インスタンスを削除する
	delete this;
}
