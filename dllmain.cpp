// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

HMODULE hModuleSelf;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, void*)
{
	switch (dwReasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		hModuleSelf = hModule;
		// 以下フォールスルー
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
