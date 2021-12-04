#pragma once
#include <Windows.h>
#include <windowsX.h>

#include "..\rsc\resource.h"

#include <regex>
#include <tchar.h>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <map>
#include <string>

#include <commctrl.h>
#pragma comment(lib,"comctl32")

class CensorDlg
{
	static CensorDlg* ptr;
	void OnClose(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	void AddWord(HWND hwnd);
	void DeleteWord(HWND hwnd);
	void ClearWords(HWND hwnd);
	void MakeWordList(HWND hwnd);

	bool CensorText(wchar_t* text);
	void ProcessFile(HWND hwnd, const wchar_t* path);
	void ProcessDirectory(HWND hwnd, const wchar_t* path);

	void GetFileListFromDirectory(const wchar_t* path, std::vector<std::wstring>& files);

	std::vector<std::wstring> words;
	std::map<std::wstring, int> top;
	int file_id;
public:
	CensorDlg();
	//~CensorDlg();
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};