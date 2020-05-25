#include "stdafx.h"

extern std::wofstream logging;

bool DataCompare (BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

BYTE* FindPattern (BYTE* dwAddress, const DWORD dwSize, BYTE* pbSig, char* szMask)
{
	const DWORD length = strlen (szMask);
	for (DWORD i = NULL; i < dwSize - length; i++)
	{
		__try
		{
			if (DataCompare (dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}
	return nullptr;
}

DWORD WINAPI KillBanner (LPVOID)
{
	auto* const hModule = GetModuleHandle (nullptr);
	MODULEINFO mInfo = { nullptr };
	if (GetModuleInformation (GetCurrentProcess (), hModule, &mInfo, sizeof (MODULEINFO))) {
		
		const LPVOID skipPod = FindPattern (reinterpret_cast<uint8_t*>(hModule), mInfo.SizeOfImage, reinterpret_cast<BYTE*>("\x83\xC4\x08\x84\xC0\x0F\x84\x00\x04\x00\x00"), "xxxxxxxxxxx");
		
		if (skipPod)
		{
			DWORD oldProtect;
			VirtualProtect (static_cast<char*>(skipPod) + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset (static_cast<char*>(skipPod) + 5, 0x90, 1);
			VirtualProtect (static_cast<char*>(skipPod) + 5, 1, oldProtect, &oldProtect);

			VirtualProtect (static_cast<char*>(skipPod) + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset (static_cast<char*>(skipPod) + 6, 0xE9, 1);
			VirtualProtect (static_cast<char*>(skipPod) + 6, 1, oldProtect, &oldProtect);
			if (logging.is_open ())
				logging << "main process - patch succeeded!" << std::endl;
		}
		else {
			if (logging.is_open ())
				logging << "main process - patch failed... :(" << std::endl;
		}

	}
	return 0;
}
