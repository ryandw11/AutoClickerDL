#pragma once
#ifndef IO_H
#define IO_H

#include "General.h"

#define IO_ACDL_SETTINGS_FILE 1
#define IO_ACDL_RECORDING_FILE 2

void applyDefaultSettings(Settings* settings);

void loadSettings(Settings* settings);
int saveSettings(const char* fileName, Settings* settings);

void fputi(FILE*, int);
void fputl(FILE*, LONG);

BOOL fgeti(FILE*, int*);
BOOL fgetl(FILE*, LONG*);

#endif