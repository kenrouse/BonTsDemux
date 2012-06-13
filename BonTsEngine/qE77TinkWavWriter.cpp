/*
	author:		qE.77T.ink
	License:	Public Domain
	
	Revision:	1.01
*/
#include "stdafx.h"
#pragma warning(disable:4512)
#pragma warning(disable:4201)
#pragma warning(disable:4819)
#include "qE77TinkWavWriter.h"
#include <ks.h>
#include <ksmedia.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <string>

/*	
	トリップの namespace とかセンスが疑われる('A`)
*/
namespace qE77Tink{ //	qE.77T.ink

WavWriter::WavWriter(CDecoderHandler *pDecoderHandler) : CFileWriter(pDecoderHandler)
{
	m_bForceRiff = false;
}

WavWriter::~WavWriter()
{
	CloseFile();
}

const bool	WavWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum()){
		return false;
	}
	
	return CFileWriter::InputMedia(pMediaData, dwInputIndex);
}

void	WavWriter::CloseFile(void)
{
	if(!m_RiffHeader.empty()){

		LPBYTE pHeader = &m_RiffHeader[0];

		//	ヘッダー更新
		if(m_llWriteSize >= 0xFFFFFFFF){
			
			if(!m_bForceRiff){
				//	RF64
				{
					RF64Chunk* p = reinterpret_cast<RF64Chunk*>(pHeader);

					p->chunkId[0] = 'R';
					p->chunkId[1] = 'F';
					p->chunkId[2] = '6';
					p->chunkId[3] = '4';

					pHeader += sizeof(RF64Chunk);
				}
				//	ds64
				{
					DataSize64Chunk* p1 = reinterpret_cast<DataSize64Chunk*>(pHeader);

					pHeader += sizeof(DataSize64Chunk);

					FormatChunk* p2 = reinterpret_cast<FormatChunk*>(pHeader);

					p1->chunkId[0] = 'd';
					p1->chunkId[1] = 's';
					p1->chunkId[2] = '6';
					p1->chunkId[3] = '4';

					ULARGE_INTEGER u;

					u.QuadPart = m_llWriteSize + m_RiffHeader.size() - 8;

					p1->riffSizeLow  = u.LowPart;
					p1->riffSizeHigh = u.HighPart;

					u.QuadPart = m_llWriteSize;

					p1->dataSizeLow  = u.LowPart;
					p1->dataSizeHigh = u.HighPart;

					u.QuadPart = m_llWriteSize / p2->blockAlignment;

					p1->sampleCountLow  = u.LowPart;
					p1->sampleCountHigh = u.HighPart;
				}
				//	fmt
				{
				}
				//	data
				{
				}
			}
		}else{
			//	RIFF
			{
				RF64Chunk* p = reinterpret_cast<RF64Chunk*>(pHeader);

				p->chunkSize = static_cast<DWORD>(m_llWriteSize) + m_RiffHeader.size() - 8;

				pHeader += sizeof(RF64Chunk);
			}
			//	JUNK
			{
				pHeader += sizeof(DataSize64Chunk);
			}
			//	fmt
			{
				pHeader += sizeof(FormatChunk);
			}
			//	data
			{
				DataChunk* p = reinterpret_cast<DataChunk*>(pHeader);

				p->chunkSize = static_cast<DWORD>(m_llWriteSize);
			}
		}

		//	ヘッダー書き込み
		m_OutFile.Write(reinterpret_cast<BYTE*>(&m_RiffHeader[0]), m_RiffHeader.size(), 0);

	}
	
	CFileWriter::CloseFile();

	//	ヘッダー開放
	m_RiffHeader.clear();
	
	m_bForceRiff = false;
}

const bool	WavWriter::OpenFile(LPCTSTR lpszFileName)
{
	//	ちゃんとフォーマットを渡してね('A`)
	return false;
}

const bool	WavWriter::OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec, bool bForceRiff)
{
	//	ヘッダー作成
	{
		DWORD dwSize = sizeof(RF64Chunk) + sizeof(DataSize64Chunk) + sizeof(FormatChunk) + sizeof(DataChunk);
		
		m_RiffHeader.reserve(dwSize);
		m_RiffHeader.resize(dwSize);
	}

	LPBYTE pHeader = &m_RiffHeader[0];

	//	RIFF
	{
		RF64Chunk* p = reinterpret_cast<RF64Chunk*>(pHeader);

		p->chunkId[0] = 'R';
		p->chunkId[1] = 'I';
		p->chunkId[2] = 'F';
		p->chunkId[3] = 'F';

		p->chunkSize  = 0xFFFFFFFF;

		p->rf64Type[0] = 'W';
		p->rf64Type[1] = 'A';
		p->rf64Type[2] = 'V';
		p->rf64Type[3] = 'E';

		pHeader += sizeof(RF64Chunk);
	}

	//	JUNK
	{
		DataSize64Chunk* p = reinterpret_cast<DataSize64Chunk*>(pHeader);

		p->chunkId[0]  = 'J';
		p->chunkId[1]  = 'U';
		p->chunkId[2]  = 'N';
		p->chunkId[3]  = 'K';

		p->chunkSize   = sizeof(DataSize64Chunk) - 8;
		//	table は 0 
		p->tableLength = 0;

		pHeader += sizeof(DataSize64Chunk);
	}

	//	fmt
	{
		FormatChunk* p = reinterpret_cast<FormatChunk*>(pHeader);

		p->chunkId[0] = 'f';
		p->chunkId[1] = 'm';
		p->chunkId[2] = 't';
		p->chunkId[3] = ' ';

		p->chunkSize  = sizeof(FormatChunk) - 8;

		WAVEFORMATEXTENSIBLE* wf = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pHeader + 8);

		switch(wChannel){
		case 1:
		case 2:
			{
				wf->Format.wFormatTag      = WAVE_FORMAT_PCM;
				wf->Format.nChannels       = wChannel;
				wf->Format.nSamplesPerSec  = dwSamplesPerSec;
				wf->Format.nBlockAlign     = wChannel * wBitsPerSample / 8;
				wf->Format.nAvgBytesPerSec = dwSamplesPerSec * wf->Format.nBlockAlign;
				wf->Format.wBitsPerSample  = wBitsPerSample;
				wf->Format.cbSize          = 0;
			}
			break;
		case 6:
			{
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
			//	それ以外は失敗にするよ
			return false;
		}

		pHeader += sizeof(FormatChunk);
	}

	//	data
	{
		DataChunk* p = reinterpret_cast<DataChunk*>(pHeader);

		p->chunkId[0] = 'd';
		p->chunkId[1] = 'a';
		p->chunkId[2] = 't';
		p->chunkId[3] = 'a';

		p->chunkSize  = 0xFFFFFFFF;
	}
	
	m_bForceRiff = bForceRiff;
	
	//	ファイルオープン
	if(!CFileWriter::OpenFile(lpszFileName)){
		return false;
	}
	
	//	ヘッダー予約
	if(!m_OutFile.Write(reinterpret_cast<BYTE*>(&m_RiffHeader[0]), m_RiffHeader.size())){
		return false;
	}
	
	return true;
}

WavSplitWriter::WavSplitWriter(CDecoderHandler *pDecoderHandler) : CMediaDecoder(pDecoderHandler)
{
	m_dwBlockAlign = 0;
}

WavSplitWriter::~WavSplitWriter()
{
	CloseFile();
}

const bool	WavSplitWriter::OpenFile(LPCTSTR lpszFileName, WORD wBitsPerSample, WORD wChannel, DWORD dwSamplesPerSec, bool bForceRiff)
{
	CloseFile();

	if(wChannel > 6){ return false; }

	std::basic_string<TCHAR> file(lpszFileName);
	
	std::basic_string<TCHAR> ext;
	std::basic_string<TCHAR> name;

	//	ファイル名分解
	std::basic_string<TCHAR>::size_type pos = file.find_last_of(_T("."));

	if(pos != std::basic_string<TCHAR>::npos){
		name = std::basic_string<TCHAR>(file.c_str(), pos);
		ext  = std::basic_string<TCHAR>(file.c_str() + pos);
	}else{
		name = file;
	}
	
	//	とりあえず 6ch まで
	static const TCHAR szChannelName[][6][8] = {
		{_T(""),},
		{_T(""),},
		{_T("-L"),_T("-R"),},
		{_T(""),},
		{_T(""),},
		{_T(""),},
		{_T("-FL"),_T("-FR"),_T("-C"),_T("-LFE"),_T("-SL"),_T("-SR"),}
	};
	
	for(WORD i=0;i<wChannel;i++){
		
		//	ファイルパスを作るよ
		std::basic_string<TCHAR> tmp(name + szChannelName[wChannel][i] + ext);
		
		WavWriter* pFile = new WavWriter(NULL);
		
		if(!pFile->OpenFile(tmp.c_str(), wBitsPerSample, 1, dwSamplesPerSec, bForceRiff)){
			delete pFile;
			return false;
		}
		
		m_OutFiles.push_back(pFile);
	}
	
	m_dwBlockAlign = wChannel * wBitsPerSample / 8;
	
	return !m_OutFiles.empty();
}

void	WavSplitWriter::CloseFile(void)
{	
	//	オブジェクト削除
	for(std::vector<WavWriter*>::iterator it=m_OutFiles.begin();it!=m_OutFiles.end();++it){
		delete (*it);
	}
	
	m_OutFiles.clear();
}

void	WavSplitWriter::Reset(void)
{
}

const DWORD WavSplitWriter::GetInputNum(void) const
{
	return 1;
}

const DWORD WavSplitWriter::GetOutputNum(void) const
{
	return 0;
}

const bool	WavSplitWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum()){
		return false;
	}

	/*
		速度は出ないけどなるべく他のモジュールに修正を入れないように作る
	*/

	std::vector<WavWriter*>& v = m_OutFiles;
	
	DWORD dwBlockAlign    = m_dwBlockAlign;
	DWORD dwChannel       = v.size();
	DWORD dwBytePerSample = dwBlockAlign / dwChannel;
	DWORD dwSample        = pMediaData->GetSize() / dwBlockAlign;
	DWORD dwChannelSize   = dwBytePerSample * dwSample;

	std::vector<BYTE> tmp(pMediaData->GetSize());

	{
		LPBYTE p1 = &tmp[0];
		LPBYTE p2 = pMediaData->GetData();

		for(DWORD i=0;i<dwSample;i++){

			for(DWORD j=0;j<dwChannel;j++){
				::CopyMemory(p1 + (dwChannelSize * j), p2 + (dwBytePerSample * j), dwBytePerSample);
			}

			p1 += dwBytePerSample;
			p2 += dwBlockAlign;

		}

	}

	{
		LPBYTE p1 = &tmp[0];

		CMediaData sample(dwChannelSize);

		for(std::vector<WavWriter*>::iterator it=v.begin();it!=v.end();++it){

			sample.SetData(p1, dwChannelSize);

			(*it)->InputMedia(&sample, dwInputIndex);

			p1 += dwChannelSize;

		}
	}

	return true;
}

}//	End of namespace qE77Tink
