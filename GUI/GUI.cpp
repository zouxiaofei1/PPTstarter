#include "stdafx.h"//头文件
#include <map>
BOOL				InitInstance(HINSTANCE, int);//部分(重要)函数的前向声明
LRESULT	CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);//主窗口

//自己定义的 有点乱

HINSTANCE hInst;// 当前实例备份变量，CreateWindow&LoadIcon时需要
const wchar_t szWindowClass[] = L"PPTstarter";//主窗口类名

HBRUSH DBlueBrush, LBlueBrush, WhiteBrush, red;//笔刷
HPEN BLACK, r;//笔
HDC rdc;

unsigned int Hash(const wchar_t *str)
{
	unsigned int seed = 131;
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

class CathyClass//控件主类
{
public:
	void InitClass(HINSTANCE HInstance)//新Class使用之前最好Init一下
	{								//  （不Init也行）
		hInstance = HInstance;
		CurCover = -1;

		//默认宋体
		DefFont = CreateFontW(48, 24, 0, 0, FW_EXTRABOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
	}

	ATOM RegisterClass(HINSTANCE h, WNDPROC proc, LPCWSTR ClassName)
	{//注册Class
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = proc;
		wcex.hInstance = h;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = ClassName;//自定义ClassName和WndProc
		return RegisterClassExW(&wcex);
	}

	void CreateButtonEx(int Number, int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, HBRUSH Leave, \
		HBRUSH Hover, HBRUSH press, HPEN Leave2, HPEN Hover2, HPEN Press2, HFONT Font, COLORREF FontRGB, LPCWSTR ID)//创建按钮
	{
		Button[Number].Left = Left; Button[Number].Top = Top;
		Button[Number].Width = Wid; Button[Number].Height = Hei;
		Button[Number].Page = Page; Button[Number].Leave = Leave;
		Button[Number].Hover = Hover; Button[Number].Press = press;
		Button[Number].Leave2 = Leave2; Button[Number].Hover2 = Hover2;
		Button[Number].Press2 = Press2; Button[Number].Font = Font;
		Button[Number].FontRGB = FontRGB;
		wcscpy_s(Button[Number].Name, name);
		wcscpy_s(Button[Number].ID, ID);
		but[Hash(ID)] = Number;
	}
	void CreateButton(int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, LPCWSTR ID)//创建按钮（简化版）
	{
		++CurButton;
		CreateButtonEx(CurButton, Left, Top, Wid, Hei, Page, name, WhiteBrush, DBlueBrush, LBlueBrush, BLACK, BLACK, BLACK, 0, RGB(0, 0, 0), ID);
	}

	inline BOOL InsideButton(int cur, POINT &point)//判断鼠标指针是否在按钮内
	{
		return (Button[cur].Left <= point.x && Button[cur].Top <= point.y && (Button[cur].Left + Button[cur].Width) >= point.x && (Button[cur].Top + Button[cur].Height) >= point.y);
	}

	void DrawButtons()//绘制按钮
	{
		int i;
		for (i = 1; i <= CurButton; ++i)
		{
			if (Button[i].Page == CurWnd || Button[i].Page == 0)
			{
				SetTextColor(hdc, Button[i].FontRGB);

				if (CurCover == i)//没有禁用&渐变色 -> 默认颜色
					if (Press == 1) {
						SelectObject(hdc, Button[i].Press);//按下按钮
						SelectObject(hdc, Button[i].Press2);
					}
					else {
						SelectObject(hdc, Button[i].Hover);//悬浮
						SelectObject(hdc, Button[i].Hover2);
					}
				else
				{
					SelectObject(hdc, Button[i].Leave);//离开
					SelectObject(hdc, Button[i].Leave2);
				}
				if (Button[i].Font == NULL)SelectObject(hdc, DefFont); else SelectObject(hdc, Button[i].Font);//字体

				Rectangle(hdc, Button[i].Left, Button[i].Top, Button[i].Left + Button[i].Width, Button[i].Top + Button[i].Height);//绘制方框

				RECT rc = GetRECT(i);

				SetBkMode(hdc, TRANSPARENT);//去掉文字背景
				DrawTextW(hdc, Button[i].Name, (int)wcslen(Button[i].Name), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			}
		}
		//SetTextColor(hdc, RGB(0, 0, 0));
	}

	RECT GetRECT(int cur)//更新Buttons的rc
	{
		RECT rc = { Button[cur].Left, Button[cur].Top,Button[cur].Left + Button[cur].Width,Button[cur].Top + Button[cur].Height };
		return rc;
	}
	void LeftButtonDown()//鼠标左键按下
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);//惯例 获取坐标
		if (CurCover != -1)//当按钮按下 & 停留在按钮上时
		{
			Press = 1;
			RECT rc = GetRECT(CurCover);
			Redraw(&rc);
		}

	}
	void ButtonGetNewInside(POINT &point)//检查point是否在check内
	{
		for (int i = 0; i <= CurButton; ++i)//历史原因，Button编号是从0开始的
			if (Button[i].Page == CurWnd || Button[i].Page == 0)
				if (InsideButton(i, point))//在按钮中
				{
					CurCover = i;//设置curcover
					RECT rc = GetRECT(i);//重绘
					Redraw(&rc);
					return;
				}
	}

	void MouseMove()//鼠标移动
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		if (CurCover == -1)ButtonGetNewInside(point);//原来不在按钮内 -> 看看现在是否移进按钮
		else//原来在
		{
			if ((Button[CurCover].Page != CurWnd && Button[CurCover].Page != 0) || !InsideButton(CurCover, point))
			{
				RECT rc = GetRECT(CurCover);
				CurCover = -1;
				Redraw(&rc);
				ButtonGetNewInside(point);//有可能从一个按钮直接移进另一个按钮内
			}
		}

		if (Msv == 0)
		{//检测鼠标移进移出的代码
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = hWnd;
			tme.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tme);
			Msv = 1;//移出
		}
		else Msv = 0;//移进
	}

	LPWSTR GetCurInsideID()
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		for (int i = 0; i <= CurButton; ++i)
			if ((Button[i].Page == CurWnd || Button[i].Page == 0))
				if (InsideButton(i, point))
					return Button[i].ID;
		return Button[0].ID;
	}
	inline int GetNumbyID(LPCWSTR ID) { return but[Hash(ID)]; }

	void SetHDC(HDC HDc)
	{
		hdc = HDc;
	}

	void Redraw(const RECT *rc) { InvalidateRect(hWnd, rc, FALSE); UpdateWindow(hWnd); }

	struct ButtonEx//按钮
	{
		long Left, Top, Width, Height, Page;
		HBRUSH Leave, Hover, Press;
		HPEN Leave2, Hover2, Press2;
		HFONT Font;
		wchar_t Name[31], ID[11], Exp[1];
		COLORREF FontRGB;
		BYTE b1[3], b2[3], p1[3], p2[3];
	}Button[2];

	std::map<unsigned int, int>but;
	HFONT DefFont;
	int Msv;//鼠标移出检测变量
	int CurButton;
	int CurCover;
	int CurWnd;
	int Press;
	HDC hdc;//缓存dc
	HDC tdc;//真实dc
	int Width, Height;
	HWND hWnd;
	HINSTANCE hInstance;
private:
}Main;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,//入口点
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

	if (!InitInstance(hInstance, nCmdShow))return FALSE;
	HACCEL hAccelTable = LoadAccelerators(hInstance, 0);

	MSG msg;
	// 主消息循环: 
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
DWORD pid = 0;
bool show = false;
void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)//主计时器
{
	bool f = false;
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapShot, &pe))return;
	while (Process32Next(hSnapShot, &pe))
	{
		if ((pe.szExeFile[0] == 'p' || pe.szExeFile[0] == 'P') && \
			(pe.szExeFile[1] == 'o' || pe.szExeFile[1] == 'O') && \
			(pe.szExeFile[2] == 'w' || pe.szExeFile[2] == 'W')
			)
		{
			if (show)return;
			show = f = true;
			Sleep(1500);
			ShowWindow(Main.hWnd, SW_SHOW);
			SetWindowPos(Main.hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			pid = pe.th32ProcessID;
		}
	}
	if (f == false)
	{
		show = false;
		pid = 0;
		ShowWindow(Main.hWnd, SW_HIDE);
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	DBlueBrush = CreateSolidBrush(RGB(0xFF, 0xF9, 0xB8));
	LBlueBrush = CreateSolidBrush(RGB(0xFF, 0xFB, 0xDB));
	WhiteBrush = CreateSolidBrush(RGB(0xFF, 0xFC, 0xEA));
	red = CreateSolidBrush(RGB(0xFF, 0xC1, 0x25));
	r = CreatePen(PS_SOLID, 1, RGB(0xFF, 0x45, 0x00));
	BLACK = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	hInst = hInstance; // 将实例句柄存储在全局变量中
	Main.InitClass(hInst);
	if (!Main.RegisterClassW(hInst, WndProc, szWindowClass))return FALSE;

	Main.hWnd = CreateWindowW(szWindowClass, L"开始", NULL, CW_USEDEFAULT, CW_USEDEFAULT, 250, 170, NULL, nullptr, hInstance, nullptr);
	SetWindowLong(Main.hWnd, GWL_STYLE, GetWindowLong(Main.hWnd, GWL_STYLE)& ~WS_CAPTION & ~WS_THICKFRAME&~WS_SYSMENU&~WS_GROUP&~WS_TABSTOP);//无边框窗口
	SetTimer(Main.hWnd, 1, 100, (TIMERPROC)TimerProc);
	if (!Main.hWnd)return FALSE;

	Main.CreateButton(0, 20, 250, 150, 0, L"开始放映", L"h");

	return TRUE;
}
//HWND T;
BOOL CALLBACK EnumTDwnd(HWND hwnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	ULONG nProcessID;
	LONG A;//遍历所有窗口
	A = GetWindowLongW(hwnd, GWL_STYLE) & WS_VISIBLE;
	if (A != 0 && GetParent(hwnd) == NULL)
	{
		::GetWindowThreadProcessId(hwnd, &nProcessID);//如果pid正确，把hwnd记录下来
		if (pid == nProcessID)
		{
			SetWindowPos(Main.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			ShowWindow(hwnd, SW_MAXIMIZE);
			SetForegroundWindow(hwnd);
			keybd_event(16, 0, 0, 0);
			keybd_event(VK_F5, 0, 0, 0);
			keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(16, 0, KEYEVENTF_KEYUP, 0);
			Sleep(120);
		}
	}
	return 1;
}

//响应函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//主窗口响应函数
{
	switch (message)
	{
	case WM_CLOSE://关闭
		PostQuitMessage(0);
		break;
	case WM_PAINT://绘图
	{
		PAINTSTRUCT ps;
		rdc = BeginPaint(hWnd, &ps);
		Main.SetHDC(rdc);
		SetBkMode(rdc, TRANSPARENT);

		SelectObject(rdc, red);
		SelectObject(rdc, r);
		Rectangle(rdc, 0, 0, 250, 30);

		Main.DrawButtons();

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		POINT point; GetCursorPos(&point); ScreenToClient(Main.hWnd, &point);
		Main.ButtonGetNewInside(point);
		if (Main.CurCover != -1)Main.LeftButtonDown();
		else PostMessage(Main.hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
		break;
	}
	case WM_LBUTTONUP:
		if (Main.CurCover != -1)
		{
			Main.Press = 0;
			RECT rc;
			rc = Main.GetRECT(Main.CurCover);
			InvalidateRect(Main.hWnd, &rc, FALSE);
		}
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(Main.hWnd, &point);
		unsigned int x;
		x = Hash(Main.GetCurInsideID());
		if (x == Hash(L"h"))
		{
			EnumWindows(EnumTDwnd, NULL);
			break;
		}
		break;
	case WM_MOUSEMOVE: {Main.MouseMove(); break; }

	case WM_MOUSELEAVE:
		PostMessage(Main.hWnd, WM_MOUSEMOVE, NULL, 0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}