#include <Windows.h>
#include <Commctrl.h>
#include <Shlobj.h>
#include <stdio.h>

#include "General.h"
#include "IO.h"

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
	HWND cpsLabel = CreateWindow(WC_STATIC, spinner.labelPtr, WS_VISIBLE | WS_CHILD | SS_LEFT, spinner.x + 64, spinner.y, 60, 20, parent, NULL, hInstance, NULL);
	HWND spinnerUpDown = CreateWindow(UPDOWN_CLASS, L"F", WS_VISIBLE | WS_CHILD | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
		0, 0, 0, 0, parent, NULL, hInstance, NULL);
	HWND spinnerEdit = CreateWindow(WC_EDIT, spinner.defaultValue, WS_VISIBLE | WS_CHILD, spinner.x, spinner.y, 60, 20, parent, NULL, hInstance, NULL);
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

/**
	Convert Windows Mouse (provided from windows mouse hook) value into the Mouse Click Type value.

	@param wParam The WM value (example: WM_LBUTTONDOWN).
	@returns The MC value (example: MC_TYPE_LEFT_DOWN). (Returns MC_TYPE_ERROR if not valid).
*/
int WMToMC(int wParam) {
	switch (wParam)
	{
	case WM_LBUTTONDOWN:
		return MC_TYPE_LEFT_DOWN;
	case WM_LBUTTONUP:
		return MC_TYPE_LEFT_UP;
	case WM_RBUTTONDOWN:
		return MC_TYPE_RIGHT_DOWN;
	case WM_RBUTTONUP:
		return MC_TYPE_RIGHT_UP;
	case WM_MBUTTONDOWN:
		return MC_TYPE_MIDDLE_DOWN;
	case WM_MBUTTONUP:
		return MC_TYPE_MIDDLE_UP;
	default:
		break;
	}
	return 0;
}

/**
	Convert Windows Keyboard (provided from windows keyboard hook) value into the Keyboard Click Type value.

	@param wParam the WM value (Example: WM_KEYDOWN)
	@returns The KB value (example: KB_TYPE_KEYDOWN). (Returns KB_TYPE_ERROR if not valid).
*/
int WMToKB(int wParam) {
	switch (wParam) 
	{
	case WM_KEYDOWN:
		return KB_TYPE_KEYDOWN;
	case WM_KEYUP:
		return KB_TYPE_KEYUP;
	case WM_SYSKEYDOWN:
		return KB_TYPE_SYSKEYDOWN;
	case WM_SYSKEYUP:
		return KB_TYPE_SYSKEYUP;
	}
	return 0;
}

/*
	Convert a MC Type to a Mouse Event Type.
	
	@param mc The mouse click type. (Example: MC_TYPE_LEFT_DOWN).
	@returns The event mouse type. (Example: MOUSEEVENTF_LEFTDOWN) (Returns MC_TYPE_ERROR if invalid input).
*/
int MCToEventMouse(int mc) {
	switch (mc) {
	case MC_TYPE_LEFT_DOWN:
		return MOUSEEVENTF_LEFTDOWN;
	case MC_TYPE_LEFT_UP:
		return MOUSEEVENTF_LEFTUP;
	case MC_TYPE_RIGHT_DOWN:
		return MOUSEEVENTF_RIGHTDOWN;
	case MC_TYPE_RIGHT_UP:
		return MOUSEEVENTF_RIGHTUP;
	case MC_TYPE_MIDDLE_DOWN:
		return MOUSEEVENTF_MIDDLEDOWN;
	case MC_TYPE_MIDDLE_UP:
		return MOUSEEVENTF_MIDDLEUP;
	default:
		return 0;
	}
}

int KBToEventKey(int kb) {
	switch (kb) {
	case KB_TYPE_SYSKEYDOWN:
	case KB_TYPE_KEYUP:
		return KEYEVENTF_KEYUP;
	}
	return 0;
}

/**
	Initalize the recording state. (Note: the default state is NONE).

	@param The pointer to the recording state to initalize.
*/
void InitRecordingState(RecordingState* state) {
	state->numberOfClicks = 0;
	state->startOfRecording = NULL;
	state->previousClick = NULL;
	state->state = REC_STATE_NONE;
	state->prevoiusSystemTime = 0;
}

/**
	Add a remember click the recording. (This will take care of every case and increment the numberOfClicks field).

	@param recState The pointer to the recording state to add the mouse click to.
	@param rememberClick The remember click struct to add. (Note: rememberClick.next does not need to be set).
*/
void AddRememberClickToState(RecordingState* recState, RememberClick rememberClick) {
	RememberClick* permElem = (RememberClick*)malloc(sizeof(RememberClick));
	if (permElem == NULL) return;

	permElem->delay = rememberClick.delay;
	permElem->type = rememberClick.type;
	permElem->next = NULL;

	if (rememberClick.type == RC_MOUSE_CLICK) {
		permElem->mi.type = rememberClick.mi.type;
		permElem->mi.x = rememberClick.mi.x;
		permElem->mi.y = rememberClick.mi.y;
	}
	else if(rememberClick.type = RC_KEY_PRESS) {
		permElem->ki.type = rememberClick.ki.type;
		permElem->ki.key = rememberClick.ki.key;
	}
	
	if (recState->numberOfClicks == 0) {
		recState->startOfRecording = permElem;
		recState->previousClick = permElem;
		recState->numberOfClicks = 1;
		return;
	}
	recState->previousClick->next = permElem;
	recState->previousClick = permElem;
	recState->numberOfClicks++;
}

/**
	Get the next mouse click for the recording state. (If the end of the recording is reached, it will look back to the begining). 
	Get the pointer to the RememberClick struct from `recState.prevoiusClick`.

	@param recState The recording state to get the next mouse click from.
*/
void NextMouseClick(RecordingState* recState) {
	recState->previousClick = recState->previousClick->next == NULL ? recState->startOfRecording : recState->previousClick->next;
}

/**
	Delete (free) the memory used by the recording state. (Note: you should call InitRecordingState() if you intended on reusing the same struct).

	@param state The recording state to free the memory from.
*/
void DeleteRecordingState(RecordingState* state) {
	if (state->numberOfClicks == 0) return;
	state->previousClick = state->startOfRecording;
	while (state->previousClick != NULL) {
		RememberClick* current = state->previousClick;
		state->previousClick = state->previousClick->next;
		free(current);
	}
	state->previousClick = NULL;
	state->startOfRecording = NULL;
	state->numberOfClicks = 0;
	state->state = REC_STATE_NONE;
}

/**
	Save a recording state to a file.  
	Note: The `previousClick` field of the recording state will be mutated.

	@param state The recording state to save to a file.
	@param location The location and name of the file to save to. (This is intented to be the output of a save file dialog box).

	@returns If the save was successful.
*/
BOOL SaveRecordingState(RecordingState* state, LPCWSTR location) {
	#pragma warning(disable: 4996)

	if (state->numberOfClicks == 0) {
		return FALSE;
	}
	state->previousClick = state->startOfRecording;

	FILE* outputFile = _wfopen(location, L"w");
	if (outputFile == NULL) return FALSE;
	fputc(IO_ACDL_RECORDING_FILE, outputFile);
	fputi(outputFile, state->numberOfClicks);
	while (state->previousClick != NULL) {
		RememberClick* currentClick = state->previousClick;
		fputi(outputFile, currentClick->type);
		fputi(outputFile, currentClick->delay);
		if (currentClick->type == RC_MOUSE_CLICK) {
			fputi(outputFile, currentClick->mi.type);
			fputl(outputFile, currentClick->mi.x);
			fputl(outputFile, currentClick->mi.y);
		}
		else {
			fputi(outputFile, currentClick->ki.type);
			fputi(outputFile, currentClick->ki.key);
		}
		state->previousClick = state->previousClick->next;
	}

	fclose(outputFile);

	state->previousClick = state->startOfRecording;

	return TRUE;
}

/**
	Load a recording state from a file.  
	Note: This will delete the passed in recording state an re-initalize it.  

	@param state The recording state to load into.
	@param location The location and name of the file to load. (Note: this is intended to be the output of an open file dialog box).

	@returns If the recording was successfully loaded. (If not, the recording state will be the default initalized state of NONE).
*/
BOOL LoadRecordingState(RecordingState* state, LPCWSTR location) {
	#pragma warning(disable: 4996)
	DeleteRecordingState(state);

	InitRecordingState(state);

	FILE* inputFile = _wfopen(location, L"r");
	if (inputFile == NULL) return FALSE;
	int type = fgetc(inputFile);
	if (type != IO_ACDL_RECORDING_FILE) {
		fclose(inputFile);
		return FALSE;
	}
	int count = 0;
	if (!fgeti(inputFile, &count)) {
		fclose(inputFile);
		return FALSE;
	}

	if (count <= 0) {
		fclose(inputFile);
		return FALSE;
	}

	int i = 0;
	for (i = 0; i < count; i++) {
		RememberClick rememberClick = { 0 };
		if (!fgeti(inputFile, &rememberClick.type) || !fgeti(inputFile, &rememberClick.delay)) {
			DeleteRecordingState(state);
			fclose(inputFile);
			return FALSE;
		}
		if (rememberClick.type == RC_MOUSE_CLICK) {
			if (!fgeti(inputFile, &rememberClick.mi.type) || !fgetl(inputFile, &rememberClick.mi.x) || !fgetl(inputFile, &rememberClick.mi.y)) {
				DeleteRecordingState(state);
				fclose(inputFile);
				return FALSE;
			}
		}
		else if (rememberClick.type = RC_KEY_PRESS) {
			if (!fgeti(inputFile, &rememberClick.ki.type) || !fgeti(inputFile, &rememberClick.ki.key)) {
				DeleteRecordingState(state);
				fclose(inputFile);
				return FALSE;
			}
		}
		else {
			DeleteRecordingState(state);
			fclose(inputFile);
			return FALSE;
		}
		AddRememberClickToState(state, rememberClick);
	}
	state->previousClick = state->startOfRecording;
	fclose(inputFile);
	state->state = REC_STATE_LOADED;

	return TRUE;
}