// set some stuff

// set the SLWA type
bool force32 = false;

#if defined(linux) || defined(__linux) || defined(__linux__)
#undef __LINUX__
#define __LINUX__   1
#endif
#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
/* Try to find out if we're compiling for WinRT or non-WinRT */
/* If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset. */
#if (defined(_MSC_VER) && (_MSC_VER >= 1700) && !_USING_V110_SDK71_)	/* _MSC_VER==1700 for MSVC 2012 */
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#undef __WINDOWS__
#define __WINDOWS__   1
/* See if we're compiling for WinRT: */
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#undef __WINRT__
#define __WINRT__ 1
#endif
#else
#undef __WINDOWS__
#define __WINDOWS__   1
#endif /* _MSC_VER < 1700 */
#endif /* defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) */
#if defined(__WINDOWS__)
#undef __WIN32__
#define __WIN32__ 1
#endif
#if defined(__APPLE__)
/* lets us know what version of Mac OS X we're compiling on */
#include "AvailabilityMacros.h"
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
/* if compiling for iPhone */
#undef __IPHONEOS__
#define __IPHONEOS__ 1
#undef __MACOSX__
#else
/* if not compiling for iPhone */
#undef __MACOSX__
#define __MACOSX__  1
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
# error SDL for Mac OS X only supports deploying on 10.5 and above.
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED < 1050 */
#endif /* TARGET_OS_IPHONE */
#endif /* defined(__APPLE__) */


#ifdef __WINDOWS__
#include <windows.h>
#define ERRORMAX 33
#define RUNCOMMANDLINE(cm,fl,prm,dir) int(ShellExecute(GetDesktopWindow(), L##cm, L##fl, L##prm, L##dir, SW_SHOWNORMAL))
typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
			//be save and return false
			return false;
		}
	}
	return bIsWow64;
}

#define IDI_ICON1 1;
#elif defined(__LINUX__)
//path = start path (should be projects/defuault)
//cmd = command to runt should be open ../../bin/$PROARCH/Rival.exe or ../../bin/Rival.exe
//args = -u../$(ProjectNAME) -glog.txt
BOOL IsWow64()
static gint sh_cmd(gchar * path, gchar * cmd, gchar * args)
{
	gchar     cmd_line[256];
	gchar   **argv;
	gint      argp;
	gint      rc = 0;

	if (cmd == NULL)
		return FALSE;

	if (cmd[0] == '\0')
		return FALSE;

	if (path != NULL) 
		chdir(path);

	snprintf(cmd_line, sizeof(cmd_line), "%s %s", cmd, args);

	rc = g_shell_parse_argv(cmd_line, &argp, &argv, NULL);
	if (!rc)
	{
		g_strfreev(argv);
		return rc;
	}

	rc = g_spawn_async(path, argv, NULL,
		G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);

	g_strfreev(argv);

	return rc;
}
#define RUNCOMMANDLINE(cm,fl,prm,dir) sh_cmd(dir,cmd, fl##" "##prm) //probably wont work fl and prm need to be one concat gchar * // also may need to convert from gint to int
#elif defined(__MACOSX__)
#define RUNCOMMANDLINE(cm,fl,prm,dir){ } //need to change the directory then open the file, not sure how to do this on ios
#endif
bool trytoopen()
{
	int a;
	if (!force32 && IsWow64())
		a = RUNCOMMANDLINE("open", "../../bin\\x64\\Rival.exe", "\"-u../MyProject\" -g\"config\\debug\\log.txt\"", "projects\\default"); //run 64 bit unless told not to
	else
		a = RUNCOMMANDLINE("open", "../../bin\\x86\\Rival.exe", "\"-u../MyProject\" -g\"config\\debug\\log.txt\"", "projects\\default"); //run 86 secondary unless forced
	if (int(a) < ERRORMAX)
	{
		a = RUNCOMMANDLINE("open", "../../bin\\x86\\Rival.exe", "\"-u../MyProject\" -g\"config\\debug\\log.txt\"", "projects\\default"); //try again just incase you never tried 32 bit
		if ( a  < ERRORMAX)
		{
			a = RUNCOMMANDLINE("open", "../../bin\\Rival.exe", "\"-u../MyProject\" -g\"config\\debug\\log.txt\"", "projects\\default"); //check legacy or if they dont have multipatform enabled
		}
	}
	return !(a < ERRORMAX);
}

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	
	bool cont;
	//change the message based on building 64 bit is posible or not
	LPCWSTR Errmsg = !force32 && IsWow64() ? L"Could not find Rival.exe. Make sure that\nRival.exe exist in one of the following:\n    bin\\Rival.exe\n    bin\\x86\\Rival.exe\n    bin\\x64\\Rival.exe\n" : L"Could not find Rival.exe.Make sure that\nRival.exe exist in one of the following : \n    bin\\Rival.exe\n    bin\\x86\\Rival.exe\n";
	LPCWSTR Titmsg = !force32 && IsWow64() ? L"Rival - EXE Path Error" : L"Rival - EXE Path Error - x86 only";
	do {
		cont = !trytoopen();
		
		if (cont) if (MessageBox(GetDesktopWindow(), Errmsg, Titmsg, MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDCANCEL) // show an error if we still couldnt open it
			break;
	} while (cont);
}








// Create the window
//int WINAPI Create_Window_Transparent(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//	LPSTR lpCommandLine, int nShowCmd, LPCTSTR title)
//{
//	nShowCmd = 0;
//
//
//
//	// This stuff gets some other stuff for the transparency
//	SLWA pSetLayeredWindowAttributes = NULL;
//	HINSTANCE hmodUSER32 = LoadLibrary(LPCWSTR("USER32.DLL"));
//	pSetLayeredWindowAttributes = (SLWA) GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
//	int posX, posY;
//	DEVMODE dmScreenSettings;
//
//	// Create the window's class
//	MSG msg;
//	WNDCLASS wc = { 0 };
//	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
//	wc.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
//	wc.lpszClassName = MYCLASSNAME;
//	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
//	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
//	wc.hInstance = hInstance;
//	wc.lpfnWndProc = WinProc;
//	//wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MyIcon));
//
//	// Register the windows class
//	RegisterClass(&wc);
//
//	// Create the actual window
//	HWND hWnd = CreateWindowEx(WS_EX_LAYERED, MYCLASSNAME, title, WS_POPUP, 0, 0,
//		GetSystemMetrics(SM_CXSCREEN),
//		GetSystemMetrics(SM_CYSCREEN),
//
//		NULL, NULL, hInstance, NULL);
//
//	// Make it transparent
//	pSetLayeredWindowAttributes(hWnd, 100, 100, LWA_ALPHA);
//
//	// Show the window
//	//SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
//	//ShowWindow(hWnd, nShowCmd);
//	//UpdateWindow(hWnd);
//	
//	//system("www.google.com");
//	//return -1;
//	// Simple Message loop
//	BOOL bRet = false;
//	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
//		if (bRet == -1) break;
//		else {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//	}
//
//	return -1;
//}
// Main window message loop
//LRESULT CALLBACK WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
//{
//	switch (Msg)
//	{
//		// When the window is destroyed
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//	}
//	return DefWindowProc(hWnd, Msg, wParam, lParam);
//}
// Main function, called on execution