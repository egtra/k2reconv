#include "stdafx.h"

void InitializeReconv();

namespace
{

HMODULE hmodK2RegexpOriginal;

typedef int (*PFNBMatch)(char* str,char *target, char *targetstartp, char *targetendp, int one_shot,
		BREGEXP **rxp,char *msg);
typedef int (*PFNBSubst)(char* str, char *target, char *targetstartp, char *targetendp,
		BREGEXP **rxp, char *msg, BCallBack callback);
typedef int (*PFNBTrans)(char* str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);
typedef int (*PFNBSplit)(char* str, char *target, char *targetendp,
		int limit, BREGEXP **rxp, char *msg);
typedef void (*PFNBRegfree)(BREGEXP* rx);
typedef char* (*PFNBRegexpVersion)(void);

PFNBMatch pfnBMatchOriginal;
PFNBSubst pfnBSubstOriginal;
PFNBTrans pfnBTransOriginal;
PFNBSplit pfnBSplitOriginal;
PFNBRegfree pfnBRegfreeOriginal;
PFNBRegexpVersion pfnBRegexpVersionOriginal;

#define INIT(name) (pfn ## name ## Original = reinterpret_cast<PFN ## name>(GetProcAddress(hmodK2RegexpOriginal, #name)))
void Initialize()
{
	if (hmodK2RegexpOriginal != NULL)
	{
		return;
	}

	hmodK2RegexpOriginal = LoadLibrary(TEXT("K2Regexp-Original.dll"));

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
