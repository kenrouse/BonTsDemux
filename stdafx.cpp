// stdafx.cpp : �W���C���N���[�h BonTsDemux.pch �݂̂�
// �܂ރ\�[�X �t�@�C���́A�v���R���p�C���ς݃w�b�_�[�ɂȂ�܂��B
// stdafx.obj �ɂ̓v���R���p�C�����ꂽ�^��񂪊܂܂�܂��B

#include "stdafx.h"


#ifdef _DEBUG
	void DebugTrace(LPCTSTR szFormat, ...)
	{
		va_list Args;
		TCHAR szTempStr[1024];

		va_start(Args , szFormat);
		wvsprintf(szTempStr, szFormat, Args);
		va_end(Args);

		::OutputDebugString(szTempStr);
	}
#endif
