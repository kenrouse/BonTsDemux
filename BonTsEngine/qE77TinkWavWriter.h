/*
	author:		qE.77T.ink
	License:	Public Domain
	
	Revision:	1.01
	
	qE.77T.ink ���N�������R�[�h�Ɋւ��Ă͒��쌠�͎咣���Ȃ��̂�
	���R�Ɏg�p���Ė��Ȃ���
	
	�����A���̂܂܂��̃N���X���g�p����� BonTsEngine �� GPL �Ɋ�������Ǝv��
	���̃R�[�h�ɗ��p����قǂ̉��l������Ƃ͎v���Ȃ�����ǂ��E�E�E('A`)
*/
#pragma once

#include "FileWriter.h"

#include <vector>

/*	
	�g���b�v�� namespace �Ƃ��Z���X���^����E�E�E
	�ǂ� namespace �v�����Ȃ������񂾂�E�E�E�{������('A`)
	�����E�E�E�����A���𓊂��Ȃ��ŁE�E�E('A`)
*/
namespace qE77Tink{ //	qE.77T.ink

//	���K�v�Ȃ��񂾂��ǈꉞ�E�E�E
#include <pshpack1.h>
/*
	http://www.ebu.ch/CMSimages/en/tec_doc_t3306-2007_tcm6-42570.pdf?display=EN
*/
// declare RiffChunk structure
struct RiffChunk{
	char chunkId[4];			// �eRIFF�f
	unsigned long chunkSize;	// 4 byte size of the traditional RIFF/WAVE file
	char riffType[4];			// �eWAVE�f
};

// declare JunkChunk structure
struct JunkChunk{
	char chunkId[4];		// �eJUNK�f
	unsigned int chunkSize;	// 4 byte size of the �eJUNK�f chunk. This must be at
	// least 28 if the chunk is intended as a
	// place-holder for a �eds64�f chunk.
//	char chunkData[];		// dummy bytes
};

// declare FormatChunk structure
struct FormatChunk{
	char chunkId[4];				// �efmt �f
	unsigned long chunkSize;		// 4 byte size of the �efmt �f chunk
	unsigned short formatType;		// WAVE_FORMAT_PCM = 0x0001, etc.
	unsigned short channelCount;	// 1 = mono, 2 = stereo, etc.
	unsigned long sampleRate;		// 32000, 44100, 48000, etc.
	unsigned long bytesPerSecond;	// only important for compressed formats
	unsigned short blockAlignment;	// container size (in bytes) of one set of samples
	unsigned short bitsPerSample;	// valid bits per sample 16, 20 or 24
	unsigned short cbSize;			// extra information (after cbSize) to store
	char extraData[22];				// extra data of WAVE_FORMAT_EXTENSIBLE when necessary
};

// declare DataChunk structure
struct DataChunk{
	char chunkId[4];			// �edata�f
	unsigned long chunkSize;	// 4 byte size of the �edata�f chunk
//	char waveData[];			// audio samples
};

// declare RF64Chunk structure
struct RF64Chunk{
	char chunkId[4];			// �eRF64�f
	unsigned long chunkSize;	// -1 = 0xFFFFFFFF means don�ft use this data, use
	// riffSizeHigh and riffSizeLow in �eds64�f chunk instead
	char rf64Type[4];			// �eWAVE�f
};

// declare ChunkSize64 structure
struct ChunkSize64{
	char chunkId[4]; 				// chunk ID (i.e. �gbig1�h   this chunk is a big one)
	unsigned long chunkSizeLow;		// low 4 byte chunk size
	unsigned long chunkSizeHigh;	// high 4 byte chunk size
};

// declare DataSize64Chunk structure
struct DataSize64Chunk{
	char chunkId[4];				// �eds64�f
	unsigned long chunkSize;		// 4 byte size of the �eds64�f chunk
	unsigned long riffSizeLow;		// low 4 byte size of RF64 block
	unsigned long riffSizeHigh;		// high 4 byte size of RF64 block
	unsigned long dataSizeLow;		// low 4 byte size of data chunk
	unsigned long dataSizeHigh;		// high 4 byte size of data chunk
	unsigned long sampleCountLow;	// low 4 byte sample count of fact chunk
	unsigned long sampleCountHigh;	// high 4 byte sample count of fact chunk
	unsigned long tableLength;		// number of valid entries in array �gtable�h
//	chunkSize64 table[];
};
#include <poppack.h>

/*
	Wav �o��

	4GB <=	�F RIFF
	4GB >	�F RF64
*/
class WavWriter : public CFileWriter{
	protected:
		std::vector<BYTE>		m_RiffHeader;
		bool					m_bForceRiff;
	public:
		WavWriter(CDecoderHandler *pDecoderHandler);
		virtual	~WavWriter();
	
		virtual	const bool	OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec, bool bForceRiff = false);
	
		/*
			CFileWriter �� virtual �����t���ĂȂ��̂��邩��
			�A�b�v�L���X�g�����ƃX�[�p�[�N���X���Ă΂�ē����Ȃ���('A`)
		*/
		virtual	const bool	OpenFile(LPCTSTR lpszFileName);
		virtual	void		CloseFile(void);
		virtual const bool	InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
};

/*
	Wav �����o��

	�`�����l������ wav ��

	4GB <=	�F RIFF
	4GB >	�F RF64
*/
class WavSplitWriter : public CMediaDecoder{
	protected:
		std::vector<WavWriter*>		m_OutFiles;
		DWORD						m_dwBlockAlign;
	public:
		WavSplitWriter(CDecoderHandler *pDecoderHandler);
		virtual	~WavSplitWriter();
		
		virtual	const bool	OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec, bool bForceRiff = false);
		virtual	void		CloseFile(void);
	
		//	IMediaDecoder
		virtual void		Reset(void);
		virtual const DWORD GetInputNum(void) const;
		virtual const DWORD GetOutputNum(void) const;
		virtual const bool	InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
};
	
}//	End of namespace qE77Tink
