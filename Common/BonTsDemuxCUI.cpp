#include "stdafx.h"
#include <iomanip>
//#include "BonTsDemux.h"
#include "BonTsDemuxCUI.h"

BonTsDemuxCUI::BonTsDemuxCUI(CCommandLine *cmdLine)
	: m_TsConverter(this)
{
	m_pCmdLine = cmdLine;

}

void  BonTsDemuxCUI::OnTsConverterStart(const ULONGLONG llFileSize)
{
	// �ϊ��J�n�C�x���g
	// �r��
	EnterCriticalSection(&m_mutexTest);
	
	std::wcout << L"FileSize="
		<< llFileSize
		<< std::endl;
}
void  BonTsDemuxCUI::OnTsConverterEnd(const ULONGLONG llFileSize)
{
	// �ϊ��I���C�x���g
	std::wcout << std::endl;
	m_bError = (llFileSize == 0) ? true:false;
	// �r������
	LeaveCriticalSection(&m_mutexTest);
}
void BonTsDemuxCUI::OnTsConverterProgress(const ULONGLONG llCurPos, const ULONGLONG llFileSize)
{
	// �i���X�V�C�x���g
	WORD wProgress;

	wProgress = (WORD)((DOUBLE)llCurPos / llFileSize * 100);
	if (m_wLastProgress != wProgress) {
		std::wcout << L"Progress:"
			<< std::setw(3) << wProgress << L"%"
			<< L"\r";
	}
}
void BonTsDemuxCUI::OnTsConverterServiceName(LPCTSTR lpszServiceName)
{
	// �T�[�r�X���X�V�C�x���g
}
void BonTsDemuxCUI::OnTsConverterServiceInfo(CProgManager *pProgManager)
{
	// �T�[�r�X���X�g�X�V�C�x���g
	TCHAR szServiceName[1024];
	WORD wServiceID;
	
	memset(szServiceName, 0, sizeof(szServiceName));

	for(WORD wIndex = 0U ; wIndex < pProgManager->GetServiceNum() ; wIndex++){
		if(pProgManager->GetServiceID(&wServiceID, wIndex)){
			if(pProgManager->GetServiceName(szServiceName, wIndex)){
				if (szServiceName != _T("")){
					// �T�[�r�X���\��
					std::wcout << L"ServiceName:" << wServiceID << L" " << szServiceName << std::endl;
				}
			}
		}
	}
}

int BonTsDemuxCUI::StartDemux(void)
{
	// �R���o�[�g�J�n
	wchar_t szModulePath[MAX_PATH],drv[_MAX_DRIVE],path[_MAX_DIR],ext[_MAX_EXT],appname[_MAX_FNAME],execname[_MAX_PATH],buff[1024];
	BOOL bTestMode, bAacDecode;
	WORD wSelectServiceID;
	CString param, ini_path, csAudioPath, csVideoPath;
	buff[0] = L'\0';
	ext[0] = L'\0';
	param = m_pCmdLine->m_ffmpeg_param;
	int nRetCode = 0;

	GetModuleFileName(NULL,szModulePath,MAX_PATH);
	_wsplitpath(szModulePath,drv,path,appname,ext);
	_wmakepath(execname,drv,path,L"cap_sts_sea",L"ini");
	ini_path = execname;

	if(param != _T("")){
		GetPrivateProfileString(L"FFMPEG_SETTING",param,L"",buff,sizeof(buff),ini_path);
		param = param + L"_EXT";
		GetPrivateProfileString(L"FFMPEG_SETTING",param,L"",ext,sizeof(ext),ini_path);
	}

	bTestMode = FALSE;
	bAacDecode = TRUE;

	csVideoPath = m_pCmdLine->m_output_file;

	if(m_pCmdLine->m_input_file != L"" && csVideoPath== L""){
		csVideoPath = m_pCmdLine->m_input_file;
		csVideoPath.Delete(csVideoPath.ReverseFind('.'),csVideoPath.GetLength());	// �g���q������
	}

	if(buff[0] == L'\0'){

		if(wcsstr(param, L"Demux(m2v+aac)") || wcsstr(param, L"Demux(aac)")){
			bAacDecode = FALSE;
			csAudioPath = csVideoPath + L".aac";
		} else {
			csAudioPath = csVideoPath + L".wav";
		}
		csVideoPath += L".m2v";
	} else if (m_pCmdLine->m_disable_rename == FALSE){
		if(ext[0] == L'\0'){
			if(wcsstr(buff, L"-vcodec copy") || wcsstr(buff, L"-vcodec mpeg") || wcsstr(buff, L"-target ntsc-dvd") ){
				csVideoPath += L".mpg";
			} else if(wcsstr(buff, L"-f mp4") || wcsstr(buff, L"-f psp") || wcsstr(buff, L"-f ipod")){
				csVideoPath += L".mp4";
			} else {
				csVideoPath += L".avi";
			}
		} else {
			csVideoPath += L".";
			csVideoPath += ext;
		}
	}
	if(wcsstr(param, L"Demux(wav)") || wcsstr(param, L"Demux(aac)")){
		csVideoPath = L"";
	} else if(wcsstr(param, L"Demux(m2v)")){
		csAudioPath = L"";
	}

	// �R���o�[�g�J�n
	wSelectServiceID = NO_SERVICE_SELECT;

	if(m_pCmdLine->m_service!=0)
	{
		wSelectServiceID = (WORD)m_pCmdLine->m_service;
	}

	InitializeCriticalSection(&m_mutexTest);

	const WORD wReturn = m_TsConverter.ConvertTsFile(m_pCmdLine->m_input_file,READ_TO_FILEEND,wSelectServiceID,
													(LPCTSTR)csVideoPath,
													(LPCTSTR)csAudioPath,
													(bAacDecode)? true : false,
													true,//(m_bLipSync)? true : false,
													buff,
													m_pCmdLine->m_sound_method,
													m_pCmdLine->m_audio_es,
													m_pCmdLine->m_rf64,
													m_pCmdLine->m_vfhokan,
													m_pCmdLine->m_audio_delay,
													m_pCmdLine->m_no_descramble
													);

	if(wReturn == ERR_FILE_CANT_OPEN){
		std::wcout << L"Error:�t�@�C���̃I�[�v���Ɏ��s���܂����B" << std::endl;
		nRetCode = 1;
	} else if(wReturn == ERR_FFMPEG_NOT_FOUND){
		std::wcout << L"Error:FFMpeg���J�n�ł��܂���B" << std::endl;
		nRetCode = 1;
	} else if(wReturn == ERR_CANT_START){
		std::wcout << L"Error:�f�R�[�h���J�n�ł��܂���B" << std::endl;
		nRetCode = 1;
	} else {
		// ����
		Sleep(100);
		// �R���o�[�g�����҂�
		EnterCriticalSection(&m_mutexTest);
	}

	DeleteCriticalSection(&m_mutexTest);
	if (nRetCode == 0 && m_bError){
		std::wcout << L"Error:FFmpeg�ŃG���[���������܂����B" << std::endl;
		nRetCode = 1;
	}
	return nRetCode;
}

BonTsDemuxCUI::~BonTsDemuxCUI(void)
{
}
