#pragma once
#include <Windows.h>
#include <windowsX.h>

#include "..\rsc\resource.h"
#include "WordList.h"
#include "MFTscan.h"

#include <regex>
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
	enum class DirectoryMethod
	{
		ALL_VOLUMES,
		VOLUME,
		DIR,
	};
	static HBRUSH brush;
	// pairs of size and filepath
	typedef std::vector<std::pair<int, std::wstring>> FilesList;

	static CensorDlg* ptr;
	void OnClose(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	std::wstring getOutPath(HWND hwnd);

	bool CensorText(wchar_t* text);
	void ProcessFile(const wchar_t* path);
	void ProcessPortion(HWND hwnd, std::vector<std::wstring> files);
	void ProcessFilesList(HWND hwnd, DirectoryMethod method);

	std::wstring SelectDir();
	FilesList GetFileListFromDirectory(const wchar_t* path, bool recursive = true);
	void PrintIntOutputList(int index, const wchar_t* text);
	void InterlockedPushBack(std::vector<std::wstring>& colletion, std::wstring elem);
	HANDLE pushback_mutex;

	HWND word_list;
	HWND output_list;
	std::vector<std::wstring> words;
	std::vector<std::wstring> processed_files;
	std::map<std::wstring, unsigned int> top;
	std::wstring out_path;
	
	LONG file_id = 1;
	LONG progress;
		
	HANDLE mutex_output;
	bool kill_thread = false;

	std::thread* threads;
	std::thread process_thread;
	std::thread timer_thread;
	unsigned int cores;
	
	unsigned int files_count = 0;
	void Timer(HWND hwnd);
	
public:
	CensorDlg();
	//~CensorDlg();
	static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};