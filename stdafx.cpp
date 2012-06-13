// stdafx.cpp : 標準インクルード BonTsDemux.pch のみを
// 含むソース ファイルは、プリコンパイル済みヘッダーになります。
// stdafx.obj にはプリコンパイルされた型情報が含まれます。

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
