// Multi2Decoder.h: CMulti2Decoder クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


class CMulti2Decoder
{
public:
	CMulti2Decoder(void);
	~CMulti2Decoder(void);
	
	void Initialize(const BYTE *pSystemKey, const BYTE *pInitialCbc);
	const bool SetScrambleKey(const BYTE *pScrambleKey);
	const bool Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const;

private:
	class DATKEY	// Data Key(Dk) 64bit
	{
	public:
		inline void SetHexData(const BYTE *pHexData);
		inline void GetHexData(BYTE *pHexData) const;
		
		DWORD dwLeft, dwRight;
	};
	
	class SYSKEY	// System Key(Sk), Expanded Key(Wk) 256bit
	{
	public:
		inline void SetHexData(const BYTE *pHexData);
		inline void GetHexData(BYTE *pHexData) const;
	
		DWORD dwKey1, dwKey2, dwKey3, dwKey4, dwKey5, dwKey6, dwKey7, dwKey8;
	};

	static inline void DecryptBlock(DATKEY &Block, const SYSKEY &WorkKey);
	static inline void EncryptBlock(DATKEY &Block, const SYSKEY &WorkKey);

	static inline void KeySchedule(SYSKEY &WorkKey, const SYSKEY &SysKey, DATKEY &DataKey);

	static inline void RoundFuncPi1(DATKEY &Block);
	static inline void RoundFuncPi2(DATKEY &Block, const DWORD dwK1);
	static inline void RoundFuncPi3(DATKEY &Block, const DWORD dwK2, const DWORD dwK3);
	static inline void RoundFuncPi4(DATKEY &Block, const DWORD dwK4);

	static inline const DWORD LeftRotate(const DWORD dwValue, const DWORD dwRotate);

	DATKEY m_InitialCbc;
	SYSKEY m_SystemKey;
	SYSKEY m_WorkKeyOdd, m_WorkKeyEven;
	
	bool m_bIsSysKeyValid;
	bool m_bIsWorkKeyValid;
};
