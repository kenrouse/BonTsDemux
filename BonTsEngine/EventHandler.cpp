#include "stdafx.h"
#include "EventHandler.h"

CEcmDat* m_pEcm;		// �Ƃ肠�����B�B

CEventHandler::CEventHandler(void)
{
	m_pTsPacketParser = NULL;
	m_pTsDescrambler = NULL;
	m_pTsServiceSelect = NULL;
	m_pFileWriter = NULL;
	m_pNetworkSend = NULL;
	m_pHttpSend = NULL;
	m_pEcm = NULL;
	m_pKeepRate = NULL;
	m_pProgManager = NULL;
	m_pTsDemuxer = NULL;
	m_pVideoFile = NULL;
	m_pAudioFile = NULL;
	m_pFFmpgVideoBand = NULL;
	m_pFFmpgVideo = NULL;
	m_pFFmpgAudioBand = NULL;
	m_pFFmpgAudio = NULL;
	m_pWaveWriter = NULL;
	m_pAacConverter = NULL;

	m_AudioLpfBuf = 0;
	m_VideoLpfBuf = 0;
	m_emergency = FALSE;

	m_server_port = 1234;
}

CEventHandler::~CEventHandler(void)
{
	Exit();
}

extern bool OpenFFmpeg(LPSTR vlc);

static const BYTE WaveHead[] =		// from CWaveWriter@BonTsDemux
{
	'R', 'I', 'F', 'F',				// +0	RIFF
	0x00U, 0x00U, 0x00U, 0x00U,		// +4	����ȍ~�̃t�@�C���T�C�Y(�t�@�C���T�C�Y - 8)
	'W', 'A', 'V', 'E',				// +8	WAVE
	'f', 'm', 't', ' ',				// +12	fmt
	0x10U, 0x00U, 0x00U, 0x00U,		// +16	fmt �`�����N�̃o�C�g��
	0x01U, 0x00U,					// +18	�t�H�[�}�b�gID
	0x02U, 0x00U,					// +20	�X�e���I
	0x80U, 0xBBU, 0x00U, 0x00U,		// +24	48KHz
	0x00U, 0xEEU, 0x02U, 0x00U,		// +28	192000Byte/s
	0x04U, 0x00U,					// +30	�u���b�N�T�C�Y
	0x10U, 0x00U,					// +32	�T���v��������̃r�b�g��
	'd', 'a', 't', 'a',				// +36	data
	0x00U, 0xffU, 0xffU, 0xffU		// +40	�g�`�f�[�^�̃o�C�g��
};


BOOL CEventHandler::Init(DWORD opt,LPCTSTR tsfname,DWORD PacketSize,DWORD buf_margin,DWORD buf_size,DWORD service)
{

	CMediaDecoder* connect;

	m_tsfname = (char*)tsfname;


	m_pEcm = new CEcmDat();
	m_pEcm->SetDiag(OPT_BDIAG & opt ? TRUE : FALSE);


	m_pKeepRate = new CKeepRate(PacketSize,							// �p�P�b�g��
							buf_margin,								// ���o�C�g���߂Ă���f�R�[�h�J�n���邩
							buf_size, 								// �o�b�t�@��MAX�T�C�Y
							( (opt&(OPT_UDP|OPT_FFMPEG)) ? 30*1024*1024 : 0)		// �ő�u�ԕ������A30Mbps�ɗ}����
							,FALSE									// copy data
							,"TS"									// name
							,this
							);		

	m_pTsPacketParser = new CTsPacketParser(this);
	m_pKeepRate->SetOutputDecoder(m_pTsPacketParser);			// m_pKeepRate->m_pTsPacketParser
	connect = m_pTsPacketParser;

	if(opt&OPT_SEL) {
		m_pTsServiceSelect = new CTsServiceSelect();
		m_pTsServiceSelect->SetServiceId(service);
		
		connect->SetOutputDecoder(m_pTsServiceSelect);			// ->m_pTsServiceSelect
		connect = m_pTsServiceSelect;
	}

	if(opt&OPT_B25) {
		m_pTsDescrambler = new CTsDescrambler(this);
		if(m_pTsDescrambler->OpenBcasCard()){
			connect->SetOutputDecoder(m_pTsDescrambler);		// ->m_pTsDescrambler
			connect = m_pTsDescrambler;
		}
	}

	if(opt&OPT_UDP) {			// UDP�o�͑I��
		m_pNetworkSend = new CNetworkSend(32767);
		m_pNetworkSend->Open("localhost",m_server_port);
		connect->SetOutputDecoder(m_pNetworkSend);				// ->m_pNetworkSend
		connect = m_pNetworkSend;

	} else if(opt&OPT_HTTP) {			// HTTP�o�͑I��
		m_pHttpSend = new CHttpSend(this,m_server_port,8*1024);
		connect->SetOutputDecoder(m_pHttpSend);					// ->m_pHttpSend
//		m_pHttpSend->RawTcp(TRUE);
		connect = m_pHttpSend;

	} else if (opt&(OPT_DEMUX|OPT_FFMPEG)) {	// demux | ffmpeg�o�͑I��(TEST)

		m_pProgManager = new CProgManager(this);
		connect->SetOutputDecoder(m_pProgManager);				// ->m_pProgManager

		m_pTsDemuxer = new CTsDemuxer(this);
		m_pTsDemuxer->EnableLipSync(TRUE);
		m_pProgManager->SetOutputDecoder(m_pTsDemuxer);			// ->m_pTsDemuxer


		if (opt&OPT_DEMUX){						// demux�o��
			char file_name[_MAX_PATH];
			
			sprintf_s(file_name,"%s.m2v",tsfname);
			m_pVideoFile = new CFileWriter(this);
			if(m_pVideoFile->OpenFile(file_name)){
				m_pTsDemuxer->SetOutputDecoder(m_pVideoFile,	// 		->m_pVideoFile
								CTsDemuxer::OUTPUT_VIDEO);
			} else {
				log_out("�t�@�C��:%s���J���܂���B",file_name);
			}
/*
				if(!m_WaveWriter.OpenFile(lpszAudioFile))throw 2UL;
				m_TsDemuxer.SetOutputDecoder(&m_AacConverter, CTsDemuxer::OUTPUT_AUDIO);
				m_AacConverter.SetOutputDecoder(&m_WaveWriter);
*/

/*
			// aac�o��
			sprintf_s(file_name,"%s.aac",tsfname);
			m_pAudioFile = new CFileWriter(this);
			if(m_pAudioFile->OpenFile(file_name)){
				m_pTsDemuxer->SetOutputDecoder(m_pAudioFile,	// 		->m_pAudioFile
								CTsDemuxer::OUTPUT_AUDIO);
			} else {
				log_out("�t�@�C��:%s���J���܂���B",file_name);
			}
*/
			// wav�o��
			sprintf_s(file_name,"%s.wav",tsfname);
			m_pAacConverter = new CAacConverter(this);
			m_pTsDemuxer->SetOutputDecoder(m_pAacConverter,	// 		->m_pAudioFile
							CTsDemuxer::OUTPUT_AUDIO);
			
			m_pWaveWriter = new CWaveWriter(this);
			if(m_pWaveWriter->OpenFile(file_name)){
				m_pAacConverter->SetOutputDecoder(m_pWaveWriter);
			} else {
				log_out("�t�@�C��:%s���J���܂���B",file_name);
			}


		} else {								// FFmpeg�o��


			m_pFFmpgVideo = new CHttpSend(this,m_server_port,8*1024);
			m_pFFmpgVideoBand = new CKeepRate(64*1024,			//packet
												0,				//margin
												20*1024*1024,	//max
												0,	//max_bps
												TRUE,			//copydata
												"M2V",			//name
												this);	//VIDEO�p�N�b�V�����o�b�t�@���
			m_pTsDemuxer->SetOutputDecoder(m_pFFmpgVideoBand,		//		->m_pFFmpgVideo
							CTsDemuxer::OUTPUT_VIDEO);
			m_pFFmpgVideoBand->SetOutputDecoder(m_pFFmpgVideo);
			m_pFFmpgVideoBand->m_debugmsg = FALSE;

/*
			// aac�o��
			m_pFFmpgAudioBand = new CKeepRate(1024,	//packet
												0,				//margin
												2*1024*1024,	//max
												256*1024,		//max_bps
												TRUE,			//copydata
												"AAC",			//name
												this);	//Audio�p�N�b�V�����o�b�t�@���
			m_pFFmpgAudioBand->m_debugmsg = FALSE;
			m_pTsDemuxer->SetOutputDecoder(m_pFFmpgAudioBand,	// 		->m_pFFmpgAudio
							CTsDemuxer::OUTPUT_AUDIO);
			m_pFFmpgAudio = new CHttpSend(this,1234,8*1024);
			m_pFFmpgAudioBand->SetOutputDecoder(m_pFFmpgAudio);
*/


			// wav�o��

			m_pAacConverter = new CAacConverter(this);
			m_pTsDemuxer->SetOutputDecoder(m_pAacConverter,	// 		->m_pFFmpgAudio
							CTsDemuxer::OUTPUT_AUDIO);

			m_pFFmpgAudioBand = new CKeepRate(1024,		//packet
												0,		//margin
												5*1024*1024,		//max
												0,		//max_bps
												TRUE,	//copydata
												"WAV",	//name
												this);	//Audio�p�N�b�V�����o�b�t�@���
			m_pFFmpgAudioBand->m_debugmsg = FALSE;
			m_pAacConverter->SetOutputDecoder(m_pFFmpgAudioBand);


			m_pFFmpgAudio = new CHttpSend(this,m_server_port,8*1024);

			CMediaData *m = new CMediaData(WaveHead, (DWORD)sizeof(WaveHead));
			m_pFFmpgAudio->InputMedia(m);
			delete m;

//			m_pFFmpgAudio->SetHeader(WaveHead,sizeof(WaveHead));

			m_pFFmpgAudioBand->SetOutputDecoder(m_pFFmpgAudio);

#if 0
/*
			FILE* f;
			size_t len;
			BYTE inbuf33[512*1024];
			Sleep(300);

			OpenFFmpeg("C:\\Program Files\\�K���v���O����\\cap_sts\\ffmpeg.exe -i http://localhost:1234/ -acodec libmp3lame  -ar 48000 -ab 192k -y out.mpg");


			f = fopen("C:\\Program Files\\�K���v���O����\\cap_sts\\test.m2v","rb");
*/
			FILE* f;
			size_t len;
			BYTE inbuf33[8*1024];

			Sleep(1000);

			OpenFFmpeg("C:\\Program Files\\�K���v���O����\\cap_sts\\ffmpeg.exe -i \"http://localhost:1234/test.wav\" -acodec libmp3lame  -ar 48000 -ab 192k -y out.mpg");

			f = fopen("C:\\Program Files\\�K���v���O����\\cap_sts\\test.wav","rb");
			while(len=fread(inbuf33,sizeof(BYTE),sizeof(inbuf33),f)){

				CMediaData *m = new CMediaData(inbuf33, (DWORD)len);
				m_pFFmpgAudio->InputMedia(m);
				delete m;
//				m = new CMediaData(inbuf33, (DWORD)len);
//				m_pFFmpgAudioBand->InputMedia(m);
			}
			fclose(f);
			m_pFFmpgAudio->Close();
			delete m_pFFmpgAudio;
#endif
		}
	} else {				//�t�@�C���o�͑I��
		m_pFileWriter = new CFileWriter(this);
		if(m_pFileWriter->OpenFile(tsfname)){
			connect->SetOutputDecoder(m_pFileWriter);			// ->m_pFileWriter
			connect = m_pFileWriter;
		} else {
			log_out("�t�@�C��:%s���J���܂���B",tsfname);
		}
	}
	return TRUE;
}

BOOL CEventHandler::CheckServerConnection(void)
{
	BOOL ret = TRUE;
	
	if ( m_pFFmpgVideo && m_pFFmpgVideo->IsOpen() == FALSE){
		ret = FALSE;
	}
	if ( m_pHttpSend && m_pHttpSend->IsOpen() == FALSE){
		ret = FALSE;
	}

	return ret;
}

void CEventHandler::Exit(void)
{
	if(m_pEcm)	m_pEcm->exit();

	if(m_pKeepRate)				m_pKeepRate->StopTrans();
	if(m_pFFmpgAudioBand)		m_pFFmpgAudioBand->StopTrans();
	if(m_pFFmpgVideoBand)		m_pFFmpgVideoBand->StopTrans();
	if(m_pFFmpgVideo)			m_pFFmpgVideo->Close();
	if(m_pFFmpgAudio)			m_pFFmpgAudio->Close();


	if(m_pKeepRate)				delete m_pKeepRate;
	if(m_pFFmpgVideoBand)		delete m_pFFmpgVideoBand;
	if(m_pFFmpgAudioBand)		delete m_pFFmpgAudioBand;


	if(m_pTsPacketParser)		delete m_pTsPacketParser;
	if(m_pTsServiceSelect)		delete m_pTsServiceSelect;
	if(m_pTsDescrambler)		delete m_pTsDescrambler;
	if(m_pNetworkSend)			delete m_pNetworkSend;
	if(m_pHttpSend)				delete m_pHttpSend;
	if(m_pFileWriter)			delete m_pFileWriter;
	if(m_pEcm)					delete m_pEcm;
	if(m_pProgManager)			delete m_pProgManager;
	if(m_pTsDemuxer)			delete m_pTsDemuxer;
	if(m_pVideoFile)			delete m_pVideoFile;
	if(m_pAudioFile)			delete m_pAudioFile;
	if(m_pFFmpgVideo)			delete m_pFFmpgVideo;
	if(m_pFFmpgAudio)			delete m_pFFmpgAudio;
	if(m_pWaveWriter)			delete m_pWaveWriter;
	if(m_pAacConverter)			delete m_pAacConverter;

	m_pTsPacketParser = NULL;
	m_pTsDescrambler = NULL;
	m_pTsServiceSelect = NULL;
	m_pFileWriter = NULL;
	m_pNetworkSend = NULL;
	m_pHttpSend = NULL;
	m_pEcm = NULL;
	m_pKeepRate = NULL;
	m_pProgManager = NULL;
	m_pTsDemuxer = NULL;
	m_pVideoFile = NULL;
	m_pAudioFile = NULL;
	m_pFFmpgVideoBand = NULL;
	m_pFFmpgVideo = NULL;
	m_pFFmpgAudioBand = NULL;
	m_pFFmpgAudio = NULL;
	m_pWaveWriter = NULL;
	m_pAacConverter = NULL;
}

BOOL CEventHandler::OnTsData(BYTE* data,DWORD len)
{
	if(m_emergency) return FALSE;
	return m_pKeepRate->InputMedia(new CMediaData(data, len));
}

BOOL CEventHandler::OnBcasData(BYTE* data,DWORD len)
{
	if(m_emergency) return FALSE;
	m_pEcm->set(data,len);
	return TRUE;
}

void CEventHandler::Reset(void)
{
}

const DWORD CEventHandler::OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	// �f�R�[�_����̃C�x���g���󂯎��(�b��)
	if(pDecoder == m_pProgManager){
		
		// �v���O�����}�l�[�W������̃C�x���g
		switch(dwEventID){
			case CProgManager::EID_SERVICE_LIST_UPDATED : {
				// �T�[�r�X�̍\�����ω�����
				WORD wVideoPID = 0xFFFF;
				WORD wAudioPID = 0xFFFF;
				m_pProgManager->GetServiceEsPID(&wVideoPID, &wAudioPID);
				m_pTsDemuxer->SetVideoPID(wVideoPID);
				m_pTsDemuxer->SetAudioPID(wAudioPID);
				return 0UL;
			}
			
			case CProgManager::EID_SERVICE_INFO_UPDATED : {
				// �T�[�r�X��񂪍X�V���ꂽ
				TCHAR szServiceName[1024] = {TEXT("�s��")};
				m_pProgManager->GetServiceName(szServiceName);
				return 0UL;
			}
		}
	} else if(pDecoder == m_pTsDemuxer){
		double diff;
	
		switch(dwEventID){
		
			case CTsDemuxer::EID_SERVICE_FIRST_AAC:					// ������ςȕ⊮�B��_��VIDEO��PTS�ɂ��킹�邽�߁A�����𖄂߂�
				{
					LONG hokan;
					diff = (double)m_pTsDemuxer->GetM2VAACDif();
					diff = diff * 48 * 4 / 90;
					hokan = (LONG)diff;
					hokan -= (hokan % 4);
					if(hokan > 0){
						BYTE* dummy = new BYTE[hokan];
						ZeroMemory(dummy,hokan);

						CMediaData *m = new CMediaData(dummy, hokan);
						delete [] dummy;

						m_pFFmpgAudioBand->InputMedia(m);
						delete m;
					} else {
						m_pTsDemuxer->m_FirstAudioPts = m_pTsDemuxer->m_FirstVideoPts;		// ���炩�̌����ŁAVideo���������Ă���B
					}
				}

				break;
			case CTsDemuxer::EID_SERVICE_M2V_COMMIT:
//				log_out("M2V	%I64d	%I64d	%d\n",m_pTsDemuxer->m_NowVideoPts,m_pTsDemuxer->m_NowAudioPts,m_pAacConverter->GetTotalSample());
				{
					LONGLONG pts_ms = m_pTsDemuxer->GetTotalVideoPts();
					LONGLONG frame_ms = m_pTsDemuxer->GetVideoTotalFrame() * 1001 * 90 / 30;		// * 1000 / 29.97(fps)  �� ms

					diff = (double)(pts_ms - frame_ms);

//					TRACE2("frame_ms:%I64d   , pts_ms:%I64d\n",frame_ms,pts_ms);

//					m_VideoLpfBuf = 0.05 * diff + 0.95 * m_VideoLpfBuf;		// PES�Ǝ��f�[�^�̍�����LPF�ɒʂ��B�W���͓K���B�B
					m_VideoLpfBuf = diff;

					if(diff){
						m_pTsDemuxer->m_FirstAudioPts += (LONGLONG)m_VideoLpfBuf;		// ���炩�̌����ŁAVideo���������Ă���B
						m_pTsDemuxer->m_FirstVideoPts += (LONGLONG)m_VideoLpfBuf;		// �␳����B
						TRACE3("frame_ms:%I64d   , pts_ms:%I64d Add:%I64d\n",frame_ms,pts_ms,m_VideoLpfBuf);
					}
				}
				break;
			case CTsDemuxer::EID_SERVICE_AAC_COMMIT:
				{

				if(m_pAacConverter->GetTotalSample() < 100000) break;

//				log_out("AAC	%I64d	%I64d	%d\n",m_pTsDemuxer->m_NowVideoPts,m_pTsDemuxer->m_NowAudioPts,m_pAacConverter->GetTotalSample());

				diff = (double)(m_pTsDemuxer->GetTotalAudioPts() / 90 - m_pAacConverter->GetTotalSample() / 48);	// PES 90kHz Audio 48kHz �炵���̂ŁA�P�ʂ�ms�ɂ��킹�č������Ƃ�B

//				m_AudioLpfBuf = 0.05 * diff + 0.95 * m_AudioLpfBuf;		// PES�Ǝ��f�[�^�̍�����LPF�ɒʂ��B�W���͓K���B�B
				m_AudioLpfBuf = diff;

				if (m_AudioLpfBuf > 2000 || m_AudioLpfBuf < -2000){		// �}2sec�ȏ�́A��肷���Ȃ̂ŋ��炭�G���[���������Ă���
					m_AudioLpfBuf = 0;
					m_pAacConverter->ResetTotalFrame();
					m_pTsDemuxer->ResetTotalAudioPts();
					m_pTsDemuxer->EnableLipSync(TRUE);
				} else {
					if(m_AudioLpfBuf > 1){					// 1ms�ȏジ��Ă�����
						TRACE2("AudioPes:%I64d   , TotalSample:%I64d\n",m_pTsDemuxer->GetTotalAudioPts(),m_pAacConverter->GetTotalSample());
						TRACE1("diff : %dms\n",(LONG)m_AudioLpfBuf);
						m_pAacConverter->SetHoseiPol((LONG)4);				
					} else if (m_AudioLpfBuf < -1){			// -1ms�ȏジ��Ă�����
						TRACE2("AudioPes:%I64d   , TotalSample:%I64d\n",m_pTsDemuxer->GetTotalAudioPts(),m_pAacConverter->GetTotalSample());
						TRACE1("diff : %dms\n",(LONG)m_AudioLpfBuf);
						m_pAacConverter->SetHoseiPol((LONG)-4);				
					} else {
						m_pAacConverter->SetHoseiPol(0);				
					}
				}

				m_pAacConverter->ResetPesPerFrame();

				}
				break;
		}
	} else {
		switch(dwEventID){
			case CHttpSend::EID_ERROR_SOCKET:
				m_emergency = TRUE;
				break;
		}
			
	}

	
	
	return 0UL;
}
