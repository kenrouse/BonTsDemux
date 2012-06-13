// TsEncode.h: TSエンコードクラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ARIB STD-B24 Part1文字列処理クラス
/////////////////////////////////////////////////////////////////////////////

class CAribString
{
public:
	static const DWORD AribToString(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen);
	
private:
	enum CODE_SET
	{
		CODE_UNKNOWN,				// 不明なグラフィックセット(非対応)
		CODE_KANJI,					// Kanji
		CODE_ALPHANUMERIC,			// Alphanumeric
		CODE_HIRAGANA,				// Hiragana
		CODE_KATAKANA,				// Katakana
		CODE_MOSAIC_A,				// Mosaic A
		CODE_MOSAIC_B,				// Mosaic B
		CODE_MOSAIC_C,				// Mosaic C
		CODE_MOSAIC_D,				// Mosaic D
		CODE_PROP_ALPHANUMERIC,		// Proportional Alphanumeric
		CODE_PROP_HIRAGANA,			// Proportional Hiragana
		CODE_PROP_KATAKANA,			// Proportional Katakana
		CODE_JIS_X0201_KATAKANA,	// JIS X 0201 Katakana
		CODE_JIS_KANJI_PLANE_1,		// JIS compatible Kanji Plane 1
		CODE_JIS_KANJI_PLANE_2,		// JIS compatible Kanji Plane 2
		CODE_ADDITIONAL_SYMBOLS		// Additional symbols
	};

	CODE_SET m_CodeG[4];
	CODE_SET *m_pLockingGL;
	CODE_SET *m_pLockingGR;
	CODE_SET *m_pSingleGL;
	
	BYTE m_byEscSeqCount;
	BYTE m_byEscSeqIndex;
	bool m_bIsEscSeqDrcs;
	
	const DWORD AribToStringInternal(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen);
	inline const DWORD ProcessCharCode(TCHAR *lpszDst, const WORD wCode, const CODE_SET CodeSet);

	inline const DWORD PutKanjiChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutAlphanumericChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutHiraganaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutKatakanaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutJisKatakanaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutSymbolsChar(TCHAR *lpszDst, const WORD wCode);

	inline void ProcessEscapeSeq(const BYTE byCode);
	
	inline void LockingShiftGL(const BYTE byIndexG);
	inline void LockingShiftGR(const BYTE byIndexG);
	inline void SingleShiftGL(const BYTE byIndexG);

	inline const bool DesignationGSET(const BYTE byIndexG, const BYTE byCode);
	inline const bool DesignationDRCS(const BYTE byIndexG, const BYTE byCode);
};
