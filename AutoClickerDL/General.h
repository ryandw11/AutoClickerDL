#pragma once

#ifndef GENERAL_H
#define GENERAL_H

#define AutoClickerDL_VER_NUM 1

#define MC_TYPE_ERROR 0
#define MC_TYPE_LEFT_DOWN 1
#define MC_TYPE_MIDDLE_DOWN 2
#define MC_TYPE_RIGHT_DOWN 3
#define MC_TYPE_LEFT_UP 4
#define MC_TYPE_MIDDLE_UP 5
#define MC_TYPE_RIGHT_UP 6

#define REC_STATE_NONE 1
#define REC_STATE_LOADED 2
#define REC_STATE_RECORDING 3
#define REC_STATE_PLAYING 4

typedef struct {
	int type;
	int delay;
	LONG x;
	LONG y;
	struct MouseClick* nextClick;
} MouseClick;

typedef struct {
	int state;
	MouseClick* startOfRecording;
	MouseClick* previousClick;
	DWORD prevoiusSystemTime;
	int numberOfClicks;
} RecordingState;

typedef struct {
	int cps;
	BOOL timedAutoClick;
	int timedAutoClickValue;
	int delayTime;
	int mouseClickType;
	int hotkey;
	int rmbStartHotkey;
	int rmbPlayHotKey;
} Settings;

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

int WMToMC(int wParam);
int MCToEventMouse(int mc);

void InitRecordingState(RecordingState*);
void AddMouseClickToState(RecordingState*, MouseClick);
void NextMouseClick(RecordingState*);
void DeleteRecordingState(RecordingState*);

BOOL SaveRecordingState(RecordingState*, LPCWSTR);
BOOL LoadRecordingState(RecordingState*, LPCWSTR);

#endif