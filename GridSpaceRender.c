#include "GridSpaceRender.h"

GridRenderContainer renderContainer;

//SolidColorRect exampleRect = { 50, 50, 450, 450, NULL };

//HBRUSH exampleBrush;

void GridSpaceRender_renderGrid(GridRenderContainer* grc) {
    if (!grc->grid)
        return;

    SolidColorRect rect;

    for (int i = 0; i < grc->grid->totalCellCount; i++) {
        //Entity* next = GridSpace_getEntityByIndex(renderContainer.grid, i);
        GridSpace_getSolidRectFromCell(grc->grid, &rect, i);
        DrawRect(&rect, grc->swap);
        SolidColorRect_dtor(&rect);
    }
}

void GridSpaceRender_renderElements(GridRenderContainer* grc) {
    if (!grc->grid)
        return;

    SolidColorRect rect;

    for (int i = 0; i < grc->grid->entityCount; i++) {
        Entity* next = GridSpace_getEntityByIndex(grc->grid, i);
        if (!Entity_isActive(next))
            continue;
        GridSpace_getSolidRectFromEntity(next, &rect);
        DrawRect(&rect, grc->swap);
    }
}

LRESULT GridSpaceRender_messageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg) {

    //case WM_CREATE:
//    {
//    HDC hdcWindow = GetDC(hwnd);
//    renderContainer.swap = CreateCompatibleDC(hdcWindow);
//    RECT rc;
//    GetClientRect(hwnd, &rc);
//    renderContainer.swapMap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
//    SelectObject(renderContainer.swap, renderContainer.swapMap);
//    return 0;
//}
 
    //case WM_ERASEBKGND: {
//    //HDC hdc = (HDC)wParam;
//    //RECT rc;
//    //GetClientRect(hwnd, &rc);
//    //FillRect(renderContainer.hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
//    return TRUE; // Indicate that the background has been erased
//}

    case WM_DESTROY:
        DeleteObject(renderContainer.swapMap);
        DeleteDC(renderContainer.swap);
        ReleaseDC(hwnd, renderContainer.hdc);
        PostQuitMessage(0);
        return 0;

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        renderContainer.swapMap = CreateCompatibleBitmap(renderContainer.hdc, rc.right - rc.left, rc.bottom - rc.top);
        SelectObject(renderContainer.swap, renderContainer.swapMap);
    }

    case WM_PAINT: {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(renderContainer.swap, &rc, (HBRUSH)(COLOR_WINDOW + 1));

        GridSpaceRender_renderGrid(&renderContainer);
        GridSpaceRender_renderElements(&renderContainer);

        BitBlt(hdc, 0, 0,
            rc.right - rc.left,
            rc.bottom - rc.top,
            renderContainer.swap,
            0, 0, SRCCOPY
        );

        EndPaint(hwnd, &ps);
        return 0;
    }
    default:
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void GridSpaceRender_loop(GridSpace* gridSpace) {

    renderContainer = CreateDefaultPixelFormattedDrawContext(L"TestWindow", GridSpaceRender_messageHandler, gridSpace);
    //renderContainer.grid = gridSpace;

    // Main message loop
    MSG msg = { 0 };

    while (GetMessage(&msg, renderContainer.hwnd, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        InvalidateRect(renderContainer.hwnd, NULL, 1);
    }
}

void GridSpaceRender_init(GridSpace* gridSpace) {

    //exampleBrush = CreateSolidBrush(RGB(255, 0, 0));
    //exampleRect.brush = exampleBrush;

    _beginthread(GridSpaceRender_loop, 0, gridSpace);
}