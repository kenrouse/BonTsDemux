// BonTsDemuxC.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "BonTsDemuxCUI.h"
#include "BonTsDemuxC.h"
#include "Commandline.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一のアプリケーション オブジェクトです。

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	std::wcout.imbue(std::locale("japanese"));
	std::wcerr.imbue(std::locale("japanese"));

	// MFC を初期化して、エラーの場合は結果を印刷します。
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: 必要に応じてエラー コードを変更してください。
		_tprintf(_T("致命的なエラー: MFC の初期化ができませんでした。\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: アプリケーションの動作を記述するコードをここに挿入してください。
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
