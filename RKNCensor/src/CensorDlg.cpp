#include "CensorDlg.h"

CensorDlg* CensorDlg::ptr = NULL;

void CensorDlg::OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CensorDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CensorDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	}
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
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->OnCommand);
	}
	return FALSE;
}
