#include "CensorDlg.h"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	setlocale(0, "");
	CensorDlg dlg;
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, CensorDlg::DlgProc);
}
