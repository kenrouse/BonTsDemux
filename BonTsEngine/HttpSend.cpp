// ProgManager.cpp: CHttpSend クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "httpsend.h"

#include <process.h>

//////////////////////////////////////////////////////////////////////
//Static要素

DWORD 				CHttpSend::m_ReqSocket = 0;
vector<SOCKET*>		CHttpSend::m_SocketAry;
HANDLE 				CHttpSend::m_thread = INVALID_HANDLE_VALUE;
SOCKET 				CHttpSend::m_server_sock = INVALID_SOCKET;
USHORT 				CHttpSend::m_port = 0;
BOOL 				CHttpSend::m_raw_tcp = 0;

int count = 0;
//Httpサーバースレッド。要求されたソケット分の接続を確立したら終了する。
unsigned __stdcall CHttpSend::ServerThread(LPVOID parm)
{
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	DWORD sock_num = 0;
	SOCKET sock;
	BOOL yes = 1;
	char buf[128];
	char inbuf[128];
//	CHttpSend* p = (CHttpSend*)parm;
	m_server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server_sock == INVALID_SOCKET) {
//		log_out("HttpSend socket WSAGetLastError : %d\n", WSAGetLastError());
		return 0;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	
	setsockopt(m_server_sock,
		SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
	
	if (bind(m_server_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
//		log_out("HttpSend bind WSAGetLastError : %d\n", WSAGetLastError());
		return 0;
	}
	
	if (listen(m_server_sock, 5) != 0) {
//		log_out("HttpSend listen WSAGetLastError : %d\n", WSAGetLastError());
		return 0;
	}
	
	memset(buf, 0, sizeof(buf));
	GetModuleHandle(NULL);
//	sprintf(buf,"HTTP/1.0 200 OK\r\nContent-Length: 1397256\r\nContent-Type: audio/media\r\n\r\n");
	sprintf(buf,"HTTP/1.0 200 OK\r\n\r\n");
	
	while (1) {
		len = sizeof(client);
		sock = accept(m_server_sock, (struct sockaddr *)&client, &len);
		if (sock == INVALID_SOCKET) {
//			log_out("HttpSend accept WSAGetLastError : %d\n", WSAGetLastError());
			break;
		}
		
		if(!m_raw_tcp){
			memset(inbuf, 0, sizeof(inbuf));
			recv(sock, inbuf, sizeof(inbuf), 0);
		//	_mbslwr((unsigned char *)inbuf);		// !!! buffer overrun
//			printf("%s", inbuf);

			send(sock, buf, (int)strlen(buf), 0);
		}
		*m_SocketAry[sock_num++] = sock;
		if(sock_num >= m_ReqSocket){
			break;
		}
	}

	closesocket(m_server_sock);
	m_server_sock = INVALID_SOCKET;
	
	return 0;

}


//////////////////////////////////////////////////////////////////////
// CHttpSend 構築/消滅
//////////////////////////////////////////////////////////////////////

CHttpSend::CHttpSend(CDecoderHandler *pDecoderHandler,USHORT Port,DWORD cache_size)
	: CMediaDecoder(pDecoderHandler)
{
	// プログラムデータベースインスタンス生成
	m_cache_size = cache_size;
	if(m_ReqSocket == 0){
		m_SocketAry.clear();
	}
	m_ReqSocket++;
	m_port = Port;

	m_sock = INVALID_SOCKET;
	m_SocketAry.push_back(&m_sock);

	m_CachePtr = 0;
	m_abyCache = new BYTE[m_cache_size];

	m_Header = NULL;
	m_HeaderLen = 0;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
	m_connection = FALSE;

	m_raw_tcp = 0;

	if(m_thread == INVALID_HANDLE_VALUE){
		m_thread = (HANDLE)_beginthreadex(NULL,0,&CHttpSend::ServerThread,(LPVOID)this,0,NULL);
	}

}

CHttpSend::~CHttpSend()
{
	// プログラムデータベースインスタンス開放
	m_ReqSocket--;

	if(m_ReqSocket == 0 && m_thread != INVALID_HANDLE_VALUE){
		if(m_server_sock != INVALID_SOCKET){
			closesocket(m_server_sock);
		}
		WaitForSingleObject(m_thread,INFINITE);
		CloseHandle(m_thread);
		m_thread = INVALID_HANDLE_VALUE;

	}
	if(m_sock != INVALID_SOCKET){
		closesocket(m_sock);
	}

	if(m_abyCache)
		delete [] m_abyCache;
	if(m_Header)
		delete [] m_Header;

	WSACleanup();	
}

void CHttpSend::Close()
{
	if(m_sock != INVALID_SOCKET){
		closesocket(m_sock);
	}
	m_sock = INVALID_SOCKET;
}

void CHttpSend::Reset()
{
	// 下位デコーダをリセット
	CMediaDecoder::Reset();
}


const DWORD CHttpSend::GetInputNum() const
{
	return 1UL;
}

const DWORD CHttpSend::GetOutputNum() const
{
	return 0UL;		// 末端
}

void CHttpSend::SetHeader(const BYTE* data,DWORD len)
{
	if(m_Header) delete [] m_Header;
	m_Header = new BYTE[len];
	memcpy(m_Header,data,len);
	m_HeaderLen = len;
}

BOOL CHttpSend::IsOpen(void)
{
	int i;
	
	if(m_connection) return TRUE;
	if(m_sock == INVALID_SOCKET){			// 排他甘いです。
		for(i = 0 ; i < 100 ; i ++){
			Sleep(100);
			if(m_sock != INVALID_SOCKET) break;
		}
		if(i == 100) return FALSE;
	}
	m_connection = TRUE;
	return TRUE;
}

BOOL CHttpSend::Send(BYTE* buff,DWORD len)
{
	size_t ret;
	if(IsOpen() == FALSE){
//		log_out("CHttpSend : Socket Connection Error\n");
		return FALSE;
	}
	if(m_Header){
		ret = send(m_sock,(const char*)m_Header,m_HeaderLen,0);
		delete [] m_Header;
		m_Header = NULL;
		m_HeaderLen = 0;
	}
	if(m_sock != INVALID_SOCKET){
		ret = send(m_sock,(const char*)buff,len,0);
		if(ret == -1){
#ifdef _DEBUG
//			log_out("CHttpSend Send WSAGetLastError : %d\n", WSAGetLastError());
#endif
				closesocket(m_sock);
				m_sock = INVALID_SOCKET;
				SendDecoderEvent(EID_ERROR_SOCKET);
			return FALSE;
		}
	}
	return TRUE;
}

const bool CHttpSend::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(m_CachePtr + pMediaData->GetSize() < m_cache_size){

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), pMediaData->GetSize());
		m_CachePtr += pMediaData->GetSize();

	} else {

		DWORD len = pMediaData->GetSize();
		BYTE* buf = pMediaData->GetData();

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), m_cache_size - m_CachePtr);	// まず、パケットを埋める
		if(Send(m_abyCache,m_cache_size) == FALSE) return false;

		buf += (m_cache_size - m_CachePtr);
		len -= (m_cache_size - m_CachePtr);

		m_CachePtr = 0;
		while(len){
			if(len >= m_cache_size){
				if(Send(buf,m_cache_size) == FALSE) return false;
				len -= m_cache_size;
				buf += m_cache_size;
			} else {
				memcpy(m_abyCache,buf,len);
				m_CachePtr += len;
				break;
			}
		}
//		ret = Send(pMediaData->GetData(), pMediaData->GetSize());		// ここに来てはいけない
	}

//		ret = Send(pMediaData->GetData(), pMediaData->GetSize());		// ここに来てはいけない

	// 次のフィルタにデータを渡す
	OutputMedia(pMediaData);
	return true;
}

//2010.05.07 バッファにたまっているデータをはき出す。
void CHttpSend::Flush()
{
	if(m_CachePtr > 0) 
	{
		Send(m_abyCache,m_CachePtr);
		m_CachePtr = 0;
	}
}
