#pragma once

typedef struct {
	int x;
	int y;
	int rangeMin;
	int rangeMax;
	int step;
	LPCWSTR labelPtr;
	int labelSize;
	LPCWSTR defaultValue;
} Spinner;

HWND CreateTabDisplayArea(HWND parent, HINSTANCE hInstance, LPCWSTR className, int width, int height, WNDPROC procCallback);
HWND CreateSpinner(HWND parent, HINSTANCE hInstance, Spinner spinner);
HWND CreateCheckBox(HWND parent, HINSTANCE hInstance, LPCWSTR text, int x, int y, int width, int height, WNDPROC procCallback);