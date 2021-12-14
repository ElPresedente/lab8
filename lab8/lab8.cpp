// lab8.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "lab8.h"
#include <iostream>
#include <windowsx.h>

#define GAMEFIELD_YSIZE 8
#define GAMEFIELD_XSIZE 8

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM); 
BOOL                Line(HDC hdc, POINT first, POINT last);
ATOM                RegisterGameWndClass();

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB8));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB8));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB8);
    wcex.lpszClassName  = TEXT("MainClass");
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindow(TEXT("MainClass"), TEXT("Test"), WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK GameFieldWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GameField* data = (GameField*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    RECT r;
    GetClientRect(hWnd, &r);
    int offsetX = (r.bottom - r.top) / GAMEFIELD_XSIZE;
    int offsetY = (r.bottom - r.top) / GAMEFIELD_YSIZE;
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            //рисуем рамочку
            Rectangle(hdc, r.left, r.top, r.right, r.bottom);
            if (data == NULL)
            {
                EndPaint(hWnd, &ps);
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            //сохраняем изначальную кисть
            HGDIOBJ original = SelectObject(hdc, GetStockObject(DC_PEN));
            SelectObject(hdc, GetStockObject(DC_BRUSH));
            SetDCBrushColor(hdc, RGB(255, 0, 0));
            //далее отрисовываем подстреленные клитки
            for (int y = 0; y < GAMEFIELD_YSIZE; y++)
            {
                for (int x = 0; x < GAMEFIELD_XSIZE; x++)
                {
                    if (data->tilesArray[x + y * GAMEFIELD_XSIZE].wasShooted)
                    {
                        Rectangle(hdc, x * offsetX, y * offsetY, (x + 1) * offsetX, (y + 1) * offsetY);
                    }
                }
            }
            //отрисовка кораблей
            SetDCPenColor(hdc, RGB(255, 255, 255)); //рамка тайлов корабля и цвет кристика - белый
            for (const Ship &currentShip : data->shipsArray)
            {
                //для этого мы и дублировали данные - просто перерисовываем клетки под нужды кораблей
                for (const ShipTile &a : currentShip.shipInfo)
                {
                    if (currentShip.isDefeated) //если корабль полностью взорван - делаем его серее
                    {
                        SetDCBrushColor(hdc, RGB(100, 100, 100));
                    }
                    else 
                    {
                        SetDCBrushColor(hdc, RGB(0, 0, 0));
                    }
                    Rectangle(hdc, a.x * offsetX, a.y * offsetY, (a.x + 1) * offsetX, (a.y + 1) * offsetY);
                    if (a.wasShooted)
                    {
                        //MessageBox(0, 0, 0, 0);
                        Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), a.y * offsetY + (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) });
                        Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), a.y * offsetY + (offsetY / 6) });
                    }
                }

            }
            SelectObject(hdc, original);
            //отрисовка "поля"
            for (int i = 0; i <= GAMEFIELD_XSIZE; i++)
            {
                Line(hdc, POINT{ 0, offsetY * i }, POINT{ r.right , offsetY * i });
            }
            for (int i = 0; i <= GAMEFIELD_YSIZE; i++)
            {
                Line(hdc, POINT{ offsetX * i, 0 }, POINT{ offsetX * i, r.bottom });
            }
            SelectObject(hdc, original);
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_LBUTTONUP:
        {
            if(data == NULL)
                return DefWindowProc(hWnd, message, wParam, lParam);
            if(!data->playerCanInteract)
                return DefWindowProc(hWnd, message, wParam, lParam);
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int x = xPos / offsetX;
            int y = yPos / offsetY;
            if (!data->shoot(x, y))
                return 0;
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
            
            HWND parent = GetParent(hWnd);
            LPARAM coords = 0;
            if (data->checkForLoosing())
            {
                coords = SET_X_LPARAM(x, GAMEFIELD_XSIZE);
                coords = SET_Y_LPARAM(y, GAMEFIELD_XSIZE);
            }
            else 
            {
                coords = SET_X_LPARAM(x, coords);
                coords = SET_Y_LPARAM(y, coords);
            }
            SendMessage(parent, CM_SHOOT, (WPARAM)hWnd, coords);
        }
        break;
        case WM_DESTROY:
        {
            delete data;
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
        case CM_CONFIGURATE:
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
            return 0;
        }
        case CM_SHOOT:
        {
            int x, y;
            do 
            {
                x = rand() % GAMEFIELD_XSIZE;
                y = rand() % GAMEFIELD_YSIZE;
            }while (!data->shoot(x, y)); //стреляем до попадиния
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
        }
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND playerWnd = NULL;
    static HWND botWnd    = NULL;
    switch (message)
    {
    case CM_SHOOT:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            //TCHAR buf[100];
            
            HWND messageSender = (HWND)wParam;
            //wsprintf(buf, TEXT("x: %d, y: %d"), x, y);
            if (x == GAMEFIELD_XSIZE || y == GAMEFIELD_YSIZE)
            {
                if (messageSender == botWnd)
                {
                    MessageBox(hWnd, TEXT("You won!"), 0, 0);
                }
                else
                {
                    MessageBox(hWnd, TEXT("You lost!"), 0, 0);
                }
            }
            else
            {
                if (messageSender == botWnd)
                {
                    SendMessage(playerWnd, CM_SHOOT, 0, 0);
                    //MessageBox(0, buf, 0, 0);
                }
            }
        }
        break;
    case WM_CREATE:
        {   
            SYSTEMTIME time;
            GetSystemTime(&time);
            srand(time.wMilliseconds);
            RegisterGameWndClass();
            playerWnd = CreateWindow(TEXT("GameFieldClass"), TEXT("ggg"), WS_CHILD | WS_VISIBLE,
                100, 100,
                200, 200,
                hWnd, NULL,
                hInst, NULL);
            botWnd = CreateWindow(TEXT("GameFieldClass"), TEXT("ggg2"), WS_CHILD | WS_VISIBLE,
                400, 100,
                200, 200,
                hWnd, NULL,
                hInst, NULL);

            GameField* playerData = new GameField(GAMEFIELD_XSIZE, GAMEFIELD_YSIZE);
            playerData->SetShip(Ship(POINT{ 0,0 }, POINT{ 1, 0 }, 2));
            playerData->SetShip(Ship(POINT{ 2,2 }, POINT{ 4, 2 }, 3));
            playerData->SetShip(Ship(POINT{ 5,6 }, POINT{ 5, 6 }, 1));
            playerData->SetShip(Ship(POINT{ 0,2 }, POINT{ 0, 5 }, 4));
            SendMessage(playerWnd, CM_CONFIGURATE, 0, (LPARAM)playerData);

            GameField* botData = new GameField(GAMEFIELD_XSIZE, GAMEFIELD_YSIZE, true);
            botData->SetShip(Ship(POINT{ 2, 3 }, POINT{ 2, 6 }, 4));
            SendMessage(botWnd, CM_CONFIGURATE, 0, (LPARAM)botData);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


BOOL Line(HDC hdc, POINT first, POINT last)
{
    MoveToEx(hdc, first.x, first.y, NULL); //сделать текущими координаты x1, y1
    return LineTo(hdc, last.x, last.y);
}
ATOM RegisterGameWndClass()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = GameFieldWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("GameFieldClass");
    return RegisterClass(&wc);
}