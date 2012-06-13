// MediaDecoder.h: CMediaDecoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaData.h"


//////////////////////////////////////////////////////////////////////
// ���f�B�A�f�R�[�_���N���X
//////////////////////////////////////////////////////////////////////

class CDecoderHandler;

class CMediaDecoder  
{
public:
	CMediaDecoder(CDecoderHandler *pDecoderHandler);
	virtual ~CMediaDecoder();

	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const = 0;
	virtual const DWORD GetOutputNum(void) const = 0;

	virtual const bool SetOutputDecoder(CMediaDecoder *pDecoder, const DWORD dwOutputIndex = 0UL, const DWORD dwInputIndex = 0UL);
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL) = 0;

protected:
	virtual const bool OutputMedia(CMediaData *pMediaData, const DWORD dwOutptIndex = 0UL);
	virtual const DWORD SendDecoderEvent(const DWORD dwEventID, PVOID pParam = NULL);

	// �o�̓s���f�[�^�x�[�X
	struct TAG_OUTPUTDECODER
	{
		CMediaDecoder *pDecoder;
		DWORD dwInputIndex;
	} m_aOutputDecoder[16];

	CDecoderHandler *m_pDecoderHandler;
};


//////////////////////////////////////////////////////////////////////
// ���f�B�A�f�R�[�_�C�x���g�n���h���C���^�t�F�[�X
//////////////////////////////////////////////////////////////////////

class CDecoderHandler
{
friend CMediaDecoder;

protected:
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);
};
