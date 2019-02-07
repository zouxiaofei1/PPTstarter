#include "stdafx.h"//ͷ�ļ�
#include <map>
BOOL				InitInstance(HINSTANCE, int);//����(��Ҫ)������ǰ������
LRESULT	CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);//������

//�Լ������ �е���

HINSTANCE hInst;// ��ǰʵ�����ݱ�����CreateWindow&LoadIconʱ��Ҫ
const wchar_t szWindowClass[] = L"PPTstarter";//����������

HBRUSH DBlueBrush, LBlueBrush, WhiteBrush, red;//��ˢ
HPEN BLACK, r;//��
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

class CathyClass//�ؼ�����
{
public:
	void InitClass(HINSTANCE HInstance)//��Classʹ��֮ǰ���Initһ��
	{								//  ����InitҲ�У�
		hInstance = HInstance;
		CurCover = -1;

		//Ĭ������
		DefFont = CreateFontW(48, 24, 0, 0, FW_EXTRABOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("����"));
	}

	ATOM RegisterClass(HINSTANCE h, WNDPROC proc, LPCWSTR ClassName)
	{//ע��Class
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = proc;
		wcex.hInstance = h;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = ClassName;//�Զ���ClassName��WndProc
		return RegisterClassExW(&wcex);
	}

	void CreateButtonEx(int Number, int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, HBRUSH Leave, \
		HBRUSH Hover, HBRUSH press, HPEN Leave2, HPEN Hover2, HPEN Press2, HFONT Font, COLORREF FontRGB, LPCWSTR ID)//������ť
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
	void CreateButton(int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, LPCWSTR ID)//������ť���򻯰棩
	{
		++CurButton;
		CreateButtonEx(CurButton, Left, Top, Wid, Hei, Page, name, WhiteBrush, DBlueBrush, LBlueBrush, BLACK, BLACK, BLACK, 0, RGB(0, 0, 0), ID);
	}

	inline BOOL InsideButton(int cur, POINT &point)//�ж����ָ���Ƿ��ڰ�ť��
	{
		return (Button[cur].Left <= point.x && Button[cur].Top <= point.y && (Button[cur].Left + Button[cur].Width) >= point.x && (Button[cur].Top + Button[cur].Height) >= point.y);
	}

	void DrawButtons()//���ư�ť
	{
		int i;
		for (i = 1; i <= CurButton; ++i)
		{
			if (Button[i].Page == CurWnd || Button[i].Page == 0)
			{
				SetTextColor(hdc, Button[i].FontRGB);

				if (CurCover == i)//û�н���&����ɫ -> Ĭ����ɫ
					if (Press == 1) {
						SelectObject(hdc, Button[i].Press);//���°�ť
						SelectObject(hdc, Button[i].Press2);
					}
					else {
						SelectObject(hdc, Button[i].Hover);//����
						SelectObject(hdc, Button[i].Hover2);
					}
				else
				{
					SelectObject(hdc, Button[i].Leave);//�뿪
					SelectObject(hdc, Button[i].Leave2);
				}
				if (Button[i].Font == NULL)SelectObject(hdc, DefFont); else SelectObject(hdc, Button[i].Font);//����

				Rectangle(hdc, Button[i].Left, Button[i].Top, Button[i].Left + Button[i].Width, Button[i].Top + Button[i].Height);//���Ʒ���

				RECT rc = GetRECT(i);

				SetBkMode(hdc, TRANSPARENT);//ȥ�����ֱ���
				DrawTextW(hdc, Button[i].Name, (int)wcslen(Button[i].Name), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			}
		}
		//SetTextColor(hdc, RGB(0, 0, 0));
	}

	RECT GetRECT(int cur)//����Buttons��rc
	{
		RECT rc = { Button[cur].Left, Button[cur].Top,Button[cur].Left + Button[cur].Width,Button[cur].Top + Button[cur].Height };
		return rc;
	}
	void LeftButtonDown()//����������
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);//���� ��ȡ����
		if (CurCover != -1)//����ť���� & ͣ���ڰ�ť��ʱ
		{
			Press = 1;
			RECT rc = GetRECT(CurCover);
			Redraw(&rc);
		}

	}
	void ButtonGetNewInside(POINT &point)//���point�Ƿ���check��
	{
		for (int i = 0; i <= CurButton; ++i)//��ʷԭ��Button����Ǵ�0��ʼ��
			if (Button[i].Page == CurWnd || Button[i].Page == 0)
				if (InsideButton(i, point))//�ڰ�ť��
				{
					CurCover = i;//����curcover
					RECT rc = GetRECT(i);//�ػ�
					Redraw(&rc);
					return;
				}
	}

	void MouseMove()//����ƶ�
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		if (CurCover == -1)ButtonGetNewInside(point);//ԭ�����ڰ�ť�� -> ���������Ƿ��ƽ���ť
		else//ԭ����
		{
			if ((Button[CurCover].Page != CurWnd && Button[CurCover].Page != 0) || !InsideButton(CurCover, point))
			{
				RECT rc = GetRECT(CurCover);
				CurCover = -1;
				Redraw(&rc);
				ButtonGetNewInside(point);//�п��ܴ�һ����ťֱ���ƽ���һ����ť��
			}
		}

		if (Msv == 0)
		{//�������ƽ��Ƴ��Ĵ���
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = hWnd;
			tme.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tme);
			Msv = 1;//�Ƴ�
		}
		else Msv = 0;//�ƽ�
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

	struct ButtonEx//��ť
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
	int Msv;//����Ƴ�������
	int CurButton;
	int CurCover;
	int CurWnd;
	int Press;
	HDC hdc;//����dc
	HDC tdc;//��ʵdc
	int Width, Height;
	HWND hWnd;
	HINSTANCE hInstance;
private:
}Main;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,//��ڵ�
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

	if (!InitInstance(hInstance, nCmdShow))return FALSE;
	HACCEL hAccelTable = LoadAccelerators(hInstance, 0);

	MSG msg;
	// ����Ϣѭ��: 
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
void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)//����ʱ��
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

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
	Main.InitClass(hInst);
	if (!Main.RegisterClassW(hInst, WndProc, szWindowClass))return FALSE;

	Main.hWnd = CreateWindowW(szWindowClass, L"��ʼ", NULL, CW_USEDEFAULT, CW_USEDEFAULT, 250, 170, NULL, nullptr, hInstance, nullptr);
	SetWindowLong(Main.hWnd, GWL_STYLE, GetWindowLong(Main.hWnd, GWL_STYLE)& ~WS_CAPTION & ~WS_THICKFRAME&~WS_SYSMENU&~WS_GROUP&~WS_TABSTOP);//�ޱ߿򴰿�
	SetTimer(Main.hWnd, 1, 100, (TIMERPROC)TimerProc);
	if (!Main.hWnd)return FALSE;

	Main.CreateButton(0, 20, 250, 150, 0, L"��ʼ��ӳ", L"h");

	return TRUE;
}
//HWND T;
BOOL CALLBACK EnumTDwnd(HWND hwnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	ULONG nProcessID;
	LONG A;//�������д���
	A = GetWindowLongW(hwnd, GWL_STYLE) & WS_VISIBLE;
	if (A != 0 && GetParent(hwnd) == NULL)
	{
		::GetWindowThreadProcessId(hwnd, &nProcessID);//���pid��ȷ����hwnd��¼����
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

//��Ӧ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//��������Ӧ����
{
	switch (message)
	{
	case WM_CLOSE://�ر�
		PostQuitMessage(0);
		break;
	case WM_PAINT://��ͼ
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