#pragma once


#define NO_SERVICE_SELECT	0xFFFF

#define ERR_FILE_CANT_OPEN		1
#define ERR_FFMPEG_NOT_FOUND	2
#define ERR_CANT_START			3

#include "MediaDecoder.h"

#include "FileReader.h"
#include "TsPacketParser.h"
#include "TsDescrambler.h"
#include "ProgManager.h"
#include "TsDemuxer.h"
#include "FileWriter.h"

#include "AacConverter.h"
#include "WaveWriter.h"
#include "qE77TinkWavWriter.h"
using namespace qE77Tink;

#include "HttpSend.h"
#include "KeepRate.h"

#include "TsServiceSelect.h"

class CTsConverter : public CDecoderHandler
{
public:
	// イベントハンドラインタフェース
	class IEventHandler
	{
	public:
		virtual void OnTsConverterStart(const ULONGLONG llFileSize) = 0;
		virtual void OnTsConverterEnd(const ULONGLONG llFileSize) = 0;
		virtual void OnTsConverterProgress(const ULONGLONG llCurPos, const ULONGLONG llFileSize) = 0;
		virtual void OnTsConverterServiceName(LPCTSTR lpszServiceName) = 0;
		virtual void OnTsConverterServiceInfo(CProgManager *pProgManager) = 0;
	};

	CTsConverter(IEventHandler *pEventHandler);
	~CTsConverter(void);

	const WORD ConvertTsFile(LPCTSTR lpszTsFile, QWORD qwInputSizeLimit , WORD wServiceSelect, LPCTSTR lpszVideoFile, LPCTSTR lpszAudioFile, const bool bDecodeAac = false, const bool bLipSync = false,LPCTSTR ffmpeg_param=NULL,DWORD SoundMethod=0,DWORD AudioEs=0,BOOL bRf64=FALSE,BOOL vhokan=FALSE,int audio_delay=0,BOOL NoDescramble=false);
	void CancelConvert(void);
	int CTsConverter::FindAvailablePort(void);

protected:
	void CloseDecoders(void);
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);

	// IMediaDecoder から派生したメディアデコーダクラス
	CFileReader m_FileReader;			// ファイルリーダ
	CTsPacketParser m_TsPacketParser;	// TSパケッタイザ
	CTsServiceSelect m_TsServiceSelect;
	CTsDescrambler m_TsDescrambler;		// TSデスクランブラ
	CProgManager m_ProgManager;			// TSプログラムマネージャ
	CTsDemuxer* m_pTsDemuxer;				// TSデマクサ
	CFileWriter m_VideoFile;			// ビデオファイル
	CFileWriter m_AudioFile;			// ビデオファイル
	CAacConverter m_AacConverter;		// AACデコーダ
	CWaveWriter m_WaveWriter;			// Waveライタ
	WavWriter m_WavWriter;
	WavSplitWriter m_WavSplitWriter;

	CKeepRate *m_pVideoBand;
	CKeepRate *m_pAudioBand;
	
	CHttpSend *m_pWaveSend;
	CHttpSend *m_pVideoSend;

	IEventHandler *m_pEventHandler;
	LONG m_AudHoseiFromVid;

	BOOL m_AudioEsNum;
	DWORD m_achannel;
	DWORD m_ServiceSelect;
	BOOL m_Useffmpg;
	BOOL m_vframe_hokan;
	BOOL m_DecodeAac;
	int m_audio_delay;

	CString* m_pAudioPath;

	DWORD m_dwFfmpegExitCode;
};
