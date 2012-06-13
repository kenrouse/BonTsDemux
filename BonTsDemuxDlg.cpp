// BonTsDemuxDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BonTsDemux.h"
#include "BonTsDemuxDlg.h"
//#include ".\bontsdemuxdlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_DOBATCH    WM_USER+100

#ifndef _ERRORCODE_DEFINED
typedef int errno_t;
#endif

namespace
{
	bool IsVistaOrHigher()
	{
		OSVERSIONINFO vi = {sizeof vi};
		::GetVersionEx(&vi);
		return vi.dwMajorVersion >= 6;
	}
}

// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CBonTsDemuxDlg ダイアログ

CBonTsDemuxDlg::CBonTsDemuxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBonTsDemuxDlg::IDD, pParent)
	, m_TsConverter(this)
	, m_csTsPath(_T(""))
	, m_csVideoPath(_T(""))
	, m_csAudioPath(_T(""))
	, m_csFolderPath(_T(""))
	, m_bAacDecode(FALSE)
	, m_bLipSync(FALSE)
	, m_check_rf64(FALSE)
	, m_vframe_hokan(FALSE)
	, m_audio_delay(0)
	, m_Descramble(TRUE)
	, m_bBackgroundMode(FALSE)
	, m_DlgParm()
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bTestMode = FALSE;
	m_wSelectServiceID = FALSE;

	//version check 
	DWORDLONG dwlConditionMask = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	OSVERSIONINFOEX osvi;
	::ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
	osvi.dwMajorVersion = 6; // 6.1 ... Windows 7, 2008R2
	osvi.dwMinorVersion = 1;
	m_bWindows7 = ::VerifyVersionInfo(&osvi,VER_MAJORVERSION|VER_MINORVERSION,dwlConditionMask);
	
}

void CBonTsDemuxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBonTsDemuxDlg)
	DDX_Text(pDX, IDC_TSPATH, m_csTsPath);
	DDX_Text(pDX, IDC_VIDEOPATH, m_csVideoPath);
	DDX_Text(pDX, IDC_FOLDERPATH, m_csFolderPath);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_COMBO1, m_ComboEncode);
	DDX_Control(pDX, IDC_BATCH_LIST, m_ListBatch);
	DDX_Control(pDX, IDC_COMBO_SOUND, m_ComboSound);
	DDX_Control(pDX, IDC_COMBO_AUDIOES, m_ComboAudioEs);
	DDX_Control(pDX, IDC_SVCOMBO, m_ComboService);
	DDX_Check(pDX, IDC_CHECK_RF64, m_check_rf64);
	DDX_Check(pDX, IDC_FRAME_HOKAN, m_vframe_hokan);
	DDX_Check(pDX, IDC_DESCRAMBLE, m_Descramble);
	DDX_Check(pDX, IDC_USE_BACKGROUND_MODE, m_bBackgroundMode);
	DDX_Text(pDX, IDC_AUDIO_DELAY, m_audio_delay);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBonTsDemuxDlg, CDialog)
	//{{AFX_MSG_MAP(CBonTsDemuxDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSETS, CBonTsDemuxDlg::OnBnClickedBrowseTs)
	ON_BN_CLICKED(IDC_BROWSEVIDEO, CBonTsDemuxDlg::OnBnClickedBrowseVideo)
	ON_BN_CLICKED(IDC_START, CBonTsDemuxDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, CBonTsDemuxDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BATCH_ADD, CBonTsDemuxDlg::OnBnClickedBatchAdd)
	ON_BN_CLICKED(IDC_BATCH_DEL, CBonTsDemuxDlg::OnBnClickedBatchDel)
	ON_LBN_SELCHANGE(IDC_BATCH_LIST, CBonTsDemuxDlg::OnLbnSelchangeBatchList)
	ON_MESSAGE(WM_DOBATCH, CBonTsDemuxDlg::OnDoBatch ) // 追加
	ON_BN_CLICKED(IDC_BUTTON_REPLACE, CBonTsDemuxDlg::OnBnClickedButtonReplace)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_SERVICE_REFLESH, CBonTsDemuxDlg::OnBnClickedServiceReflesh)
	ON_BN_CLICKED(IDC_BROWSEFOLDER, OnBnClickedBrowseFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CBonTsDemuxDlg メッセージ ハンドラ

BOOL CBonTsDemuxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

	wchar_t szModulePath[MAX_PATH],drv[_MAX_DRIVE],path[_MAX_DIR],ext[_MAX_EXT],appname[_MAX_FNAME],execname[_MAX_PATH];
//	wchar_t enc_param[256];
//	enc_param[0] = L'\0';
	GetModuleFileName(NULL,szModulePath,MAX_PATH);
	_wsplitpath(szModulePath,drv,path,appname,ext);
	_wmakepath(execname,drv,path,L"cap_sts_sea",L"ini");
	m_ini_path = execname;

	FILE	*fp;
	CString pg;
	wchar_t buffer[WCHAR_MAX];
	errno_t err;
	m_pCmdLine = &((CBonTsDemuxApp*)AfxGetApp())->m_CmdInfo;

	m_ComboEncode.Clear();

	m_ComboEncode.AddString(L"Demux(m2v+wav)");
	m_ComboEncode.AddString(L"Demux(m2v+aac)");

#if _MSC_VER >= 1400
	err = _wfopen_s(&fp, execname , L"rt");	//読み込み専用、テキストモード
#else
	fp = _wfopen(execname , L"rt");			//読み込み専用、テキストモード
	err = (fp == NULL);
#endif

	if(err == 0){

		for(;;){
			if(fgetws(buffer, WCHAR_MAX, fp) == NULL){	//一行読み込み(S-JIS -> Unicode自動変換)
				// 中身がない
				fclose(fp);
				break;
			}
			if(wcsstr(buffer, L"[VLC_SETTING]") == buffer){
				pg = L"";
				continue;
			} else if(wcsstr(buffer, L"[FFMPEG_SETTING]") == buffer){
				pg = L"ffmpeg:";
				continue;
			} else if(wcsstr(buffer, L"[") == buffer){
				pg = "";
			} else if(wcsstr(buffer, L";")){
				continue;
			} else if(wcsstr(buffer, L"PATH")){
				continue;
			} else if(wcsstr(buffer, L"_EXT")){
				continue;
			}

			if(pg != ""){
				wchar_t* arg;
				arg = wcsstr(buffer, L"=");
				if(arg == NULL) continue;
				*arg=L'\0';
				CString buf;
				buf=buffer;
				buf.Trim();
				m_ComboEncode.AddString(buf);
			}
		}
		fclose(fp);
	}
	m_ComboEncode.AddString(L"Demux(m2v)");
	m_ComboEncode.AddString(L"Demux(wav)");
	m_ComboEncode.AddString(L"Demux(aac)");

	m_DlgParm.Load();

	if(m_ComboEncode.SelectString(-1,m_pCmdLine->m_bffmpeg_param ? m_pCmdLine->m_ffmpeg_param: m_DlgParm.m_ffmpeg_param  ) == CB_ERR){
		m_ComboEncode.SetCurSel(0);
	}

	m_ComboSound.AddString(TEXT("Stereo(主+副)"));		// SoundMethod 0
	m_ComboSound.AddString(TEXT("主音声"));				// SoundMethod 1
	m_ComboSound.AddString(TEXT("副音声"));				// SoundMethod 2
	m_ComboSound.AddString(TEXT("強制5.1ch"));			// SoundMethod 3
	m_ComboSound.AddString(TEXT("強制5.1ch(Split)"));	// SoundMethod 4
	m_ComboSound.SetCurSel(m_pCmdLine->m_bsound_method ? m_pCmdLine->m_sound_method : m_DlgParm.m_sound_method);

	m_ComboAudioEs.AddString(TEXT("0"));
	m_ComboAudioEs.AddString(TEXT("1"));
	m_ComboAudioEs.AddString(TEXT("2"));
	m_ComboAudioEs.SetCurSel(m_pCmdLine->m_baudio_es ? m_pCmdLine->m_audio_es : m_DlgParm.m_audio_es);

	if(m_pCmdLine->m_service){
		CComboBox *pServiceCombo = static_cast<CComboBox *>(GetDlgItem(IDC_SVCOMBO));
		CString Str;
		Str.Format(L"%d - noname",m_pCmdLine->m_service);
		pServiceCombo->AddString(Str);
		pServiceCombo->SetItemData(0,m_pCmdLine->m_service);
		pServiceCombo->SetCurSel(0);
	}

	m_csTsPath = m_pCmdLine->m_input_file;
	m_csVideoPath = m_pCmdLine->m_output_file;
	if(m_csTsPath != L"" && m_csVideoPath== L""){
		m_csVideoPath = m_csTsPath;
		m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());	// 拡張子を消す
	}

	m_check_rf64 = m_pCmdLine->m_brf64 ? m_pCmdLine->m_rf64 :m_DlgParm.m_rf64 ;
	m_vframe_hokan = m_pCmdLine->m_bvfhokan ? m_pCmdLine->m_vfhokan : m_DlgParm.m_vfhokan;
	m_audio_delay = m_pCmdLine->m_baudio_delay ? m_pCmdLine->m_audio_delay : m_DlgParm.m_audio_delay;
	m_Descramble = !( m_pCmdLine->m_bno_descramble ? m_pCmdLine->m_no_descramble : m_DlgParm.m_no_descramble);
//++ 2010.03.04 added by pika
	m_bBackgroundMode = m_pCmdLine->m_bbackground_mode  ? m_pCmdLine->m_background_mode :m_DlgParm.m_background_mode;;
//--
	m_csFolderPath = m_DlgParm.m_output_dir;

    // ITaskbarList3 インターフェースのアクティベート
	if ( m_bWindows7) {
		HRESULT hRes = m_ptrTaskbarList3.CoCreateInstance(CLSID_TaskbarList);
		ATLASSERT(SUCCEEDED(hRes));
	}

	UpdateData(FALSE);

	LockControl(FALSE);

	if(m_pCmdLine->m_start){
		// -start 指定の場合も「変換開始」ボタンクリックと同等の処理に
		if(m_csVideoPath != _TEXT("")){
			OnBnClickedBatchAdd();
		}

		m_ListBatch.SetCurSel(0);
		UpdateData(TRUE);
		OnLbnSelchangeBatchList();

		StartDemux();
	}
	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}
/*
	enum CmdLineNextField {INPUT_FILE,OUTPUT_FILE,SERVICE_NUM,AUDIO_ES,FFMPEG_PARAM,SOUND_METHOD};

	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
	DWORD m_next;
	DWORD m_sound_method;
	DWORD m_service;
	DWORD m_audio_es;
	CString m_input_file;
	CString m_output_file;
	CString m_ffmpeg_param;
*/
void CBonTsDemuxDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CBonTsDemuxDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CBonTsDemuxDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBonTsDemuxDlg::OnDestroy()
{

	//画面設定値の保存 
	//自動起動して終了する場合は保存しない
	if ( !m_pCmdLine->m_start && !m_pCmdLine->m_quit) {
		
		UpdateData(TRUE);

		CString param;
		m_ComboEncode.GetLBText( m_ComboEncode.GetCurSel(),param );

		m_DlgParm.m_ffmpeg_param	= param;
		m_DlgParm.m_sound_method	= m_ComboSound.GetCurSel();
		m_DlgParm.m_audio_es		= m_ComboAudioEs.GetCurSel();
		m_DlgParm.m_rf64			= m_check_rf64;
		m_DlgParm.m_vfhokan			= m_vframe_hokan;
		m_DlgParm.m_audio_delay		= m_audio_delay;
		m_DlgParm.m_background_mode	= m_bBackgroundMode;
		m_DlgParm.m_no_descramble	= !m_Descramble;
		m_DlgParm.m_output_dir		= m_csFolderPath;

		m_DlgParm.Save();
	}

	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}

void CBonTsDemuxDlg::OnBnClickedBrowseTs()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	CFileDialog Dlg(TRUE, TEXT("ts"), NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ENABLESIZING, TEXT("TSファイル (*.ts)|*.ts||"));
	TCHAR FileNameBuf[MAX_PATH * 50];
	FileNameBuf[0] = _T('\0');
	Dlg.m_ofn.lpstrFile = FileNameBuf;
	Dlg.m_ofn.nMaxFile = lengthof(FileNameBuf);
	Dlg.m_ofn.lpstrInitialDir = m_DlgParm.m_tspath_dir;

	if(Dlg.DoModal() == IDOK){
		UpdateData(TRUE);

		POSITION pos = Dlg.GetStartPosition();
		CString csTsPath = Dlg.GetNextPathName(pos);

		//2010.05.20 最後に開いたファイルのフォルダを保存する。
		TCHAR *buff = new TCHAR[csTsPath.GetLength()+1];
		_tcscpy(buff,csTsPath.GetBuffer());
		PathRemoveFileSpec(buff);
		m_DlgParm.m_tspath_dir = buff;
		delete buff;

		if (pos == NULL) {
			// 単一ファイル
		//	m_csTsPath = Dlg.GetPathName();
			m_csTsPath = csTsPath;

			m_bTestMode = TRUE;
			const WORD wReturn = m_TsConverter.ConvertTsFile(m_csTsPath, TEST_READ_SIZE, NO_SERVICE_SELECT, NULL, NULL);

			if (m_csFolderPath.IsEmpty()) {
				m_csVideoPath = m_csTsPath;
			} else {
				TCHAR buf[MAX_PATH];
				::PathCombine(buf, m_csFolderPath, ::PathFindFileName(m_csTsPath));
				m_csVideoPath = buf;
			}
			m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());

	//		m_csVideoPath = m_csTsPath;
	//		m_csVideoPath.Replace(TEXT(".ts"), TEXT(".avi"));
			
	//		m_csAudioPath = m_csTsPath;
	//		m_csAudioPath.Replace(TEXT(".ts"), TEXT(".aac"));
		} else {
			// 複数ファイル
			pos = Dlg.GetStartPosition();
			while (pos != NULL) {
				UpdateData(TRUE);

				m_csTsPath = Dlg.GetNextPathName(pos);
				
				if (m_csFolderPath.IsEmpty()) {
					m_csVideoPath = m_csTsPath;
				} else {
					TCHAR buf[MAX_PATH];
					::PathCombine(buf, m_csFolderPath, ::PathFindFileName(m_csTsPath));
					m_csVideoPath = buf;
				}
				if(m_csVideoPath.ReverseFind('.') && m_csVideoPath.ReverseFind('.') > m_csVideoPath.ReverseFind('\\')){
					m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());
				}
				
				m_ComboService.ResetContent();

				UpdateData(FALSE);
				OnBnClickedBatchAdd();
			}
		}
				
		UpdateData(FALSE);
	}
}

void CBonTsDemuxDlg::OnBnClickedBrowseVideo()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);

	m_csVideoPath = m_csTsPath;
	m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());

	CFileDialog Dlg(FALSE, TEXT("*"), m_csVideoPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING, TEXT("ファイル (*.*)|*.*||"));
	if(Dlg.DoModal() != IDOK)return;

	m_csVideoPath = Dlg.GetPathName();
	if(m_csVideoPath.ReverseFind('.') && m_csVideoPath.ReverseFind('.') > m_csVideoPath.ReverseFind('\\')){
		m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());
	}

	UpdateData(FALSE);
}

int CALLBACK CBonTsDemuxDlg::BrowseCallbackProc(HWND hwnd, UINT uMsg,
		LPARAM lParam, LPARAM lpData)
{
	static TCHAR buf[MAX_PATH];
	
	switch (uMsg) {
	case BFFM_INITIALIZED:
		if (((LPCTSTR)lpData)[0] == _T('\0')) {	// m_csFolderPath は空か？
			// カレントディレクトリを選択状態にする。
			::GetCurrentDirectory(lengthof(buf), buf);
			lpData = (LPARAM) buf;
		}
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
	case BFFM_VALIDATEFAILED:
		// Edit Box に入力されたパスを選択状態に。
	//	GetFullPathName((LPCTSTR) lParam, lengthof(buf), buf, NULL); // 手抜き
		::PathCombine(buf, buf, (LPCTSTR) lParam);	// QQQ
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) buf);
		return 1;
	case BFFM_SELCHANGED:
		// BIF_EDITBOX を指定すると BIF_RETURNONLYFSDIRS が効かなくなるので
		// 代わりに自前で処理
		::SendMessage(hwnd, BFFM_ENABLEOK, 0,
				::SHGetPathFromIDList((LPITEMIDLIST) lParam, buf));
		break;
	default:
		break;
	}
	return 0;
}

void CBonTsDemuxDlg::OnBnClickedBrowseFolder()
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);
	
	BROWSEINFO bi = {0};
	bi.hwndOwner = GetSafeHwnd();
//	bi.pidlRoot = 0;
//	bi.pszDisplayName = szDispName;
	bi.lpszTitle = _T("デフォルトの出力フォルダを選択してください。");
//	bi.ulFlags = BIF_EDITBOX | BIF_VALIDATE /* | BIF_RETURNONLYFSDIRS */;
	bi.ulFlags = BIF_USENEWUI | BIF_VALIDATE;
	bi.lpfn = CBonTsDemuxDlg::BrowseCallbackProc;
	bi.lParam = (LPARAM) static_cast<LPCTSTR>(m_csFolderPath);
	
	// フォルダの参照ダイアログボックスの表示
	LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
	if (pidl != NULL) {
		TCHAR szDir[MAX_PATH];
		// PIDL をファイルシステムのパスに変換
		if (::SHGetPathFromIDList(pidl, szDir)) {
			m_csFolderPath = szDir;
		}
		::CoTaskMemFree(pidl);
	}
	
	UpdateData(FALSE);
}

/*
void CBonTsDemuxDlg::OnBnClickedBrowseAudio()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);

	CFileDialog Dlg(FALSE, TEXT("aac"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING, TEXT("aacファイル (*.aac)|*.aac|wavファイル (*.wav)|*.wav||"));
	if(Dlg.DoModal() != IDOK)return;
	
	m_csAudioPath = Dlg.GetPathName();
	m_bAacDecode = (Dlg.GetFileExt() == TEXT("aac"))? false : true;
		
	UpdateData(FALSE);
}

void CBonTsDemuxDlg::OnBnClickedVideoOut()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);
	
	GetDlgItem(IDC_VIDEOPATH)->EnableWindow(m_bVideoOut);
	GetDlgItem(IDC_BROWSEVIDEO)->EnableWindow(m_bVideoOut);
}

void CBonTsDemuxDlg::OnBnClickedAudioOut()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);
	
}

void CBonTsDemuxDlg::OnBnClickedAacDecode()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);
	
	if(m_bAacDecode){
		m_csAudioPath.Replace(TEXT(".aac"), TEXT(".wav"));
		}
	else{
		m_csAudioPath.Replace(TEXT(".wav"), TEXT(".aac"));
		}
		
	UpdateData(FALSE);
}
*/

void CBonTsDemuxDlg::StartDemux(void)
{
	wchar_t buff[1024],ext[256];
	CString param;
	buff[0] = L'\0';
	ext[0] = L'\0';
	if(m_ComboEncode.GetCurSel()>=0){
		m_ComboEncode.GetLBText( m_ComboEncode.GetCurSel(),param );
		GetPrivateProfileString(L"FFMPEG_SETTING",param,L"",buff,sizeof(buff),m_ini_path);
		param = param + L"_EXT";
		GetPrivateProfileString(L"FFMPEG_SETTING",param,L"",ext,sizeof(ext),m_ini_path);
	}

	m_bTestMode = FALSE;
	m_bAacDecode = TRUE;

	if(buff[0] == L'\0'){
//		m_csVideoPath.Replace(TEXT(".avi"), TEXT(""));

		if(wcsstr(param, L"Demux(m2v+aac)") || wcsstr(param, L"Demux(aac)")){
			m_bAacDecode = FALSE;
			m_csAudioPath = m_csVideoPath + L".aac";
		} else {
			m_csAudioPath = m_csVideoPath + L".wav";
		}
		m_csVideoPath += L".m2v";
	} else if (m_pCmdLine->m_disable_rename == FALSE){
		if(ext[0] == L'\0'){
			if(wcsstr(buff, L"-vcodec copy") || wcsstr(buff, L"-vcodec mpeg") || wcsstr(buff, L"-target ntsc-dvd") ){
				m_csVideoPath += L".mpg";
			} else if(wcsstr(buff, L"-f mp4") || wcsstr(buff, L"-f psp") || wcsstr(buff, L"-f ipod")){
				m_csVideoPath += L".mp4";
			} else {
				m_csVideoPath += L".avi";
			}
		} else {
			m_csVideoPath += L".";
			m_csVideoPath += ext;
		}
	}
	if(wcsstr(param, L"Demux(wav)") || wcsstr(param, L"Demux(aac)")){
		m_csVideoPath = L"";
	} else if(wcsstr(param, L"Demux(m2v)")){
		m_csAudioPath = L"";
	}
/*
	switch(m_ComboSound.GetCurSel()){
		case 0:		// Normal Stereo
			break;
		case 1:		// Right Only
			break;
		case 2:		// Left Only
			break;
		case 3:		// Force 5.1ch
			break;
		default:
			break;
	}
*/
	// コンバート開始
	CComboBox *pServiceCombo = static_cast<CComboBox *>(GetDlgItem(IDC_SVCOMBO));
	m_wSelectServiceID = NO_SERVICE_SELECT;
	int iSelPos = pServiceCombo->GetCurSel();
	if(iSelPos!=-1)
	{
		m_wSelectServiceID = static_cast<WORD>(pServiceCombo->GetItemData(iSelPos));
	}

	const WORD wReturn = m_TsConverter.ConvertTsFile(m_csTsPath,READ_TO_FILEEND,m_wSelectServiceID,
													(LPCTSTR)m_csVideoPath,
													(LPCTSTR)m_csAudioPath,
													(m_bAacDecode)? true : false,
													true,//(m_bLipSync)? true : false,
													buff,
													m_ComboSound.GetCurSel(),
													m_ComboAudioEs.GetCurSel(),
													m_check_rf64,
													m_vframe_hokan,
													m_audio_delay,
													!m_Descramble
													);

	if(wReturn == ERR_FILE_CANT_OPEN){
		::AfxMessageBox(TEXT("ファイルのオープンに失敗しました。"));
	} else if(wReturn == ERR_FFMPEG_NOT_FOUND){
		::AfxMessageBox(TEXT("FFMpegを開始できません。"));
	} else if(wReturn == ERR_CANT_START){
		::AfxMessageBox(TEXT("デコードを開始できません。"));
	} else {
		// 正常
		// ステータス表示
		DisplayBatchStatus(TEXT("処理開始"));
	}
}

LRESULT CBonTsDemuxDlg::OnDoBatch(WPARAM wParam, LPARAM lParam )
{
    // ここにメッセージを受け取ったときの処理を記述
	OnLbnSelchangeBatchList();

	StartDemux();
	return 0;
}


void CBonTsDemuxDlg::OnBnClickedStart()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_Abort = FALSE;
//	OnBnClickedAacDecode();
	
//	if(!m_bVideoOut && !m_bAudioOut){
//		::AfxMessageBox(TEXT("ビデオまたはオーディオを出力設定にしてください。"));
//		return;
//		}

	if(m_csVideoPath != _TEXT("")){
		OnBnClickedBatchAdd();
	}
	m_ListBatch.SetCurSel(0);
	UpdateData(TRUE);
	OnLbnSelchangeBatchList();

	StartDemux();
}

void CBonTsDemuxDlg::OnBnClickedStop()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_Abort = TRUE;
	m_TsConverter.CancelConvert();
}

void CBonTsDemuxDlg::LockControl(const BOOL bLock)
{
	// コントロールをロックする
	const BOOL bEnable = (bLock)? FALSE : TRUE;

	GetDlgItem(IDC_START)->EnableWindow(bEnable);
	GetDlgItem(IDC_BROWSETS)->EnableWindow(bEnable);
	GetDlgItem(IDC_BROWSEVIDEO)->EnableWindow(bEnable);
	GetDlgItem(IDC_BROWSEFOLDER)->EnableWindow(bEnable);
	GetDlgItem(IDC_TSPATH)->EnableWindow(bEnable);
	GetDlgItem(IDC_VIDEOPATH)->EnableWindow(bEnable);
	GetDlgItem(IDC_FOLDERPATH)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO1)->EnableWindow(bEnable);
	GetDlgItem(IDC_BATCH_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_BATCH_ADD)->EnableWindow(bEnable);
	GetDlgItem(IDC_BATCH_DEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_SOUND)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_AUDIOES)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_REPLACE)->EnableWindow(bEnable);
	GetDlgItem(IDC_SVCOMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_SERVICE_REFLESH)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_RF64)->EnableWindow(bEnable);
	GetDlgItem(IDC_FRAME_HOKAN)->EnableWindow(bEnable);
	GetDlgItem(IDC_AUDIO_DELAY)->EnableWindow(bEnable);
	GetDlgItem(IDC_DESCRAMBLE)->EnableWindow(bEnable);
	GetDlgItem(IDC_USE_BACKGROUND_MODE)->EnableWindow(bEnable);

	GetDlgItem(IDC_STOP)->EnableWindow(!bEnable);
}

void  CBonTsDemuxDlg::OnTsConverterStart(const ULONGLONG llFileSize)
{
	// 変換開始イベント
	LockControl(TRUE);

	if (m_ListBatch.GetCurSel() == 0 ) {
		m_Progress.SetRange(0, 1000);
		if (m_bWindows7){
			m_ptrTaskbarList3->SetProgressValue(m_hWnd, 0, 1000);
			m_ptrTaskbarList3->SetProgressState(m_hWnd, TBPF_NORMAL);
		}
		m_Progress.SetPos(0);
	}

	if (m_bBackgroundMode) {
		::SetPriorityClass(GetCurrentProcess(),
			IsVistaOrHigher()
			? PROCESS_MODE_BACKGROUND_BEGIN
			: IDLE_PRIORITY_CLASS);
		// XPまででも一応プロセス優先度を下げておく。気休めくらいの効果しかないと思うけど。
	}
}

void  CBonTsDemuxDlg::OnTsConverterEnd(const ULONGLONG llFileSize)
{
	// 変換終了イベント
	int sel = m_ListBatch.GetCurSel() + 1;

	LockControl(FALSE);
	if(m_bTestMode){
		SetDlgItemText(IDC_INFOVIEW, TEXT("サーチ完了"));
		return;
	}

	if(m_ListBatch.GetCount() <= sel || m_Abort || llFileSize==0){

		m_Progress.SetPos(0);

		TBPFLAG tflag = TBPF_NOPROGRESS;

		if (m_Abort) {
			SetDlgItemText(IDC_INFOVIEW, TEXT("変換が中止されました"));
		}else if(llFileSize==0){
			SetDlgItemText(IDC_INFOVIEW, TEXT("FFMPEGでエラーが発生したので変換を中止しました"));
			tflag = TBPF_ERROR;
		}else{
			SetDlgItemText(IDC_INFOVIEW, TEXT("変換が完了しました"));
		}

		if (m_bWindows7)
			m_ptrTaskbarList3->SetProgressState(m_hWnd,tflag);

		m_csVideoPath = "";
		m_csTsPath = "";

		GetDlgItem(IDC_TSPATH)->SetWindowTextW(L"");
		GetDlgItem(IDC_VIDEOPATH)->SetWindowTextW(L"");
		if(m_pCmdLine->m_quit){
			EndDialog(0);
		} else {
			FlashWindow(TRUE);
		}
	} else {
		m_ListBatch.SetCurSel(sel);
		::PostMessage(m_hWnd,WM_DOBATCH,0,0);
	}
	if (m_bBackgroundMode) {
		::SetPriorityClass(GetCurrentProcess(),
			IsVistaOrHigher()
			? PROCESS_MODE_BACKGROUND_END
			: NORMAL_PRIORITY_CLASS);
	}
}

void CBonTsDemuxDlg::OnTsConverterProgress(const ULONGLONG llCurPos, const ULONGLONG llFileSize)
{
	// 進捗更新イベント
	if(m_bTestMode) return;
	
	//進捗表示を全体表示にする
	
	int progress = 	(int)(llCurPos * 1000 / llFileSize);
	TRACE(L"progress =%d\n",progress);
	m_Progress.SetPos(progress);

	//Windows 7 ではタスクバーで全体の進捗表示
	if (m_bWindows7) {
		int c0 = 1000 / m_ListBatch.GetCount();
		progress = 	(c0 * m_ListBatch.GetCurSel()) + (int)(llCurPos * c0 / llFileSize);
		m_ptrTaskbarList3->SetProgressValue(m_hWnd,progress,1000);
	}
}

void CBonTsDemuxDlg::OnTsConverterServiceName(LPCTSTR lpszServiceName)
{
	// サービス名更新イベント
	CString csText;
	csText.Format(TEXT("サービス名： %s"), lpszServiceName);
	//SetDlgItemText(IDC_INFOVIEW, csText);
	DisplayBatchStatus(csText);
}

void CBonTsDemuxDlg::OnTsConverterServiceInfo(CProgManager *pProgManager)
{
	// サービスリスト更新イベント
	CComboBox *pServiceCombo = static_cast<CComboBox *>(GetDlgItem(IDC_SVCOMBO));

	int iCurSel = pServiceCombo->GetCurSel();
	if(pServiceCombo->GetCount()==0) iCurSel = -1;
	WORD wCurServiceSel = static_cast<WORD>(pServiceCombo->GetItemData(iCurSel));
	if(!m_bTestMode){
		wCurServiceSel = m_wSelectServiceID;
	}
	pServiceCombo->ResetContent();
	TCHAR szServiceName[1024];
	WORD wServiceID;
	bool bSelected = false;
	
	// サービス名更新
	for(WORD wIndex = 0U ; wIndex < pProgManager->GetServiceNum() ; wIndex++){
		if(pProgManager->GetServiceID(&wServiceID, wIndex)){
			CString Str;
			if(pProgManager->GetServiceName(szServiceName, wIndex)){				
				Str.Format(TEXT("%5d : %s"),wServiceID,szServiceName);
			}
			else{				
				Str.Format(TEXT("%5d : サービス名取得中..."),wServiceID);
			}
			// サービス更新
			int iPos = pServiceCombo->AddString(Str);
			pServiceCombo->SetItemData(iPos,wServiceID);
			// リスト変更前に選択しているアイテムなら再選択する
			if(iCurSel != -1){
				if(wCurServiceSel==wServiceID){
					pServiceCombo->SetCurSel(iPos);
					bSelected = true;
				}
			}
		}
	}
	// 選択がまったくない場合、一番上のアイテムを選択する
	if(!bSelected || m_bTestMode){
		pServiceCombo->SetCurSel(0);
	}
}
void CBonTsDemuxDlg::OnBnClickedBatchAdd()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData(TRUE);

	CString buff,buff2,num;
	if(m_csTsPath == L"" || m_csVideoPath == L""){
		AfxMessageBox(L"ファイル名がありません。");
		return;
	}
//	buff = m_csTsPath+L","+m_csVideoPath+L","+m_csAudioPath;
	m_csTsPath.Replace(L"\"",L"");
	m_csVideoPath.Replace(L"\"",L"");
	buff = L"\"" + m_csTsPath+L"\",\""+m_csVideoPath + L"\"";

	num.Format(L"%d",m_ComboEncode.GetCurSel());
	buff += L",";
	buff += num;

	num.Format(L"%d",m_ComboSound.GetCurSel());
	buff += L",";
	buff += num;

	num.Format(L"%d",m_ComboAudioEs.GetCurSel());
	buff += L",";
	buff += num;

	num.Format(L"%d",m_check_rf64);
	buff += L",";
	buff += num;

	num.Format(L"%d",m_vframe_hokan);
	buff += L",";
	buff += num;

	num.Format(L"%d",m_audio_delay);
	buff += L",";
	buff += num;

	num.Format(L"%d",m_Descramble);
	buff += L",";
	buff += num;

	num.Format(L"%d",m_ComboService.GetCurSel());	
	buff += L",";
	buff += num;

	for(int i = 0 ; i < m_ComboService.GetCount() ; i ++){
		wchar_t param[256];
		m_ComboService.GetLBText( i,param );
		buff += L",";
		buff += param;
		buff2.Format(L"%d",(DWORD)m_ComboService.GetItemData(i));
		buff += L",";
		buff += buff2;
	}
	
	//重複チェック
	int cnt;
	cnt = m_ListBatch.GetCount();
	for(int i = 0 ; i < cnt ; i++){
		CString item;
		m_ListBatch.GetText(i,item);
		if(item == buff) return;
	}
	
	int sel = m_ListBatch.GetCurSel();
	if(sel <= 0){
		m_ListBatch.InsertString(m_ListBatch.GetCount(),buff);
	} else {
		m_ListBatch.InsertString(sel,buff);
	}

	m_ListBatch.SetHorizontalExtent(1200);	// リストボックスの幅を適当に広げる。

	m_csVideoPath = "";
	m_csTsPath = "";

	m_ComboService.ResetContent();

	UpdateData(FALSE);
}

void CBonTsDemuxDlg::OnBnClickedBatchDel()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	int i = m_ListBatch.GetCurSel();
	m_ListBatch.DeleteString(i);
	if (i >= m_ListBatch.GetCount()) {
		i = m_ListBatch.GetCount() - 1;
	}
//	m_ListBatch.SetCurSel(0);
	m_ListBatch.SetCurSel(i);

	if (m_ListBatch.GetCount() == 0) {
		m_ListBatch.SetHorizontalExtent(0);
	}

	m_csVideoPath = "";
	m_csTsPath = "";

	m_ComboService.ResetContent();

	UpdateData(FALSE);
}

void CBonTsDemuxDlg::OnLbnSelchangeBatchList()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	CString item,num;
	int sel = m_ListBatch.GetCurSel();
	if(sel <0) return;

	m_ListBatch.GetText(m_ListBatch.GetCurSel(),item);
	RtlToken tok((LPCTSTR)item, L",");
	m_csTsPath = tok.GetNextToken();
	m_csVideoPath = tok.GetNextToken();
//	m_csAudioPath = tok.GetNextToken();

	num = tok.GetNextToken();
	m_ComboEncode.SetCurSel(_ttoi((LPCTSTR)num));
	num = tok.GetNextToken();
	m_ComboSound.SetCurSel(_ttoi((LPCTSTR)num));
	num = tok.GetNextToken();
	m_ComboAudioEs.SetCurSel(_ttoi((LPCTSTR)num));

	num = tok.GetNextToken();
	m_check_rf64 = _ttoi((LPCTSTR)num);
	num = tok.GetNextToken();
	m_vframe_hokan = _ttoi((LPCTSTR)num);
	num = tok.GetNextToken();
	m_audio_delay = _ttoi((LPCTSTR)num);
	num = tok.GetNextToken();
	m_Descramble = _ttoi((LPCTSTR)num);

	num = tok.GetNextToken();
	m_ComboService.ResetContent();
	for(int i = 0;; i ++){
		item = tok.GetNextToken();
		if(item == L"") break;
		m_ComboService.AddString(item);
		item = tok.GetNextToken();
		m_ComboService.SetItemData(i,_ttoi((LPCTSTR)item));
	}
	m_ComboService.SetCurSel(_ttoi((LPCTSTR)num));


	UpdateData(FALSE);
}

void CBonTsDemuxDlg::OnBnClickedButtonReplace()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	if(m_csTsPath == L"" || m_csVideoPath == L""){
		AfxMessageBox(L"ファイル名がありません。");
		return;
	}

//	OnBnClickedBatchDel();
	m_ListBatch.DeleteString(m_ListBatch.GetCurSel());
	OnBnClickedBatchAdd();
}

void CBonTsDemuxDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
    wchar_t FileName[_MAX_PATH];
    int NameSize = lengthof(FileName);
    int FileNumber;
    CString str;
    int i;

    FileNumber = DragQueryFile(hDropInfo, 0xffffffff, FileName, NameSize);

    for(i=0; i<FileNumber; i++){
        DragQueryFile(hDropInfo, i, FileName, NameSize);
		if(wcsstr(FileName, L".ts")){
			UpdateData(TRUE);

			m_csTsPath = FileName;

			//2010.05.20 最後に開いたファイルのフォルダを保存する。
			PathRemoveFileSpec(FileName);
			m_DlgParm.m_tspath_dir = FileName;
			
//			m_csVideoPath = m_csTsPath;
//			m_csVideoPath.Replace(TEXT(".ts"), TEXT(".avi"));
			if (m_csFolderPath.IsEmpty()) {
				m_csVideoPath = m_csTsPath;
			} else {
				TCHAR buf[MAX_PATH];
				::PathCombine(buf, m_csFolderPath, ::PathFindFileName(m_csTsPath));
				m_csVideoPath = buf;
			}
			if(m_csVideoPath.ReverseFind('.') && m_csVideoPath.ReverseFind('.') > m_csVideoPath.ReverseFind('\\')){
				m_csVideoPath.Delete(m_csVideoPath.ReverseFind('.'),m_csVideoPath.GetLength());
			}
			
//			m_csAudioPath = m_csTsPath;
//			m_csAudioPath.Replace(TEXT(".ts"), TEXT(".aac"));

			m_ComboService.ResetContent();

			UpdateData(FALSE);
			OnBnClickedBatchAdd();
		}
    }

//	__super::OnDropFiles(hDropInfo);
}

void CBonTsDemuxDlg::OnBnClickedServiceReflesh()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_bTestMode = TRUE;
	const WORD wReturn = m_TsConverter.ConvertTsFile(m_csTsPath, TEST_READ_SIZE, NO_SERVICE_SELECT, NULL, NULL);
}

//バッチ処理中のステータス表示
void CBonTsDemuxDlg::DisplayBatchStatus(CString str)
{
	if(m_bTestMode){
		SetDlgItemText(IDC_INFOVIEW,str);
	}else{
		CString csText;

		csText.Format(TEXT("変換中です： %d/%d...  "),  m_ListBatch.GetCurSel() +1,  m_ListBatch.GetCount());
		csText.Append(str);
		SetDlgItemText(IDC_INFOVIEW,csText);
	}
}
