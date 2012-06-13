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
		::SetLastError(0x000000AAUL);	// �u�v���������\�[�X�͎g�p���ł��B�v
		return false;
		}

	// �t�@�C���A�N�Z�X�����\�z
	DWORD dwAccess = 0x00000000UL;

	if(bFlags & CNF_READ )dwAccess |= GENERIC_READ;
	if(bFlags & CNF_WRITE)dwAccess |= GENERIC_WRITE;

	if(!dwAccess){
		::SetLastError(0x00000057UL);	// �u�p�����[�^������������܂���B�v
		return false;
		}

	// �t�@�C�����L�����\�z
	DWORD dwShare = 0x00000000UL;

	if(bFlags & CNF_SHAREREAD  )dwShare |= FILE_SHARE_READ;
	if(bFlags & CNF_SHAREWRITE )dwShare |= FILE_SHARE_WRITE;
	if(bFlags & CNF_SHAREDELETE)dwShare |= FILE_SHARE_DELETE;

	// �t�@�C���쐬�����\�z
	DWORD dwCreate = 0x00000000UL;

	if(bFlags & CNF_NEW)dwCreate |= CREATE_ALWAYS;
	else dwCreate |= OPEN_EXISTING;

	// �t�@�C���I�[�v��
	m_hFile = CreateFile(lpszName, dwAccess, dwShare, NULL, dwCreate, 0UL, NULL);
	
	return (m_hFile != INVALID_HANDLE_VALUE)? true : false;
}

void CNFile::Close(void)
{
	// �t�@�C���N���[�Y
	if(m_hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
}

const bool CNFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// �t�@�C�����[�h
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
	// �t�@�C�����[�h
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return FALSE;
}

const bool CNFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// �t�@�C�����C�g
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
	// �t�@�C���V�[�N
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return false;
}

const ULONGLONG CNFile::GetSize(void) const
{
	// �t�@�C���T�C�Y�擾
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
	// �|�W�V�����擾
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
	// �t�@�C���V�[�N
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
