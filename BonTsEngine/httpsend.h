// ProgManager.h: CHttpSend クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MediaDecoder.h"
#include "winsock.h"

#include<vector>

using namespace std;

#define HTTPSEND_CACHE_SIZE	m_cache_size//(8*1024)

class CHttpSend : public CMediaDecoder
{
public:
//	enum EVENTID
//	{
//		EID_SERVICE_LIST_UPDATED,	// サービスリスト更新
//		EID_SERVICE_INFO_UPDATED	// サービス情報更新
//	};
	enum EVENTID
	{
		EID_ERROR_SOCKET,
	};

	CHttpSend(CDecoderHandler *pDecoderHandler,USHORT port=1234,DWORD cache_size=(8*1024));
	virtual ~CHttpSend();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// HttpSend
	void Close();
	void Flush();	//2010.05.07 fuji 
	void SetHeader(const BYTE* data,DWORD len);
	BOOL IsOpen(void);
	void RawTcp(BOOL flag){m_raw_tcp = flag;}



	BYTE* m_Header;
	DWORD m_HeaderLen;
	

private:
	SOCKET m_sock;
	DWORD m_CachePtr;
	BYTE* m_abyCache;
	BOOL Send(BYTE* buff,DWORD len);
	DWORD m_cache_size;
	static BOOL m_raw_tcp;

	static DWORD m_ReqSocket;
	static vector<SOCKET*> m_SocketAry;
	static HANDLE m_thread;
	static SOCKET m_server_sock;
	static USHORT m_port;
	static unsigned __stdcall ServerThread(LPVOID parm);
	BOOL m_connection;

};

