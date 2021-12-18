#pragma once
#include <Windows.h>
#include <cstdio>
#include <vector>
#include <string>

namespace MFT
{
	HANDLE OpenVolume(char letter);
	bool QueryUsnJournal(HANDLE volume, PUSN_JOURNAL_DATA usn_journal_data);
	bool CreateUsnJournal(HANDLE volume);
	USN OpenUSN(HANDLE volume);
	std::vector<std::pair<int, std::wstring>> ScanVolumeMFT(char letter);
	std::vector<char> getVolumeList();
	bool isNTFS(char letter);
	bool isVolumeExist(char letter);
}
