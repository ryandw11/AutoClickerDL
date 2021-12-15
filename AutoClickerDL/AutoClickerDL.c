#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdio.h>
#include <time.h>
#include <wchar.h>
#include <Windows.h>
#include <Commctrl.h>
#include <Shlobj.h>

#ifndef UNICODE
#define UNICODE
#endif

#include "General.h"
#include "IO.h"

#define WIDTH 400
#define HEIGHT 500

// A macro to insert code that converts an int to a wide character array.
#define itowc(_NAME) wchar_t _NAME[5] = { 0 };\
	swprintf(_NAME, 4, L"%d", loadedSettings._NAME);

#define SETTINGS_TIMED_CHECK_BOX 1
#define SETTINGS_SAVE_BUTTON 2

// The handle to the main window.
HWND mainWindowHandle;

// The different tabs.
HWND tabControl, generalDisplayArea, settingsDisplayArea, rememberClickDisplayArea;

// The hotkey control for the start/stop of the Auto Clicker.
HWND startStopHotKey;
// The spinners
HWND cpsSpinnerHWD, timedAutoSpinnerHWD, delaySpinnerHWD;
// The comboboxes
HWND mouseButtonComboBoxHWD;

// The timer used by the clicker. Not null when enabled, NULL when not enabled.
UINT_PTR autoClickerTimer = NULL;
time_t startClickerTime;

// Keeps track of the current settings when the autoclicker is turned on.
Settings currentSettings;

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


	Settings loadedSettings = { 0 };
	loadSettings("settings.acdl", &loadedSettings);

	/*
	===============
	General Display Area
	===============
	*/

	HWND infoLabel = CreateWindow(WC_STATIC, L"Welcome to AutoClickerDL! Head to the settings tab to \npick what button to enable the auto clicking!", WS_VISIBLE | WS_CHILD | SS_CENTER,
		0, 23, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);

	HWND authorLabel = CreateWindow(WC_STATIC, L"Created by: Ryandw11", WS_VISIBLE | WS_CHILD | SS_CENTER,
		0, HEIGHT - 100, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	
	// Notify the user that they are not running the program as an administrator and that may limit some features.
	if (!IsUserAnAdmin()) {
		HWND notAdminLabel = CreateWindow(WC_STATIC, L"Note: Program is not being ran with admin perms! This\n may limit some features", WS_VISIBLE | WS_CHILD | SS_CENTER,
			0, HEIGHT - 160, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	}

	Spinner cpsSpinner = { 0 };
	cpsSpinner.x = 100;
	cpsSpinner.y = 123;
	cpsSpinner.step = 1;
	cpsSpinner.labelPtr = L"CPS";
	cpsSpinner.labelSize = 0;
	cpsSpinner.rangeMin = 1;
	cpsSpinner.rangeMax = 80;
	itowc(cps)
	cpsSpinner.defaultValue = cps;
	cpsSpinnerHWD = CreateSpinner(generalDisplayArea, hInstance, cpsSpinner);


	/*
	===============
	Settings Display Area
	===============
	*/
	HWND hotKeyLabel = CreateWindow(WC_STATIC, L"Start/Stop Auto Clicker:", WS_VISIBLE | WS_CHILD,
		10, 23, 150, 20, settingsDisplayArea, NULL, hInstance, NULL);
	startStopHotKey = CreateWindow(HOTKEY_CLASS, L"Start/StopHotKey", WS_VISIBLE | WS_CHILD,
		10, 43, 150, 20, settingsDisplayArea, NULL, hInstance, NULL);
	SendMessage(startStopHotKey, HKM_SETHOTKEY, MAKEWORD(LOBYTE(loadedSettings.hotkey), HIBYTE(loadedSettings.hotkey)), 0);
	RegisterHotKey(windowHandle, 0, HIBYTE(loadedSettings.hotkey), LOBYTE(loadedSettings.hotkey));

	HWND checkbox = CreateWindow(WC_BUTTON, L"Timed Auto Click", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 70, 130, 20, settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX, hInstance, NULL);
	CheckDlgButton(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX, loadedSettings.timedAutoClick);

	Spinner timedAutoClickSpinner = { 0 };
	timedAutoClickSpinner.x = 10;
	timedAutoClickSpinner.y = 100;
	timedAutoClickSpinner.step = 1;
	timedAutoClickSpinner.labelPtr = L"Seconds";
	timedAutoClickSpinner.labelSize = 0;
	timedAutoClickSpinner.rangeMin = 1;
	timedAutoClickSpinner.rangeMax = 20;
	itowc(timedAutoClickValue);
	timedAutoClickSpinner.defaultValue = timedAutoClickValue;
	timedAutoSpinnerHWD = CreateSpinner(settingsDisplayArea, hInstance, timedAutoClickSpinner);
	EnableWindow(timedAutoSpinnerHWD, loadedSettings.timedAutoClick);
	EnableWindow((HWND)SendMessage(timedAutoSpinnerHWD, UDM_GETBUDDY, NULL, NULL), loadedSettings.timedAutoClick);

	HWND delayLabel = CreateWindow(WC_STATIC, L"Delay between mouse down and up:", WS_VISIBLE | WS_CHILD,
		10, 130, 200, 40, settingsDisplayArea, NULL, hInstance, NULL);
	Spinner delayClickSpinner = { 0 };
	delayClickSpinner.x = 10;
	delayClickSpinner.y = 170;
	delayClickSpinner.step = 10;
	delayClickSpinner.labelPtr = L"ms";
	delayClickSpinner.labelSize = 0;
	delayClickSpinner.rangeMin = 0;
	delayClickSpinner.rangeMax = 100;
	itowc(delayTime);
	delayClickSpinner.defaultValue = delayTime;
	delaySpinnerHWD = CreateSpinner(settingsDisplayArea, hInstance, delayClickSpinner);

	HWND mouseButtonLabel = CreateWindow(WC_STATIC, L"Mouse button to click:", WS_VISIBLE | WS_CHILD,
		10, 200, 200, 40, settingsDisplayArea, NULL, hInstance, NULL);

	mouseButtonComboBoxHWD = CreateWindow(WC_COMBOBOX, L"", CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		10, 220, 120, 20, settingsDisplayArea, NULL, hInstance, NULL);
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Left Click");
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Right Click");
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Middle Click");
	// Set the current selection for the combo box.
	SendMessage(mouseButtonComboBoxHWD, CB_SETCURSEL, loadedSettings.mouseClickType, NULL);

	HWND saveButton = CreateWindow(WC_BUTTON, L"Save Settings", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		WIDTH / 2 - (80), HEIGHT - 120, 130, 35, settingsDisplayArea, SETTINGS_SAVE_BUTTON, hInstance, NULL);

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
	Updates a settings struct with the current settings.

	@param settings The pointer to the Settings struct to update.
*/
void updateCurrentSettings(Settings* settings) {
	settings->cps = SendMessage(cpsSpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->timedAutoClick = IsDlgButtonChecked(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX);
	settings->timedAutoClickValue = SendMessage(timedAutoSpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->delayTime = SendMessage(delaySpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->mouseClickType = SendMessage(mouseButtonComboBoxHWD, CB_GETCURSEL, NULL, NULL);
	settings->hotkey = SendMessage(startStopHotKey, HKM_GETHOTKEY, NULL, NULL);
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
		else if (cmd == BN_CLICKED) {
			int id = LOWORD(wParam);
			// Handle the check click for the timed checkbox.
			if (id == SETTINGS_TIMED_CHECK_BOX) {
				if (IsDlgButtonChecked(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX)) {
					CheckDlgButton(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX, BST_UNCHECKED);
					EnableWindow(timedAutoSpinnerHWD, FALSE);
					EnableWindow((HWND)SendMessage(timedAutoSpinnerHWD, UDM_GETBUDDY, NULL, NULL), FALSE);
				}
				else {
					CheckDlgButton(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX, BST_CHECKED);
					EnableWindow(timedAutoSpinnerHWD, TRUE);
					EnableWindow((HWND)SendMessage(timedAutoSpinnerHWD, UDM_GETBUDDY, NULL, NULL), TRUE);
				}
			}
			// Handle the clicking of the save button.
			else if (id == SETTINGS_SAVE_BUTTON) {
				Settings settings = { 0 };
				updateCurrentSettings(&settings);
				if(saveSettings("settings.acdl", &settings) == 0)
					MessageBox(NULL, L"Your current settings have been successfuly saved!", L"Save Settings", MB_OK | MB_ICONINFORMATION);
				else
					MessageBox(NULL, L"Your current settings could not be saved! Does this program have the correct permissions?", L"Save Settings Error", MB_OK | MB_ICONEXCLAMATION);
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
	Get the mouse event type depending on the current settings.

	@param up The pointer to the up variable that will be set with LEFT, RIGHT, or MIDDLEDOWN.
	@param down The pointer to the down variable that will be set with LEFT, RIGHT, or MIDDLEUP.
	@param settings The pointer of the settings struct to pull the mouse button setting from.
*/
void getMouseFromSettings(int* up, int* down, Settings* settings) {
	switch (settings->mouseClickType) {
	case 0:
		(*down) = MOUSEEVENTF_LEFTDOWN;
		(*up) = MOUSEEVENTF_LEFTUP;
		return;
	case 1:
		(*down) = MOUSEEVENTF_RIGHTDOWN;
		(*up) = MOUSEEVENTF_RIGHTUP;
		return;
	case 2:
		(*down) = MOUSEEVENTF_MIDDLEDOWN;
		(*up) = MOUSEEVENTF_MIDDLEUP;
		return;
	default:
		(*down) = MOUSEEVENTF_LEFTDOWN;
		(*up) = MOUSEEVENTF_LEFTUP;
		return;
	}
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
			updateCurrentSettings(&currentSettings);
			startClickerTime = time(NULL);
			autoClickerTimer = SetTimer(hwnd, 1001, (1000 / currentSettings.cps), (TIMERPROC)NULL);
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
		int up = 0;
		int down = 0;
		getMouseFromSettings(&up, &down, &currentSettings);
		if (currentSettings.delayTime == 0) {
			// When the timmer is triggered, send a click event.
			INPUT inputs[2] = { 0 };

			inputs[0].type = INPUT_MOUSE;
			inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | down;
			inputs[1].type = INPUT_MOUSE;
			inputs[1].mi.dwFlags = MOUSEEVENTF_MOVE | up;
			SendInput(2, &inputs, sizeof(INPUT));
		}
		else {
			INPUT inputOne = { 0 };
			INPUT inputTwo = { 0 };

			inputOne.type = INPUT_MOUSE;
			inputOne.mi.dwFlags = MOUSEEVENTF_MOVE | down;
			inputTwo.type = INPUT_MOUSE;
			inputTwo.mi.dwFlags = MOUSEEVENTF_MOVE | up;
			SendInput(1, &inputOne, sizeof(INPUT));
			Sleep(currentSettings.delayTime);
			SendInput(1, &inputTwo, sizeof(INPUT));
		}

		// If timer mode is enabled, shut off the timer if over time.
		if (IsDlgButtonChecked(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX) && time(NULL) - startClickerTime  >= SendMessage(timedAutoSpinnerHWD, UDM_GETPOS32, NULL, NULL)) {
			KillTimer(hwnd, autoClickerTimer);
			autoClickerTimer = NULL;
		}
	}
	break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}