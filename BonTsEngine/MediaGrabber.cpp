// MediaGrabber.cpp: CMediaGrabber クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaGrabber.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CMediaGrabber::CMediaGrabber(CDecoderHandler *pDecoderHandler)
	: CMediaDecoder(pDecoderHandler)
	, m_pfnMediaGrabFunc(NULL)
	, m_pfnResetGrabFunc(NULL)
	, m_pMediaGrabParam(NULL)
	, m_pResetGrabParam(NULL)
{
	
}

CMediaGrabber::~CMediaGrabber()
{

}

void CMediaGrabber::Reset(void)
{
	// コールバックに通知する
	if(m_pfnResetGrabFunc)m_pfnResetGrabFunc(m_pResetGrabParam);

	CMediaDecoder::Reset();
}

const DWORD CMediaGrabber::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CMediaGrabber::GetOutputNum(void) const
{
	return 1UL;
}

const bool CMediaGrabber::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	// コールバックに通知する
	if(m_pfnMediaGrabFunc)m_pfnMediaGrabFunc(pMediaData, m_pMediaGrabParam);

	// 下位デコーダにデータを渡す
	OutputMedia(pMediaData);

	return true;
}

void CMediaGrabber::SetMediaGrabCallback(const MEDIAGRABFUNC pCallback, const PVOID pParam)
{
	// メディア受け取りコールバックを登録する
	m_pfnMediaGrabFunc = pCallback;
	m_pMediaGrabParam = pParam;
}

void CMediaGrabber::SetResetGrabCallback(const RESETGRABFUNC pCallback, const PVOID pParam)
{
	// リセット受け取りコールバックを登録する
	m_pfnResetGrabFunc = pCallback;
	m_pResetGrabParam = pParam;
}
