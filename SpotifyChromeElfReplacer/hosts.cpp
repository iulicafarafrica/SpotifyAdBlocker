#include "stdafx.h"

// internet proxy auto discovery
bool g_skip_wpad = false;
// Logging system
std::wofstream logging;

_getaddrinfo getaddrinfo_orig;

// check if ads hostname
bool is_blockhost (const char* nodename) {
	const std::string nnodename (nodename);

	if (nnodename.compare("wpad") == 0)
		return g_skip_wpad ? true : false;

	if (std::string::npos != nnodename.find ("google"))
		return true;
	if (std::string::npos != nnodename.find ("doubleclick"))
		return true;
	if (std::string::npos != nnodename.find ("qualaroo.com"))
		return true;
	return false;
}

int WSAAPI getaddrinfo_hook (
	_In_opt_	const char* nodename,
	_In_opt_	const char* servname,
	_In_opt_	const struct addrinfo* hints,
	_Out_		struct addrinfo** res)
{
	auto isblock = std::async (std::launch::async, is_blockhost, nodename);
	const auto result = getaddrinfo_orig (nodename, servname, hints, res);
	if (result == 0) {
		if (isblock.get ()) {
			for (auto* ptr = *res; nullptr != ptr; ptr = ptr->ai_next) {
				auto* const ipv4 = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
				ipv4->sin_addr.S_un.S_addr = INADDR_ANY;
			}
			if (logging.is_open ())
				logging << "blocked - " << nodename << std::endl;
		}
		else {
			if (logging.is_open ())
				logging << "allowed - " << nodename << std::endl;
		}
	}
	return result;
}

