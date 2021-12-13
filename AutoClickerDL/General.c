#include <Windows.h>
#include <Commctrl.h>
#include <Shlobj.h>

#include "General.h"

HWND CreateTabDisplayArea(HWND parent, HINSTANCE hInstance, LPCWSTR className, int x, int y, int width, int height, WNDPROC procCallback) {
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = procCallback;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = className;

	RegisterClass(&wc);

	HWND displayArea = CreateWindowEx(NULL, className, className, WS_CHILD, x, y, width, height, parent, NULL, hInstance, NULL);
	return displayArea;
}

HWND CreateSpinner(HWND parent, HINSTANCE hInstance, Spinner spinner) {
	HWND cpsLabel = CreateWindow(WC_STATIC, spinner.labelPtr, WS_VISIBLE | WS_CHILD | SS_LEFT, spinner.x + 64, spinner.y, 40, 20, parent, NULL, hInstance, NULL);
	HWND spinnerUpDown = CreateWindow(UPDOWN_CLASS, L"F", WS_VISIBLE | WS_CHILD | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
		0, 0, 0, 0, parent, NULL, hInstance, NULL);
	HWND spinnerEdit = CreateWindow(WC_EDIT, L"2", WS_VISIBLE | WS_CHILD, spinner.x, spinner.y, 60, 20, parent, NULL, hInstance, NULL);
	// Set the buddy of the spinner.
	SendMessage(spinnerUpDown, UDM_SETBUDDY, spinnerEdit, NULL);
	UDACCEL spinnerValueChange = { 0 };
	spinnerValueChange.nSec = 0.5;
	spinnerValueChange.nInc = spinner.step * -1;
	// Fix the spinner to work the way you think one would work.
	SendMessage(spinnerUpDown, UDM_SETACCEL, 1, &spinnerValueChange);
	// Set the range of the spinner to 1-80.
	SendMessage(spinnerUpDown, UDM_SETRANGE, NULL, MAKELPARAM(spinner.rangeMin, spinner.rangeMax));

	return (spinnerUpDown);
}