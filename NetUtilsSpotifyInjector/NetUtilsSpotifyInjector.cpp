// Defines the exported functions for the DLL application... well, supposed to at least
// netutils is an injector; there's no need to export functions as Spotify will passively just
// look for this dll, and access the entry point normally, unlike chrome_elf,
// where exported functions are necessary to provide additional functions, 
// because chrome_elf is a replacer (basically a hacked version), 
// and netutils is an injectory 

#include "stdafx.h"


