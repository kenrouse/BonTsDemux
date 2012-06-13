// FIFO Buffer
//  CBuffer   : 通常版
//  CBufferMT : マルチスレッド時の非同期アクセスをCriticalSectionで保護
//  CBuffer_CriticalSection : 内部使用専用

#pragma once

#include "Lock.h"
#include <list>

// デフォルトのバッファサイズ(1,048,576 byte)
#define CBUFFER_DEFAULT_FIFO_SIZE	(1<<20)

// デバッグ用
#define USE_CRITICAL_SECTION

struct FIFOBUFFER_DATA
{
	BYTE *pData;
	DWORD dwDataSize;
	DWORD dwReadPos;
};

// Buffer
class CBuffer
{
public:
	CBuffer(DWORD dwBufferSize=CBUFFER_DEFAULT_FIFO_SIZE);
	virtual ~CBuffer(void);

	BOOL Alloc(DWORD dwBufferSize);
	void Clear(void);

	DWORD GetBufferSize();
	DWORD GetBufferEmptySize();
	DWORD GetDataSize();

	DWORD Put(BYTE *pData,DWORD dwDataSize,BOOL bAppendFront=FALSE);
	DWORD Get(BYTE *pBuffer,DWORD dwBufferSize,BOOL bPositionForward=TRUE);
	DWORD GetFrontBuffer(BYTE *pBuffer,DWORD dwBufferSize,BOOL bPositionForward=TRUE);
	BYTE ReferenceTopByte();
	DWORD crc32(DWORD dwSize);

protected:
	DWORD				m_dwBufferSize;
	std::list<FIFOBUFFER_DATA> m_listData;
	DWORD				m_dwDataSize;
};

// Multi Thread Version(with Critical Section)
class CBufferMT : public CBuffer
{
public:
	CBufferMT(DWORD dwBufferSize=CBUFFER_DEFAULT_FIFO_SIZE);
	virtual ~CBufferMT(void);

	BOOL Alloc(DWORD dwBufferSize);
	void Clear(void);

	DWORD Put(BYTE *pData,DWORD dwDataSize,BOOL bAppendFront=FALSE);
	DWORD Get(BYTE *pBuffer,DWORD dwBufferSize,BOOL bPositionForward=TRUE);
	DWORD GetFrontBuffer(BYTE *pBuffer,DWORD dwBufferSize,BOOL bPositionForward=TRUE);
	BYTE ReferenceTopByte();
	DWORD crc32(DWORD dwSize);

protected:
	LOCK_OBJ	m_LockObj;
};
