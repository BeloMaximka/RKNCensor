#pragma once
#include <Windows.h>
#include <windowsX.h>
#include "..\rsc\resource.h"
#include <tchar.h>

class CensorDlg
{
	static CensorDlg* ptr;
	void OnClose(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
public:
	CensorDlg();
	//~CensorDlg();
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};