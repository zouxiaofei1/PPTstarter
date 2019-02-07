#pragma once
#include "stdafx.h"
#ifndef _WIN64
constexpr auto Bitstr = L"32Bit Update 199";
#else
constexpr auto Bitstr = L"64Bit Update 199";
#endif

void charTowchar(const char *chr, wchar_t *wchar, int size);
void s(LPCWSTR cathy)
{
	MessageBox(NULL, cathy, L"", NULL);
}
void s(int cathy)
{
	wchar_t Cathytmp[34];
	_itow_s(cathy, Cathytmp, 10);
	MessageBox(NULL, Cathytmp, L"", NULL);
}

void charTowchar(const char *chr, wchar_t *wchar, int size)
{
	MultiByteToWideChar(CP_ACP, 
		0, chr, strlen(chr) + 1,
		wchar, 
		size / sizeof(wchar[0]));
}

#pragma warning(disable:4244)

void change(void *Src, bool wow)
{
	unsigned int v5, v10;
	BYTE *v6, *v7, v8, *v9, *i;
	HKEY phkResult;
	LPCWSTR lpSubKeya;

	phkResult = 0;

	if (wow)RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\WOW6432Node\\TopDomain\\e-Learning Class\\Student", 0, 0x20006u, &phkResult);
	else
		RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\TopDomain\\e-Learning Class\\Student", 0, 0x20006u, &phkResult);

	v5 = (rand() % 40 + 83) & 0xFFFFFFFC;
	v6 = (BYTE *)operator new[](v5);
	v7 = v6;
	if (v5 >> 1)
	{
		lpSubKeya = (LPCWSTR)(v5 >> 1);
		do
		{
			*(WORD *)v7 = rand();
			v7 += 2;
			lpSubKeya = (LPCWSTR)((char *)lpSubKeya - 1);
		} while (lpSubKeya);
	}
	v8 = rand() % (v5 - 68) + 2;
	*v6 = v8;
	v6[v5 - 1] = v8;
	if (Src && wcslen((const wchar_t *)Src))
		memcpy(&v6[v8], Src, 2 * wcslen((const wchar_t *)Src) + 2);
	else
	{
		v9 = &v6[v8];
		*v9 = 0;
		v9[1] = 0;
	}
	v10 = v5 >> 2;

	for (i = v6; v10; --v10)
	{
		*(DWORD *)i ^= 0x150f0f15u;
		i += 4;
	}

	RegSetValueExW(phkResult, L"Knock", 0, 3u, v6, v5);
	RegSetValueExW(phkResult, L"Knock1", 0, 3u, v6, v5);
	operator delete[](v6);

	RegCloseKey(phkResult);
}
#pragma warning(default:4244)
BOOL LoadNTDriver(LPCWSTR lpszDriverName, LPCWSTR lpszDriverPath)

{
	wchar_t szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);
	//s(szDriverImagePath);
	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

								 //打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		bRet = FALSE;
		goto BeforeLeave;
	}

	//创建驱动所对应的服务
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;

	//判断服务是否失败
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//s(99);
			bRet = FALSE;
			goto BeforeLeave;
		}
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			//s(999);
			bRet = FALSE;
			goto BeforeLeave;
		}
	}

	//开启此项服务
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				//s(99999);
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	//离开前关闭句柄
BeforeLeave:
	if (hServiceDDK)CloseServiceHandle(hServiceDDK);
	if (hServiceMgr)CloseServiceHandle(hServiceMgr);
	return bRet;
}



//卸载驱动程序  
BOOL UnloadNTDriver(LPCWSTR szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		bRet = FALSE;
		goto BeforeLeave;
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if (hServiceDDK == NULL)
	{
		bRet = FALSE;
		goto BeforeLeave;
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta);
	//动态卸载驱动程序。  
	DeleteService(hServiceDDK);
	bRet = TRUE;
BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK)CloseServiceHandle(hServiceDDK);
	if (hServiceMgr)CloseServiceHandle(hServiceMgr);
	return bRet;
}

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
#define CreateStrs \
	Main.CreateString(L"安装成功，请按Scroll Lock键查看效果！", L"OneOK");\
	Main.CreateString(L"安装失败\n文件不存在", L"OneFail");\
	Main.CreateString(L"卸载成功", L"unQSOK");\
	Main.CreateString(L"提示", L"Info");\
	Main.CreateString(L"一键安装", L"setQS");\
	Main.CreateString(L"卸载", L"unQS");\
	Main.CreateString(L"启动失败", L"StartFail");\
	Main.CreateString(L"Sethc删除失败！权限不足", L"DSR3Fail");\
	Main.CreateString(L"Sethc删除失败！\n可能因为权限不足/驱动文件不存在", L"DSR0Fail");\
	Main.CreateString(L"Sethc复制失败！\n权限不足", L"CSFail");\
	Main.CreateString(L"Sethc复制失败！\n文件不存在", L"NoSethc");\
	Main.CreateString(L"Sethc卸载失败！\n备份文件丢失", L"USFail");\
	Main.CreateString(L"NTSD复制失败！\n权限不足", L"CNTSDFail");\
	Main.CreateString(L"NTSD复制失败！\n文件不存在", L"NoNTSD");\
	Main.CreateString(L"打开键值失败!\n可能因为没有足够权限或极域未安装", L"ACFail");\
	Main.CreateString(L"修改键值失败!\n(未知错误)", L"ACUKE");\
	Main.CreateString(L"修改成功", L"ACOK");\
	Main.CreateString(L"出错了...可能是因为键值不存在\n按确定强制读取", L"VPFail");\
	Main.CreateString(L"出错了...\n按确定强制读取", L"VPUKE");\
	Main.CreateString(L"密码为空或键值不存在", L"VPNULL");\
	Main.CreateString(L"删除文件", L"Tdelete");\
	Main.CreateString(L"复制文件", L"Tcopy");\
	Main.CreateString(L"按快捷键Ctrl+B切换", L"Tctrl+b");\
	Main.CreateString(L"极域不同版本密码格式不一样", L"Tcp1");\
	Main.CreateString(L"不保证修改成功", L"Tcp2");\
	Main.CreateString(L"作者：zouxiaofei1 cxy villager", L"Tcoder");\
	Main.CreateString(L"版本：C++ v1.9 正式版", L"Tver");\
	Main.CreateString(Bitstr, L"Tver2");\
	Main.CreateString(L"CopyLeft© SA软件 2015 - 2019", L"_Tleft");\
	Main.CreateString(L"© SA软件 2015 - 2019", L"Tleft");\
	Main.CreateString(L"版权 - 不存在的", L"Tleft2");\
	Main.CreateString(L"极域破解v1.9 不是管理员", L"Tmain");\
	Main.CreateString(L"极域破解v1.9", L"Tmain2");\
	Main.CreateString(L"这不是文件 / 文件夹！", L"TINotF");\
	Main.CreateString(L"请启动360！360可能会报加载驱动，请放行！\n如果执行后蓝屏，请把dump送至作者邮箱", L"360Start");\
	Main.CreateString(L"加载驱动失败!\n可能是因为权限不足/操作被360拦截/文件不存在", L"360Fail");\
	Main.CreateString(L"当前非管理员模式，蓝屏可能无效", L"BSODAsk");\
	Main.CreateString(L"密码为:", L"pswdis");\
	Main.CreateString(L"极域电子教室 - 已连接至教师端", L"tnd");\
	CatchWnd.CreateString(L"剩余 ", L"Timer1");\
	CatchWnd.CreateString(L" 秒", L"Timer2");\
	CatchWnd.CreateString(NULL, L"back");\
	CatchWnd.CreateString(L"进程名称:", L"Processnam");\
	CatchWnd.CreateString(L"延迟:", L"Delay");\
	CatchWnd.CreateString(L" 秒", L"second");\
	Main.CreateString(L"explorer.exe", L"E_runinVD");\
	Main.CreateString(L"输入端口", L"E_ApplyCh");\
	Main.CreateString(L"输入密码", L"E_CP");\
	Main.CreateString(L"浏览文件/文件夹", L"E_View");\
	Main.CreateString(L"StudentMain", L"E_TDname");\
	Main.CreateString(L"确定要把密码改成", L"CPAsk1");\
	Main.CreateString(L"么？", L"CPAsk2");\
	Main.CreateString(L"下载成功", L"Loaded");\
	Main.CreateString(L"正在下载", L"Loading");\
	Main.CreateString(L"- 不可用 ", L"Useless");\
	Main.CreateString(L"- 可用 ", L"Usable");\
	Main.CreateString(L"- 推荐 ", L"Rec");\
	Main.CreateString(L"- 不推荐 ", L"nRec");\
	Main.CreateString(L"按 Ctrl+P 组合键可显示/隐藏此窗口", L"Ttip1");\
	Main.CreateString(L"一般情况下建议使用“一键安装”或“虚拟桌面”", L"Ttip2");\
	Main.CreateString(L"系统位数:", L"Tbit");\
	Main.CreateString(L"系统版本:", L"Twinver");\
	Main.CreateString(L"cmd状态:", L"Tcmd");\
	Main.CreateString(L"", L"TTDv");\
	Main.CreateString(L"极域版本: ", L"_TTDv");\
	Main.CreateString(L"未知或不存在", L"TTDunk");\
	Main.CreateString(L"2010或之前", L"TTDold");\
	Main.CreateString(L"IP地址:", L"TIP");\
	Main.CreateString(L"存在", L"TcmdOK");\
	Main.CreateString(L"不存在", L"TcmdNO");\
	Main.CreateString(L"打游戏", L"Games");\
	Main.CreateString(L"停止", L"Gamee");\
	Main.CreateString(L"已删除", L"Deleted");\
	Main.CreateString(L"已安装", L"Installed");\
	Main.CreateString(L"应用层", L"Ring3");\
	Main.CreateString(L"驱动层", L"Ring0");\
	Main.CreateString(L"安装", L"Setup");\
	Main.CreateString(L"已卸载", L"Uned");\
	Main.CreateString(L"极域状态: ", L"TDState");\
	Main.CreateString(L"PID: \\", L"TDPID");\
	Main.CreateString(L"极域状态: ", L"_TDState");\
	Main.CreateString(L"PID: ", L"_TDPID");\
	Main.CreateString(L"下载失败", L"DownFail");\
	Main.CreateString(L"极域破解1.9", L"Title");\
	Main.CreateString(L"打开文件夹",L"OpenFolder");\
	CatchWnd.CreateString(L"已经吃掉了 ", L"Eat1");\
	CatchWnd.CreateString(L" 个窗口", L"Eat2");\
	CatchWnd.CreateString(L"捕捉窗口", L"Title");\
	UpWnd.CreateString(L"下载文件", L"Title")

bool Findquotations(wchar_t* zxf, wchar_t zxf2[])//命令行调用找到"双引号"
{
	wchar_t tmp0;
	wchar_t *tmp1 = wcsstr(zxf, L"\"");
	wchar_t *tmp2 = wcsstr(tmp1 + 1, L"\"");
	if (tmp1 == 0 || tmp2 == 0)return false;
	tmp0 = *tmp2;
	*tmp2 = 0;
	wcscpy(zxf2, tmp1 + 1);
	*tmp2 = tmp0;
	return true;
}
