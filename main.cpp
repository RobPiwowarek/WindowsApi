#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>

#define GAS_RADIUS 125
#define PAIN_WIDTH 10
#define PAIN_HEIGHT 10
#define PAIN_START_POSITION_Y 10
#define PAIN_START_POSITION_X_LEFT 0.1
#define PAIN_BALL_DX 10
#define PAIN_BALL_DY 2

#define SETMAPMODE() {\
	GetClientRect(hwnd, &rect);\
	SetViewportOrgEx(hdc, rect.right/2, rect.bottom/2, NULL);\
	SetMapMode(hdc, MM_LOMETRIC);\
	}

typedef struct GasParticle {
    HWND hwnd;

    int startX, startY;
    int radius;
    int x, y;
    int dx, dy;

} GasParticle;

COLORREF gasColor = RGB(168, 43, 126);
char appName[] = "Gas diffuse";

std::vector<GasParticle *> gases;
std::vector<GasParticle *> toRemove;
const int GAS_COUNT = 8;
HWND GAS_HWND;
HWND OTHER_CONTAINER_HALF_HWND;
bool connected = false;

int PARTICLE_MOVES_RIGHT = RegisterWindowMessage("RIGHT");
int PARTICLE_MOVES_LEFT = RegisterWindowMessage("LEFT");
int CONNECT_RIGHT = RegisterWindowMessage("CONNECT_RIGHT");
int CONNECT_LEFT = RegisterWindowMessage("CONNECT_LEFT");

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void PaintBoard(HDC *hdc, RECT rect);

void PaintGasParticle(HDC *, BOOL visibility = TRUE);

void InitializeGasParticle(GasParticle *gas);

void MoveGasParticles(HWND hwnd, RECT rect);

int randomlyGenerate1OrMinus1();

int CmToPixels(double cm) {
    double tst = (double) GetDeviceCaps(GetDC(NULL), LOGPIXELSY);
    return (int) ((double) (tst / 2.54) * cm);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    srand(time(NULL));

    for (int i = 0; i < GAS_COUNT; ++i) {
        gases.push_back(new GasParticle());
    }

    for (auto it: gases) {
        InitializeGasParticle(it);
    }

    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    /* The Window structure */
    wincl.hInstance = hInstance;
    wincl.lpszClassName = appName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    GAS_HWND = hwnd = CreateWindowEx(
            0,                   /* Extended possibilites for variation */
            appName,         /* Classname */
            "Gas Diffuse",       /* Title Text */
            WS_OVERLAPPEDWINDOW, /* default window */
            CW_USEDEFAULT,       /* Windows decides the position */
            CW_USEDEFAULT,       /* were the window ends up on the screen */
            CmToPixels(PAIN_WIDTH) - GetSystemMetrics(SM_CXSIZE),    /* The programs width */
            CmToPixels(PAIN_HEIGHT),/* and height in pixels */
            HWND_DESKTOP,        /* The window is a child-window to desktop */
            NULL,                /* No menu */
            hInstance,       /* Program Instance handler */
            NULL                 /* No Window Creation data */
    );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);

    SendMessageA(HWND_BROADCAST, CONNECT_LEFT, reinterpret_cast<WPARAM>(GAS_HWND), 0);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0)) {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void InitializeGasParticle(GasParticle *gas) {
    gas->radius = GAS_RADIUS;
    gas->dx = PAIN_BALL_DX * randomlyGenerate1OrMinus1();
    gas->dy = PAIN_BALL_DY * randomlyGenerate1OrMinus1();

    gas->startX = -326-2*gas->radius;
    gas->startY = 0;

    gas->x = gas->startX;
    gas->y = gas->startY;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    const WORD ID_TIMER = 1;
    GetClientRect(hwnd, &rect);
    SETMAPMODE();
    DPtoLP(hdc, (PPOINT) &rect, 2);
    std::cout << rect.right << " " << rect.left << " " << rect.top << " " << rect.bottom << std::endl;

    if (message == PARTICLE_MOVES_LEFT) {
        GasParticle *gas = new GasParticle();
        gas->dy = lParam;
        gas->y = wParam;
        gas->x = rect.right;
        gas->radius = GAS_RADIUS;
        gas->dx = -PAIN_BALL_DX;
        gas->hwnd = GAS_HWND;

        gases.push_back(gas);
    } else if (message == CONNECT_RIGHT && !connected) {
        OTHER_CONTAINER_HALF_HWND = reinterpret_cast<HWND>(wParam);
        SendMessageA(OTHER_CONTAINER_HALF_HWND, CONNECT_LEFT, reinterpret_cast<WPARAM>(GAS_HWND), 0);
        connected = true;
    } else
        switch (message) {
            case WM_CREATE:
                if (SetTimer(hwnd, ID_TIMER, 50, NULL) == 0)
                    MessageBox(hwnd, "Cannot create timer!", "Fatal error!", MB_ICONSTOP);
            case WM_PAINT:
                hdc = BeginPaint(hwnd, &ps);
                PaintBoard(&hdc, rect);
                SETMAPMODE();
                DPtoLP(hdc, (PPOINT) &rect, 2);
                if (connected)
                    MoveGasParticles(hwnd, rect);
                PaintGasParticle(&hdc);
                EndPaint(hwnd, &ps);
                return 0;
            case WM_TIMER:
                hdc = GetDC(hwnd);
                if (connected) {
                    SETMAPMODE();
                    DPtoLP(hdc, (PPOINT) &rect, 2);
                    PaintGasParticle(&hdc, FALSE);
                    MoveGasParticles(hwnd, rect);
                    PaintGasParticle(&hdc, TRUE);
                }
                ReleaseDC(hwnd, hdc);
                break;
            case WM_DESTROY:
                PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
                break;
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
        }
    return 0;
}

void MoveGasParticles(HWND hwnd, RECT rect) {
    for (auto gas: gases) {
        gas->x += gas->dx;
        gas->y += gas->dy;

        if (gas->y <= rect.bottom - 2*gas->radius) {
            gas->y = rect.bottom + gas->radius;
            gas->dy *= -1; //odwrocenie
        } else if (gas->x <= -rect.left - 2 * gas->radius) {
            gas->x = -rect.left - gas->radius;
            gas->dx *= -1;
        } else if (gas->y >= rect.bottom) {
            gas->dy *= -1;
        } else if (gas->x > rect.right) {
            SendMessageA(OTHER_CONTAINER_HALF_HWND, PARTICLE_MOVES_RIGHT, gas->y, gas->dy);
            toRemove.push_back(gas);
        } else {
            for (auto it: gases) {
                if (it == gas)
                    continue;
                else {
                    if (abs(it->x - gas->x) < gas->radius)
                        if (abs(it->y - gas->y) < gas->radius) {
                            auto tempDx = gas->dx;
                            auto tempDy = gas->dy;

                            gas->dx = it->dx;
                            gas->dy = it->dy;
                            it->dx = tempDx;
                            it->dy = tempDy;
                        }
                }
            }
        }
    }

    for (auto gasToRemove: toRemove) {
        auto it = std::remove(gases.begin(), gases.end(), gasToRemove);
        gases.erase(it);
    }

    if (toRemove.size() != 0)
        toRemove.clear();
}

int randomlyGenerate1OrMinus1() {
    if (rand() % 2 == 0)
        return -1;
    else
        return 1;
}

void PaintBoard(HDC *hdc, RECT rect) {
    HBRUSH hBrush = CreateSolidBrush(RGB(10, 20, 30));
    SelectObject(*hdc, hBrush);
    Rectangle(*hdc, rect.left, rect.top, rect.right, rect.bottom);
}

void PaintGasParticle(HDC *hdc, BOOL visibility) {
    HPEN hPen;
    HBRUSH hBrush;

    if (visibility == TRUE) {
        hPen = CreatePen(PS_SOLID, 1, gasColor);
        hBrush = CreateSolidBrush(gasColor);
    } else {
        hPen = CreatePen(PS_SOLID, 1, RGB(10, 20, 30));
        hBrush = CreateSolidBrush(RGB(10, 20, 30));
    }
    SelectObject(*hdc, hPen);
    SelectObject(*hdc, hBrush);

    for (auto it: gases) {
        Ellipse(*hdc, it->x, it->y, it->x + it->radius, it->y + it->radius);
    }

    DeleteObject(hPen);
    DeleteObject(hBrush);
}