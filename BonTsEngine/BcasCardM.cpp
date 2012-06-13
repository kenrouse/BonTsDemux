// BcasCard.cpp: CBcasCard クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <memory>
#include "BcasCardM.h"
#include "EcmDat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#pragma comment(lib, "WinScard.lib")


using std::auto_ptr;


CBcasCardM::CBcasCardM()
	: m_hBcasCard(NULL)
	, m_bIsEstablish(false)
	, m_dwLastError(BCEC_NOERROR)
{
	// 内部状態初期化
	::ZeroMemory(&m_BcasCardInfo, sizeof(m_BcasCardInfo));
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	m_bIsEstablish = true;
		
	// カードリーダ列挙
	EnumCardReader();

}

CBcasCardM::~CBcasCardM()
{
	CloseCard();

}

const DWORD CBcasCardM::GetCardReaderNum(void) const
{
	// カードリーダー数を返す
	return 1;
}

LPCTSTR CBcasCardM::GetCardReaderName(const DWORD dwIndex) const
{
	// カードリーダー名を返す
	return (LPCTSTR)"Hitachi/Maxell M-500U/M-520U 0";
}

const bool CBcasCardM::OpenCard(LPCTSTR lpszReader)
{
	// リソースマネージャコンテキストの確立
	if(!m_bIsEstablish){
		m_dwLastError = BCEC_NOTESTABLISHED;
		return false;
		}
	
	// 一旦クローズする
	CloseCard();


	if(lpszReader){
		// 指定されたカードリーダに対してオープンを試みる
		DWORD dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
		m_hBcasCard = 1;
		}
	else{
		// 全てのカードリーダに対してオープンを試みる
		DWORD dwIndex = 0;
	
		while(GetCardReaderName(dwIndex)){
			if(OpenCard(GetCardReaderName(dwIndex++)))return true;			
			}
		
		return false;
		}

	// カード初期化
	if(!InitialSetting())return false;

	m_dwLastError = BCEC_NOERROR;

	return true;
}

void CBcasCardM::CloseCard(void)
{
	// カードをクローズする
}

const DWORD CBcasCardM::GetLastError(void) const
{
	// 最後に発生したエラーを返す
	return m_dwLastError;
}

const bool CBcasCardM::EnumCardReader(void)
{

	m_dwLastError = BCEC_NOERROR;

	return true;
}

const bool CBcasCardM::TransmitCommand(const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize)
{

	return true;
}


const BYTE system_key[]={0x36,0x31,0x04,0x66,0x4B,0x17,0xEA,0x5C,0x32,0xDF,0x9C,0xF5,0xC4,0xC3,0x6C,0x1B,
						0xEC,0x99,0x39,0x21,0x68,0x9D,0x4B,0xB7,0xB7,0x4E,0x40,0x84,0x0D,0x2E,0x7D,0x98,};
const BYTE init_cbc[]={	0xFE,0x27,0x19,0x99,0x19,0x69,0x09,0x11,};

const BYTE card_id[] = {0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};

const bool CBcasCardM::InitialSetting(void)
{


	// レスポンス解析
	::CopyMemory(m_BcasCardInfo.BcasCardID, card_id, 6);	// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey, system_key, 32);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, init_cbc, 8);	// +48	Descrambler CBC initial value

	TRACE0("BcasCardID : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.BcasCardID,6);
	TRACE0("SystemKey : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.SystemKey,32);
	TRACE0("InitialCbc : ");
//	DebugDump((BYTE*)&m_BcasCardInfo.InitialCbc,8);

	// ECMステータス初期化
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}

const BYTE * CBcasCardM::GetBcasCardID(void)
{
	// Card ID を返す
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.BcasCardID;
}

const BYTE * CBcasCardM::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value を返す
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.InitialCbc;
}

const BYTE * CBcasCardM::GetSystemKey(void)
{
	// Descrambling System Key を返す
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}
	
	m_dwLastError = BCEC_NOERROR;
	
	return m_BcasCardInfo.SystemKey;
}


CEcmDat* m_pEcm;		// とりあえず。。

const BYTE * CBcasCardM::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE EcmReceiveCmd[] = {0x90, 0x34, 0x00, 0x00};

	// 「ECM Receive Command」を処理する
	if(!m_hBcasCard){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
		}

	// ECMサイズをチェック
	if(!pEcmData || (dwEcmSize < 30) || (dwEcmSize > 256)){
		m_dwLastError = BCEC_BADARGUMENT;
		return NULL;
		}

	// キャッシュをチェックする
	if(!StoreEcmData(pEcmData, dwEcmSize)){
		// ECMが同一の場合はキャッシュ済みKsを返す
		m_dwLastError = BCEC_NOERROR;
		return m_EcmStatus.KsData;
		}

	// バッファ準備
	DWORD dwRecvSize = 0;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// コマンド構築
	::CopyMemory(SendData, EcmReceiveCmd, sizeof(EcmReceiveCmd));				// CLA, INS, P1, P2
	SendData[sizeof(EcmReceiveCmd)] = (BYTE)dwEcmSize;							// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(EcmReceiveCmd) + 1], pEcmData, dwEcmSize);	// ECM
	SendData[sizeof(EcmReceiveCmd) + dwEcmSize + 1] = 0x00;						// RESPONSE DATA LENGTH

	m_pEcm->GetKsKey(SendData,sizeof(EcmReceiveCmd) + dwEcmSize + 2,RecvData);
	dwRecvSize = 25;

	// サイズチェック
	if(dwRecvSize != 25){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwLastError = BCEC_TRANSMITERROR;
		return NULL;
		}	
	
	// レスポンス解析
	::CopyMemory(m_EcmStatus.KsData, &RecvData[6], sizeof(m_EcmStatus.KsData));

	// リターンコード解析
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		// Purchased: Viewing
		case 0x0200 :	// Payment-deferred PPV
		case 0x0400 :	// Prepaid PPV
		case 0x0800 :	// Tier
			m_dwLastError = BCEC_NOERROR;
			return m_EcmStatus.KsData;
		
		// 上記以外(視聴不可)
		default :
			m_dwLastError = BCEC_ECMREFUSED;
			return NULL;
		}

}

const bool CBcasCardM::StoreEcmData(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	bool bUpdate = false;
	
	// ECMデータ比較
	if(m_EcmStatus.dwLastEcmSize != dwEcmSize){
		// サイズが変化した
		bUpdate = true;
		}
	else{
		// サイズが同じ場合はデータをチェックする
		for(DWORD dwPos = 0 ; dwPos < dwEcmSize ; dwPos++){
			if(pEcmData[dwPos] != m_EcmStatus.LastEcmData[dwPos]){
				// データが不一致
				bUpdate = true;
				break;
				}			
			}
		}

	// ECMデータを保存する
	if(bUpdate){
		m_EcmStatus.dwLastEcmSize = dwEcmSize;
		::CopyMemory(m_EcmStatus.LastEcmData, pEcmData, dwEcmSize);
		}

	return bUpdate;
}
