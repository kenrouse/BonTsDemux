// FileWriter.cpp: CFileWriter クラスのインプリメンテーション
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
// 構築/消滅
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

//2010.05.07 キャッシュに残っている内容をファイルに出力する。 
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

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), FILE_CACHE_SIZE - m_CachePtr);	// まず、パケットを埋める
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
	// 一旦閉じる
	CloseFile();

	// ファイルを開く
	BYTE bFlags2 = CNCachedFile::CNF_WRITE | CNCachedFile::CNF_NEW | CNCachedFile::CNF_SHAREREAD | bFlags;
	return (m_OutFile.Open(lpszFileName, bFlags2))? true : false;
}

void CFileWriter::CloseFile(void)
{
	// ファイルを閉じる
	Flush();	//2010.05.07 
	m_OutFile.Close();
	
	m_llWriteSize = 0U;
	m_llWriteCount = 0U;
}

const LONGLONG CFileWriter::GetWriteSize(void) const
{
	// 書き込み済みサイズを返す
	return m_llWriteSize;
}

const LONGLONG CFileWriter::GetWriteCount(void) const
{
	// 書き込み回数を返す
	return m_llWriteCount;
}
