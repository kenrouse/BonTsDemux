// WaveWriter.h: CWaveWriter クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "FileWriter.h"

#include <vector>

#include <ks.h>
#include <ksmedia.h>
#pragma warning(disable:4819)
#include <mmreg.h>
#pragma warning(default:4819)
#include <mmsystem.h>

/////////////////////////////////////////////////////////////////////////////
// Waveファイル出力(48KHz 16bit Streo PCMデータをWavファイルに書き出す)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		書き込みデータ
/////////////////////////////////////////////////////////////////////////////

class CWaveWriter : public CFileWriter  
{
protected:
	DWORD		m_dwFormatSize;
public:
	CWaveWriter(CDecoderHandler *pDecoderHandler);
	virtual ~CWaveWriter();

// CFileWriter
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	const bool OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec);
	void CloseFile(void);
};
