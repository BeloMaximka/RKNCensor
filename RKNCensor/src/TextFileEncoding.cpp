#include "TextFileEncoding.h"

// check string for illegal UTF8 byte sequences
bool isValidUTF8(const char* string)
{
	if (!string)
		return true;

	const unsigned char* bytes = (const unsigned char*)string;
	unsigned int cp;
	int num;

	while (*bytes != 0x00)
	{
		if ((*bytes & 0x80) == 0x00)
		{
			// U+0000 to U+007F 
			cp = (*bytes & 0x7F);
			num = 1;
		}
		else if ((*bytes & 0xE0) == 0xC0)
		{
			// U+0080 to U+07FF 
			cp = (*bytes & 0x1F);
			num = 2;
		}
		else if ((*bytes & 0xF0) == 0xE0)
		{
			// U+0800 to U+FFFF 
			cp = (*bytes & 0x0F);
			num = 3;
		}
		else if ((*bytes & 0xF8) == 0xF0)
		{
			// U+10000 to U+10FFFF 
			cp = (*bytes & 0x07);
			num = 4;
		}
		else
			return false;

		bytes++;
		for (int i = 1; i < num; ++i)
		{
			if ((*bytes & 0xC0) != 0x80)
				return false;
			cp = (cp << 6) | (*bytes & 0x3F);
			bytes++;
		}

		if ((cp > 0x10FFFF) ||
			((cp >= 0xD800) && (cp <= 0xDFFF)) ||
			((cp <= 0x007F) && (num != 1)) ||
			((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
			((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
			((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
			return false;
	}

	return true;
}

CHARACTER_ENCODING getTextFileEncoding(const wchar_t* filename)
{
	CHARACTER_ENCODING encoding = CHARACTER_ENCODING::ANSI;

	unsigned char uniTxt[] = { 0xFF, 0xFE };// Unicode file header
	unsigned char endianTxt[] = { 0xFE, 0xFF };// Unicode big endian file header
	unsigned char utf8Txt[] = { 0xEF, 0xBB };// UTF_8 file header

	DWORD dwBytesRead = 0;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = NULL;
		CloseHandle(hFile);
		return encoding;
	}
	BYTE* lpHeader = new BYTE[2];
	ReadFile(hFile, lpHeader, 2, &dwBytesRead, NULL); // read BOM
	CloseHandle(hFile);

	// check BOM
	if (lpHeader[0] == uniTxt[0] && lpHeader[1] == uniTxt[1])// Unicode file
		encoding = CHARACTER_ENCODING::UTF16_LE;
	else if (lpHeader[0] == endianTxt[0] && lpHeader[1] == endianTxt[1])//  Unicode big endian file
		encoding = CHARACTER_ENCODING::UTF16_BE;
	else if (lpHeader[0] == utf8Txt[0] && lpHeader[1] == utf8Txt[1])// UTF-8 file
		encoding = CHARACTER_ENCODING::UTF8_BOM;
	else // check encodings without BOM
	{

		HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		int size = GetFileSize(hFile, NULL);
		char* buff = new char[size + 1];
		ReadFile(hFile, buff, size, &dwBytesRead, NULL);
		buff[size] = 0;
		CloseHandle(hFile);
		if (isValidUTF8(buff))
			encoding = CHARACTER_ENCODING::UTF8;
		else
			encoding = CHARACTER_ENCODING::ANSI;   //Ascii
		delete[] buff;
	}
	delete[]lpHeader;
	return encoding;
}

std::wifstream openTextFile(const wchar_t* path, std::locale& out_loc)
{
	std::wifstream file(path);
	if (!file.is_open())
		return file;

	std::locale loc("Russian.1251");
	out_loc = loc;
	switch (getTextFileEncoding(path))
	{
	case CHARACTER_ENCODING::UTF16_LE:
	case CHARACTER_ENCODING::UTF16_BE:
		loc = std::locale(std::locale(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>{});
		out_loc = std::locale(std::locale(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::generate_header>{});
		break;
	case CHARACTER_ENCODING::UTF8_BOM:
	case CHARACTER_ENCODING::UTF8:
		loc = std::locale(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>{});
		out_loc = std::locale(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>{});
		break;
	}
	file.imbue(loc);
	return file;
}
