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
	SendMessage(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&first), reinterpret_cast<LPARAM>(&last));
	if (first > last)
	{
		std::swap(first, last);
	}

	//�I��͈́i�L�����b�g�j���܂ލs��T���i���̍s�ɂ��镶���񂷂ׂĂ𓾂邽�߁j
	auto lineNumberOfStartPos = Edit_LineFromChar(hwnd, -1);

	//�s����T���iEM_GETLINE��WORD�ŕ�����w�肷�邽�߁AWORD�̍ő�l������Ƃ��Ă���j
	auto lineLength = static_cast<WORD>(std::min(
		static_cast<int>(std::numeric_limits<WORD>::max()),
		Edit_LineLength(hwnd, -1)));

	if (lineLength == 0)
	{
		return 0;
	}

	if (static_cast<LONG>(last - first) > lineLength)
	{
		last = first + lineLength;
		Edit_SetSel(hwnd, first, last);
	}
	auto selectedLength = last - first; //�I�𕔕��̒����i�I��͈͂��Ȃ���΂��̂܂�0�ŗǂ��j
	LRESULT lr = sizeof (RECONVERTSTRING) + lineLength + 1;
	if (prcs != nullptr)
	{
		std::vector<TCHAR> buf(lineLength);
		LRESULT bufLen = Edit_GetLine(hwnd, lineNumberOfStartPos, buf.data(), lineLength);
		buf.resize(static_cast<std::size_t>(bufLen));

		//�K�v�ȏ��̐ݒ�
		//1�s���̕�����ƕϊ������̃I�t�Z�b�g��n���悤�ɂ��Ă���
		auto dst = reinterpret_cast<PTSTR>(&prcs[1]);
		_tcsncpy(dst, buf.data(), buf.size());
		dst[buf.size()] = TEXT('\0');

		auto lineStartingPos = static_cast<DWORD>(Edit_LineIndex(hwnd, lineNumberOfStartPos));
		auto strOffset = first - lineStartingPos;
		prcs->dwStrLen = bufLen;
		prcs->dwStrOffset = sizeof *prcs;
		prcs->dwCompStrLen = selectedLength;
		prcs->dwCompStrOffset = strOffset;
		prcs->dwTargetStrLen = selectedLength;
		prcs->dwTargetStrOffset = strOffset;

		if (selectedLength == 0)
		{
			auto himc = ::ImmGetContext(hwnd);
			if (himc == nullptr)
			{
				return 0;
			}
			::ImmSetCompositionString(himc, SCS_QUERYRECONVERTSTRING, prcs, prcs->dwSize, nullptr, 0);
			Edit_SetSel(hwnd, lineStartingPos + prcs->dwCompStrOffset, lineStartingPos + prcs->dwCompStrOffset + prcs->dwCompStrLen);
			::ImmReleaseContext(hwnd, himc);
		}
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
	if (auto const hwndTarget = FindEditWindow())
	{
		oldWindowProc = SubclassWindow(hwndTarget, WindowProc);
	}
}
