#include "stdafx.h"
#include "TsBuffer.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTsBuffer::CTsBuffer(DWORD MaxBuffer,DWORD MarginBuffer) : CMediaDecoder(NULL)
{
	m_dwMargin = MarginBuffer;
	m_pBuff = new CRingBuffer(MaxBuffer);
//	m_dwSize = 0;
}

CTsBuffer::~CTsBuffer(void)
{
/*
	if(m_hThread){
		m_Terminate = TRUE;
		SetEvent(m_hEvent);
		WaitForSingleObject(m_hThread,1000);
		m_hThread = NULL;
	}
	CloseHandle(m_hEvent);
	::DeleteCriticalSection(&m_CriticalSection);
*/	

	delete m_pBuff;
}


const DWORD CTsBuffer::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CTsBuffer::GetOutputNum(void) const
{
	return 1UL;
}

const bool CTsBuffer::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	bool bReturn = true;
/*
	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);
	m_q.push_back(*pTsPacket);
	m_dwSize += pTsPacket->GetSize();

	if(m_dwSize > m_dwMargin){
		m_dwSize -= m_q.front().GetSize();
		OutputMedia(&m_q.front());
//		m_q.front().Delete();
		m_q.pop_front();
	}
*/

	m_pBuff->set(pMediaData->GetData(),pMediaData->GetSize());

	DWORD size = m_pBuff->size();
	if(size > m_dwMargin){
		size -= m_dwMargin;
		BYTE* buff = new BYTE[size];

		m_pBuff->get(buff,size);			
		m_pBuff->flash(size);

		CMediaData* data = new CMediaData(buff, size);

		delete [] buff;

		OutputMedia(data);

		data->Delete();

	}
	
	return bReturn;
}

void CTsBuffer::Reset(void)
{

}
