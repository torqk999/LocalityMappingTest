#pragma once

#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include "LMDefs.h"
#include "Utils.h"


void DrawRect(SolidColorRect* rect, HDC hdc);

LRESULT CALLBACK defaultWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

GridRenderContainer CreateDefaultPixelFormattedDrawContext(
	const wchar_t* windowName, WNDPROC windowProcess, GridSpace* gridSpace);




//int main() {
//
//    brush = CreateSolidBrush(RGB(255, 0, 0));
//
//    // Create OpenGL context and get the HDC
//    //ContextContainer cc = CreateDefaultPixelFormattedDrawContext(L"Test Window");
//
//
//    
//
//    // Main message loop
//    MSG msg = {};
//    while (GetMessage(&msg, nullptr, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return 0;
//}
