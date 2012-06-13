// FileWriter.cpp: CFileWriter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CFileWriter::CFileWriter(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_llWriteSize(0U)
	, m_llWriteCount(0U)
{
	m_CachePtr = 0;
	m_abyCache = new BYTE[FILE_CACHE_SIZE];
}

CFileWriter::~CFileWriter()
{
	CloseFile();
	delete [] m_abyCache;
}

void CFileWriter::Reset(void)
{
	m_CachePtr = 0;
}

const DWORD CFileWriter::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CFileWriter::GetOutputNum(void) const
{
	return 0UL;
}

//2010.05.07 �L���b�V���Ɏc���Ă�����e���t�@�C���ɏo�͂���B 
void CFileWriter::Flush()
{
	if (m_CachePtr > 0 ) {
		m_OutFile.Write(m_abyCache,m_CachePtr);
		m_llWriteSize += FILE_CACHE_SIZE;
		m_OutFile.Flush();
		m_CachePtr = 0;
	}
}

const bool CFileWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;
	if(m_CachePtr + pMediaData->GetSize() < FILE_CACHE_SIZE){

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), pMediaData->GetSize());
		m_CachePtr += pMediaData->GetSize();

	} else {

		DWORD len = pMediaData->GetSize();
		BYTE* buf = pMediaData->GetData();

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), FILE_CACHE_SIZE - m_CachePtr);	// �܂��A�p�P�b�g�𖄂߂�
		m_OutFile.Write(m_abyCache,FILE_CACHE_SIZE);
		m_llWriteSize += FILE_CACHE_SIZE;

		buf += (FILE_CACHE_SIZE - m_CachePtr);
		len -= (FILE_CACHE_SIZE - m_CachePtr);

		m_CachePtr = 0;
		while(len){
			if(len >= FILE_CACHE_SIZE){
				m_OutFile.Write(buf,FILE_CACHE_SIZE);
				m_llWriteSize += FILE_CACHE_SIZE;
				len -= FILE_CACHE_SIZE;
				buf += FILE_CACHE_SIZE;
			} else {
				memcpy(m_abyCache,buf,len);
				m_CachePtr += len;
				break;
			}
		}
	}

	return true;
}

const bool CFileWriter::OpenFile(LPCTSTR lpszFileName, const BYTE bFlags)
{
	// ��U����
	CloseFile();

	// �t�@�C�����J��
	BYTE bFlags2 = CNCachedFile::CNF_WRITE | CNCachedFile::CNF_NEW | CNCachedFile::CNF_SHAREREAD | bFlags;
	return (m_OutFile.Open(lpszFileName, bFlags2))? true : false;
}

void CFileWriter::CloseFile(void)
{
	// �t�@�C�������
	Flush();	//2010.05.07 
	m_OutFile.Close();
	
	m_llWriteSize = 0U;
	m_llWriteCount = 0U;
}

const LONGLONG CFileWriter::GetWriteSize(void) const
{
	// �������ݍς݃T�C�Y��Ԃ�
	return m_llWriteSize;
}

const LONGLONG CFileWriter::GetWriteCount(void) const
{
	// �������݉񐔂�Ԃ�
	return m_llWriteCount;
}
