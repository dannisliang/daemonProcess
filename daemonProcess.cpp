// daemonProcess.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <io.h> 
#include <iostream>
#include <string>

using namespace std;

//隐藏DOS黑窗口
#pragma comment(linker,"/subsystem:\"windows\"  /entry:\"mainCRTStartup\"" ) 
#pragma  warning( disable: 4996)
//定义路径最大程度
#define MAX_PATH 4096

//定义写入的注册表路径
#define SELFSTART_REGEDIT_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"


//ansi转unicode
wchar_t* AnsiToUnicode(const char* lpszStr)
{
	wchar_t* lpUnicode;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, NULL, 0);
	if (0 == nLen)
		return NULL;

	lpUnicode = new wchar_t[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(wchar_t)* (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	return lpUnicode;
}

//注册表自启动
BOOL autostart(char *pPath)
{
	char pName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, pName, MAX_PATH);
	string val =string(pName) + " " +"\'"+string(pPath)+"\'";//注册表需要填入的参数

	LPCTSTR lpSubKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKey;
	REGSAM flag = KEY_WOW64_64KEY;
	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS | flag, &hKey);
	//LONG lRet = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);  
	if (ERROR_SUCCESS != lRet)
	{
		return false;
	}
	wchar_t *pchrName = AnsiToUnicode(val.c_str());
	lRet = ::RegSetValueEx(hKey,AnsiToUnicode(pPath), NULL, REG_SZ, (LPBYTE)pchrName, wcslen(pchrName) * sizeof(TCHAR) + 1); //设置注册表项  
	if (ERROR_SUCCESS != lRet)
	{
		return false;
	}

	::RegCloseKey(hKey); //与RegCreateKeyEx配对写  
	return true;

}


//输入程序路径

int main(int argc,char * argv[])
{
	cout << "使用方法:" << "daemonProcess.exe 'xx.py' 或者 daemonProcess.exe 'xx.exe'";
	int strLen = sizeof(argv[1]);
	string Cmd;//cmd命令
	char * pPath = argv[1];
	//strcpy(pPath, argv[1]);//程序路径

	//const char *x=&pPath[strLen-2];//判断文件名最后一个是否为py结束
	string x = string(pPath);

	if(x.find(".py")!=string::npos){
		Cmd = "pythonw.exe  " + string(pPath);
	}
	else {
		 Cmd =string(pPath);
	}
	char * pCmd = (LPSTR)Cmd.c_str();
	//strcpy(pCmd, Cmd.c_str());//所执行的命令为pCmd

	//设置程序开机自启动
	if (!autostart(pPath))
	{
		cout << "守护进程开机自启动失败" << endl;
		return -1;
	}

	STARTUPINFOA si;
	//进程对象
	PROCESS_INFORMATION pi;
	//初始化
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	//无限循环，监视守护进程
	do {
		//检查守护程序是否存在
		if (_access(pPath, 0) != -1)
		{
			//创建子进程，判断是否执行成功
			if (!CreateProcessA(NULL, pCmd, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
			{
				cout << "守护进程启动失败，程序即将退出" << endl;
				return -1;
			}

			//启动成功，获取进程的ID
			cout << "守护进程成功，ID:" << pi.dwProcessId << endl;
			//无限等待子进程退出
			WaitForSingleObject(pi.hProcess, INFINITE);
			//如果退出了
			cout << "守护进程退出了。。。" << endl;
			//关闭进程和句柄
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
		{
			cout << "守护程序不存在" << endl;
		}
		//睡一下，重启
		Sleep(1000);
	} while (true);

	return 0;
}


// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
