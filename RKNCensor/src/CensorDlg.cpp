#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "CensorDlg.h"
#include <wow64apiset.h>
#include <fstream>
#include <string>
#include <codecvt>

CensorDlg* CensorDlg::ptr = NULL;

void CensorDlg::OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CensorDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CensorDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_ADDWORD_BTN:
		
		AddWord(hwnd);
		break;
	case IDC_DELWORD:
		DeleteWord(hwnd);
		break;
	case IDC_CLEARLIST_BTN:
		ClearWords(hwnd);
		break;
	case IDC_RADIO1:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), FALSE);
		break;
	case IDC_RADIO2:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), FALSE);
		break;
	case IDC_RADIO3:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), TRUE);
		break;
	case IDC_START_BTN:
		TCHAR path[256];
		GetDlgItemText(hwnd, IDC_DIR_EDIT, path, 256);
		ProcessDirectory(hwnd, path);
		break;
	}
}

void CensorDlg::AddWord(HWND hwnd)
{
	TCHAR word[256];
	GetDlgItemText(hwnd, IDC_WORD_EDIT, word, 256);
	if (_tcslen(word) && SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_FINDSTRINGEXACT, -1, LPARAM(word)) == LB_ERR)
	{
		std::transform(word, word + 256, word, ::towlower);
		SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_ADDSTRING, 0, LPARAM(word));
	}
}

void CensorDlg::DeleteWord(HWND hwnd)
{
	int index = SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_DELETESTRING, WPARAM(index), 0);
}

void CensorDlg::ClearWords(HWND hwnd)
{
	SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_RESETCONTENT, 0, 0);
}

void CensorDlg::ProcessFile(HWND hwnd, const wchar_t* path)
{
	std::wifstream file(path);
	std::locale ru(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>{});
	file.imbue(ru);
	if (file.is_open())
	{
		std::wstring file_name(path);
		file_name.erase(0, file_name.find_last_of('\\'));
		file_name.insert(file_name.begin(), '.');
		CopyFile(path, file_name.c_str(), FALSE);
		wchar_t text[256];
		file.getline(text, 256);
		std::wstring result = std::regex_replace(text, regex, L"$***");
		file.close();

		file_name.replace(file_name.end() - 4, file_name.end(),L"_new.txt");
		std::wofstream new_file(file_name.c_str());
		new_file.imbue(ru);
		new_file << result;
		new_file.close();
	}
}

void CensorDlg::ProcessDirectory(HWND hwnd, const wchar_t* path)
{
	//PVOID OldValue = NULL;
	//Wow64DisableWow64FsRedirection(&OldValue);
	WIN32_FIND_DATAW wfd;
	wchar_t* mask = new wchar_t[wcslen(path) + 8];
	wsprintf(mask, TEXT("%s%s"), path, L"\\*.txt");
	HANDLE const hFind = FindFirstFileW(mask, &wfd);
	//setlocale(LC_ALL, "");

	if (hFind != INVALID_HANDLE_VALUE)
	{
		regex = MakeRegexFromList(hwnd);
		do
		{
			wchar_t* filePath = new wchar_t[wcslen(path) + wcslen(wfd.cFileName) + 2];
			wsprintf(filePath, TEXT("%s%s%s"), path, L"\\", wfd.cFileName);
			ProcessFile(hwnd, filePath);
			delete[] filePath;
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
	delete[] mask;
}

std::wregex CensorDlg::MakeRegexFromList(HWND hwnd)
{
	std::wstring regex;
	WCHAR text[256];
	int count = SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < count; i++)
	{
		SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETTEXT, WPARAM(i), LPARAM(text));
		regex += text;
		regex += L"|";
	}
	if (regex.size())
	{
		regex.pop_back();
	}
	return std::wregex(regex, std::regex_constants::icase);
}

CensorDlg::CensorDlg()
{
	ptr = this;
}

BOOL CensorDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}
