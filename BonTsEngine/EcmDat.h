#pragma once
#include "windows.h"

#include "RingBuffer.h"

#include<iostream>
#include<queue>

using namespace std;

typedef struct _ECM_DAT{
	BYTE key[25];
	ULONG magic;
	DWORD time;
}ECM_DAT;


// Critical Section Lapper
class CEcmDat
{
public:
	CEcmDat();
	~CEcmDat();

	void set(BYTE* data,ULONG size);
	int size(void){ return (int)m_q.size(); }
	ECM_DAT get(ULONG magic,BOOL &ret);
	void SetDiag(BOOL flg){m_diagmode = flg;}
	void exit(void);
	void GetKsKey(BYTE* cmd,ULONG len,BYTE* ret);
	
private:

	CRingBuffer*		m_pbuf;
	deque<ECM_DAT>  m_q;
	HANDLE m_handle;
	BOOL m_exit;
	BOOL m_diagmode;
	BOOL m_start;
	CRITICAL_SECTION m_CriticalSection;

	void Lock(void);
	void UnLock(void);
};
