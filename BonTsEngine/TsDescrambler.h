// TsDescrambler.h: CTsDescrambler �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsStream.h"
#include "TsTable.h"
#include "TsUtilClass.h"
#include "BcasCard.h"
#include "Multi2Decoder.h"


/////////////////////////////////////////////////////////////////////////////
// MULTI2�X�N�����u������(ECM�ɂ��y�C���[�h�̃X�N�����u������������)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CTsPacket		�Í�TS�p�P�b�g
// Output	#0	: CTsPacket		����TS�p�P�b�g
/////////////////////////////////////////////////////////////////////////////

class CTsDescrambler : public CMediaDecoder  
{
public:
	CTsDescrambler(CDecoderHandler *pDecoderHandler);
	virtual ~CTsDescrambler();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CDescrambler
	const bool OpenBcasCard(DWORD *pErrorCode = NULL);
	void CloseBcasCard(void);
	const bool GetBcasCardID(BYTE *pCardID);

	const DWORD GetInputPacketCount(void) const;
	const DWORD GetScramblePacketCount(void) const;

protected:
	class CEcmProcessor;
	class CEsProcessor;

	static void CALLBACK OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);

	CTsPidMapManager m_PidMapManager;
//	CBcasCard m_BcasCard;
	CBcasCard m_BcasCard;

	DWORD m_dwInputPacketCount;
	DWORD m_dwScramblePacketCount;
};


// ECM���������N���X
class CTsDescrambler::CEcmProcessor : public CDynamicReferenceable, public CPsiSingleTable
{
public:
	CEcmProcessor(CBcasCard *pBcasCard);

// CTsPidMapTarget
	virtual void OnPidMapped(const WORD wPID, const PVOID pParam);
	virtual void OnPidUnmapped(const WORD wPID);

	const bool DescramblePacket(CTsPacket *pTsPacket);
	
protected:
// CPsiSingleTable
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

private:
	CMulti2Decoder m_Multi2Decoder;
	CBcasCard *m_pBcasCard;

	bool m_bLastEcmSucceed;
};


// ES�X�N�����u�����������N���X
class CTsDescrambler::CEsProcessor : public CTsPidMapTarget
{
public:
	CEsProcessor(CTsDescrambler::CEcmProcessor *pEcmProcessor);
	virtual ~CEsProcessor();

// CTsPidMapTarget
	virtual const bool StorePacket(const CTsPacket *pPacket);
	virtual void OnPidUnmapped(const WORD wPID);

private:
	CTsDescrambler::CEcmProcessor *m_pEcmProcessor;
};
