#include <windows.h>
#include <winuser.h>

#define	patternLength 32
BYTE	pattern[] = {0x83,0x3D,0xAC,0xA2,0x26,0x01,0x00,0x56,0x74,0x76,0xA1,0xBC,0xA7,0x26,0x01,0x83,0x78,0x54,0x00,0x74,0x6B,0x8B,0x40,0x54,0x83,0x78,0x40,0x00,0x74,0x62,0x8B,0x40};

bool DataCompare(const BYTE* OpCodes, const BYTE* Mask, const char* StrMask)
{
	while (*StrMask)
	{
		if (*StrMask == 'x' && *OpCodes != *Mask)
			return false;
		++StrMask;
		++OpCodes;
		++Mask;
	}
	return true;
}

DWORD FindPattern(DWORD StartAddress, DWORD CodeLen, BYTE* Mask, char* StrMask, unsigned short ignore)
{
	unsigned short Ign = 0;
	DWORD i = 0;
	while (Ign <= ignore)
	{
		if (DataCompare((BYTE*)(StartAddress + i++), Mask, StrMask))
			++Ign;
		else if (i >= CodeLen)
			return 0;
	}
	return StartAddress + i - 1;
}

DWORD WINAPI Start(LPVOID lpParam)
{
	char patternMask[patternLength + 1];
	for (int i = 0; i < patternLength; i++) patternMask[i] = 'x';
	patternMask[patternLength] = 0;
	DWORD target;
	for (int count = 0; count < 10; count++) {
		target = FindPattern(0x401000, 0xE52000, pattern, patternMask, 0);
		if (target) 
			break; 
		else 
			Sleep(300); //pattern not found, try again in 300ms, max 10 times
	}
	if (target) {
		//pattern found, remove write protection and overwrite target code
		DWORD originalProtection;
		VirtualProtect((void*)target, patternLength, PAGE_EXECUTE_READWRITE, &originalProtection);
		BYTE* ptr = (BYTE*)target;
		ptr[0x0F] = 0x85;
		ptr[0x10] = 0xC0;
		ptr[0x11] = 0x74;
		ptr[0x12] = 0x6D;
		ptr[0x13] = 0xEB;
		ptr[0x14] = 0x6F;
		ptr[0x84] = 0x83;
		ptr[0x85] = 0x78;
		ptr[0x86] = 0x54;
		ptr[0x87] = 0x00;
		ptr[0x88] = 0x74;
		ptr[0x89] = 0xF6;
		ptr[0x8A] = 0xEB;
		ptr[0x8B] = 0x89;
		VirtualProtect((void*)target, patternLength, originalProtection, NULL);
		return 0;
	} else {
		//pattern not found, display error
		MessageBox(0, "Kasumi crash pattern NOT FOUND! Fix not applied!", "ME2KasumiCrashFix", MB_OK | MB_ICONWARNING);
		return 0;
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	return 1;
}