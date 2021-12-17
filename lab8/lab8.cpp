// lab8.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "lab8.h"
#include <iostream>
#include <windowsx.h>

#define GAMEFIELD_YSIZE 10
#define GAMEFIELD_XSIZE 10

// Глобальные переменные:
HINSTANCE  hInst;                                // текущий экземпляр
GameField* playerData;
GameField* botData;
HWND       playerWnd;
HWND       botWnd;
bool rulesStatements[11];

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM); 
BOOL                Line(HDC hdc, POINT first, POINT last);
ATOM                RegisterGameWndClass();
void                EmitEvent(Event eventNum, int x = 0, int y = 0);

void checkTile(int x, int y);
void checkRandomTile();
void checkShipHit(int x, int y);
void checkCurrentPlayerShips();

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    MyRegisterClass(hInstance);

    memset(rulesStatements, false, 11 * sizeof(bool));
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
            //далее отрисовываем подстреленные бипки
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
                    if (data->playerCanInteract && false)
                    {
                        if (a.wasShooted)
                        {
                            Rectangle(hdc, a.x * offsetX, a.y * offsetY, (a.x + 1) * offsetX, (a.y + 1) * offsetY);
                            Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), a.y * offsetY + (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) });
                            Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), a.y * offsetY + (offsetY / 6) });
                        }
                    }
                    else
                    {
                        Rectangle(hdc, a.x * offsetX, a.y * offsetY, (a.x + 1) * offsetX, (a.y + 1) * offsetY);
                        if (a.wasShooted)
                        {
                            //MessageBox(0, 0, 0, 0);
                            Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), a.y * offsetY + (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) });
                            Line(hdc, POINT{ a.x * offsetX + (offsetX / 6), (a.y + 1) * offsetY - (offsetY / 6) }, POINT{ (a.x + 1) * offsetX - (offsetX / 6), a.y * offsetY + (offsetY / 6) });
                        }
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
            HWND parent = GetParent(hWnd);
            LPARAM coords = 0;
            coords = SET_X_LPARAM(x, coords);
            coords = SET_Y_LPARAM(y, coords);
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
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case CM_SHOOT:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            HWND messageSender = (HWND)wParam;
            EmitEvent(TILE_CLICK, x, y);
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
                300, 300,
                hWnd, NULL,
                hInst, NULL);
            botWnd = CreateWindow(TEXT("GameFieldClass"), TEXT("ggg2"), WS_CHILD | WS_VISIBLE,
                500, 100,
                300, 300,
                hWnd, NULL,
                hInst, NULL);

            playerData = new GameField(GAMEFIELD_XSIZE, GAMEFIELD_YSIZE);
            playerData->SetShip(Ship(POINT{ 0,0 }, POINT{ 1, 0 }, 2));
            playerData->SetShip(Ship(POINT{ 2,2 }, POINT{ 4, 2 }, 3));
            playerData->SetShip(Ship(POINT{ 5,6 }, POINT{ 5, 6 }, 1));
            playerData->SetShip(Ship(POINT{ 0,2 }, POINT{ 0, 5 }, 4));
            SendMessage(playerWnd, CM_CONFIGURATE, 0, (LPARAM)playerData);

            botData = new GameField(GAMEFIELD_XSIZE, GAMEFIELD_YSIZE, true);
            botData->SetShip(Ship(POINT{ 0, 0 }, POINT{ 1, 0 }, 2));
            botData->SetShip(Ship(POINT{ 3, 1 }, POINT{ 5, 1 }, 3));
            botData->SetShip(Ship(POINT{ 2, 3 }, POINT{ 2, 6 }, 4));
            SendMessage(botWnd, CM_CONFIGURATE, 0, (LPARAM)botData);
            EmitEvent(APPLICATION_START);
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

void EmitEvent(Event eventNum, int x, int y)
{
    rulesStatements[eventNum] = true;
    if (rulesStatements[APPLICATION_START])
    {
        // MessageBox(0, L"Start", 0, 0);
        rulesStatements[APPLICATION_START] = false;
        rulesStatements[PLAYER_TURN] = true;
        return;
    }
    if (rulesStatements[PLAYER_TURN] && rulesStatements[CHANGE_TURN])
    {
        // MessageBox(0, L"Change turn to comp", 0, 0);
        rulesStatements[PLAYER_TURN] = false;
        rulesStatements[CHANGE_TURN] = false;
        EmitEvent(COMPUTER_TURN);
    }
    if (rulesStatements[COMPUTER_TURN] && rulesStatements[CHANGE_TURN])
    {
        // MessageBox(0, L"Change turn to player", 0, 0);
        rulesStatements[COMPUTER_TURN] = false;
        rulesStatements[CHANGE_TURN] = false;
        rulesStatements[PLAYER_TURN] = true;
    }
    if (rulesStatements[PLAYER_TURN] && rulesStatements[TILE_CLICK])
    {
        // MessageBox(0, L"Player clickk", 0, 0);
        rulesStatements[TILE_CLICK] = false;
        checkTile(x, y);
        return;
    }
    if (rulesStatements[TILE_AVIALABLE])
    {
        // MessageBox(0, L"Chosen tile avialable", 0, 0);
        rulesStatements[TILE_AVIALABLE] = false;
        rulesStatements[SHOOT_TO_TILE] = true; 
        checkShipHit(x, y);
        return;
    }
    if (rulesStatements[SHOOT_TO_TILE] && rulesStatements[SHIP_HIT])
    {
        // MessageBox(0, L"Hit", 0, 0);
        rulesStatements[SHOOT_TO_TILE] = false;
        rulesStatements[SHIP_HIT] = false;
        checkCurrentPlayerShips();
        return;
    }
    if (rulesStatements[SHOOT_TO_TILE] && rulesStatements[SHIP_MISS])
    {
        // MessageBox(0, L"miss", 0, 0);
        rulesStatements[SHOOT_TO_TILE] = false;
        rulesStatements[SHIP_MISS] = false;
        EmitEvent(CHANGE_TURN, x, y);
        return;
    }
    if (rulesStatements[ALL_SHIPS_BROKEN] && rulesStatements[PLAYER_TURN])
    {
        rulesStatements[ALL_SHIPS_BROKEN] = false;
        rulesStatements[PLAYER_TURN] = false;
        MessageBox(0, TEXT("Игрок выиграл"), 0, 0);
        return;
    }
    if (rulesStatements[ALL_SHIPS_BROKEN] && rulesStatements[COMPUTER_TURN])
    {
        rulesStatements[ALL_SHIPS_BROKEN] = false;
        rulesStatements[COMPUTER_TURN] = false;
        MessageBox(0, TEXT("Компьютер выиграл"), 0, 0);
        return;
    }
    if (rulesStatements[COMPUTER_TURN])
    {
        // MessageBox(0, L"Comp randomly shoot", 0, 0);
        checkRandomTile();
        return;
    }
}

void checkTile(int x, int y)
{
    bool isPlayer = rulesStatements[PLAYER_TURN];
    GameField* field;
    if (isPlayer)
    {
        field = botData;
    }
    else
    {
        field = playerData;
    }
    if (field->shoot(x, y))
    {
        EmitEvent(TILE_AVIALABLE, x, y);
    }
    if (isPlayer)
    {
        RedrawWindow(botWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
    }
    else
    {
        RedrawWindow(playerWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
    }
}

void checkRandomTile()
{
    GameField* field;
    field = playerData;
    int x, y;
    do
    {
        x = rand() % GAMEFIELD_XSIZE;
        y = rand() % GAMEFIELD_YSIZE;
    } while (!field->shoot(x, y));
    RedrawWindow(playerWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME);
    EmitEvent(TILE_AVIALABLE, x, y);
}

void checkShipHit(int x, int y)
{
    GameField* field;
    if (rulesStatements[PLAYER_TURN])
    {
        field = botData;
    }
    else
    {
        field = playerData;
    }
    for (Ship& ship : field->shipsArray)
    {
        for (ShipTile& tile : ship.shipInfo)
        {
            if (tile.x == x && tile.y == y)
            {
                EmitEvent(SHIP_HIT, x, y);
                return;
            }
        }
    }
    EmitEvent(SHIP_MISS, x, y);
}

void checkCurrentPlayerShips()
{
    GameField* field;
    if (rulesStatements[PLAYER_TURN])
    {
        field = botData;
    }
    else
    {
        field = playerData;
    }
    bool atLeastOneAlive = false;
    for (Ship& ship : field->shipsArray)
    {
        if (ship.isDefeated == false)
            atLeastOneAlive = true;
    }
    if (!atLeastOneAlive)
        EmitEvent(ALL_SHIPS_BROKEN);
}