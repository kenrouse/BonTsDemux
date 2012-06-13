#pragma once

#include "MediaDecoder.h"
#include "Buffer.h"

#define NETWORK_SEND_DIRECT	0

#define CACHE_SIZE (16*1024)

class CNetworkSend  : public CMediaDecoder  
{

public:
	CNetworkSend(DWORD dwSendBlockSize=0);
	~CNetworkSend(void);

	// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0);

	//NetwordSend
	BOOL Open(char *address,WORD port);
	void Close();
	BOOL IsOpen();
	DWORD GetOpenTick();
	BOOL Send(void *lpData,DWORD dwDataSize,DWORD *lpdwWritten=NULL);
	unsigned __int64 GetSendTotalSize()
	{
		return m_ui64SendSizeTotal;
	}
	void GetIpAddress(BYTE *pbyAddr)
	{
		if(IsOpen())
		{
			pbyAddr[0]=m_addr.sin_addr.S_un.S_un_b.s_b1;
			pbyAddr[1]=m_addr.sin_addr.S_un.S_un_b.s_b2;
			pbyAddr[2]=m_addr.sin_addr.S_un.S_un_b.s_b3;
			pbyAddr[3]=m_addr.sin_addr.S_un.S_un_b.s_b4;
		} else {
			pbyAddr[0]=pbyAddr[1]=pbyAddr[2]=pbyAddr[3]=0;
		}
	}

protected:
	CBufferMT *m_pBuf;
	SOCKET	m_sock;
	struct sockaddr_in m_addr;
	DWORD m_dwBlockSendSize;
	DWORD m_dwSendStartTick;
	unsigned __int64 m_ui64SendSizeTotal;

	BOOL _SendBlock(DWORD dwSendSize,DWORD *lpdwWritten);

	DWORD m_CachePtr;
	BYTE* m_abyCache;
};
