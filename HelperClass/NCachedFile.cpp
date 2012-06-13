// NCachedFile.cpp: CNCachedFile �N���X�̃C���v�������e�[�V����
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
// �\�z/����
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
		// ���C�g�L���b�V���L��
		m_bIsWritable = true;
		}
	else if(!(bFlags & CNFile::CNF_WRITE) && (bFlags & CNFile::CNF_READ) && !(bFlags & CNFile::CNF_NEW)){
		// ���[�h�L���b�V���L��
		m_bIsWritable = false;
		}
	else{
		// �t���O�̑g�ݍ��킹����Ή�
		::SetLastError(0x00000057UL);	// �u�p�����[�^������������܂���B�v
		return false;
		}

	// �t�@�C���I�[�v��
	if(!CNFile::Open(lpszName, bFlags))return false;

	// �o�b�t�@�m��
	if(!m_bIsWritable){
		// �ǂݍ��݃o�b�t�@
		m_dwBuffSize = (dwBuffSize <= GetSize())? dwBuffSize : (DWORD)GetSize();
		}

	if(m_pBuff)delete [] m_pBuff;
	m_pBuff = new BYTE [m_dwBuffSize];

	if(!m_pBuff){
		Close();
		::SetLastError(0x0000000EUL);	// �u���̑������������̂ɏ\���ȋL���̈悪����܂���B�v
		return false;
		}

	// �o�b�t�@������
	m_llDataPos = 0ULL;
	m_llDataSize = 0ULL;
	m_llCurPos = 0ULL;

	::SetLastError(0x00000000UL);	// �u����͐���ɏI�����܂����B�v

	return true;
}

void CNCachedFile::Close(void)
{
	// ���������݃f�[�^�t���b�V��
	Flush();

	CNFile::Close();

	if(m_pBuff){
		delete [] m_pBuff;
		m_pBuff = NULL;
		}
}

const bool CNCachedFile::Read(BYTE *pBuff, const DWORD dwLen)
{
	// �G���[����
	if(m_bIsWritable){
		::SetLastError(0x00000001UL);	// �u�s���Ȋ֐��ł��B�v
		return false;
		}

	if(!dwLen){
		::SetLastError(0x00000057UL);	// �u�p�����[�^������������܂���B�v
		return false;
		}

	return CNFile::Read(pBuff, dwLen);
}

const bool CNCachedFile::Read(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	if(m_bIsWritable){
		::SetLastError(0x00000001UL);	// �u�s���Ȋ֐��ł��B�v
		return false;
		}

	// �t�@�C���V�[�N
	if(Seek(llPos)){
		return Read(pBuff, dwLen);
		}

	return false;
}

const bool CNCachedFile::Write(const BYTE *pBuff, const DWORD dwLen)
{
	// �t�@�C����������
	if(!m_bIsWritable){
		::SetLastError(0x00000001UL);	// �u�s���Ȋ֐��ł��B�v
		return false;
		}

	if(!dwLen){
		::SetLastError(0x00000057UL);	// �u�p�����[�^������������܂���B�v
		return false;
		}

	// �o�b�t�@�����O����
	if((m_dwBuffSize - (DWORD)m_llDataSize) <= dwLen){
		// �o�b�t�@�s��
		if(!Flush())return false;
		}

	// �o�b�t�@�����O
	if((m_dwBuffSize - (DWORD)m_llDataSize) >= dwLen){
		::CopyMemory(m_pBuff + m_llDataSize, pBuff, dwLen);
		m_llDataSize += dwLen;
		}
	else{
		if(!CNFile::Write(pBuff, dwLen, m_llCurPos))return false;
		}

	// �t�@�C���|�W�V�����X�V
	m_llCurPos += (ULONGLONG)dwLen;

	::SetLastError(0x00000000UL);	// �u����͐���ɏI�����܂����B�v

	return true;
}

const bool CNCachedFile::Write(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// �t�@�C����������
	if(!m_bIsWritable){
		::SetLastError(0x00000001UL);	// �u�s���Ȋ֐��ł��B�v
		return false;
		}

	// �t�@�C���V�[�N
	if(Seek(llPos)){
		return Write(pBuff, dwLen);
		}

	return false;
}

const ULONGLONG CNCachedFile::GetPos(void) const
{
	// �_���I�ȃt�@�C���|�W�V������Ԃ�
	return (m_bIsWritable)? m_llCurPos : CNFile::GetPos();
}

const bool CNCachedFile::Seek(const ULONGLONG llPos)
{
	// �V�[�N�O�Ƀt���b�V��
	if(!Flush())return false;

	// �V�[�N
	if(!CNFile::Seek(llPos))return false;

	// �t�@�C���|�W�V�����X�V
	m_llCurPos = llPos;
	m_llDataPos = llPos;

	return true;
}

const bool CNCachedFile::Flush(void)
{
	if(!m_bIsWritable || !m_llDataSize)return true;

	// �o�b�t�@�擪�ʒu�ɏ�������
	if(!CNFile::Write(m_pBuff, (DWORD)m_llDataSize, m_llDataPos))return false;

	// �t�@�C���|�W�V�����𕜋A
	if(!CNFile::Seek(m_llCurPos))return false;

	// �o�b�t�@�T�C�Y�N���A
	m_llDataSize = 0ULL;
	m_llDataPos = m_llCurPos;

	return true;
}
