#include "misc.h"

BOOL IsHexChar(const BYTE ch)
{
	switch (ch)
	{
		case '0': case '1': case '2': case '3': 
		case '4': case '5': case '6': case '7': 
		case '8': case '9': 
		case 'A': case 'a': case 'B': case 'b':
		case 'C': case 'c': case 'D': case 'd':
		case 'E': case 'e': case 'F': case 'f':
			return TRUE;
		default:
			return FALSE;
	}
}

// caller must free the buffer returned
BYTE *HexToBinary(char *Input, DWORD InputLength, DWORD *OutputLength)
{
	DWORD i, j, ByteCount = 0;
	char temp_byte[3];
	BYTE*ByteString = NULL;

	if (!InputLength || !OutputLength) return NULL;
	else *OutputLength = 0;

	while (*Input && isspace(*Input)) { Input++; InputLength--; }
	if (!*Input) return NULL;
	if (Input[0] == '\"') { Input++; InputLength--; }
	BYTE* p = (BYTE*)strchr(Input, '\"');
	if (p) InputLength--;

	if (InputLength > 2 && Input[2] == ' ') // assume spaces
	{
		for (i = 0; i < InputLength; i += 3)
		{
			while (i < InputLength && isspace(Input[i])) i++; // skip over extra space, \r, and \n
			if (i >= InputLength) break;

			if (!IsHexChar(Input[i]))
			{
				goto abort;
			}

			if (i+1 >= InputLength || !Input[i+1])
			{
				goto abort;
			}

			if (i+2 < InputLength && Input[i+2] && !isspace(Input[i+2]))
			{
				goto abort;
			}

			ByteCount++;
		}

		if (!ByteCount)
		{
			goto abort;
		}

		ByteString = malloc(ByteCount+1);
		if (!ByteString)
		{
			goto abort;
		}
			
		memset(ByteString, 0, ByteCount+1);
		for (i = 0, j = 0; j < ByteCount; i += 3, j++)
		{			
			while (isspace(Input[i])) i++; // skip over extra space, \r, and \n
			temp_byte[0] = Input[i];
			temp_byte[1] = Input[i+1];
			temp_byte[2] = 0;
			ByteString[j] = (BYTE)strtoul(temp_byte, NULL, 16);
		}
	}
	else if (InputLength > 2 && Input[0] == '\\')
	{
		for (i = 0; i < InputLength; i += 2)
		{
			if (Input[i] != '\\' || (Input[i+1] != 'x' && Input[i+1] != '0'))
			{
				goto abort;
			}
			i += 2;

			if (!IsHexChar(Input[i]))
			{
				goto abort;
			}
			if (i+1 >= InputLength || !Input[i+1])
			{
				goto abort;
			}

			ByteCount++;
		}

		if (!ByteCount)
		{
			goto abort;
		}

		ByteString = malloc(ByteCount+1);
		if (!ByteString)
		{
			goto abort;
		}
			
		memset(ByteString, 0, ByteCount+1);
		for (i = j = 0; j < ByteCount; i += 2, j++)
		{
			i += 2;
			temp_byte[0] = Input[i];
			temp_byte[1] = Input[i+1];
			temp_byte[2] = 0;
			ByteString[j] = (BYTE)strtoul(temp_byte, NULL, 16);
		}
	}
	else // assume it is a hex string - no spaces - 2 bytes per character
	{
		for (i = 0; i < InputLength; i += 2)
		{
				if (!IsHexChar(Input[i]))
			{
				goto abort;
			}
			if (i+1 >= InputLength || !Input[i+1])
			{
				goto abort;
			}

			ByteCount++;
		}

		if (!ByteCount)
		{
			goto abort;
		}

		ByteString = malloc(ByteCount+1);
		if (!ByteString)
		{
			goto abort;
		}
			
		memset(ByteString, 0, ByteCount+1);
		for (i = 0, j = 0; j < ByteCount; i += 2, j++)
		{
			temp_byte[0] = Input[i];
			temp_byte[1] = Input[i+1];
			temp_byte[2] = 0;
			ByteString[j] = (BYTE)strtoul(temp_byte, NULL, 16);
		}
	}

	*OutputLength = ByteCount;
	return ByteString;

abort:
	if (OutputLength) *OutputLength = 0;
	if (ByteString) free(ByteString);
	return NULL;
}

