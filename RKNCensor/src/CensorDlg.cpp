#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "CensorDlg.h"
#include <wow64apiset.h>
#include <chrono>
#include "TextFileEncoding.h"

CensorDlg* CensorDlg::ptr = NULL;
HBRUSH CensorDlg::CensorDlg::brush = CreateSolidBrush(RGB(255, 255, 255));

void CensorDlg::OnClose(HWND hwnd)
{
	DeleteObject(brush);
	EndDialog(hwnd, 0);
}

BOOL CensorDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	output_list = GetDlgItem(hwnd, IDC_OUTPUT_LIST);
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
		//SendDlgItemMessage(hwnd, IDC_OUTPUT_LIST, LB_DELETESTRING, WPARAM(0), 0);
		//SendDlgItemMessage(hwnd, IDC_OUTPUT_LIST, LB_INSERTSTRING, WPARAM(0), LPARAM(L"Œ¡–¿¡Œ“ ¿..."));
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
		if (process_thread.get_id() != std::thread::id())
		{
			return;
		}
		top.clear();
		file_id = 0;
		progress = 0;
		SendDlgItemMessage(hwnd, IDC_PROGRESS1, PBM_SETPOS, WPARAM(0), 0);
		SendMessage(output_list, LB_RESETCONTENT, 0, 0);
		for (size_t i = 0; i < 3; i++)
			SendMessage(output_list, LB_INSERTSTRING, WPARAM(i), LPARAM(L"."));

		MakeWordList(hwnd);
		WCHAR path[256];
		GetDlgItemText(hwnd, IDC_DIR_EDIT, path, 256);
		std::wstring temp = path;
		process_thread = std::thread(&CensorDlg::ProcessDirectory, this, hwnd, temp);
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
					for (size_t j = 1; j < temp.size(); j++)
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

void CensorDlg::ProcessFile(const wchar_t* path)
{
	std::locale out_loc; // get locale from src file
	std::wifstream file = openTextFile(path, out_loc); // open with correct locale
	int id = InterlockedAdd(&file_id, 1); // unique id to avoid overwrite
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
			if (kill_thread)
			{
				if (!replacement)
					_wunlink(file_name.c_str());
				file.close();
				cpy_file.close();
				return;
			}
			if (!replacement)
				replacement = CensorText(&result[0]);
			else
				CensorText(&result[0]);
			cpy_file << result << std::endl;
		}
		file.close();
		cpy_file.close();

		if (!replacement)
			_wunlink(file_name.c_str());
		else
		{
			// filename.cpy.txt -> filename.txt
			file_name.replace(file_name.end() - 8, file_name.end(), L".txt");
			CopyFile(path, file_name.c_str(), FALSE);
		}
	}
}

void CensorDlg::ProcessFiles(HWND hwnd, std::vector<std::wstring> files)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	for (size_t i = 0; i < files.size(); i++)
	{
		ProcessFile(files[i].c_str());
		int prog = InterlockedAdd(&progress, 1);
		PostMessage(GetDlgItem(hwnd, IDC_PROGRESS1), PBM_SETPOS, WPARAM(progress), 0);
	}
}

void CensorDlg::ProcessDirectory(HWND hwnd, std::wstring path)
{
	std::thread timer_thread(&CensorDlg::Timer, this, hwnd);
	PrintIntOutputList(0, L"œŒÀ”◊≈Õ»≈ —œ»— ¿ ‘¿…ÀŒ¬...");
	FilesList files = GetFileListFromDirectory(path.c_str());
	PrintIntOutputList(0, L"Œ¡–¿¡Œ“ ¿..");
	files_count = files.size();
	SendDlgItemMessage(hwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, files.size()));
	
	for (size_t i = 0; i < cores; i++)
	{
		if (threads[i].joinable())
		{
			threads[i].join();
		}
	}
	std::sort(files.begin(), files.end()); // sort by filesize

	std::vector<std::vector<std::wstring>> portions;
	portions.resize(cores);
	bool reversed = false;
	while (files.size())
	{
		int count = files.size() < cores ? files.size() : cores;
		if (reversed)
		{
			for (int i = 0; i < count; i++)
			{
				portions[i].push_back(std::get<std::wstring>(*(files.end() - 1)));
				files.erase(files.end() - 1);
			}
			reversed = false;
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				portions[i].push_back(std::get<std::wstring>(files[0]));
				files.erase(files.begin());
			}
			reversed = true;
		}
	}
	for (size_t i = 0; i < cores; i++)
	{
		threads[i] = std::thread(&CensorDlg::ProcessFiles, this, hwnd, portions[i]);
		portions[i].clear();
	}
	for (size_t i = 0; i < cores; i++)
	{
		threads[i].join();
	}
	kill_thread = true;
	timer_thread.join();
	process_thread.detach();
}

CensorDlg::FilesList CensorDlg::GetFileListFromDirectory(const wchar_t* path, bool recursive)
{
	//Wow64DisableWow64FsRedirection(&OldValue);
	FilesList files;
	WIN32_FIND_DATAW wfd;
	wchar_t* mask = new wchar_t[wcslen(path) + 4];
	wsprintf(mask, TEXT("%s%s"), path, L"\\*");
	HANDLE const hFind = FindFirstFileW(mask, &wfd);
	delete[] mask;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(wfd.cFileName, L".") && wcscmp(wfd.cFileName, L"..") && (recursive))
				{

					wchar_t* filePath = new wchar_t[wcslen(path) + wcslen(wfd.cFileName) + 2];
					wsprintf(filePath, L"%s%s%s", path, L"\\", wfd.cFileName);
					FilesList add = GetFileListFromDirectory(filePath);
					delete[] filePath;

					if (add.size())
					{
						//files.resize(files.size() + add.size());
						files.insert(files.end(), add.begin(), add.end());
					}
				}
			}
			else if (wcslen(wfd.cFileName) >= 4 && !wcscmp(wfd.cFileName + wcslen(wfd.cFileName) - 4, L".txt"))
			{
				wchar_t* filePath = new wchar_t[wcslen(path) + wcslen(wfd.cFileName) + 2];
				wsprintf(filePath, L"%s%s%s", path, L"\\", wfd.cFileName);

				HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				int size = GetFileSize(hFile, NULL);
				CloseHandle(hFile);

				files.push_back(std::make_pair(size, filePath));
				delete[] filePath;
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}

	return files;
}

void CensorDlg::PrintIntOutputList(int i, const wchar_t* text)
{
	WaitForSingleObject(mutex_output, INFINITE);
	mutex_output = CreateMutex(NULL, TRUE, NULL);
	SendMessage(output_list, LB_DELETESTRING, WPARAM(i), 0);
	SendMessage(output_list, LB_INSERTSTRING, WPARAM(i), LPARAM(text));
	ReleaseMutex(mutex_output);
}

void CensorDlg::Timer(HWND hwnd)
{
	using namespace std;
	typedef chrono::high_resolution_clock Time;
	typedef chrono::seconds sec;
	typedef chrono::duration<float> duration;
	auto start = Time::now();
	do
	{
		auto current = Time::now();
		duration fs = current - start;
		sec dur = std::chrono::duration_cast<sec>(fs);
		uint32_t hh = dur.count() / 3600;
		uint32_t mm = (dur.count() % 3600) / 60;
		uint32_t ss = (dur.count() % 3600) % 60;
		wchar_t str[64];
		wsprintf(str, L"œÓ¯ÎÓ ‚ÂÏÂÌË: %02d:%02d:%02d", hh, mm, ss);
		WaitForSingleObject(mutex_output, INFINITE);
		mutex_output = CreateMutex(NULL, TRUE, NULL);
		SendMessage(output_list, LB_DELETESTRING, WPARAM(2), 0);
		SendMessage(output_list, LB_INSERTSTRING, WPARAM(2), LPARAM(str));
		wsprintf(str, L"Œ·‡·ÓÚ‡ÌÓ Ù‡ÈÎÓ‚: %d ËÁ %d", progress, files_count);
		SendMessage(output_list, LB_DELETESTRING, WPARAM(1), 0);
		SendMessage(output_list, LB_INSERTSTRING, WPARAM(1), LPARAM(str));
		ReleaseMutex(mutex_output);
		std::this_thread::sleep_for(1s);
	} while (!kill_thread);
}

CensorDlg::CensorDlg()
{
	cores = std::thread::hardware_concurrency();
	threads = new std::thread[cores];
	ptr = this;
}

BOOL CensorDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	case WM_CTLCOLORLISTBOX:
		if ((HWND)lParam == GetDlgItem(hwnd, IDC_CENSOR_LIST))
		{
			SetTextColor((HDC)wParam, RGB(180, 0, 0));
			return (INT_PTR)brush;
		}
		break;
	}
	
	return FALSE;
}
