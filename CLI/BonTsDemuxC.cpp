// BonTsDemuxC.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "BonTsDemuxCUI.h"
#include "BonTsDemuxC.h"
#include "Commandline.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// �B��̃A�v���P�[�V���� �I�u�W�F�N�g�ł��B

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	std::wcout.imbue(std::locale("japanese"));
	std::wcerr.imbue(std::locale("japanese"));

	// MFC �����������āA�G���[�̏ꍇ�͌��ʂ�������܂��B
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: �K�v�ɉ����ăG���[ �R�[�h��ύX���Ă��������B
		_tprintf(_T("�v���I�ȃG���[: MFC �̏��������ł��܂���ł����B\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: �A�v���P�[�V�����̓�����L�q����R�[�h�������ɑ}�����Ă��������B
		CCommandLine m_CmdInfo;
	    theApp.ParseCommandLine(m_CmdInfo);

		m_CmdInfo.m_no_gui = TRUE;
		
		if(m_CmdInfo.m_help){

			std::wcout << CMisc::GetVersion(); 
			std::wcout <<  argv[0] ;
			std::wcout << CMisc::GetUsage(); 
			return 0;
		}

		BonTsDemuxCUI cui(&m_CmdInfo);
		nRetCode = cui.StartDemux();
	}

	return nRetCode;
}
