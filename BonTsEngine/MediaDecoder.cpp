// MediaDecoder.cpp: CMediaDecoder �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaDecoder.h"
#include "MediaData.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CMediaDecoder �\�z/����
//////////////////////////////////////////////////////////////////////

CMediaDecoder::CMediaDecoder(CDecoderHandler *pDecoderHandler)
	: m_pDecoderHandler(pDecoderHandler)
{
	// �o�̓t�B���^�z����N���A����
	::ZeroMemory(m_aOutputDecoder, sizeof(m_aOutputDecoder));
}

CMediaDecoder::~CMediaDecoder()
{

}

void CMediaDecoder::Reset()
{
	// ���̃t�B���^�����Z�b�g����
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			m_aOutputDecoder[dwOutputIndex].pDecoder->Reset();
			}
		}
}

const bool CMediaDecoder::SetOutputDecoder(CMediaDecoder *pDecoder, const DWORD dwOutputIndex, const DWORD dwInputIndex)
{
	// �o�̓t�B���^���Z�b�g����
	if(dwOutputIndex < GetOutputNum()){
		m_aOutputDecoder[dwOutputIndex].pDecoder = pDecoder;
		m_aOutputDecoder[dwOutputIndex].dwInputIndex = dwInputIndex;
		return true;
		}

	return false;
}

const bool CMediaDecoder::OutputMedia(CMediaData *pMediaData, const DWORD dwOutptIndex)
{
	// �f�t�H���g�̏o�͏���

	// ���̃t�B���^�Ƀf�[�^��n��
	if(dwOutptIndex < GetOutputNum()){
		if(m_aOutputDecoder[dwOutptIndex].pDecoder){
			return m_aOutputDecoder[dwOutptIndex].pDecoder->InputMedia(pMediaData, m_aOutputDecoder[dwOutptIndex].dwInputIndex);
			}
		}

	return false;
}

const DWORD CMediaDecoder::SendDecoderEvent(const DWORD dwEventID, PVOID pParam)
{
	// �C�x���g��ʒm����
		return m_pDecoderHandler->OnDecoderEvent(this, dwEventID, pParam);
}


//////////////////////////////////////////////////////////////////////
// CDecoderHandler �\�z/����
//////////////////////////////////////////////////////////////////////

const DWORD CDecoderHandler::OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	// �f�t�H���g�̏���
	return 0UL;
}
