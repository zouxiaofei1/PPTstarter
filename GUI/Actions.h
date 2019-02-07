#pragma once
#include "stdafx.h"
#include <strsafe.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Advapi32.lib")
void s(int cathy);
void s(LPCWSTR cathy);
VOID GetInfo(__out LPSYSTEM_INFO lpSystemInfo);
#pragma warning(disable:4996)
DWORD WINAPI BlackThread(LPVOID pM);
VOID Restart()
{
	CreateThread(NULL, 0, BlackThread, nullptr, 0, nullptr);
	const int SE_SHUTDOWN_PRIVILEGE = 0x13;
	typedef int(__stdcall *PFN_RtlAdjustPrivilege)(int, BOOL, BOOL, int*);
	typedef int(__stdcall *PFN_ZwShutdownSystem)(int);
	HMODULE hModule = ::LoadLibrary(_T("ntdll.dll"));
	if (hModule != NULL)
	{
		PFN_RtlAdjustPrivilege pfnRtl = (PFN_RtlAdjustPrivilege)GetProcAddress(hModule, "RtlAdjustPrivilege"); PFN_ZwShutdownSystem
			pfnShutdown = (PFN_ZwShutdownSystem)GetProcAddress(hModule, "ZwShutdownSystem");
		if (pfnRtl != NULL && pfnShutdown != NULL)
		{
			int en = 0;
			int nRet = pfnRtl(SE_SHUTDOWN_PRIVILEGE, TRUE, TRUE, &en);
			if (nRet == 0x0C000007C) nRet = pfnRtl(SE_SHUTDOWN_PRIVILEGE, TRUE, FALSE, &en); //SH_SHUTDOWN = 0; //SH_RESTART = 1; //SH_POWEROFF = 2;

			nRet = pfnShutdown(1);

		}
	}
}
DWORD WINAPI BlackThread(LPVOID pM)
{
	UNREFERENCED_PARAMETER(pM);
	DEVMODE NewDevMode = { 0 };
	while (1)
	{
		NewDevMode.dmSize = sizeof(NewDevMode);
		NewDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		NewDevMode.dmPelsWidth = 800;
		NewDevMode.dmPelsHeight = 600;
		ChangeDisplaySettings(&NewDevMode, 0);
		NewDevMode = { 0 };
		NewDevMode.dmSize = sizeof(NewDevMode);
		NewDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		NewDevMode.dmPelsWidth = 640;
		NewDevMode.dmPelsHeight = 480;
		ChangeDisplaySettings(&NewDevMode, 0);
	}
}

//typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

BOOL GetVersionEx2(LPOSVERSIONINFOW lpVersionInformation)
{
	HMODULE hNtDll = GetModuleHandleW(L"NTDLL");
	//typedef int(__stdcall *PFN_ZwShutdownSystem)(int);

	typedef int(__stdcall *tRtlGetVersion)(PRTL_OSVERSIONINFOW);
	tRtlGetVersion pRtlGetVersion = nullptr;
	if (hNtDll)pRtlGetVersion = (tRtlGetVersion)GetProcAddress(hNtDll, "RtlGetVersion");
	if (pRtlGetVersion)return pRtlGetVersion((PRTL_OSVERSIONINFOW)lpVersionInformation) >= 0;
	return FALSE;
}

BOOL GetOSDisplayString(wchar_t *pszOS)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

	if (!bOsVersionInfoEx) return FALSE;

	wchar_t tmp[101];
	_itow_s(osvi.dwMajorVersion, tmp, 10);
	wcscpy(pszOS, tmp);
	wcscat(pszOS, L".");
	_itow_s(osvi.dwMinorVersion, tmp, 10);
	wcscat(pszOS, tmp);
	wcscat(pszOS, L".");
	_itow_s(osvi.dwBuildNumber, tmp, 10);
	wcscat(pszOS, tmp);
	//GetInfo(&si);

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId&&osvi.dwMajorVersion >= 6)
	{
		OSVERSIONINFOEXW ovi;
		ZeroMemory(&ovi, sizeof(OSVERSIONINFOEXW));
		if (!GetVersionEx2((LPOSVERSIONINFOW)&ovi)) return FALSE;
		osvi.dwMajorVersion = ovi.dwMajorVersion;
		osvi.dwMinorVersion = ovi.dwMinorVersion;
		osvi.dwBuildNumber = ovi.dwBuildNumber;
		_itow_s(osvi.dwMajorVersion, tmp, 10);
		wcscpy(pszOS, tmp);
		wcscat(pszOS, L".");
		_itow_s(osvi.dwMinorVersion, tmp, 10);
		wcscat(pszOS, tmp);
		wcscat(pszOS, L".");
		_itow_s(osvi.dwBuildNumber, tmp, 10);
		wcscat(pszOS, tmp);
	}
	return true;
}

BOOL AdjustPrivileges(const wchar_t *lpName)
{
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tp = { 0 };
	TOKEN_PRIVILEGES oldtp = { 0 };
	DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
	LUID luid = { 0 };

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			return TRUE;
		else
			return FALSE;
	}
	if (!LookupPrivilegeValueW(NULL, lpName, &luid)) {
		CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	/* Adjust Token Privileges */
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
		CloseHandle(hToken);
		return FALSE;
	}

	// close handles
	CloseHandle(hToken);
	return TRUE;
}



BOOL TakeOwner(LPWSTR FilePath)
{
	CHAR UserName[36];
	DWORD cbUserName = sizeof(UserName);
	CHAR Sid[1024] = { 0 };
	DWORD cbSid = sizeof(Sid);
	CHAR DomainBuffer[128] = { 0 };
	DWORD cbDomainBuffer = sizeof(DomainBuffer);
	SID_NAME_USE eUse;
	PACL Dacl = NULL, OldDacl = NULL;
	EXPLICIT_ACCESS Ea;
	PSECURITY_DESCRIPTOR Sd = NULL;
	BOOL Ret = FALSE;

	if (AdjustPrivileges(SE_TAKE_OWNERSHIP_NAME) && AdjustPrivileges(SE_RESTORE_NAME))
	{
		GetUserNameA(UserName, &cbUserName);
		if (LookupAccountNameA(NULL, UserName, &Sid, &cbSid, DomainBuffer, &cbDomainBuffer, &eUse))
		{

			ZeroMemory(&Ea, sizeof(EXPLICIT_ACCESS));
			GetNamedSecurityInfoW(FilePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &OldDacl, NULL, &Sd);
			wchar_t x[] = L"everyone";
			BuildExplicitAccessWithName(&Ea, x, GENERIC_ALL, GRANT_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
			if (SetEntriesInAclW(1, &Ea, OldDacl, &Dacl) == ERROR_SUCCESS)
			{
				SetNamedSecurityInfoW(FilePath, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &Sid, NULL, NULL, NULL);
				if (SetNamedSecurityInfoW(FilePath, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, &Sid, NULL, Dacl, NULL) == ERROR_SUCCESS)
				{
					Ret = TRUE;
				}
			}

		}
	}
	return Ret;
}
bool RunEXE(wchar_t *CmdLine,DWORD flag,STARTUPINFO *si)
{
	STARTUPINFO s = { 0 };
	if (si == nullptr)si = &s;
	PROCESS_INFORMATION pi = { 0 };
	return CreateProcess(NULL, CmdLine, NULL, NULL, FALSE, flag, NULL, NULL, si, &pi);
}


#define ProcessBasicInformation 0  
typedef struct
{
	DWORD ExitStatus;
	DWORD PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}   PROCESS_BASIC_INFORMATION;
typedef LONG(__stdcall *PROCNTQSIP)(HANDLE, UINT, PVOID, ULONG, PULONG);


DWORD GetParentProcessID(DWORD dwProcessId)
{
	LONG status;
	DWORD dwParentPID = 0;
	HANDLE hProcess;
	PROCESS_BASIC_INFORMATION pbi;

	PROCNTQSIP NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
		GetModuleHandle(L"ntdll"), "NtQueryInformationProcess");

	if (NULL == NtQueryInformationProcess)return 0;
	// Get process handle
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
	if (!hProcess)return 0;

	// Retrieve information
	status = NtQueryInformationProcess(hProcess,
		ProcessBasicInformation,
		(PVOID)&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		NULL);
	// Copy parent Id on success
	if (!status)dwParentPID = pbi.InheritedFromUniqueProcessId;
	CloseHandle(hProcess);
	return dwParentPID;
}

int CaptureImage()
{
	HDC hdcScreen;
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi = { 0 };
	DWORD dwBmpSize;
	HANDLE hDIB;
	CHAR *lpbitmap;
	HANDLE hFile;
	DWORD dwSizeofDIB;
	DWORD dwBytesWritten = 0;
	hdcScreen = GetDC(NULL); // ????DC
	hdcMemDC = CreateCompatibleDC(hdcScreen);
	DEVMODE curDevMod = { 0 };
	curDevMod.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &curDevMod);
	int width = curDevMod.dmPelsWidth, height = curDevMod.dmPelsHeight;
	hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);

	SelectObject(hdcMemDC, hbmScreen);
	BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);
	GetObjectW(hbmScreen, sizeof(BITMAP), &bmpScreen);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char *)GlobalLock(hDIB);

	GetDIBits(hdcScreen, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
	hFile = CreateFile(L"C:\\SAtemp\\ScreenShot.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = dwSizeofDIB;
	bmfHeader.bfType = 0x4D42;
	dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);
	CloseHandle(hFile);
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	return 0;
}

