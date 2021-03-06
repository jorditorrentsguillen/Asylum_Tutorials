
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "uxtheme.lib")

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "animatedgif.h"
#include "customcombo.h"

// M <=> menu
#define IDM_OPEN_ITEM		1001
#define IDM_SAVE_ITEM		1002
#define IDM_SAVEAS_ITEM		1003
#define IDM_EXIT_ITEM		1004
#define IDM_ABOUT_ITEM		1005

// C <=> control
#define IDC_BUTTON1			1020
#define IDC_TIMER			1021
#define IDC_TRACKBAR		1022
#define IDC_COMBO1			1023
#define IDC_COMBO2			1024

enum IndicatorType
{
	Circle = IDC_RADIO1,
	Square = IDC_RADIO2,
	Gear = IDC_RADIO3
};

HWND			hwnd			= 0;
HWND			button1			= 0;
HWND			spinner			= 0;
HWND			trackbar		= 0;
HWND			label1			= 0;
HWND			label2			= 0;
AnimatedGIF*	animgif			= 0;
CustomCombo*	combo1			= 0;
CustomCombo*	combo2			= 0;
IndicatorType	type			= Circle;
ULONG_PTR		gdiplustoken	= 0;

INT InitControls()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplustoken, &gdiplusStartupInput, NULL);

	INITCOMMONCONTROLSEX	iccs;
	HFONT					font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	HINSTANCE				hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

	iccs.dwICC = ICC_STANDARD_CLASSES|ICC_BAR_CLASSES;
	iccs.dwSize = sizeof(INITCOMMONCONTROLSEX);
	
	InitCommonControlsEx(&iccs);

	// menu
	HMENU menu = CreateMenu();
	HMENU submenu1 = CreatePopupMenu();
	HMENU submenu2 = CreatePopupMenu();
	
	AppendMenuA(submenu1, MF_STRING, IDM_OPEN_ITEM, "&Open");
	AppendMenuA(submenu1, MF_STRING, IDM_SAVE_ITEM, "&Save");
	AppendMenuA(submenu1, MF_STRING, IDM_SAVEAS_ITEM, "Save &As");
	AppendMenuA(submenu1, MF_SEPARATOR, 0, 0);
	AppendMenuA(submenu1, MF_STRING, IDM_EXIT_ITEM, "&Exit");
	AppendMenuA(submenu2, MF_STRING, IDM_ABOUT_ITEM, "&About");

	AppendMenuA(menu, MF_STRING|MF_POPUP, (UINT)submenu1, "&File");
	AppendMenuA(menu, MF_STRING|MF_POPUP, (UINT)submenu2, "&Help");
	
	SetMenu(hwnd, menu);

	// button
	int starty = 30;

	button1 = CreateWindow(
		"BUTTON", "Click Me", WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
		300, starty, 120, 40, hwnd, (HMENU)IDC_BUTTON1, hinst, 0);

	if( !button1 )
		return 0;

	SendMessage(button1, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

	// spinner
	spinner = CreateWindow(
		"STATIC", "", WS_VISIBLE|WS_CHILD|SS_OWNERDRAW,
		30, starty, 32, 32, hwnd, (HMENU)IDI_CIRCLE_ICON, hinst, 0);

	if( !spinner )
		return 0;

	animgif = new AnimatedGIF(spinner);
	animgif->SetResImage(MAKEINTRESOURCEW(IDI_CIRCLE_ICON));

	// trackbar with labels
	trackbar = CreateWindowEx(
		0, TRACKBAR_CLASS, "trackbar1", WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS,
		80, starty + 2, 200, 30, hwnd, (HMENU)IDC_TRACKBAR, hinst, NULL);

	if( !trackbar )
		return 0;

	SendMessage(trackbar, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1, 5));
	SendMessage(trackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)2);
	SendMessage(trackbar, TBM_SETTICFREQ, 1, 0);
	SendMessage(trackbar, TBM_SETPAGESIZE, 0, (LPARAM)1);
	SendMessage(trackbar, TBM_SETLINESIZE, 0, (LPARAM)1);

	label1 = CreateWindow(
		"STATIC", "Slow", SS_LEFT|WS_VISIBLE|WS_CHILD,
		85, starty + 35, 50, 20, hwnd, 0, hinst, NULL);

	label2 = CreateWindow(
		"STATIC", "Fast", SS_RIGHT|WS_VISIBLE|WS_CHILD,
		220, starty + 35, 50, 20, hwnd, 0, hinst, NULL);

	SendMessage(label1, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	SendMessage(label2, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

	// combobox
	RECT rc1 = { 6, 110, 6 + 160, 120 + 14 };
	RECT rc2 = { 270, 110, 270 + 160, 120 + 14 };

	combo1 = new CustomCombo();
	combo2 = new CustomCombo();

	combo1->Initialize(IDC_COMBO1, hwnd, rc1);
	combo2->Initialize(IDC_COMBO2, hwnd, rc2);

	combo1->AddString("Furunkulusz");
	combo1->AddString("@Kaktusz b�nusz");
	combo1->AddString("@Trolibusz");
	combo1->AddString("T�kfilkusz");
	combo1->SetCurSel(0);

	combo2->AddString("@Tutihogyix");
	combo2->AddString("Csodaturmix");
	combo2->AddString("Hangjanix");
	combo2->AddString("@Obelix");
	combo2->AddString("Sokadikix");
	combo2->SetCurSel(2);
	
	// timer
	SetTimer(hwnd, IDC_TIMER, 80, 0);
	return 1;
}

INT_PTR WINAPI DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_ACTIVATE:
		SendMessage(GetDlgItem(hWnd, type), BM_SETCHECK, BST_CHECKED, 0);
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 1);
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			if( BST_CHECKED == SendMessage(GetDlgItem(hWnd, IDC_RADIO1), BM_GETCHECK, 0, 0) )
			{
				type = Circle;
				animgif->SetResImage(MAKEINTRESOURCEW(IDI_CIRCLE_ICON));
			}
			else if( BST_CHECKED == SendMessage(GetDlgItem(hWnd, IDC_RADIO2), BM_GETCHECK, 0, 0) )
			{
				type = Square;
				animgif->SetResImage(MAKEINTRESOURCEW(IDI_SQUARE_ICON));
			}
			else
			{
				type = Gear;
				animgif->SetResImage(MAKEINTRESOURCEW(IDI_GEAR_ICON));
			}

			SendMessage(hWnd, WM_CLOSE, 0, 0);

			return TRUE;

		case IDCANCEL:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return TRUE;
		}

	default:
		break;
	}

	return FALSE;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if( combo1 && (HWND)lParam == combo1->GetHandle() )
		combo1->ProcessCommands(wParam, lParam);
	else if( combo2 && (HWND)lParam == combo2->GetHandle() )
		combo2->ProcessCommands(wParam, lParam);

	switch( msg )
	{
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_VSCROLL:
	case WM_HSCROLL:
		if( (HWND)lParam == trackbar )
		{
			switch( LOWORD(wParam) )
			{
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN: {
				// page up/down or arrows were pressed
				unsigned short pos =
					(unsigned short)SendMessageA(trackbar, TBM_GETPOS, 0, 0);

				SetTimer(hWnd, IDC_TIMER, (6 - pos) * 20, 0);
				} break;

			case TB_THUMBTRACK: {
				// the thumb was dragged
				unsigned short pos = HIWORD(wParam);
				SetTimer(hWnd, IDC_TIMER, (6 - pos) * 20, 0);
				} break;
			}
		}

		break;

	case WM_COMMAND:
		// standard controls
		switch( LOWORD(wParam) )
		{
		case IDM_EXIT_ITEM:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_ABOUT_ITEM:
			MessageBoxA(hwnd, "WinAPI tutorial 2", "About", MB_OK);
			break;

		case IDC_BUTTON1:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, &DialogProc);
			break;
		}

		break;

	case WM_TIMER:
		if( animgif )
			animgif->NextFrame();

		InvalidateRect(spinner, 0, TRUE);
		break;

	case WM_DRAWITEM: {
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;

		if( pDIS->hwndItem == spinner && animgif )
			animgif->Draw();
		else if( pDIS->hwndItem == combo1->GetHandle() )
			combo1->Render(pDIS);
		else if( pDIS->hwndItem == combo2->GetHandle() )
			combo2->Render(pDIS);

		} break;

	case WM_KEYUP:
		switch(wParam)
		{
		case VK_ESCAPE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		(WNDPROC)WndProc,
		0L, 0L,
		hInst,
		NULL,
		NULL,
		CreateSolidBrush(RGB(240, 240, 240)),
		NULL,
		"TestClass",
		NULL
	};

	RegisterClassEx(&wc);
	
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);

	DWORD style = WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_SYSMENU|WS_BORDER|WS_CAPTION;

	hwnd = CreateWindowA("TestClass", "Winapi2", style,
		(w - 450) / 2, (h - 200) / 2, 450, 200,
		NULL, NULL, wc.hInstance, NULL);
	
	if( !hwnd )
	{
		MessageBoxA(NULL, "Could not create window!", "Error!", MB_OK);
		goto _end;
	}

	if( !InitControls() )
	{
		MessageBoxA(NULL, "Could not init controls!", "Error!", MB_OK);
		goto _end;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while( msg.message != WM_QUIT )
	{
		while( PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

_end:
	if( combo1 )
		delete combo1;

	if( combo2 )
		delete combo2;

	if( animgif )
		animgif->Dispose();

	if( gdiplustoken )
		Gdiplus::GdiplusShutdown(gdiplustoken);

	UnregisterClass("TestClass", wc.hInstance);
	return 0;
}
