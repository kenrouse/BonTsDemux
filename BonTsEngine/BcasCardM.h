// BcasCard.h: CBcasCardM クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <winscard.h>


using std::vector;
using std::wstring;
using std::string;


// エラーコード
#define BCEC_NOERROR			0x00000000	// エラーなし
#define BCEC_INTERNALERROR		0x00000001	// 内部エラー
#define BCEC_NOTESTABLISHED		0x00000002	// コンテキスト確立失敗
#define BCEC_NOCARDREADERS		0x00000003	// カードリーダがない
#define BCEC_ALREADYOPEN		0x00000004	// 既にオープン済み
#define BCEC_CARDOPENERROR		0x00000005	// カードオープン失敗
#define BCEC_CARDNOTOPEN		0x00000006	// カード未オープン
#define BCEC_TRANSMITERROR		0x00000007	// 通信エラー
#define BCEC_BADARGUMENT		0x00000008	// 引数が不正
#define BCEC_ECMREFUSED			0x00000009	// ECM受付拒否


class CBcasCardM
{
public:
	CBcasCardM();
	~CBcasCardM();

	const DWORD GetCardReaderNum(void) const;
	LPCTSTR GetCardReaderName(const DWORD dwIndex = 0) const;

	const bool OpenCard(LPCTSTR lpszReader = NULL);
	void CloseCard(void);

	const BYTE * GetBcasCardID(void);
	const BYTE * GetInitialCbc(void);
	const BYTE * GetSystemKey(void);
	const BYTE * GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize);

	const DWORD GetLastError(void) const;

protected:
	const bool EnumCardReader(void);
	const bool TransmitCommand(const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize = NULL);
	const bool InitialSetting(void);

	SCARDCONTEXT m_ScardContext;
	SCARDHANDLE m_hBcasCard;

	bool m_bIsEstablish;

#ifdef _UNICODE	
	vector<wstring> m_CardReaderArray;
#else
	vector<string> m_CardReaderArray;
#endif

	struct TAG_BCASCARDINFO
	{
		BYTE BcasCardID[6];		// Card ID
		BYTE SystemKey[32];		// Descrambling system key
		BYTE InitialCbc[8];		// Descrambler CBC initial value
	} m_BcasCardInfo;
	
	struct TAG_ECMSTATUS
	{
		DWORD dwLastEcmSize;	// 最後に問い合わせのあったECMサイズ
		BYTE LastEcmData[256];	// 最後に問い合わせのあったECMデータ
		BYTE KsData[16];		// Ks Odd + Even	
	} m_EcmStatus;
	
	DWORD m_dwLastError;

private:
	const bool StoreEcmData(const BYTE *pEcmData, const DWORD dwEcmSize);
};
