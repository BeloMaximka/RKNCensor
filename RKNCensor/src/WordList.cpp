#include "WordList.h"
#include <algorithm>
#include <fstream>
#include <locale>

void WordList::addWord(HWND list)
{
	WCHAR word[256];
	GetWindowText(list, word, 256);
	if (wcslen(word) && SendMessage(list, LB_FINDSTRINGEXACT, -1, LPARAM(word)) == LB_ERR)
	{
		std::transform(word, word + 256, word, ::towlower);
		SendMessage(list, LB_ADDSTRING, 0, LPARAM(word));
	}
}

void WordList::deleteSelected(HWND list)
{
	int index = SendMessage(list, LB_GETCURSEL, 0, 0);
	SendMessage(list, LB_DELETESTRING, WPARAM(index), 0);
}

void WordList::clear(HWND list)
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
}

void WordList::makeWordList(HWND list, std::vector<std::wstring>& words)
{
	words.clear();
	WCHAR text[256];
	int count = SendMessage(list, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < count; i++)
	{
		SendMessage(list, LB_GETTEXT, WPARAM(i), LPARAM(text));
		words.push_back(text);
	}
}

void WordList::loadWordsFromFile(HWND list)
{
	OPENFILENAME ofn;
	WCHAR szFile[MAX_PATH]{ 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Текстовые файлы\0*.txt\0Все файлы\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetOpenFileName(&ofn);
	WordList::clear(list);

	std::locale loc;
	std::wstring word;
	std::wifstream file = openTextFile(szFile, loc);
	while (!file.eof())
	{
		getline(file, word);
		SendMessage(list, LB_ADDSTRING, NULL, LPARAM(word.c_str()));
	}
	file.close();
}
