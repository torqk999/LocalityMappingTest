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