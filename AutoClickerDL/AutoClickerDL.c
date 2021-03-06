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
#include "resource.h"

#define WIDTH 400
#define HEIGHT 500

// A macro to insert code that converts an int to a wide character array.
#define itowc(_NAME) wchar_t _NAME[5] = { 0 };\
	swprintf(_NAME, 4, L"%d", loadedSettings._NAME);

#define SETTINGS_TIMED_CHECK_BOX 1
#define SETTINGS_SAVE_BUTTON 2
#define REMEMBER_SAVE_RECORDING 3
#define REMEMBER_LOAD_RECORDING 4
#define AUTO_CLICK_HOTKEY 5
#define REMEMBER_HOTKEY 6
#define REMEMBER_PLAY_HOTKEY 7
#define PRESS_KEY_HOTKEY 8
#define MODE_BUTTON_GROUP 9
#define RADIO_BUTTON_AUTO_CLICK 10
#define RADIO_BUTTON_AUTO_PRESS 11
#define SETTINGS_PRESS_UP_CHECK_BOX 12

// The handle to the main window.
HWND mainWindowHandle;

// The different tabs.
HWND tabControl, generalDisplayArea, settingsDisplayArea, rememberClickDisplayArea;

// The hotkey control for the start/stop of the Auto Clicker.
HWND startStopHotKey, rmbClkRecordHK, rmbClkRecordPlayHK, pressHotKey;
// The spinners
HWND cpsSpinnerHWD, ppsSpinnerHWD, timedAutoSpinnerHWD, delaySpinnerHWD;
// The comboboxes
HWND mouseButtonComboBoxHWD;
// The labels
HWND rememberClickStatus;
// Buttons
HWND saveRecordingButton;
// Radio Buttons
HWND autoClickRadioButton, autoPressRadioButton;

// The timer used by the clicker. Not null when enabled, NULL when not enabled.
UINT_PTR autoClickerTimer = NULL;
time_t startClickerTime;

// The timer used for the remember click play timer.
UINT_PTR rememberClickTimer = NULL;

// Keep track of the state of a recording.
RecordingState recordingState;

// Keeps track of the current settings when the autoclicker is turned on.
Settings currentSettings;

// The hook handle for the mouse hook.
HHOOK mouseHook, keyHook;

// The normal and activated icons.
HICON normalIcon, clickActivatedIcon;

// Process callbacks.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GeneralProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RememberProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Hook Callback
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// Helper Functions
int GetAutoMode();
void UpdateAutoModeWindows(int);


/*
	This is the main method for the program.
*/
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	// Prevent more than 1 instance of the program from running.
	HANDLE mutexHandle = CreateMutex(NULL, TRUE, L"com_ryandw11_autoclickerdl");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"AutoClickerDL is already running! Please close the exisiting instance of the program before opening it again.", L"AutoClickerDL Already Open", MB_OK | MB_ICONERROR);
		return 0;
	}

	// Class name of the window.
	const wchar_t CLASS_NAME[] = L"AutoClickerDL Window Class";

	// Load the icons
	normalIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	clickActivatedIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));

	// Create and register the primary window class.
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = normalIcon;

	RegisterClass(&wc);

	// Creation of the Window.
	HWND windowHandle = CreateWindowEx(0, CLASS_NAME, L"AutoClickerDL",
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);

	// If failed, return.
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

	generalDisplayArea = CreateTabDisplayArea(tabControl, hInstance, L"GeneralDisplayArea", 0, 23, WIDTH, HEIGHT - 23, GeneralProc);
	settingsDisplayArea = CreateTabDisplayArea(tabControl, hInstance, L"SettingsDisplayArea", 0, 23, WIDTH, HEIGHT - 23, SettingsProc);
	rememberClickDisplayArea = CreateTabDisplayArea(tabControl, hInstance, L"RememberDisplayArea", 0, 23, WIDTH, HEIGHT - 23, RememberProc);

	ShowWindow(generalDisplayArea, TRUE);

	Settings loadedSettings = { 0 };
	loadSettings("settings.acdl", &loadedSettings);

	/*
	===============
	General Display Area
	===============
	*/

	HWND infoLabel = CreateWindow(WC_STATIC, L"Welcome to AutoClickerDL! This application allows you\nto quickly and automatically trigger mouse clicks \nand keyboard presses. Head to the settings tab to pick \nwhat button to enable the auto clicking and pressing!", WS_VISIBLE | WS_CHILD | SS_CENTER,
		0, 23, WIDTH - 10, 80, generalDisplayArea, NULL, hInstance, NULL);

	HWND authorLabel = CreateWindow(WC_STATIC, L"Created by: Ryandw11", WS_VISIBLE | WS_CHILD | SS_CENTER,
		0, HEIGHT - 100, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	
	// Notify the user that they are not running the program as an administrator and that may limit some features.
	/*if (!IsUserAnAdmin()) {
		HWND notAdminLabel = CreateWindow(WC_STATIC, L"Note: Program is not being ran with admin perms! This\n may limit some features", WS_VISIBLE | WS_CHILD | SS_CENTER,
			0, HEIGHT - 160, WIDTH, 30, generalDisplayArea, NULL, hInstance, NULL);
	}*/

	HWND mouseAutoCLickLable = CreateWindow(WC_STATIC, L"Mouse Clicker:", WS_VISIBLE | WS_CHILD | SS_CENTER | BOLD_FONTTYPE,
		0, 120, WIDTH - 20, 20, generalDisplayArea, NULL, hInstance, NULL);

	Spinner cpsSpinner = { 0 };
	cpsSpinner.x = WIDTH/2 - 60;
	cpsSpinner.y = 143;
	cpsSpinner.step = 1;
	cpsSpinner.labelPtr = L"CPS";
	cpsSpinner.labelSize = 0;
	cpsSpinner.rangeMin = 1;
	cpsSpinner.rangeMax = 80;
	itowc(cps)
	cpsSpinner.defaultValue = cps;
	cpsSpinnerHWD = CreateSpinner(generalDisplayArea, hInstance, cpsSpinner);


	// Mouse Button Selection
	HWND mouseButtonLabel = CreateWindow(WC_STATIC, L"Mouse Button:", WS_VISIBLE | WS_CHILD,
		100, 175, 200, 40, generalDisplayArea, NULL, hInstance, NULL);

	mouseButtonComboBoxHWD = CreateWindow(WC_COMBOBOX, L"", CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		200, 173, 120, 20, generalDisplayArea, NULL, hInstance, NULL);
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Left Click");
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Right Click");
	SendMessage(mouseButtonComboBoxHWD, CB_ADDSTRING, NULL, L"Middle Click");
	// Set the current selection for the combo box.
	SendMessage(mouseButtonComboBoxHWD, CB_SETCURSEL, loadedSettings.mouseClickType, NULL);

	HWND keyAutoPressLabel = CreateWindow(WC_STATIC, L"Key Presser:", WS_VISIBLE | WS_CHILD | SS_CENTER | BOLD_FONTTYPE,
		0, 220, WIDTH - 20, 20, generalDisplayArea, NULL, hInstance, NULL);

	Spinner pressPerSecondSpinner = { 0 };
	pressPerSecondSpinner.x = WIDTH / 2 - 60;
	pressPerSecondSpinner.y = 240;
	pressPerSecondSpinner.step = 1;
	pressPerSecondSpinner.labelPtr = L"CPS";
	pressPerSecondSpinner.labelSize = 0;
	pressPerSecondSpinner.rangeMin = 1;
	pressPerSecondSpinner.rangeMax = 80;
	itowc(pps)
	pressPerSecondSpinner.defaultValue = pps;
	ppsSpinnerHWD = CreateSpinner(generalDisplayArea, hInstance, pressPerSecondSpinner);

	// Press Hot Key
	HWND pressKeyLabel = CreateWindow(WC_STATIC, L"Key:", WS_VISIBLE | WS_CHILD | SS_CENTER,
		100, 270, 30, 20, generalDisplayArea, NULL, hInstance, NULL);
	pressHotKey = CreateWindow(HOTKEY_CLASS, L"KeyToPress", WS_VISIBLE | WS_CHILD,
		140, 270, 100, 20, generalDisplayArea, PRESS_KEY_HOTKEY, hInstance, NULL);
	SendMessage(pressHotKey, HKM_SETHOTKEY, MAKEWORD(LOBYTE(loadedSettings.autoPressKey), HIBYTE(loadedSettings.autoPressKey)), 0);

	// Mode Radio Buttons
	autoClickRadioButton = CreateWindow(L"BUTTON", L"Mouse Clicker", WS_VISIBLE | WS_CHILD | WS_GROUP | WS_TABSTOP | BS_AUTORADIOBUTTON,
		80, 350, 110, 20, generalDisplayArea, RADIO_BUTTON_AUTO_CLICK, hInstance, NULL);
	autoPressRadioButton = CreateWindow(L"BUTTON", L"Key Presser", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		200, 350, 100, 20, generalDisplayArea, RADIO_BUTTON_AUTO_PRESS, hInstance, NULL);

	CheckDlgButton(generalDisplayArea, RADIO_BUTTON_AUTO_CLICK, loadedSettings.mode == MODE_AUTO_CLICK ? 1 : 0);
	CheckDlgButton(generalDisplayArea, RADIO_BUTTON_AUTO_PRESS, loadedSettings.mode == MODE_AUTO_PRESS ? 1 : 0);
	UpdateAutoModeWindows(loadedSettings.mode);


	/*
	===============
	Settings Display Area
	===============
	*/
	HWND hotKeyLabel = CreateWindow(WC_STATIC, L"Start/Stop Auto Clicker or Presser:", WS_VISIBLE | WS_CHILD,
		10, 23, WIDTH, 20, settingsDisplayArea, NULL, hInstance, NULL);
	startStopHotKey = CreateWindow(HOTKEY_CLASS, L"Start/StopHotKey", WS_VISIBLE | WS_CHILD,
		10, 43, 150, 20, settingsDisplayArea, AUTO_CLICK_HOTKEY, hInstance, NULL);
	SendMessage(startStopHotKey, HKM_SETHOTKEY, MAKEWORD(LOBYTE(loadedSettings.hotkey), HIBYTE(loadedSettings.hotkey)), 0);
	RegisterHotKey(windowHandle, AUTO_CLICK_HOTKEY, HIBYTE(loadedSettings.hotkey), LOBYTE(loadedSettings.hotkey));

	HWND checkbox = CreateWindow(WC_BUTTON, L"Timed Auto Click or Press", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 70, 200, 20, settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX, hInstance, NULL);
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

	HWND delayLabel = CreateWindow(WC_STATIC, L"Delay between mouse/key down and up:", WS_VISIBLE | WS_CHILD,
		10, 130, WIDTH, 40, settingsDisplayArea, NULL, hInstance, NULL);
	Spinner delayClickSpinner = { 0 };
	delayClickSpinner.x = 10;
	delayClickSpinner.y = 150;
	delayClickSpinner.step = 10;
	delayClickSpinner.labelPtr = L"ms";
	delayClickSpinner.labelSize = 0;
	delayClickSpinner.rangeMin = 0;
	delayClickSpinner.rangeMax = 100;
	itowc(delayTime);
	delayClickSpinner.defaultValue = delayTime;
	delaySpinnerHWD = CreateSpinner(settingsDisplayArea, hInstance, delayClickSpinner);

	HWND keyUpCheckbox = CreateWindow(WC_BUTTON, L"Trigger Key Up", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		10, 180, 200, 20, settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX, hInstance, NULL);
	CheckDlgButton(settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX, loadedSettings.pressKeyUp);

	HWND saveButton = CreateWindow(WC_BUTTON, L"Save Settings", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		WIDTH / 2 - (80), HEIGHT - 120, 130, 35, settingsDisplayArea, SETTINGS_SAVE_BUTTON, hInstance, NULL);

	/*
	===============
	Remember Click Display Area
	===============
	*/

	HWND rmbClkRecordHKLabel = CreateWindow(WC_STATIC, L"Start/Stop Recording Hot Key:", WS_VISIBLE | WS_CHILD,
			10, 10, 200, 20, rememberClickDisplayArea, NULL, hInstance, NULL);
	rmbClkRecordHK = CreateWindow(HOTKEY_CLASS, L"RmbClkRecordStartHotKey", WS_VISIBLE | WS_CHILD,
		10, 33, 150, 20, rememberClickDisplayArea, REMEMBER_HOTKEY, hInstance, NULL);
	SendMessage(rmbClkRecordHK, HKM_SETHOTKEY, MAKEWORD(LOBYTE(loadedSettings.rmbStartHotkey), HIBYTE(loadedSettings.rmbStartHotkey)), 0);
	RegisterHotKey(windowHandle, REMEMBER_HOTKEY, HIBYTE(loadedSettings.rmbStartHotkey), LOBYTE(loadedSettings.rmbStartHotkey));

	HWND rmbClkRecordPlayHKLabel = CreateWindow(WC_STATIC, L"Play Recording Hot Key:", WS_VISIBLE | WS_CHILD,
		10, 58, 170, 20, rememberClickDisplayArea, NULL, hInstance, NULL);
	rmbClkRecordPlayHK = CreateWindow(HOTKEY_CLASS, L"RmbClkRecordStopHotKey", WS_VISIBLE | WS_CHILD,
		10, 78, 150, 20, rememberClickDisplayArea, REMEMBER_PLAY_HOTKEY, hInstance, NULL);
	SendMessage(rmbClkRecordPlayHK, HKM_SETHOTKEY, MAKEWORD(LOBYTE(loadedSettings.rmbPlayHotKey), HIBYTE(loadedSettings.rmbPlayHotKey)), 0);
	RegisterHotKey(windowHandle, REMEMBER_PLAY_HOTKEY, HIBYTE(loadedSettings.rmbPlayHotKey), LOBYTE(loadedSettings.rmbPlayHotKey));

	rememberClickStatus = CreateWindow(WC_STATIC, L"Load or create a Remember Click recording.", WS_VISIBLE | WS_CHILD | SS_CENTER,
		10, HEIGHT/2 - 60, WIDTH - 30, 40, rememberClickDisplayArea, NULL, hInstance, NULL);

	saveRecordingButton = CreateWindow(WC_BUTTON, L"Save Recording", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		WIDTH / 2 - (80), HEIGHT - 160, 130, 35, rememberClickDisplayArea, REMEMBER_SAVE_RECORDING, hInstance, NULL);
	EnableWindow(saveRecordingButton, FALSE);
	HWND loadRecordingButton = CreateWindow(WC_BUTTON, L"Load Recording", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		WIDTH / 2 - (80), HEIGHT - 120, 130, 35, rememberClickDisplayArea, REMEMBER_LOAD_RECORDING, hInstance, NULL);

	// Initalize the state of the recording.
	InitRecordingState(&recordingState);

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

	ReleaseMutex(mutexHandle);
	CloseHandle(mutexHandle);

	return 0;
}

/*
	Updates a settings struct with the current settings.

	@param settings The pointer to the Settings struct to update.
*/
void updateCurrentSettings(Settings* settings) {
	settings->cps = SendMessage(cpsSpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->mouseClickType = SendMessage(mouseButtonComboBoxHWD, CB_GETCURSEL, NULL, NULL);
	settings->pps = SendMessage(ppsSpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->autoPressKey = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
	settings->mode = GetAutoMode();
	settings->timedAutoClick = IsDlgButtonChecked(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX);
	settings->timedAutoClickValue = SendMessage(timedAutoSpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->delayTime = SendMessage(delaySpinnerHWD, UDM_GETPOS32, NULL, NULL);
	settings->pressKeyUp = IsDlgButtonChecked(settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX);
	settings->hotkey = SendMessage(startStopHotKey, HKM_GETHOTKEY, NULL, NULL);
	settings->rmbStartHotkey = SendMessage(rmbClkRecordHK, HKM_GETHOTKEY, NULL, NULL);
	settings->rmbPlayHotKey = SendMessage(rmbClkRecordPlayHK, HKM_GETHOTKEY, NULL, NULL);
}

/**
	Checks if one of the hot key controls are in focus.

	@returns If one of the hotkey controls are in focus.
*/
BOOL isHotkeyControlInFocus() {
	HWND handle = GetFocus();
	if (handle == startStopHotKey || handle == rmbClkRecordHK || handle == rmbClkRecordPlayHK || handle == pressHotKey) {
		return TRUE;
	}
	return FALSE;
}

/*
	This is the callback for the messages of the general display background.
*/
LRESULT CALLBACK GeneralProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
	{
		int cmd = HIWORD(wParam);
		// Hot key selector change.
		if (cmd == EN_CHANGE) {
			int loword = LOWORD(wParam);

			if (loword == PRESS_KEY_HOTKEY) {
				// Unregister and re-register global hot key with the change.
				int result = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				if (LOBYTE(result) != 0)
					SetFocus(mainWindowHandle);
			}
		}
		else if (cmd == BN_CLICKED) {
			int button = LOWORD(wParam);

			if (button == RADIO_BUTTON_AUTO_CLICK) {
				UpdateAutoModeWindows(MODE_AUTO_CLICK);
			}
			else if (button == RADIO_BUTTON_AUTO_PRESS) {
				UpdateAutoModeWindows(MODE_AUTO_PRESS);
			}
		}
	}
	break;
	break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
			if (loword == AUTO_CLICK_HOTKEY) {
				// Unregister and re-register global hot key with the change.
				UnregisterHotKey(mainWindowHandle, AUTO_CLICK_HOTKEY);
				int result = SendMessage(startStopHotKey, HKM_GETHOTKEY, NULL, NULL);
				RegisterHotKey(mainWindowHandle, AUTO_CLICK_HOTKEY, HIBYTE(result), LOBYTE(result));
				if(LOBYTE(result) != 0)
					SetFocus(mainWindowHandle);
			}
		}
		// Handle a button being clicked.
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
	The process callback for the remember display area.
*/
LRESULT CALLBACK RememberProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
	{
		int cmd = HIWORD(wParam);
		// Triggered by the hotkey change.
		if (cmd == EN_CHANGE) {
			// Identification of the hotkey control.
			int loword = LOWORD(wParam);
			if (loword == REMEMBER_HOTKEY) {
				// Unregister and re-register global hot key with the change.
				UnregisterHotKey(mainWindowHandle, REMEMBER_HOTKEY);
				int result = SendMessage(rmbClkRecordHK, HKM_GETHOTKEY, NULL, NULL);
				RegisterHotKey(mainWindowHandle, REMEMBER_HOTKEY, HIBYTE(result), LOBYTE(result));
				if (LOBYTE(result) != 0)
					SetFocus(mainWindowHandle);
			}
			else if (loword == REMEMBER_PLAY_HOTKEY) {
				// Unregister and re-register global hot key with the change.
				UnregisterHotKey(mainWindowHandle, REMEMBER_PLAY_HOTKEY);
				int result = SendMessage(rmbClkRecordPlayHK, HKM_GETHOTKEY, NULL, NULL);
				RegisterHotKey(mainWindowHandle, REMEMBER_PLAY_HOTKEY, HIBYTE(result), LOBYTE(result));
				if (LOBYTE(result) != 0)
					SetFocus(mainWindowHandle);
			}
		}
		// Triggered by a button press.
		else if (cmd == BN_CLICKED) {
			int id = LOWORD(wParam);
			// If the load recording button was pressed.
			if (id == REMEMBER_LOAD_RECORDING) {
				// Only trigger if the recording state is loaded or none.
				if (!(recordingState.state == REC_STATE_LOADED || recordingState.state == REC_STATE_NONE)) {
					break;
				}
				OPENFILENAME ofn;
				TCHAR szFile[260] = { 0 };
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = mainWindowHandle;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = L"AutoClickerDL File (.acdl)\0*.acdl\0";
				ofn.nFilterIndex = 0;
				ofn.lpstrDefExt = L"acdl";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					if (LoadRecordingState(&recordingState, ofn.lpstrFile)) {
						MessageBox(mainWindowHandle, L"Successfully loaded the remember click file!", L"Successful Load", MB_ICONINFORMATION);
						wchar_t str[200] = { 0 };
						swprintf(str, 200, L"Recording loaded with %d inputs!\0", recordingState.numberOfClicks);
						SetWindowText(rememberClickStatus, str);
					}
					else {
						MessageBox(mainWindowHandle, L"Could not load remember click file, the file might be corrupted or in the incorrect format.", L"Error While Loading", MB_ICONERROR);
						SetWindowText(rememberClickStatus, L"Load or create a Remember Click recording.");
						EnableWindow(saveRecordingButton, FALSE);
						InitRecordingState(&recordingState);
					}
				}
			}
			// If the save recording button was pressed.
			else if (id == REMEMBER_SAVE_RECORDING) {
				// Only trigger if the recording state is loaded.
				if (recordingState.state != REC_STATE_LOADED) {
					break;
				}
				OPENFILENAME ofn;
				TCHAR szFile[260] = { 0 };
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = mainWindowHandle;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = L"AutoClickerDL File (.acdl)\0*.acdl\0";
				ofn.nFilterIndex = 0;
				ofn.lpstrDefExt = L"acdl";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (GetSaveFileName(&ofn)) {
					if (SaveRecordingState(&recordingState, ofn.lpstrFile)) {
						MessageBox(mainWindowHandle, L"Successfully saved the remember click file!", L"Successful Save", MB_ICONINFORMATION);
					}
					else {
						MessageBox(mainWindowHandle, L"Could not save remember click file. Does this program have permission to the location where you are trying to save?", L"Error While Saving", MB_ICONERROR);
					}
				}
			}
		}
	}
	break;
	case WM_TIMER:
	{
		// This timer is triggered by the remember click play hotkey.

		RememberClick* currentClick = recordingState.previousClick;
		if (GetTickCount() - recordingState.prevoiusSystemTime < currentClick->delay) {
			break;
		}
		if (currentClick->type == RC_MOUSE_CLICK) {
			int mouseClickType = MCToEventMouse(currentClick->mi.type);
			if (mouseClickType != MC_TYPE_ERROR) {
				INPUT input = { 0 };
				// Move the cursor first.
				SetCursorPos(currentClick->mi.x, currentClick->mi.y);
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_MOVE | mouseClickType;
				SendInput(1, &input, sizeof(INPUT));
			}
		}
		else if (currentClick->type == RC_KEY_PRESS) {
			int keyClickType = KBToEventKey(currentClick->ki.type);
			if (keyClickType != 0) {
				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.dwFlags = keyClickType;
				input.ki.wVk = currentClick->ki.key;
				SendInput(1, &input, sizeof(INPUT));
			}
			else {
				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = currentClick->ki.key;
				SendInput(1, &input, sizeof(INPUT));
			}
		}
		recordingState.prevoiusSystemTime = GetTickCount();
		NextMouseClick(&recordingState);
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
		switch (wParam) {
		// If the hotkey for the autoclicker was pressed.
		case AUTO_CLICK_HOTKEY:
		{
			// If a hotkey contorl is in focus, don't trigger the auto clicker.
			if (isHotkeyControlInFocus() || recordingState.state == REC_STATE_PLAYING || recordingState.state == REC_STATE_RECORDING) {
				return;
			}
			// If the timmer is not running, trigger it.
			if (autoClickerTimer == NULL) {
				updateCurrentSettings(&currentSettings);
				startClickerTime = time(NULL);
				autoClickerTimer = SetTimer(hwnd, 1001, (1000 / currentSettings.cps), (TIMERPROC)NULL);
				SetClassLong(hwnd, GCL_HICON, clickActivatedIcon);
			}
			// Else, kill it.
			else {
				KillTimer(hwnd, autoClickerTimer);
				autoClickerTimer = NULL;
				SetClassLong(hwnd, GCL_HICON, normalIcon);

				// Send a key up event if needed.
				if (GetAutoMode() == MODE_AUTO_PRESS && !IsDlgButtonChecked(settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX)) {
					INPUT input = { 0 };
					input.type = INPUT_KEYBOARD;
					input.ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
					input.ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput(1, &input, sizeof(INPUT));
				}
			}
		}
		break;
		// If the hotkey to remember the clicks was pressed.
		case REMEMBER_HOTKEY:
		{
			// If a hotkey contorl is in focus, don't trigger the recording.
			if (isHotkeyControlInFocus()) {
				return;
			}
			if (autoClickerTimer != NULL) {
				return;
			}
			// If the recording state is none or loaded, then start recording.
			if (recordingState.state == REC_STATE_NONE || recordingState.state == REC_STATE_LOADED) {
				// Delete the existing data if the recording is loaded.
				if (recordingState.state == REC_STATE_LOADED) {
					DeleteRecordingState(&recordingState);
					InitRecordingState(&recordingState);
				}
				// Set the program to record.
				recordingState.state = REC_STATE_RECORDING;
				// Set the previous system time to the current number of milliseconds since the system started.
				recordingState.prevoiusSystemTime = GetTickCount();
				mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
				keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
				SetWindowText(rememberClickStatus, L"Recording inputs...");
				// Change the icon to gray.
				SetClassLong(hwnd, GCL_HICON, clickActivatedIcon);
			}
			// If the program is currently recording mouse clicks, stop the recording.
			else if (recordingState.state == REC_STATE_RECORDING) {
				recordingState.state = REC_STATE_LOADED;
				recordingState.prevoiusSystemTime = 0;
				recordingState.previousClick = recordingState.startOfRecording;
				// Unhook the mouse hook to prevent lag if an error were to occur.
				UnhookWindowsHookEx(mouseHook);
				UnhookWindowsHookEx(keyHook);
				mouseHook = NULL;
				keyHook = NULL;
				EnableWindow(saveRecordingButton, TRUE);
				wchar_t str[200] = {0};
				swprintf(str, 200, L"Recording loaded with %d inputs!\0", recordingState.numberOfClicks);
				SetWindowText(rememberClickStatus, str);
				// Change the icon back to normal.
				SetClassLong(hwnd, GCL_HICON, normalIcon);
			}
		}
		break;
		// If the play recording hotkey was pressed.
		case REMEMBER_PLAY_HOTKEY:
		{
			if (isHotkeyControlInFocus()) {
				return;
			}
			if (autoClickerTimer != NULL) {
				return;
			}
			// If the state is loaded, then play the recording.
			if (recordingState.state == REC_STATE_LOADED) {
				recordingState.state = REC_STATE_PLAYING;
				recordingState.prevoiusSystemTime = GetTickCount();
				wchar_t str[200] = { 0 };
				swprintf(str, 200, L"Playing recording with %d inputs!\0", recordingState.numberOfClicks);
				SetWindowText(rememberClickStatus, str);

				recordingState.previousClick = recordingState.startOfRecording;
				rememberClickTimer = SetTimer(rememberClickDisplayArea, 1002, USER_TIMER_MINIMUM, (TIMERPROC)NULL);
				SetClassLong(hwnd, GCL_HICON, clickActivatedIcon);
			}
			// If a recording is being played, stop the recording.
			else if (recordingState.state == REC_STATE_PLAYING) {
				KillTimer(rememberClickDisplayArea, rememberClickTimer);
				recordingState.state = REC_STATE_LOADED;
				recordingState.prevoiusSystemTime = 0;
				recordingState.previousClick = recordingState.startOfRecording;
				wchar_t str[200] = { 0 };
				swprintf(str, 200, L"Recording loaded with %d inputs!\0", recordingState.numberOfClicks);
				SetWindowText(rememberClickStatus, str);
				SetClassLong(hwnd, GCL_HICON, normalIcon);
			}
		}
		break;
		}

		
	}
	break;
	// This timer is for the normal auto clicker.
	case WM_TIMER:
	{
		if (GetAutoMode() == MODE_AUTO_CLICK) {
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
		}
		else if (GetAutoMode() == MODE_AUTO_PRESS) {
			BOOL triggerKeyUp = IsDlgButtonChecked(settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX);
			if (currentSettings.delayTime == 0) {
				// When the timmer is triggered, send a press event.
				INPUT inputs[2] = { 0 };

				inputs[0].type = INPUT_KEYBOARD;
				inputs[0].ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				inputs[1].type = INPUT_KEYBOARD;
				inputs[1].ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(triggerKeyUp ? 2 : 1, &inputs, sizeof(INPUT));
			}
			else {
				INPUT inputOne = { 0 };
				INPUT inputTwo = { 0 };

				inputOne.type = INPUT_KEYBOARD;
				inputOne.ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				inputTwo.type = INPUT_KEYBOARD;
				inputTwo.ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				inputTwo.ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(1, &inputOne, sizeof(INPUT));
				Sleep(currentSettings.delayTime);
				if(triggerKeyUp)
					SendInput(1, &inputTwo, sizeof(INPUT));
			}
		}

		// If timer mode is enabled, shut off the timer if over time.
		if (IsDlgButtonChecked(settingsDisplayArea, SETTINGS_TIMED_CHECK_BOX) && time(NULL) - startClickerTime  >= SendMessage(timedAutoSpinnerHWD, UDM_GETPOS32, NULL, NULL)) {
			KillTimer(hwnd, autoClickerTimer);
			autoClickerTimer = NULL;

			if (GetAutoMode() == MODE_AUTO_PRESS && !IsDlgButtonChecked(settingsDisplayArea, SETTINGS_PRESS_UP_CHECK_BOX)) {
				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = SendMessage(pressHotKey, HKM_GETHOTKEY, NULL, NULL);
				input.ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(1, &input, sizeof(INPUT));
			}
		}
	}
	break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
	This is the callback for the mouse hook.

	The mouse hook is set when the remember click record hotkey is pressed.
*/
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if(nCode < 0 || recordingState.state != REC_STATE_RECORDING)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	int mcType = WMToMC(wParam);
	// Unknown / unwanted action, skip.
	if(mcType == 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	RememberClick rc = { 0 };
	rc.next = NULL;
	rc.type = RC_MOUSE_CLICK;
	rc.delay = GetTickCount() - recordingState.prevoiusSystemTime;
	// Set the previous system time to the current number of milliseconds since the system started.
	recordingState.prevoiusSystemTime = GetTickCount();
	rc.mi.type = mcType;
	POINT p = { 0 };
	GetCursorPos(&p);
	rc.mi.x = p.x;
	rc.mi.y = p.y;
	AddRememberClickToState(&recordingState, rc);

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0 || recordingState.state != REC_STATE_RECORDING)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	int kbType = WMToKB(wParam);
	if(kbType == 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	RememberClick rc = { 0 };
	rc.next = NULL;
	rc.type = RC_KEY_PRESS;
	rc.delay = GetTickCount() - recordingState.prevoiusSystemTime;
	recordingState.prevoiusSystemTime = GetTickCount();
	rc.ki.type = kbType;
	LPKBDLLHOOKSTRUCT kb = (LPKBDLLHOOKSTRUCT) lParam;
	rc.ki.key = kb->vkCode;

	AddRememberClickToState(&recordingState, rc);

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

/**
* Get the current auto mode.
* 
* @return MODE_AUTO_CLICK, MODE_AUTO_PRESS, or MODE_ERROR
*/
int GetAutoMode() {
	if (IsDlgButtonChecked(generalDisplayArea, RADIO_BUTTON_AUTO_CLICK)) return MODE_AUTO_CLICK;
	else if (IsDlgButtonChecked(generalDisplayArea, RADIO_BUTTON_AUTO_PRESS)) return MODE_AUTO_PRESS;
	else return MODE_ERROR;
}

void UpdateAutoModeWindows( int mode ) {
	if (mode == MODE_AUTO_CLICK) {
		EnableWindow(cpsSpinnerHWD, TRUE);
		EnableWindow((HWND)SendMessage(cpsSpinnerHWD, UDM_GETBUDDY, NULL, NULL), TRUE);
		EnableWindow(mouseButtonComboBoxHWD, TRUE);
		EnableWindow(pressHotKey, FALSE);
		EnableWindow(ppsSpinnerHWD, FALSE);
		EnableWindow((HWND)SendMessage(ppsSpinnerHWD, UDM_GETBUDDY, NULL, NULL), FALSE);
	}
	else if (mode == MODE_AUTO_PRESS) {
		EnableWindow(cpsSpinnerHWD, FALSE);
		EnableWindow((HWND)SendMessage(cpsSpinnerHWD, UDM_GETBUDDY, NULL, NULL), FALSE);
		EnableWindow(mouseButtonComboBoxHWD, FALSE);
		EnableWindow(pressHotKey, TRUE);
		EnableWindow(ppsSpinnerHWD, TRUE);
		EnableWindow((HWND)SendMessage(ppsSpinnerHWD, UDM_GETBUDDY, NULL, NULL), TRUE);
	}
}