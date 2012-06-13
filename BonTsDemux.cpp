// BonTsDemux.cpp : アプリケーションのクラス動作を定義します。
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


// CBonTsDemuxApp コンストラクション

CBonTsDemuxApp::CBonTsDemuxApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CBonTsDemuxApp オブジェクトです。

CBonTsDemuxApp theApp;


// CBonTsDemuxApp 初期化

BOOL CBonTsDemuxApp::InitInstance()
{
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	::CoInitialize(NULL);

	CWinApp::InitInstance();

    ParseCommandLine(m_CmdInfo);

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	//SetRegistryKey(_T("BonTsDemux"));
	
	if (m_CmdInfo.m_help){
		//ヘルプメッセージの表示
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
			// TODO: ダイアログが <OK> で消された時のコードを
			//  記述してください。
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: ダイアログが <キャンセル> で消された時のコードを
			//  記述してください。
		}
	} else 
	{
		BonTsDemuxCUI cui(&m_CmdInfo);
		cui.StartDemux();
	}
	::CoUninitialize();

	// ダイアログは閉じられました。アプリケーションのメッセージ ポンプを開始しないで
	//  アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}
