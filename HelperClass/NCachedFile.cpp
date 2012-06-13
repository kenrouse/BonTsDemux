// NCachedFile.cpp: CNCachedFile クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NCachedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNCachedFile::CNCachedFile()
	: CNFile()
	, m_bIsWritable(false)
	, m_pBuff(NULL)
	, m_dwBuffSize(0UL)
	, m_llDataPos(0ULL)
	, m_llDataSize(0ULL)
	, m_llCurPos(0ULL)
{

}

CNCachedFile::~CNCachedFile()
{
	Close();
}

const bool CNCachedFile::Open(LPCTSTR lpszName, const BYTE bFlags, const DWORD dwBuffSize)
{
	if((bFlags & CNFile::CNF_WRITE) && !(bFlags & CNFile::CNF_READ)){
		// ライトキャッシュ有効
		m_bIsWritable = true;
		}
	else if(!(bFlags & CNFile::CNF_WRITE) && (bFlags & CNFile::CNF_READ) && !(bFlags & CNFile::CNF_NEW)){
		// リードキャッシュ有効
		m_bIsWritable = false;
		}
	else{
		// フラグの組み合わせが非対応
		::SetLastError(0x00000057UL);	// 「パラメータが正しくありません。」
		return false;
		}

	// ファイルオープン
	if(!CNFile::Open(lpszName, bFlags))return false;

	// バッファ確保
	if(!m_bIsWritable){
		// 読み込みバッファ
		m_dwBuffSize = (dwBuffSize <= GetSize())? dwBuffSize : (DWORD)GetSize();
		}

	if(m_pBuff)delete [] m_pBuff;
	m_pBuff = new BYTE [m_dwBuffSize];

	if(!m_pBuff){
		Close();
		::SetLastError(0x0000000EUL);	// 「この操作を完了するのに十分な記憶領域がありません。」
		return false;
		}

	// バッファ初期化
	m_llDataPos = 0ULL;
	m_llDataSize = 0ULL;
	m_llCurPos = 0ULL;

	::SetLastError(0x00000000UL);	// 「操作は正常に終了しました。」

	return true;
}

void CNCachedFile::Close(void)
{
	// 未書き込みデータフラッシュ
	Flush();

	CNFile::Close();

	if(m_pBuff){
		delete [] m_pBuff;
		m_pBuff = NULL;
		}
}

const bool CNCachedFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// エラー処理
	if(m_bIsWritable){
		::SetLastError(0x00000001UL);	// 「不正な関数です。」
		return false;
		}

	if(!dwLen){
		::SetLastError(0x00000057UL);	// 「パラメータが正しくありません。」
		return false;
		}

	return CNFile::Read(pBuff, dwLen);
}

const bool CNCachedFile::Read(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	if(m_bIsWritable){
		::SetLastError(0x00000001UL);	// 「不正な関数です。」
		return false;
		}

	// ファイルシーク
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return false;
}

const bool CNCachedFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// ファイル書き込み
	if(!m_bIsWritable){
		::SetLastError(0x00000001UL);	// 「不正な関数です。」
		return false;
		}

	if(!dwLen){
		::SetLastError(0x00000057UL);	// 「パラメータが正しくありません。」
		return false;
		}

	// バッファリング判定
	if((m_dwBuffSize - (DWORD)m_llDataSize) <= dwLen){
		// バッファ不足
		if(!Flush())return false;
		}

	// バッファリング
	if((m_dwBuffSize - (DWORD)m_llDataSize) >= dwLen){
		::CopyMemory(m_pBuff + m_llDataSize, pBuff, dwLen);
		m_llDataSize += dwLen;
		}
	else{
		if(!CNFile::Write(pBuff, dwLen, m_llCurPos))return false;
		}

	// ファイルポジション更新
	m_llCurPos += (ULONGLONG)dwLen;

	::SetLastError(0x00000000UL);	// 「操作は正常に終了しました。」

	return true;
}

const bool CNCachedFile::Write(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// ファイル書き込み
	if(!m_bIsWritable){
		::SetLastError(0x00000001UL);	// 「不正な関数です。」
		return false;
		}

	// ファイルシーク
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return false;
}

const ULONGLONG CNCachedFile::GetPos(void) const
{
	// 論理的なファイルポジションを返す
	return (m_bIsWritable)? m_llCurPos : CNFile::GetPos();
}

const bool CNCachedFile::Seek(const ULONGLONG llPos)
{
	// シーク前にフラッシュ
	if(!Flush())return false;

	// シーク
	if(!CNFile::Seek(llPos))return false;

	// ファイルポジション更新
	m_llCurPos = llPos;
	m_llDataPos = llPos;

	return true;
}

const bool CNCachedFile::Flush(void)
{
	if(!m_bIsWritable || !m_llDataSize)return true;

	// バッファ先頭位置に書き込み
	if(!CNFile::Write(m_pBuff, (DWORD)m_llDataSize, m_llDataPos))return false;

	// ファイルポジションを復帰
	if(!CNFile::Seek(m_llCurPos))return false;

	// バッファサイズクリア
	m_llDataSize = 0ULL;
	m_llDataPos = m_llCurPos;

	return true;
}
