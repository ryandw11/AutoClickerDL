#pragma once
#ifndef IO_H
#define IO_H

#include "General.h"

void loadSettings(Settings* settings);
int saveSettings(const char* fileName, Settings* settings);

#endif