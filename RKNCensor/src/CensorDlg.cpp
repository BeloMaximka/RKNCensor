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
	case IDC_DELWORD:
		DeleteWord(hwnd);
		break;
	case IDC_CLEARLIST_BTN:
		ClearWords(hwnd);
		break;
	case IDC_RADIO1:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), FALSE);
		break;
	case IDC_RADIO2:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), FALSE);
		break;
	case IDC_RADIO3:
		EnableWindow(GetDlgItem(hwnd, IDC_VOLUME_EDIT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_DIR_EDIT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_VIEWDIR_BTN), TRUE);
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

void CensorDlg::DeleteWord(HWND hwnd)
{
	TCHAR word[256];
	int index = SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_DELETESTRING, WPARAM(index), 0);
}

void CensorDlg::ClearWords(HWND hwnd)
{
	SendDlgItemMessage(hwnd, IDC_CENSOR_LIST, LB_RESETCONTENT, 0, 0);
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
