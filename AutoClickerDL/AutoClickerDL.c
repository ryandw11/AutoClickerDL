#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdio.h>
#include <Windows.h>
#include <Commctrl.h>
#include <Shlobj.h>

#ifndef UNICODE
#define UNICODE
#endif

#include "General.h"

#define WIDTH 400
#define HEIGHT 500

// The handle to the main window.
HWND mainWindowHandle;

// The different tabs.
HWND tabControl, generalDisplayArea, settingsDisplayArea, rememberClickDisplayArea;

// The hotkey control for the start/stop of the Auto Clicker.
HWND startStopHotKey;
// The spinner for clicks per second.
HWND spinnerHWD;

// The timer used by the clicker. Not null when enabled, NULL when not enabled.
UINT_PTR autoClickerTimer = NULL;

// Process callbacks.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Main method.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	// Class name of the window.
	const wchar_t CLASS_NAME[] = L"AutoClickerDL Window Class";

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Creation of the Window.
	HWND windowHandle = CreateWindowEx(0, CLASS_NAME, L"AutoClickerDL",
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (windowHandle == NULL) {
		return 0;
	}

	mainWindowHandle = windowHandle;

	// Define the tab control (handle in WIN32 terms)
	tabControl = CreateWindow(WC_TABCONTROL, L"", WS_VISIBLE | WS_CHILD | TCS_FOCUSONBUTTONDOWN,
		0, 0, WIDTH, HEIGHT, windowHandle, NULL, hInstance, NULL);

	// Create the 3 tab sections: General, Settings, and Remember Click
	TCITEM tabItem = { 0 };
	tabItem.mask = TCIF_TEXT;
	tabItem.iImage = -1;
	tabItem.pszText = L"General";
	TabCtrl_InsertItem(tabControl, 0, &tabItem);
	tabItem.pszText = L"Settings";
	TabCtrl_InsertItem(tabControl, 1, &tabItem);
	tabItem.pszText = L"Remember Click";
	TabCtrl_InsertItem(tabControl, 2, &tabItem);

	generalDisplayArea = CreateWindow(WC_STATIC, L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, 23, WIDTH, HEIGHT-23, tabControl, NULL, hInstance, NULL);
	settingsDisplayArea = CreateTabDisplayArea(tabControl, hInstance, L"SettingsDisplayArea", 0, 23, WIDTH, HEIGHT - 23, SettingsProc);
	rememberClickDisplayArea = CreateWindow(WC_STATIC, L"", WS_CHILD | WS_BORDER,
		0, 23, WIDTH, HEIGHT - 23, tabControl, NULL, hInstance, NULL);


	/*
	===============
	General Display Area
	===============
	*/

	HWND infoLabel = CreateWindow(WC_STATIC, L"Welcome to AutoClickerDL! Head to the settings tab to \npick what button to enable the auto clicking!", WS_VISIBLE | WS_CHILD | SS_CENTER,
		0, 23, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	
	// Notify the user that they are not running the program as an administrator and that may limit some features.
	if (!IsUserAnAdmin()) {
		HWND notAdminLabel = CreateWindow(WC_STATIC, L"Note: Program is not being ran with admin perms! This\n may limit some features", WS_VISIBLE | WS_CHILD | SS_CENTER,
			0, HEIGHT - 120, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	}

	Spinner spinner = { 0 };
	spinner.x = 100;
	spinner.y = 123;
	spinner.step = 1;
	spinner.labelPtr = L"CPS";
	spinner.labelSize = 0;
	spinner.rangeMin = 1;
	spinner.rangeMax = 80;
	spinnerHWD = CreateSpinner(generalDisplayArea, hInstance, spinner);


	/*
	===============
	Settings Display Area
	===============
	*/
	HWND hotKeyLabel = CreateWindow(WC_STATIC, L"Start/Stop Auto Clicker:", WS_VISIBLE | WS_CHILD,
		10, 23, 150, 20, settingsDisplayArea, NULL, hInstance, NULL);
	startStopHotKey = CreateWindow(HOTKEY_CLASS, L"Start/StopHotKey", WS_VISIBLE | WS_CHILD,
		10, 43, 150, 20, settingsDisplayArea, NULL, hInstance, NULL);
	SendMessage(startStopHotKey, HKM_SETHOTKEY, MAKEWORD(VK_F1, 0), 0);
	RegisterHotKey(windowHandle, 0, MOD_NOREPEAT, VK_F1);


	/*
	===============
	Window Messages
	===============
	*/
	ShowWindow(windowHandle, nCmdShow);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

/*
	This is the callback for the messages of the settings display background.
*/
LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
	{
		int cmd = HIWORD(wParam);
		// Triggered by the hotkey change.
		if (cmd == EN_CHANGE) {
			// Identification of the hotkey control.
			int loword = LOWORD(wParam);
			if (loword == 0) {
				// Unregister and re-register global hot key with the change.
				UnregisterHotKey(mainWindowHandle, 0);
				int result = SendMessage(startStopHotKey, HKM_GETHOTKEY, NULL, NULL);
				RegisterHotKey(mainWindowHandle, 0, HIBYTE(result), LOBYTE(result));
			}
		}
	}
	break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
	Callback for the main window message process.
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_NOTIFY:
	{
		// If the notify event for the a tab change.
		if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
			int tabID = TabCtrl_GetCurSel(tabControl);
			switch (tabID) {
			case 0:
				ShowWindow(generalDisplayArea, TRUE);
				ShowWindow(settingsDisplayArea, FALSE);
				ShowWindow(rememberClickDisplayArea, FALSE);
				break;
			case 1: 
				ShowWindow(generalDisplayArea, FALSE);
				ShowWindow(settingsDisplayArea, TRUE);
				ShowWindow(rememberClickDisplayArea, FALSE);
				break;
			case 2:
				ShowWindow(generalDisplayArea, FALSE);
				ShowWindow(settingsDisplayArea, FALSE);
				ShowWindow(rememberClickDisplayArea, TRUE);
				break;
			default:
				break;
			}
		}
	}
	break;
	// When a global hotkey for the program is triggered.
	case WM_HOTKEY:
	{
		// If the timmer is not running, trigger it.
		if (autoClickerTimer == NULL) {
			int pos = SendMessage(spinnerHWD, UDM_GETPOS32, NULL, NULL);
			autoClickerTimer = SetTimer(hwnd, 1001, (1000 / pos), (TIMERPROC)NULL);
		}
		// Else, kill it.
		else {
			KillTimer(hwnd, autoClickerTimer);
			autoClickerTimer = NULL;
		}

		
	}
	break;
	case WM_TIMER:
	{
		// When the timmer is triggered, send a click event.
		INPUT inputs[2] = { 0 };

		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
		SendInput(2, &inputs, sizeof(INPUT));
	}
	break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}