#pragma once

#include "MediaDecoder.h"

#include<iostream>
#include<queue>

using namespace std;

class CKeepRate  : public CMediaDecoder  
{

public:
	CKeepRate(DWORD PacketSize,DWORD Margin=(512*1024),DWORD BufferMax=(1*1024*1024),DWORD max_bps=(40*1024*1024),BOOL copydata = FALSE,LPCTSTR name = FALSE,CDecoderHandler* pDecoderHandler=NULL);
	~CKeepRate(void);

	// IMediaDecoder

	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0);

	const bool InputMedia_Func(CMediaData *pMediaData, const DWORD dwInputIndex);
	void StopTrans(void){	m_term = TRUE;	};
	void CompleteTrans(void);			//2010.05.07 fuji
	DWORD WaitForThread(DWORD timeout);	//2010.05.07 fuji

	BOOL m_term;

	void Lock(void);
	void UnLock(void);

	deque<CMediaData*>  m_q;
	
	HANDLE m_event;
	DWORD m_Bps;
	DWORD m_max;
	DWORD m_margin;
	BOOL m_bThreadIdle;
	DWORD m_PacketSize;
	BOOL m_copydata;
	LPCTSTR m_name;
	BOOL m_debugmsg;
	BOOL m_complete; //2010.05.07 fuji 

private:

	CRITICAL_SECTION m_CriticalSection;
	
	HANDLE m_thread;
	
};
