#pragma once

#include "LMDefs.h"

SolidColorRect RandomizeRect(int size);

void SolidColorRect_dtor(SolidColorRect* rect);

HBRUSH RandomSolidColor();

void SwapMemory(void* a, void* b, size_t r);

void InitMemory(void* trg, size_t r);

inline float randf() {
	return (float)rand() / (float)RAND_MAX;
}

inline void UpdateDebugger(Debugger* debugger) {
	debugger->counter++;
	if (debugger->counter < debugger->procRate)
		return;

	debugger->counter = 0;

	int i = 0;
	while (debugger->debugCall[i]) {
		debugger->debugCall[i](debugger->args[i]);
		i++;
	}
}