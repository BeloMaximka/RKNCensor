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

void CensorDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_ADDWORD_BTN:
		AddWord(hwnd);
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
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}
