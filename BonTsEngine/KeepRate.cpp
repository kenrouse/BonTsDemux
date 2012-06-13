#include "stdafx.h"
#include "KeepRate.h"

#include <windows.h>
#include <process.h>

#define _KR_DEBUGMSG

///////////////////////////////////////////////////////////////////////////////////////////////////
// �X���b�h����

DWORD GetTime(DWORD before)
{
	DWORD now = GetTickCount();
	if(now >= before){
		return (now - before);
	} else {
		return (((DWORD)0xffffffff - before) + now + 1);
	}
}


static DWORD WINAPI thread_proc(LPVOID param)
{
	size_t size;
	DWORD  send_size = 0;	// �]�������o�C�g��
	CMediaData* media;
	DWORD time = GetTickCount();			//�]���J�n����;

	CKeepRate* p = (CKeepRate*) param;
	while(p->m_term == 0){

		p->Lock();
		size = p->m_q.size();
		if (size == 0 && p->m_complete == TRUE ) {
			p->UnLock();
			break;
		}

		if(size > p->m_margin || p->m_complete ){
			DWORD time_buff,time_acpt;
			
			media = p->m_q.back();
			#ifdef _DEBUG
			if(p->m_debugmsg)
				TRACE3("CKeepRate %s : Send %X %d\n",p->m_name ,media,GetTickCount());
			#endif
			p->m_q.pop_back();
			p->UnLock();

			send_size += media->GetSize();

			p->InputMedia_Func(media,0);

			#ifdef _DEBUG
			if(p->m_debugmsg)
				TRACE2("byte : %d(ttl:%d)\n",media->GetSize(),send_size);
			#endif
			media->Delete();

			if(p->m_Bps){
				time_acpt = send_size * 1000 / p->m_Bps;	// ���e�ł��鎞��
			} else {
				time_acpt = 0;
			}

			time_buff = GetTime(time);		// �ȑO�̓]����������A���݂̓]�������܂ł̊Ԃ̎���
			#ifdef _DEBUG
			if(p->m_debugmsg)
				TRACE2("acpt : %d  real : %d\n",time_acpt,time_buff);
			#endif
			//��������]���̏ꍇ�́ASleep�ŗ}����

			if(time_acpt > time_buff){		// ���e�ł��鎞�Ԃ��A�]���������Ԃ̂ق����������
				DWORD sleep_time = time_acpt - time_buff;
				#ifdef _DEBUG
				if(p->m_debugmsg)
					TRACE2("CKeepRate %s : Sleep %d ms\n",p->m_name ,sleep_time);
				#endif
				Sleep(sleep_time);
			}
			if(time_buff > 500){
				send_size = 0;
				time = GetTickCount();			//�]���J�n����
			}
		} else {
			p->UnLock();
		}


		if(size <= p->m_margin && p->m_term == 0){
			#ifdef _DEBUG
			if(p->m_debugmsg)
				TRACE1("CKeepRate %s : Buffer Empty\n",p->m_name );
			#endif

			p->Lock();
			p->m_bThreadIdle = TRUE;
			p->UnLock();
			WaitForSingleObject(p->m_event, INFINITE);
			p->m_bThreadIdle = FALSE;
			send_size = 0;
			time = GetTickCount();			//�]���J�n����
//			TRACE0("CKeepRate : Event Occur\n");
		}
	}
	p->m_bThreadIdle = TRUE;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CKeepRate�N���X
// �ő�]�����x�̗}���A�X�g���[����FIFO�����A�x���o�b�t�@����

CKeepRate::CKeepRate(DWORD PacketSize,DWORD Margin,DWORD BufferMax,DWORD max_bps,BOOL copydata,LPCTSTR name,CDecoderHandler* pDecoderHandler)
		 : CMediaDecoder(pDecoderHandler)

{
	m_Bps = max_bps / 8;		// Bps�ɕϊ�
	
	m_term = 0;
	m_complete = FALSE;
	m_bThreadIdle = FALSE;
	m_PacketSize = PacketSize;
	m_copydata = copydata;
	m_name = name;
	#ifdef _DEBUG
	m_debugmsg = TRUE;
	#endif	
	::InitializeCriticalSection(&m_CriticalSection);
	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
//	m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_proc, this, 0, NULL);
	m_thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall*)(void*))thread_proc, this, 0, NULL);
	if(m_PacketSize){
		m_max = BufferMax / m_PacketSize;
		m_margin = Margin / m_PacketSize;
	} else {
		m_max = 0;
		m_margin = 0;
	}
}

CKeepRate::~CKeepRate(void)
{
	m_term = 1;

	SetEvent(m_event);
	
	WaitForSingleObject(m_thread,10000);

	CloseHandle(m_event);

	Reset();

	::DeleteCriticalSection(&m_CriticalSection);
}


void CKeepRate::Lock(void)
{
	::EnterCriticalSection(&m_CriticalSection);
}

void CKeepRate::UnLock(void)
{
	::LeaveCriticalSection(&m_CriticalSection);
}

const DWORD CKeepRate::GetInputNum(void) const
{
	return 1;
}

const DWORD CKeepRate::GetOutputNum(void) const
{
	return 1;
}

const bool CKeepRate::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(pMediaData->GetData() == NULL) return true;
	
	if(m_copydata){
		CMediaData* m;
#if 1
		DWORD len = pMediaData->GetSize();
		BYTE* buf = pMediaData->GetData();
		while(len){
			if(len >= m_PacketSize){
				m = new CMediaData(buf,m_PacketSize);
				Lock();
				m_q.push_front( m );				// MediaData��que�ɃR�s�[���ďI��
				UnLock();
				buf += m_PacketSize;
				len -= m_PacketSize;
			} else {
				m = new CMediaData(buf,len);
				Lock();
				m_q.push_front( m );				// MediaData��que�ɃR�s�[���ďI��
				UnLock();
				len = 0;
			}
		}
#else
		m = new CMediaData(pMediaData->GetData(),pMediaData->GetSize());
		m_q.push_front( m );				// MediaData��que�ɃR�s�[���ďI��
#endif
	} else {
		Lock();
		m_q.push_front( pMediaData );				// MediaData��que�ɃR�s�[���ďI��
		UnLock();
	}
RETRY:
	Lock();
	size_t size = m_q.size();
	UnLock();
	if(m_bThreadIdle && size >= m_margin){
//		TRACE0("CKeepRate : SetEvent\n");
		SetEvent(m_event);
	} else if (size > m_max){
//		log_out(L"Warning : %s Buffer FULL\n",m_name ? m_name : "noname" );
//		UnLock();
//		Reset();		// �o�b�t�@�j��
		Sleep(5);
		goto RETRY;
		return true;
	}
	#ifdef _DEBUG
	if(m_debugmsg)
//		TRACE4("CKeepRate %s : Buffer Que : %d %X %d\n",m_name  , size ,pMediaData,GetTickCount());
		__noop;
	#endif

	return true;
}

const bool CKeepRate::InputMedia_Func(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	OutputMedia(pMediaData);
	return true;
}

void CKeepRate::Reset(void)
{
	Lock();

	for(size_t i = 0 ; i < m_q.size() ; i ++){
		m_q[i]->Delete();
	}
	m_q.clear();

	UnLock();

	// �����f�R�[�_������������
//	CMediaDecoder::Reset();

}

//�������̃f�[�^�����������Ă���I������B
void CKeepRate::CompleteTrans(void){
	
	m_complete = TRUE ;
	SetEvent(m_event);
}

//���M�X���b�h�̏I�����܂�
DWORD CKeepRate::WaitForThread(DWORD timeout){
	SetEvent(m_event);
	return WaitForSingleObject(m_thread,timeout);
}