#include "stdafx.h"
#include <vector>
#include <algorithm>
#include <limits>

namespace
{

BOOL CALLBACK enumThreadWindowProc(HWND hwnd, LPARAM lParam)
{
	TCHAR className[256];
	GetClassName(hwnd, className, ARRAYSIZE(className));

	if (_tcscmp(className, TEXT("TK2EditMain")) == 0)
	{
		*reinterpret_cast<HWND*>(lParam) = hwnd;
		return FALSE;
	}
	return TRUE;
}

HWND FindEditWindow()
{
	HWND hwnd = nullptr;
	EnumThreadWindows(GetCurrentThreadId(), enumThreadWindowProc, reinterpret_cast<LPARAM>(&hwnd));
	if (hwnd != nullptr)
	{
		return FindWindowEx(hwnd, nullptr, TEXT("TK2Edit"), nullptr);
	}
	return nullptr;
}

WNDPROC oldWindowProc;

LRESULT OnImeRequestReconvertString(HWND hwnd, RECONVERTSTRING* prcs)
{
	DWORD first = 0, last = 0;
	CallWindowProc(oldWindowProc, hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&first), reinterpret_cast<LPARAM>(&last));
	if (first > last)
	{
		std::swap(first, last);
	}

	//�I��͈́i�L�����b�g�j���܂ލs��T���i���̍s�ɂ��镶���񂷂ׂĂ𓾂邽�߁j
	LRESULT lineStartPos = CallWindowProc(oldWindowProc, hwnd, EM_LINEFROMCHAR, first, 0);

	//�s����T��
	WORD lineLength = static_cast<WORD>(std::max(
		static_cast<LPARAM>(std::numeric_limits<WORD>::max()),
		CallWindowProc(oldWindowProc, hwnd, EM_LINELENGTH, lineStartPos, 0)));
	if (static_cast<LONG>(last - first) > lineLength)
	{
		last = first + lineLength;
		CallWindowProc(oldWindowProc, hwnd, EM_SETSEL, first, last);
	}
	DWORD selStrLen = last - first; //�I�𕔕��̒����i�I��͈͂��Ȃ���΂��̂܂�0�ŗǂ��j
	LRESULT lr = sizeof (RECONVERTSTRING) + selStrLen + 1;
	if (prcs != nullptr)
	{
		std::vector<TCHAR> buf(lineLength);
		reinterpret_cast<WORD&>(buf[0]) = selStrLen;
		LRESULT bufLen = CallWindowProc(oldWindowProc, hwnd, EM_GETLINE, lineStartPos, reinterpret_cast<LPARAM>(&buf[0]));
		buf.resize(static_cast<std::size_t>(bufLen));

		//�K�v�ȏ��̐ݒ�
		//1�s���̕�����ƕϊ������̃I�t�Z�b�g��n���悤�ɂ��Ă���
		PTSTR dst = reinterpret_cast<PTSTR>(&prcs[1]);
		std::copy(buf.begin(), buf.end(), dst);
		dst[buf.size()] = TEXT('\0');

		DWORD strOffset = sizeof *prcs + (first - CallWindowProc(oldWindowProc, hwnd, EM_LINEINDEX, lineStartPos, 0));
		prcs->dwStrLen = bufLen;
		prcs->dwStrOffset = sizeof *prcs;
		prcs->dwCompStrLen = selStrLen;
		prcs->dwCompStrOffset = strOffset;
		prcs->dwTargetStrLen = selStrLen;
		prcs->dwTargetStrOffset = strOffset;
	}
	return lr;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_IME_REQUEST && wParam == IMR_RECONVERTSTRING)
	{
		return OnImeRequestReconvertString(hwnd, reinterpret_cast<RECONVERTSTRING*>(lParam));
	}
	return CallWindowProc(oldWindowProc, hwnd, msg, wParam, lParam);
}

} // end of unnamed namespace

void InitializeReconv()
{
	LoadLibrary("k2regexp.dll");
	if (HWND const hwndTarget = FindEditWindow())
	{
		SubclassWindow(hwndTarget, WindowProc);
	}
}
