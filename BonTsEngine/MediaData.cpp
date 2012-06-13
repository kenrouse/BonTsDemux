// MediaData.cpp: CMediaData �N���X�̃C���v�������e�[�V����
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
// �\�z/����
//////////////////////////////////////////////////////////////////////

// stl::vector<BYTE>�ŏ�������������������������Ȃ�

#define	MINBUFSIZE	256UL		// �ŏ��o�b�t�@�T�C�Y
#define MINADDSIZE	256UL		// �ŏ��ǉ��m�ۃT�C�Y


CMediaData::CMediaData()
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// ��̃o�b�t�@�𐶐�����

}

CMediaData::CMediaData(const CMediaData &Operand)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �R�s�[�R���X�g���N�^
	*this = Operand;
}

CMediaData::CMediaData(const DWORD dwBuffSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �o�b�t�@�T�C�Y���w�肵�ăo�b�t�@�𐶐�����
	GetBuffer(dwBuffSize);
}

CMediaData::CMediaData(const BYTE *pData, const DWORD dwDataSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �f�[�^�����l���w�肵�ăo�b�t�@�𐶐�����
	SetData(pData, dwDataSize);
}

CMediaData::CMediaData(const BYTE byFiller, const DWORD dwDataSize)
	: m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �t�B���f�[�^���w�肵�ăo�b�t�@�𐶐�����
	SetSize(dwDataSize, byFiller);
}

CMediaData::~CMediaData()
{
	if(m_pData)delete [] m_pData;		
}

CMediaData & CMediaData::operator = (const CMediaData &Operand)
{
	// �o�b�t�@�T�C�Y�̏��܂ł̓R�s�[���Ȃ�
	SetData(Operand.m_pData, Operand.m_dwDataSize);

	return *this;
}

BYTE * CMediaData::GetData() const
{
	// �o�b�t�@�|�C���^���擾����
	return (m_dwDataSize)? m_pData : NULL;
}

const DWORD CMediaData::GetSize() const
{
	// �f�[�^�T�C�Y���擾����
	return m_dwDataSize;
}

void CMediaData::SetAt(const DWORD dwPos, const BYTE byData)
{
	// 1�o�C�g�Z�b�g����
	if(dwPos < m_dwDataSize)m_pData[dwPos] = byData;
}

const BYTE CMediaData::GetAt(const DWORD dwPos) const
{
	// 1�o�C�g�擾����
	return (dwPos < m_dwDataSize)? m_pData[dwPos] : 0x00U;
}

const DWORD CMediaData::SetData(const BYTE *pData, const DWORD dwDataSize)
{
	if(dwDataSize){
		// �o�b�t�@�m��
		GetBuffer(dwDataSize);

		// �f�[�^�Z�b�g
		::CopyMemory(m_pData, pData, dwDataSize);
		}

	// �T�C�Y�Z�b�g
	m_dwDataSize = dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const BYTE *pData, const DWORD dwDataSize)
{
	if(!dwDataSize)return m_dwDataSize;

	// �o�b�t�@�m��
	GetBuffer(m_dwDataSize + dwDataSize);
	
	// �f�[�^�ǉ�
	::CopyMemory(&m_pData[m_dwDataSize], pData, dwDataSize);

	// �T�C�Y�Z�b�g
	m_dwDataSize += dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const CMediaData *pData)
{
	return AddData(pData->m_pData, pData->m_dwDataSize);
}

const DWORD CMediaData::AddByte(const BYTE byData)
{
	// �o�b�t�@�m��
	GetBuffer(m_dwDataSize + 1UL);
	
	// �f�[�^�ǉ�
	m_pData[m_dwDataSize] = byData;

	// �T�C�Y�X�V
	m_dwDataSize++;

	return m_dwDataSize;
}

const DWORD CMediaData::TrimHead(const DWORD dwTrimSize)
{
	// �f�[�^�擪��؂�l�߂�
	if(!m_dwDataSize || !dwTrimSize){
		// �������Ȃ�
		}
	else if(dwTrimSize >= m_dwDataSize){
		// �S�̂�؂�l�߂�
		m_dwDataSize = 0UL;		
		}
	else{
		// �f�[�^���ړ�����
		::MoveMemory(m_pData, m_pData + dwTrimSize, m_dwDataSize - dwTrimSize);
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::TrimTail(const DWORD dwTrimSize)
{
	// �f�[�^������؂�l�߂�
	if(!m_dwDataSize || !dwTrimSize){
		// �������Ȃ�
		}
	else if(dwTrimSize >= m_dwDataSize){
		// �S�̂�؂�l�߂�
		m_dwDataSize = 0UL;		
		}
	else{
		// �f�[�^������؂�l�߂�
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::GetBuffer(const DWORD dwGetSize)
{
	if(dwGetSize <= m_dwBuffSize)return m_dwBuffSize;

	// ���Ȃ��Ƃ��w��T�C�Y���i�[�ł���o�b�t�@���m�ۂ���
	if(!m_pData){
		// �o�b�t�@�m�ۂ܂�
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		m_pData = new BYTE [m_dwBuffSize];
		}
	else if(dwGetSize > m_dwBuffSize){
		// �v���T�C�Y�̓o�b�t�@�T�C�Y�𒴂���
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		if(m_dwBuffSize < (m_dwDataSize * 2UL))m_dwBuffSize = m_dwDataSize * 2UL;

		BYTE *pNewBuffer = new BYTE [m_dwBuffSize];

		// �f�[�^�R�s�[
		if(m_dwDataSize){
			::CopyMemory(pNewBuffer, m_pData, m_dwDataSize);
			}
		
		// ���o�b�t�@�J��
		delete [] m_pData;

		// �o�b�t�@�����ւ�
		m_pData = pNewBuffer;
		}

	return m_dwBuffSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize)
{
	if(dwSetSize){
		// �o�b�t�@�m��
		GetBuffer(dwSetSize);
		}

	// �T�C�Y�Z�b�g
	m_dwDataSize = dwSetSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize, const BYTE byFiller)
{
	// �T�C�Y�Z�b�g
	SetSize(dwSetSize);
	
	// �f�[�^�Z�b�g
	if(dwSetSize){
		::FillMemory(m_pData, dwSetSize, byFiller);
		}
		
	return m_dwDataSize;
}

void CMediaData::ClearSize(void)
{
	// �f�[�^�T�C�Y���N���A����
	m_dwDataSize = 0UL;
}

void CMediaData::Delete()
{
	// �C���X�^���X���폜����
	delete this;
}
