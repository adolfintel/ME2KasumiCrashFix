#include <windows.h>
#include <winuser.h>

#define	PATTERN_LENGTH 32
BYTE	pattern[] = {0x83,0x3D,0xAC,0xA2,0x26,0x01,0x00,0x56,0x74,0x76,0xA1,0xBC,0xA7,0x26,0x01,0x83,0x78,0x54,0x00,0x74,0x6B,0x8B,0x40,0x54,0x83,0x78,0x40,0x00,0x74,0x62,0x8B,0x40};

DWORD FindPattern(DWORD start, DWORD codeLength, BYTE* pattern, unsigned int patternLength) {
	unsigned int matchLength = 0;
	for (DWORD ptr = start; ptr <= start + codeLength - patternLength; ptr++) {
		while (((BYTE*)ptr)[matchLength] == pattern[matchLength]) {
			matchLength++;
			if (matchLength == patternLength) return ptr;
		}
		matchLength = 0;
	}
	return NULL;
}

DWORD WINAPI Start(LPVOID lpParam)
{
	DWORD target;
	for (int count = 0; count < 10; count++) {
		target = FindPattern(0x401000, 0xE52000, pattern, PATTERN_LENGTH);
		if(!target) Sleep(300); //pattern not found, try again in 300ms, max 10 times
	}
	if (target) {
		//pattern found, remove write protection and overwrite target code
		DWORD originalProtection;
		VirtualProtect((void*)target, PATTERN_LENGTH, PAGE_EXECUTE_READWRITE, &originalProtection);
		((BYTE*)target)[0x0F] = 0x85;
		((BYTE*)target)[0x10] = 0xC0;
		((BYTE*)target)[0x11] = 0x74;
		((BYTE*)target)[0x12] = 0x6D;
		((BYTE*)target)[0x13] = 0xEB;
		((BYTE*)target)[0x14] = 0x6F;
		((BYTE*)target)[0x84] = 0x83;
		((BYTE*)target)[0x85] = 0x78;
		((BYTE*)target)[0x86] = 0x54;
		((BYTE*)target)[0x87] = 0x00;
		((BYTE*)target)[0x88] = 0x74;
		((BYTE*)target)[0x89] = 0xF6;
		((BYTE*)target)[0x8A] = 0xEB;
		((BYTE*)target)[0x8B] = 0x89;
		VirtualProtect((void*)target, PATTERN_LENGTH, originalProtection, NULL);
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