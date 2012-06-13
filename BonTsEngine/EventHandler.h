#pragma once

#include "TsPacketParser.h"
#include "TsDescrambler.h"
#include "FileWriter.h"
#include "tsserviceselect.h"
#include "NetworkSend.h"
#include "EcmDat.h"
#include "KeepRate.h"
#include "ProgManager.h"
#include "TsDemuxer.h"
#include "httpsend.h"
#include "WaveWriter.h"
#include "AacConverter.h"

#define OPT_SEL			0x00000001
#define OPT_B25			0x00000002
#define OPT_UDP			0x00000004
#define OPT_HTTP		0x00000008
#define OPT_DEMUX		0x00000010
#define OPT_FILEW		0x00000020
#define OPT_BDIAG		0x00000040
#define OPT_FFMPEG		0x00000080

class CEventHandler : public CDecoderHandler
{
public:

	CEventHandler(void);
	~CEventHandler(void);

	// IMediaDecoder から派生したメディアデコーダクラス

	BOOL Init(DWORD opt,LPCTSTR tsfname,DWORD PacketSize,DWORD buf_margin,DWORD buf_size,DWORD service);
	void Exit(void);

	BOOL OnBcasData(BYTE* data,DWORD len);
	BOOL OnTsData(BYTE* data,DWORD len);
	void Reset(void);
	BOOL CheckServerConnection(void);

	void SetServerPort(WORD port) { m_server_port = port; }

protected:
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);

private:
	CTsPacketParser* m_pTsPacketParser;
	CTsDescrambler* m_pTsDescrambler;
	CTsServiceSelect* m_pTsServiceSelect;
	CFileWriter* m_pFileWriter;
	CNetworkSend* m_pNetworkSend;
	CKeepRate* m_pKeepRate;
	CProgManager* m_pProgManager;
	CTsDemuxer* m_pTsDemuxer;
	CFileWriter* m_pVideoFile;
	CFileWriter* m_pAudioFile;
	CKeepRate* m_pFFmpgVideoBand;
	CHttpSend* m_pFFmpgVideo;
	CKeepRate* m_pFFmpgAudioBand;
	CHttpSend* m_pFFmpgAudio;
	CWaveWriter* m_pWaveWriter;
	CAacConverter* m_pAacConverter;

	CHttpSend* m_pHttpSend;

	char* m_tsfname;
	double m_AudioLpfBuf;
	double m_VideoLpfBuf;

	BOOL m_emergency;
	WORD m_server_port;

};
