/*
	author:		qE.77T.ink
	License:	Public Domain
	
	Revision:	1.01
	
	qE.77T.ink が起こしたコードに関しては著作権は主張しないので
	自由に使用して問題ないよ
	
	多分、このままこのクラスを使用すると BonTsEngine の GPL に感染すると思う
	このコードに流用するほどの価値があるとは思えないけれども・・・('A`)
*/
#pragma once

#include "FileWriter.h"

#include <vector>

/*	
	トリップの namespace とかセンスが疑われる・・・
	良い namespace 思いつかなかったんだよ・・・本当だよ('A`)
	あぁ・・・そこ、物を投げないで・・・('A`)
*/
namespace qE77Tink{ //	qE.77T.ink

//	↓必要ないんだけど一応・・・
#include <pshpack1.h>
/*
	http://www.ebu.ch/CMSimages/en/tec_doc_t3306-2007_tcm6-42570.pdf?display=EN
*/
// declare RiffChunk structure
struct RiffChunk{
	char chunkId[4];			// ‘RIFF’
	unsigned long chunkSize;	// 4 byte size of the traditional RIFF/WAVE file
	char riffType[4];			// ‘WAVE’
};

// declare JunkChunk structure
struct JunkChunk{
	char chunkId[4];		// ‘JUNK’
	unsigned int chunkSize;	// 4 byte size of the ‘JUNK’ chunk. This must be at
	// least 28 if the chunk is intended as a
	// place-holder for a ‘ds64’ chunk.
//	char chunkData[];		// dummy bytes
};

// declare FormatChunk structure
struct FormatChunk{
	char chunkId[4];				// ‘fmt ’
	unsigned long chunkSize;		// 4 byte size of the ‘fmt ’ chunk
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
	char chunkId[4];			// ‘data’
	unsigned long chunkSize;	// 4 byte size of the ‘data’ chunk
//	char waveData[];			// audio samples
};

// declare RF64Chunk structure
struct RF64Chunk{
	char chunkId[4];			// ‘RF64’
	unsigned long chunkSize;	// -1 = 0xFFFFFFFF means don’t use this data, use
	// riffSizeHigh and riffSizeLow in ‘ds64’ chunk instead
	char rf64Type[4];			// ‘WAVE’
};

// declare ChunkSize64 structure
struct ChunkSize64{
	char chunkId[4]; 				// chunk ID (i.e. “big1”   this chunk is a big one)
	unsigned long chunkSizeLow;		// low 4 byte chunk size
	unsigned long chunkSizeHigh;	// high 4 byte chunk size
};

// declare DataSize64Chunk structure
struct DataSize64Chunk{
	char chunkId[4];				// ‘ds64’
	unsigned long chunkSize;		// 4 byte size of the ‘ds64’ chunk
	unsigned long riffSizeLow;		// low 4 byte size of RF64 block
	unsigned long riffSizeHigh;		// high 4 byte size of RF64 block
	unsigned long dataSizeLow;		// low 4 byte size of data chunk
	unsigned long dataSizeHigh;		// high 4 byte size of data chunk
	unsigned long sampleCountLow;	// low 4 byte sample count of fact chunk
	unsigned long sampleCountHigh;	// high 4 byte sample count of fact chunk
	unsigned long tableLength;		// number of valid entries in array “table”
//	chunkSize64 table[];
};
#include <poppack.h>

/*
	Wav 出力

	4GB <=	： RIFF
	4GB >	： RF64
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
			CFileWriter に virtual 属性付いてないのあるから
			アップキャストされるとスーパークラスが呼ばれて動かないよ('A`)
		*/
		virtual	const bool	OpenFile(LPCTSTR lpszFileName);
		virtual	void		CloseFile(void);
		virtual const bool	InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
};

/*
	Wav 分割出力

	チャンネル毎に wav 化

	4GB <=	： RIFF
	4GB >	： RF64
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
