#include "StdAfx.h"
#include "BonTsDemuxDlgParm.h"

CBonTsDemuxDlgParm::CBonTsDemuxDlgParm(void)
	:m_ffmpeg_param(""),
	m_sound_method(0),
	m_audio_es(0),
	m_rf64(FALSE),
	m_vfhokan(FALSE),
	m_audio_delay(0),
	m_background_mode(FALSE),
	m_no_descramble(FALSE),
	m_output_dir(""),
	m_tspath_dir("")
{
	
	m_pWinApp = AfxGetApp();
	//INIƒtƒ@ƒCƒ‹–¼¶¬
	wchar_t szModulePath[MAX_PATH],drv[_MAX_DRIVE],path[_MAX_DIR],ext[_MAX_EXT],appname[_MAX_FNAME],execname[_MAX_PATH];
	GetModuleFileName(NULL,szModulePath,MAX_PATH);
	_wsplitpath(szModulePath,drv,path,appname,ext);
	_wmakepath(execname,drv,path,appname,L"ini");
	free((void*)m_pWinApp->m_pszProfileName);
	m_pWinApp->m_pszProfileName = _tcsdup(execname);

	//free((void*)m_pWinApp->m_pszRegistryKey );
	//m_pWinApp->m_pszRegistryKey = NULL;
}

CBonTsDemuxDlgParm::~CBonTsDemuxDlgParm(void)
{
}

void CBonTsDemuxDlgParm::Load(void)
{
	m_ffmpeg_param		= m_pWinApp->GetProfileString(L"DlgParm",L"ffmpeg_param"	 ,L""    );
	m_sound_method		= m_pWinApp->GetProfileInt   (L"DlgParm",L"sound_method"	 ,0     );
	m_audio_es			= m_pWinApp->GetProfileInt   (L"DlgParm",L"audio_es"		 ,0     );
	m_rf64				= m_pWinApp->GetProfileInt   (L"DlgParm",L"rf64"			 ,FALSE );
	m_vfhokan			= m_pWinApp->GetProfileInt   (L"DlgParm",L"vfhokan"			 ,FALSE );
	m_audio_delay		= m_pWinApp->GetProfileInt   (L"DlgParm",L"audio_delay"		 ,0     );
	m_background_mode	= m_pWinApp->GetProfileInt   (L"DlgParm",L"background_mode"	 ,FALSE );
	m_no_descramble		= m_pWinApp->GetProfileInt   (L"DlgParm",L"no_descramble"	 ,FALSE );
	m_output_dir		= m_pWinApp->GetProfileString(L"DlgParm",L"output_dir"		 ,L"");
	m_tspath_dir		= m_pWinApp->GetProfileString(L"DlgParm",L"tspath_dir"		 ,L"");

}
void CBonTsDemuxDlgParm::Save(void)
{
	m_pWinApp->WriteProfileString(L"DlgParm",L"ffmpeg_param",m_ffmpeg_param);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"sound_method",m_sound_method);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"audio_es",m_audio_es);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"rf64",m_rf64);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"vfhokan",m_vfhokan);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"audio_delay",m_audio_delay);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"background_mode",m_background_mode);
	m_pWinApp->WriteProfileInt   (L"DlgParm",L"no_descramble",m_no_descramble);
	m_pWinApp->WriteProfileString(L"DlgParm",L"output_dir",m_output_dir);
	m_pWinApp->WriteProfileString(L"DlgParm",L"tspath_dir",m_tspath_dir);
}
