#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "CensorDlg.h"
#include <wow64apiset.h>
#include "TextFileEncoding.h"

CensorDlg* CensorDlg::ptr = NULL;

void CensorDlg::OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CensorDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SendDlgItemMessage(hwnd, IDC_PROGRESS1, PBM_SETBARCOLOR, 0, LPARAM(RGB(0, 200, 0)));
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
		top.clear();
		file_id = 0;
		MakeWordList(hwnd);
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

void CensorDlg::MakeWordList(HWND hwnd)
{
	words.clear();
	WCHAR text[256];
	int count = SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < count; i++)
	{
		SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETTEXT, WPARAM(i), LPARAM(text));
		words.push_back(text);
	}
}

bool CensorDlg::CensorText(wchar_t* text)
{
	bool replacement = false;

	wchar_t min = 65535;
	wchar_t max = 0;
	for (auto& temp : words)
	{
		if (temp[0] < min)
			min = temp[0];
		if (temp[0] > max)
			max = temp[0];
	}
	int len = wcslen(text);
	bool mismatch;
	wchar_t ch;
	for (int i = 0; i < len; i++)
	{
		ch = towlower(text[i]);
		if (ch >= min && ch <= max)
		{
			for (auto& temp : words)
			{
				// first letter match
				if (temp[0] == ch)
				{
					mismatch = false;
					// check other letters
					for (int j = 1; j < temp.size(); j++)
					{
						ch = towlower(text[i + j]);
						if (temp[j] != ch)
						{
							mismatch = true;
							break;
						}
					}
					if (!mismatch)
					{
						replacement = true;
						top[temp]++;
						// replace with *
						wmemset(&text[i], L'*', temp.size());
						i += temp.size(); // move i to next symbols
						break;
					}
				}
			}
		}
	}
	return replacement;
}

void CensorDlg::ProcessFile(HWND hwnd, const wchar_t* path)
{
	std::locale out_loc; // get locale from src file
	std::wifstream file = openTextFile(path, out_loc); // open with correct locale
	int id = file_id++; // unique id to avoid overwrite
	if (file.is_open())
	{
		// path to filename.cpy.txt
		std::wstring file_name(path);
		file_name.erase(0, file_name.find_last_of('\\') + 1);
		std::wstring id_str = std::to_wstring(id) + L'.';
		file_name.insert(file_name.begin(), id_str.begin(), id_str.end());
		file_name.replace(file_name.end() - 4, file_name.end(), L".cpy.txt");
		std::wofstream cpy_file(file_name.c_str());

		cpy_file.imbue(out_loc); // locale for copy

		bool replacement = false;
		std::wstring result;
		while (getline(file, result)) // main loop
		{
			if (!replacement)
				replacement = CensorText(&result[0]);
			else
				CensorText(&result[0]);
			cpy_file << result << std::endl;
		}
		file.close();
		cpy_file.close();
		
		if (!replacement)
			DeleteFile(file_name.c_str());
		else
		{
			// filename.cpy.txt -> filename.txt
			file_name.replace(file_name.end() - 8, file_name.end(), L".txt");
			CopyFile(path, file_name.c_str(), FALSE);
		}
	}
}

void CensorDlg::ProcessDirectory(HWND hwnd, const wchar_t* path)
{
	std::vector<std::wstring> files;
	GetFileListFromDirectory(path, files);
	SendDlgItemMessage(hwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, files.size()));
	for (int i = 0; i < files.size(); i++)
	{
		ProcessFile(hwnd, files[i].c_str());
		SendDlgItemMessage(hwnd, IDC_PROGRESS1, PBM_SETPOS, WPARAM(i + 1), 0);
	}
}

void CensorDlg::GetFileListFromDirectory(const wchar_t* path, std::vector<std::wstring>& files)
{
	//Wow64DisableWow64FsRedirection(&OldValue);
	WIN32_FIND_DATAW wfd;
	wchar_t* mask = new wchar_t[wcslen(path) + 8];
	wsprintf(mask, TEXT("%s%s"), path, L"\\*.txt");
	HANDLE const hFind = FindFirstFileW(mask, &wfd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t* filePath = new wchar_t[wcslen(path) + wcslen(wfd.cFileName) + 2];
			wsprintf(filePath, TEXT("%s%s%s"), path, L"\\", wfd.cFileName);
			files.push_back(filePath);
			delete[] filePath;
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
	delete[] mask;
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
