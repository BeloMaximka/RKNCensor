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
#include <thread>

#include <commctrl.h>
#pragma comment(lib,"comctl32")

class CensorDlg
{
	static HBRUSH brush;
	// pairs of size and filepath
	typedef std::vector<std::pair<int, std::wstring>> FilesList;

	static CensorDlg* ptr;
	void OnClose(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	void AddWord(HWND hwnd);
	void DeleteWord(HWND hwnd);
	void ClearWords(HWND hwnd);
	void MakeWordList(HWND hwnd);

	bool CensorText(wchar_t* text);
	void ProcessFile(const wchar_t* path);
	void ProcessFiles(HWND hwnd, std::vector<std::wstring> files);
	void ProcessDirectory(HWND hwnd, std::wstring path);

	FilesList GetFileListFromDirectory(const wchar_t* path, bool recursive = true);
	void PrintIntOutputList(int i, const wchar_t* text);

	std::vector<std::wstring> words;
	std::map<std::wstring, int> top;

	LONG file_id = 1;
	LONG progress;
	HANDLE mutex_output;
	std::thread* threads;
	unsigned int timer_exit;
	std::thread process_thread;
	unsigned int cores;
	unsigned int files_count = 0;
	void Timer(HWND hwnd);
	HWND output_list;
	bool kill_thread;
public:
	CensorDlg();
	//~CensorDlg();
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};