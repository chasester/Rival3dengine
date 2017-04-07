#include <windows.h>

// set some stuff
#define WS_EX_LAYERED   0x00080000
#define LWA_ALPHA       0x00000002
#define MYCLASSNAME (LPCWSTR("__transparentwnd__"))

// set the SLWA type
typedef BOOL(WINAPI *SLWA)(HWND, COLORREF, BYTE, DWORD);

// Main window message loop
LRESULT CALLBACK WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		// When the window is destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}
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

// Create the window
int WINAPI Create_Window_Transparent(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCommandLine, int nShowCmd, LPCTSTR title)
{
	nShowCmd = 0;



	// This stuff gets some other stuff for the transparency
	SLWA pSetLayeredWindowAttributes = NULL;
	HINSTANCE hmodUSER32 = LoadLibrary(LPCWSTR("USER32.DLL"));
	pSetLayeredWindowAttributes = (SLWA) GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
	int posX, posY;
	DEVMODE dmScreenSettings;

	// Create the window's class
	MSG msg;
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
	wc.lpszClassName = MYCLASSNAME;
	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WinProc;
	//wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MyIcon));

	// Register the windows class
	RegisterClass(&wc);

	// Create the actual window
	HWND hWnd = CreateWindowEx(WS_EX_LAYERED, MYCLASSNAME, title, WS_POPUP, 0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),

		NULL, NULL, hInstance, NULL);

	// Make it transparent
	pSetLayeredWindowAttributes(hWnd, 100, 100, LWA_ALPHA);

	// Show the window
	//SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
	//ShowWindow(hWnd, nShowCmd);
	//UpdateWindow(hWnd);
	
	//system("www.google.com");
	//return -1;
	// Simple Message loop
	BOOL bRet = false;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) break;
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return -1;
}

// Main function, called on execution
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCommandLine,
	_In_ int       nShowCmd
)
{
	if(!IsWow64())
		ShellExecute(GetDesktopWindow(), L"open", L"bin\\tesseract.exe", NULL, NULL, SW_SHOWNORMAL);
	else
		ShellExecute(GetDesktopWindow(), L"open", L"bin64\\tesseract.exe", NULL, NULL, SW_SHOWNORMAL);
	// Create the window using the function above
	//Create_Window_Transparent(hInstance, hPrevInstance, lpCommandLine, nShowCmd, LPCWSTR("My Transparent Window!"));
	//Sleep(10000);
}