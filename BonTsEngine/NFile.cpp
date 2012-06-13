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
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CNFile::~CNFile()
{
	Close();
}

const BOOL CNFile::Open(LPCTSTR lpszName, BYTE bFlags)
{
	if(m_hFile != INVALID_HANDLE_VALUE){
		SetLastError(0x000000AA);	// 「要求したリソースは使用中です。」
		return FALSE;
		}

	// ファイルアクセス属性構築
	DWORD dwAccess = 0x00000000;
	if(bFlags & CNF_READ )dwAccess |= GENERIC_READ;
	if(bFlags & CNF_WRITE)dwAccess |= GENERIC_WRITE;
	else bFlags &= ~((BYTE)CNF_WRITE);
	if(!dwAccess){
		SetLastError(0x00000057);	// 「パラメータが正しくありません。」
		return FALSE;
		}

	// ファイル共有属性構築
	DWORD dwShare = 0x00000000;
	if(bFlags & CNF_SHAREREAD )dwShare |= FILE_SHARE_READ;
	if(bFlags & CNF_SHAREWRITE)dwShare |= FILE_SHARE_WRITE;

	// ファイル作成属性構築
	DWORD dwCreate = 0x00000000;
	if(bFlags & CNF_NEW)dwCreate |= CREATE_ALWAYS;
	else dwCreate |= OPEN_EXISTING;

	// ファイルオープン
	m_hFile = CreateFile(lpszName, dwAccess, dwShare, NULL, dwCreate, 0, NULL);
	
	return (m_hFile != INVALID_HANDLE_VALUE)? TRUE : FALSE;
}

void CNFile::Close(void)
{
	// ファイルクローズ
	if(m_hFile != INVALID_HANDLE_VALUE){
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
}

const BOOL CNFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// ファイルリード
	DWORD dwRead = 0;

	if(ReadFile(m_hFile, pBuff, dwLen, &dwRead, NULL)){
		if(dwRead == dwLen){
			return TRUE;
			}
		}
	
	return FALSE;
}

const BOOL CNFile::Read(BYTE *pBuff, const DWORD dwLen, const LONGLONG llPos)
{
	// ファイルリード
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return FALSE;
}

const BOOL CNFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// ファイルライト
	DWORD dwWritten = 0;

	if(WriteFile(m_hFile, pBuff, dwLen, &dwWritten, NULL)){
		if(dwWritten == dwLen){
			return TRUE;
			}		
		}

	return FALSE;
}

const BOOL CNFile::Write(const BYTE *pBuff, const DWORD dwLen, const LONGLONG llPos)
{
	// ファイルシーク
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return FALSE;
}

const BOOL CNFile::Seek(const LONGLONG llPos)
{
	// ファイルシーク
	LONG lPosHigh = (LONG)(llPos >> 32);

	if(SetFilePointer(m_hFile, (LONG)(llPos & 0xFFFFFFFF), &lPosHigh, FILE_BEGIN) == 0xFFFFFFFF){
		if(GetLastError() != NO_ERROR){
			return FALSE;
			}		
		}

	return TRUE;
}

const LONGLONG CNFile::GetSize(void)
{
	// ファイルサイズ取得
	DWORD dwSizeHi = 0;
	DWORD dwSizeLo = 0;

	dwSizeLo = GetFileSize(m_hFile, &dwSizeHi);

	if((dwSizeLo == 0xFFFFFFFF) && (GetLastError() != NO_ERROR)){
		return (LONGLONG)-1;
		}
	
	return ((LONGLONG)dwSizeHi << 32) | (LONGLONG)dwSizeLo;
}

LPCTSTR CNFile::GetErrorMessage(void)
{
	static TCHAR szMessage[1024] = {'\0'};

	TRACE(TEXT("GetLastError() = %lu\n"), GetLastError());

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 0x00000000, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szMessage, sizeof(szMessage), NULL);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szMessage, sizeof(szMessage), NULL);

	return szMessage;
}
