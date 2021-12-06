#pragma once
#include <Windows.h>
#include <cstdio>
#include <vector>
#include <string>

HANDLE OpenVolume(char letter) {
	WCHAR volume_path[8];
	swprintf(volume_path, 8, L"\\\\.\\%c:", letter);
	return CreateFileW(volume_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

bool QueryUsnJournal(HANDLE volume, PUSN_JOURNAL_DATA usn_journal_data)
{
	DWORD cb;
	return DeviceIoControl(volume, FSCTL_QUERY_USN_JOURNAL, NULL, 0, usn_journal_data, sizeof(USN_JOURNAL_DATA), &cb, NULL);
}

bool CreateUsnJournal(HANDLE volume)
{
	DWORD cb;
	CREATE_USN_JOURNAL_DATA cujd;
	cujd.MaximumSize = 32768 * 1024; //32M
	cujd.AllocationDelta = 4096 * 1024; //4M
	return DeviceIoControl(volume, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &cb, NULL);
}

USN OpenUSN(HANDLE volume) {
	USN_JOURNAL_DATA ujd;
	if (!QueryUsnJournal(volume, &ujd)) {
		switch (GetLastError())
		{
		case ERROR_JOURNAL_NOT_ACTIVE:
		{
			CreateUsnJournal(volume);
			QueryUsnJournal(volume, &ujd);
		}
		break;
		case ERROR_JOURNAL_DELETE_IN_PROGRESS:
		{
			DWORD cb;
			DELETE_USN_JOURNAL_DATA del_ujd;
			del_ujd.UsnJournalID = ujd.UsnJournalID;
			del_ujd.DeleteFlags = USN_DELETE_FLAG_NOTIFY;
			DeviceIoControl(volume, FSCTL_DELETE_USN_JOURNAL, &del_ujd, sizeof(DELETE_USN_JOURNAL_DATA), NULL, 0, &cb, NULL);
			CreateUsnJournal(volume);
			QueryUsnJournal(volume, &ujd);
		}
		break;
		}
	}
	return ujd.NextUsn;
}

void ScanVolumeMFT(char letter, std::vector<std::wstring>& files) {
	int buffer_size = sizeof(DWORDLONG) + 0x80000;
	BYTE* recv_buffer = new BYTE[buffer_size];
	ZeroMemory(recv_buffer, buffer_size);
	HANDLE volume = OpenVolume(letter);
	DWORD byte_ret;
	PUSN_RECORD record;
	PUSN_RECORD end;
	MFT_ENUM_DATA med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = 0;
	med.HighUsn = OpenUSN(volume);
	while (DeviceIoControl(volume, FSCTL_ENUM_USN_DATA, &med, sizeof(med), recv_buffer, buffer_size, &byte_ret, NULL))
	{
		record = ((PUSN_RECORD)(&recv_buffer[sizeof(USN)]));
		end = ((PUSN_RECORD)(recv_buffer + byte_ret));
		for (; record < end; record = ((PUSN_RECORD)((PBYTE)record + record->RecordLength)))
		{
			size_t size = wcslen(record->FileName);
			if (size >= 4 && !wcscmp(record->FileName + size - 4, L".txt"))
			{
				FILE_ID_DESCRIPTOR fid = { sizeof(fid), FileIdType };
				fid.FileId.QuadPart = record->FileReferenceNumber;

				FILE_NAME_INFO* fni = (PFILE_NAME_INFO)LocalAlloc(0, 514);
				//ZeroMemory(fni, 514);
				HANDLE file = OpenFileById(volume, &fid, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0, 0);
				GetFileInformationByHandleEx(file, FileNameInfo, fni, 514);
				CloseHandle(file);

				*(fni->FileName + fni->FileNameLength / 2) = 0;
				wchar_t* path = new wchar_t[fni->FileNameLength / 2 + 3];
				wsprintf(path, L"%c%c%s", letter, ':', fni->FileName);
				files.push_back(path);

				delete[] path;
				LocalFree(fni);

			}
		}
		med.StartFileReferenceNumber = *(DWORDLONG*)recv_buffer;
	}
	delete[] recv_buffer;
	CloseHandle(volume);
}