#pragma once
#include "windows.h"

// Critical Section Lapper
class CRingBuffer
{
public:
	CRingBuffer(ULONG size=1024,char* name = "TS");
	~CRingBuffer();

	void set(BYTE* data,ULONG size);
	void get(BYTE* data,ULONG size);
	ULONG size(void){return m_size;}
	void flash(ULONG size);

	BYTE 			// �w�肳�ꂽ�|�C���^�̒l
	at(
		ULONG size	// �ǂݍ��ރ|�C���g�̒l
	)
	{
		if(m_read + size >= m_max){		//RING�ׂ��Ƃ�
			ULONG modlen = m_max - m_read;
			m_read = 0;
			size -= modlen;
		}
		return m_ptr[size];
	}

	void reset(void);

	ULONG read(void){return m_read;}
	ULONG write(void){return m_write;}
	ULONG max_size(void){return m_max;}
	
	
private:
	ULONG m_size;
	ULONG m_read;
	ULONG m_write;
	ULONG m_max;
	char* m_ptr;
	BOOL m_full_msg;
	
	CRITICAL_SECTION m_CriticalSection;
	void Lock(void);
	void UnLock(void);

	char* m_name;
};
