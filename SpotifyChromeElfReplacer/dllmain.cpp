// dllmain.cpp: dll entry point
#include "stdafx.h"

extern bool g_skip_wpad;
extern std::wofstream logging;
extern _getaddrinfo getaddrinfo_orig;

static const char* configFile = "./config.ini";

void init_log (const char* logFile)
{
	if (GetPrivateProfileInt("Config", "Log", 0, configFile) == 1) {
		logging.open (logFile, std::ios::out | std::ios::app);
		logging << "SpotifyChromeElfReplacer - Build date: " << __TIMESTAMP__ << std::endl;
	}
}

BOOL APIENTRY DllMain (const _In_ HMODULE hModule,
					   const _In_ DWORD  ul_reason_for_call,
					   _In_ LPVOID lpReserved
)
{
	DisableThreadLibraryCalls (hModule);
	const std::string procname = GetCommandLine ();
	// only Spotify process - this helps avoid false positives
	if (std::string::npos != procname.find ("Spotify.exe")) {
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			if (std::string::npos == procname.find ("--type=")) {
				// block ads - main process
				CreateThread (nullptr, NULL, KillBanner, nullptr, 0, nullptr);
				init_log ("main_log.txt");
			}
			else if (std::string::npos != procname.find ("--type=utility")) {
				// block ads - utility process
				init_log ("utility_log.txt");
				getaddrinfo_orig = reinterpret_cast<_getaddrinfo>(GetProcAddress(GetModuleHandle("ws2_32.dll"), "getaddrinfo"));
				if (getaddrinfo_orig) {
					Mhook_SetHook (reinterpret_cast<PVOID*>(&getaddrinfo_orig), getaddrinfo_hook);
					if (logging.is_open ())
						logging << "Mhook_SetHook - getaddrinfo success!" << std::endl;
				}
				else {
					if (logging.is_open ())
						logging << "Mhook_SetHook - getaddrinfo failed!" << std::endl;
				}
				if (1 == GetPrivateProfileInt ("Config", "Skip_wpad", 0, configFile))
					g_skip_wpad = true;
			}
			break;
		case DLL_PROCESS_DETACH:
			if (logging.is_open ()) {
				logging.flush ();
				logging.close ();
			}
			break;
		default:
			if (ul_reason_for_call > 1)
			{
				// todo: add special case for thread cases (DLL_THREAD_ATTACH/DETACH) (??)
			}
			break;
		}
		
	}
	return TRUE;
}

