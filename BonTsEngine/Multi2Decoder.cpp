// Multi2Decoder.cpp: CMulti2Decoder クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Multi2Decoder.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define SCRAMBLE_ROUND		4UL


inline void CMulti2Decoder::DATKEY::SetHexData(const BYTE *pHexData)
{
	BYTE * const pHexThis = reinterpret_cast<BYTE *>(this);

	// バイトオーダー変換
	pHexThis[ 3] = pHexData[ 0];	pHexThis[ 2] = pHexData[ 1];	pHexThis[ 1] = pHexData[ 2];	pHexThis[ 0] = pHexData[ 3];
	pHexThis[ 7] = pHexData[ 4];	pHexThis[ 6] = pHexData[ 5];	pHexThis[ 5] = pHexData[ 6];	pHexThis[ 4] = pHexData[ 7];
}

inline void CMulti2Decoder::DATKEY::GetHexData(BYTE *pHexData) const
{
	const BYTE *pHexThis = reinterpret_cast<const BYTE *>(this);

	// バイトオーダー変換
	pHexData[ 0] = pHexThis[ 3];	pHexData[ 1] = pHexThis[ 2];	pHexData[ 2] = pHexThis[ 1];	pHexData[ 3] = pHexThis[ 0];
	pHexData[ 4] = pHexThis[ 7];	pHexData[ 5] = pHexThis[ 6];	pHexData[ 6] = pHexThis[ 5];	pHexData[ 7] = pHexThis[ 4];
}

inline void CMulti2Decoder::SYSKEY::SetHexData(const BYTE *pHexData)
{
	BYTE * const pHexThis = reinterpret_cast<BYTE *>(this);

	// バイトオーダー変換
	pHexThis[ 3] = pHexData[ 0];	pHexThis[ 2] = pHexData[ 1];	pHexThis[ 1] = pHexData[ 2];	pHexThis[ 0] = pHexData[ 3];
	pHexThis[ 7] = pHexData[ 4];	pHexThis[ 6] = pHexData[ 5];	pHexThis[ 5] = pHexData[ 6];	pHexThis[ 4] = pHexData[ 7];
	pHexThis[11] = pHexData[ 8];	pHexThis[10] = pHexData[ 9];	pHexThis[ 9] = pHexData[10];	pHexThis[ 8] = pHexData[11];
	pHexThis[15] = pHexData[12];	pHexThis[14] = pHexData[13];	pHexThis[13] = pHexData[14];	pHexThis[12] = pHexData[15];
	pHexThis[19] = pHexData[16];	pHexThis[18] = pHexData[17];	pHexThis[17] = pHexData[18];	pHexThis[16] = pHexData[19];
	pHexThis[23] = pHexData[20];	pHexThis[22] = pHexData[21];	pHexThis[21] = pHexData[22];	pHexThis[20] = pHexData[23];
	pHexThis[27] = pHexData[24];	pHexThis[26] = pHexData[25];	pHexThis[25] = pHexData[26];	pHexThis[24] = pHexData[27];
	pHexThis[31] = pHexData[28];	pHexThis[30] = pHexData[29];	pHexThis[29] = pHexData[30];	pHexThis[28] = pHexData[31];
}

inline void CMulti2Decoder::SYSKEY::GetHexData(BYTE *pHexData) const
{
	const BYTE *pHexThis = reinterpret_cast<const BYTE *>(this);

	// バイトオーダー変換
	pHexData[ 0] = pHexThis[ 3];	pHexData[ 1] = pHexThis[ 2];	pHexData[ 2] = pHexThis[ 1];	pHexData[ 3] = pHexThis[ 0];
	pHexData[ 4] = pHexThis[ 7];	pHexData[ 5] = pHexThis[ 6];	pHexData[ 6] = pHexThis[ 5];	pHexData[ 7] = pHexThis[ 4];
	pHexData[ 8] = pHexThis[11];	pHexData[ 9] = pHexThis[10];	pHexData[10] = pHexThis[ 9];	pHexData[11] = pHexThis[ 8];
	pHexData[12] = pHexThis[15];	pHexData[13] = pHexThis[14];	pHexData[14] = pHexThis[13];	pHexData[15] = pHexThis[12];
	pHexData[16] = pHexThis[19];	pHexData[17] = pHexThis[18];	pHexData[18] = pHexThis[17];	pHexData[19] = pHexThis[16];
	pHexData[20] = pHexThis[23];	pHexData[21] = pHexThis[22];	pHexData[22] = pHexThis[21];	pHexData[23] = pHexThis[20];
	pHexData[24] = pHexThis[27];	pHexData[25] = pHexThis[26];	pHexData[26] = pHexThis[25];	pHexData[27] = pHexThis[24];
	pHexData[28] = pHexThis[31];	pHexData[29] = pHexThis[30];	pHexData[30] = pHexThis[29];	pHexData[31] = pHexThis[28];
}

CMulti2Decoder::CMulti2Decoder(void)
	: m_bIsSysKeyValid(false)
	, m_bIsWorkKeyValid(false)
{

}

CMulti2Decoder::~CMulti2Decoder(void)
{

}

void CMulti2Decoder::Initialize(const BYTE *pSystemKey, const BYTE *pInitialCbc)
{
	m_bIsSysKeyValid = true;
	m_bIsWorkKeyValid = false;

	// Descrambling System Keyセット
	m_SystemKey.SetHexData(pSystemKey);
	
	// Descrambler CBC Initial Valueセット
	m_InitialCbc.SetHexData(pInitialCbc);
}

const bool CMulti2Decoder::SetScrambleKey(const BYTE *pScrambleKey)
{
	if(!m_bIsSysKeyValid)return false;
	
	if(!pScrambleKey){
		// キーが設定されない場合はデコード不能にする(不正な復号による破損防止のため)
		m_bIsWorkKeyValid = false;
		return false;
		}

	// Scramble Key Odd/Even をセットする
	DATKEY ScrKeyOdd, ScrKeyEven;
	
	// バイトオーダー変換
	ScrKeyOdd.SetHexData(&pScrambleKey[0]);
	ScrKeyEven.SetHexData(&pScrambleKey[8]);

	// キースケジュール
	KeySchedule(m_WorkKeyOdd, m_SystemKey, ScrKeyOdd);
	KeySchedule(m_WorkKeyEven, m_SystemKey, ScrKeyEven);
	
	m_bIsWorkKeyValid = true;
	
	return true;
}

const bool CMulti2Decoder::Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const
{
	if(!byScrCtrl)return true;										// スクランブルなし
	else if(!m_bIsSysKeyValid || !m_bIsWorkKeyValid)return false;	// スクランブルキー未設定
	else if((byScrCtrl != 2U) && (byScrCtrl != 3U))return false;	// スクランブル制御不正

	// ワークキー選択
	const SYSKEY &WorkKey = (byScrCtrl == 3)? m_WorkKeyOdd : m_WorkKeyEven;

	DWORD dwPos;
	DATKEY CbcData = m_InitialCbc;
	DATKEY SrcData;

	// CBCモード
	for(dwPos = 0UL ; (dwSize - dwPos) >= sizeof(DATKEY) ; dwPos += sizeof(DATKEY)){
		SrcData.SetHexData(&pData[dwPos]);
		DecryptBlock(SrcData, WorkKey);
		SrcData.dwLeft ^= CbcData.dwLeft;
		SrcData.dwRight ^= CbcData.dwRight;
		CbcData.SetHexData(&pData[dwPos]);
		SrcData.GetHexData(&pData[dwPos]);
		}

	// OFBモード
	if(dwPos < dwSize){
		BYTE Remain[sizeof(DATKEY)];
		EncryptBlock(CbcData, WorkKey);
		CbcData.GetHexData(Remain);

		for(BYTE *pRemain = Remain ; (dwSize - dwPos) > 0UL ; dwPos++, pRemain++){
			pData[dwPos] ^= *pRemain;
			}
		}

	return true;
}

inline void CMulti2Decoder::DecryptBlock(DATKEY &Block, const SYSKEY &WorkKey)
{
	// Block Decryption
	for(DWORD dwRound = 0UL ; dwRound < SCRAMBLE_ROUND ; dwRound++){
		RoundFuncPi4(Block, WorkKey.dwKey8);
		RoundFuncPi3(Block, WorkKey.dwKey6, WorkKey.dwKey7);
		RoundFuncPi2(Block, WorkKey.dwKey5);
		RoundFuncPi1(Block);
		RoundFuncPi4(Block, WorkKey.dwKey4);
		RoundFuncPi3(Block, WorkKey.dwKey2, WorkKey.dwKey3);
		RoundFuncPi2(Block, WorkKey.dwKey1);
		RoundFuncPi1(Block);
		}
}

inline void CMulti2Decoder::EncryptBlock(DATKEY &Block, const SYSKEY &WorkKey)
{
	// Block Encryption
	for(DWORD dwRound = 0UL ; dwRound < SCRAMBLE_ROUND ; dwRound++){
		RoundFuncPi1(Block);
		RoundFuncPi2(Block, WorkKey.dwKey1);
		RoundFuncPi3(Block, WorkKey.dwKey2, WorkKey.dwKey3);
		RoundFuncPi4(Block, WorkKey.dwKey4);
		RoundFuncPi1(Block);
		RoundFuncPi2(Block, WorkKey.dwKey5);
		RoundFuncPi3(Block, WorkKey.dwKey6, WorkKey.dwKey7);
		RoundFuncPi4(Block, WorkKey.dwKey8);
		}
}

inline void CMulti2Decoder::KeySchedule(SYSKEY &WorkKey, const SYSKEY &SysKey, DATKEY &DataKey)
{
	// Key Schedule
	RoundFuncPi1(DataKey);									// π1

	RoundFuncPi2(DataKey, SysKey.dwKey1);					// π2
	WorkKey.dwKey1 = DataKey.dwLeft;

	RoundFuncPi3(DataKey, SysKey.dwKey2, SysKey.dwKey3);	// π3
	WorkKey.dwKey2 = DataKey.dwRight;

	RoundFuncPi4(DataKey, SysKey.dwKey4);					// π4
	WorkKey.dwKey3 = DataKey.dwLeft;
	
	RoundFuncPi1(DataKey);									// π1
	WorkKey.dwKey4 = DataKey.dwRight;
	
	RoundFuncPi2(DataKey, SysKey.dwKey5);					// π2
	WorkKey.dwKey5 = DataKey.dwLeft;

	RoundFuncPi3(DataKey, SysKey.dwKey6, SysKey.dwKey7);	// π3
	WorkKey.dwKey6 = DataKey.dwRight;

	RoundFuncPi4(DataKey, SysKey.dwKey8);					// π4
	WorkKey.dwKey7 = DataKey.dwLeft;

	RoundFuncPi1(DataKey);									// π1
	WorkKey.dwKey8 = DataKey.dwRight;
}

inline void CMulti2Decoder::RoundFuncPi1(DATKEY &Block)
{
	// Elementary Encryption Function π1
	Block.dwRight ^= Block.dwLeft;
}

inline void CMulti2Decoder::RoundFuncPi2(DATKEY &Block, const DWORD dwK1)
{
	// Elementary Encryption Function π2
	const DWORD dwY = Block.dwRight + dwK1;
	const DWORD dwZ = LeftRotate(dwY, 1UL) + dwY - 1UL;
	Block.dwLeft ^= LeftRotate(dwZ, 4UL) ^ dwZ;
}

inline void CMulti2Decoder::RoundFuncPi3(DATKEY &Block, const DWORD dwK2, const DWORD dwK3)
{
	// Elementary Encryption Function π3
	const DWORD dwY = Block.dwLeft + dwK2;
	const DWORD dwZ = LeftRotate(dwY, 2UL) + dwY + 1UL;
	const DWORD dwA = LeftRotate(dwZ, 8UL) ^ dwZ;
	const DWORD dwB = dwA + dwK3;
	const DWORD dwC = LeftRotate(dwB, 1UL) - dwB;
	Block.dwRight ^= (LeftRotate(dwC, 16UL) ^ (dwC | Block.dwLeft));
}

inline void CMulti2Decoder::RoundFuncPi4(DATKEY &Block, const DWORD dwK4)
{
	// Elementary Encryption Function π4
	const DWORD dwY = Block.dwRight + dwK4;
	Block.dwLeft ^= (LeftRotate(dwY, 2UL) + dwY + 1UL);
}

inline const DWORD CMulti2Decoder::LeftRotate(const DWORD dwValue, const DWORD dwRotate)
{
	// 左ローテート
	return (dwValue << dwRotate) | (dwValue >> (sizeof(dwValue) * 8UL - dwRotate));
}
