// AacConverter.h: CAacConverter クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "aacdecoder.h"


/////////////////////////////////////////////////////////////////////////////
// AACデコーダ(AACをPCMにデコードする)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CAdtsFrame		入力データ
// Output	#0	: CMediaData		出力データ
/////////////////////////////////////////////////////////////////////////////

class CAacConverter : public CMediaDecoder  
{
public:
	CAacConverter(CDecoderHandler *pDecoderHandler);
	virtual ~CAacConverter();

// IMediaDecoder
	virtual void Reset(void);
	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CAacConverter
	const BYTE GetLastChannelNum(void) const;
	LONGLONG GetTotalSample(void);
	void SetHoseiPol(LONG diff);
	void ResetTotalFrame(void){	m_total_frame = 0;m_pes_frame=0;}
	void ResetPesPerFrame(void){ m_pes_frame = 0; }
	LONGLONG GetPesPerFrame(void) { return m_pes_frame; }
	void SetCutFrame(DWORD frame) { m_CutFrame = frame; }
	void SetStereoMethod(DWORD method) { m_StereoToMono = method; }

	const BYTE GetOutputChannel(void) const;
	void	SetOutputChannel(const BYTE byChannel);

	static LONGLONG m_total_frame;
protected:
	static void CALLBACK OnLpcmFrame(const BYTE *pData, const DWORD dwSamples, const BYTE byChannel, PVOID pParam);

	CAacDecoder m_AacDecoder;
	CMediaData m_PcmBuffer;

	BYTE m_byLastChannelNum;

	BYTE m_byOutputChannel;

private:
	static const DWORD DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples);

	static const DWORD UpMixMono(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD UpMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD UpMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples);

	static LONGLONG m_pes_frame;
	static LONG m_HoseiPol;

	static void Mabiki(CMediaData* buf,LONG ms,DWORD ch=4);
	static void Hokan(CMediaData* buf,LONG ms,DWORD ch=4);
	static const void StereoToMono(short *pDst, DWORD dwSamples,BOOL left);

	static DWORD m_CutFrame;

	static DWORD m_StereoToMono;


};
