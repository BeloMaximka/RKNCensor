#pragma once
#include <Windows.h>
#include <windowsX.h>
#include "..\rsc\resource.h"
#include <tchar.h>
#include <algorithm>

class CensorDlg
{
	static CensorDlg* ptr;
	void OnClose(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	void AddWord(HWND hwnd);
public:
	CensorDlg();
	//~CensorDlg();
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};