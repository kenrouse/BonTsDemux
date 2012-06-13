/*
	CBonTsDemuxDlgParm 
	Dlg‚Ì‰Šúİ’è’l‚ğŠÇ—‚·‚éƒNƒ‰ƒX
	2010/05/19 by fuji
*/

#pragma once

class CBonTsDemuxDlgParm
{
public:
	CBonTsDemuxDlgParm(void);
	~CBonTsDemuxDlgParm(void);

	CString m_ffmpeg_param;
    DWORD	m_sound_method;
    DWORD	m_audio_es;
	BOOL	m_rf64;
	BOOL	m_vfhokan;
	int		m_audio_delay;
	BOOL	m_background_mode ;
	BOOL	m_no_descramble;

	CString m_output_dir;
	CString m_tspath_dir;
	void Save(void);
	void Load(void);

private:

	CWinApp* m_pWinApp;
};

