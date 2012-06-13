#include "stdafx.h"
#include "NetworkSend.h"

#pragma comment(lib,"ws2_32.lib")


CNetworkSend::CNetworkSend(DWORD dwSendBlockSize) : CMediaDecoder(NULL)
{
	m_pBuf=new CBufferMT(max(1<<20,dwSendBlockSize*2));
	m_sock=INVALID_SOCKET;
	m_dwBlockSendSize=dwSendBlockSize;
	m_dwSendStartTick=0;
	m_ui64SendSizeTotal=0;

	WSADATA stData;
	WSAStartup(MAKEWORD(2,0),&stData);
	m_CachePtr = 0;

	m_abyCache = new BYTE[CACHE_SIZE];
}

CNetworkSend::~CNetworkSend(void)
{
	Close();
	delete m_pBuf;
	WSACleanup();
	delete [] m_abyCache;
}

BOOL CNetworkSend::Open(char *address,WORD port)
{	
	if(IsOpen()) Close();

	struct hostent *pHost;
	pHost=gethostbyname(address);
	if(!pHost) return FALSE;

	m_sock=socket(AF_INET,SOCK_DGRAM,0);
	if(m_sock==INVALID_SOCKET) return FALSE;

	m_addr.sin_family=AF_INET;
	m_addr.sin_port=htons(port);
	m_addr.sin_addr.S_un.S_un_b.s_b1=(BYTE)*((pHost->h_addr_list[0])+0);
	m_addr.sin_addr.S_un.S_un_b.s_b2=(BYTE)*((pHost->h_addr_list[0])+1);
	m_addr.sin_addr.S_un.S_un_b.s_b3=(BYTE)*((pHost->h_addr_list[0])+2);
	m_addr.sin_addr.S_un.S_un_b.s_b4=(BYTE)*((pHost->h_addr_list[0])+3);

	m_ui64SendSizeTotal=0;
	m_dwSendStartTick=GetTickCount();

	return TRUE;
}

void CNetworkSend::Close()
{
	if(IsOpen())
	{
		closesocket(m_sock);
		m_sock=INVALID_SOCKET;
	}
}

BOOL CNetworkSend::IsOpen()
{
	return m_sock!=INVALID_SOCKET;
}

DWORD CNetworkSend::GetOpenTick()
{
	if(!IsOpen()) return 0;
	return GetTickCount()-m_dwSendStartTick;
}

BOOL CNetworkSend::Send(void *lpData,DWORD dwDataSize,DWORD *lpdwWritten)
{
	if(!IsOpen()) return FALSE;
	if(m_dwBlockSendSize==0)
	{
		int ret=0;
		ret=sendto(m_sock,(char*)lpData,dwDataSize,0,(struct sockaddr *)&m_addr,sizeof(m_addr));
		if(ret!=SOCKET_ERROR)
		{
			m_ui64SendSizeTotal+=ret;
			if(lpdwWritten) *lpdwWritten=ret;
			return TRUE;
		} else {
			if(lpdwWritten) *lpdwWritten=0;
			return FALSE;
		}
	} else {
		DWORD dwSize;
		dwSize=m_pBuf->Put((BYTE*)lpData,dwDataSize);
		if(dwSize==dwDataSize)
		{
			if(m_pBuf->GetDataSize()>m_dwBlockSendSize)
			{
				return _SendBlock(m_dwBlockSendSize,lpdwWritten);
			}
			if(lpdwWritten) *lpdwWritten=0;
			return TRUE;
		} else {
			return FALSE;
		}		
	}
	return TRUE;
}

BOOL CNetworkSend::_SendBlock(DWORD dwSendSize,DWORD *lpdwWritten)
{
	if(!IsOpen()) return FALSE;

	BOOL bRet=FALSE;
	DWORD dwSize;
	BYTE *pBuf=new BYTE[m_dwBlockSendSize];
	int ret=0;

	if(m_dwBlockSendSize>0)
	{
		dwSize=m_pBuf->Get(pBuf,dwSendSize);
	} else {
		dwSize=m_pBuf->Get(pBuf,m_dwBlockSendSize);
	}
	ret=sendto(m_sock,(char*)pBuf,dwSize,0,(struct sockaddr *)&m_addr,sizeof(m_addr));
	if(ret!=SOCKET_ERROR)
	{
		m_ui64SendSizeTotal+=ret;
		if(lpdwWritten) *lpdwWritten=ret;
		bRet=TRUE;
	}
	delete []pBuf;
	return bRet;
}




const DWORD CNetworkSend::GetInputNum(void) const
{
	return 1;
}

const DWORD CNetworkSend::GetOutputNum(void) const
{
	return 0;
}

const bool CNetworkSend::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{

	bool bReturn = true;

	if(m_CachePtr + pMediaData->GetSize() <= CACHE_SIZE){

		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), pMediaData->GetSize());
		m_CachePtr += pMediaData->GetSize();

	} else if (pMediaData->GetSize() <= CACHE_SIZE){

		Send(m_abyCache, m_CachePtr , NULL);
		m_CachePtr = 0;
		memcpy(&m_abyCache[m_CachePtr],pMediaData->GetData(), pMediaData->GetSize());
		m_CachePtr += pMediaData->GetSize();
		return true;

	} else {

		Send(pMediaData->GetData(), pMediaData->GetSize() , NULL);		// ここに来てはいけない

	}

	OutputMedia(pMediaData);

//	pMediaData->Delete();

	return bReturn;
}

void CNetworkSend::Reset(void)
{
	// 下流デコーダを初期化する
	CMediaDecoder::Reset();
}
