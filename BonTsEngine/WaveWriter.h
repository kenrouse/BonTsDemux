// WaveWriter.h: CWaveWriter �N���X�̃C���^�[�t�F�C�X
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
// Wave�t�@�C���o��(48KHz 16bit Streo PCM�f�[�^��Wav�t�@�C���ɏ����o��)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		�������݃f�[�^
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
