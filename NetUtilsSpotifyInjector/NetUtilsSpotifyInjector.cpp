// Defines the exported functions for the DLL application... well, supposed to at least
// netutils is an injector; there's no need to export functions as Spotify will passively just
// look for this dll, and access the entry point normally, unlike chrome_elf,
// where exported functions are necessary to provide additional functions, 
// because chrome_elf is a replacer (basically a hacked version), 
// and netutils is an injector

// necessary to invoke header into memory
#include "stdafx.h"

// the one exception to not exporting anything
// invoke the assembler multiple times in a row in a loop to create easy/lightweight memory offset in stack (and maybe even heap, due to how the iasm is intertwined with many pointers so the iasm actually works)
extern "C" __declspec(naked) void gc_invokeinlineassembler()
{
	for(auto i = 0; i != 3; i++)
	{
		__asm __asm __asm retn
	}
}