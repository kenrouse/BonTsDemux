#pragma once


class CCommandLine : public CCommandLineInfo
{

public:
    CCommandLine();

	enum CmdLineNextField {PARAM_NONE = 0,INPUT_FILE,OUTPUT_FILE,SERVICE_NUM,AUDIO_ES,FFMPEG_PARAM,SOUND_METHOD,AUDIO_DELAY};

	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
	DWORD m_next;
	DWORD m_sound_method;
	DWORD m_service;
	DWORD m_audio_es;
	CString m_input_file;
	CString m_output_file;
	CString m_ffmpeg_param;
	BOOL m_start;
	BOOL m_quit;
	BOOL m_disable_rename;
	BOOL m_rf64;
	BOOL m_vfhokan;
	int m_audio_delay;
	BOOL m_no_descramble;
	BOOL m_no_gui;
//++ 2010.03.04 added by pika
	BOOL m_background_mode;
//--
	BOOL m_help;

	//2010.05.20 パラメータが設定されたかどうかのフラグ by fuji
	BOOL m_bffmpeg_param		;
	BOOL m_bsound_method		;
	BOOL m_bservice			    ;
	BOOL m_baudio_es			;
	BOOL m_brf64				;
	BOOL m_bvfhokan			    ;
	BOOL m_baudio_delay		    ;
	BOOL m_bbackground_mode	    ;
	BOOL m_bno_descramble		;
};

class CBonTsDemuxApp : public CWinApp
{
public:
	CBonTsDemuxApp();
	CCommandLine m_CmdInfo;

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};
