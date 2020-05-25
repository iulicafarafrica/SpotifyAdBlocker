// dll entry point
#include "stdafx.h"
#include "mhook-lib/mhook.h"
#include "hosts.h"
#include <WinSock2.h>
#include <iostream>
#include <algorithm>
#include <Psapi.h>
#include <vector>
#include <wchar.h>
#include <winuser.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Version.lib")

/*
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
} ---> external handle fix */

typedef int (WSAAPI* _getaddrinfo)(
	_In_opt_       PCSTR      pNodeName,
	_In_opt_       PCSTR      pServiceName,
	_In_opt_ const ADDRINFOA  *pHints,
	_Out_          PADDRINFOA *ppResult
	);

typedef int (WSAAPI* _WSASend)(
	_In_  SOCKET                             s,
	_In_  LPWSABUF                           lpBuffers,
	_In_  DWORD                              dwBufferCount,
	_Out_ LPDWORD                            lpNumberOfBytesSent,
	_In_  DWORD                              dwFlags,
	_In_  LPWSAOVERLAPPED                    lpOverlapped,
	_In_  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

static _getaddrinfo getaddrinfo_orig;
static _WSASend WSASend_orig;

int WSAAPI getaddrinfo_hook(
	_In_opt_ const PCSTR      pNodeName,
	_In_opt_ const PCSTR      pServiceName,
	_In_opt_ const ADDRINFOA  *pHints,
	_Out_          PADDRINFOA *ppResult)
{
	if (pNodeName)
	{
		for (size_t i = 0; i < sizeof(HostNames) / sizeof(HostNames[0]); i++)
		{
			if (!_strcmpi(pNodeName, HostNames[i]))
				return WSANO_RECOVERY;
		}
	}
	return getaddrinfo_orig(pNodeName, pServiceName, pHints, ppResult);
}

LPVOID Search(char* pPattern, const size_t patternSize, uint8_t* scanStart, size_t scanSize)
{
	__try
	{
		const auto res = std::search(
			scanStart, scanStart + scanSize, pPattern, pPattern + patternSize,
			[](const uint8_t val1, const uint8_t val2) { return (val1 == val2); }
		);

		return (res >= scanStart + scanSize) ? nullptr : res;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

bool DataCompare(BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

BYTE* FindPattern(BYTE* dwAddress, const DWORD dwSize, BYTE* pbSig, char* szMask)
{
	const DWORD length = strlen(szMask);
	for (DWORD i = NULL; i < dwSize - length; i++)
	{
		__try
		{
			if (DataCompare(dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}
	return nullptr;
}

#define HOST_STR "Host: "

int WSAAPI WSASend_hook(
	_In_ const SOCKET                             s,
	_In_ const LPWSABUF                           lpBuffers,
	_In_ const DWORD                              dwBufferCount,
	_Out_ const LPDWORD                            lpNumberOfBytesSent,
	_In_ const DWORD                              dwFlags,
	_In_ const LPWSAOVERLAPPED                    lpOverlapped,
	_In_ const LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	if (lpBuffers)
	{
		for (DWORD x = 0; x < dwBufferCount; x++)
		{
			const auto res = Search(HOST_STR, sizeof(HOST_STR) - 1, reinterpret_cast<uint8_t*>(lpBuffers[x].buf), lpBuffers[x].len);

			if (res)
			{
				const size_t max_len = reinterpret_cast<uint8_t*>(lpBuffers[x].buf) + lpBuffers[x].len - static_cast<uint8_t*>(res);

				for (size_t i = 0; i < sizeof(HostNames) / sizeof(HostNames[0]); i++)
				{
					const auto l = strlen(HostNames[i]);
					if (l < max_len && !_strnicmp(static_cast<char*>(res) + sizeof(HOST_STR) - 1, HostNames[i], l))
						return WSAENETUNREACH;
				}
			}
		}
	}

	return WSASend_orig(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

static std::string Utf16ToUtf8(const std::wstring& s)
{
	const auto size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);

	std::vector<char> buf(size);
	WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], size, nullptr, nullptr);

	return std::string(&buf[0]);
}

void __stdcall LoadAPI(LPVOID* destination, const LPCSTR apiName)
{
	if (*destination)
		return;
	
	wchar_t path[MAX_PATH];
	wchar_t windows[MAX_PATH];
	GetSystemDirectoryW(windows, MAX_PATH);
	const auto utf8strwcharpath = Utf16ToUtf8(path);
	const auto utf8strwcharwindows = Utf16ToUtf8(windows);
	const auto lppath = reinterpret_cast<LPSTR>(const_cast<char*>(utf8strwcharpath.c_str()));
	
	wsprintf(lppath, reinterpret_cast<LPCSTR>(L"%s\\netutils.dll"), windows);

	auto hModule = GetModuleHandle(reinterpret_cast<LPCSTR>(path));
	if (!hModule)
	{
		hModule = LoadLibrary(reinterpret_cast<LPCSTR>(path));
		if (!hModule)
		{
			return;
		}
	}
	*destination = GetProcAddress(hModule, apiName);
}

#define API_EXPORT_ORIG(N) \
	static LPVOID _##N = NULL;	\
	char S_##N[] = "" # N; \
	extern "C" __declspec(dllexport) __declspec(naked) void N ## () \
	{ \
		__asm pushad \
		__asm push offset S_##N \
		__asm push offset _##N \
		__asm call LoadAPI \
		__asm popad \
		__asm jmp [_##N] \
	} \

API_EXPORT_ORIG(NetApiBufferAllocate)
API_EXPORT_ORIG(NetApiBufferFree)
API_EXPORT_ORIG(NetApiBufferReallocate)
API_EXPORT_ORIG(NetApiBufferSize)
API_EXPORT_ORIG(NetRemoteComputerSupports)
API_EXPORT_ORIG(NetapipBufferAllocate)
API_EXPORT_ORIG(NetpIsComputerNameValid)
API_EXPORT_ORIG(NetpIsDomainNameValid)
API_EXPORT_ORIG(NetpIsGroupNameValid)
API_EXPORT_ORIG(NetpIsRemote)
API_EXPORT_ORIG(NetpIsRemoteNameValid)
API_EXPORT_ORIG(NetpIsShareNameValid)
API_EXPORT_ORIG(NetpIsUncComputerNameValid)
API_EXPORT_ORIG(NetpIsUserNameValid)
API_EXPORT_ORIG(NetpwListCanonicalize)
API_EXPORT_ORIG(NetpwListTraverse)
API_EXPORT_ORIG(NetpwNameCanonicalize)
API_EXPORT_ORIG(NetpwNameCompare)
API_EXPORT_ORIG(NetpwNameValidate)
API_EXPORT_ORIG(NetpwPathCanonicalize)
API_EXPORT_ORIG(NetpwPathCompare)
API_EXPORT_ORIG(NetpwPathType)

#define API_COPY(M, N) \
	_##N = GetProcAddress(M, #N);


typedef char (__fastcall* _is_skippable)(
	char* This,
	void*
);

typedef int(__fastcall* _can_focus)(
	char* This,
	void*
);

typedef int(__fastcall* _now_playing)(
	char* This,
	void*,
	void* Unk
);


static _is_skippable is_skippable_orig;
static _can_focus can_focus_orig;
static _now_playing now_playing_orig;

static DWORD dwCurrentTrackUriOffset = 0x0;
static LPVOID pfnSkippableStart = nullptr;
static char lastPlayingUri[2048] = {0};
static bool skipTrack = false;

__declspec(naked) void is_skippable_hook()
{
	__asm {
		mov eax, 1
		ret
	}
}

_declspec(naked) void can_focus_hook()
{
	__asm {
		xor eax, eax
		ret
	}
}

__declspec(naked) void skip_through_stack()
{
	__asm {
		mov eax, 2
		xor eax, eax 
		jmp eax
		xor eax, eax 
		mov eax, 1
		ret
	}
}

DWORD WINAPI SkipTrack(LPVOID)
{
	auto cnt = 0;
	while (skipTrack && cnt++ < 2)
	{
		Sleep(250);
		if (skipTrack)
		{
			keybd_event(VK_MEDIA_NEXT_TRACK, 0x0, KEYEVENTF_EXTENDEDKEY, NULL);
			keybd_event(VK_MEDIA_NEXT_TRACK, 0x0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, NULL);
			break;
		}
	}
	return 0;
}

int __fastcall now_playing_hook(char* This, void* Edx, void* Track)
{
	const auto szCurrentTrackUri = static_cast<char*>(*reinterpret_cast<void**>(static_cast<char*>(Track) + dwCurrentTrackUriOffset));

	__try
	{
		if (strncmp(szCurrentTrackUri, lastPlayingUri, 2048))
		{
			strncpy_s(lastPlayingUri, szCurrentTrackUri, 2048);

			// If the now playing track is an ad or interruption, immediately skip using old method (simulating a "skip" media button press)
			if (!strncmp(szCurrentTrackUri, "spotify:ad:", 11) || !strncmp(szCurrentTrackUri, "spotify:interruption:", 21))
			{
				skipTrack = true;
				CreateThread(nullptr, 0, SkipTrack, nullptr, 0, nullptr);
			}
			else
			{
				skipTrack = false;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}

	return now_playing_orig(This, Edx, Track);
}

LPVOID FindFunction(char* hModule, const DWORD hModuleSize, char* midFuncPtn, const int lenMidFuncPtn, const int seekBackCount, char* startFuncPtn, const int lenStartFuncPtn)
{
	const auto pfnAddr = Search(midFuncPtn, lenMidFuncPtn, reinterpret_cast<uint8_t*>(hModule), hModuleSize);
	if (!pfnAddr) return nullptr;
	char* pfnStart = nullptr;
	auto pfnCurrent = static_cast<char*>(pfnAddr) - seekBackCount;
	while ((pfnCurrent = static_cast<char*>(Search(startFuncPtn, lenStartFuncPtn, reinterpret_cast<uint8_t*>(pfnCurrent), hModule + hModuleSize - pfnCurrent))) &&
			pfnCurrent < pfnAddr)
	{
		pfnStart = pfnCurrent;
		pfnCurrent++;
	}

	return pfnStart;
}

static char* ZeroString = "0\0";

void Patch(const HMODULE hModule, const MODULEINFO mInfo)
{
	DWORD d;
	VirtualProtect(hModule, mInfo.SizeOfImage, PAGE_EXECUTE_READWRITE, &d);
	const LPVOID hEndOfModule = reinterpret_cast<uint8_t*>(hModule) + mInfo.SizeOfImage;

	// Hook skippable function (make all tracks skippable, and even if the tracks are skippable, skip some sanity checks to skip tracks faster)
	pfnSkippableStart = FindFunction(reinterpret_cast<char*>(hModule), mInfo.SizeOfImage, "\x74\x04\xc6\x45\xbf\x01\xf6\xc3\x02\x74\x0b", 11, 1024,
		"\x55\x8b\xec\x6a\xff", 5);
	
	// fix for 1.0.91.183 addresses
	if (!pfnSkippableStart)
		pfnSkippableStart = FindFunction(reinterpret_cast<char*>(hModule), mInfo.SizeOfImage, "\x8D\x46\x1C\xC7\x45\xB8\x01\x00\x00\x00\x50\x8D\x45\xC0\x50\xE8", 16, 1024,
			"\x55\x8b\xec\x6a\xff", 5);

	if (pfnSkippableStart)
	{
		is_skippable_orig = static_cast<_is_skippable>(pfnSkippableStart);
		Mhook_SetHook(reinterpret_cast<PVOID*>(&is_skippable_orig), is_skippable_hook);
	}

	// Hook now playing function (determine what current track is playing)
	auto szNowPlaying = Search("now_playing_uri\0", 16, reinterpret_cast<uint8_t*>(hModule), mInfo.SizeOfImage);
	if (szNowPlaying) {
		char szNowPlayingPattern[7];
		strcpy_s(szNowPlayingPattern, "\x6a\x0f\x68\x00\x00\x00\x00");
		memcpy(szNowPlayingPattern + 3, &szNowPlaying, sizeof(LPVOID));

		const auto pfnNowPlaying = FindFunction(reinterpret_cast<char*>(hModule), mInfo.SizeOfImage, szNowPlayingPattern, 7, 1024,
		                                        "\x55\x8b\xec\x6a\xff", 5);

		if (pfnNowPlaying)
		{
			auto pfnUriPtn = Search("\x6a\xff\x8d\x87", 4, static_cast<uint8_t*>(pfnNowPlaying), static_cast<char*>(hEndOfModule) - static_cast<char*>(pfnNowPlaying));

			// fix for 1.0.91.183 addresses
			if (!pfnUriPtn)
				pfnUriPtn = Search("\x74\x1a\x8d\x86", 4, static_cast<uint8_t*>(pfnNowPlaying), static_cast<char*>(hEndOfModule) - static_cast<char*>(pfnNowPlaying));

			if (pfnUriPtn)
			{
				dwCurrentTrackUriOffset = *reinterpret_cast<DWORD*>(static_cast<char*>(pfnUriPtn) + 4);
				now_playing_orig = static_cast<_now_playing>(pfnNowPlaying);
				Mhook_SetHook(reinterpret_cast<PVOID*>(&now_playing_orig), now_playing_hook);
			}
		}
	}

	// Hook focus function (disable focus for ads)
	LPVOID pfnRequireFocus = reinterpret_cast<uint8_t*>(hModule);
	while ((pfnRequireFocus = Search("\x8d\x46\x40\x50\x8d\x45\xc0\x50\xe8", 9, static_cast<uint8_t*>(pfnRequireFocus), static_cast<char*>(hEndOfModule) - static_cast<char*>(pfnRequireFocus))))
	{
		if (*(static_cast<char*>(pfnRequireFocus) - 5) == 0x68 &&
			!strcmp(static_cast<char*>(*reinterpret_cast<LPVOID*>(static_cast<char*>(pfnRequireFocus) - 4)), "require_focus"))
		{
			// Find the start of the function
			LPVOID pfnRequireFocusStart = nullptr;
			LPVOID pfnRequireFocusCurrent = static_cast<char*>(pfnRequireFocus) - 500;
			while ((pfnRequireFocusCurrent = Search("\x55\x8b\xec\x6a\xff", 5, static_cast<uint8_t*>(pfnRequireFocusCurrent), static_cast<char*>(hEndOfModule) - static_cast<char*>(pfnRequireFocusCurrent))) &&
				pfnRequireFocusCurrent < pfnRequireFocus)
			{
				pfnRequireFocusStart = pfnRequireFocusCurrent;
				pfnRequireFocusCurrent = static_cast<char*>(pfnRequireFocusCurrent) + 1;
			}
			if (pfnRequireFocusStart)
			{
				can_focus_orig = static_cast<_can_focus>(pfnRequireFocusStart);
				Mhook_SetHook(reinterpret_cast<PVOID*>(&can_focus_orig), can_focus_hook);
				break;
			}
		}
		pfnRequireFocus = static_cast<char*>(pfnRequireFocus) + 1;
	}

	auto cur = reinterpret_cast<uint8_t*>(hModule);
	const auto end = cur + mInfo.SizeOfImage;

	while (cur < end)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(cur, &mbi, sizeof(mbi));
		if (mbi.Protect & PAGE_EXECUTE_READ ||
			mbi.Protect & PAGE_EXECUTE_READWRITE ||
			mbi.Protect & PAGE_READWRITE ||
			mbi.Protect & PAGE_READONLY ||
			mbi.Protect & PAGE_EXECUTE_WRITECOPY)
		{
			// Patch 5 second minimum wait to skip video ads
			auto skipStuckSeconds = static_cast<LPVOID>(FindPattern(static_cast<uint8_t*>(mbi.BaseAddress), mbi.RegionSize,
			                                                        reinterpret_cast<BYTE*>(
				                                                        "\x83\xc4\x08\x6a\x00\x68\xe8\x03\x00\x00\xff\x70\x04\xff\x30\xe8\x00\x00\x00\x00\x8d\x4d\xc0"),
			                                                        "xxxxxxxxxxxxxxxx????xxx"));
			auto oneThousandMsOffset = 6;

			// fix for 1.0.91.183 addresses
			if (!skipStuckSeconds)
			{
				skipStuckSeconds = static_cast<LPVOID>(FindPattern(static_cast<uint8_t*>(mbi.BaseAddress), mbi.RegionSize,
				                                                   reinterpret_cast<BYTE*>("\xb9\xe8\x03\x00\x00\xf7\xe9\x83\xc4\x1c"), "xxxxxxxxxx"));
				oneThousandMsOffset = 1;
			}
			if (skipStuckSeconds)
			{
				DWORD oldProtect;
				VirtualProtect(static_cast<char*>(skipStuckSeconds) + oneThousandMsOffset, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
				*reinterpret_cast<DWORD*>(static_cast<char*>(skipStuckSeconds) + oneThousandMsOffset) = 0;
				VirtualProtect(static_cast<char*>(skipStuckSeconds) + oneThousandMsOffset, 4, oldProtect, &oldProtect);
				break;
			}
		}
		cur = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
	}
}

void PatchNet()
{
	auto hModule = GetModuleHandle(reinterpret_cast<LPCSTR>(L"ws2_32.dll"));
	if (!hModule)
		hModule = LoadLibrary(reinterpret_cast<LPCSTR>(L"ws2_32.dll"));

	if (hModule)
	{
		getaddrinfo_orig = reinterpret_cast<_getaddrinfo>(GetProcAddress(hModule, "getaddrinfo"));
		if (getaddrinfo_orig)
			Mhook_SetHook(reinterpret_cast<PVOID*>(&getaddrinfo_orig), getaddrinfo_hook);
		WSASend_orig = reinterpret_cast<_WSASend>(GetProcAddress(hModule, "WSASend"));
		if (WSASend_orig)
			Mhook_SetHook(reinterpret_cast<PVOID*>(&WSASend_orig), WSASend_hook);
	}
}

void PatchAdMain(const HMODULE hModule, const MODULEINFO mInfo)
{
	// fix for 1.0.91.183 addresses
	LPVOID adMissingIdAddr = FindPattern(reinterpret_cast<uint8_t*>(hModule), mInfo.SizeOfImage, reinterpret_cast<BYTE*>("\x84\xC0\x0F\x85\x00\x00\x00\x00\x6A\x0D\x68"), "xxxx????xxx");
	auto adMissingNopOffset = 2;
	auto adMissingNopCount = 6;

	// fallback old version
	if (!adMissingIdAddr) {
		adMissingIdAddr = FindPattern(reinterpret_cast<uint8_t*>(hModule), mInfo.SizeOfImage, reinterpret_cast<BYTE*>("\x84\xc0\x75\x00\x6a\x0d\x68"), "xxx?xxx");
		adMissingNopOffset = 2, adMissingNopCount = 2;
	}
	if (adMissingIdAddr)
	{
		DWORD oldProtect;
		VirtualProtect(static_cast<char*>(adMissingIdAddr) + adMissingNopOffset, adMissingNopCount, PAGE_EXECUTE_READWRITE, &oldProtect);
		memset(static_cast<char*>(adMissingIdAddr) + adMissingNopOffset, 0x90, adMissingNopCount);
		VirtualProtect(static_cast<char*>(adMissingIdAddr) + adMissingNopOffset, adMissingNopCount, oldProtect, &oldProtect);
	}
}

void WriteAll(const HMODULE hModule, const MODULEINFO mInfo)
{
	auto cur = reinterpret_cast<uint8_t*>(hModule);
	const auto end = cur + mInfo.SizeOfImage;

	while (cur < end)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(cur, &mbi, sizeof(mbi));
		if (!(mbi.Protect & PAGE_GUARD)) {
			DWORD dwOldProtect;
			if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect) &&
				mbi.Type & MEM_MAPPED)
				VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
		}
		cur = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
	}
}

typedef struct
{
	DWORD dwMajor;
	DWORD dwMinor;
	DWORD dwBuild;
	DWORD dwRevision;
} version_t;

typedef struct {
	WORD             wLength;
	WORD             wValueLength;
	WORD             wType;
	WCHAR            szKey[16];
	WORD             Padding1;
	VS_FIXEDFILEINFO Value;
	WORD             Padding2;
	WORD             Children;
} VS_VERSIONINFO;

BOOL GetFileVersionInfo(version_t* v)
{
	auto ok = FALSE;
	WCHAR moduleFilePath[MAX_PATH];
	DWORD verHandle;
	GetModuleFileName(GetModuleHandle(nullptr), reinterpret_cast<LPSTR>(moduleFilePath), MAX_PATH);
	auto verSize = GetFileVersionInfoSize(reinterpret_cast<LPSTR>(moduleFilePath), &verHandle);
	if (verSize)
	{
		LPVOID verBuffer;
		UINT size;
		const LPVOID verData = new char[verSize];
		if (GetFileVersionInfo(reinterpret_cast<LPCSTR>(moduleFilePath), verHandle, verSize, verData) &&
			VerQueryValueA(verData, "\\", &verBuffer, &size) &&
			size)
		{
			auto verInfo = static_cast<VS_VERSIONINFO*>(verData);
			if (verInfo->Value.dwSignature == 0xfeef04bd)
			{
				v->dwMajor = verInfo->Value.dwFileVersionMS >> 16 & 0xffff;
				v->dwMinor = verInfo->Value.dwFileVersionMS & 0xffff;
				v->dwBuild = verInfo->Value.dwFileVersionLS >> 16 & 0xffff;
				v->dwRevision = verInfo->Value.dwFileVersionLS & 0xffff;
				ok = TRUE;
			}
		}
		// dereference pointer
		delete static_cast<char*>(verData);
	}
	return ok;
}

DWORD WINAPI MainThread(LPVOID)
{
	// Block known ad hosts via function hooks
	__try {
		PatchNet();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	const auto hModule = GetModuleHandle(nullptr);
	MODULEINFO mInfo = { nullptr };
	if (GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
		// Attempt to make entire module writable
		__try {
			WriteAll(hModule, mInfo);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		// Perform fallback patches (just in-case the main method fails)
		// Only allow for version 1.1.0.xx and below
		version_t v;
		if (GetFileVersionInfo(&v) && v.dwMajor <= 1 && v.dwMinor <= 1 && v.dwBuild <= 0)
		{
			__try {
				Patch(hModule, mInfo);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}

		// Perform main ad patch
		__try {
			PatchAdMain(hModule, mInfo);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return 0;
}

// Entry-point
BOOL APIENTRY DllMain(const HMODULE hModule,
                      const DWORD  ul_reason_for_call,
                      LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		// Only patch the main process and none of the renderers/workers
		if (!wcsstr(reinterpret_cast<wchar_t const*>(GetCommandLine()), L"--type="))
			CreateThread(nullptr, NULL, MainThread, nullptr, 0, nullptr);
		break;
	}
	return TRUE;
}

