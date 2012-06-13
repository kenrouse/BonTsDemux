// BonTsDemuxDlg.h : ヘッダー ファイル
//

#pragma once
#include "afxcmn.h"
#include "TsConverter.h"
#include "BonTsDemuxDlgParm.h"
#include "afxwin.h"
#include "Commandline.h"

#define TEST_READ_SIZE 5*1024*1024

// CBonTsDemuxDlg ダイアログ
class CBonTsDemuxDlg : public CDialog, protected CTsConverter::IEventHandler
{
// コンストラクション
public:
	CBonTsDemuxDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CBonTsDemuxDlg)
	enum { IDD = IDD_BONTSDEMUX_DIALOG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CBonTsDemuxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート
	//}}AFX_VIRTUAL

// 実装
protected:

	// 生成された、メッセージ割り当て関数
	//{{AFX_MSG(CBonTsDemuxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
//	afx_msg void OnBnClickedAacDecode();
	afx_msg void OnBnClickedBrowseTs();
	afx_msg void OnBnClickedBrowseVideo();
	afx_msg void OnBnClickedBrowseFolder();
//	afx_msg void OnBnClickedBrowseAudio();
//	afx_msg void OnBnClickedVideoOut();
//	afx_msg void OnBnClickedAudioOut();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDoBatch(UINT wParam,LONG lParam);
	afx_msg void OnBnClickedBatchAdd();
	afx_msg void OnBnClickedBatchDel();
	afx_msg void OnLbnSelchangeBatchList();
	afx_msg void OnBnClickedButtonReplace();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedServiceReflesh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	


	void LockControl(const BOOL bLock = TRUE);

// CTsConverter::IEventHandler
	virtual void OnTsConverterStart(const ULONGLONG llFileSize);
	virtual void OnTsConverterEnd(const ULONGLONG llFileSize);
	virtual void OnTsConverterProgress(const ULONGLONG llCurPos, const ULONGLONG llFileSize);
	virtual void OnTsConverterServiceName(LPCTSTR lpszServiceName);
	virtual void OnTsConverterServiceInfo(CProgManager *pProgManager);

	HICON m_hIcon;
	CString m_csTsPath;
	CString m_csVideoPath;
	CString m_csAudioPath;
	CString m_csFolderPath;
	BOOL m_bVideoOut;
	BOOL m_bAudioOut;
	BOOL m_bAacDecode;
	BOOL m_bLipSync;
	CProgressCtrl m_Progress;
	
	CTsConverter m_TsConverter;
	CComboBox m_ComboEncode;
	CString m_ini_path;
private:
	BOOL m_Abort;
	CListBox m_ListBatch;
	void StartDemux(void);
	CComboBox m_ComboSound;
	CComboBox m_ComboAudioEs;
	BOOL m_bTestMode;
	BOOL m_wSelectServiceID;
	CCommandLine* m_pCmdLine;
	CComboBox m_ComboService;
	BOOL m_check_rf64;
	BOOL m_vframe_hokan;
	int m_audio_delay;
	BOOL m_Descramble;
	BOOL m_bBackgroundMode;
	CComPtr<ITaskbarList3> m_ptrTaskbarList3;
	BOOL m_bWindows7;  // Windows 7以降かどうか

	CBonTsDemuxDlgParm m_DlgParm; //2010.05.20 画面設定保存用 by fuji 

	void CBonTsDemuxDlg::DisplayBatchStatus(CString str);

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};


// strtok のクラス実装
class RtlToken
{
protected:
// Members
    CString m_strText;
    CString m_strSep;

public:
// Ctor/Dtor
    RtlToken(LPCTSTR lpszText = _T(""), LPCTSTR lpszSeparator = _T
(" ")) : \
        m_strText(lpszText), m_strSep(lpszSeparator)
    {
    }
    ~RtlToken()
    {
        m_strText.Empty();
        m_strSep.Empty();
    }

    void operator()(LPCTSTR lpszText = _T(""), LPCTSTR lpszSeparator = _T
(" "))
    {
        m_strText = lpszText;
        m_strSep = lpszSeparator;
    }
    void operator=(LPCTSTR lpszText)
    {
        m_strText = lpszText;
    }
    void SetSeparator(LPCTSTR lpszSeparator = _T(" "))
    {
        m_strSep = lpszSeparator;
    }

// Operations
    // 参照されていない残り部分を返し、バッファをクリアする
    CString GetRestString()
    {
        CString strRet = m_strText;
        m_strText.Empty();
        return strRet;
    }

    // 一度使用した文字列と直後の区切り文字列は削除する
    CString GetNextToken()
    {
        //ATLASSERT( !m_strSep.IsEmpty() );

        CString strRet = m_strText;

        if (m_strSep.IsEmpty()) {
            m_strText.Empty();
            return strRet;
        }

		//一文字目がダブルクォーテーションのときはカンマをエスケープ
		int iStart = 0;
		if (strRet.Left(1) ==L"\""){
			const int iPos_dq = strRet.Find(L"\"",1);
			iStart = iPos_dq + 1;
		}
        const int nPos = strRet.Find(m_strSep,iStart);
        if (nPos >= 0)
            strRet = strRet.Left(nPos);
		
        // standby for next call
        const int nLen = strRet.GetLength();
        if (nLen >= 0)
            m_strText.Delete(0, m_strSep.GetLength() + nLen);
	
		if (iStart > 0 ) {
			strRet.Replace(L"\"",L"");
		}

        return strRet;
    }
};
