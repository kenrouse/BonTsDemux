// TsDescrambler.h: CTsDescrambler クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TSSERVICESELECT_H_)
#define _TSSERVICESELECT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "MediaDecoder.h"
#include "TsStream.h"
#include "TsTable.h"

using std::vector;


/////////////////////////////////////////////////////////////////////////////
// 指定サービスで使用するPID+EITやTOT等を取り出す
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CTsPacket		平分TSパケット
// Output	#0	: CTsPacket		平分TSパケット
/////////////////////////////////////////////////////////////////////////////

class CTsServiceSelect : public CMediaDecoder  
{
public:
	CTsServiceSelect();
	~CTsServiceSelect();

// IMediaDecoder
	virtual void Reset(void);

	virtual void SetServiceId(int ServiceId);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0);

	const DWORD GetInputPacketCount(void) const;
	const DWORD GetOutputPacketCount(void) const;
	DWORD GetAudioEs(DWORD service,WORD* video,WORD* audio);
//++ 2010.03.04 added by pika
	DWORD GetAudioEs2(int serviceID, DWORD service,WORD* video,WORD* audio);
//--

protected:
	const bool ProcPatPacket(const CTsPacket *pTsPacket);
	const bool ProcPmtPacket(const CTsPacket *pTsPacket);
	const bool ProcEcmPacket(const CTsPacket *pTsPacket);

	void OnPmtUpdate(const CPmtTable &PmtTable);
	bool IsEpgData(WORD wPID);
	DWORD crc32(BYTE *head, BYTE *tail);

	struct TAG_PMTSET
	{
		WORD wPmtPID;
		CPmtTable PmtTable;
	};
	struct TAG_ECMSET
	{
		WORD wEcmPID;
	};

	CPatTable m_PatTable;
	vector<TAG_PMTSET> m_PmtList;
	vector<TAG_ECMSET> m_EcmList;

	CPatTable m_lastPatTable;
	vector<TAG_PMTSET> m_lastPmtList;
	time_t m_lastPatTime;

	DWORD m_dwInputPacketCount;
	DWORD m_dwOutputPacketCount;

	WORD m_wEcmSetIndex;

	int m_ServiceId;
	WORD m_wPmtPID;
};

#endif // !defined(_TSDESCRAMBLER_H_)
