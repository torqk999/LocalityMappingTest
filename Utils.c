#include "Utils.h"

SolidColorRect RandomizeRect(int size) {
    SolidColorRect rect = { 0,0,0,0,NULL };
    int left = rand() % 800;
    int top = rand() % 600;
    rect.rect.left = left;
    rect.rect.top = top;
    rect.rect.right = left + size;
    rect.rect.bottom = top + size;
    rect.brush = RandomSolidColor();
    return rect;
}

void SolidColorRect_dtor(SolidColorRect* rect) {
    DeleteObject(rect->brush);
    rect->brush = NULL;
}

HBRUSH RandomSolidColor() {
    return CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
}

void SwapMemory(void* a, void* b, size_t r){
    size_t* locA = a;
    size_t* locB = b;
    size_t swap;
    for (size_t i = 0; (i * sizeof(size_t)) < r; i++) {

        swap = locA[i];
        locA[i] = locB[i];
        locB[i] = swap;
    }
}

void InitMemory(void* trg, size_t r){
    size_t* locTrg = trg;
    for (size_t i = 0; (i * sizeof(size_t)) < r; i++)
        locTrg[i] = 0;
}

//locA[i] ^= locB[i];
//locB[i] ^= locA[i];
//locA[i] ^= locB[i];
