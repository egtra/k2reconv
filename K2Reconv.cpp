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

	//選択範囲（キャレット）を含む行を探す（その行にある文字列すべてを得るため）
	auto lineNumberOfStartPos = Edit_LineFromChar(hwnd, -1);

	//行末を探す（EM_GETLINEがWORDで文字列指定するため、WORDの最大値を上限としている）
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
	auto selectedLength = last - first; //選択部分の長さ（選択範囲がなければそのまま0で良い）
	LRESULT lr = sizeof (RECONVERTSTRING) + lineLength + 1;
	if (prcs != nullptr)
	{
		std::vector<TCHAR> buf(lineLength);
		LRESULT bufLen = Edit_GetLine(hwnd, lineNumberOfStartPos, buf.data(), lineLength);
		buf.resize(static_cast<std::size_t>(bufLen));

		//必要な情報の設定
		//1行分の文字列と変換部分のオフセットを渡すようにしている
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
