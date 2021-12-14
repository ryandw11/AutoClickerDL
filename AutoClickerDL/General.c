#include <Windows.h>
#include <Commctrl.h>
#include <Shlobj.h>

#include "General.h"

/*
	Create a display area for the tab. (This create a seperate window class for each tab display area.)

	@param parent The parent window for the display area.
	@param hInstance The hInstance for the program.
	@param className The class name to use for the created Window Class.
	@param x The x coordinate of the display area.
	@param y The y coordinate of the display area.
	@param width The width of the dispaly area.
	@param height The height of the display area.
	@param procCallback The callback function to use for the process callback.

	@returns The handle for the created display area.
*/
HWND CreateTabDisplayArea(HWND parent, HINSTANCE hInstance, LPCWSTR className, int x, int y, int width, int height, WNDPROC procCallback) {
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = procCallback;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = className;
	wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));

	RegisterClass(&wc);

	HWND displayArea = CreateWindowEx(NULL, className, className, WS_CHILD | WS_BORDER, x, y, width, height, parent, NULL, hInstance, NULL);
	return displayArea;
}

/*
	Create a spinner control.

	@param parent The parent window of the spinner.
	@param hInstance The hInstance of the program.
	@param spinner The struct that details the creation of the spinner.

	@returns The handle to the UPDOWN control.
*/
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