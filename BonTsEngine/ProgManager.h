// ProgManager.h: CProgManager �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "MediaDecoder.h"
#include "TsStream.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// �ԑg���Ǘ��N���X
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CTsPacket	TS�p�P�b�g
// Output	#0	: CTsPacket	TS�p�P�b�g
/////////////////////////////////////////////////////////////////////////////

class CProgManager : public CMediaDecoder
{
public:
	enum EVENTID
	{
		EID_SERVICE_LIST_UPDATED,	// �T�[�r�X���X�g�X�V
		EID_SERVICE_INFO_UPDATED	// �T�[�r�X���X�V
	};
	
	CProgManager(CDecoderHandler *pDecoderHandler);
	virtual ~CProgManager();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CProgManager
	const WORD GetServiceNum(void) const;
	const bool GetServiceID(WORD *pwServiceID, const WORD wIndex = 0U) const;
	const bool GetServiceEsPID(WORD *pwVideoPID, WORD *pwAudioPID, const WORD wIndex = 0U) const;
	const DWORD GetServiceName(LPTSTR lpszDst, const WORD wIndex = 0U) const;

protected:
	class CProgDatabase;

	virtual void OnServiceListUpdated(void);
	virtual void OnServiceInfoUpdated(void);

	struct TAG_SERVICEINFO
	{
		WORD wServiceID;
		WORD wVideoEsPID;
		WORD wAudioEsPID;
		TCHAR szServiceName[256];
	};

	vector<TAG_SERVICEINFO> m_ServiceList;

	CTsPidMapManager m_PidMapManager;
	CProgDatabase *m_pProgDatabase;
};


/////////////////////////////////////////////////////////////////////////////
// �ԑg���f�[�^�x�[�X�N���X
/////////////////////////////////////////////////////////////////////////////

class CProgManager::CProgDatabase
{
public:
	CProgDatabase(CProgManager &ProgManager);
	virtual ~CProgDatabase();

	void Reset(void);
	void UnmapTable(void);
	
	const WORD GetServiceIndexByID(const WORD wServiceID);

	// CProgManager�Ə�񂪃_�u���Ă���̂Ō������ׂ�
	struct TAG_SERVICEINFO
	{
		WORD wServiceID;
		WORD wVideoEsPID;
		WORD wAudioEsPID;
		BYTE byServiceType;
		TCHAR szServiceName[256];

		bool bIsUpdated;

		// ���L�͏��Ƃ��ē��ɕs�v�H
		BYTE byVideoComponentTag;
		BYTE byAudioComponentTag;
		
		WORD wPmtTablePID;			
		BYTE byRunningStatus;
		bool bIsCaService;
	};

	vector<TAG_SERVICEINFO> m_ServiceList;
	WORD m_wTransportStreamID;
	
private:
	static void CALLBACK OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnSdtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);

	CProgManager &m_ProgManager;
	CTsPidMapManager &m_PidMapManager;
};
