#include "Render.h"

PIXELFORMATDESCRIPTOR defaultPFD = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0, 0, 0, 0,
    24,
    8,
    0,
    PFD_MAIN_PLANE,
    0,
    0, 0, 0
};

void DrawRect(SolidColorRect* rect, HDC hdc) {
    SelectObject(hdc, rect->brush);
    Rectangle(
        hdc,
        rect->rect.left,
        rect->rect.top,
        rect->rect.right,
        rect->rect.bottom
    );
}

LRESULT defaultWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        //DrawFrame(hdc);
        EndPaint(hwnd, &ps);
    }
    return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

#define DEF_WIDTH 800
#define DEF_HEIGHT 600

GridRenderContainer CreateDefaultPixelFormattedDrawContext(
    const wchar_t* windowName,
    WNDPROC windowProcess,
    GridSpace* gridSpace
) {

    //GridRenderContainer cc;

    const wchar_t* className = L"OpenGLWindowClass";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = windowProcess;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = className;
    RegisterClassW(&wc);

    float c = gridSpace->cellSize;

    int width = c > 0 ? gridSpace->dimCount > 0 ? c * gridSpace->dimCellCounts[0] : DEF_WIDTH : DEF_WIDTH;
    int height = c > 0 ? gridSpace->dimCount > 1 ? c * gridSpace->dimCellCounts[1] : DEF_HEIGHT : DEF_HEIGHT;

    // Create the window
    HWND hwnd = CreateWindowExW(
        0,
        className,
        windowName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hwnd) {
        printf("Failed to create window");
        return (GridRenderContainer) {0,0,0,0,0,0};
    }

    PIXELFORMATDESCRIPTOR pfd = defaultPFD;

    HDC hdc = GetDC(hwnd);
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HDC swap = CreateCompatibleDC(hdc);

    //printf("%d\n", GetLastError());

    RECT rc;
    GetClientRect(hwnd, &rc);
    HBITMAP swapMap = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
    SelectObject(swap, swapMap);

    ShowWindow(hwnd, SW_SHOW);

    return (GridRenderContainer){
        hwnd,
        hdc,
        swap,
        swapMap,
        NULL,
        gridSpace
    };
}

