// FileWriter.h: CFileWriter クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "ncachedfile.h"


/////////////////////////////////////////////////////////////////////////////
// 汎用ファイル出力(CMediaDataをそのままファイルに書き出す)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		書き込みデータ
/////////////////////////////////////////////////////////////////////////////
#define FILE_CACHE_SIZE (64*1024)		// 一定サイズごとに、ファイルに書き込む。ネットワークドライブ対応

class CFileWriter : public CMediaDecoder  
{
public:
	CFileWriter(CDecoderHandler *pDecoderHandler);
	virtual ~CFileWriter();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CFileWriter
	const bool OpenFile(LPCTSTR lpszFileName, const BYTE bFlags = 0UL);
	void CloseFile(void);
	void Flush(void);	//2010.05.07
	const LONGLONG GetWriteSize(void) const;
	const LONGLONG GetWriteCount(void) const;

protected:
	CNCachedFile m_OutFile;
	
	LONGLONG m_llWriteSize;
	LONGLONG m_llWriteCount;

	DWORD m_CachePtr;
	BYTE* m_abyCache;
};
