#include "stdafx.h"
#include "EcmDat.h"

CEcmDat::CEcmDat()
{
	::InitializeCriticalSection(&m_CriticalSection);

	m_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_exit = FALSE;
	m_start = FALSE;
	m_pbuf = new CRingBuffer(1024*1,"BCAS");
}

CEcmDat::~CEcmDat()
{
	CloseHandle(m_handle);
	delete m_pbuf;

	::DeleteCriticalSection(&m_CriticalSection);
}

void CEcmDat::Lock(void)
{
	::EnterCriticalSection(&m_CriticalSection);
}

void CEcmDat::UnLock(void)
{
	::LeaveCriticalSection(&m_CriticalSection);
}


void CEcmDat::set(BYTE* data,ULONG size)
{
	const BYTE ecm_cmd[4] = {0x90, 0x34, 0x00, 0x00};
	ULONG i,j;
	ECM_DAT ecm;

	if(size==0) return;

	m_pbuf->set(data,size);
	size = m_pbuf->size();
	if(size < 8) return;

	BYTE* m = new BYTE[size];
	m_pbuf->get(m,size);

	for(i = 3; i < size - 4; i++)	// ECMをサーチ
		if(memcmp(m + i, ecm_cmd, 4) == 0) break;

	if(i == size - 4){			// ECMが見つからなかった場合
		m_pbuf->flash(i-3);		// 見つからなかったところまでポインタ移動
		goto END;
	}

	for(i = i - 3; i+m[i+2]+3 < size ;){
		if(memcmp(m+i+3, ecm_cmd, 4) == 0){
			{		// ECM 完結チェック
				ULONG chk;
				chk = i;
				if((ULONG)m[chk+2]+4+2 >= (ULONG)size) break;
				chk += (ULONG)m[chk+2]+4;
				if((ULONG)chk+2 >= size) break;
				if((ULONG)m[chk+2]+4+chk > (ULONG)size) break;
			}

			ecm.magic = 0;

//			TRACE0("Store ecm: ");
//			DebugDump((BYTE*)&m[i+8],m[i+7]+1);
			for(j = 0; j < m[i+7]; j++)
				ecm.magic ^= m[i+8+j] << (4*(j%5));

			i += m[i+2]+4;
			memcpy(ecm.key,m+i+3,25);

//			TRACE0("Store key: ");
//			DebugDump(ecm.key,25);
			ecm.time = GetTickCount();

			Lock();
			m_q.push_front( ecm );
			UnLock();

			SetEvent(m_handle);

		}
		i += m[i+2]+4;
		if(i+2 >= size) break;
	}

	m_pbuf->flash(i);		// 解析したところまでポインタ移動

END:
	delete [] m;
}

ECM_DAT CEcmDat::get(ULONG magic,BOOL &r)
{
	ECM_DAT ret;

	int i,size;
	int retry = 0;

RETRY:
	ZeroMemory((void*)&ret,sizeof(ret));

	Lock();

	size = (int)m_q.size();

	for(i = 0; i < size ; i ++){
		if(m_q[i].magic == magic){
//			TRACE1("Hit Ptr:%d\n",i);
			ret = m_q[i];
			break;
		}
	}

	if(i == size){
		ResetEvent(m_handle);

		UnLock();
		
		WaitForSingleObject(m_handle, /*INFINITE*/300);		// -300msでタイムアウトとする
		if(retry++ >= 1){
			if (m_start && !m_exit){
//				LOG_OUT("Warning : ECM Lost\n");
//				TRACE0("Warning : ECM Lost\n");
				m_start = FALSE;
			}
			return ret;			// 2回以上やっても駄目なら、あきらめてしまう。
		}
		if(	m_exit == TRUE) return ret;
		goto RETRY;
	} else {
		UnLock();
		m_start = TRUE;
	}

	Lock();

	if(m_q.size() > 50 ){
		m_q.pop_back();
	}

	UnLock();
	r = TRUE;

	return ret;
	
}

void CEcmDat::GetKsKey(BYTE* cmd,ULONG len,BYTE* ret)
{
	ULONG magic=0;
	ECM_DAT ecm;

	int j;

//	TRACE0("	Get ecm: ");
//	DebugDump((BYTE*)cmd,len);

	for(j = 0; j < (int)len-5 ; j++){
		magic ^= cmd[j+5] << (4*(j%5));
	}

	BOOL r;
	ULONG tim = GetTickCount();

	ecm = get(magic,r);
	if(r == FALSE){
		ZeroMemory(ret,25);
		return;
	}

//	TRACE0("		Hit ecm: ");
//	DebugDump(ecm.key,25);
	memcpy(ret,ecm.key,25);

	if(m_diagmode){
		if(GetTickCount() > ecm.time){
			if(ecm.time){
//				LOG_OUT("ECM Margin : %d ms\n",GetTickCount() - ecm.time);
			}
		} else {
//			LOG_OUT("ECM Margin : -%d ms\n",ecm.time - tim);
		}

	}
	
	return;
}

void CEcmDat::exit(void)
{
	m_exit = TRUE;
	SetEvent(m_handle);
}
