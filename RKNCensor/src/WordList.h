#pragma once
#include <Windows.h>
#include "TextFileEncoding.h"
#include <vector>
#include <string>

namespace WordList
{
	void addWord(HWND list, HWND edit);
	void deleteSelected(HWND list);
	void clear(HWND list);
	void makeWordList(HWND list, std::vector<std::wstring>& words);
	void loadWordsFromFile(HWND list);
}