// WaveWriter.cpp: CWaveWriter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CWaveWriter::CWaveWriter(CDecoderHandler *pDecoderHandler)
	: CFileWriter(pDecoderHandler)
{
	m_dwFormatSize = 0;
}

CWaveWriter::~CWaveWriter()
{
	CloseFile();
}

const bool CWaveWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	// 4GB�̐����`�F�b�N
	if((m_llWriteSize + (ULONGLONG)pMediaData->GetSize()) >= 0xFFFFFFFFULL)return true;
	
	// �t�@�C����������
	return CFileWriter::InputMedia(pMediaData, dwInputIndex);
}

const bool CWaveWriter::OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec)
{
	if(!CFileWriter::OpenFile(lpszFileName))return false;

	//	�w�b�_�[
	std::vector<BYTE> wfbuf;

	//	wChannel �ȊO�̈����̃G���[�`�F�b�N���ĂȂ���Ɍ��߂���('A`)
	switch(wChannel){
	case 1:
	case 2:
			{
			m_dwFormatSize = sizeof(WAVEFORMATEX);

			wfbuf.reserve(m_dwFormatSize);
			wfbuf.resize(m_dwFormatSize);

			WAVEFORMATEX* wf = reinterpret_cast<WAVEFORMATEX*>(&wfbuf[0]);

			wf->wFormatTag      = WAVE_FORMAT_PCM;
			wf->nChannels       = wChannel;
			wf->nSamplesPerSec  = dwSamplesPerSec;
			wf->nBlockAlign     = wChannel * wBitsPerSample / 8;
			wf->nAvgBytesPerSec = dwSamplesPerSec * wf->nBlockAlign;
			wf->wBitsPerSample  = wBitsPerSample;
			wf->cbSize          = 0;
			}
		break;
	case 6:
			{
			m_dwFormatSize = sizeof(WAVEFORMATEX) + 22;

			wfbuf.reserve(m_dwFormatSize);
			wfbuf.resize(m_dwFormatSize);

			WAVEFORMATEXTENSIBLE* wf = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(&wfbuf[0]);;

			wf->Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
			wf->Format.nChannels       = wChannel;
			wf->Format.nSamplesPerSec  = dwSamplesPerSec;
			wf->Format.nBlockAlign     = wChannel * wBitsPerSample / 8;
			wf->Format.nAvgBytesPerSec = dwSamplesPerSec * wf->Format.nBlockAlign;
			wf->Format.wBitsPerSample  = wBitsPerSample;
			wf->Format.cbSize          = 22;

			wf->Samples.wValidBitsPerSample = wBitsPerSample;

			wf->dwChannelMask = SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT   |
							    SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
							    SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT;
			wf->SubFormat     = KSDATAFORMAT_SUBTYPE_PCM;
		}
		break;
	default:
		//	����ȊO�͎��s�ɂ����
		return false;
	}

	static const BYTE dummySize[] = { 0x00U, 0x00U, 0x00U, 0x00U };

	//	RIFF �`�����N
	{
		FOURCC fc = ::mmioStringToFOURCC(_T("RIFF"), 0);

		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&fc), sizeof(FOURCC)))return false;
		if(!m_OutFile.Write(dummySize, sizeof(dummySize)))return false;

		fc = ::mmioStringToFOURCC(_T("WAVE"), 0);
	
		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&fc), sizeof(FOURCC)))return false;
	}
	
	//	fmt �`�����N
	{
		FOURCC fc = ::mmioStringToFOURCC(_T("fmt "), 0);
	
		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&fc), sizeof(FOURCC)))return false;
		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&m_dwFormatSize), 4))return false;

		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&wfbuf[0]), m_dwFormatSize))return false;
	}

	//	data �`�����N
	{
		FOURCC fc = ::mmioStringToFOURCC(_T("data"), 0);
	
		if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&fc), sizeof(FOURCC)))return false;
		if(!m_OutFile.Write(dummySize, sizeof(dummySize)))return false;
	}

	return true;
}

void CWaveWriter::CloseFile(void)
{	
	// RIFF�w�b�_�ɃT�C�Y����������
	DWORD dwLength = (DWORD)m_llWriteSize;
	m_OutFile.Write((BYTE *)&dwLength, 4UL, 24 + m_dwFormatSize);
	dwLength += 16 + m_dwFormatSize;
	m_OutFile.Write((BYTE *)&dwLength, 4UL, 4ULL);		

	// �t�@�C�������
	CFileWriter::CloseFile();
}
