// NFile.cpp: CNFile クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNFile::CNFile()
	: m_hFile(INVALID_HANDLE_VALUE)
{

}

CNFile::~CNFile()
{
	Close();
}

const bool CNFile::Open(LPCTSTR lpszName, const BYTE bFlags)
{
	if(m_hFile != INVALID_HANDLE_VALUE){
		::SetLastError(0x000000AAUL);	// 「要求したリソースは使用中です。」
		return false;
		}

	// ファイルアクセス属性構築
	DWORD dwAccess = 0x00000000UL;

	if(bFlags & CNF_READ )dwAccess |= GENERIC_READ;
	if(bFlags & CNF_WRITE)dwAccess |= GENERIC_WRITE;

	if(!dwAccess){
		::SetLastError(0x00000057UL);	// 「パラメータが正しくありません。」
		return false;
		}

	// ファイル共有属性構築
	DWORD dwShare = 0x00000000UL;

	if(bFlags & CNF_SHAREREAD  )dwShare |= FILE_SHARE_READ;
	if(bFlags & CNF_SHAREWRITE )dwShare |= FILE_SHARE_WRITE;
	if(bFlags & CNF_SHAREDELETE)dwShare |= FILE_SHARE_DELETE;

	// ファイル作成属性構築
	DWORD dwCreate = 0x00000000UL;

	if(bFlags & CNF_NEW)dwCreate |= CREATE_ALWAYS;
	else dwCreate |= OPEN_EXISTING;

	// ファイルオープン
	m_hFile = CreateFile(lpszName, dwAccess, dwShare, NULL, dwCreate, 0UL, NULL);
	
	return (m_hFile != INVALID_HANDLE_VALUE)? true : false;
}

void CNFile::Close(void)
{
	// ファイルクローズ
	if(m_hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
}

const bool CNFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// ファイルリード
	DWORD dwRead = 0;

	if(ReadFile(m_hFile, pBuff, dwLen, &dwRead, NULL)){
		if(dwRead == dwLen){
			return true;
			}
		}
	
	return false;
}

const bool CNFile::Read(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// ファイルリード
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return FALSE;
}

const bool CNFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// ファイルライト
	DWORD dwWritten = 0UL;

	if(WriteFile(m_hFile, pBuff, dwLen, &dwWritten, NULL)){
		if(dwWritten == dwLen){
			return true;
			}		
		}

	return false;
}

const bool CNFile::Write(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// ファイルシーク
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return false;
}

const ULONGLONG CNFile::GetSize(void) const
{
	// ファイルサイズ取得
	DWORD dwSizeHi = 0UL;
	DWORD dwSizeLo = 0UL;

	dwSizeLo = GetFileSize(m_hFile, &dwSizeHi);

	if((dwSizeLo == 0xFFFFFFFFUL) && (GetLastError() != NO_ERROR)){
		return 0ULL;
		}
	
	return ((ULONGLONG)dwSizeHi << 32) | (ULONGLONG)dwSizeLo;
}

const ULONGLONG CNFile::GetPos(void) const
{
	// ポジション取得
	LONG lPosHigh = 0LL;
	DWORD dwPosLow = ::SetFilePointer(m_hFile, 0LL, &lPosHigh, FILE_CURRENT);

	if(dwPosLow == 0xFFFFFFFFUL){
		if(::GetLastError() != NO_ERROR){
			return 0ULL;
			}		
		}

	return ((ULONGLONG)lPosHigh << 32) | (ULONGLONG)dwPosLow;
}

const bool CNFile::Seek(const ULONGLONG llPos)
{
	// ファイルシーク
	LONG lPosHigh = (LONG)(llPos >> 32);

	if(::SetFilePointer(m_hFile, (LONG)(llPos & 0xFFFFFFFFULL), &lPosHigh, FILE_BEGIN) == 0xFFFFFFFFUL){
		if(::GetLastError() != NO_ERROR){
			return false;
			}		
		}

	return true;
}

LPCTSTR CNFile::GetErrorMessage(void) const
{
	static TCHAR szMessage[1024] = {TEXT('\0')};

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 0x00000000, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szMessage, sizeof(szMessage), NULL);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szMessage, sizeof(szMessage), NULL);

	return szMessage;
}
