#pragma once
#include "TsConverter.h"
#include "CommandLine.h"

class BonTsDemuxCUI :
	protected CTsConverter::IEventHandler
{
public:
	BonTsDemuxCUI(CCommandLine *cmdLine);
	int StartDemux(void);
	~BonTsDemuxCUI(void);
protected:
	// CTsConverter::IEventHandler
	virtual void OnTsConverterStart(const ULONGLONG llFileSize);
	virtual void OnTsConverterEnd(const ULONGLONG llFileSize);
	virtual void OnTsConverterProgress(const ULONGLONG llCurPos, const ULONGLONG llFileSize);
	virtual void OnTsConverterServiceName(LPCTSTR lpszServiceName);
	virtual void OnTsConverterServiceInfo(CProgManager *pProgManager);
private:
	CRITICAL_SECTION m_mutexTest;
	CCommandLine* m_pCmdLine;
	CTsConverter m_TsConverter;
	WORD m_wLastProgress;
	bool m_bError;
};
