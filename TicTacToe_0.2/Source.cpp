#include <Windows.h>
#include <Windowsx.h>
#include <sstream>
#include <commdlg.h>
#include "resource.h"
#include <ctime>
#include "Game.h"
using std::wostringstream;

#include <tchar.h>

/*#include <dsound.h>
#pragma comment(lib, "Dsound3d.dll")*/

#include <d2d1.h> // Include the Direct2D API.
#pragma comment(lib, "d2d1.lib") // Connect to the Direct2D Import Library.

#include <dwrite.h> // Include the DirectWrite API.
#pragma comment(lib, "dwrite.lib") // Connect to the DirectWrite Import Library.
//#include <iostream>

enum Colors {Background = 0, Font = 1, Grid = 2, Xmark = 3, Omark = 4, Gradient = 5};

LRESULT _stdcall WindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd);
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
HRESULT CreateGraphics(HWND hWnd);
LRESULT WinnerCheck(HWND hWnd);
LRESULT Reset(HWND hWnd);
LRESULT KeyCommands(HWND hWnd, WPARAM wParam);
LRESULT GradientChange();
void DestroyGraphics();
void SetColors();

//Safe Release
template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

POINT ptMouse;
Game game;
char mode;
char cheats = 0;

ID2D1Factory* pD2DFactory = NULL;
ID2D1HwndRenderTarget* pRT = NULL;
ID2D1SolidColorBrush *pBrush = NULL;
IDWriteTextFormat *pTF = NULL;
IDWriteFactory* pDWFactory = NULL;
ID2D1RadialGradientBrush *m_pRadialGradientBrush;
ID2D1GradientStopCollection *pGradientStops = NULL;
D2D1_POINT_2F dptMouse = {};
D2D1_COLOR_F col[6] = {D2D1::ColorF(D2D1::ColorF::Aquamarine), D2D1::ColorF(D2D1::ColorF::Black), D2D1::ColorF(D2D1::ColorF::Green),
	D2D1::ColorF(D2D1::ColorF::White), D2D1::ColorF(D2D1::ColorF::White), D2D1::ColorF(D2D1::ColorF::AntiqueWhite)};
HCURSOR cur[2] = {};
D2D1_POINT_2F focus[2] = {};
D2D1_GRADIENT_STOP gradientStops[2];
bool focusVis = false;
char WinLine = 0;
float width = 3.5f;

// Array of COLORREFs used to store the custom colors.
COLORREF clrarrCustom[16] =
{
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
};





int _stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	srand((unsigned int)time(0));
	ptMouse.x = 100; ptMouse.y = 100;
	focus[0].x = focus[0].y = 300.0f;
	focus[1].y = focus[1].x = 500.0f;
	mode = 0;
	//Get Rid of warnings on unrefernced parameters
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	//Step 1: Register the window class(struct)
	WNDCLASS wc = {};
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"Games!";
	wc.hCursor = LoadCursorW(hInstance, IDC_HAND);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
	RegisterClass(&wc);

	//Step 2: Create our Window
	HWND hWnd = CreateWindow(wc.lpszClassName, L"Games!!", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 300, 100, 800, 800, HWND_DESKTOP, NULL, wc.hInstance, NULL); 

	//Step 3: Show/Update Window
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	cur[0] = LoadCursorW(hInstance, MAKEINTRESOURCEW(IDC_CURSOR1));
	cur[1] =  LoadCursorW(hInstance, MAKEINTRESOURCEW(IDC_CURSOR2));


	// Accelerator Table
	HACCEL hAccel = LoadAcceleratorsW(hInstance,
		MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	//Step 4: Message Loop
	MSG msg = {};
	BOOL bRet;
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateAcceleratorW(hWnd, hAccel, &msg);
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}

	CoUninitialize();
	return msg.wParam;
}


LRESULT _stdcall WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch(Msg)
	{
	case WM_CLOSE:
		{
			//Last-Chance saving here
			int nMBResult = MessageBoxW(hWnd, L"Are you sure you want to quit?", L"BAD DECISION!", MB_YESNO | MB_ICONSTOP);	
			if(nMBResult == IDYES)
				DestroyWindow(hWnd);
			break;
		}

	case WM_CREATE:
		if(FAILED(CreateGraphics(hWnd)))
		{
			return -1;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(-3);
		break;
		//WM_QUIT

	case WM_PAINT:
		return OnPaint(hWnd);
		break;

	case WM_KEYDOWN:
		if(wParam == VK_BACK && mode > 10)
		{
			mode = 4;
			InvalidateRect(hWnd, NULL, true);
		}
		else if(wParam == VK_BACK && mode > 2)
		{
			mode = 0;
			SetColors();
			InvalidateRect(hWnd, NULL, true);
		}
		else
		{
			KeyCommands(hWnd, wParam);
		}
		break;
	case WM_LBUTTONDOWN:
		return OnLButtonDown(hWnd, wParam, lParam);
	case WM_COMMAND:
		return OnCommand(hWnd, wParam, lParam);
		break;

	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

LRESULT OnPaint(HWND hWnd)
{

	pRT->BeginDraw();
	pRT->Clear(col[Background]);

	D2D1_RECT_F back = {};
	back.top = back.left = 0;
	back.bottom = back.right = 800;

	pRT->FillRectangle(back, m_pRadialGradientBrush);
	pRT->DrawRectangle(back, pBrush, 1, NULL);
#pragma region Menu
	if(mode == 0)
	{

		D2D1_RECT_F MenuBox = {};
		MenuBox.left = 200; MenuBox.right = 600;
		MenuBox.top = 100; MenuBox.bottom = 700;
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(MenuBox, pBrush, width);
		D2D1_RECT_F title = {};
		title.left = 300; title.right = 500;
		wostringstream oss[6];
		oss[0] << "     Games!!!";
		oss[1] << "   TicTacToe";
		oss[2] << " ";
		oss[3] << " ";
		oss[4] << "   Options";
		oss[5] << "    Exit";
		for(int x = 0; x < 6; x++)
		{
			if (x <= 1 || x >= 4)
			{
				pBrush->SetColor(col[Font]);
				title.top = 25.0f + x*100.0f; title.bottom = 75.0f + x*100.0f;
				pRT->DrawTextW(oss[x].str().c_str(), oss[x].str().length(), pTF, &title, pBrush);
				pBrush->SetColor(col[Grid]);
				pRT->DrawRectangle(title, pBrush, 4.5f); 
			}
		}
	}
#pragma endregion


#pragma region TicTacToe
	if (mode == 1)
	{
		SetCursor(cur[!game.WhoseTurn()]);

		D2D1_RECT_F scoreBox = {};
		scoreBox.left = 175; scoreBox.right = 650;
		scoreBox.bottom = 50; scoreBox.top = 75;
		wostringstream oss;
		pBrush->SetColor(col[Font]);
		oss << "Score:  X's - " << game.GetScore()[0] << "  O's - " << game.GetScore()[1] << "  Ties - " << game.GetScore()[2];
		pRT->DrawTextW(oss.str().c_str(), oss.str().length(), pTF, &scoreBox, pBrush);

		D2D1_RECT_F rect1;
		rect1.top = rect1.left = 100;
		rect1.bottom = rect1.right = 700;
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(rect1, pBrush, width);

		D2D1_POINT_2F pt1 = {300, 100}, pt2 = {300, 700};
		pRT->DrawLine(pt1, pt2, pBrush, width);

		pt1 = D2D1::Point2F(500.0f, 100.0f);
		pt2 = D2D1::Point2F(500.0f, 700.0f);
		pRT->DrawLine(pt1, pt2, pBrush, width);

		pt1 = D2D1::Point2F(100.0f, 300.0f);
		pt2 = D2D1::Point2F(700.0f, 300.0f);
		pRT->DrawLine(pt1, pt2, pBrush, width);

		pt1 = D2D1::Point2F(100.0f, 500.0f);
		pt2 = D2D1::Point2F(700.0f, 500.0f);
		pRT->DrawLine(pt1, pt2, pBrush, width);

		if(focusVis)
		{
			D2D1_RECT_F focusRect;
			focusRect.top = focus[0].y+10; focusRect.left = focus[0].x+10;
			focusRect.bottom = focus[1].y-10; focusRect.right = focus[1].x-10;
			pBrush->SetColor(col[(!game.WhoseTurn())+3]);
			pRT->DrawRectangle(focusRect, pBrush, width-1.0f);
		}

		float* move = game.GetMove();
		short increm = 0;

		/*if(WinLine != 0)
		{

		}*/

		for(int x = 0; x < 3; x++)
			for(int y = 0; y < 3; y++)
			{
				move[1] = 100.0f + x*200.0f;
				move[2] = 100.0f + y*200.0f;
				move[3] = 300.0f + x*200.0f;
				move[4] = 300.0f + y*200.0f;
				if(game.GetGrid()[increm] == 1)
				{
					pBrush->SetColor(col[Xmark]);
					pt1 = D2D1::Point2F(move[1], move[2]);
					pt2 = D2D1::Point2F(move[3], move[4]);
					pRT->DrawLine(pt1, pt2, pBrush, width);

					pt1 = D2D1::Point2F(move[1]+200, move[2]);
					pt2 = D2D1::Point2F(move[3]-200, move[4]);
					pRT->DrawLine(pt2, pt1, pBrush, width);
				}
				else if(game.GetGrid()[increm] == 0)
				{
					pBrush->SetColor(col[Omark]);
					pt1 = D2D1::Point2F(move[1]+100, move[4]-100);
					pRT->DrawEllipse(D2D1::Ellipse(pt1, 100.0, 100.0), pBrush, width);

				}
				increm++;
			} 
	}  
#pragma endregion

#pragma region HangMan
	if(mode == 2)
	{
		pRT->Clear(D2D1::ColorF(D2D1::ColorF::Chocolate));
		D2D1_RECT_F Grid;
		Grid.left = 200.0f; Grid.top = 500.0f;
		Grid.right = 250.0f; Grid.bottom = 550.0f;
		wostringstream letter;
		char let = L'A';

		for(int x = 0; x < 26; x++)
		{
			letter << let++;
			pRT->DrawTextW(letter.str().c_str(), 1, pTF, Grid, pBrush);
			if(x == 8)
			{
				Grid.left = 200.0f; Grid.top = 550.0f;
				Grid.right = 250.0f; Grid.bottom = 600.0f;
			}
			else if(x == 16)
			{
				Grid.left = 200.0f; Grid.top = 600.0f;
				Grid.right = 250.0f; Grid.bottom = 650.0f;
			}
			else
				Grid.left += 50; Grid.right += 50;
			letter.str(L"");
		}
	}
#pragma endregion

#pragma region Options
	if(mode == 4)
	{

		wostringstream oss[5] = {};
		oss[0] << "Options";
		oss[1] << "Instructions";
		//oss[2] << "Cheats";
		oss[3] << "Credits";
		oss[4] << "Back";

		D2D1_RECT_F MenuBox = {};
		MenuBox.left = 200; MenuBox.right = 600;
		MenuBox.top = 100; MenuBox.bottom = 700;
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(MenuBox, pBrush, width);

		D2D1_RECT_F title = {};
		title.left = 300; title.right = 500;
		for(int x = 0; x < 5; x++)
		{
			if(x == 2)
				continue;
			pBrush->SetColor(col[Font]);
			title.top = 25.0f + x*100.0f; title.bottom = 75.0f + x*100.0f;
			pRT->DrawTextW(oss[x].str().c_str(), oss[x].str().length(), pTF, &title, pBrush);
			pBrush->SetColor(col[Grid]);
			pRT->DrawRectangle(title, pBrush, 4.5f);
		}
	}
#pragma endregion


#pragma region Instructions
	if(mode == 11)
	{
		wostringstream TTT, Hang;
		D2D1_RECT_F ttt;
		TTT << L"TicTacToe";
		ttt.left = 25.0f; ttt.top = 50.f;
		ttt.right = 200.0f; ttt.bottom = 100.0f;
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pRT->DrawRectangle(ttt, pBrush, width);

		TTT.str(L"");
		TTT << L"\n\tUse the mouse numpad or arrow keys to\n"
			<< L"\tplace an X or an O on the grid.\n"
			<< L"\tGet 3 marks in a row while\n"
			<< L"\tblocking your opponent to win." 
			<< L"\n\t(Secret cheat: Press Ctrl+F to swap marks,"
			<< L"\n\tCtrl+R to Override marks,\n"
			<< L"\tand Ctrl+0 to remove opponent marks)" << 
			L"\n\n\t\tPress Backspace to go back...";
		ttt.left = 100.0f; ttt.top = 100.f;
		ttt.right = 750.0f; ttt.bottom = 700.0f;
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pRT->DrawRectangle(ttt, pBrush, width);
	}
#pragma endregion

#pragma region Credits
	if(mode == 13)
	{
		wostringstream TTT, Hang;
		D2D1_RECT_F ttt;
		TTT << L"  Credits";
		ttt.left = 125.0f; ttt.top = 250.f;
		ttt.right = 300.0f; ttt.bottom = 300.0f;
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pRT->DrawRectangle(ttt, pBrush, width);

		TTT.str(L"");
		TTT << L"\tDeShon D. Thomas-Wright" <<
			L"\n\t\tDTW3@fullsail.edu" << 
			L"\n\n\n  Press Backspace to go back...";
		ttt.left = 200.0f; ttt.top = 300.f;
		ttt.right = 750.0f; ttt.bottom = 500.0f;
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pRT->DrawRectangle(ttt, pBrush, width);
	}
#pragma endregion

#pragma region Winner
	if(mode == 99)
	{
		wostringstream TTT, Hang;
		D2D1_RECT_F ttt;
		TTT << L"  Winner!";
		ttt.left = 300.0f; ttt.top = 250.f;
		ttt.right = 500.0f; ttt.bottom = 300.0f;
		pBrush->SetColor(col[Font]);
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(ttt, pBrush, width);

		if (game.WinnerCheck(WinLine, false) > 0)
		{
			TTT.str(L"");
			TTT << L"\tPlayer " << game.WhoseTurn()+1 << " wins\nWould you like to play again?";
			ttt.left = 200.0f; ttt.top = 300.f;
			ttt.right = 750.0f; ttt.bottom = 500.0f;
			pBrush->SetColor(col[Font]);
			pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
			pBrush->SetColor(col[Grid]);
			pRT->DrawRectangle(ttt, pBrush, width); 
		}
		else
		{
			TTT.str(L"");
			TTT << L"\tIt's a tie...\nWould you like to play again?";
			ttt.left = 200.0f; ttt.top = 300.f;
			ttt.right = 750.0f; ttt.bottom = 500.0f;
			pBrush->SetColor(col[Font]);
			pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
			pBrush->SetColor(col[Grid]);
			pRT->DrawRectangle(ttt, pBrush, width);
		}
		TTT.str(L"");
		TTT << (L"\tYes");
		ttt.left = 100.0f; ttt.top = 600.f;
		ttt.right = 350.0f; ttt.bottom = 700.0f;
		pBrush->SetColor(col[Font]);
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(ttt, pBrush, width);

		TTT.str(L"");
		TTT << (L"\tNo");
		ttt.left = 500.0f; ttt.top = 600.f;
		ttt.right = 750.0f; ttt.bottom = 700.0f;
		pBrush->SetColor(col[Font]);
		pRT->DrawTextW(TTT.str().c_str(), TTT.str().length(), pTF, &ttt, pBrush);
		pBrush->SetColor(col[Grid]);
		pRT->DrawRectangle(ttt, pBrush, width);
	}
#pragma endregion

	//EndPaint(hWnd, &ps);
	HRESULT hr = pRT->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		//TODO: Destroy Direct2D.
		//TODO: Recreate Direct2D.
	}

	ValidateRect(hWnd, NULL);

	return 0;
}

LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	ptMouse.x = GET_X_LPARAM(lParam);
	ptMouse.y = GET_Y_LPARAM(lParam);

#pragma region MenuLM
	if(mode == 0)
	{
		if(ptMouse.x >=300 && ptMouse.x < 500
			&& ptMouse.y > 125 && ptMouse.y < 575)
		{
			if(ptMouse.y > 125 && ptMouse.y < 175)
			{
				//TicTacToe
				mode = 1;
				EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_X, MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_O, MF_ENABLED);
				SetColors();
				game.AI(false);
				game.Reset(true);
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 225 && ptMouse.y < 275)
			{
				//HangMan
				mode = 2;
				SetColors();
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 325 && ptMouse.y < 375)
			{
				// 
			}
			else if(ptMouse.y > 425 && ptMouse.y < 475)
			{
				//Options
				mode = 4;
				SetColors();
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 525 && ptMouse.y < 575)
			{
				//Exit
				DestroyWindow(hWnd);
			}
		}
	}
#pragma endregion

#pragma region TicTacToeLM
	if (mode == 1)
	{

		short gridPosX = -1, gridPosY = -1;

		if(ptMouse.x >=100 && ptMouse.x < 700
			&& ptMouse.y > 100 && ptMouse.y < 700)
		{

			if(ptMouse.x > 100 && ptMouse.x < 300)
				gridPosX = 0;
			else if(ptMouse.x > 300 && ptMouse.x < 500)
				gridPosX = 1;
			else
				gridPosX = 2;

			if(ptMouse.y > 100 && ptMouse.y < 300)
				gridPosY = 0;
			else if(ptMouse.y > 300 && ptMouse.y < 500)
				gridPosY = 1;
			else
				gridPosY = 2;
		}

		if(game.CheckGrid(gridPosX, gridPosY))
		{
			game.DrawMark();
			InvalidateRect(hWnd, NULL, false);
			if(WinnerCheck(hWnd) == 0)
			{
				if(game.AITurn())
					SetCursor(cur[0]);
				InvalidateRect(hWnd, NULL, false);
				WinnerCheck(hWnd);
			}
		} 
	}  
#pragma endregion

#pragma region OptionLM
	if(mode == 4)
	{
		if(ptMouse.x >=300 && ptMouse.x < 500
			&& ptMouse.y > 125 && ptMouse.y < 575)
		{
			if(ptMouse.y > 125 && ptMouse.y < 175)
			{
				//Instructions
				mode = 11;
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 225 && ptMouse.y < 275)
			{
				//Cheats
				mode = 12;
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 325 && ptMouse.y < 375)
			{
				//Credit 
				mode = 13;
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
			}
			else if(ptMouse.y > 425 && ptMouse.y < 475)
			{
				mode = 0;
				SetColors();
				ptMouse.x = NULL;
				ptMouse.y = NULL;
				InvalidateRect(hWnd, NULL, true);
				//Back
			}

		}
	}
#pragma endregion

#pragma region WinnerLM
	if(mode == 99)
	{
		if(ptMouse.x >=100 && ptMouse.x < 750
			&& ptMouse.y > 600 && ptMouse.y < 700)
		{
			if(ptMouse.x < 350)
			{
				//Yes
				mode = 1;
				SetCursor(cur[0]);
				Reset(hWnd);
			}
			else if(ptMouse.x > 500)
			{
				//No/Exit
				DestroyWindow(hWnd);
			}
		}
	}
#pragma endregion

	return 0;
}

HRESULT CreateGraphics(HWND hWnd)
{
	// Initialize the Direct2D Factory.
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Factory."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	// Get the dimensions of the client.
	RECT rc;
	GetClientRect(hWnd, &rc);

	// Initialize a Direct2D Size Structure.
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	// Create the Direct2D Render Target.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, size), &pRT);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Render Target."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	pRT->SetDpi(96.0f, 96.0f);

	// Create the Direct2D Solid Color Brush.
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(0x0), &pBrush);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Solid Color Brush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	// Initialize the DirectWrite Factory.
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		(IUnknown**)&pDWFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Factory."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	// Create the DirectWrite Text Format.
	hr = pDWFactory->CreateTextFormat(_T("Comic Sans MS"), NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, _T(""), &pTF);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Text Format."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	pTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	GradientChange();

	return S_OK;
};

void DestroyGraphics()
{

	SafeRelease(&pGradientStops);

	SafeRelease(&m_pRadialGradientBrush);

	SafeRelease(&pTF);

	SafeRelease(&pDWFactory);

	// Release the Direct2D Solid Color Brush.
	SafeRelease(&pBrush);


	// Release the Direct2D Render Target.
	SafeRelease(&pRT);

	// Release the Direct2D Factory.
	if (pD2DFactory)
	{
		pD2DFactory->Release();
		pD2DFactory = NULL;
	}
}

LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case ID_FILE_MENU:
		mode = 0;
		EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_X, MF_DISABLED);
		EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_O, MF_DISABLED);
		SetColors();
		game.Reset(true);
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_FILE_EXIT:
		DestroyWindow(hWnd);
		break;
	case ID_NEW_PVP:
		mode = 1;
		EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_X, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_O, MF_ENABLED);
		SetColors();
		game.AI(false);
		game.Reset(true);
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_NEW_PVAI:
		mode = 1;
		EnableMenuItem(GetMenu(hWnd), ID_COLORS_TICTACTOE, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), ID_TICTACTOE_O, MF_ENABLED);
		SetColors();
		game.AI(true);
		game.Reset(true);
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_LINESIZE_SMALL:
		width = 1.5f;
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_LINESIZE_MEDIUM:
		width = 3.5f;
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_LINESIZE_LARGE:
		width = 5.5f;
		InvalidateRect(hWnd, NULL, true);
		break;
	case ID_FLIP:
		if (mode == 1)
		{
			game.Cheater(0);
			cheats |= 8;
			InvalidateRect(hWnd, NULL, true); 
		}
		break;
	case ID_ULT:
		if (mode == 1)
		{
			game.Cheater(1);
			cheats |= 16;
			InvalidateRect(hWnd, NULL, true); 
		}
		break;
	case ID_FORGET:
		if (mode == 1)
		{
			game.Cheater(2);
			cheats |= 32;
			InvalidateRect(hWnd, NULL, true); 
		}
		break;
	case ID_COLORS_BACKGROUND:
		{
			CHOOSECOLOR cc;
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.Flags = CC_RGBINIT;
			cc.lpCustColors = clrarrCustom;

			// Invoke the Color Selection dialog.
			if(ChooseColorW(&cc))
			{
				// NOTE: The selected color is stored in the cc.rgbResult variable.
				//col = D2D1::ColorF(cc.rgbResult);
				col[Background] = D2D1::ColorF(GetRValue(cc.rgbResult)/255.0f,
					GetGValue(cc.rgbResult)/255.0f,
					GetBValue(cc.rgbResult)/255.0f);
				// Repaint the screen.
				GradientChange();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case ID_GRIDLINES_COLOR:
		{
			CHOOSECOLOR cc;
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.Flags = CC_RGBINIT;
			cc.lpCustColors = clrarrCustom;

			// Invoke the Color Selection dialog.
			if(ChooseColorW(&cc))
			{
				// NOTE: The selected color is stored in the cc.rgbResult variable.
				//col = D2D1::ColorF(cc.rgbResult);
				col[Grid] = D2D1::ColorF(GetRValue(cc.rgbResult)/255.0f,
					GetGValue(cc.rgbResult)/255.0f,
					GetBValue(cc.rgbResult)/255.0f);
				// Repaint the screen.
				GradientChange();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case ID_COLORS_ALLRANDOM:
		{
			for(int x = 0; x < 6; x++)
			{
				float r = rand()%256/255.0f, g = rand()%256/255.0f, b = rand()%256/255.0f;
				col[x] = D2D1::ColorF(r, g, b);
			}
			// Repaint the screen.
			GradientChange();
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case ID_COLORS_FONT:
		{
			CHOOSECOLOR cc;
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.Flags = CC_RGBINIT;
			cc.lpCustColors = clrarrCustom;

			// Invoke the Color Selection dialog.
			if(ChooseColorW(&cc))
			{
				// NOTE: The selected color is stored in the cc.rgbResult variable.
				//col = D2D1::ColorF(cc.rgbResult);
				col[Font] = D2D1::ColorF(GetRValue(cc.rgbResult)/255.0f,
					GetGValue(cc.rgbResult)/255.0f,
					GetBValue(cc.rgbResult)/255.0f);
				// Repaint the screen.
				GradientChange();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case ID_TICTACTOE_X:
		{
			CHOOSECOLOR cc;
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.Flags = CC_RGBINIT;
			cc.lpCustColors = clrarrCustom;

			// Invoke the Color Selection dialog.
			if(ChooseColorW(&cc))
			{
				// NOTE: The selected color is stored in the cc.rgbResult variable.
				//col = D2D1::ColorF(cc.rgbResult);
				col[Xmark] = D2D1::ColorF(GetRValue(cc.rgbResult)/255.0f,
					GetGValue(cc.rgbResult)/255.0f,
					GetBValue(cc.rgbResult)/255.0f);
				// Repaint the screen.
				GradientChange();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case ID_TICTACTOE_O:
		{
			CHOOSECOLOR cc;
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.Flags = CC_RGBINIT;
			cc.lpCustColors = clrarrCustom;

			// Invoke the Color Selection dialog.
			if(ChooseColorW(&cc))
			{
				// NOTE: The selected color is stored in the cc.rgbResult variable.
				//col = D2D1::ColorF(cc.rgbResult);
				col[Omark] = D2D1::ColorF(GetRValue(cc.rgbResult)/255.0f,
					GetGValue(cc.rgbResult)/255.0f,
					GetBValue(cc.rgbResult)/255.0f);
				// Repaint the screen.
				GradientChange();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	default:
		MessageBox(HWND_DESKTOP, std::to_wstring(LOWORD(wParam)).c_str(), L"Menu, not handled", MB_OK);
		break;
	}
	return 0;
}

LRESULT WinnerCheck(HWND hWnd)
{
	//wostringstream oss;

	if(game.WinnerCheck(WinLine) >= -1)
	{
		mode = 99;
		InvalidateRect(hWnd, NULL, true);
		/*int yesno = MessageBox(hWnd, oss.str().c_str(), L"Winner!", MB_YESNO);
		if(yesno == IDYES)
		{
		SetCursor(cur[0]);
		Reset(hWnd);
		}
		else
		DestroyWindow(hWnd);*/
		return 1;
	}
	return 0;
}

LRESULT Reset(HWND hWnd)
{
	game.Reset();
	WinLine = 0;
	InvalidateRect(hWnd, NULL, true);
	return 0;
}

LRESULT KeyCommands(HWND hWnd, WPARAM wParam)
{
	if (mode == 1)
	{
		short gridPosX = -1, gridPosY = -1;
		SetCursor(cur[game.WhoseTurn()]);

		switch(wParam)
		{
		case VK_NUMPAD1:
			gridPosX = 0; gridPosY = 2;
			break;
		case VK_NUMPAD2:
			gridPosX = 1; gridPosY = 2;
			break;
		case VK_NUMPAD3:
			gridPosX = 2; gridPosY = 2;
			break;
		case VK_NUMPAD4:
			gridPosX = 0; gridPosY = 1;
			break;
		case VK_NUMPAD5:
			gridPosX = 1; gridPosY = 1;
			break;
		case VK_NUMPAD6:
			gridPosX = 2; gridPosY = 1;
			break;
		case VK_NUMPAD7:
			gridPosX = 0; gridPosY = 0;
			break;
		case VK_NUMPAD8:
			gridPosX = 1; gridPosY = 0;
			break;
		case VK_NUMPAD9:
			gridPosX = 2; gridPosY = 0;
			break;

		case VK_LEFT:
			if(!focusVis)
				focusVis = !focusVis;
			else
			{
				if(focus[1].x-200.0f <= 100.0f)
				{
					focus[1].x = 700.0f;
					focus[0].x = 500.0f;
				}
				else
				{
					focus[1].x -= 200.0f;
					focus[0].x -= 200.0f;
				}
			}
			InvalidateRect(hWnd, NULL, true);
			break;
		case VK_RIGHT:
			if(!focusVis)
				focusVis = !focusVis;
			else
			{
				if(focus[1].x+200.0f > 700.0f)
				{
					focus[1].x = 300.0f;
					focus[0].x = 100.0f;
				}
				else
				{
					focus[1].x += 200.0f;
					focus[0].x += 200.0f;
				}
			}
			InvalidateRect(hWnd, NULL, true);
			break;
		case VK_UP:
			if(!focusVis)
				focusVis = !focusVis;
			else
			{
				if(focus[1].y-200.0f <= 100.0f)
				{
					focus[1].y = 700.0f;
					focus[0].y = 500.0f;
				}
				else
				{
					focus[1].y -= 200.0f;
					focus[0].y -= 200.0f;
				}
			}
			InvalidateRect(hWnd, NULL, true);
			break;
		case VK_DOWN:
			if(!focusVis)
				focusVis = !focusVis;
			else
			{
				if(focus[1].y+200.0f > 700.0f)
				{
					focus[1].y = 300.0f;
					focus[0].y = 100.0f;
				}
				else
				{
					focus[1].y += 200.0f;
					focus[0].y += 200.0f;
				}
			}
			InvalidateRect(hWnd, NULL, true);
			break;
		case VK_RETURN:
			float tempX = 0, tempY = 0;
			tempX = (focus[0].x - 100.0f)*0.005f;
			tempY = (focus[0].y - 100.0f)*0.005f;
			if(game.CheckGrid((short)tempX, (short)tempY))
			{
				game.DrawMark();
				InvalidateRect(hWnd, NULL, false);
				if(WinnerCheck(hWnd) == 0)
				{
					if(game.AITurn())
						SetCursor(cur[0]);
					InvalidateRect(hWnd, NULL, false);
					WinnerCheck(hWnd);
				}
			} 
			break;
		}

		if(game.CheckGrid(gridPosX, gridPosY))
		{
			game.DrawMark();
			InvalidateRect(hWnd, NULL, false);
			if(WinnerCheck(hWnd) == 0)
			{
				if(game.AITurn())
					SetCursor(cur[0]);
				InvalidateRect(hWnd, NULL, false);
				WinnerCheck(hWnd);
			}
		} 
	} 
	else if(mode == 99)
		if(wParam == VK_RETURN)
		{
			mode = 1;
			Reset(hWnd);
		}

		return 0;
}

LRESULT GradientChange()
{
	HRESULT hr;
	SafeRelease(&pGradientStops);
	SafeRelease(&m_pRadialGradientBrush);

	gradientStops[0].color = col[Background];
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = col[Gradient];
	gradientStops[1].position = 1.0f;
	// Create the ID2D1GradientStopCollection from a previously
	// declared array of D2D1_GRADIENT_STOP structs.
	hr = pRT->CreateGradientStopCollection(gradientStops, 2, D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP, &pGradientStops);

	// The center of the gradient is in the center of the box.
	// The gradient origin offset was set to zero(0, 0) or center in this case.
	if (SUCCEEDED(hr))
	{
		hr = pRT->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(
			D2D1::Point2F(400, 400), D2D1::Point2F(0, 0), 450, 450),
			pGradientStops, &m_pRadialGradientBrush);
	}
	return 0;
}

void SetColors()
{
	switch(mode)
	{

	case 0:
		col[Background] = D2D1::ColorF(D2D1::ColorF::Aquamarine);
		col[Font] = D2D1::ColorF(D2D1::ColorF::Black);
		col[Grid] = D2D1::ColorF(D2D1::ColorF::Green);
		col[Gradient] = D2D1::ColorF(D2D1::ColorF::AntiqueWhite);
		GradientChange();
		SetCursor(HCURSOR(IDC_HAND));
		break;
	case 1:
		col[Background] = D2D1::ColorF(D2D1::ColorF::Crimson);
		col[Font] = D2D1::ColorF(D2D1::ColorF::Azure);
		col[Grid] = D2D1::ColorF(D2D1::ColorF::DarkGoldenrod);
		col[Xmark] = D2D1::ColorF(D2D1::ColorF::Gray);
		col[Omark] = D2D1::ColorF(D2D1::ColorF::Black);
		col[Gradient] = D2D1::ColorF(D2D1::ColorF::Cornsilk);
		GradientChange();
		SetCursor(cur[0]);
		break;
	case 4:
		col[Background] = D2D1::ColorF(D2D1::ColorF::DarkMagenta);
		col[Font] = D2D1::ColorF(D2D1::ColorF::AntiqueWhite);
		col[Grid] = D2D1::ColorF(D2D1::ColorF::Black);
		col[Gradient] = D2D1::ColorF(D2D1::ColorF::Cyan);
		GradientChange();
		break;
	}
}