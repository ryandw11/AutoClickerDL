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

#define KB_TYPE_ERROR 0
#define KB_TYPE_KEYUP 1
#define KB_TYPE_KEYDOWN 2
#define KB_TYPE_SYSKEYUP 3
#define KB_TYPE_SYSKEYDOWN 4

#define REC_STATE_NONE 1
#define REC_STATE_LOADED 2
#define REC_STATE_RECORDING 3
#define REC_STATE_PLAYING 4

#define MODE_ERROR 0
#define MODE_AUTO_CLICK 1
#define MODE_AUTO_PRESS 2

#define RC_MOUSE_CLICK 0
#define RC_KEY_PRESS 1

/*
	The representation of a MouseClick for use with the Remeber Click feature.
*/
typedef struct {
	// The type of click (denoted by MC_TYPE_ macros).
	int type;
	// The x position.
	LONG x;
	// The y position.
	LONG y;
} MouseClick;

typedef struct {
	// The type of click (denoted by KB_TYPE_ macros).
	int type;
	// The key.
	int key;
} KeyPress;

typedef struct {
	// The type of the RememberClick (RC_MOUSE_CLICK or RC_KEY_PRESS).
	int type;
	// The delay (in milliseconds) from the previous click.
	int delay;
	union {
		MouseClick mi;
		KeyPress ki;
	};
	// The pointer to the next click in the recording.
	struct RememberClick* next;
} RememberClick;

/*
	The current state of the recording system for the Remember Click feature.
*/
typedef struct {
	// The current state (denoted by REC_STATE_ macros).
	int state;
	// The first click in a recording.
	RememberClick* startOfRecording;
	// The previous click that was done in a recording.
	RememberClick* previousClick;
	// The previous system time in milliseconds.
	DWORD prevoiusSystemTime;
	// The number of clicks stored.
	int numberOfClicks;
} RecordingState;

/*
	The struct that contains all of the settings for the program.
*/
typedef struct {
	int cps;
	int mouseClickType;
	int pps;
	int autoPressKey;
	int mode;
	BOOL timedAutoClick;
	int timedAutoClickValue;
	int delayTime;
	int pressKeyUp;
	int hotkey;
	int rmbStartHotkey;
	int rmbPlayHotKey;
} Settings;

/*
	The struct used to define the properties of a spinner for creation.
*/
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
int WMToKB(int wParam);
int KBToEventKey(int kb);

void InitRecordingState(RecordingState*);
void AddRememberClickToState(RecordingState*, RememberClick);
void NextMouseClick(RecordingState*);
void DeleteRecordingState(RecordingState*);

BOOL SaveRecordingState(RecordingState*, LPCWSTR);
BOOL LoadRecordingState(RecordingState*, LPCWSTR);

#endif