//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "CensorDlg.h"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	setlocale(0, "");
	CensorDlg dlg;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, CensorDlg::DlgProc);
	return 0;
}