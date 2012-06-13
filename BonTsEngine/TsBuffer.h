#pragma once

#include "MediaDecoder.h"
#include "RingBuffer.h"
#include "TsPacketParser.h"

#include<iostream>
#include<queue>

using namespace std;

class CTsBuffer  : public CMediaDecoder  
{

public:
	CTsBuffer(DWORD MaxBuffer,DWORD MarginBuffer);
	~CTsBuffer(void);

	// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0);



protected:

	HANDLE m_hThread;

	deque<CTsPacket>  m_q;

	CRingBuffer*	m_pBuff;


	DWORD m_dwMargin;
	DWORD m_dwSize;


};
