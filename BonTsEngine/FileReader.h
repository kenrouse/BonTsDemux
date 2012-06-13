// FileReader.h: CFileReader クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "ncachedfile.h"


/////////////////////////////////////////////////////////////////////////////
// 汎用ファイル入力(ファイルから読み込んだデータをデコーダグラフに入力する)
/////////////////////////////////////////////////////////////////////////////
// Output	#0	: CMediaData		読み込みデータ
/////////////////////////////////////////////////////////////////////////////


#define DEF_READSIZE	0x00200000UL		// 2MB
#define READ_TO_FILEEND	QWORD_MAX			// ファイル終端まで

class CFileReader : public CMediaDecoder  
{
public:
	enum EVENTID
	{
		EID_READ_ASYNC_START,		// 非同期リード開始
		EID_READ_ASYNC_END,			// 非同期リード終了
		EID_READ_ASYNC_PREREAD,		// 非同期リード前
		EID_READ_ASYNC_POSTREAD		// 非同期リード後
	};

	CFileReader(CDecoderHandler *pDecoderHandler);
	virtual ~CFileReader();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex);

// CFileReader
	const bool OpenFile(LPCTSTR lpszFileName,QWORD qwReadSizeLimit=READ_TO_FILEEND);
	void CloseFile(void);

	const DWORD ReadSync(const DWORD dwReadSize = DEF_READSIZE);
	const DWORD ReadSync(const DWORD dwReadSize, const ULONGLONG llReadPos);

	const bool StartReadAnsync(const DWORD dwReadSize = DEF_READSIZE, const ULONGLONG llReadPos = 0ULL);
	void StopReadAnsync(void);

	const ULONGLONG GetReadPos(void) const;
	const ULONGLONG GetFileSize(void) const;

protected:
//	CNFile m_InFile;
	CNCachedFile m_InFile;
	CMediaData m_ReadBuffer;
	QWORD m_ReadSizeLimit;

	HANDLE m_hReadAnsyncThread;
	DWORD m_dwReadAnsyncThreadID;
	bool m_bKillSignal;
	
private:
	static DWORD WINAPI ReadAnsyncThread(LPVOID pParam);
};
