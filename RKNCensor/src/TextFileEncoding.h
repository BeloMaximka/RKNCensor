#pragma once
#include <string>
#include <Windows.h>
#include <fstream>
#include <locale>
#include <codecvt>

enum class CHARACTER_ENCODING
{

	ANSI,
	UTF16_LE,
	UTF16_BE,
	UTF8_BOM,
	UTF8
};

// check string for illegal UTF8 byte sequences
bool isValidUTF8(const char* string);
CHARACTER_ENCODING getTextFileEncoding(const wchar_t* filename);
std::wifstream openTextFile(const wchar_t* path, std::locale& out_loc);
