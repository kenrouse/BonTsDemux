// NFile.cpp: CNFile �N���X�̃C���v�������e�[�V����
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
// �\�z/����
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
		SetLastError(0x000000AA);	// �u�v���������\�[�X�͎g�p���ł��B�v
		return FALSE;
		}

	// �t�@�C���A�N�Z�X�����\�z
	DWORD dwAccess = 0x00000000;
	if(bFlags & CNF_READ )dwAccess |= GENERIC_READ;
	if(bFlags & CNF_WRITE)dwAccess |= GENERIC_WRITE;
	else bFlags &= ~((BYTE)CNF_WRITE);
	if(!dwAccess){
		SetLastError(0x00000057);	// �u�p�����[�^������������܂���B�v
		return FALSE;
		}

	// �t�@�C�����L�����\�z
	DWORD dwShare = 0x00000000;
	if(bFlags & CNF_SHAREREAD )dwShare |= FILE_SHARE_READ;
	if(bFlags & CNF_SHAREWRITE)dwShare |= FILE_SHARE_WRITE;

	// �t�@�C���쐬�����\�z
	DWORD dwCreate = 0x00000000;
	if(bFlags & CNF_NEW)dwCreate |= CREATE_ALWAYS;
	else dwCreate |= OPEN_EXISTING;

	// �t�@�C���I�[�v��
	m_hFile = CreateFile(lpszName, dwAccess, dwShare, NULL, dwCreate, 0, NULL);
	
	return (m_hFile != INVALID_HANDLE_VALUE)? TRUE : FALSE;
}

void CNFile::Close(void)
{
	// �t�@�C���N���[�Y
	if(m_hFile != INVALID_HANDLE_VALUE){
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
}

const BOOL CNFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// �t�@�C�����[�h
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
	// �t�@�C�����[�h
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return FALSE;
}

const BOOL CNFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// �t�@�C�����C�g
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
	// �t�@�C���V�[�N
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return FALSE;
}

const BOOL CNFile::Seek(const LONGLONG llPos)
{
	// �t�@�C���V�[�N
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
	// �t�@�C���T�C�Y�擾
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
