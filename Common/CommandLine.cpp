
#include "stdafx.h"
#include "CommandLine.h"


CCommandLine::CCommandLine():
	m_help				(FALSE),
	m_bffmpeg_param	    (FALSE),
	m_bsound_method	    (FALSE),
	m_bservice			(FALSE),
	m_baudio_es		    (FALSE),
	m_brf64			    (FALSE),
	m_bvfhokan			(FALSE),
	m_baudio_delay	 	(FALSE),
	m_bbackground_mode  (FALSE),
	m_bno_descramble    (FALSE)
{
    m_next = 0;
    m_sound_method = 0;
    m_service = 0;
    m_audio_es = 0;
	m_start = FALSE;
	m_quit = FALSE;
	m_disable_rename = FALSE;
	m_rf64 = FALSE;
	m_vfhokan = FALSE;
	m_input_file = "";
	m_output_file = "";
	m_ffmpeg_param = "";
	m_audio_delay = 0;
	m_no_gui = FALSE;
//++ 2010.03.04 added by pika
	m_background_mode = FALSE;
//--
}

void CCommandLine::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
    CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
    if( bFlag ) {
        CString strOption = pszParam;
        if( strOption == _T("i") ) {
            m_next = INPUT_FILE;
        } else if( strOption == _T("o") ) {
            m_next = OUTPUT_FILE;
        } else if( strOption == _T("srv") ) {
            m_next = SERVICE_NUM;
        } else if( strOption == _T("es") ) {
            m_next = AUDIO_ES;
        } else if( strOption == _T("encode") ) {
            m_next = FFMPEG_PARAM;
        } else if( strOption == _T("sound") ) {
            m_next = SOUND_METHOD;
        } else if( strOption == _T("start") ) {
            m_start = TRUE;
			//startが指定された場合は、すべてのパラメータが設定されたものとする
			//未設定のものは規定値となる。
			m_bffmpeg_param	= TRUE;
			m_bsound_method	= TRUE;
			m_bservice		= TRUE;
			m_baudio_es		= TRUE;
			m_brf64			= TRUE;
			m_bvfhokan		= TRUE;
			m_baudio_delay	= TRUE;
			m_bbackground_mode= TRUE;
			m_bno_descramble	= TRUE;
        } else if( strOption == _T("quit") ) {
            m_quit = TRUE;
        } else if( strOption == _T("dn") ) {
            m_disable_rename = TRUE;
        } else if( strOption == _T("rf64") ) {
            m_rf64 = TRUE;
			m_brf64 = TRUE;
        } else if( strOption == _T("vf") ) {
            m_vfhokan = TRUE;
            m_bvfhokan = TRUE;
        } else if( strOption == _T("nd") ) {
            m_no_descramble = TRUE;
            m_bno_descramble = TRUE;
        } else if( strOption == _T("delay") ) {
            m_next = AUDIO_DELAY;
		} else if( strOption == _T("nogui") ) {
			m_no_gui = TRUE;
//++ 2010.03.04 added by pika
		} else if( strOption == _T("bg") || strOption == _T("background") ) {
			m_background_mode = TRUE;
			m_bbackground_mode = TRUE;
//--
		} else if ( strOption == _T("help") || strOption == _T("h") || strOption == _T("?")) {
			m_help = TRUE;
		} else {
			if ((m_next == AUDIO_DELAY) && _istdigit(pszParam[0])) {	// 負の音声遅延補正値が指定された場合
				m_audio_delay = -_ttoi(pszParam);
				m_next = 0;
			}
		}
	} else {
		if(m_next == INPUT_FILE){
			m_input_file = pszParam;
		} else if(m_next == OUTPUT_FILE){
			m_output_file = pszParam;
		} else if(m_next == FFMPEG_PARAM){
			m_ffmpeg_param = pszParam;
			m_bffmpeg_param = TRUE;
		} else if(m_next == SOUND_METHOD){
			m_sound_method = (DWORD)_ttoi(pszParam);
			m_bsound_method = TRUE;
		} else if(m_next == SERVICE_NUM){
			m_service = (DWORD)_ttoi(pszParam);
		} else if(m_next == AUDIO_ES){
			m_audio_es = (DWORD)_ttoi(pszParam);
			m_baudio_es = TRUE;
		} else if(m_next == AUDIO_DELAY){
			m_audio_delay = _ttoi(pszParam);
			m_baudio_delay = TRUE;
		}
		m_next = 0;
	}
}

