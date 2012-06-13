// BonTsDemux.cpp : �A�v���P�[�V�����̃N���X������`���܂��B
//

#include "stdafx.h"
#include "BonTsDemux.h"
#include "BonTsDemuxDlg.h"
#include "BonTsDemuxCUI.h"
#include "Commandline.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBonTsDemuxApp

BEGIN_MESSAGE_MAP(CBonTsDemuxApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBonTsDemuxApp �R���X�g���N�V����

CBonTsDemuxApp::CBonTsDemuxApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
}


// �B��� CBonTsDemuxApp �I�u�W�F�N�g�ł��B

CBonTsDemuxApp theApp;


// CBonTsDemuxApp ������

BOOL CBonTsDemuxApp::InitInstance()
{
	// �A�v���P�[�V���� �}�j�t�F�X�g�� visual �X�^�C����L���ɂ��邽�߂ɁA
	// ComCtl32.dll Version 6 �ȍ~�̎g�p���w�肷��ꍇ�́A
	// Windows XP �� InitCommonControlsEx() ���K�v�ł��B�����Ȃ���΁A�E�B���h�E�쐬�͂��ׂĎ��s���܂��B
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// �A�v���P�[�V�����Ŏg�p���邷�ׂẴR���� �R���g���[�� �N���X���܂߂�ɂ́A
	// �����ݒ肵�܂��B
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	::CoInitialize(NULL);

	CWinApp::InitInstance();

    ParseCommandLine(m_CmdInfo);

	// �W��������
	// �����̋@�\���g�킸�ɍŏI�I�Ȏ��s�\�t�@�C����
	// �T�C�Y���k���������ꍇ�́A�ȉ�����s�v�ȏ�����
	// ���[�`�����폜���Ă��������B
	// �ݒ肪�i�[����Ă��郌�W�X�g�� �L�[��ύX���܂��B
	// TODO: ��Ж��܂��͑g�D���Ȃǂ̓K�؂ȕ������
	// ���̕������ύX���Ă��������B
	//SetRegistryKey(_T("BonTsDemux"));
	
	if (m_CmdInfo.m_help){
		//�w���v���b�Z�[�W�̕\��
		CString helpmsg;
		helpmsg = CMisc::GetVersion();
		helpmsg.Append(TEXT("BonTsDemux.exe"));
		helpmsg.Append(CMisc::GetUsage());

		AfxMessageBox(helpmsg,MB_ICONINFORMATION );
	} else if (!m_CmdInfo.m_no_gui) { 
		CBonTsDemuxDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: �_�C�A���O�� <OK> �ŏ����ꂽ���̃R�[�h��
			//  �L�q���Ă��������B
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: �_�C�A���O�� <�L�����Z��> �ŏ����ꂽ���̃R�[�h��
			//  �L�q���Ă��������B
		}
	} else 
	{
		BonTsDemuxCUI cui(&m_CmdInfo);
		cui.StartDemux();
	}
	::CoUninitialize();

	// �_�C�A���O�͕����܂����B�A�v���P�[�V�����̃��b�Z�[�W �|���v���J�n���Ȃ���
	//  �A�v���P�[�V�������I�����邽�߂� FALSE ��Ԃ��Ă��������B
	return FALSE;
}
