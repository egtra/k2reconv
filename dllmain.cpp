// dllmain.cpp : DLL �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
#include "stdafx.h"

HMODULE hModuleSelf;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, void*)
{
	switch (dwReasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		hModuleSelf = hModule;
		// �ȉ��t�H�[���X���[
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
