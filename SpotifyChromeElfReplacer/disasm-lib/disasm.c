#undef NDEBUG
#include <assert.h>
#include <Windows.h>
#include "disasm.h"

#ifdef NO_SANITY_CHECKS
#define NDEBUG
#undef assert
#define assert(x)
#endif

ARCHITECTURE_FORMAT SupportedArchitectures[] =
{
	{ ARCH_X86,	&X86 },
	{ ARCH_X86_16, &X86 },
	{ ARCH_X64,	&X86 },
	{ ARCH_UNKNOWN, NULL }
};

typedef struct _DISASM_ARG_INFO
{
	INSTRUCTION *MatchedInstruction;
	BOOL MatchPrefix;
	U8 *Opcode;
	U32 OpcodeLength;
	INSTRUCTION_TYPE InstructionType;
	U32 Count;
} DISASM_ARG_INFO;

BOOL InitInstruction(INSTRUCTION *Instruction, DISASSEMBLER *Disassembler);
struct _ARCHITECTURE_FORMAT *GetArchitectureFormat(ARCHITECTURE_TYPE Type);

BOOL InitDisassembler(DISASSEMBLER *Disassembler, const ARCHITECTURE_TYPE Architecture)
{
	memset(Disassembler, 0, sizeof(DISASSEMBLER));
	Disassembler->Initialized = DISASSEMBLER_INITIALIZED;
	
	ARCHITECTURE_FORMAT* ArchFormat = GetArchitectureFormat(Architecture);
	if (!ArchFormat) { assert(0); return FALSE; }
	Disassembler->ArchType = ArchFormat->Type;
	Disassembler->Functions = ArchFormat->Functions;
	return TRUE;
}

void CloseDisassembler(DISASSEMBLER *Disassembler)
{
	memset(Disassembler, 0, sizeof(DISASSEMBLER));
}

BOOL InitInstruction(INSTRUCTION *Instruction, DISASSEMBLER *Disassembler)
{
	memset(Instruction, 0, sizeof(INSTRUCTION));
	Instruction->Initialized = INSTRUCTION_INITIALIZED;
	Instruction->Disassembler = Disassembler;
	memset(Instruction->String, ' ', MAX_OPCODE_DESCRIPTION-1);
	Instruction->String[MAX_OPCODE_DESCRIPTION-1] = '\0';
	return TRUE;
}

INSTRUCTION *GetInstruction(DISASSEMBLER *Disassembler, const U64 VirtualAddress, U8 *Address, const U32 Flags)
{
	if (Disassembler->Initialized != DISASSEMBLER_INITIALIZED) { assert(0); return NULL; }
	assert(Address);
	InitInstruction(&Disassembler->Instruction, Disassembler);
	Disassembler->Instruction.Address = Address;	
	Disassembler->Instruction.VirtualAddressDelta = VirtualAddress - (U64)Address;
	if (!Disassembler->Functions->GetInstruction(&Disassembler->Instruction, Address, Flags))
	{
		assert(Disassembler->Instruction.Address == Address);
		assert(Disassembler->Instruction.Length < MAX_INSTRUCTION_LENGTH);

		// Save the address that failed, in case the lower-level disassembler didn't
		Disassembler->Instruction.Address = Address;
		Disassembler->Instruction.ErrorOccurred = TRUE;
		return NULL;
	}
	return &Disassembler->Instruction;
}

static ARCHITECTURE_FORMAT *GetArchitectureFormat(const ARCHITECTURE_TYPE Type)
{
	for (ARCHITECTURE_FORMAT* Format = SupportedArchitectures; Format->Type != ARCH_UNKNOWN; Format++)
	{
		if (Format->Type == Type) return Format;
	}

	assert(0);
	return NULL;
}

