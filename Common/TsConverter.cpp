#include "StdAfx.h"
#include "TsConverter.h"

//////////////////////////////////////////////////////////////////////////////
//FFMPEG起動関連

STARTUPINFO ffmpeg_si;
PROCESS_INFORMATION ffmpeg_pi;

/** kemaruya **/
CString m_szLogFilePath;
HANDLE hOutput = NULL;
/** kemaruya **/

bool OpenFFmpeg(LPWSTR vlc)
{
	// ffmpegの起動。
	::ZeroMemory(&ffmpeg_si, sizeof(STARTUPINFO));
	ffmpeg_si.cb           = sizeof(STARTUPINFO);
	ffmpeg_si.dwFlags      = STARTF_USESHOWWINDOW;
	ffmpeg_si.wShowWindow  = SW_MINIMIZE;
	
	//ffmpegのある場所をHOMEとして設定する
	size_t envsize;
	wchar_t szModulePath[MAX_PATH]  ,drv[_MAX_DRIVE],path[_MAX_DIR] ,ext[_MAX_EXT],appname[_MAX_FNAME],home_path[_MAX_PATH];
	GetModuleFileName(NULL,szModulePath,MAX_PATH);
	_wsplitpath(szModulePath,drv,path,appname,ext);
	_wmakepath(home_path,drv,path,L"",L"");
	_wputenv_s(L"HOME",home_path);

	//ffmpegの起動コマンドをファイルに記録
	wchar_t ffmpeg_exec[_MAX_PATH];
	_wmakepath(ffmpeg_exec,drv,path,L"ffmpeg_exec",L"txt");

	CStdioFile of(ffmpeg_exec,CFile::modeWrite|CFile::modeCreate);
	of.WriteString(vlc);
	of.Close();

#if _DEBUG
	std::wcout << vlc << endl;
	TRACE(vlc);
#endif
	

	/** kemaruya **/
	SECURITY_ATTRIBUTES sec_attr;
	ZeroMemory(&sec_attr,sizeof(sec_attr));
	sec_attr.nLength = sizeof(sec_attr);
	sec_attr.bInheritHandle = TRUE;

    hOutput = ::CreateFile(m_szLogFilePath.GetString(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		&sec_attr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL );
 
    ffmpeg_si.dwFlags    = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    ffmpeg_si.hStdOutput = hOutput;
	ffmpeg_si.hStdError  = hOutput;

	BOOL ret = ::CreateProcess(NULL, vlc, NULL, NULL, TRUE,
		CREATE_NO_WINDOW, NULL, NULL, &ffmpeg_si, &ffmpeg_pi);

	//BOOL ret = ::CreateProcess(NULL, vlc, NULL, NULL, FALSE, /*DETACHED_PROCESS*///0
	//		CREATE_NEW_CONSOLE
	//	, NULL, NULL, &ffmpeg_si, &ffmpeg_pi);

	/** kemaruya **/

	if (ret) {
		CloseHandle(ffmpeg_pi.hThread);
	} else {
		ffmpeg_pi.hProcess = INVALID_HANDLE_VALUE;
		//MessageBox(NULL, L"ffmpeg起動失敗。\n",L"エラー",MB_OK);
		return FALSE;
	}

	return TRUE;
}

DWORD CloseFFmpeg(void)
{
	DWORD ret = 0;
	if (ffmpeg_pi.hProcess != INVALID_HANDLE_VALUE) {
		if(::WaitForSingleObject(ffmpeg_pi.hProcess, 0) == WAIT_TIMEOUT){
	//		log_out("ffmpegの終了を待っています。\n");
			::WaitForSingleObject(ffmpeg_pi.hProcess, INFINITE);
	//		log_out("ffmpeg終了。\n");
		}
		GetExitCodeProcess(ffmpeg_pi.hProcess,&ret);
		CloseHandle(ffmpeg_pi.hProcess);
		ffmpeg_pi.hProcess = INVALID_HANDLE_VALUE;
	}

	/** kemaruya **/
	if( NULL != hOutput )
	{
		::CloseHandle(hOutput);
	}
	hOutput = NULL;
	/** kemaruya **/

	return ret;
}



CTsConverter::CTsConverter(IEventHandler *pEventHandler)
	: m_pEventHandler(pEventHandler)
	, m_FileReader(this)
	, m_TsPacketParser(this)
	, m_ProgManager(this)
	, m_TsDescrambler(this)
	, m_VideoFile(this)
	, m_AudioFile(this)
	, m_AacConverter(this)
	, m_WaveWriter(this)
	, m_WavWriter(this)
	, m_WavSplitWriter(this)
{
	m_AudHoseiFromVid = FALSE;
	m_pWaveSend = NULL;
	m_pVideoSend = NULL;
	
	m_pVideoBand = NULL;
	m_pAudioBand = NULL;

	m_pTsDemuxer = NULL;
	m_Useffmpg = FALSE;
	m_vframe_hokan = FALSE;
	m_DecodeAac = FALSE;
	m_audio_delay = 0;

	m_pAudioPath = NULL;
}

CTsConverter::~CTsConverter(void)
{
	CancelConvert();
}

static BYTE WaveHead[] =		// from CWaveWriter@BonTsDemux
{
	'R', 'I', 'F', 'F',				// +0	RIFF
	0x00U, 0xf8U, 0xffU, 0xffU,		// +4	これ以降のファイルサイズ(ファイルサイズ - 8)
	'W', 'A', 'V', 'E',				// +8	WAVE
	'f', 'm', 't', ' ',				// +12	fmt
	0x10U, 0x00U, 0x00U, 0x00U,		// +16	fmt チャンクのバイト数
	0x01U, 0x00U,					// +18	フォーマットID
	0x02U, 0x00U,					// +20	ステレオ
	0x80U, 0xBBU, 0x00U, 0x00U,		// +24	48kHz
	0x00U, 0xEEU, 0x02U, 0x00U,		// +28	192000Byte/s
	0x04U, 0x00U,					// +30	ブロックサイズ
	0x10U, 0x00U,					// +32	サンプルあたりのビット数
	'd', 'a', 't', 'a',				// +36	data
	0x00U, 0xffU, 0xffU, 0xffU		// +40	波形データのバイト数
};

int CTsConverter::FindAvailablePort(void)
{
	
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;

	WSAStartup(MAKEWORD(2,0), &wsaData);
	sock0 = socket(AF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
	int size_of_addr = sizeof(addr);
	getsockname(sock0,(struct sockaddr *)&addr, &size_of_addr);
	closesocket(sock0);

	return addr.sin_port;
}
const WORD CTsConverter::ConvertTsFile(LPCTSTR lpszTsFile, QWORD qwInputSizeLimit,WORD wServiceSelect, LPCTSTR lpszVideoFile, LPCTSTR lpszAudioFile,
									   const bool bDecodeAac, const bool bLipSync,LPCTSTR ffmpeg_param,DWORD SoundMethod,DWORD AudioEs,BOOL bRf64,BOOL vhokan,
									   int audio_delay,BOOL NoDescramble)
{
/* 
	－AACデコードなし－
	m_FileReader → [m_TsPacketParser] → m_TsDescrambler → m_ProgManager → m_TsDemuxer → m_VideoFile
　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　→ m_AudioFile
	－AACデコードあり－
	m_FileReader → [m_TsPacketParser] → m_TsDescrambler → m_ProgManager → m_TsDemuxer → m_VideoFile
　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　→ m_AacConverter → m_WaveWriter
*/
	// 空いているポートを見つける。（多重起動対策）
	int port_number = FindAvailablePort();
	//port_number = 1234;

	// デコーダを一旦クローズ
	CloseDecoders();

	m_vframe_hokan = vhokan;

	m_AudioEsNum = AudioEs;
	m_ServiceSelect = wServiceSelect;

	m_AacConverter.SetOutputChannel(2);
	m_AacConverter.SetStereoMethod(0);		// Stereo To Mono
	WaveHead[22] = 0x02;
	m_achannel = 2*2;
	m_Useffmpg = FALSE;
	m_DecodeAac = bDecodeAac;
	m_audio_delay = audio_delay;

	if (lpszAudioFile) {
		m_pAudioPath = new CString(lpszAudioFile);
	}

	if(SoundMethod == 3 || SoundMethod == 4){			// 5.1ch指定
		m_AacConverter.SetOutputChannel(6);
		WaveHead[22] = 0x06;
		m_achannel = 6*2;
	} else if (SoundMethod == 1 || SoundMethod == 2){
		m_AacConverter.SetStereoMethod(SoundMethod);
	}

	try{
		// TSファイルを開く
		if(!m_FileReader.OpenFile(lpszTsFile,qwInputSizeLimit))throw 0UL;
		m_pTsDemuxer = new CTsDemuxer(this);

		if(ffmpeg_param == NULL || ffmpeg_param[0] == L'\0'){

			if(lpszVideoFile != NULL && lpszVideoFile[0] != L'\0'){
				if(!m_VideoFile.OpenFile(lpszVideoFile))throw 1UL;
				m_pTsDemuxer->SetOutputDecoder(&m_VideoFile, CTsDemuxer::OUTPUT_VIDEO);
			}

			if(lpszAudioFile != NULL && lpszAudioFile[0] != L'\0'){
				if(bDecodeAac){
					// AACデコードあり
					if(SoundMethod==4){		// 5.1ch split
						if(!m_WavSplitWriter.OpenFile(lpszAudioFile, 16, 6 , 48000))throw 2UL;					
						m_AacConverter.SetOutputDecoder(&m_WavSplitWriter);
					} else {				// not split
						if(!m_WavWriter.OpenFile(lpszAudioFile, 16, SoundMethod==3 ? 6 : 2 , 48000 , bRf64 ? FALSE : TRUE))throw 2UL;
						m_AacConverter.SetOutputDecoder(&m_WavWriter);
					}
					m_pTsDemuxer->SetOutputDecoder(&m_AacConverter, CTsDemuxer::OUTPUT_AUDIO);
				} else {
					// AACデコードなし
					if(!m_AudioFile.OpenFile(lpszAudioFile, CNCachedFile::CNF_SHAREDELETE))throw 3UL;
					m_pTsDemuxer->SetOutputDecoder(&m_AudioFile, CTsDemuxer::OUTPUT_AUDIO);
				}
			}
		} else {
			m_Useffmpg = TRUE;

/********************************/
// Audioを先にffmpegに渡すようにする。
//  Videoを先に渡すとffmpegでvideo streamを解析するために必要なデータを送信し終わる前に
//  BonTsDemuxでAudioを送信しようとして固まってしまうため
			m_pTsDemuxer->SetOutputDecoder(&m_AacConverter, CTsDemuxer::OUTPUT_AUDIO);

			m_pAudioBand = new CKeepRate(1024*64,		//packet
												0,		//margin
												10*1024*1024,		//max
												0,		//max_bps
												TRUE,	//copydata
												L"WAV",	//name
												this);	//Audio用クッションバッファ作る
			m_AacConverter.SetOutputDecoder(m_pAudioBand);

//			m_pWaveSend = new CHttpSend(this,1234,64*1024);
			m_pWaveSend = new CHttpSend(this,port_number,64*1024);
			m_pAudioBand->SetOutputDecoder(m_pWaveSend);
/********************************/

			m_pVideoBand = new CKeepRate(64*1024,			//packet
											0,				//margin
											20*1024*1024,	//max
											0,	//max_bps
											TRUE,			//copydata
											L"M2V",			//name
											this);	//VIDEO用クッションバッファ作る
//			m_pVideoSend = new CHttpSend(this,1234,64*1024);
			m_pVideoSend = new CHttpSend(this,port_number,64*1024);
				
			m_pTsDemuxer->SetOutputDecoder(m_pVideoBand, CTsDemuxer::OUTPUT_VIDEO);
			m_pVideoBand->SetOutputDecoder(m_pVideoSend);

/* Audioを先に出力するように変更したのでここはコメントアウト
			m_pTsDemuxer->SetOutputDecoder(&m_AacConverter, CTsDemuxer::OUTPUT_AUDIO);

			m_pAudioBand = new CKeepRate(1024*64,		//packet
												0,		//margin
												10*1024*1024,		//max
												0,		//max_bps
												TRUE,	//copydata
												L"WAV",	//name
												this);	//Audio用クッションバッファ作る
			m_AacConverter.SetOutputDecoder(m_pAudioBand);

//			m_pWaveSend = new CHttpSend(this,1234,64*1024);
			m_pWaveSend = new CHttpSend(this,port_number,64*1024);
			m_pAudioBand->SetOutputDecoder(m_pWaveSend);
*/

			CMediaData *m = new CMediaData(WaveHead, (DWORD)sizeof(WaveHead));
			m_pWaveSend->InputMedia(m);
			delete m;
		}


		// B-CASカードが利用可能なときはTSデスクランブラを接続する
		if(!NoDescramble && m_TsDescrambler.OpenBcasCard()){
			// B-CASカード利用可能
			m_TsPacketParser.SetOutputDecoder(&m_TsDescrambler);
			m_TsDescrambler.SetOutputDecoder(&m_TsServiceSelect);	
			m_TsServiceSelect.SetOutputDecoder(&m_ProgManager);	
			}
		else{
			// B-CASカード利用不可
			m_TsPacketParser.SetOutputDecoder(&m_TsServiceSelect);	
			m_TsServiceSelect.SetOutputDecoder(&m_ProgManager);
			}

		// デコーダグラフ接続
		m_FileReader.SetOutputDecoder(&m_TsPacketParser);
		m_ProgManager.SetOutputDecoder(m_pTsDemuxer);
		m_pTsDemuxer->EnableLipSync(bLipSync);
		}
	catch(DWORD dwErrorCode){
		// エラー発生
		CloseDecoders();
		return ERR_FILE_CANT_OPEN;
		}
	if(ffmpeg_param && ffmpeg_param[0] != L'\0'){

		wchar_t szModulePath[MAX_PATH]  ,drv[_MAX_DRIVE],path[_MAX_DIR] ,ext[_MAX_EXT],appname[_MAX_FNAME],execname[_MAX_PATH];
	//	wchar_t enc_param[256];
	//	enc_param[0] = L'\0';
		GetModuleFileName(NULL,szModulePath,MAX_PATH);
		_wsplitpath(szModulePath,drv,path,appname,ext);
		_wmakepath(execname,drv,path,L"ffmpeg",L"exe");

		CString str_port_number;
		str_port_number.Format(L"%d",port_number);
		CString arg;
		arg = ffmpeg_param;
//		arg = L" -f mpeg2video -i \"http://localhost:1234/video.m2v\" -f wav -i \"http://localhost:1234/audio.wav\" " + arg;
//		arg = L" -f mpeg2video -i \"http://localhost:" + str_port_number + L"/video.m2v\" -f wav -i \"http://localhost:" + str_port_number + L"/audio.wav\" " + arg;
//		arg = L" -f mpegvideo -i \"http://127.0.0.1:" + str_port_number + L"/video.m2v\" -f wav -i \"http://127.0.0.1:" + str_port_number + L"/audio.wav\" " + arg;
		arg = L" -f wav -i \"http://127.0.0.1:" + str_port_number + L"/audio.wav\" -f mpegvideo -i \"http://127.0.0.1:" + str_port_number + L"/video.m2v\" " + arg;
		arg = L"\"" + arg;
		arg = execname + arg;
		arg = L"\"" + arg;

		if(lpszVideoFile){
			arg = arg + L" \"";
			arg += lpszVideoFile;
			arg = arg + L"\"";
		} else {
			arg = arg + L" testout.mpg";
		}

		/** kemaruya **/
		m_szLogFilePath = lpszVideoFile;
		m_szLogFilePath += L".log";
		/** kemaruya **/

		if(!OpenFFmpeg(arg.GetBuffer(1)))
		{
			// エラー
			return ERR_FFMPEG_NOT_FOUND;
		}

		//if(!m_pVideoSend->IsOpen())
		if(!m_pWaveSend->IsOpen())
		{
			// エラー
			return ERR_CANT_START;
		}
	}


	// デコーダグラフリセット
	m_FileReader.Reset();
	
	// コンバート開始
	if(!m_FileReader.StartReadAnsync())
	{
		return ERR_CANT_START;
	}
	return 0;
}

void CTsConverter::CancelConvert(void)
{
	// コンバート中止
	m_FileReader.StopReadAnsync();
}

void CTsConverter::CloseDecoders(void)
{
	//2010.05.07 fuji
	if(m_pTsDemuxer)	{
		m_pTsDemuxer->Close();
	}

	// デコーダを閉じる
	m_TsDescrambler.CloseBcasCard();
	m_FileReader.CloseFile();
	m_VideoFile.CloseFile();
	m_AudioFile.CloseFile();
	m_WaveWriter.CloseFile();
	m_WavSplitWriter.CloseFile();
	m_WavWriter.CloseFile();

	while((m_pVideoBand && m_pVideoBand->m_bThreadIdle == FALSE) && (m_pAudioBand && m_pAudioBand->m_bThreadIdle == FALSE)){
		Sleep(100);
	}

	//2010.05.07 fuji 終了時に残りを出力する。
	if(m_pVideoBand)	m_pVideoBand->CompleteTrans();
	if(m_pAudioBand)	m_pAudioBand->CompleteTrans();


	DWORD ret_audio = 0 , ret_video = 0;
	do{
		if(m_pVideoBand && m_pAudioBand )
		{	
			ret_video = m_pVideoBand->WaitForThread(100);
			if ( ret_video == 0 ){ 
				m_pVideoSend->Flush();
				m_pVideoSend->Close();
			}
			ret_audio = m_pAudioBand->WaitForThread(100);
			if ( ret_audio == 0 ){ 
				m_pWaveSend->Flush();
				m_pWaveSend->Close();
			}
		}
	}while( ret_audio != 0 || ret_video != 0 );

	//if(m_pVideoBand)	m_pVideoBand->StopTrans();
	//if(m_pAudioBand)	m_pAudioBand->StopTrans();

//	if(m_pWaveSend)		m_pWaveSend->Close();
//	if(m_pVideoSend)	m_pVideoSend->Close();

	if(m_pVideoBand)	delete m_pVideoBand;
	if(m_pAudioBand) 	delete m_pAudioBand;
	
	if(m_pWaveSend)		delete m_pWaveSend;
	if(m_pVideoSend)	delete m_pVideoSend;

	if(m_pTsDemuxer)	delete m_pTsDemuxer;

	m_pWaveSend = NULL;
	m_pVideoSend = NULL;

	m_pVideoBand = NULL;
	m_pAudioBand = NULL;

	m_pTsDemuxer = NULL;

	if(m_pAudioPath)	delete m_pAudioPath;
	m_pAudioPath = NULL;

	m_dwFfmpegExitCode = CloseFFmpeg();
}

const DWORD CTsConverter::OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	// デコーダからのイベントを受け取る(暫定)
	if(pDecoder == &m_ProgManager){
		
		// プログラムマネージャからのイベント
		switch(dwEventID){
			case CProgManager::EID_SERVICE_LIST_UPDATED : {
				// サービスの構成が変化した
				m_pEventHandler->OnTsConverterServiceInfo(&m_ProgManager);
				WORD wVideoPID = 0xFFFF;
				WORD wAudioPID = 0xFFFF;
			
				if(m_ServiceSelect==NO_SERVICE_SELECT){
					
					m_ProgManager.GetServiceEsPID(&wVideoPID, &wAudioPID);

					//0番目のサービスIDを処理対象なのでそれを保存しておく
					WORD wSearchServiceID;
					m_ProgManager.GetServiceID(&wSearchServiceID,0);
					m_ServiceSelect = wSearchServiceID;

//++ 2010.03.04 added by pika
//	 サービス指定なしで音声ESが指定されたとき、
//	一つ目のサービスが指定されたものとして処理する
					if(m_AudioEsNum > 0) {
						//WORD wSearchServiceID;
						//if(m_ProgManager.GetServiceID(&wSearchServiceID,0)){//一つ目のサービス取得
							m_TsServiceSelect.GetAudioEs2(wSearchServiceID, m_AudioEsNum,NULL,&wAudioPID);
						//}
					}

//--				
				} else {
					for(WORD wSearchServiceIndex=0;wSearchServiceIndex<m_ProgManager.GetServiceNum();wSearchServiceIndex++){
						WORD wSearchServiceID;
						if(m_ProgManager.GetServiceID(&wSearchServiceID,wSearchServiceIndex)){
							if(wSearchServiceID==m_ServiceSelect){
								m_ProgManager.GetServiceEsPID(&wVideoPID, &wAudioPID, wSearchServiceIndex);	
								if(m_AudioEsNum){
//++ 2010.03.04 modified by pika
//									m_TsServiceSelect.GetAudioEs(m_AudioEsNum,NULL,&wAudioPID);
									m_TsServiceSelect.GetAudioEs2(m_ServiceSelect, m_AudioEsNum,NULL,&wAudioPID);
//--
									}
								}
							}
						}
					}
				m_pTsDemuxer->SetVideoPID(wVideoPID);
				m_pTsDemuxer->SetAudioPID(wAudioPID);
				m_pEventHandler->OnTsConverterServiceName(TEXT("サーチ中..."));

				/*
				TCHAR szServiceName[1024] = {TEXT("不明")};
				for(WORD wSearchServiceIndex=0;wSearchServiceIndex<m_ProgManager.GetServiceNum();wSearchServiceIndex++){
					WORD wSearchServiceID;
					if(m_ProgManager.GetServiceID(&wSearchServiceID,wSearchServiceIndex)){
						if(wSearchServiceID==m_ServiceSelect){
								m_ProgManager.GetServiceName(szServiceName,wSearchServiceIndex);
								m_pEventHandler->OnTsConverterServiceName(szServiceName);
						}
					}
				}
				m_pEventHandler->OnTsConverterServiceName(szServiceName);
				*/

				return 0UL;
				}

			case CProgManager::EID_SERVICE_INFO_UPDATED : {
				// サービス情報が更新された
				m_pEventHandler->OnTsConverterServiceInfo(&m_ProgManager);
				TCHAR szServiceName[1024] = {TEXT("不明")};
				for(WORD wSearchServiceIndex=0;wSearchServiceIndex<m_ProgManager.GetServiceNum();wSearchServiceIndex++){
					WORD wSearchServiceID;
					if(m_ProgManager.GetServiceID(&wSearchServiceID,wSearchServiceIndex)){
						if(wSearchServiceID==m_ServiceSelect){
								m_ProgManager.GetServiceName(szServiceName,wSearchServiceIndex);
								m_pEventHandler->OnTsConverterServiceName(szServiceName);
							}
						}
					}
				return 0UL;
				}
			}
	} else if(pDecoder == m_pTsDemuxer){
	
		switch(dwEventID){
		
			case CTsDemuxer::EID_SERVICE_FIRST_AAC:					// しょっぱな補完。基点をVIDEOのPTSにあわせるため、差分を埋める
				{
					LONG hokan;
					LONGLONG diff;
					diff = m_pTsDemuxer->GetM2VAACDif() + m_audio_delay*90;
					if (!m_DecodeAac) {
						// AACデコードなし
						// 差分をファイル名にセット
						diff /= 90;
						if (m_pAudioPath) {
							CString newAudioPath(*m_pAudioPath, m_pAudioPath->GetLength() - 4);	// 拡張子を切り捨て
							CString delay;
							delay.Format(TEXT(" DELAY %dms.aac"), (int) diff);
							newAudioPath += delay;
							MoveFile(*m_pAudioPath, newAudioPath);
						}
					} else {
						// AACデコードあり
						hokan = (LONG)(diff * 48 * m_achannel / 90);
						hokan -= (hokan % m_achannel);
						if(hokan > 1920000 || hokan < -1920000){
							TRACE(L"初回で10秒以上ずれている。\n");
							break;
						}
						TRACE2("初回補正 :hokan = %d, diff=%d \n",hokan,diff);
						if(hokan > 0){							//Videoが早い場合
							BYTE* dummy = new BYTE[hokan];
							ZeroMemory(dummy,hokan);

							CMediaData *m = new CMediaData(dummy, hokan);

							delete [] dummy;

							if(m_Useffmpg) m_pAudioBand->InputMedia(m);
							else m_WaveWriter.InputMedia(m);


							delete m;
							m_AacConverter.m_total_frame += hokan / m_achannel;

						} else if (hokan < 0){					// Videoが遅い場合
							m_AacConverter.SetCutFrame((DWORD)(diff *-1* 48 / 90));	// Video先頭にあわせるために、削除するAudioフレーム数を設定
							m_pTsDemuxer->m_FirstAudioPts = m_pTsDemuxer->m_FirstVideoPts;			// 先頭がVIDEOと同期するので、Audioの先頭PTSをVIDEOと同じにする
						}
					}
				}

				break;
			case CTsDemuxer::EID_SERVICE_M2V_COMMIT:
				if(m_vframe_hokan){
					if(m_AacConverter.GetTotalSample() < 100000) break;

					LONGLONG pts_ms = m_pTsDemuxer->GetTotalVideoPts();
					LONGLONG frame_ms = (LONGLONG)m_pTsDemuxer->GetVideoTotalFrame() * 3003;		// * 1000 / 29.97(fps)  → ms
					LONGLONG diff;

					diff = (pts_ms - frame_ms);

					if(diff >= 70 || diff <= -70){
						m_pTsDemuxer->m_FirstAudioPts += (LONGLONG)diff;		// 何らかの原因で、Videoが欠落している。
						m_pTsDemuxer->m_FirstVideoPts += (LONGLONG)diff;		// 補正する。
						TRACE((LPCTSTR)L"frame_ms:%I64d   , pts_ms:%I64d\n",frame_ms,pts_ms);
					}
				}
				break;
			case CTsDemuxer::EID_SERVICE_AAC_COMMIT:
				{

				if(m_AacConverter.GetTotalSample() < 100000) break;

//				TRACE2("AAC	%I64d	%d\n",m_pTsDemuxer->m_NowAudioPts,m_AacConverter.GetTotalSample());

				LONGLONG diff;
				diff = (m_pTsDemuxer->GetTotalAudioPts() / 90 - m_AacConverter.GetTotalSample() / 48);	// PTS 90kHz Audio 48kHz らしいので、単位をmsにあわせて差分をとる。

//				m_AudioLpfBuf = 0.05 * diff + 0.95 * m_AudioLpfBuf;		// PESと実データの差分をLPFに通す。係数は適当。。
//				m_AudioLpfBuf = diff;
				if(m_pTsDemuxer->m_NowAudioPts == -1) break;

				if (diff > 5000 || diff < -5000){		// ±2sec以上は、やりすぎなので恐らくエラーが発生している
					diff = 0;
					m_AacConverter.ResetTotalFrame();
					m_pTsDemuxer->ResetTotalAudioPts();
					m_pTsDemuxer->EnableLipSync(TRUE);
				} else {
					if (diff == 0){
						m_AacConverter.SetHoseiPol(0);
						m_AudHoseiFromVid = 0;
					} else if(diff >= 70){					// 2 frame 以上ずれていたら
						TRACE((LPCTSTR)L"diff : %dms\n",(LONG)diff);
						if(diff >= 200){												// 200ms以上ずれていたら、NULLで一気に補完
							BYTE* dummy = new BYTE[(int)diff*48*m_achannel];			// 音声フレームが足りていない場合は、NULLで一気に補完するように変更
							ZeroMemory(dummy,(int)diff*48*m_achannel);

							CMediaData *m = new CMediaData(dummy, (int)diff*48*m_achannel);
							delete [] dummy;

							if(m_Useffmpg) m_pAudioBand->InputMedia(m);
							else m_WaveWriter.InputMedia(m);

							delete m;

							m_AacConverter.m_total_frame += diff*48;
						} else {														// 200ms未満だったら、徐々に補完
							m_AacConverter.SetHoseiPol((LONG)4);				
							m_AudHoseiFromVid = 1;
						}

					} else if (diff <= -70){			// -2 frame以上ずれていたら
						TRACE((LPCTSTR)L"diff : %dms\n",(LONG)diff);			// 音声フレームが先走っている場合は、徐々にずらしていく(カットするとノイズになるため)
						m_AacConverter.SetHoseiPol((LONG)-4);				
						m_AudHoseiFromVid = -1;
					} else if (m_AudHoseiFromVid < 0){	// 
						TRACE((LPCTSTR)L"diff : %dms\n",(LONG)diff);
						m_AacConverter.SetHoseiPol((LONG)-4);				
						m_AudHoseiFromVid = -1;
					} else if (m_AudHoseiFromVid > 0){
						TRACE((LPCTSTR)L"diff : %dms\n",(LONG)diff);
						m_AacConverter.SetHoseiPol((LONG)4);				
						m_AudHoseiFromVid = 1;
					}
				}

				m_AacConverter.ResetPesPerFrame();

				}
				break;
		}
	} else if(pDecoder == &m_FileReader){
		DWORD ExitCode=0;
		ULONGLONG fs = 0;
		
		// ファイルリーダからのイベント
		switch(dwEventID){
			case CFileReader::EID_READ_ASYNC_START :
				// 非同期リード開始
				m_pEventHandler->OnTsConverterStart(m_FileReader.GetFileSize());
				return 0UL;
			
			case CFileReader::EID_READ_ASYNC_END :
				// 非同期リード終了					
				fs = m_FileReader.GetFileSize();
				CloseDecoders();

				//FFMPEGの戻り値をチェック
				if(m_dwFfmpegExitCode==0)
					m_pEventHandler->OnTsConverterEnd(fs);
				else
					//FFMPEGでエラーが発生した場合は0を返す
					m_pEventHandler->OnTsConverterEnd(0);

//				m_WaveSend.Close();
//				m_VideoSend.Close();
//				CloseFFmpeg();
				return 0UL;
			
			case CFileReader::EID_READ_ASYNC_POSTREAD :
				// 非同期リード後
				m_pEventHandler->OnTsConverterProgress(m_FileReader.GetReadPos(), m_FileReader.GetFileSize());
				return 0UL;
			}		
		}

	
	
	return 0UL;

}
