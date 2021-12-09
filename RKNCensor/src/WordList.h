#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <algorithm>

namespace WordList
{
	void addWord(HWND list);
	void deleteSelected(HWND list);
	void clear(HWND list);
	void makeWordList(HWND list, std::vector<std::wstring>& words);
}