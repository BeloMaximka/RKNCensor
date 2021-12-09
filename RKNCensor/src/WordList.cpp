#include "WordList.h"

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