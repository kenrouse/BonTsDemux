// FileWriter.cpp: CFileWriter クラスのインプリメンテーション
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
// 構築/消滅
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
	// 下位デコーダをリセットする
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
	// ソースでコーダのため常にエラーを返す
	return false;
}

const bool CFileReader::OpenFile(LPCTSTR lpszFileName,QWORD qwReadSizeLimit)
{
	// 一旦閉じる
	CloseFile();

	// ファイルを開く
	m_ReadSizeLimit = qwReadSizeLimit;
	return (m_InFile.Open(lpszFileName, CNCachedFile::CNF_READ | CNCachedFile::CNF_SHAREREAD | CNCachedFile::CNF_SHAREWRITE))? true : false;
}

void CFileReader::CloseFile(void)
{
	// ファイルを閉じる
	StopReadAnsync();
	m_InFile.Close();
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize)
{
	// 読み込みサイズ計算
	ULONGLONG llRemainSize = m_InFile.GetSize() - m_InFile.GetPos();
	const DWORD dwReqSize = (llRemainSize > (ULONGLONG)dwReadSize)? dwReadSize : (DWORD)llRemainSize;
	if(!dwReqSize)return 0UL;

	// バッファ確保
	//m_ReadBuffer.SetSize(dwReadSize);
	m_ReadBuffer.SetSize(dwReqSize); //2010.05.07 読み込んだサイズにセットする。
	
	// ファイル読み込み
	if(!m_InFile.Read(m_ReadBuffer.GetData(), dwReqSize))return 0UL;
	
	// データ出力
	OutputMedia(&m_ReadBuffer);
	
	return dwReqSize;
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize, const ULONGLONG llReadPos)
{
	// ファイルシーク
	if(!m_InFile.Seek(llReadPos))return 0UL;

	// データ出力
	return ReadSync(dwReadSize);
}

const bool CFileReader::StartReadAnsync(const DWORD dwReadSize, const ULONGLONG llReadPos)
{
	if(!m_bKillSignal)return false;
	
	if(m_hReadAnsyncThread){
		::CloseHandle(m_hReadAnsyncThread);
		m_hReadAnsyncThread = NULL;
		}

	// ファイルシーク
	if(!m_InFile.Seek(llReadPos))return false;

	// 非同期リードスレッド起動
	m_dwReadAnsyncThreadID = 0UL;
	m_bKillSignal = false;

	if(!(m_hReadAnsyncThread = ::CreateThread(NULL, 0UL, CFileReader::ReadAnsyncThread, (LPVOID)this, 0UL, &m_dwReadAnsyncThreadID))){
		return false;
		}

	return true;
}

void CFileReader::StopReadAnsync(void)
{
	// 非同期リード停止
	m_bKillSignal = true;
}

const ULONGLONG CFileReader::GetReadPos(void) const
{
	// ファイルポジションを返す
	return m_InFile.GetPos();
}

const ULONGLONG CFileReader::GetFileSize(void) const
{
	// ファイルサイズを返す
	return m_InFile.GetSize();
}

DWORD WINAPI CFileReader::ReadAnsyncThread(LPVOID pParam)
{
	// 非同期リードスレッド(ファイルリードとグラフ処理を別スレッドにするとより性能が向上する)
	CFileReader *pThis = static_cast<CFileReader *>(pParam);

	// 「非同期リード開始」イベント通知
	pThis->SendDecoderEvent(EID_READ_ASYNC_START);

	while(!pThis->m_bKillSignal && (pThis->m_InFile.GetPos() < min(pThis->m_InFile.GetSize(),pThis->m_ReadSizeLimit))){
		
		// 「非同期リード前」イベント通知
		if(pThis->SendDecoderEvent(EID_READ_ASYNC_PREREAD))break;
		
		// ファイル同期リード
		pThis->ReadSync();
			
		// 「非同期リード後」イベント通知
		if(pThis->SendDecoderEvent(EID_READ_ASYNC_POSTREAD))break;
		}

	// 「非同期リード終了」イベント通知
	pThis->SendDecoderEvent(EID_READ_ASYNC_END);

	pThis->m_bKillSignal = true;

	return 0UL;
}
