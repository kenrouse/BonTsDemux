// TsDescriptor.h: �L�q�q���b�p�[�N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsEncode.h"
#include "TsDescriptor.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// �L�q�q�̊��N���X
/////////////////////////////////////////////////////////////////////////////

CBaseDesc::CBaseDesc()
{
	Reset();
}

CBaseDesc::CBaseDesc(const CBaseDesc &Operand)
{
	// �R�s�[�R���X�g���N�^
	*this = Operand;
}

CBaseDesc::~CBaseDesc()
{

}

CBaseDesc & CBaseDesc::operator = (const CBaseDesc &Operand)
{
	// ������Z�q
	CopyDesc(&Operand);

	return *this;
}

void CBaseDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// �C���X�^���X�̃R�s�[
	m_byDescTag = pOperand->m_byDescTag;
	m_byDescLen = pOperand->m_byDescLen;
	m_bIsValid = pOperand->m_bIsValid;
}

const bool CBaseDesc::ParseDesc(const BYTE *pHexData, const WORD wDataLength)
{
	Reset();
	
	// ���ʃt�H�[�}�b�g���`�F�b�N
	if(!pHexData)return false;										// �f�[�^����
	else if(wDataLength < 2U)return false;							// �f�[�^���Œ�L�q�q�T�C�Y����
	else if(wDataLength < (WORD)(pHexData[1] + 2U))return false;	// �f�[�^���L�q�q�̃T�C�Y����������

	m_byDescTag = pHexData[0];
	m_byDescLen = pHexData[1];

	// �y�C���[�h���
	if(StoreContents(&pHexData[2])){
		m_bIsValid = true;
		}
	
	return m_bIsValid;
}

const bool CBaseDesc::IsValid(void) const
{
	// �f�[�^���L��(��͍�)���ǂ�����Ԃ�
	return m_bIsValid;
}

const BYTE CBaseDesc::GetTag(void) const
{
	// �L�q�q�^�O��Ԃ�
	return m_byDescTag;
}

const BYTE CBaseDesc::GetLength(void) const
{
	// �L�q�q����Ԃ�
	return m_byDescLen;
}

void CBaseDesc::Reset(void)
{
	// ��Ԃ��N���A����
	m_byDescTag = 0x00U;
	m_byDescLen = 0U;
	m_bIsValid = false;
}

const bool CBaseDesc::StoreContents(const BYTE *pPayload)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CCaMethodDesc::CCaMethodDesc()
	: CBaseDesc()
{
	Reset();
}

CCaMethodDesc::CCaMethodDesc(const CCaMethodDesc &Operand)
{
	*this = Operand;
}

CCaMethodDesc & CCaMethodDesc::operator = (const CCaMethodDesc &Operand)
{
	CopyDesc(&Operand);

	return *this;
}

void CCaMethodDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// �C���X�^���X�̃R�s�[
	CBaseDesc::CopyDesc(pOperand);

	const CCaMethodDesc *pSrcDesc = dynamic_cast<const CCaMethodDesc *>(pOperand);
	
	if(pSrcDesc){
		m_wCaMethodID = pSrcDesc->m_wCaMethodID;
		m_wCaPID = pSrcDesc->m_wCaPID;
		m_PrivateData = pSrcDesc->m_PrivateData;
		}
}

void CCaMethodDesc::Reset(void)
{
	CBaseDesc::Reset();

	m_wCaMethodID = 0x0000U;		// Conditional Access Method ID
	m_wCaPID = 0xFFFFU;				// Conditional Access PID
	m_PrivateData.ClearSize();		// Private Data
}

const WORD CCaMethodDesc::GetCaMethodID(void) const
{
	// Conditional Access Method ID ��Ԃ�
	return m_wCaMethodID;
}

const WORD CCaMethodDesc::GetCaPID(void) const
{
	// Conditional Access PID
	return m_wCaPID;
}

const CMediaData * CCaMethodDesc::GetPrivateData(void) const
{
	// Private Data ��Ԃ�
	return &m_PrivateData;
}

const bool CCaMethodDesc::StoreContents(const BYTE *pPayload)
{
	// �t�H�[�}�b�g���`�F�b�N
	if(m_byDescTag != 0x09U)return false;								// �^�O���s��
	else if(m_byDescLen < 4U)return false;								// CA���\�b�h�L�q�q�̍ŏ��T�C�Y��4
	else if((pPayload[2] & 0xE0U) != 0xE0U)return false;				// �Œ�r�b�g���s��

	// �L�q�q�����
	m_wCaMethodID = (WORD)pPayload[0] << 8 | (WORD)pPayload[1];			// +0,1	Conditional Access Method ID
	m_wCaPID = (WORD)(pPayload[2] & 0x1FU) << 8 | (WORD)pPayload[3];	// +2,3	Conditional Access PID
	m_PrivateData.SetData(&pPayload[4], m_byDescLen - 4U);				// +4-	Private Data

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CServiceDesc::CServiceDesc()
	: CBaseDesc()
{
	Reset();
}

CServiceDesc::CServiceDesc(const CServiceDesc &Operand)
{
	*this = Operand;
}

CServiceDesc & CServiceDesc::operator = (const CServiceDesc &Operand)
{
	CopyDesc(&Operand);
	
	return *this;
}

void CServiceDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// �C���X�^���X�̃R�s�[
	CBaseDesc::CopyDesc(pOperand);

	const CServiceDesc *pSrcDesc = dynamic_cast<const CServiceDesc *>(pOperand);
	
	if(pSrcDesc){
		m_byServiceType = pSrcDesc->m_byServiceType;
		::lstrcpy(m_szProviderName, pSrcDesc->m_szProviderName);
		::lstrcpy(m_szServiceName, pSrcDesc->m_szServiceName);
		}
}

void CServiceDesc::Reset(void)
{
	CBaseDesc::Reset();
	
	m_byServiceType = 0x00U;			// Service Type
	m_szProviderName[0] = TEXT('\0');	// Service Provider Name
	m_szServiceName[0] = TEXT('\0');	// Service Name
}

const BYTE CServiceDesc::GetServiceType(void) const
{
	// Service Type��Ԃ�
	return m_byServiceType;
}

const DWORD CServiceDesc::GetProviderName(LPTSTR lpszDst) const
{
	// Service Provider Name��Ԃ�
	if(lpszDst)::lstrcpy(lpszDst, m_szProviderName);

	return ::lstrlen(m_szProviderName);
}

const DWORD CServiceDesc::GetServiceName(LPTSTR lpszDst) const
{
	// Service Provider Name��Ԃ�
	if(lpszDst)::lstrcpy(lpszDst, m_szServiceName);

	return ::lstrlen(m_szServiceName);
}

const bool CServiceDesc::StoreContents(const BYTE *pPayload)
{
	// �t�H�[�}�b�g���`�F�b�N
	if(m_byDescTag != 0x48U)return false;		// �^�O���s��
	else if(m_byDescLen < 3U)return false;		// �T�[�r�X�L�q�q�̃T�C�Y�͍Œ�3

	// �L�q�q�����
	m_byServiceType = pPayload[0];				// +0	Service Type
	
	BYTE byPos = 1U;
	
	// Provider Name
	if(pPayload[byPos + 0]){
		CAribString::AribToString(m_szProviderName, &pPayload[byPos + 1], pPayload[byPos + 0]);
		byPos += pPayload[byPos + 0];
		}
	
	byPos++;

	// Service Name
	if(pPayload[byPos + 0]){
		CAribString::AribToString(m_szServiceName, &pPayload[byPos + 1], pPayload[byPos + 0]);
		byPos += pPayload[byPos + 0];
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier �L�q�q���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CStreamIdDesc::CStreamIdDesc()
	: CBaseDesc()
{
	Reset();
}

CStreamIdDesc::CStreamIdDesc(const CStreamIdDesc &Operand)
{
	*this = Operand;
}

CStreamIdDesc & CStreamIdDesc::operator = (const CStreamIdDesc &Operand)
{
	CopyDesc(&Operand);

	return *this;
}

void CStreamIdDesc::CopyDesc(const CBaseDesc *pOperand)
{
	// �C���X�^���X�̃R�s�[
	CBaseDesc::CopyDesc(pOperand);

	const CStreamIdDesc *pSrcDesc = dynamic_cast<const CStreamIdDesc *>(pOperand);
	
	if(pSrcDesc){
		m_byComponentTag = pSrcDesc->m_byComponentTag;
		}
}

void CStreamIdDesc::Reset(void)
{
	CBaseDesc::Reset();

	m_byComponentTag = 0x00U;	// Component Tag
}

const BYTE CStreamIdDesc::GetComponentTag(void) const
{
	// Component Tag ��Ԃ�
	return m_byComponentTag;
}

const bool CStreamIdDesc::StoreContents(const BYTE *pPayload)
{
	// �t�H�[�}�b�g���`�F�b�N
	if(m_byDescTag != 0x52U)return false;		// �^�O���s��
	else if(m_byDescLen != 1U)return false;		// �X�g���[��ID�L�q�q�̃T�C�Y�͏��1

	// �L�q�q�����
	m_byComponentTag = pPayload[0];				// +0	Component Tag

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// �L�q�q�u���b�N���ۉ��N���X
/////////////////////////////////////////////////////////////////////////////

CDescBlock::CDescBlock()
{

}

CDescBlock::CDescBlock(const CDescBlock &Operand)
{
	*this = Operand;
}

CDescBlock::~CDescBlock()
{
	Reset();
}

CDescBlock & CDescBlock::operator = (const CDescBlock &Operand)
{
	// �C���X�^���X�̃R�s�[
	Reset();
	m_DescArray.resize(Operand.m_DescArray.size());
	
	for(WORD wIndex = 0UL ; wIndex < m_DescArray.size() ; wIndex++){
		m_DescArray[wIndex] = CreateDescInstance(Operand.m_DescArray[wIndex]->GetTag());
		m_DescArray[wIndex]->CopyDesc(Operand.m_DescArray[wIndex]);
		}
	
	return *this;
}

const WORD CDescBlock::ParseBlock(const BYTE *pHexData, const WORD wDataLength)
{
	if(!pHexData || (wDataLength < 2U))return 0U;

	// ��Ԃ��N���A
	Reset();

	// �w�肳�ꂽ�u���b�N�Ɋ܂܂��L�q�q����͂���
	WORD wPos = 0UL;
	CBaseDesc *pNewDesc = NULL;
	
	while(wPos < wDataLength){
		// �u���b�N����͂���
		if(!(pNewDesc = ParseDesc(&pHexData[wPos], wDataLength - wPos)))break;

		// ���X�g�ɒǉ�����
		m_DescArray.push_back(pNewDesc);

		// �ʒu�X�V
		wPos += (pNewDesc->GetLength() + 2U);
		}

	return m_DescArray.size();
}

const CBaseDesc * CDescBlock::ParseBlock(const BYTE *pHexData, const WORD wDataLength, const BYTE byTag)
{
	// �w�肳�ꂽ�u���b�N�Ɋ܂܂��L�q�q����͂��Ďw�肳�ꂽ�^�O�̋L�q�q��Ԃ�
	return (ParseBlock(pHexData, wDataLength))? GetDescByTag(byTag) : NULL;
}

void CDescBlock::Reset(void)
{
	// �S�ẴC���X�^���X���J������
	for(WORD wIndex = 0U ; wIndex < m_DescArray.size() ; wIndex++){
		delete m_DescArray[wIndex];
		}
		
	m_DescArray.clear();
}

const WORD CDescBlock::GetDescNum(void) const
{
	// �L�q�q�̐���Ԃ�
	return m_DescArray.size();
}

const CBaseDesc * CDescBlock::GetDescByIndex(const WORD wIndex) const
{
	// �C���f�b�N�X�Ŏw�肵���L�q�q��Ԃ�
	return (wIndex < m_DescArray.size())? m_DescArray[wIndex] : NULL;
}

const CBaseDesc * CDescBlock::GetDescByTag(const BYTE byTag) const
{
	// �w�肵���^�O�Ɉ�v����L�q�q��Ԃ�
	for(WORD wIndex = 0U ; wIndex < m_DescArray.size() ; wIndex++){
		if(m_DescArray[wIndex]->GetTag() == byTag)return m_DescArray[wIndex];
		}

	return NULL;
}

CBaseDesc * CDescBlock::ParseDesc(const BYTE *pHexData, const WORD wDataLength)
{
	if(!pHexData || (wDataLength < 2U))return NULL;

	// �^�O�ɑΉ������C���X�^���X�𐶐�����
	CBaseDesc *pNewDesc = CreateDescInstance(pHexData[0]);

	// �������s��
	if(!pNewDesc)return NULL;

	// �L�q�q����͂���
	if(!pNewDesc->ParseDesc(pHexData, wDataLength)){
		// �G���[����
		delete pNewDesc;
		return NULL;
		}

	return pNewDesc;
}

CBaseDesc * CDescBlock::CreateDescInstance(const BYTE byTag)
{
	// �^�O�ɑΉ������C���X�^���X�𐶐�����
	switch(byTag){
		case CCaMethodDesc::DESC_TAG	: return new CCaMethodDesc;
		case CServiceDesc::DESC_TAG		: return new CServiceDesc;
		case CStreamIdDesc::DESC_TAG	: return new CStreamIdDesc;
		default							: return new CBaseDesc;
		}
}
