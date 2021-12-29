#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "IO.h"

#pragma warning(disable: 4996)
void loadSettings(const char* fileName, Settings* settings) {
	FILE* inputFile = fopen(fileName, "r");

	if (inputFile == NULL) {
		// Unable to open file.
		settings->cps = 2;
		settings->timedAutoClick = 0;
		settings->timedAutoClickValue = 5;
		settings->delayTime = 0;
		settings->hotkey = 112;
		settings->rmbStartHotkey = VK_F2;
		settings->rmbPlayHotKey = VK_F3;
		return;
	}

	char hotkey[5] = { 0 }, rmbStartHotkey[5] = { 0 }, rmbStopHotkey[5] = { 0 }, rmbPlayHotkey[5] = {0};
	char ver;
	char cps, timedAutoClick, timedAutoClickValue, delayTime, mouseClickType;
	int output = fscanf(inputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", &ver, &cps, &timedAutoClick, &timedAutoClickValue, &delayTime, &mouseClickType, 
		&hotkey[0], &hotkey[1], &hotkey[2], &hotkey[3],
		&rmbStartHotkey[0], &rmbStartHotkey[1], &rmbStartHotkey[2], &rmbStartHotkey[3],
		&rmbPlayHotkey[0], &rmbPlayHotkey[1], &rmbPlayHotkey[2], &rmbPlayHotkey[3]
	);
	settings->cps = cps;
	settings->timedAutoClick = timedAutoClick;
	settings->timedAutoClickValue = timedAutoClickValue;
	settings->delayTime = delayTime;
	settings->mouseClickType = mouseClickType;

	int hotkeyI = atoi(hotkey);
	settings->hotkey = hotkeyI;
	settings->rmbStartHotkey = atoi(rmbStartHotkey);
	settings->rmbPlayHotKey = atoi(rmbPlayHotkey);
	fclose(inputFile);
}


// FILE FORMAT::
// (1 bytes vernum) - (1 bytes CPS) - (1 byte bool) - (1 bytes - timedAutoClickerValue) - (1 bytes - delay time) - (1 bytes - mouse click type) - (4 bytes - hotkey) -
// (4 bytes - start recording hotkey) - (4 bytes - play recording hotkey)
int saveSettings(const char* fileName, Settings* settings) {
	FILE* outputFile = fopen(fileName, "w");

	if (outputFile == NULL) {
		// Unable to open file.
		return -1;
	}
	char hotkey[5] = { 0 }, rmbStartHotkey[5] = { 0 }, rmbPlayHotkey[5] = { 0 };
	itoa(settings->hotkey, hotkey, 10);
	itoa(settings->rmbStartHotkey, rmbStartHotkey, 10);
	itoa(settings->rmbPlayHotKey, rmbPlayHotkey, 10);
	fprintf(outputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", IO_ACDL_SETTINGS_FILE, settings->cps, settings->timedAutoClick, settings->timedAutoClickValue, settings->delayTime, settings->mouseClickType,
		hotkey[0],hotkey[1],hotkey[2],hotkey[3],
		rmbStartHotkey[0], rmbStartHotkey[1], rmbStartHotkey[2], rmbStartHotkey[3],
		rmbPlayHotkey[0], rmbPlayHotkey[1], rmbPlayHotkey[2], rmbPlayHotkey[3]
	);
	fclose(outputFile);
	return 0;
}

void fputi(FILE* file, int input) {
	char charr[5] = { 0 };
	itoa(input, charr, 10);
	fprintf(file, "%c%c%c%c", charr[0], charr[1], charr[2], charr[3]);
}

void fputl(FILE* file, LONG input) {
	char charr[9] = { 0 };
	ltoa(input, charr, 10);
	fprintf(file, "%c%c%c%c%c%c%c%c", charr[0], charr[1], charr[2], charr[3], charr[4], charr[5], charr[6], charr[7]);
}

BOOL fgeti(FILE* file, int* output) {
	char charr[5] = { 0 };
	int opt = fscanf(file, "%c%c%c%c", &charr[0], &charr[1], &charr[2], &charr[3]);
	*output = atoi(charr);
	return opt != 4 ? FALSE : TRUE;
}

BOOL fgetl(FILE* file, LONG* output) {
	char charr[9] = { 0 };
	int opt = fscanf(file, "%c%c%c%c%c%c%c%c", &charr[0], &charr[1], &charr[2], &charr[3], &charr[4], &charr[5], &charr[6], &charr[7]);
	*output = atol(charr);
	return opt != 8 ? FALSE : TRUE;
}