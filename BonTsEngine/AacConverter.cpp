// AacConverter.cpp: CAacConverter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AacConverter.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// 5.1ch�_�E���~�b�N�X�ݒ�
#define DMR_CENTER			0.5		// 50%
#define DMR_FRONT			1.0		// 100%
#define DMR_REAR			0.7		// 70%
#define DMR_LFE				0.7		// 70%


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////
LONGLONG CAacConverter::m_total_frame = 0;
LONGLONG CAacConverter::m_pes_frame   = 0;
LONG     CAacConverter::m_HoseiPol    = 0;
DWORD     CAacConverter::m_CutFrame    = 0;
DWORD	CAacConverter::m_StereoToMono = 0;

CAacConverter::CAacConverter(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_PcmBuffer(0x00200000UL)
	, m_byLastChannelNum(0U)
	, m_byOutputChannel(2U)
{
	// AAC�f�R�[�_�I�[�v��
	m_AacDecoder.OpenDecoder(CAacConverter::OnLpcmFrame, (PVOID)this);
}

CAacConverter::~CAacConverter()
{
	// // AAC�f�R�[�_�N���[�Y
	m_AacDecoder.CloseDecoder();
}

void CAacConverter::Reset(void)
{
	// ��ԃ��Z�b�g
	m_byLastChannelNum = 0U;
	m_AacDecoder.ResetDecoder();
	m_PcmBuffer.ClearSize();
	m_HoseiPol = 0;
	m_total_frame = 0;
	m_pes_frame = 0;
	m_CutFrame = 0;

	CMediaDecoder::Reset();
}

const DWORD CAacConverter::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CAacConverter::GetOutputNum(void) const
{
	return 1UL;
}

const bool CAacConverter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	CAdtsFrame *pAdtsFrame = static_cast<CAdtsFrame *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pAdtsFrame)return false;

	// AAC�f�R�[�_�ɓ���
	return m_AacDecoder.Decode(pAdtsFrame);
}

const BYTE CAacConverter::GetLastChannelNum(void) const
{
	// �ŐV�̃`�����l������Ԃ�
	return m_byLastChannelNum;
}

void CALLBACK CAacConverter::OnLpcmFrame(const BYTE *pData, const DWORD dwSamples, const BYTE byChannel, PVOID pParam)
{
	CAacConverter *pThis = static_cast<CAacConverter *>(pParam);

	DWORD dwBlockAlign = pThis->m_byOutputChannel * 2;

	// �o�̓|�C���^�擾
	DWORD dwOutSize = 0UL;
	pThis->m_PcmBuffer.SetSize(dwSamples * dwBlockAlign);
	BYTE *pOutBuff = pThis->m_PcmBuffer.GetData();

	if(pData){
		if(pThis->m_byOutputChannel == 6){
	
			// �A�b�v�~�b�N�X�H
			switch(byChannel){
				case 1U:	dwOutSize = UpMixMono((short *)pOutBuff, (const short *)pData, dwSamples);		break;
				case 2U:	dwOutSize = UpMixStreao((short *)pOutBuff, (const short *)pData, dwSamples);		break;
				case 6U:	dwOutSize = UpMixSurround((short *)pOutBuff, (const short *)pData, dwSamples);	break;
				}
	
		}else if(pThis->m_byOutputChannel == 2){
	
			// �_�E���~�b�N�X
			switch(byChannel){
				case 1U:	dwOutSize = DownMixMono((short *)pOutBuff, (const short *)pData, dwSamples);		break;
				case 2U:	dwOutSize = DownMixStreao((short *)pOutBuff, (const short *)pData, dwSamples);		break;
				case 6U:	dwOutSize = DownMixSurround((short *)pOutBuff, (const short *)pData, dwSamples);	break;
				}
			if(m_StereoToMono){
				StereoToMono((short *)pOutBuff, dwSamples , m_StereoToMono == 1 ? TRUE : FALSE);
			}
		}
	} else {
		dwOutSize = dwSamples * dwBlockAlign;
		ZeroMemory(pOutBuff,dwOutSize);
	}

	// ���̃f�R�[�_�ɃT���v����n��

	pThis->m_byLastChannelNum = byChannel;
	if(m_CutFrame){
		if(m_CutFrame >= pThis->m_PcmBuffer.GetSize()/dwBlockAlign){
			m_CutFrame -= pThis->m_PcmBuffer.GetSize()/dwBlockAlign;
			pThis->m_PcmBuffer.ClearSize();
			return;
		} else {
			pThis->m_PcmBuffer.TrimHead(m_CutFrame*dwBlockAlign);
			m_CutFrame = 0;
		}
	} else if(m_HoseiPol < 0){
		Mabiki(&pThis->m_PcmBuffer,m_HoseiPol,dwBlockAlign);
	} else if (m_HoseiPol > 0){
		Hokan(&pThis->m_PcmBuffer,m_HoseiPol,dwBlockAlign);
	}
	m_total_frame += pThis->m_PcmBuffer.GetSize()/dwBlockAlign;
	m_pes_frame += pThis->m_PcmBuffer.GetSize()/dwBlockAlign;
	pThis->OutputMedia(&pThis->m_PcmBuffer);
}

void CAacConverter::Mabiki(CMediaData* buf,LONG ms,DWORD dwBlockAlign)
{
	// ms to sample
	DWORD sample = -ms;		// �������݃Q�C��(�Ԉ����T���v����)
	BYTE* data = (BYTE*)buf->GetData();
	DWORD size = buf->GetSize() / dwBlockAlign;
	DWORD i,skip;

	if(sample >= size){
		buf->SetSize(0);
		return;
	}
	sample= size / sample;
	if(sample==0) sample=2;
	
	skip = 0;
	for(i = 0 ;  ; i ++){
		if(i && (i % sample) == 0) skip++;
		if(skip){
			memcpy(&data[i*dwBlockAlign],&data[(i+skip)*dwBlockAlign],dwBlockAlign);
		}
		if( (i+skip) >= size-1) break;
	}

	buf->SetSize(i*dwBlockAlign);	
}

void CAacConverter::Hokan(CMediaData* buf,LONG ms,DWORD dwBlockAlign)
{
	DWORD sample = ms;		// �������݃Q�C��(�Ԉ����T���v����)
	BYTE* data = (BYTE*)buf->GetData();
	DWORD size = buf->GetSize() / dwBlockAlign;
	DWORD i,skip;

	skip = size / sample;
	if(!skip) skip=2;
	size += sample;
	
	buf->SetSize(size*dwBlockAlign);

	for(i = size-1 ; i > 0 ; i --){
		if(i != (size-1) && (i % skip)==0){
			sample--;
			memcpy(&data[i*dwBlockAlign],&data[(i+1)*dwBlockAlign],dwBlockAlign);
			i--;
		}
		if(sample){
			memcpy(&data[i*dwBlockAlign],&data[(i - sample)*dwBlockAlign],dwBlockAlign);
		}
	}
}


const DWORD CAacConverter::DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 1ch �� 2ch ��d��
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 2UL + 0UL] = pSrc[dwPos];	// L
		pDst[dwPos * 2UL + 1UL] = pSrc[dwPos];	// R
		}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

const DWORD CAacConverter::UpMixMono(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 1ch �� 2ch ��d�� �� 6ch
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 6UL + 0UL] = pSrc[dwPos];	// L
		pDst[dwPos * 6UL + 1UL] = pSrc[dwPos];	// R
		pDst[dwPos * 6UL + 2UL] = 0;			//	C
		pDst[dwPos * 6UL + 3UL] = 0;			//	L
		pDst[dwPos * 6UL + 4UL] = 0;			//	BL
		pDst[dwPos * 6UL + 5UL] = 0;			//	BR
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 12UL;
}

const DWORD CAacConverter::DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 2ch �� 2ch �X���[
	::CopyMemory(pDst, pSrc, dwSamples * 4UL);

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

const DWORD CAacConverter::UpMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 2ch �� 2ch �X���[
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 6UL + 0UL] = pSrc[dwPos * 2UL + 0UL];	// L
		pDst[dwPos * 6UL + 1UL] = pSrc[dwPos * 2UL + 1UL];	// R
		pDst[dwPos * 6UL + 2UL] = 0;			//	C
		pDst[dwPos * 6UL + 3UL] = 0;			//	L
		pDst[dwPos * 6UL + 4UL] = 0;			//	BL
		pDst[dwPos * 6UL + 5UL] = 0;			//	BR
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 12UL;
}

const void CAacConverter::StereoToMono(short *pDst, DWORD dwSamples,BOOL left)
{
	DWORD i;

	for(i = 0 ; i < (dwSamples) ; i ++){
		if(left){
			pDst[i*2+1] = pDst[i*2];
		} else {
			pDst[i*2] = pDst[i*2+1];
		}
	}
}

const DWORD CAacConverter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 2ch �_�E���~�b�N�X
	int iOutLch, iOutRch;

	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		// �_�E���~�b�N�X
		iOutLch = (int)(
					(double)pSrc[dwPos * 6UL + 1UL]	* DMR_FRONT		+
					(double)pSrc[dwPos * 6UL + 3UL]	* DMR_REAR		+
					(double)pSrc[dwPos * 6UL + 0UL]	* DMR_CENTER	+
					(double)pSrc[dwPos * 6UL + 5UL]	* DMR_LFE
					);

		iOutRch = (int)(
					(double)pSrc[dwPos * 6UL + 2UL]	* DMR_FRONT		+
					(double)pSrc[dwPos * 6UL + 4UL]	* DMR_REAR		+
					(double)pSrc[dwPos * 6UL + 0UL]	* DMR_CENTER	+
					(double)pSrc[dwPos * 6UL + 5UL]	* DMR_LFE
					);

		// �N���b�v
		if(iOutLch > 32767L)iOutLch = 32767L;
		else if(iOutLch < -32768L)iOutLch = -32768L;

		if(iOutRch > 32767L)iOutRch = 32767L;
		else if(iOutRch < -32768L)iOutRch = -32768L;

		pDst[dwPos * 2UL + 0UL] = (short)iOutLch;	// L
		pDst[dwPos * 2UL + 1UL] = (short)iOutRch;	// R
		}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

LONGLONG CAacConverter::GetTotalSample(void)
{
	return m_total_frame;
}


const DWORD CAacConverter::UpMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 5.1ch
	/*
		5.1 WAV
		�O�����`�����l�� �O���E�`�����l�� �O�������`�����l�� LFE ������`�����l�� ����E�`�����l�� 
		5.1 AAC
		�O�������`�����l�� �O�����`�����l�� �O���E�`�����l�� ������`�����l�� ����E�`�����l�� LFE 
	*/
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 6UL + 0UL] = pSrc[dwPos * 6UL + 1UL];	//	L
		pDst[dwPos * 6UL + 1UL] = pSrc[dwPos * 6UL + 2UL];	//	R
		pDst[dwPos * 6UL + 2UL] = pSrc[dwPos * 6UL + 0UL];	//	C
		pDst[dwPos * 6UL + 3UL] = pSrc[dwPos * 6UL + 5UL];	//	L
		pDst[dwPos * 6UL + 4UL] = pSrc[dwPos * 6UL + 3UL];	//	BL
		pDst[dwPos * 6UL + 5UL] = pSrc[dwPos * 6UL + 4UL];	//	BR
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 12UL;
}

const BYTE CAacConverter::GetOutputChannel(void) const
{
	return m_byOutputChannel;
}

void	CAacConverter::SetOutputChannel(const BYTE byChannel)
{
	m_byOutputChannel = byChannel;
}
void CAacConverter::SetHoseiPol(LONG diff)
{
	m_HoseiPol = diff;
}

