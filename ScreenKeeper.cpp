// ScreenKeeper.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <windows.h>
#include <GdiPlus.h>
#include <shellapi.h>
#pragma comment( lib, "gdiplus" )
#define WM_MYMESSAGE (WM_USER + 1)

using namespace std;

void CheckKey(int key);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void gdiscreen(wchar_t[256]);
void printScreen();

void gdiscreen(wchar_t path[256])
{
	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
 
	{
		HDC scrdc, memdc;
		HBITMAP membit;
		scrdc = ::GetDC(0);
		int Height = GetSystemMetrics(SM_CYSCREEN);
		int Width = GetSystemMetrics(SM_CXSCREEN);
		memdc = CreateCompatibleDC(scrdc);
		membit = CreateCompatibleBitmap(scrdc, Width, Height);
		HBITMAP hOldBitmap =(HBITMAP) SelectObject(memdc, membit);
		BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);
 
		Gdiplus::Bitmap bitmap(membit, NULL);
		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);
		cout << "\nSaving...";
		bitmap.Save(path, &clsid);
		cout << "Done! (" << *path << ")\n";

		SelectObject(memdc, hOldBitmap);
		DeleteObject(memdc);
		DeleteObject(membit);

		::ReleaseDC(0,scrdc);
	}
 
	GdiplusShutdown(gdiplusToken);
}

#define BTN_SCREENSHOT 1
#define BTN_EXIT 2
#define BTN_HIDE 3
#define BTN_FOLDER 4
void CreateLayout(HWND hWnd)
{
	int n = 1;
	int width = 300;
	int height = 40;
	int spacing = 10;

	CreateWindow(TEXT("button"), TEXT("Take Screenshot"),    
		WS_VISIBLE | WS_CHILD ,
		40, ((height + spacing)*n++), width, height,      
		hWnd, (HMENU) BTN_SCREENSHOT, NULL, NULL);

	CreateWindow(TEXT("button"), TEXT("Show Screenshots Folder"),    
		WS_VISIBLE | WS_CHILD ,
		40, ((height + spacing)*n++), width, height,       
		hWnd, (HMENU) BTN_FOLDER, NULL, NULL);

	CreateWindow(TEXT("button"), TEXT("Hide This Window"),    
		WS_VISIBLE | WS_CHILD ,
		40, ((height + spacing)*n++), width, height,       
		hWnd, (HMENU) BTN_HIDE, NULL, NULL);

	CreateWindow(TEXT("button"), TEXT("Exit"),    
		WS_VISIBLE | WS_CHILD ,
		40, ((height + spacing)*n++), width, height,        
		hWnd, (HMENU) BTN_EXIT, NULL, NULL);
}

char* GetScreenshotsFolder()
{
	TCHAR szEXEPath[MAX_PATH];
	char actualpath[MAX_PATH];
	GetModuleFileName ( NULL, szEXEPath, MAX_PATH );

	/* Prints the full path */
	int last = 0;
	int end = 0;
	for(int j=0; szEXEPath[j]!=0; j++)
	{
		actualpath[j]=szEXEPath[j];
		if(actualpath[j] == '\\')
		{
			last = j;
		}
		end = j;
	}
	actualpath[end+1] = '\0';
	//actualpath[j+2] = '\0';
	cout << "\n\n";

	char finalpath[MAX_PATH];
	end = 0;
	for(int k=0; k<=last; k++)
	{
		finalpath[k] = actualpath[k];
		end = k;
	}
	finalpath[end+1] = '\0';
	strcat(finalpath,"Screenshots\\");

	return finalpath;
}

NOTIFYICONDATA nid;
void CreateTrayIcon(HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 100;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_MYMESSAGE;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcscpy_s(nid.szTip, L"ScreenKeeper");
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void KillProgram(HWND hWnd)
{
	DestroyWindow(hWnd);
	Shell_NotifyIcon(NIM_DELETE, &nid);
	PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
		case WM_CREATE:
			CreateLayout(hWnd);
			CreateTrayIcon(hWnd);
			system("mkdir Screenshots");
			break;
		case WM_DESTROY:
			KillProgram(hWnd);
			break;
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == BTN_SCREENSHOT) {
				printScreen();
			}
			else if (LOWORD(wParam) == BTN_FOLDER) {
				// open screenshots folder
				
				char command[MAX_PATH + 50];
				strcpy(command,"explorer ");
				strcat(command,GetScreenshotsFolder());

				cout << "command: " << command << "\n";
				system(command);
			}
			else if (LOWORD(wParam) == BTN_HIDE) {
				ShowWindow(hWnd, SW_HIDE);
			}
			else if (LOWORD(wParam) == BTN_EXIT) {
				KillProgram(hWnd);
			}
			break;
		}
		case WM_MYMESSAGE:
			switch(lParam)
			{
				case WM_LBUTTONDBLCLK:
					ShowWindow(hWnd, SW_SHOW);
					break;
				case WM_LBUTTONUP:
					cout << "PASS2!";
					break;
				default:
					return DefWindowProc(hWnd, msg, wParam, lParam);
			};
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	};
	return 0;
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes
 
	ImageCodecInfo* pImageCodecInfo = NULL;
 
	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure
 
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure
 
	GetImageEncoders(num, size, pImageCodecInfo);
 
	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}
 
	free(pImageCodecInfo);
	return 0;
}


LRESULT CALLBACK KeyboardHook(
	int nCode, // hook code
	WPARAM wParam, // message identifier
	LPARAM lParam // pointer to structure with message data
);

HHOOK hHook;

int APIENTRY WinMain(HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow )
{
	//AllocConsole();
	//HWND consoleWindow = GetConsoleWindow();
	//freopen("CONOUT$", "w", stdout);
	
	////////////////////////////////////////////////////////////
	MSG  msg;    
	WNDCLASS wc = {0};
	wc.lpszClassName = TEXT("ScreenKeeper");
	wc.hInstance     = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc   = WndProc;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	
	RegisterClass(&wc);
	CreateWindow( wc.lpszClassName, TEXT("ScreenKeeper"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		200, 200, 400, 400, 0, 0, hInstance, 0);  

	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);
	///////////////////////////////////////////////////////////////////



	//MessageBox(NULL, TEXT("After"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
	hHook = SetWindowsHookEx(13, KeyboardHook, hInstance, 0);
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}// NOP while not WM_QUIT

	UnhookWindowsHookEx(hHook);
	return (int) msg.wParam;
}

int picCount = 0;

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam )
{
	if (nCode == HC_ACTION)
	if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
	CheckKey (((PKBDLLHOOKSTRUCT)lParam)->vkCode);
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void CheckKey(int key)
{
	if(key == 44)
	{
		cout << "[PRNT]";
		printScreen();
	}
	else
	{
		cout << "[" << key << "]";
	}
}

void printScreen()
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	picCount++;

	wchar_t filename[256];
	//wsprintf(filename,L"Screens\\screen_%02d-%02d-%02d_%02d-%02d-%02d.png",
	wsprintf(filename,L"Screenshots/screen_%02d-%02d-%02d_%02d-%02d-%02d-%02d.png",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond,
		picCount);
	gdiscreen(filename);
}

