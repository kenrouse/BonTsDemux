// TsDescrambler.cpp: CTsDescrambler クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsserviceselect.h"
#include "time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTsServiceSelect::CTsServiceSelect() : CMediaDecoder(NULL)
	, m_dwInputPacketCount(0)
	, m_dwOutputPacketCount(0)
	, m_ServiceId(0)
	, m_lastPatTime(0)
{
}

CTsServiceSelect::~CTsServiceSelect()
{
}

void CTsServiceSelect::Reset(void)
{
	// 内部状態を初期化する
	m_PatTable.Reset();
	m_PmtList.clear();
	m_lastPatTable.Reset();
	m_lastPmtList.clear();

	// 統計データ初期化
	m_dwInputPacketCount = 0;
	m_dwOutputPacketCount = 0;

	// 下流デコーダを初期化する
	CMediaDecoder::Reset();
}

void CTsServiceSelect::SetServiceId(int ServiceId)
{
	m_ServiceId = ServiceId;
}

const DWORD CTsServiceSelect::GetInputNum(void) const
{
	return 1;
}

const DWORD CTsServiceSelect::GetOutputNum(void) const
{
	return 1;
}

const bool CTsServiceSelect::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);
	bool bPlainPacket = true;

	// 入力パケット数カウント
	if(m_dwInputPacketCount < 0xFFFFFFFF)m_dwInputPacketCount++;

	// PIDルーティング
	if(!ProcPatPacket(pTsPacket)){
		if(!ProcPmtPacket(pTsPacket)){
			if(!ProcEcmPacket(pTsPacket)){
				if(!IsEpgData(pTsPacket->GetPID())) {	// NIT,EIT,TOT等は残しておく
					WORD wIndex;
					for(wIndex = 0 ; wIndex < m_PatTable.GetProgramNum() ; wIndex++){
						// 先頭ストリームがMPEG2以外は破棄
						if(wIndex < m_PmtList.size()){
							if(m_PmtList[wIndex].PmtTable.GetStreamTypeID(0) == 0x02) {
								if(m_PmtList[wIndex].PmtTable.GetPcrPID() == pTsPacket->GetPID())
									break;
								WORD wEIndex;
								for(wEIndex = 0;wEIndex < m_PmtList[wIndex].PmtTable.GetEsInfoNum();wEIndex++) {
									if(m_PmtList[wIndex].PmtTable.GetEsPID(wEIndex) == pTsPacket->GetPID()) {
										// MPEG2,AAC,字幕以外は破棄
										if(m_PmtList[wIndex].PmtTable.GetStreamTypeID(wEIndex) == 0x0d
											|| m_PmtList[wIndex].PmtTable.GetStreamTypeID(wEIndex) == 0x06		// ISO/IEC 13818-1 PES packets containing private data 
											)
											bPlainPacket = false;
										break;
									}
								}
								if(wEIndex != m_PmtList[wIndex].PmtTable.GetEsInfoNum())
									break;
							}
						}
					}
					if(wIndex == m_PatTable.GetProgramNum())
						bPlainPacket = false;
					if(m_lastPatTime > time(NULL) && !bPlainPacket) {
						bPlainPacket = true;
						for(wIndex = 0 ; wIndex < m_lastPatTable.GetProgramNum() ; wIndex++){
							// 先頭ストリームがMPEG2以外は破棄
							if(wIndex < m_lastPmtList.size()){
								if(m_lastPmtList[wIndex].PmtTable.GetStreamTypeID(0) == 0x02) {
									if(m_lastPmtList[wIndex].PmtTable.GetPcrPID() == pTsPacket->GetPID())
										break;
									WORD wEIndex;
									for(wEIndex = 0;wEIndex < m_lastPmtList[wIndex].PmtTable.GetEsInfoNum();wEIndex++) {
										if(m_lastPmtList[wIndex].PmtTable.GetEsPID(wEIndex) == pTsPacket->GetPID()) {
											// MPEG2,AAC,字幕以外は破棄
											if(m_lastPmtList[wIndex].PmtTable.GetStreamTypeID(wEIndex) == 0x0d
												|| m_PmtList[wIndex].PmtTable.GetStreamTypeID(wEIndex) == 0x06		// ISO/IEC 13818-1 PES packets containing private data 
												)
												bPlainPacket = false;
											break;
										}
									}
									if(wEIndex != m_lastPmtList[wIndex].PmtTable.GetEsInfoNum())
										break;
								}
							}
						}
						if(wIndex == m_lastPatTable.GetProgramNum())
							bPlainPacket = false;
					}
				}
			}
		}
	}

	// PATを書き換える
	if(pTsPacket->GetPID() == 0 && m_ServiceId != 0) {
		BYTE newhead[188];
		memset(newhead,0xff,188);
		memcpy(newhead,pTsPacket->GetData(),13);
		newhead[6] = 0xb0;
		newhead[7] = 17;
		newhead[13] = newhead[14] = 0;
		newhead[15] = 0xe0;
		newhead[16] = 0x10;
		newhead[17] = (m_ServiceId >> 8);
		newhead[18] = (m_ServiceId & 0xff);
		newhead[19] = (m_wPmtPID >> 8) | 0xe0;
		newhead[20] = (m_wPmtPID & 0xff);
		newhead[21] = newhead[22] = newhead[23] = newhead[24] = 0;
		DWORD crc = crc32(&newhead[5],&newhead[21]);
		newhead[21] = (BYTE)(crc>>24)&0xff;
		newhead[22] = (BYTE)(crc>>16)&0xff;
		newhead[23] = (BYTE)(crc>>8)&0xff;
		newhead[24] = (BYTE)(crc)&0xff;
		pTsPacket->SetData(newhead,188);
	}
	if(bPlainPacket){
		// 出力パケット数カウント
		if(m_dwOutputPacketCount < 0xFFFFFFFF)m_dwOutputPacketCount++;

		// 平分パケットを下流デコーダにデータを渡す
		OutputMedia(pMediaData);
		}
	else{
		// 解除不可能の場合はパケットを破棄する
		pMediaData->ClearSize();
		}

	return true;
}

DWORD CTsServiceSelect::crc32(BYTE *head, BYTE *tail)
{
	DWORD crc;
	BYTE *p;

	static const DWORD table[256] = {
		0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
		0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
		0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 
		0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
		
		0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
		0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75, 
		0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
		0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
		
		0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
		0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
		0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 
		0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
		
		0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
		0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
		0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 
		0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,

		0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
		0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072, 
		0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
		0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,

		0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 
		0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
		0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
		0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
		
		0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
		0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
		0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 
		0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,

		0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
		0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 
		0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
		0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,

		0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 
		0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
		0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
		0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53, 
		
		0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
		0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
		0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
		0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,

		0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
		0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B, 
		0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
		0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
		
		0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 
		0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
		0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
		0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3, 
		
		0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
		0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
		0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
		0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,

		0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
		0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
		0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
		0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,

		0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
		0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
		0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
		0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 

		0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
		0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
		0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 
		0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
	};
	
	crc = 0xffffffff;

	p = head;
	while(p < tail){
		crc = (crc << 8) ^ table[ ((crc >> 24) ^ p[0]) & 0xff ];
		p += 1;
	}

	return crc;
}

bool CTsServiceSelect::IsEpgData(WORD wPID)
{
	if(wPID == 0x10)
		return true;
	if(wPID == 0x11)
		return true;
	if(wPID == 0x12)
		return true;
	if(wPID == 0x14)
		return true;
	return false;
}

const DWORD CTsServiceSelect::GetInputPacketCount(void) const
{
	// 入力パケット数を返す
	return m_dwInputPacketCount;
}

const DWORD CTsServiceSelect::GetOutputPacketCount(void) const
{
	// 出力パケット数を返す
	return m_dwOutputPacketCount;
}

const bool CTsServiceSelect::ProcPatPacket(const CTsPacket *pTsPacket)
{
	if(pTsPacket->m_Header.bTransportErrorIndicator) return true;

	// PATのパケット処理する
	if(pTsPacket->GetPID() != 0x0000)return false;

	// PATテーブル構築
	if(!m_PatTable.StorePacket(pTsPacket))return true;

	// テーブルが更新された、ECM/PMTリストを更新する(本当は差分のみの更新の方がいい？)
	if(m_PmtList.size() > 0) {
		m_lastPatTable = m_PatTable;
		m_lastPmtList = m_PmtList;
		m_lastPatTime = time(NULL) + 2;
	}
	m_PmtList.clear();
	m_PmtList.resize(m_PatTable.GetProgramNum());

	for(WORD wIndex = 0 ; wIndex < m_PatTable.GetProgramNum(); wIndex++){
		// PMT PIDセット
		if(m_ServiceId == 0 || m_PatTable.GetProgramID(wIndex) == m_ServiceId) {
			m_PmtList[wIndex].wPmtPID = m_PatTable.GetPmtPID(wIndex);
			m_wPmtPID = m_PmtList[wIndex].wPmtPID;
		}
	}
		
	return true;
}

DWORD CTsServiceSelect::GetAudioEs(DWORD service,WORD* video,WORD* audio)
{
	int num = m_PmtList[0].PmtTable.GetEsInfoNum();
	for(int i = 0 ; i < num ; i ++){
		if(m_PmtList[0].PmtTable.GetStreamTypeID(i) == 0x0f){
			if(audio) *audio = m_PmtList[0].PmtTable.GetEsPID(i);
			if(service == 0) break;
			service --;
		}
	}
	
	return 0;
}

//++ 2010.03.04 added by pika
//とりあえずこうすれば
//全サービスが保存されたTSの２番目以降のサービスの音声をES指定有りでも無しでも
//取り出すことができる
DWORD CTsServiceSelect::GetAudioEs2(int serviceID, DWORD service,WORD* video,WORD* audio)
{
	if(audio)
	{
		for(WORD wIndex = 0 ; wIndex < m_PatTable.GetProgramNum(); wIndex++){
			// PMT PIDセット
			if(m_PatTable.GetProgramID(wIndex) == serviceID) {
				int num = m_PmtList[wIndex].PmtTable.GetEsInfoNum();
				for(int i = 0 ; i < num ; i ++){
					if(m_PmtList[wIndex].PmtTable.GetStreamTypeID(i) == 0x0f){
						*audio = m_PmtList[wIndex].PmtTable.GetEsPID(i);
						if(service == 0) break;
						service --;
					}
				}
			}
		}
	}
	
	return 0;
}
//--

const bool CTsServiceSelect::ProcPmtPacket(const CTsPacket *pTsPacket)
{
	// PMTのパケットを処理する
	for(WORD wIndex = 0 ; wIndex < m_PatTable.GetProgramNum() ; wIndex++){
		if(pTsPacket->GetPID() == m_PmtList[wIndex].wPmtPID){
			if(m_PmtList[wIndex].PmtTable.StorePacket(pTsPacket)){
				// テーブルが更新された、ECMリストを更新する
				OnPmtUpdate(m_PmtList[wIndex].PmtTable);
				}
			return true;
		}		
	}
	if(m_lastPatTime > time(NULL)) {
		for(WORD wIndex = 0 ; wIndex < m_lastPatTable.GetProgramNum() ; wIndex++){
			if(wIndex < m_lastPmtList.size()){
				if(pTsPacket->GetPID() == m_lastPmtList[wIndex].wPmtPID){
					return true;
				}		
			}
		}
	}
	return false;
}

const bool CTsServiceSelect::ProcEcmPacket(const CTsPacket *pTsPacket)
{
	// ECMのパケットを処理する
	for(m_wEcmSetIndex = 0 ; m_wEcmSetIndex < m_EcmList.size() ; m_wEcmSetIndex++){
		if(pTsPacket->GetPID() == m_EcmList[m_wEcmSetIndex].wEcmPID){
			return true;
			}		
		}

	return false;
}

void CTsServiceSelect::OnPmtUpdate(const CPmtTable &PmtTable)
{
	const WORD wEcmPID = PmtTable.GetEcmPID();

	if(wEcmPID >= 0x1FFF)return;

	// 既存のECMリストを検索して、なければ追加する
	for(WORD wIndex = 0 ; wIndex < m_EcmList.size() ; wIndex++){
		if(wEcmPID == m_EcmList[wIndex].wEcmPID)return;
		}

	TAG_ECMSET NewEcmSet;
	NewEcmSet.wEcmPID = wEcmPID;
	m_EcmList.push_back(NewEcmSet);
}
