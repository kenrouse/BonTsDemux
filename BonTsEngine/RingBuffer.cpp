#include "stdafx.h"
#include "RingBuffer.h"

CRingBuffer::CRingBuffer(ULONG size,char* name)
{
	::InitializeCriticalSection(&m_CriticalSection);

	reset();

	m_max = size;
	m_name = name;

	m_ptr = new char[size];
}

CRingBuffer::~CRingBuffer()
{
	delete [] m_ptr;

	::DeleteCriticalSection(&m_CriticalSection);
}

void CRingBuffer::Lock(void)
{
	::EnterCriticalSection(&m_CriticalSection);
}

void CRingBuffer::UnLock(void)
{
	::LeaveCriticalSection(&m_CriticalSection);
}

void CRingBuffer::reset(void)
{
	m_size = 0;
	m_read = 0;
	m_write = 0;
	m_full_msg = FALSE;
}

void CRingBuffer::set(BYTE* data,ULONG size)
{
	ULONG size_org = size;

	Lock();
	
	if(m_write + size >= m_max){		//RINGŒ×‚®‚Æ‚«
		if(m_write < m_read){
			if(!m_full_msg){
//				log_out("Warning : %s Buffer FULL\n",m_name);
				m_full_msg = TRUE;
			}
			reset();
			goto END;
		}
		ULONG modlen = m_max - m_write;
		size = size - modlen;
		if(m_read <= size){
			if(!m_full_msg){
//				log_out("Warning : %s Buffer FULL\n",m_name);
				m_full_msg = TRUE;
			}
			reset();
			goto END;
		}
		memcpy((void*)&m_ptr[m_write],(void*)data,modlen);
		m_write = 0;
		data += modlen;
	}
	if(m_write > m_read && m_write + size <= m_read ||
		m_write < m_read && m_write + size >= m_read){
		if(!m_full_msg){
//			log_out("Warning : %s Buffer FULL\n",m_name);
			m_full_msg = TRUE;
		}
		reset();
		goto END;
	}
	memcpy((void*)&m_ptr[m_write],data,size);
	m_write += size;
	m_size += size_org;
	m_full_msg = FALSE;

END:
	UnLock();

}

void CRingBuffer::get(BYTE* data,ULONG size)
{
	Lock();

	if(m_read + size >= m_max){		//RINGŒ×‚®‚Æ‚«
		ULONG modlen = m_max - m_read;
		memcpy((void*)data,(void*)&m_ptr[m_read],modlen);
		size = size - modlen;
		data += modlen;
		memcpy(data,(void*)&m_ptr[0],size);
		goto END;
	}
	memcpy(data,(void*)&m_ptr[m_read],size);

END:
	UnLock();
}

void CRingBuffer::flash(ULONG size)
{
	Lock();

	m_size -= size;
	
	if(m_read + size >= m_max){		//RINGŒ×‚®‚Æ‚«
		ULONG modlen = m_max - m_read;
		m_read = 0;
		size -= modlen;
	}
	m_read += size;

	UnLock();
}
