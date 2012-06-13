// FileWriter.cpp: CFileWriter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CFileReader::CFileReader(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_ReadBuffer((BYTE)0x00U, DEF_READSIZE)
	, m_ReadSizeLimit(QWORD_MAX)
	, m_hReadAnsyncThread(NULL)
	, m_dwReadAnsyncThreadID(0UL)
	, m_bKillSignal(true)
{

}

CFileReader::~CFileReader()
{
	CloseFile();

	if(m_hReadAnsyncThread)::CloseHandle(m_hReadAnsyncThread);
}

void CFileReader::Reset(void)
{
	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();
}

const DWORD CFileReader::GetInputNum(void) const
{
	return 0UL;
}

const DWORD CFileReader::GetOutputNum(void) const
{
	return 1UL;
}

const bool CFileReader::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	// �\�[�X�ŃR�[�_�̂��ߏ�ɃG���[��Ԃ�
	return false;
}

const bool CFileReader::OpenFile(LPCTSTR lpszFileName,QWORD qwReadSizeLimit)
{
	// ��U����
	CloseFile();

	// �t�@�C�����J��
	m_ReadSizeLimit = qwReadSizeLimit;
	return (m_InFile.Open(lpszFileName, CNCachedFile::CNF_READ | CNCachedFile::CNF_SHAREREAD | CNCachedFile::CNF_SHAREWRITE))? true : false;
}

void CFileReader::CloseFile(void)
{
	// �t�@�C�������
	StopReadAnsync();
	m_InFile.Close();
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize)
{
	// �ǂݍ��݃T�C�Y�v�Z
	ULONGLONG llRemainSize = m_InFile.GetSize() - m_InFile.GetPos();
	const DWORD dwReqSize = (llRemainSize > (ULONGLONG)dwReadSize)? dwReadSize : (DWORD)llRemainSize;
	if(!dwReqSize)return 0UL;

	// �o�b�t�@�m��
	//m_ReadBuffer.SetSize(dwReadSize);
	m_ReadBuffer.SetSize(dwReqSize); //2010.05.07 �ǂݍ��񂾃T�C�Y�ɃZ�b�g����B
	
	// �t�@�C���ǂݍ���
	if(!m_InFile.Read(m_ReadBuffer.GetData(), dwReqSize))return 0UL;
	
	// �f�[�^�o��
	OutputMedia(&m_ReadBuffer);
	
	return dwReqSize;
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize, const ULONGLONG llReadPos)
{
	// �t�@�C���V�[�N
	if(!m_InFile.Seek(llReadPos))return 0UL;

	// �f�[�^�o��
	return ReadSync(dwReadSize);
}

const bool CFileReader::StartReadAnsync(const DWORD dwReadSize, const ULONGLONG llReadPos)
{
	if(!m_bKillSignal)return false;
	
	if(m_hReadAnsyncThread){
		::CloseHandle(m_hReadAnsyncThread);
		m_hReadAnsyncThread = NULL;
		}

	// �t�@�C���V�[�N
	if(!m_InFile.Seek(llReadPos))return false;

	// �񓯊����[�h�X���b�h�N��
	m_dwReadAnsyncThreadID = 0UL;
	m_bKillSignal = false;

	if(!(m_hReadAnsyncThread = ::CreateThread(NULL, 0UL, CFileReader::ReadAnsyncThread, (LPVOID)this, 0UL, &m_dwReadAnsyncThreadID))){
		return false;
		}

	return true;
}

void CFileReader::StopReadAnsync(void)
{
	// �񓯊����[�h��~
	m_bKillSignal = true;
}

const ULONGLONG CFileReader::GetReadPos(void) const
{
	// �t�@�C���|�W�V������Ԃ�
	return m_InFile.GetPos();
}

const ULONGLONG CFileReader::GetFileSize(void) const
{
	// �t�@�C���T�C�Y��Ԃ�
	return m_InFile.GetSize();
}

DWORD WINAPI CFileReader::ReadAnsyncThread(LPVOID pParam)
{
	// �񓯊����[�h�X���b�h(�t�@�C�����[�h�ƃO���t������ʃX���b�h�ɂ���Ƃ�萫�\�����シ��)
	CFileReader *pThis = static_cast<CFileReader *>(pParam);

	// �u�񓯊����[�h�J�n�v�C�x���g�ʒm
	pThis->SendDecoderEvent(EID_READ_ASYNC_START);

	while(!pThis->m_bKillSignal && (pThis->m_InFile.GetPos() < min(pThis->m_InFile.GetSize(),pThis->m_ReadSizeLimit))){
		
		// �u�񓯊����[�h�O�v�C�x���g�ʒm
		if(pThis->SendDecoderEvent(EID_READ_ASYNC_PREREAD))break;
		
		// �t�@�C���������[�h
		pThis->ReadSync();
			
		// �u�񓯊����[�h��v�C�x���g�ʒm
		if(pThis->SendDecoderEvent(EID_READ_ASYNC_POSTREAD))break;
		}

	// �u�񓯊����[�h�I���v�C�x���g�ʒm
	pThis->SendDecoderEvent(EID_READ_ASYNC_END);

	pThis->m_bKillSignal = true;

	return 0UL;
}
