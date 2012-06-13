// Lock(CriticalSection)

#pragma once

// Critical Section Lapper
typedef CRITICAL_SECTION LOCK_OBJ;
class CLock
{
public:
	CLock(LOCK_OBJ *lpLockObj=NULL)
	{
		m_pLockObj=lpLockObj;
		if(m_pLockObj) EnterCriticalSection(m_pLockObj);
	}
	~CLock()
	{
		if(m_pLockObj) LeaveCriticalSection(m_pLockObj);
	}
	static void CreateLockObj(LOCK_OBJ *lpLock)
	{
		InitializeCriticalSection(lpLock);
	}
	static void DestroyLockObj(LOCK_OBJ *lpLock)
	{
		DeleteCriticalSection(lpLock);
	}

private:
	LOCK_OBJ *m_pLockObj;
};
