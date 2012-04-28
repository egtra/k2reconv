#include "stdafx.h"
#include <array>
#include <Shlwapi.h>

void InitializeReconv();
extern HMODULE hModuleSelf;

namespace
{

HMODULE hmodK2RegexpOriginal;

#define DEFINE(name) decltype(&name) pfn ## name ## Original
#define INIT(name) (pfn ## name ## Original = reinterpret_cast<decltype(&name)>(GetProcAddress(hmodK2RegexpOriginal, #name)))

DEFINE(BMatch);
DEFINE(BSubst);
DEFINE(BTrans);
DEFINE(BSplit);
DEFINE(BRegfree);
DEFINE(BRegexpVersion);

void Initialize()
{
	if (hmodK2RegexpOriginal != NULL)
	{
		return;
	}

	std::array<TCHAR, MAX_PATH> path;
	::GetModuleFileName(hModuleSelf, path.data(), MAX_PATH);
	::PathRemoveFileSpec(path.data());
	::PathAppend(path.data(), TEXT("K2Regexp-Original.dll"));
	hmodK2RegexpOriginal = LoadLibrary(path.data());

	INIT(BMatch);
	INIT(BSubst);
	INIT(BTrans);
	INIT(BSplit);
	INIT(BRegfree);
	INIT(BRegexpVersion);

	InitializeReconv();
}

} // end of unnamed namespace

int BMatch(char* str, char *target, char *targetstartp, char *targetendp, int one_shot, BREGEXP **rxp, char *msg)
{
	Initialize();
	return pfnBMatchOriginal(str, target, targetstartp, targetendp, one_shot, rxp, msg);
}

int BSubst(char* str, char *target, char *targetstartp, char *targetendp, BREGEXP **rxp, char *msg, BCallBack callback)
{
	Initialize();
	return pfnBSubstOriginal(str, target, targetstartp, targetendp, rxp, msg, callback);
}

int BTrans(char* str, char *target, char *targetendp, BREGEXP **rxp, char *msg)
{
	Initialize();
	return pfnBTransOriginal(str, target, targetendp, rxp, msg);
}

int BSplit(char* str, char *target, char *targetendp, int limit, BREGEXP **rxp, char *msg)
{
	Initialize();
	return pfnBSplitOriginal(str, target, targetendp, limit, rxp, msg);
}

void BRegfree(BREGEXP* rx)
{
	Initialize();
	pfnBRegfreeOriginal(rx);
}

char* BRegexpVersion(void)
{
	Initialize();
	return pfnBRegexpVersionOriginal();
}
